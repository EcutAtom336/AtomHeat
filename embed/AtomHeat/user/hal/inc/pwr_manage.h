#ifndef _PWR_MANAGE_H_
#define _PWR_MANAGE_H_

#include <stdint.h>

/**
 * @brief 使能 3.3v_3 电源（用于控制加速度计）
 * 
 */
void pwr_manage_3v3_3_enable();

/**
 * @brief 失能 3.3v_3 电源（用于控制加速度计）
 * 
 */
void pwr_manage_3v3_3_disable();

/**
 * @brief 使能 12v 电源
 * 
 */
void pwr_manage_12v_enable();

/**
 * @brief 失能 12v 电源
 * 
 */
void pwr_manage_12v_disable();

/**
 * @brief 使能 24v 电源（设计之初电压为24中，实际电源不是24v）
 * 
 */
void pwr_manage_24v_enable();

/**
 * @brief 失能 24v 电源（设计之初电压为24中，实际电源不是24v）
 * 
 */
void pwr_manage_24v_disable();

/**
 * @brief 设置系统散热风扇 pwm 占空比
 * 
 * @param duty 
 */
void pwr_manage_sys_fan_duty_set(uint8_t duty);

/**
 * @brief 设置加热板散热风扇 pwm 占空比
 * 
 * @param duty 
 */
void pwr_manage_heat_pad_fan_duty_set(uint8_t duty);

#endif