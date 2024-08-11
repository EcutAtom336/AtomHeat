#include "set_heat_info_page.h"
/***/
#include "color.h"
#include "encoder.h"
#include "lcd_init.h"
#include "heating.h"
#include "my_lcd.h"
#include "heat_info.h"
#include "measure.h"
#include "buzzer.h"
#include "multi_button_port.h"
/***/
#define LOG_TAG "set page"
#include "elog.h"
//
#include "multi_button.h"
#include "easyflash.h"
//
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"
/***/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 设置加热流程界面
 * 此界面可设置加热流程、功率限制
 * 分段加热分为预热段、加热段、焊接端、保温段和冷却段五段
 * 长按按键可保存预设、短按按键可加载预设
 */

// static EventGroupHandle_t set_page_evtgp_handle;
// static StaticEventGroup_t set_page_evtgp;

static HeatInfo_t heat_info;

static uint8_t menu_num = 0;
static uint8_t select   = 0;

static struct Button btn1;
static struct Button btn2;
static struct Button btn3;
static struct Button btn4;
static struct Button btn5;
static struct Button encoder_btn;

// 从加热界面退出会在加热界面，用这个定时器刷新一下
static TaskHandle_t refreash_task_handle;
static StaticTask_t refreash_task;
static uint8_t refreash_task_stack[1024];

static TimerHandle_t auto_close_screen_timer_handle;
static StaticTimer_t auto_close_screen_timer;

static void set_heat_procedure_page_pause();
static void set_heat_procedure_page_resume();

const static uint16_t constant_select_indicator_pos_cn[4][2] = {
    {68, 0},
    {140, 0},   //
    {100, 14},  //
    {85, 27},   //
};

const static uint16_t stage_select_indicator_pos_cn[13][2] = {
    {68, 0},   {140, 0},    //
    {100, 14},              //
    {86, 51},  {134, 51},   //
    {86, 67},  {134, 67},   //
    {86, 83},  {134, 83},   //
    {86, 99},  {134, 99},   //
    {86, 115}, {134, 115},  //
};

const static uint16_t constant_select_indicator_pos_en[4][2] = {
    {70, 3},
    {135, 3},   //
    {100, 14},  //
    {85, 27},   //
};

const static uint16_t stage_select_indicator_pos_en[13][2] = {
    {70, 3},   {135, 3},    //
    {100, 19},              //
    {80, 51},  {135, 51},   //
    {80, 67},  {135, 67},   //
    {80, 83},  {135, 83},   //
    {80, 99},  {135, 99},   //
    {80, 115}, {135, 115},  //
};

