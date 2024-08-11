#include "heating.h"
/***/
#include "heat_info.h"
#include "buzzer.h"
#include "set_heat_info_page.h"
#include "remind_page.h"
#include "my_lcd.h"
#include "color.h"
#include "heat_pad.h"
#include "measure.h"
#include "multi_button_port.h"
#include "lcd_init.h"
/***/
#define LOG_TAG "heating"
#include "elog.h"
//
#include "multi_button.h"
//
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#include "task.h"
/***/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static TaskHandle_t heating_task_handle;
static StaticTask_t heating_task;
static uint8_t heating_task_stack[1024];

static struct Button encoder_btn;

static uint16_t temp_record[140];
static uint32_t record_num;
static uint16_t temp_show_x[140];
static uint16_t temp_show_y[140];
static uint16_t cur_power, cur_temp;

static HeatInfo_t heat_info;

static uint16_t find_max;

static void heating_ui_renew_cn() {
  lcd_fill(0, 0, LCD_W, LCD_H, heat_info.check_slant ? BLACK : RED);

  const uint16_t OX = 20, OY = 110;
  const uint16_t TEMP_PER_PIXEL = 1, SEC_PER_PIXEL = 1;

  // 坐标原点：20，110，x轴全长：140，y轴全长：110
  // x 轴为时间坐标，每像素 1 秒
  // y 轴为温度坐标，每像素 2 摄氏度

  // 显示坐标轴// y轴
  lcd_draw_line(20, 0, 20, 110, WHITE);    // y轴
  lcd_draw_line(0, 110, 160, 110, WHITE);  // x轴

  // 最新温度为 y 轴中值，
  // 同时限制 y 轴最大温度为 320℃，
  // 最小温度为 0℃，
  // 基准温度为 y 轴最小值

  if (record_num >= 1) {
    // 找出基准温度
    uint16_t benchmark_temp =
        temp_record[(record_num - 1) % (sizeof(temp_record) / 2)] >
                55 * TEMP_PER_PIXEL
            ? temp_record[(record_num - 1) % (sizeof(temp_record) / 2)] -
                  55 * TEMP_PER_PIXEL
            : 0;
    if (benchmark_temp > 320 - TEMP_PER_PIXEL * 110)
      benchmark_temp = 320 - TEMP_PER_PIXEL * 110;

    // 计算温度坐标
    for (size_t i = 0; i < (record_num > 140 ? 140 : record_num); i++) {
      temp_show_x[i] = 21 + i;  // range: [21, 160]
      if (temp_record[(record_num - (record_num > 140 ? 140 : record_num) + i) %
                      (sizeof(temp_record) / 2)] >
          benchmark_temp + TEMP_PER_PIXEL * 110)
        temp_show_y[i] = 0;
      else if (temp_record[(record_num - (record_num > 140 ? 140 : record_num) +
                            i) %
                           (sizeof(temp_record) / 2)] < benchmark_temp)
        temp_show_y[i] = 109;
      else
        temp_show_y[i] =
            OY - (temp_record[(record_num -
                               (record_num > 140 ? 140 : record_num) + i) %
                              (sizeof(temp_record) / 2)] -
                  benchmark_temp) /
                     TEMP_PER_PIXEL;  // range: [0, 110]
    }

    // 显示 y 轴标签
    lcd_show_num(0, 0, benchmark_temp + TEMP_PER_PIXEL * 110, 3, WHITE, WHITE,
                 12);
    lcd_show_num(0, 50, benchmark_temp + 55, 3, WHITE, WHITE, 12);
    lcd_show_num(0, 95, benchmark_temp, 3, WHITE, WHITE, 12);

    // 显示曲线
    lcd_draw_multi_line(temp_show_x, temp_show_y,
                        record_num > 140 ? 140 : record_num, WHITE);

    // 显示温度和功率
    lcd_show_font(0, 115, (uint8_t *)"温度：", WHITE, WHITE, FontSize12);
    lcd_show_num(37, 115, cur_temp, 3, WHITE, WHITE, 12);
    lcd_show_font(60, 115, (uint8_t *)"℃", WHITE, WHITE, FontSize12);

    lcd_show_font(80, 115, (uint8_t *)"功率：", WHITE, WHITE, FontSize12);
    lcd_show_num(117, 115, cur_power, 3, WHITE, WHITE, 12);
    lcd_show_str(140, 115, (uint8_t *)"W", WHITE, WHITE, 12);

  } else {
    // 显示缺省 y 轴标签
    lcd_show_num(0, 0, TEMP_PER_PIXEL * 110, 3, WHITE, WHITE, 12);
    lcd_show_num(0, 50, 55, 3, WHITE, WHITE, 12);
    lcd_show_num(0, 95, 0, 3, WHITE, WHITE, 12);

    // 显示缺省温度和功率
    lcd_show_str(0, 115, (uint8_t *)"temp: N/A C", WHITE, WHITE, 12);
    lcd_show_str(80, 115, (uint8_t *)"temp: N/A C", WHITE, WHITE, 12);
  }

  lcd_refreash_request();
}

static void btn_handler(void *param) {
  struct Button *btn = (struct Button *)param;
  PressEvent evt     = get_button_event(btn);

  switch (btn->button_id) {
    case EncoderBtn: {
      if (evt == LONG_PRESS_START) {
        set_heat_info_page_start();
        heating_stop();
      }
      break;
    }
  }
}

