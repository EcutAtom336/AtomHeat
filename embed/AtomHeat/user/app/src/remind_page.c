#include "remind_page.h"
//
#include "buzzer.h"
#include "multi_button.h"
#include "multi_button_port.h"
#include "my_lcd.h"
#include "lcd_init.h"
#include "color.h"
#include "set_heat_info_page.h"
//
#define LOG_TAG "remind page"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

static TaskHandle_t remind_task_handle;
static uint8_t remind_task_stack[1024];
static StaticTask_t remind_task;

static struct Button encoder_btn;

static void remind_page_ui_renew(RemindType_t t) {
  lcd_fill(0, 0, LCD_W, LCD_H, BLACK);

  switch (t) {
    case RemindTypeVl: {
      lcd_show_font(0, 48, (uint8_t *)"电压过低", WHITE, WHITE, FontSize12);
      lcd_show_str(0, 60, (uint8_t *)"voltage too low.", WHITE, WHITE, 12);
      break;
    }
    case RemindTypeSlant: {
      lcd_show_font(0, 48, (uint8_t *)"机身倾斜", WHITE, WHITE, FontSize12);
      lcd_show_str(0, 60, (uint8_t *)"machine tilt.", WHITE, WHITE, 12);
      break;
    }
    case RemindTypeFinish: {
      lcd_show_font(0, 48, (uint8_t *)"加热结束", WHITE, WHITE, FontSize12);
      lcd_show_str(0, 60, (uint8_t *)"end of heating.", WHITE, WHITE, 12);
      break;
    }
  }

  lcd_refreash_request();
}

void remind_task_code() {
  RemindType_t r;

  TickType_t wait_notify_tick = portMAX_DELAY, tick;

  uint8_t finish_remind_count;

  while (1) {
    if (xTaskNotifyWait(0, 0, (uint32_t *)&r, wait_notify_tick) ==
        pdPASS) {
      switch (r) {
        case RemindTypeNone: {
          wait_notify_tick = portMAX_DELAY;
          break;
        }
        case RemindTypeSlant: {
          wait_notify_tick = 0;
          break;
        }
        case RemindTypeVl: {
          buzzer_sing(BuzzerToneWarning);

          wait_notify_tick = 0;
          break;
        }
        case RemindTypeFinish: {
          finish_remind_count = 0;
          break;
        }
        default:
          break;
      }
      continue;
    }

    switch (r) {
      case RemindTypeSlant:
      case RemindTypeVl: {
        wait_notify_tick = 0;

        buzzer_sing(BuzzerToneWarning);

        break;
      }
      case RemindTypeFinish: {
        if (finish_remind_count++ > 3) {
          wait_notify_tick = portMAX_DELAY;
          continue;
        }
        buzzer_sing(BuzzerTone4khz500ms);
        break;
      }
      default:
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void btn_handler(void *param) {
  struct Button *btn = (struct Button *)param;
  PressEvent evt     = get_button_event(btn);

  buzzer_sing(BuzzerToneClick);

  switch (btn->button_id) {
    case EncoderBtn: {
      if (evt == PRESS_DOWN) {
        remind_page_stop();
        set_heat_info_page_start();
      }
      break;
    }
    default:
      break;
  }
}

void remind_page_start(RemindType_t t) {
  xTaskNotify(remind_task_handle, t, eSetValueWithOverwrite);

  remind_page_ui_renew(t);

  button_start(&encoder_btn);
}

void remind_page_stop() {
  xTaskNotify(remind_task_handle, 0, eSetValueWithOverwrite);

  button_stop(&encoder_btn);
}

void remind_page_init() {
  button_init(&encoder_btn, multi_button_port_read_btn, 0, EncoderBtn);

  button_attach(&encoder_btn, PRESS_DOWN, btn_handler);

  remind_task_handle = xTaskCreateStatic(
      remind_task_code, "remind", sizeof(remind_task_stack) / 4, NULL, 12,
      (StackType_t *)remind_task_stack, &remind_task);
  configASSERT(remind_task_handle);
}
