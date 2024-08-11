#include "logo_page.h"
/***/
#include "my_lcd.h"
#include "pic.h"
#include "set_heat_info_page.h"
/***/
#include "FreeRTOS.h"
#include "timers.h"
/***/

static TimerHandle_t logo_page_timer_handle;
static StaticTimer_t logo_page_timer;

void logo_page_start() {
  lcd_show_pic(0, 0, LOGO_IMG_HEIGHT, LOGO_IMG_WIDTH, LOGO_IMG);

  xTimerStart(logo_page_timer_handle, portMAX_DELAY);
}

static void timer_handler() { set_heat_info_page_start(); }

void logo_page_init() {
  logo_page_timer_handle =
      xTimerCreateStatic("logoPage", pdMS_TO_TICKS(1500), pdFALSE, 0,
                         timer_handler, &logo_page_timer);
  configASSERT(logo_page_timer_handle);
}
