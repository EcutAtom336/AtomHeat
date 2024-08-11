#ifndef _HEATING_H_
#define _HEATING_H_

#include "heat_info.h"

/**
 * @brief 加热时任务初始化
 * 
 */
void heating_init();

/**
 * @brief 启动加热
 * 
 * @param p_heat_info 
 */
void heating_start(HeatInfo_t *p_heat_info);

/**
 * @brief 停止加热
 * 
 */
void heating_stop();

#endif