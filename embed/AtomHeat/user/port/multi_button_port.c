#include "multi_button_port.h"
/***/
#define LOG_TAG "multi button port"
#include "elog.h"
//
#include "multi_button.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"
/***/
#include <stdbool.h>

static TaskHandle_t btn_handler_task_handle;
static StaticTask_t btn_handler_task;
static uint8_t btn_handler_task_stack[2048];

const uint32_t btn_port_table[BtnMax] = {
    GPIOD, GPIOD, GPIOD, GPIOB, GPIOB, GPIOB,
};

const uint32_t btn_pin_table[BtnMax] = {
    GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_5, GPIO_PIN_4, GPIO_PIN_8,
};

uint8_t multi_button_port_read_btn(uint8_t btn) {
  static uint16_t last_encoder_count;
  static uint8_t encoder_p_status, encoder_n_status;
  uint16_t encoder_count;
  uint8_t ret;

  if (btn == EncoderP || btn == EncoderN) {
    encoder_count = timer_counter_read(TIMER3);

    if (last_encoder_count == 0) last_encoder_count = encoder_count;

    if (btn == EncoderP)
      ret = encoder_count > last_encoder_count ? 0 : 1;
    else
      ret = encoder_count < last_encoder_count ? 0 : 1;

    last_encoder_count = encoder_count;
  } else
    ret = gpio_input_bit_get(btn_port_table[btn], btn_pin_table[btn]) == RESET
              ? 0
              : 1;

  return ret;
}

static void multi_button_port_handler(void *param) {
  for (;;) {
    button_ticks();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void multi_button_port_init() {
  btn_handler_task_handle = xTaskCreateStatic(
      multi_button_port_handler, "btnHandler",
      sizeof(btn_handler_task_stack) / 4, NULL, 12,
      (StackType_t *)btn_handler_task_stack, &btn_handler_task);
  configASSERT(btn_handler_task_handle);

  log_i("init complete");
}
