/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */

#include "util_flash.h"
#include "heat_info.h"
//
#define LOG_TAG "ef"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "semphr.h"
//
#include "gd32f4xx.h"
//
#include <easyflash.h>
#include <stdarg.h>

static SemaphoreHandle_t easyflash_mutex_handle;
static StaticSemaphore_t easyflash_mutex;

static HeatInfo_t default_heat_info_max_temp = {
    .is_constant = 1,
    .power_limit = 95,
    .check_slant = 1,
    .stage =
        {
            {
                .duartion_s = 5,
                .temp       = 320,
            },

            {
                .duartion_s = 0,
                .temp       = 100,
            },

            {
                .duartion_s = 0,
                .temp       = 100,
            },

            {
                .duartion_s = 0,
                .temp       = 100,
            },

            {
                .duartion_s = 0,
                .temp       = 100,
            },

        },
};

static HeatInfo_t default_heat_info_reflow = {
    .is_constant = 0,
    .power_limit = 95,
    .check_slant = 1,
    .stage =
        {
            {
                .duartion_s = 60,
                .temp       = 120,
            },

            {
                .duartion_s = 90,
                .temp       = 110,
            },

            {
                .duartion_s = 90,
                .temp       = 240,
            },

            {
                .duartion_s = 30,
                .temp       = 180,
            },

            {
                .duartion_s = 30,
                .temp       = 100,
            },

        },
};

static uint32_t boot_count = 0;

/* default environment variables set for user */
static const ef_env default_env_set[] = {
    {"heatInfo1", &default_heat_info_max_temp, sizeof(HeatInfo_t)},
    {"heatInfo2", &default_heat_info_reflow, sizeof(HeatInfo_t)},
    {"heatInfo3", &default_heat_info_reflow, sizeof(HeatInfo_t)},
    {"heatInfo4", &default_heat_info_reflow, sizeof(HeatInfo_t)},
    {"heatInfo5", &default_heat_info_reflow, sizeof(HeatInfo_t)},
    {"bootCount", &boot_count, sizeof(uint32_t)},
};

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
  EfErrCode result = EF_NO_ERR;

  *default_env      = default_env_set;
  *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

  easyflash_mutex_handle = xSemaphoreCreateMutexStatic(&easyflash_mutex);
  configASSERT(easyflash_mutex_handle);

  return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
  EfErrCode result = EF_NO_ERR;

  for (size_t i = 0; i < size; i++)
    *(uint8_t *)((uint32_t)(buf) + i) = *(__IO uint8_t *)(addr + i);

  return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
  EfErrCode result  = EF_NO_ERR;
  size_t erase_size = 0;
  FlashSectorNum_t s;
  fmc_state_enum fs;

  /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
  EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

  fmc_unlock();

  fmc_flag_clear(FMC_FLAG_RDDERR);
  fmc_flag_clear(FMC_FLAG_PGSERR);
  fmc_flag_clear(FMC_FLAG_PGMERR);
  fmc_flag_clear(FMC_FLAG_WPERR);
  fmc_flag_clear(FMC_FLAG_OPERR);
  fmc_flag_clear(FMC_FLAG_END);

  do { fs = fmc_state_get(); } while (fs != FMC_READY);

  while (erase_size < size) {
    s  = util_flash_get_sector_num_by_addr(addr);
    fs = util_flash_erase_sector(s);
    if (fs != FMC_READY) {
      result = EF_ERASE_ERR;
      break;
    }
    erase_size += util_flash_get_sector_size_byte(s);
  }

  fmc_lock();

  return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
  EfErrCode result = EF_NO_ERR;
  fmc_state_enum fs;

  fmc_unlock();

  fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR |
                 FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

  for (size_t i = 0; i < size; i++) {
    fs = fmc_byte_program(addr + i, ((uint8_t *)buf)[i]);
    if (*(__IO uint8_t *)(addr + i) != ((uint8_t *)buf)[i]) {
      result = EF_WRITE_ERR;
      break;
    }
  }

  fmc_lock();

  return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
  xSemaphoreTake(easyflash_mutex_handle, portMAX_DELAY);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) { xSemaphoreGive(easyflash_mutex_handle); }

/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {
#ifdef PRINT_DEBUG

  va_list args;

  /* args point to the first variable parameter */
  va_start(args, format);

  /* You can add your code under here. */

  va_end(args);

#endif
}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
  va_list args;

  /* args point to the first variable parameter */
  va_start(args, format);

  /* You can add your code under here. */

  va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
  va_list args;

  /* args point to the first variable parameter */
  va_start(args, format);

  /* You can add your code under here. */

  va_end(args);
}
