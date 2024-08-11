#include "util_flash.h"
//
#include "gd32f4xx.h"
//

uint32_t util_flash_get_sector_header_by_addr(uint32_t addr) {
  if (addr >= ADDR_FLASH_SECTOR_0 && addr < ADDR_FLASH_SECTOR_1)
    return ADDR_FLASH_SECTOR_0;
  else if (addr >= ADDR_FLASH_SECTOR_1 && addr < ADDR_FLASH_SECTOR_2)
    return ADDR_FLASH_SECTOR_1;
  else if (addr >= ADDR_FLASH_SECTOR_2 && addr < ADDR_FLASH_SECTOR_3)
    return ADDR_FLASH_SECTOR_2;
  else if (addr >= ADDR_FLASH_SECTOR_3 && addr < ADDR_FLASH_SECTOR_4)
    return ADDR_FLASH_SECTOR_3;
  else if (addr >= ADDR_FLASH_SECTOR_4 && addr < ADDR_FLASH_SECTOR_5)
    return ADDR_FLASH_SECTOR_4;
  else if (addr >= ADDR_FLASH_SECTOR_5 && addr < ADDR_FLASH_SECTOR_6)
    return ADDR_FLASH_SECTOR_5;
  else if (addr >= ADDR_FLASH_SECTOR_6 && addr < ADDR_FLASH_SECTOR_7)
    return ADDR_FLASH_SECTOR_6;
  else if (addr >= ADDR_FLASH_SECTOR_7 && addr < ADDR_FLASH_SECTOR_8)
    return ADDR_FLASH_SECTOR_7;
  else if (addr >= ADDR_FLASH_SECTOR_8 && addr < ADDR_FLASH_SECTOR_9)
    return ADDR_FLASH_SECTOR_8;
  else if (addr >= ADDR_FLASH_SECTOR_9 && addr < ADDR_FLASH_SECTOR_10)
    return ADDR_FLASH_SECTOR_9;
  else if (addr >= ADDR_FLASH_SECTOR_10 && addr < ADDR_FLASH_SECTOR_11)
    return ADDR_FLASH_SECTOR_10;
  else if (addr >= ADDR_FLASH_SECTOR_11 && addr < ADDR_FLASH_SECTOR_12)
    return ADDR_FLASH_SECTOR_11;
  else if (addr >= ADDR_FLASH_SECTOR_12 && addr < ADDR_FLASH_SECTOR_13)
    return ADDR_FLASH_SECTOR_12;
  else if (addr >= ADDR_FLASH_SECTOR_13 && addr < ADDR_FLASH_SECTOR_14)
    return ADDR_FLASH_SECTOR_13;
  else if (addr >= ADDR_FLASH_SECTOR_14 && addr < ADDR_FLASH_SECTOR_15)
    return ADDR_FLASH_SECTOR_14;
  else if (addr >= ADDR_FLASH_SECTOR_15 && addr < ADDR_FLASH_SECTOR_16)
    return ADDR_FLASH_SECTOR_15;
  else if (addr >= ADDR_FLASH_SECTOR_16 && addr < ADDR_FLASH_SECTOR_17)
    return ADDR_FLASH_SECTOR_16;
  else if (addr >= ADDR_FLASH_SECTOR_17 && addr < ADDR_FLASH_SECTOR_18)
    return ADDR_FLASH_SECTOR_17;
  else if (addr >= ADDR_FLASH_SECTOR_18 && addr < ADDR_FLASH_SECTOR_19)
    return ADDR_FLASH_SECTOR_18;
  else if (addr >= ADDR_FLASH_SECTOR_19 && addr < ADDR_FLASH_SECTOR_20)
    return ADDR_FLASH_SECTOR_19;
  else if (addr >= ADDR_FLASH_SECTOR_20 && addr < ADDR_FLASH_SECTOR_21)
    return ADDR_FLASH_SECTOR_20;
  else if (addr >= ADDR_FLASH_SECTOR_21 && addr < ADDR_FLASH_SECTOR_22)
    return ADDR_FLASH_SECTOR_21;
  else  // if (addr >= ADDR_FLASH_SECTOR_22 && addr < ADDR_FLASH_SECTOR_23)
    return ADDR_FLASH_SECTOR_22;
}

