#include "pwr_manage.h"
/***/
#define LOG_TAG "pwr manage"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"

void pwr_manage_3v3_3_enable() { gpio_bit_set(GPIOE, GPIO_PIN_9); }

void pwr_manage_3v3_3_disable() { gpio_bit_reset(GPIOE, GPIO_PIN_9); }

void pwr_manage_12v_enable() { gpio_bit_set(GPIOC, GPIO_PIN_8); }

void pwr_manage_12v_disable() { gpio_bit_reset(GPIOC, GPIO_PIN_8); }

void pwr_manage_24v_enable() { gpio_bit_set(GPIOE, GPIO_PIN_5); }

void pwr_manage_24v_disable() { gpio_bit_reset(GPIOE, GPIO_PIN_5); }

void pwr_manage_sys_fan_duty_set(uint8_t duty) {
  timer_channel_output_pulse_value_config(TIMER8, TIMER_CH_0,
                                          duty > 100 ? 10000 : duty * 100);
}

void pwr_manage_heat_pad_fan_duty_set(uint8_t duty) {
  timer_channel_output_pulse_value_config(TIMER8, TIMER_CH_1,
                                          duty > 100 ? 10000 : duty * 100);
}
