# MINI 加热台 - AtomHeat 软件部分

## 子模块（位于 hal）

- buzzer

对应用提供预设的 buzzer 音频。

向应用层提供以下方法和类型。

```c
typedef enum {
  BuzzerToneSuccess,
  BuzzerToneFail,
  BuzzerToneWarning,
  BuzzerToneClick,
  BuzzerTone4khz500ms,
  BuzzerToneMax
} BuzzerToneNum_t;

/**
 * @brief 蜂鸣器相关初始化
 *
 */
void buzzer_init();

/**
 * @brief 蜂鸣器发声
 *
 * @param tone_num 音频编号
 */
void buzzer_sing(BuzzerToneNum_t tone_num);
```

- encoder

管理编码器相关回调。

向应用层提供以下方法和类型：

```c
typedef void (*EncoderCb_t)(int8_t change_val);

/**
 * @brief 编码器相关初始化
 *
 */
void encoder_init();

/**
 * @brief 添加编码器处理回调（仅支持一个回调）
 *
 * @param cb
 */
void encoder_cb_add(EncoderCb_t cb);

/**
 * @brief 删除编码器处理回调
 *
 */
void encoder_cb_del();
```

- heat_info

加热参数类型和加热参数修正。

向应用层提供以下方法和类型：

```c
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
```

- heat_pad

加热板控制相关。

向应用层提供以下方法：

```c
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
```

- measure

使用 adc 测量模拟量和管理对应数据。

对应用层提供以下方法和类型：

```c
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
```

- pid

pid 算法相关。

对应用层提供以下方法和类型：

```c
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
```

- pwr_manage

电源管理相关。

对应用层提供以下方法：

```c
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
```

## 应用模块（位于 app）

- set_heat_info_page

用于设置加热参数，启动加热流程。

提供以下方法：

```c
/**
 * @brief 初始化设置加热参数界面
 * 
 */
void set_heat_info_page_init();

/**
 * @brief 启动设置加热参数界面
 * 
 */
void set_heat_info_page_start();

/**
 * @brief 停止设置加热参数界面
 * 
 */
void set_heat_info_page_stop();
```

- heating_page

显示功率、温度和温度曲线，倾倒检测。

提供以下方法：

```c
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
```

- remind_page

提示界面。由 heating 界面启动，用于提示用户。

提供以下类型和方法：

```c
typedef enum {
  RemindTypeNone,
  RemindTypeVl,
  RemindTypeSlant,
  RemindTypeFinish,
  RemindTypeMax
} RemindType_t;

/**
 * @brief 提示页相关初始化
 *
 */
void remind_page_init();

/**
 * @brief 启动提示页
 *
 * @param remind 提示信息
 */
void remind_page_start(RemindType_t remind);

/**
 * @brief 停止提示页
 *
 */
void remind_page_stop();
```

## 其他软件

- 响应式按键管理库 [Multi_button](https://github.com/0x1abin/MultiButton)

- 轻量级 flash 管理库 [EasyFlash](https://github.com/armink/EasyFlash.git)

- 轻量级 log 库 [EasyLogger](https://github.com/armink/EasyLogger.git)

- st lsm6ds3tr-c 驱动库 [lsm6ds3tr-c-pid](https://github.com/STMicroelectronics/lsm6ds3tr-c-pid.git)

- 轻量级 rtos [Freertos](https://github.com/FreeRTOS/FreeRTOS.git)