static void heating_task_code() {
  // 收集和显示温度和功率数据
  // 倾斜检测

  TickType_t wait_notify_tick = portMAX_DELAY, temp_record_tick = 0;

  MeasurePwrInfo_t pwr_info;
  MeasureSportInfo_t sport_info;

  int8_t target_power         = 0;
  uint8_t cur_stage_num       = 0;
  TickType_t first_reach_tick = portMAX_DELAY, cur_stage_tick = 0,
             last_check_tick = 0;

  uint8_t slant_count = 0, vl_count = 0;

  uint32_t notify_val = 0;

  while (1) {
    xTaskNotifyWait(0, 0xFFFFFFFF, &notify_val, wait_notify_tick);

    if (notify_val & (1 << 0)) {  // 启动信号
      wait_notify_tick = 0;
      cur_stage_tick   = 0;
      first_reach_tick = portMAX_DELAY;
      last_check_tick  = xTaskGetTickCount();
      record_num       = 0;
      target_power     = 0;
      cur_stage_num    = 0;
      slant_count      = 0;
      vl_count         = 0;
      heat_pad_set_max_pwr(heat_info.power_limit);
      if (heat_info.is_constant) heat_pad_set_temp(heat_info.stage[0].temp);
      continue;
    }
    if (notify_val & (1 << 1)) {  // 停止信号
      wait_notify_tick = portMAX_DELAY;

      heat_pad_disable();
      continue;
    }

    measure_pwr_data_get(&pwr_info);
    measure_sport_data_get(&sport_info);

    // 检测倾斜状态和电压
    if (heat_info.check_slant) {
      if (sport_info.acc_mg_all < 800 || sport_info.acc_mg_all > 1200 ||
          sport_info.acc_z_mg < 900 || sport_info.acc_z_mg > 1100)
        slant_count++;
      else
        slant_count = 0;
      if (pwr_info.pwr_in_vol < 16.5)
        vl_count++;
      else
        vl_count = 0;
      if (slant_count >= 5U) {
        log_i(
            "stop heat, reason: machine tilt, acc all: %.2f mg, acc "
            "z: %.2f mg",
            sport_info.acc_mg_all, sport_info.acc_z_mg);
        heating_stop();
        remind_page_start(RemindTypeSlant);
        continue;
      } else if (vl_count >= 20U) {
        log_i("stop heat, reason: voltage too low, voltage: %.2f",
              pwr_info.pwr_in_vol);
        heating_stop();
        remind_page_start(RemindTypeVl);
        continue;
      }
    }

    // 控制温度
    if (!heat_info.is_constant) {
      // 计算加热时间
      // 正负误差 5 摄氏度都算达到温度区间
      if (fabsf(pwr_info.heat_pad_temp - heat_info.stage[cur_stage_num].temp) <
          5.0f) {
        cur_stage_tick += xTaskGetTickCount() - last_check_tick;
        if (first_reach_tick == portMAX_DELAY) {
          first_reach_tick = xTaskGetTickCount();
          log_i("first reach stage %u temp, tick val: %u, be equal to %u ms",
                cur_stage_num, first_reach_tick,
                first_reach_tick / portTICK_PERIOD_MS);
        }
      }

      // 检测加热过程
      if (cur_stage_tick / portTICK_PERIOD_MS >=
          pdMS_TO_TICKS(heat_info.stage[cur_stage_num].duartion_s * 1000)) {
        log_i("stage %u heat complete, time consuming: %u ms", cur_stage_num,
              (xTaskGetTickCount() - first_reach_tick) / portTICK_PERIOD_MS);
        // 切换加热过程
        if (cur_stage_num != 4 &&
            heat_info.stage[cur_stage_num + 1].duartion_s != 0) {
          cur_stage_num++;
          first_reach_tick = portMAX_DELAY;
          cur_stage_tick   = 0;
          log_i("switch to heat stage %u: temp: %u , duartion: %u s",
                cur_stage_num, heat_info.stage[cur_stage_num].temp,
                heat_info.stage[cur_stage_num].duartion_s);
        } else {
          // 加热结束
          log_i("end of heat");
          buzzer_sing(BuzzerToneSuccess);
          heating_stop();
          remind_page_start(RemindTypeFinish);
          continue;
        }
      }
      // 设置目标温度
      heat_pad_set_temp(heat_info.stage[cur_stage_num].temp);
    }
    last_check_tick = xTaskGetTickCount();

    // 记录温度
    cur_temp  = (uint16_t)roundf(pwr_info.heat_pad_temp);
    cur_power = roundf(pwr_info.heat_pad_pwr);
    if (pdMS_TO_TICKS(xTaskGetTickCount() - temp_record_tick) >= 1000U) {
      temp_record_tick = xTaskGetTickCount();
      temp_record[record_num++ % (sizeof(temp_record) / 2)] = cur_temp;
    }

    // 更新界面
    heating_ui_renew_cn();

    vTaskDelay(pdMS_TO_TICKS(100U));
  }
}

void heating_start(HeatInfo_t *p_heat_info) {
  memcpy(&heat_info, p_heat_info, sizeof(HeatInfo_t));

  button_start(&encoder_btn);

  xTaskNotify(heating_task_handle, 1 << 0, eSetBits);
}

void heating_stop() {
  button_stop(&encoder_btn);

  xTaskNotify(heating_task_handle, 1 << 1, eSetBits);
}

void heating_init() {
  button_init(&encoder_btn, multi_button_port_read_btn, 0, EncoderBtn);

  button_attach(&encoder_btn, LONG_PRESS_START, btn_handler);

  for (size_t i = 0; i < sizeof(temp_show_x); i++) {
    temp_show_x[i] = 20;
    temp_show_y[i] = 20;
  }

  heating_task_handle = xTaskCreateStatic(
      heating_task_code, "heating", sizeof(heating_task_stack) / 4, NULL, 14,
      (StackType_t *)&heating_task_stack, &heating_task);
  configASSERT(heating_task_handle);
}
