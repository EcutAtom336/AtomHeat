#include "encoder.h"
/***/
#define LOG_TAG "encoder"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"

static TaskHandle_t encoder_handler_task_handle;
static StaticTask_t encoder_handler_task;
static uint8_t encoder_handler_task_stack[2048];

static EncoderCb_t encoder_cb;

void encoder_handler_task_code() {
  uint16_t last_counter, counter;
  counter      = timer_counter_read(TIMER3);
  last_counter = counter;
  int8_t scale;

  for (;;) {
    counter = timer_counter_read(TIMER3);

    scale = (counter - last_counter) / 4;

    if (scale != 0) {
      encoder_cb != NULL ? encoder_cb((scale)) : (0);
      last_counter += scale * 4;
    }

    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void encoder_cb_add(EncoderCb_t cb) { encoder_cb = cb; }

void encoder_cb_del() { encoder_cb = NULL; }

void encoder_init() {
  encoder_handler_task_handle = xTaskCreateStatic(
      encoder_handler_task_code, "encoder",
      sizeof(encoder_handler_task_stack) / 4, NULL, 12,
      (StackType_t*)encoder_handler_task_stack, &encoder_handler_task);
  configASSERT(encoder_handler_task_handle);
  
  log_i("init complete");
}
