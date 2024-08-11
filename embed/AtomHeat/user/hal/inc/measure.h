#ifndef _MEASURE_H_
#define _MEASURE_H_

#include <math.h>

typedef struct {
  float_t pwr_in_vol;
  float_t heat_pad_temp;
  float_t heat_pad_pwr;
  float_t heat_pad_vol;
  float_t heat_pad_cur;
} MeasurePwrInfo_t;

typedef struct {
  float_t acc_x_mg, acc_y_mg, acc_z_mg, acc_mg_all;
  float_t deg_x_mg, deg_y_mg, deg_z_mg, deg_mg_all;
} MeasureSportInfo_t;

void measure_init();

/**
 * @brief 获取功率数据
 *
 * @param out 输出指针
 */
void measure_pwr_data_get(MeasurePwrInfo_t *out);

/**
 * @brief 获取运动数据
 *
 * @param out 输出指针
 */
void measure_sport_data_get(MeasureSportInfo_t *out);

#endif