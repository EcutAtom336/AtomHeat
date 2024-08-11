#ifndef _HEAT_PAD_H_
#define _HEAT_PAD_H_

#include <stdint.h>

extern const uint16_t HEAT_PAD_MIN_TEMP, HEAT_PAD_MAX_TEMP;

/**
 * @brief 加热板相关初始化
 *
 */
void heat_pad_init();

/**
 * @brief 失能加热板
 *
 */
void heat_pad_disable();

/**
 * @brief 设置加热台最大功率，该操作会自动使能加热板
 * 
 * @param p 功率
 */
void heat_pad_set_max_pwr(uint8_t p);

/**
 * @brief 设置加热板温度，该操作会自动使能加热板
 * 
 * @param temp 温度
 */
void heat_pad_set_temp(uint16_t temp);

#endif