FlashSectorNum_t util_flash_get_sector_num_by_addr(uint32_t addr) {
  if (addr >= ADDR_FLASH_SECTOR_0 && addr < ADDR_FLASH_SECTOR_1)
    return FlashSector0;
  else if (addr >= ADDR_FLASH_SECTOR_1 && addr < ADDR_FLASH_SECTOR_2)
    return FlashSector1;
  else if (addr >= ADDR_FLASH_SECTOR_2 && addr < ADDR_FLASH_SECTOR_3)
    return FlashSector2;
  else if (addr >= ADDR_FLASH_SECTOR_3 && addr < ADDR_FLASH_SECTOR_4)
    return FlashSector3;
  else if (addr >= ADDR_FLASH_SECTOR_4 && addr < ADDR_FLASH_SECTOR_5)
    return FlashSector4;
  else if (addr >= ADDR_FLASH_SECTOR_5 && addr < ADDR_FLASH_SECTOR_6)
    return FlashSector5;
  else if (addr >= ADDR_FLASH_SECTOR_6 && addr < ADDR_FLASH_SECTOR_7)
    return FlashSector6;
  else if (addr >= ADDR_FLASH_SECTOR_7 && addr < ADDR_FLASH_SECTOR_8)
    return FlashSector7;
  else if (addr >= ADDR_FLASH_SECTOR_8 && addr < ADDR_FLASH_SECTOR_9)
    return FlashSector8;
  else if (addr >= ADDR_FLASH_SECTOR_9 && addr < ADDR_FLASH_SECTOR_10)
    return FlashSector9;
  else if (addr >= ADDR_FLASH_SECTOR_10 && addr < ADDR_FLASH_SECTOR_11)
    return FlashSector10;
  else if (addr >= ADDR_FLASH_SECTOR_11 && addr < ADDR_FLASH_SECTOR_12)
    return FlashSector11;
  else if (addr >= ADDR_FLASH_SECTOR_12 && addr < ADDR_FLASH_SECTOR_13)
    return FlashSector12;
  else if (addr >= ADDR_FLASH_SECTOR_13 && addr < ADDR_FLASH_SECTOR_14)
    return FlashSector13;
  else if (addr >= ADDR_FLASH_SECTOR_14 && addr < ADDR_FLASH_SECTOR_15)
    return FlashSector14;
  else if (addr >= ADDR_FLASH_SECTOR_15 && addr < ADDR_FLASH_SECTOR_16)
    return FlashSector15;
  else if (addr >= ADDR_FLASH_SECTOR_16 && addr < ADDR_FLASH_SECTOR_17)
    return FlashSector16;
  else if (addr >= ADDR_FLASH_SECTOR_17 && addr < ADDR_FLASH_SECTOR_18)
    return FlashSector17;
  else if (addr >= ADDR_FLASH_SECTOR_18 && addr < ADDR_FLASH_SECTOR_19)
    return FlashSector18;
  else if (addr >= ADDR_FLASH_SECTOR_19 && addr < ADDR_FLASH_SECTOR_20)
    return FlashSector19;
  else if (addr >= ADDR_FLASH_SECTOR_20 && addr < ADDR_FLASH_SECTOR_21)
    return FlashSector20;
  else if (addr >= ADDR_FLASH_SECTOR_21 && addr < ADDR_FLASH_SECTOR_22)
    return FlashSector21;
  else  // if (addr >= ADDR_FLASH_SECTOR_22 && addr < ADDR_FLASH_SECTOR_23)
    return FlashSector22;
}

uint32_t util_flash_get_sector_size_byte(FlashSectorNum_t s) {
  switch (s) {
    case FlashSector0:
    case FlashSector1:
    case FlashSector2:
    case FlashSector3:
      return 16U * 1024U;
      break;
    case FlashSector4:
      return 64U * 1024U;
      break;
    case FlashSector5:
    case FlashSector6:
    case FlashSector7:
    case FlashSector8:
    case FlashSector9:
    case FlashSector10:
    case FlashSector11:
      return 128U * 1024U;
      break;
    case FlashSector12:
    case FlashSector13:
    case FlashSector14:
    case FlashSector15:
      return 16U * 1024U;
      break;
    case FlashSector16:
      return 64U * 1024U;
      break;
    case FlashSector17:
    case FlashSector18:
    case FlashSector19:
    case FlashSector20:
    case FlashSector21:
    case FlashSector22:
    case FlashSector23:
      return 128U * 1024U;
      break;
    case FlashSector24:
    case FlashSector25:
    case FlashSector26:
    case FlashSector27:
    default:
      return 256U * 1024U;
      break;
  }
}

fmc_state_enum util_flash_erase_sector(FlashSectorNum_t s) {
  fmc_state_enum fs = FMC_OPERR;
  fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR |
                 FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
  switch (s) {
    case FlashSector0:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_0);
      break;
    case FlashSector1:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_1);
      break;
    case FlashSector2:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_2);
      break;
    case FlashSector3:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_3);
      break;
    case FlashSector4:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_4);
      break;
    case FlashSector5:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_5);
      break;
    case FlashSector6:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_6);
      break;
    case FlashSector7:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_7);
      break;
    case FlashSector8:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_8);
      break;
    case FlashSector9:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_9);
      break;
    case FlashSector10:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_10);
      break;
    case FlashSector11:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_11);
      break;
    case FlashSector12:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_12);
      break;
    case FlashSector13:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_13);
      break;
    case FlashSector14:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_14);
      break;
    case FlashSector15:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_15);
      break;
    case FlashSector16:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_16);
      break;
    case FlashSector17:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_17);
      break;
    case FlashSector18:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_18);
      break;
    case FlashSector19:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_19);
      break;
    case FlashSector20:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_20);
      break;
    case FlashSector21:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_21);
      break;
    case FlashSector22:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_22);
      break;
    case FlashSector23:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_23);
      break;
    case FlashSector24:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_24);
      break;
    case FlashSector25:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_25);
      break;
    case FlashSector26:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_26);
      break;
    case FlashSector27:
      fs = fmc_sector_erase(CTL_SECTOR_NUMBER_27);
      break;
    default:
      break;
  }
  return fs;
}
