#include "measure.h"
/***/
#define LOG_TAG "measure"
#include "elog.h"
//
#include "lsm6ds3tr-c_reg.h"
//
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
/***/
#include "gd32f4xx.h"
/***/
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#define ADC_RAW_IDX_PWR_IN_VOL            0
#define ADC_RAW_IDX_HEAT_CUR_SIG          1
#define ADC_RAW_IDX_THERMOCOUPLE_SIG      2
#define ADC_RAW_IDX_HEAT_VOL_SIG          3
#define ADC_RAW_IDX_THERMOCOUPLE_SIG_VREF 4

#define PWR_VOL_R_H                       68000.0f
#define PWR_VOL_R_L                       7500.0f

#define HEAT_PAD_VOL_R_H                  68000.0f
#define HEAT_PAD_VOL_R_L                  7500.0f

#define HEAT_PAD_CUR_R                    0.005f

#define ADC_FULL                          4095.0f

#define VREF                              3.0f

#define GET_REALITY_VOL(adc_val)          (float_t) adc_val / ADC_FULL *VREF

// pwr measure var
static TaskHandle_t measure_pwr_task_handle;
static StaticTask_t measure_pwr_task;
static uint8_t measure_pwr_task_stack[1024];

static uint16_t adc_dma_buf[5];

static SemaphoreHandle_t measure_pwr_info0_mutex_handle;
static StaticSemaphore_t measure_pwr_info0_mutex;
static SemaphoreHandle_t measure_pwr_info1_mutex_handle;
static StaticSemaphore_t measure_pwr_info1_mutex;

static MeasurePwrInfo_t pwr_info0;
static MeasurePwrInfo_t pwr_info1;

// sport measure var
static TaskHandle_t measure_sport_task_handle;
static StaticTask_t measure_sport_task;
static uint8_t measure_sport_task_stack[1024];

static SemaphoreHandle_t measure_sport_data_mutex_handle;
static StaticSemaphore_t measure_sport_data_mutex;

static MeasureSportInfo_t sport_info;

