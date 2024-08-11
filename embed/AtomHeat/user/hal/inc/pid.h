#ifndef _PID_H_
#define _PID_H_

#include <math.h>

typedef struct {
  float_t kp, ki, kd;
  float_t integral, integral_max;
  float_t current, target;
  float_t last_err;
  float_t out, out_min, out_max;
} PidDirData_t;

typedef struct {
  float_t kp, ki, kd;
  float_t last_err1, last_err2;
  float_t target, current, dead_space;
  float_t inc_val;
} PidIncData_t;

/**
 * @brief 位置式 pid 运算
 *
 * @param pid_data pid 数据
 */
void pid_dir(PidDirData_t *pid_data);

/**
 * @brief 增量式 pid 运算
 *
 * @param pin_data pid 数据
 */
void pid_inc(PidIncData_t *pin_data);

#endif