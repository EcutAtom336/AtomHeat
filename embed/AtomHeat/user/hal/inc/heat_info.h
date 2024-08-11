#ifndef _HEAT_INFO_H_
#define _HEAT_INFO_H_

#include <stdint.h>

typedef struct {
  uint32_t duartion_s;
  uint16_t temp;
} HeatStage_t;

typedef struct {
  uint8_t is_constant : 1;
  uint8_t check_slant : 1;
  uint8_t power_limit;
  HeatStage_t stage[5];
} HeatInfo_t;

/**
 * @brief 检测并修正加热信息参数
 *
 * @param p_heat_info
 */
void heat_info_revise(HeatInfo_t* p_heat_info);

#endif