void measure_pwr_task_code() {
  uint8_t log_count;

  dma_single_data_parameter_struct init_struct = {
      .periph_addr         = (uint32_t)(&ADC_RDATA(ADC0)),
      .periph_inc          = DMA_PERIPH_INCREASE_DISABLE,
      .memory0_addr        = (uint32_t)adc_dma_buf,
      .memory_inc          = DMA_MEMORY_INCREASE_ENABLE,
      .periph_memory_width = DMA_PERIPH_WIDTH_16BIT,
      .circular_mode       = DMA_CIRCULAR_MODE_DISABLE,
      .direction           = DMA_PERIPH_TO_MEMORY,
      .number              = 5,
      .priority            = DMA_PRIORITY_HIGH,
  };

  MeasurePwrInfo_t *p_write_pwr_info = &pwr_info0;

  uint32_t sample_count = 0;

  const uint8_t TMP_SIZE = 10;
  float_t pwr_in_vol_tmp[TMP_SIZE];
  float_t heat_pad_vol_tmp[TMP_SIZE];
  float_t heat_pad_cur_tmp[TMP_SIZE];
  float_t heat_pad_temp_tmp[TMP_SIZE];
  uint8_t tmp_idx_write_head = 0;

  while (1) {
    dma_deinit(DMA1, DMA_CH0);                               // dma反初始化
    dma_single_data_mode_init(DMA1, DMA_CH0, &init_struct);  // 配置dma
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    dma_flow_controller_config(DMA1, DMA_CH0, DMA_FLOW_CONTROLLER_PERI);

    adc_interrupt_enable(ADC0, ADC_INT_EOC);

    dma_channel_enable(DMA1, DMA_CH0);                       // 使能dma通道
    adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);  // 软件触发adc转换

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // 等待转换完成中断

    pwr_in_vol_tmp[tmp_idx_write_head] =
        GET_REALITY_VOL(adc_dma_buf[ADC_RAW_IDX_PWR_IN_VOL]) /
        (PWR_VOL_R_L / (PWR_VOL_R_H + PWR_VOL_R_L));

    heat_pad_vol_tmp[tmp_idx_write_head] =
        GET_REALITY_VOL(adc_dma_buf[ADC_RAW_IDX_HEAT_VOL_SIG]) /
        (HEAT_PAD_VOL_R_L / (HEAT_PAD_VOL_R_H + HEAT_PAD_VOL_R_L));

    heat_pad_cur_tmp[tmp_idx_write_head] =
        GET_REALITY_VOL(adc_dma_buf[ADC_RAW_IDX_HEAT_CUR_SIG]) / 0.5f;

    heat_pad_temp_tmp[tmp_idx_write_head] =
        (GET_REALITY_VOL(adc_dma_buf[ADC_RAW_IDX_THERMOCOUPLE_SIG]) -
         GET_REALITY_VOL(adc_dma_buf[ADC_RAW_IDX_THERMOCOUPLE_SIG_VREF])) /
        0.005f;

    if (++tmp_idx_write_head >= TMP_SIZE) tmp_idx_write_head = 0;

    sample_count++;

    // 获取互斥锁
    if (p_write_pwr_info == &pwr_info0) {
      xSemaphoreTake(measure_pwr_info0_mutex_handle, portMAX_DELAY);
    } else if (p_write_pwr_info == &pwr_info1) {
      xSemaphoreTake(measure_pwr_info1_mutex_handle, portMAX_DELAY);
    }

    // 计算结果
    for (size_t i = 0; i < (sample_count > TMP_SIZE ? TMP_SIZE : sample_count);
         i++) {
      p_write_pwr_info->pwr_in_vol += pwr_in_vol_tmp[i];
      p_write_pwr_info->heat_pad_vol += heat_pad_vol_tmp[i];
      p_write_pwr_info->heat_pad_cur += heat_pad_cur_tmp[i];
      p_write_pwr_info->heat_pad_temp += heat_pad_temp_tmp[i];
    }
    p_write_pwr_info->pwr_in_vol /=
        (sample_count > TMP_SIZE ? TMP_SIZE : sample_count);
    p_write_pwr_info->heat_pad_vol /=
        (sample_count > TMP_SIZE ? TMP_SIZE : sample_count);
    p_write_pwr_info->heat_pad_cur /=
        (sample_count > TMP_SIZE ? TMP_SIZE : sample_count);
    p_write_pwr_info->heat_pad_temp /=
        (sample_count > TMP_SIZE ? TMP_SIZE : sample_count);

    p_write_pwr_info->heat_pad_pwr =
        p_write_pwr_info->heat_pad_vol * p_write_pwr_info->heat_pad_cur;

    // 释放互斥锁并切换 buf
    if (p_write_pwr_info == &pwr_info0) {
      xSemaphoreGive(measure_pwr_info0_mutex_handle);
      p_write_pwr_info = &pwr_info1;
    } else if (p_write_pwr_info == &pwr_info1) {
      xSemaphoreGive(measure_pwr_info1_mutex_handle);
      p_write_pwr_info = &pwr_info0;
    }

    // 每隔一段时间输出一次log
    // if (log_count++ > 80) {
    //   log_count = 0;
    //   log_i(
    //       "pwr in vol:%04u heat cur sig:%04u thermocouple sig:%04u "
    //       "heat vol sig:%04u thermocouple sig vref:%04u\npwr in vol:%.2f "
    //       "heat pad vol:%.2f heat pad cur:%.2f heat pad temp:%.2f",
    //       adc_dma_buf[ADC_RAW_IDX_PWR_IN_VOL],
    //       adc_dma_buf[ADC_RAW_IDX_HEAT_CUR_SIG],
    //       adc_dma_buf[ADC_RAW_IDX_THERMOCOUPLE_SIG],
    //       adc_dma_buf[ADC_RAW_IDX_HEAT_VOL_SIG],
    //       adc_dma_buf[ADC_RAW_IDX_THERMOCOUPLE_SIG_VREF], pwr_in_vol_tmp,
    //       heat_pad_vol_tmp, heat_pad_cur_tmp, heat_pad_temp_tmp);
    // }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void measure_sport_task_code() {
  int16_t fifo_raw[3];
  while (1) {
    xSemaphoreTake(measure_sport_data_mutex_handle, portMAX_DELAY);

    // lsm6ds3tr_c_fifo_raw_data_get(NULL, (uint8_t *)fifo_raw,
    // sizeof(fifo_raw));
    lsm6ds3tr_c_acceleration_raw_get(NULL, fifo_raw);
    // lsm6ds3tr_c_fifo_pattern_get(NULL, NULL);

    sport_info.acc_x_mg = lsm6ds3tr_c_from_fs2g_to_mg(fifo_raw[0]);
    sport_info.acc_y_mg = lsm6ds3tr_c_from_fs2g_to_mg(fifo_raw[1]);
    sport_info.acc_z_mg = lsm6ds3tr_c_from_fs2g_to_mg(fifo_raw[2]);
    sport_info.acc_mg_all =
        pow(pow(sport_info.acc_x_mg, 2) + pow(sport_info.acc_y_mg, 2) +
                pow(sport_info.acc_z_mg, 2),
            0.5);

    // log_d("acc x: %f mg, y: %f mg, z: %f mg all: %f mg", sport_info.acc_x_mg,
    //       sport_info.acc_y_mg, sport_info.acc_z_mg, sport_info.acc_mg_all);

    vTaskDelay(500);

    xSemaphoreGive(measure_sport_data_mutex_handle);
  }
}

