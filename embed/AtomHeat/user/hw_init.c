#include "hw_init.h"
/***/
#include "pwr_manage.h"
/***/
#define LOG_TAG "hw init"
#include "elog.h"
//
#include "lsm6ds3tr-c_reg.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"

/* IO distribute
 *
 * GPIO
 *
 * PB2      LED
 *
 * PB4      BTN5
 * PB5      BTN4
 * PB8      ENCODER_BTN
 * PD5      BTN1
 * PD6      BTN2
 * PD7      BTN3
 *
 * PB10     LSM6DS3TR_CS
 * PD9      LSM6DS3TR_INT1
 * PE15     LSM6DS3TR_INT2
 *
 * PC8      12V_EN
 * PE3      24V_PGOOD
 * PE5      24V_EN
 * PE9      3.3V_3_EN
 *
 * PD2      LCD_CS
 * PE2      LCD_BL
 * PE4      LCD_RESET
 * PE6      LCD_DC
 *
 *****
 *
 * ADC
 * PB1      ADC1    PWR_IN_VOL
 * PC0      ADC10   HEAT_CUR_SIG
 * PC1      ADC11   THERMOCOUPLE_SIG
 * PC2      ADC12   HEAT_VOL_SIG
 * PC3      ADC13   THERMOCOUPLE_SIG_VREF
 *
 *****
 *
 * SPI
 * PB13     SPI1_SCK
 * PB14     SPI1_MISO
 * PB15     SPI1_MOSI
 * PC10     SPI2_SCK
 * PC12     SPI2_MOSI
 *
 *****
 *
 * TIMER
 * PA2      T8CH0     SYS_FAN_ADJUST
 * PA3      T8CH1     HEAT_PAD_FAN_ADJUST
 * PB6      T3CH0     ENCODER_B
 * PB7      T3CH1     ENCODER_A
 * PB9      T10CH0    HEAT_CTRL
 * PB11     T1CH3     BUZZER
 */

static void gpio_init() {
  // LED
  // 开发板上 led
  rcu_periph_clock_enable(RCU_GPIOB);

  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);

  // BTNx
  // TC223A 输出端为开漏输出，需要上拉
  // 编码器按钮需要上拉
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOD);

  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8);
  gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

  // LSM6DS3TR_C
  rcu_periph_clock_enable(RCU_GPIOB);
  // rcu_periph_clock_enable(RCU_GPIOD);
  // rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_10);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_10);
  gpio_bit_set(GPIOB, GPIO_PIN_10);

  // LSM6DS3TR_INT1 未使用
  //  gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_9);

  // LSM6DS3TR_INT2 未使用
  // gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);

  // 12V_EN
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_8);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_8);

  // 24V_EN
  rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_5);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);

  // 24V_PGOOD 未使用
  // rcu_periph_clock_enable(RCU_GPIOE);

  // gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
  // gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
  // GPIO_PIN_3);

  // LCD control
  rcu_periph_clock_enable(RCU_GPIOD);
  rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);

  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_6);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
                          GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_6);

  // 3.3V_3_EN
  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
}

static void adc_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);
  rcu_periph_clock_enable(RCU_ADC0);
  rcu_periph_clock_enable(RCU_DMA1);
  //  io
  gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
  gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE,
                GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  //
  adc_resolution_config(ADC0, ADC_RESOLUTION_12B);  // 设置 adc 精度
  adc_data_alignment_config(ADC0,
                            ADC_DATAALIGN_RIGHT);  // 设置 adc 数据对齐方式

  // 配置规则组长度
  adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 5);

  // 配置规则组
  adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_1, ADC_SAMPLETIME_480);
  adc_routine_channel_config(ADC0, 1, ADC_CHANNEL_10, ADC_SAMPLETIME_480);
  adc_routine_channel_config(ADC0, 2, ADC_CHANNEL_11, ADC_SAMPLETIME_480);
  adc_routine_channel_config(ADC0, 3, ADC_CHANNEL_12, ADC_SAMPLETIME_480);
  adc_routine_channel_config(ADC0, 4, ADC_CHANNEL_13, ADC_SAMPLETIME_480);

  // adc 模式
  adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);

  adc_end_of_conversion_config(ADC0, ADC_EOC_SET_SEQUENCE);

  adc_enable(ADC0);
  vTaskDelay(pdMS_TO_TICKS(1));
  adc_calibration_enable(ADC0);
  // adc dma config
  adc_dma_mode_enable(ADC0);

  nvic_irq_enable(ADC_IRQn, 4, 0);
}

