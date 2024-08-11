#include "pid.h"
/***/
#include <math.h>

void pid_dir(PidDirData_t *pid_data) {
  float_t err;

  // 计算误差
  err = pid_data->target - pid_data->current;

  // 积分
  pid_data->integral += err * pid_data->ki;
  // 积分限幅
  if (pid_data->integral > pid_data->integral_max)
    pid_data->integral = pid_data->integral_max;
  else if (pid_data->integral < -pid_data->integral_max)
    pid_data->integral = -pid_data->integral_max;

  // 计算输出
  pid_data->out = (err * pid_data->kp) + (pid_data->integral) +
                  ((err - pid_data->last_err) * pid_data->kd);
  // 输出限幅
  if (pid_data->out > pid_data->out_max)
    pid_data->out = pid_data->out_max;
  else if (pid_data->out < pid_data->out_min)
    pid_data->out = pid_data->out_min;

  // 保存误差
  pid_data->last_err = err;
}

void pid_inc(PidIncData_t *pid_data) {
  float_t err = pid_data->target - pid_data->current;

  if (fabsf(err) < pid_data->dead_space)
    pid_data->inc_val = 0;
  else
    pid_data->inc_val =
        (pid_data->kp * (err - pid_data->last_err1)) + (pid_data->ki * err) +
        (pid_data->kd * (err - 2 * pid_data->last_err1 + pid_data->last_err2));

  pid_data->last_err2 = pid_data->last_err1;
  pid_data->last_err1 = err;
}