static void set_page_ui_renew_cn() {
  lcd_fill(0, 0, LCD_W, LCD_H, heat_info.check_slant ? BLACK : RED);

  lcd_show_font(
      0, 0,
      heat_info.is_constant ? (uint8_t *)"模式：恒温" : (uint8_t *)"模式：分段",
      WHITE, WHITE, FontSize12);

  lcd_show_font(
      84, 0, heat_info.check_slant ? (uint8_t *)"开始" : (uint8_t *)"强制开始",
      WHITE, WHITE, FontSize12);

  lcd_show_font(0, 14, (uint8_t *)"功率限制：", WHITE, WHITE, FontSize12);
  lcd_show_num(61, 14, heat_info.power_limit, 3, WHITE, WHITE, 12);
  lcd_show_str(80, 14, (uint8_t *)"W", WHITE, WHITE, 12);
  if (heat_info.is_constant) {
    lcd_show_font(0, 27, (uint8_t *)"温度：", WHITE, WHITE, FontSize12);
    lcd_show_num(37, 27, heat_info.stage[0].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(58, 27, (uint8_t *)"℃", WHITE, WHITE, FontSize12);
  } else {
    const uint16_t TEMP_POS_X = 50, DURATION_S_POS_X = 95;

    lcd_show_font(TEMP_POS_X, 35, (uint8_t *)"温度", WHITE, WHITE, FontSize12);
    lcd_show_font(DURATION_S_POS_X, 35, (uint8_t *)"时间", WHITE, WHITE,
                  FontSize12);

    lcd_show_font(0, 51, (uint8_t *)"预热段", WHITE, WHITE, FontSize12);
    lcd_show_num(TEMP_POS_X, 51, heat_info.stage[0].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(TEMP_POS_X + 20, 51, (uint8_t *)"℃", WHITE, WHITE,
                  FontSize12);
    lcd_show_num(DURATION_S_POS_X, 51, heat_info.stage[0].duartion_s, 4, WHITE,
                 WHITE, 12);
    lcd_show_str(DURATION_S_POS_X + 25, 51, (uint8_t *)"s", WHITE, WHITE, 12);

    lcd_show_font(0, 67, (uint8_t *)"加热段", WHITE, WHITE, FontSize12);
    lcd_show_num(TEMP_POS_X, 67, heat_info.stage[1].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(TEMP_POS_X + 20, 67, (uint8_t *)"℃", WHITE, WHITE,
                  FontSize12);
    lcd_show_num(DURATION_S_POS_X, 67, heat_info.stage[1].duartion_s, 4, WHITE,
                 WHITE, 12);
    lcd_show_str(DURATION_S_POS_X + 25, 67, (uint8_t *)"s", WHITE, WHITE, 12);

    lcd_show_font(0, 83, (uint8_t *)"焊接段", WHITE, WHITE, FontSize12);
    lcd_show_num(TEMP_POS_X, 83, heat_info.stage[2].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(TEMP_POS_X + 20, 83, (uint8_t *)"℃", WHITE, WHITE,
                  FontSize12);
    lcd_show_num(DURATION_S_POS_X, 83, heat_info.stage[2].duartion_s, 4, WHITE,
                 WHITE, 12);
    lcd_show_str(DURATION_S_POS_X + 25, 83, (uint8_t *)"s", WHITE, WHITE, 12);

    lcd_show_font(0, 99, (uint8_t *)"保温段", WHITE, WHITE, FontSize12);
    lcd_show_num(TEMP_POS_X, 99, heat_info.stage[3].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(TEMP_POS_X + 20, 99, (uint8_t *)"℃", WHITE, WHITE,
                  FontSize12);
    lcd_show_num(DURATION_S_POS_X, 99, heat_info.stage[3].duartion_s, 4, WHITE,
                 WHITE, 12);
    lcd_show_str(DURATION_S_POS_X + 25, 99, (uint8_t *)"s", WHITE, WHITE, 12);

    lcd_show_font(0, 115, (uint8_t *)"冷却段", WHITE, WHITE, FontSize12);
    lcd_show_num(TEMP_POS_X, 115, heat_info.stage[4].temp, 3, WHITE, WHITE, 12);
    lcd_show_font(TEMP_POS_X + 20, 115, (uint8_t *)"℃", WHITE, WHITE,
                  FontSize12);
    lcd_show_num(DURATION_S_POS_X, 115, heat_info.stage[4].duartion_s, 4, WHITE,
                 WHITE, 12);
    lcd_show_str(DURATION_S_POS_X + 25, 115, (uint8_t *)"s", WHITE, WHITE, 12);
  }
  // 显示光标
  lcd_show_str(
      heat_info.is_constant ? constant_select_indicator_pos_cn[menu_num][0]
                            : stage_select_indicator_pos_cn[menu_num][0],
      heat_info.is_constant ? constant_select_indicator_pos_cn[menu_num][1]
                            : stage_select_indicator_pos_cn[menu_num][1],
      (uint8_t *)(select ? "<<" : "< "), WHITE, WHITE, 12);

  lcd_refreash_request();
}

static void set_page_ui_renew_en() {
  lcd_fill(0, 0, LCD_W, LCD_H, BLACK);

  lcd_show_str(0, 3, (uint8_t *)"mdoe:", WHITE, WHITE, 12);

  lcd_show_str(100, 3, (uint8_t *)"start", WHITE, WHITE, 12);

  uint8_t pwr_limit_str_buf[17];
  snprintf((char *)pwr_limit_str_buf, sizeof(pwr_limit_str_buf),
           "power limit: %u", heat_info.power_limit);
  lcd_show_str(0, 19, pwr_limit_str_buf, WHITE, WHITE, 12);

  if (heat_info.is_constant) {
    uint8_t const_temp_str_buf[20];
    snprintf((char *)const_temp_str_buf, sizeof(const_temp_str_buf),
             "temp(C):%u", heat_info.stage[0].temp);
    lcd_show_str(0, 35, const_temp_str_buf, WHITE, WHITE, 12);
  } else {
    // lcd_show_str(55, 35, (uint8_t *)"temp(C)", WHITE, WHITE, 12, 1);
    // lcd_show_str(105, 35, (uint8_t *)"time(s)", WHITE, WHITE, 12, 1);

    lcd_show_str(0, 51, (uint8_t *)"stage1", WHITE, WHITE, 12);
    lcd_show_num(45, 51, heat_info.stage[0].temp, 3, WHITE, BLACK, 12);
    lcd_show_num(105, 51, heat_info.stage[0].duartion_s, 4, WHITE, BLACK, 12);

    lcd_show_str(0, 67, (uint8_t *)"stage2", WHITE, WHITE, 12);
    lcd_show_num(45, 67, heat_info.stage[1].temp, 3, WHITE, BLACK, 12);
    lcd_show_num(105, 67, heat_info.stage[1].duartion_s, 4, WHITE, BLACK, 12);

    lcd_show_str(0, 83, (uint8_t *)"stage3", WHITE, WHITE, 12);
    lcd_show_num(45, 83, heat_info.stage[2].temp, 3, WHITE, BLACK, 12);
    lcd_show_num(105, 83, heat_info.stage[2].duartion_s, 4, WHITE, BLACK, 12);

    lcd_show_str(0, 99, (uint8_t *)"stage4", WHITE, WHITE, 12);
    lcd_show_num(45, 99, heat_info.stage[3].temp, 3, WHITE, BLACK, 12);
    lcd_show_num(105, 99, heat_info.stage[3].duartion_s, 4, WHITE, BLACK, 12);

    lcd_show_str(0, 115, (uint8_t *)"stage5", WHITE, WHITE, 12);
    lcd_show_num(45, 115, heat_info.stage[4].temp, 3, WHITE, BLACK, 12);
    lcd_show_num(105, 115, heat_info.stage[4].duartion_s, 4, WHITE, BLACK, 12);
  }
  // 显示光标
  lcd_show_str(
      heat_info.is_constant ? constant_select_indicator_pos_en[menu_num][0]
                            : stage_select_indicator_pos_en[menu_num][0],
      heat_info.is_constant ? constant_select_indicator_pos_en[menu_num][1]
                            : stage_select_indicator_pos_en[menu_num][1],
      (uint8_t *)(select ? "<<" : "< "), WHITE, WHITE, 12);

  lcd_refreash_request();
}

static void btn_handler(void *param) {
  xTimerReset(auto_close_screen_timer_handle, portMAX_DELAY);

  struct Button *btn = (struct Button *)param;
  PressEvent evt     = get_button_event(btn);

  size_t saved_len, read_len;
  EfErrCode ef_err;

  switch (btn->button_id) {
    case BtnNum1: {
      if (evt == PRESS_UP) {
        heat_info.check_slant = 1;

        buzzer_sing(BuzzerToneClick);

        read_len = ef_get_env_blob("heatInfo1", &heat_info, sizeof(heat_info),
                                   &saved_len);
      } else if (evt == LONG_PRESS_START) {
        buzzer_sing(BuzzerToneClick);

        ef_err = ef_set_env_blob("heatInfo1", &heat_info, sizeof(HeatInfo_t));
      }
      break;
    }
    case BtnNum2: {
      heat_info.check_slant = 1;

      if (evt == PRESS_UP) {
        buzzer_sing(BuzzerToneClick);

        read_len = ef_get_env_blob("heatInfo2", &heat_info, sizeof(heat_info),
                                   &saved_len);
      } else if (evt == LONG_PRESS_START) {
        buzzer_sing(BuzzerToneClick);

        ef_err = ef_set_env_blob("heatInfo2", &heat_info, sizeof(HeatInfo_t));
      }
      break;
    }
    case BtnNum3: {
      heat_info.check_slant = 1;

      if (evt == PRESS_UP) {
        buzzer_sing(BuzzerToneClick);

        read_len = ef_get_env_blob("heatInfo3", &heat_info, sizeof(heat_info),
                                   &saved_len);
      } else if (evt == LONG_PRESS_START) {
        buzzer_sing(BuzzerToneClick);

        ef_err = ef_set_env_blob("heatInfo3", &heat_info, sizeof(HeatInfo_t));
      }
      break;
    }
    case BtnNum4: {
      heat_info.check_slant = 1;

      if (evt == PRESS_UP) {
        buzzer_sing(BuzzerToneClick);

        read_len = ef_get_env_blob("heatInfo4", &heat_info, sizeof(heat_info),
                                   &saved_len);
      } else if (evt == LONG_PRESS_START) {
        buzzer_sing(BuzzerToneClick);

        ef_err = ef_set_env_blob("heatInfo4", &heat_info, sizeof(HeatInfo_t));
      }
      break;
    }
    case BtnNum5: {
      heat_info.check_slant = 1;

      if (evt == PRESS_UP) {
        buzzer_sing(BuzzerToneClick);

        read_len = ef_get_env_blob("heatInfo5", &heat_info, sizeof(heat_info),
                                   &saved_len);
      } else if (evt == LONG_PRESS_START) {
        buzzer_sing(BuzzerToneClick);

        ef_err = ef_set_env_blob("heatInfo5", &heat_info, sizeof(HeatInfo_t));
      }
      break;
    }
    case EncoderBtn: {
      if (evt == PRESS_DOWN) {
        buzzer_sing(BuzzerToneClick);
        if (menu_num == 1) {
          // 选中开始
          MeasurePwrInfo_t pwr_info;
          MeasureSportInfo_t sport_info;

          measure_pwr_data_get(&pwr_info);
          measure_sport_data_get(&sport_info);

          // 检测电压和机身状态
          if (pwr_info.pwr_in_vol < 16.5) {
            log_i("start heat fail, reason: voltage too low, voltage: %.2f",
                  pwr_info.pwr_in_vol);
            buzzer_sing(BuzzerToneFail);
            break;
          }

          if (heat_info.check_slant &&
              (sport_info.acc_mg_all < 950 || sport_info.acc_mg_all > 1050 ||
               sport_info.acc_z_mg < 950 || sport_info.acc_z_mg > 1050)) {
            log_i(
                "start heat fail, reason: machine tilt, acc all: %.2f mg, acc "
                "z: %.2f mg",
                sport_info.acc_mg_all, sport_info.acc_z_mg);
            buzzer_sing(BuzzerToneFail);
            heat_info.check_slant = 0;
            break;
          }

          menu_num = 0;
          buzzer_sing(BuzzerToneSuccess);
          set_heat_info_page_stop();
          heating_start(&heat_info);
        } else
          // 未选中开始
          select = !select;
      }
      break;
    }
    default:
      break;
  }

  if (heat_info.is_constant && menu_num > 3) menu_num = 0;

  xTaskNotify(refreash_task_handle, 1U << 2, eSetBits);
}

static void btn_weak_up_handler() { set_heat_procedure_page_resume(); }

static void encoder_handler(int8_t change_val) {
  static int16_t v;
  v += change_val;

  xTimerReset(auto_close_screen_timer_handle, portMAX_DELAY);

  heat_info.check_slant = 1;

  if (select) {
    // 已选中菜单项
    switch (menu_num) {
      case 0:
        heat_info.is_constant = !heat_info.is_constant;
        break;
      case 1:
        break;
      case 2:
        heat_info.power_limit += 5 * change_val;
        break;
      case 3:
        heat_info.stage[0].temp += 10 * change_val;
        break;
      case 4:
        heat_info.stage[0].duartion_s +=
            (abs(change_val) > 2 ? 50 : 5) * change_val;
        break;
      case 5:
        heat_info.stage[1].temp += 10 * change_val;
        break;
      case 6:
        heat_info.stage[1].duartion_s +=
            (abs(change_val) > 2 ? 50 : 5) * change_val;
        break;
      case 7:
        heat_info.stage[2].temp += 10 * change_val;
        break;
      case 8:
        heat_info.stage[2].duartion_s +=
            (abs(change_val) > 2 ? 50 : 5) * change_val;
        break;
      case 9:
        heat_info.stage[3].temp += 10 * change_val;
        break;
      case 10:
        heat_info.stage[3].duartion_s +=
            (abs(change_val) > 2 ? 50 : 5) * change_val;
        break;
      case 11:
        heat_info.stage[4].temp += 10 * change_val;
        break;
      case 12:
        heat_info.stage[4].duartion_s +=
            (abs(change_val) > 2 ? 50 : 5) * change_val;
        break;
      default:
        break;
    }
    heat_info_revise(&heat_info);
    if (heat_info.stage[0].duartion_s < 5) heat_info.stage[0].duartion_s = 5;
  } else {
    change_val > 0 ? menu_num++ : menu_num--;  // 未选中菜单项，菜单项递增或递减
    // 菜单项溢出判断
    if ((!heat_info.is_constant && menu_num > 12) ||
        (heat_info.is_constant && menu_num > 3))
      menu_num = 0;
  }

  xTaskNotify(refreash_task_handle, 1U << 2, eSetBits);
}

static void encoder_weak_up_handler(int8_t change_val) {
  set_heat_procedure_page_resume();
}

static void set_heat_procedure_page_resume() {
  button_attach(&btn1, PRESS_UP, btn_handler);
  button_attach(&btn2, PRESS_UP, btn_handler);
  button_attach(&btn3, PRESS_UP, btn_handler);
  button_attach(&btn4, PRESS_UP, btn_handler);
  button_attach(&btn5, PRESS_UP, btn_handler);
  button_attach(&encoder_btn, PRESS_DOWN, btn_handler);
  button_attach(&btn1, LONG_PRESS_START, btn_handler);
  button_attach(&btn2, LONG_PRESS_START, btn_handler);
  button_attach(&btn3, LONG_PRESS_START, btn_handler);
  button_attach(&btn4, LONG_PRESS_START, btn_handler);
  button_attach(&btn5, LONG_PRESS_START, btn_handler);

  encoder_cb_add(encoder_handler);

  xTimerReset(auto_close_screen_timer_handle, portMAX_DELAY);

  LCD_BLK_Set();
}

static void set_heat_procedure_page_pause() {
  button_attach(&btn1, PRESS_DOWN, btn_weak_up_handler);
  button_attach(&btn2, PRESS_DOWN, btn_weak_up_handler);
  button_attach(&btn3, PRESS_DOWN, btn_weak_up_handler);
  button_attach(&btn4, PRESS_DOWN, btn_weak_up_handler);
  button_attach(&btn5, PRESS_DOWN, btn_weak_up_handler);
  button_attach(&encoder_btn, PRESS_DOWN, btn_weak_up_handler);

  encoder_cb_add(encoder_weak_up_handler);

  LCD_BLK_Clr();
}

static void timer_handler(TimerHandle_t t) { set_heat_procedure_page_pause(); }

void set_heat_info_page_start() {
  button_start(&btn1);
  button_start(&btn2);
  button_start(&btn3);
  button_start(&btn4);
  button_start(&btn5);
  button_start(&encoder_btn);

  xTimerStart(auto_close_screen_timer_handle, portMAX_DELAY);

  encoder_cb_add(encoder_handler);

  xTaskNotify(refreash_task_handle, 1U << 0, eSetBits);
}

void set_heat_info_page_stop() {
  button_stop(&btn1);
  button_stop(&btn2);
  button_stop(&btn3);
  button_stop(&btn4);
  button_stop(&btn5);
  button_stop(&encoder_btn);

  xTimerStop(auto_close_screen_timer_handle, portMAX_DELAY);

  xTaskNotify(refreash_task_handle, 1U << 1, eSetBits);

  encoder_cb_del();
}

static void refeash_task_code() {
  uint32_t notify_val;
  TickType_t wait_motify_tick   = portMAX_DELAY;
  BaseType_t wait_notify_result = pdFALSE;

  while (1) {
    wait_notify_result =
        xTaskNotifyWait(0, 0xFFFFFFFF, &notify_val, wait_motify_tick);
    if (wait_notify_result) {
      if (notify_val & (1U << 0)) {
        wait_motify_tick = pdMS_TO_TICKS(100);
      } else if (notify_val & (1U << 1)) {
        wait_motify_tick = portMAX_DELAY;
      } else if (notify_val & (1U << 2)) {
        set_page_ui_renew_cn();
      }
      continue;
    }
    set_page_ui_renew_cn();
  }
}

void set_heat_info_page_init() {
  button_init(&btn1, multi_button_port_read_btn, 0, BtnNum1);
  button_init(&btn2, multi_button_port_read_btn, 0, BtnNum2);
  button_init(&btn3, multi_button_port_read_btn, 0, BtnNum3);
  button_init(&btn4, multi_button_port_read_btn, 0, BtnNum4);
  button_init(&btn5, multi_button_port_read_btn, 0, BtnNum5);
  button_init(&encoder_btn, multi_button_port_read_btn, 0, EncoderBtn);

  button_attach(&btn1, PRESS_UP, btn_handler);
  button_attach(&btn2, PRESS_UP, btn_handler);
  button_attach(&btn3, PRESS_UP, btn_handler);
  button_attach(&btn4, PRESS_UP, btn_handler);
  button_attach(&btn5, PRESS_UP, btn_handler);
  button_attach(&encoder_btn, PRESS_DOWN, btn_handler);
  button_attach(&btn1, LONG_PRESS_START, btn_handler);
  button_attach(&btn2, LONG_PRESS_START, btn_handler);
  button_attach(&btn3, LONG_PRESS_START, btn_handler);
  button_attach(&btn4, LONG_PRESS_START, btn_handler);
  button_attach(&btn5, LONG_PRESS_START, btn_handler);

  size_t read_len, saved_len;

  read_len =
      ef_get_env_blob("heatInfo1", &heat_info, sizeof(heat_info), &saved_len);
  heat_info_revise(&heat_info);

  if (heat_info.stage[0].duartion_s < 5) heat_info.stage[0].duartion_s = 5;

  auto_close_screen_timer_handle = xTimerCreateStatic(
      "autoCloseScreen", pdMS_TO_TICKS(600000), pdFALSE, 0,
      set_heat_procedure_page_pause, &auto_close_screen_timer);
  configASSERT(auto_close_screen_timer_handle);

  refreash_task_handle = xTaskCreateStatic(
      refeash_task_code, "refreash", sizeof(refreash_task_stack) / 4, NULL, 12,
      (StackType_t *)(&refreash_task_stack), &refreash_task);
  configASSERT(refreash_task_handle);
}
