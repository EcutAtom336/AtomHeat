#include "lsm6ds3tr_port.h"
/***/
#include "lsm6ds3tr-c_reg.h"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"

stmdev_write_ptr st_w_ptr;
stmdev_read_ptr st_r_ptr;
stmdev_mdelay_ptr st_mdelay_ptr;

typedef struct {
  uint32_t cs_port;
  uint32_t cs_pin;
  uint32_t write_data;
  uint32_t read_data;
  uint32_t size;
} SpiTrans_t;

// int32_t spi_write(void *param, uint8_t reg, const uint8_t *p_data,
//                   uint16_t len) {
//   return 0;
// }

// int32_t spi_read(void *param, uint8_t reg, uint8_t *p_data, uint16_t len) {
//   return 0;
// }

void spi_send_8b(uint8_t dat) {
  for (size_t i = 0; i < 8; i++) {
    gpio_bit_reset(GPIOB, GPIO_PIN_13);
    (dat >> (7 - i)) & 1 ? gpio_bit_set(GPIOB, GPIO_PIN_15)
                         : gpio_bit_reset(GPIOB, GPIO_PIN_15);
    gpio_bit_set(GPIOB, GPIO_PIN_13);
  }
}

uint8_t spi_recv_8b() {
  uint8_t dat = 0;
  for (size_t i = 0; i < 8; i++) {
    gpio_bit_reset(GPIOB, GPIO_PIN_13);

    gpio_bit_set(GPIOB, GPIO_PIN_13);

    gpio_input_bit_get(GPIOB, GPIO_PIN_14) == SET ? dat += (1 << (7 - i)) : 0;
  }
  return dat;
}

int32_t lsm6ds3tr_c_read_reg(const stmdev_ctx_t *ctx, uint8_t reg,
                             uint8_t *data, uint16_t len) {
  gpio_bit_reset(GPIOB, GPIO_PIN_10);

  spi_send_8b(reg | 0x80);

  for (size_t i = 0; i < len; i++) { *(data + i) = spi_recv_8b(); }

  gpio_bit_set(GPIOB, GPIO_PIN_10);

  return 0;
}

int32_t lsm6ds3tr_c_write_reg(const stmdev_ctx_t *ctx, uint8_t reg,
                              uint8_t *data, uint16_t len) {
  gpio_bit_reset(GPIOB, GPIO_PIN_10);

  spi_send_8b(reg & 0x7F);

  for (size_t i = 0; i < len; i++) { spi_send_8b(*(data + i)); }

  gpio_bit_set(GPIOB, GPIO_PIN_10);

  return 0;
}