#include "heat_pad.h"
/***/
#include "pid.h"
#include "measure.h"
#include "pwr_manage.h"
/***/
#define LOG_TAG "heat pad"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/***/
#include "gd32f4xx.h"

typedef enum {
  HeatPadCtrlActionDisable,
  HeatPadCtrlActionSetTemp,
  HeatPadCtrlActionSetMaxPower,
} HeatPadCtrlAction_t;

typedef struct {
  HeatPadCtrlAction_t act;
  uint16_t val;
} HeatPadCtrlPkg_t;

const uint16_t HEAT_PAD_MIN_TEMP = 100, HEAT_PAD_MAX_TEMP = 320;

static TaskHandle_t heat_pad_ctrl_task_handle;
static StaticTask_t heat_pad_ctrl_task;
static uint8_t heat_pad_ctrl_task_stack[1024];

static QueueHandle_t heat_pad_ctrl_queue_handle;
static uint8_t heat_pad_ctrl_queue_buf[sizeof(HeatPadCtrlPkg_t) * 4];
static StaticQueue_t heat_pad_ctrl_queue;

void heat_pad_ctrl_task_code() {
  const float_t TEMP_L_H_SPLIT = 280.0F;

  const float_t TEMP_DIR_PID_P_LTEMP = 3.0F, TEMP_DIR_PID_I_LTEMP = 0.1F,
                TEMP_DIR_PID_D_LTEMP = 0.2F;

  const float_t TEMP_DIR_PID_P_HTEMP = 3.0F, TEMP_DIR_PID_I_HTEMP = 0.1F,
                TEMP_DIR_PID_D_HTEMP = 0.2F;

  // 功率控制使用增量式pid，温度控制使用位置式pid
  PidIncData_t pwr_inc_pid = {
      .kp         = 0.16f,
      .ki         = 0.1f,
      .kd         = 0.12f,
      .last_err1  = 0.0f,
      .last_err2  = 0.0f,
      .target     = 0.0f,
      .current    = 0.0f,
      .inc_val    = 0.0f,
      .dead_space = 1.0f,
  };
  PidDirData_t temp_dir_pid = {
      .kp           = 2.5f,
      .ki           = 0.1f,
      .kd           = 0.2f,
      .integral     = 0.0f,
      .integral_max = 0.0f,
      .current      = 0.0f,
      .target       = 0.0f,
      .last_err     = 0.0f,
      .out          = 0.0f,
      .out_min      = -100.0f,
      .out_max      = 60.0f,
  };

  MeasurePwrInfo_t pwr_info;

  TickType_t weak_tick = 0;

  float_t heat_pad_duty      = 0;
  uint8_t heat_pad_is_enable = 0;

  HeatPadCtrlPkg_t heat_pad_ctrl_pkg;
  TickType_t wait_queue_tick = 0;
  BaseType_t queue_recv_result;
  while (1) {
    queue_recv_result = xQueueReceive(heat_pad_ctrl_queue_handle,
                                      &heat_pad_ctrl_pkg, wait_queue_tick);

    weak_tick = xTaskGetTickCount();

    if (queue_recv_result == pdPASS) {
      switch (heat_pad_ctrl_pkg.act) {
        case HeatPadCtrlActionDisable: {
          temp_dir_pid.target  = 30;  // 设置一个默认值
          temp_dir_pid.out_max = 60;
          heat_pad_is_enable   = 0;
          heat_pad_duty        = 0;
          continue;
          break;
        }
        case HeatPadCtrlActionSetMaxPower: {
          wait_queue_tick      = 0;
          temp_dir_pid.out_max = heat_pad_ctrl_pkg.val;
          heat_pad_is_enable   = 1;
          break;
        }
        case HeatPadCtrlActionSetTemp: {
          wait_queue_tick     = 0;
          temp_dir_pid.target = heat_pad_ctrl_pkg.val;
          heat_pad_is_enable  = 1;
          break;
        }
        default:
          break;
      }
    }

    // 更新功率数据
    measure_pwr_data_get(&pwr_info);

    if (!heat_pad_is_enable) {
      pwr_manage_sys_fan_duty_set(25);
      timer_channel_output_pulse_value_config(TIMER10, TIMER_CH_0, 0);
      if (pwr_info.heat_pad_temp < 60) {
        wait_queue_tick = portMAX_DELAY;
        pwr_manage_heat_pad_fan_duty_set(0);
        continue;
      } else {
        wait_queue_tick = 0;
        pwr_manage_heat_pad_fan_duty_set(100);
      }
    } else {
      // 温度pid控制

      // 动态参数
      if (pwr_info.heat_pad_temp > TEMP_L_H_SPLIT) {
        temp_dir_pid.kp = TEMP_DIR_PID_P_HTEMP;
        temp_dir_pid.ki = TEMP_DIR_PID_I_HTEMP;
        temp_dir_pid.kd = TEMP_DIR_PID_D_HTEMP;
      } else {
        temp_dir_pid.kp = TEMP_DIR_PID_P_LTEMP;
        temp_dir_pid.ki = TEMP_DIR_PID_I_LTEMP;
        temp_dir_pid.kd = TEMP_DIR_PID_D_LTEMP;
      }
      temp_dir_pid.integral_max = pwr_info.heat_pad_temp * 0.12;

      temp_dir_pid.current = pwr_info.heat_pad_temp;
      pid_dir(&temp_dir_pid);

      if (pwr_info.heat_pad_temp < temp_dir_pid.target) {
        if (pwr_info.heat_pad_temp > TEMP_L_H_SPLIT)
          pwr_inc_pid.target = pwr_info.heat_pad_temp * 0.13f;
        else
          pwr_inc_pid.target = pwr_info.heat_pad_temp * 0.12f;
      }

      // 温度pid输出目标功率，将目标功率写入功率pid
      if (temp_dir_pid.out > 0) {
        if (pwr_info.heat_pad_temp > TEMP_L_H_SPLIT)
          pwr_inc_pid.target =
              temp_dir_pid.out + pwr_info.heat_pad_temp * 0.13f;
        else
          pwr_inc_pid.target =
              temp_dir_pid.out + pwr_info.heat_pad_temp * 0.12f;
        pwr_manage_heat_pad_fan_duty_set(0);
      } else if (temp_dir_pid.out > -10.0F) {
        pwr_manage_heat_pad_fan_duty_set(0);
        if (pwr_info.heat_pad_temp > TEMP_L_H_SPLIT)
          pwr_inc_pid.target = pwr_info.heat_pad_temp * 0.13f;
        else
          pwr_inc_pid.target = pwr_info.heat_pad_temp * 0.12f;
      } else if (temp_dir_pid.out > -20.0F) {
        pwr_manage_heat_pad_fan_duty_set(0);
        if (pwr_info.heat_pad_temp > TEMP_L_H_SPLIT)
          pwr_inc_pid.target =
              temp_dir_pid.out + pwr_info.heat_pad_temp * 0.13f;
        else
          pwr_inc_pid.target =
              temp_dir_pid.out + pwr_info.heat_pad_temp * 0.12f;
      } else {
        if (pwr_info.heat_pad_temp - temp_dir_pid.target >
            pwr_info.heat_pad_temp * 0.1)
          pwr_manage_heat_pad_fan_duty_set(temp_dir_pid.out);
        else
          pwr_manage_heat_pad_fan_duty_set(0);
        pwr_inc_pid.target = 0;
      }

      // 限制功率
      if (pwr_inc_pid.target > temp_dir_pid.out_max)
        pwr_inc_pid.target = temp_dir_pid.out_max;

      // 功率pid控制
      pwr_inc_pid.current = pwr_info.heat_pad_pwr;
      pid_inc(&pwr_inc_pid);
      heat_pad_duty += pwr_inc_pid.inc_val;
      // 加热台pwm占空比限制
      if (heat_pad_duty > 100)
        heat_pad_duty = 100;
      else if (heat_pad_duty < 0)
        heat_pad_duty = 0;

      // 设置加热台 pwm 占空比
      timer_channel_output_pulse_value_config(
          TIMER10, TIMER_CH_0, heat_pad_duty > 100 ? 2000 : 20 * heat_pad_duty);

      // 没有设计系统温度检测硬件，根据加热功率调整系统散热风扇
      pwr_manage_sys_fan_duty_set((uint8_t)heat_pad_duty + 25U > 80U
                                      ? 80U
                                      : (uint8_t)heat_pad_duty + 25U);
    }

    xTaskDelayUntil(&weak_tick, pdMS_TO_TICKS(100));
  }
}