void measure_init() {
  measure_pwr_info0_mutex_handle =
      xSemaphoreCreateMutexStatic(&measure_pwr_info0_mutex);
  configASSERT(measure_pwr_info0_mutex_handle);

  measure_pwr_info1_mutex_handle =
      xSemaphoreCreateMutexStatic(&measure_pwr_info1_mutex);
  configASSERT(measure_pwr_info1_mutex_handle);

  measure_pwr_task_handle = xTaskCreateStatic(
      measure_pwr_task_code, "measurePwr", sizeof(measure_pwr_task_stack) / 4,
      NULL, 10, (StackType_t *)measure_pwr_task_stack, &measure_pwr_task);
  configASSERT(measure_pwr_task_handle);

  measure_sport_data_mutex_handle =
      xSemaphoreCreateMutexStatic(&measure_sport_data_mutex);
  configASSERT(measure_sport_data_mutex_handle);

  measure_sport_task_handle = xTaskCreateStatic(
      measure_sport_task_code, "measureSport",
      sizeof(measure_sport_task_stack) / 4, NULL, 10,
      (StackType_t *)measure_sport_task_stack, &measure_sport_task);
  configASSERT(measure_sport_task_handle);

  log_i("init complete");
}

void measure_pwr_data_get(MeasurePwrInfo_t *out) {
  configASSERT(out != NULL);

  if (xSemaphoreTake(measure_pwr_info0_mutex_handle, 0) == pdTRUE) {
    memcpy(out, &pwr_info0, sizeof(MeasurePwrInfo_t));
    xSemaphoreGive(measure_pwr_info0_mutex_handle);
  } else if (xSemaphoreTake(measure_pwr_info1_mutex_handle, portMAX_DELAY) ==
             pdTRUE) {
    memcpy(out, &pwr_info1, sizeof(MeasurePwrInfo_t));
    xSemaphoreGive(measure_pwr_info1_mutex_handle);
  }
}

void measure_sport_data_get(MeasureSportInfo_t *out) {
  configASSERT(out != NULL);

  xSemaphoreTake(measure_sport_data_mutex_handle, portMAX_DELAY);

  memcpy(out, &sport_info, sizeof(MeasureSportInfo_t));

  xSemaphoreGive(measure_sport_data_mutex_handle);
}

void ADC_IRQHandler() {
  if (adc_interrupt_flag_get(ADC0, ADC_INT_FLAG_EOC) == SET) {
    adc_interrupt_flag_clear(ADC0, ADC_INT_FLAG_EOC);
  }
  vTaskNotifyGiveFromISR(measure_pwr_task_handle, NULL);
}
