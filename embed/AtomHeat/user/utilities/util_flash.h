#ifndef _UTIL_FLASH_H_
#define _UTIL_FLASH_H_

#include "gd32f4xx.h"
//
#include <stdint.h>

/* base address of the flash sectors */
#define ADDR_FLASH_SECTOR_0 \
  ((uint32_t)0x08000000) /* Base address of Sector 0, 16 K bytes   */
#define ADDR_FLASH_SECTOR_1 \
  ((uint32_t)0x08004000) /* Base address of Sector 1, 16 K bytes   */
#define ADDR_FLASH_SECTOR_2 \
  ((uint32_t)0x08008000) /* Base address of Sector 2, 16 K bytes   */
#define ADDR_FLASH_SECTOR_3 \
  ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 K bytes   */
#define ADDR_FLASH_SECTOR_4 \
  ((uint32_t)0x08010000) /* Base address of Sector 4, 64 K bytes   */
#define ADDR_FLASH_SECTOR_5 \
  ((uint32_t)0x08020000) /* Base address of Sector 5, 128 K bytes  */
#define ADDR_FLASH_SECTOR_6 \
  ((uint32_t)0x08040000) /* Base address of Sector 6, 128 K bytes  */
#define ADDR_FLASH_SECTOR_7 \
  ((uint32_t)0x08060000) /* Base address of Sector 7, 128 K bytes  */
#define ADDR_FLASH_SECTOR_8 \
  ((uint32_t)0x08080000) /* Base address of Sector 8, 128 K bytes  */
#define ADDR_FLASH_SECTOR_9 \
  ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 K bytes  */
#define ADDR_FLASH_SECTOR_10 \
  ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes */
#define ADDR_FLASH_SECTOR_11 \
  ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes */
#define ADDR_FLASH_SECTOR_12 \
  ((uint32_t)0x08100000) /* Base address of Sector 12, 16 K bytes  */
#define ADDR_FLASH_SECTOR_13 \
  ((uint32_t)0x08104000) /* Base address of Sector 13, 16 K bytes  */
#define ADDR_FLASH_SECTOR_14 \
  ((uint32_t)0x08108000) /* Base address of Sector 14, 16 K bytes  */
#define ADDR_FLASH_SECTOR_15 \
  ((uint32_t)0x0810C000) /* Base address of Sector 15, 16 K bytes  */
#define ADDR_FLASH_SECTOR_16 \
  ((uint32_t)0x08110000) /* Base address of Sector 16, 64 K bytes  */
#define ADDR_FLASH_SECTOR_17 \
  ((uint32_t)0x08120000) /* Base address of Sector 17, 128 K bytes */
#define ADDR_FLASH_SECTOR_18 \
  ((uint32_t)0x08140000) /* Base address of Sector 18, 128 K bytes */
#define ADDR_FLASH_SECTOR_19 \
  ((uint32_t)0x08160000) /* Base address of Sector 19, 128 K bytes */
#define ADDR_FLASH_SECTOR_20 \
  ((uint32_t)0x08180000) /* Base address of Sector 20, 128 K bytes */
#define ADDR_FLASH_SECTOR_21 \
  ((uint32_t)0x081A0000) /* Base address of Sector 21, 128 K bytes */
#define ADDR_FLASH_SECTOR_22 \
  ((uint32_t)0x081C0000) /* Base address of Sector 22, 128 K bytes */
#define ADDR_FLASH_SECTOR_23 \
  ((uint32_t)0x081E0000) /* Base address of Sector 23, 128 K bytes */

typedef enum {
  FlashSector0,
  FlashSector1,
  FlashSector2,
  FlashSector3,
  FlashSector4,
  FlashSector5,
  FlashSector6,
  FlashSector7,
  FlashSector8,
  FlashSector9,
  FlashSector10,
  FlashSector11,
  FlashSector12,
  FlashSector13,
  FlashSector14,
  FlashSector15,
  FlashSector16,
  FlashSector17,
  FlashSector18,
  FlashSector19,
  FlashSector20,
  FlashSector21,
  FlashSector22,
  FlashSector23,
  FlashSector24,
  FlashSector25,
  FlashSector26,
  FlashSector27,
  FlashSectorMax,
} FlashSectorNum_t;

uint32_t util_flash_get_sector_header_by_addr(uint32_t addr);

FlashSectorNum_t util_flash_get_sector_num_by_addr(uint32_t addr);

uint32_t util_flash_get_sector_size_byte(FlashSectorNum_t s);

fmc_state_enum util_flash_erase_sector(FlashSectorNum_t s);

#endif