void heat_pad_set_temp(uint16_t temp) {
  if (temp < HEAT_PAD_MIN_TEMP || temp > HEAT_PAD_MAX_TEMP) return;
  HeatPadCtrlPkg_t pkg = {
      .act = HeatPadCtrlActionSetTemp,
      .val = temp,
  };
  xQueueSendToFront(heat_pad_ctrl_queue_handle, &pkg, portMAX_DELAY);
}

void heat_pad_set_max_pwr(uint8_t p) {
  if (p > 120) return;
  HeatPadCtrlPkg_t pkg = {
      .act = HeatPadCtrlActionSetMaxPower,
      .val = p,
  };
  xQueueSendToFront(heat_pad_ctrl_queue_handle, &pkg, portMAX_DELAY);
}

void heat_pad_disable() {
  HeatPadCtrlPkg_t pkg = {
      .act = HeatPadCtrlActionDisable,
      .val = 0,
  };
  xQueueSendToBack(heat_pad_ctrl_queue_handle, &pkg, portMAX_DELAY);
}

void heat_pad_init() {
  heat_pad_ctrl_queue_handle = xQueueCreateStatic(
      sizeof(heat_pad_ctrl_queue_buf) / sizeof(HeatPadCtrlPkg_t),
      sizeof(HeatPadCtrlPkg_t), heat_pad_ctrl_queue_buf, &heat_pad_ctrl_queue);
  configASSERT(heat_pad_ctrl_queue_handle);

  heat_pad_ctrl_task_handle = xTaskCreateStatic(
      heat_pad_ctrl_task_code, "heatPadCtrl",
      sizeof(heat_pad_ctrl_task_stack) / 4, NULL, 15,
      (StackType_t *)heat_pad_ctrl_task_stack, &heat_pad_ctrl_task);
  configASSERT(heat_pad_ctrl_task_handle);

  log_i("init complete");
}