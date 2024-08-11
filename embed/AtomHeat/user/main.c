#include "buzzer.h"
#include "hw_init.h"
#include "lcd_init.h"
#include "pwr_manage.h"
#include "remind_page.h"
#include "heat_pad.h"
#include "led.h"
#include "my_lcd.h"
#include "encoder.h"
#include "measure.h"
#include "heating.h"
#include "multi_button_port.h"
#include "my_lcd.h"
#include "set_heat_info_page.h"
#include "logo_page.h"
/***/
#define LOG_TAG "main"
#include "elog.h"
//
#include "easyflash.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "systick.h"
#include "gd32f4xx.h"
/***/
#include <stddef.h>
#include <stdio.h>

static TaskHandle_t init_task_handle;
static StaticTask_t init_task;
static uint8_t init_task_stack[2048];

static void init_code() {
  elog_init();
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_start();

  on_chip_hw_init();   // 片上硬件初始化
  on_board_hw_init();  // 板级硬件初始化

  // fmc_wscnt_set(WS_WSCNT_15);

  // 软件包初始化
  EfErrCode ef_err;
  ef_err = easyflash_init();
  configASSERT(ef_err == EF_NO_ERR);

  // lcd 初始化
  LCD_Init();
  lcd_handler_init();

  // 所有任务使用静态分配方式，在此初始化

  led_init();                // 开发板 led 初始化
  multi_button_port_init();  // 按键扫描任务
  buzzer_init();             // 蜂鸣器任务
  measure_init();            // 测量任务
  encoder_init();            // 编码器任务
  heat_pad_init();           // 加热台

  // 应用程序任务初始化
  logo_page_init();
  set_heat_info_page_init();
  heating_init();
  remind_page_init();

  pwr_manage_12v_enable();
  pwr_manage_24v_enable();

  // 开机提示音
  buzzer_sing(BuzzerToneSuccess);

  // 启动 LOGO 页
  logo_page_start();

  lcd_refreash_request();

  // 显示开机次数
  uint32_t boot_count, saved_size, read_size;
  read_size = ef_get_env_blob("bootCount", &boot_count, sizeof(boot_count),
                              &saved_size);
  log_d("read size: %u, saved size: %u, boot count: %u", read_size, saved_size,
        boot_count);

  boot_count++;
  ef_err = ef_set_env_blob("bootCount", &boot_count, sizeof(uint32_t));
  log_d("ef err: %u", ef_err);

  vTaskDelete(NULL);
}

int main() {
  nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

  init_task_handle = xTaskCreateStatic(
      init_code, "init", sizeof(init_task_stack) / 4, NULL,
      configMAX_PRIORITIES - 1, (StackType_t *)init_task_stack, &init_task);
  configASSERT(init_task_handle);

  systick_config();

  vTaskStartScheduler();
}
