#ifndef _LSM6DS3TR_PORT_H_
#define _LSM6DS3TR_PORT_H_

#include "lsm6ds3tr-c_reg.h"
//
#include <stdint.h>

int32_t i2c_write(void *param, uint8_t reg,
                  const uint8_t *data, uint16_t len);

int32_t lsm6ds3tr_c_read_reg(const stmdev_ctx_t *ctx,
                             uint8_t reg, uint8_t *data,
                             uint16_t len);

int32_t lsm6ds3tr_c_write_reg(const stmdev_ctx_t *ctx,
                              uint8_t reg, uint8_t *data,
                              uint16_t len);

#endif