static void my_spi_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);
  // rcu_periph_clock_enable(RCU_SPI1);
  rcu_periph_clock_enable(RCU_SPI2);
  rcu_periph_clock_enable(RCU_DMA0);
  // io
  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                GPIO_PIN_13 | GPIO_PIN_15);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_13 | GPIO_PIN_15);
  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_14);

  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10 | GPIO_PIN_12);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_10 | GPIO_PIN_12);
  gpio_af_set(GPIOC, GPIO_AF_6, GPIO_PIN_10 | GPIO_PIN_12);
  // SPI2
  spi_parameter_struct spi_struct = {
      .device_mode          = SPI_MASTER,
      .trans_mode           = SPI_TRANSMODE_BDTRANSMIT,
      .frame_size           = SPI_FRAMESIZE_8BIT,
      .nss                  = SPI_NSS_SOFT,
      .endian               = SPI_ENDIAN_MSB,
      .clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE,
      .prescale             = SPI_PSC_2,
  };
  spi_init(SPI2, &spi_struct);
  spi_enable(SPI2);

  // SPI1 未使用
  //  spi_parameter_struct spi_struct1 = {
  //      .device_mode          = SPI_MASTER,
  //      .trans_mode           = SPI_TRANSMODE_BDTRANSMIT,
  //      .frame_size           = SPI_FRAMESIZE_8BIT,
  //      .nss                  = SPI_NSS_SOFT,
  //      .endian               = SPI_ENDIAN_MSB,
  //      .clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE,
  //      .prescale             = SPI_PSC_256,
  //  };
  //  spi_init(SPI1, &spi_struct1);
  //  spi_enable(SPI1);

  // dma
  spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);

  nvic_irq_enable(DMA0_Channel5_IRQn, 4, 0);

  // 128 * 160 * 2 = 40960 Byte 一帧
  //  10 Mhz SPI
  //  24 fps/s = 41.67 ms/fps
  //  40960 * 8 / 10000000 = 0.032768 s = 32.768 ms
}

static void my_timer_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_TIMER1);
  rcu_periph_clock_enable(RCU_TIMER3);
  rcu_periph_clock_enable(RCU_TIMER8);
  rcu_periph_clock_enable(RCU_TIMER10);
  rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
  // io
  // timer8
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN,
                GPIO_PIN_2 | GPIO_PIN_3);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
                          GPIO_PIN_2 | GPIO_PIN_3);
  gpio_af_set(GPIOA, GPIO_AF_3, GPIO_PIN_2 | GPIO_PIN_3);
  // timer3
  // 用于编码器，使能上拉
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
  gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_6 | GPIO_PIN_7);
  // timer10
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_9);
  gpio_af_set(GPIOB, GPIO_AF_3, GPIO_PIN_9);
  // timer 1
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO_PIN_11);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_11);
  gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_11);

  //
  timer_parameter_struct timer_init_param;
  timer_oc_parameter_struct timer_oc_init_param;
  timer_ic_parameter_struct icpara;

  // timet3 for encoder
  timer_init_param.prescaler         = 0;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_DOWN;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV1;
  timer_init_param.period            = 65535U;
  timer_init_param.repetitioncounter = 0;
  timer_init(TIMER3, &timer_init_param);
  icpara.icpolarity  = TIMER_IC_POLARITY_RISING;
  icpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
  icpara.icprescaler = TIMER_IC_PSC_DIV1;
  icpara.icfilter    = 3;
  timer_input_capture_config(TIMER3, TIMER_CH_0, &icpara);
  timer_quadrature_decoder_mode_config(TIMER3, TIMER_QUAD_DECODER_MODE2,
                                       TIMER_IC_POLARITY_FALLING,
                                       TIMER_IC_POLARITY_FALLING);
  timer_auto_reload_shadow_enable(TIMER3);
  timer_counter_value_config(TIMER3, 0x7FFF);
  timer_enable(TIMER3);

  // timer8 for fan
  // 100 hz
  timer_init_param.prescaler         = SystemCoreClock / 1U / 1000000U - 1U;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_UP;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV4;
  timer_init_param.period            = 1000000U / 100U - 1U;
  timer_init_param.repetitioncounter = 0;
  timer_init(TIMER8, &timer_init_param);
  timer_oc_init_param.outputstate  = TIMER_CCX_ENABLE;
  timer_oc_init_param.outputnstate = TIMER_CCXN_DISABLE;
  timer_oc_init_param.ocpolarity   = TIMER_OC_POLARITY_HIGH;
  timer_oc_init_param.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
  timer_oc_init_param.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
  timer_oc_init_param.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
  timer_channel_output_config(TIMER8, TIMER_CH_0, &timer_oc_init_param);
  timer_channel_output_config(TIMER8, TIMER_CH_1, &timer_oc_init_param);
  timer_channel_output_mode_config(TIMER8, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_mode_config(TIMER8, TIMER_CH_1, TIMER_OC_MODE_PWM0);
  timer_channel_output_pulse_value_config(TIMER8, TIMER_CH_0, 0);
  timer_channel_output_pulse_value_config(TIMER8, TIMER_CH_1, 0);
  timer_enable(TIMER8);

  // timer10 for heat ctrl
  // 500 hz
  timer_init_param.prescaler         = SystemCoreClock / 1U / 1000000U - 1U;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_UP;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV4;
  timer_init_param.period            = 1000000U / 500U - 1U;
  timer_init_param.repetitioncounter = 0U;
  timer_init(TIMER10, &timer_init_param);
  timer_oc_init_param.outputstate  = TIMER_CCX_ENABLE;
  timer_oc_init_param.outputnstate = TIMER_CCXN_DISABLE;
  timer_oc_init_param.ocpolarity   = TIMER_OC_POLARITY_HIGH;
  timer_oc_init_param.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
  timer_oc_init_param.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH;
  timer_oc_init_param.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;
  timer_channel_output_config(TIMER10, TIMER_CH_0, &timer_oc_init_param);
  timer_channel_output_mode_config(TIMER10, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_pulse_value_config(TIMER10, TIMER_CH_0, 0);
  timer_enable(TIMER10);

  // timer1 for buzzer
  // 频率不固定，占空比固定50
  timer_init_param.prescaler         = SystemCoreClock / 1U / 1000000U - 1U;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_UP;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV4;
  timer_init_param.period            = 1000000U / 4000U - 1U;
  timer_init_param.repetitioncounter = 0;
  timer_init(TIMER1, &timer_init_param);
  timer_oc_init_param.outputstate  = TIMER_CCX_ENABLE;
  timer_oc_init_param.outputnstate = TIMER_CCXN_DISABLE;
  timer_oc_init_param.ocpolarity   = TIMER_OC_POLARITY_HIGH;
  timer_oc_init_param.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
  timer_oc_init_param.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
  timer_oc_init_param.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
  timer_channel_output_config(TIMER1, TIMER_CH_3, &timer_oc_init_param);
  timer_channel_output_mode_config(TIMER1, TIMER_CH_3, TIMER_OC_MODE_PWM0);
  timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_3, 0);
  timer_enable(TIMER1);
}

void on_chip_hw_init() {
  gpio_init();
  adc_init();
  my_timer_init();
  my_spi_init();
  log_i("on chip hw init complete");
}

static void lsm6ds3tr_c_init() {
  lsm6ds3tr_c_reset_set(NULL, PROPERTY_ENABLE);
  uint8_t rst;
  // do { lsm6ds3tr_c_reset_get(NULL, &rst); } while (rst);

  vTaskDelay(pdMS_TO_TICKS(50));

  uint8_t id;
  lsm6ds3tr_c_device_id_get(NULL, &id);
  log_i("lsm6ds3tr-c id: %u", id);

  lsm6ds3tr_c_xl_full_scale_set(NULL, LSM6DS3TR_C_2g);

  // lsm6ds3tr_c_fifo_mode_set(NULL, LSM6DS3TR_C_STREAM_MODE);
  // lsm6ds3tr_c_fifo_watermark_set(NULL, 4);

  // lsm6ds3tr_c_fifo_data_rate_set(NULL, LSM6DS3TR_C_FIFO_12Hz5);

  lsm6ds3tr_c_xl_data_rate_set(NULL, LSM6DS3TR_C_XL_ODR_52Hz);
  // lsm6ds3tr_c_fifo_xl_batch_set(NULL, LSM6DS3TR_C_FIFO_XL_DEC_2);

  lsm6ds3tr_c_block_data_update_set(NULL, 1);
}

void on_board_hw_init() {
  pwr_manage_3v3_3_disable();
  vTaskDelay(pdMS_TO_TICKS(30));
  pwr_manage_3v3_3_enable();
  vTaskDelay(pdMS_TO_TICKS(30));

  lsm6ds3tr_c_init();
  log_i("on board hw init complete");
}
