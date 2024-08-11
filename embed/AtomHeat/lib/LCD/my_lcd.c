#include "my_lcd.h"
/***/
#include "color.h"
#include "font.h"
#include "lcd_init.h"
/***/
#define LOG_TAG "my lcd"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_groups.h"
/***/
#include "gd32f4xx.h"
/***/
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define LCD_BUF_SET(x, y, color)    \
  *(uint16_t *)((uint32_t)buf +     \
                ((x) > 159 ? 159    \
                 : (x) < 0 ? 0      \
                           : (x)) * \
                    2 +             \
                ((y) > 128 ? 128    \
                 : (y) < 0 ? 0      \
                           : (y)) * \
                    160 * 2) = ((color) << 8) + ((color) >> 8)

// #define LCD_BUF_SET(x, y, color)                           \
//   *(uint16_t *)((uint32_t)buf + (x) * 2 + (y) * 160 * 2) = \
//       ((color) << 8) + ((color) >> 8)

static uint16_t buf[160 * 128];

static SemaphoreHandle_t buf_mutex_handle;
static StaticSemaphore_t buf_mutex;

static TaskHandle_t flush_task_handle;
static StaticTask_t flush_task;
static uint8_t flush_task_stack[1024];

static SemaphoreHandle_t lcd_flash_compl_sem_handle;
static StaticSemaphore_t lcd_flash_compl_sem;

static void lcd_buf_flush() {
  dma_single_data_parameter_struct init_struct_s = {
      .periph_addr         = (uint32_t)&SPI_DATA(SPI2),
      .periph_inc          = DMA_PERIPH_INCREASE_DISABLE,
      .memory0_addr        = (uint32_t)buf,
      .memory_inc          = DMA_MEMORY_INCREASE_ENABLE,
      .periph_memory_width = DMA_PERIPH_WIDTH_8BIT,
      .circular_mode       = DMA_CIRCULAR_MODE_DISABLE,
      .direction           = DMA_MEMORY_TO_PERIPH,
      // .number              = sizeof(buf) + 10,
      .number   = sizeof(buf),
      .priority = DMA_PRIORITY_HIGH,
  };
  TickType_t tick;

  while (1) {
    // 等待刷新请求
    ulTaskNotifyTake(0xFFFFFFFF, portMAX_DELAY);

    xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);

    LCD_CS_Clr();

    dma_deinit(DMA0, DMA_CH5);
    dma_single_data_mode_init(DMA0, DMA_CH5, &init_struct_s);
    dma_interrupt_flag_clear(DMA0, DMA_CH5, DMA_INT_FLAG_FTF);
    dma_channel_enable(DMA0, DMA_CH5);
    dma_interrupt_enable(DMA0, DMA_CH5, DMA_INT_FTF);

    xSemaphoreTake(lcd_flash_compl_sem_handle, pdMS_TO_TICKS(100));

    xSemaphoreGive(buf_mutex_handle);

    LCD_CS_Set();
  }
}

void lcd_refreash_request() { xTaskNotifyGive(flush_task_handle); }

#warning

void lcd_buf_lock() { xSemaphoreTake(buf_mutex_handle, portMAX_DELAY); }

void lcd_buf_unlock() { xSemaphoreGive(buf_mutex_handle); }

void lcd_handler_init() {
  buf_mutex_handle = xSemaphoreCreateMutexStatic(&buf_mutex);
  configASSERT(buf_mutex_handle);

  lcd_flash_compl_sem_handle =
      xSemaphoreCreateBinaryStatic(&lcd_flash_compl_sem);
  configASSERT(lcd_flash_compl_sem_handle);

  flush_task_handle =
      xTaskCreateStatic(lcd_buf_flush, "lcdFlush", sizeof(flush_task_stack) / 4,
                        NULL, 8, (StackType_t *)flush_task_stack, &flush_task);
  configASSERT(flush_task_handle);

  lcd_fill(0, 0, LCD_W, LCD_H, 0);

  lcd_refreash_request();
}

static void lcd_fill_unmutex(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                             uint16_t color) {
  for (size_t i = x0; i < x1; i++)
    for (size_t j = y0; j < y1; j++) LCD_BUF_SET(i, j, color);
}

void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
              uint16_t color) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  lcd_fill_unmutex(x0, y0, x1, y1, color);

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  LCD_BUF_SET(x, y, color);

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_draw_line_unmutex(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                           uint16_t color) {
  uint16_t t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;  // 计算坐标增量
  delta_y = y2 - y1;
  uRow    = x1;  // 画线起点坐标
  uCol    = y1;
  if (delta_x > 0)
    incx = 1;  // 设置单步方向
  else if (delta_x == 0)
    incx = 0;  // 垂直线
  else {
    incx    = -1;
    delta_x = -delta_x;
  }
  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;  // 水平线
  else {
    incy    = -1;
    delta_y = -delta_y;
  }
  if (delta_x > delta_y)
    distance = delta_x;  // 选取基本增量坐标轴
  else
    distance = delta_y;
  for (t = 0; t < distance + 1; t++) {
    LCD_BUF_SET(uRow, uCol, color);  // 画点
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                   uint16_t color) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  lcd_draw_line_unmutex(x1, y1, x2, y2, color);

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_draw_multi_line(uint16_t *x, uint16_t *y, uint16_t point_num,
                         uint16_t color) {
  if (point_num < 2) return;
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  for (size_t i = 0; i < point_num - 1; i++)
    lcd_draw_line_unmutex(
        *(uint16_t *)((uint32_t)x + i * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)y + i * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)x + (i + 1) * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)y + (i + 1) * sizeof(uint16_t)), color);

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t color) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  lcd_draw_line_unmutex(x1, y1, x2, y1, color);
  lcd_draw_line_unmutex(x1, y1, x1, y2, color);
  lcd_draw_line_unmutex(x1, y2, x2, y2, color);
  lcd_draw_line_unmutex(x2, y1, x2, y2, color);

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  uint16_t a, b;
  a = 0;
  b = r;
  while (a <= b) {
    LCD_BUF_SET(x0 - b, y0 - a, color);
    LCD_BUF_SET(x0 + b, y0 - a, color);
    LCD_BUF_SET(x0 - a, y0 + b, color);
    LCD_BUF_SET(x0 - a, y0 - b, color);
    LCD_BUF_SET(x0 + b, y0 + a, color);
    LCD_BUF_SET(x0 + a, y0 - b, color);
    LCD_BUF_SET(x0 + a, y0 + b, color);
    LCD_BUF_SET(x0 - b, y0 + a, color);
    a++;
    if ((a * a + b * b) > (r * r))  // 判断要画的点是否过远
      b--;
  }

  xSemaphoreGive(buf_mutex_handle);
}

static void my_lcd_show_hanz12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 12,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT12_SIZE; font_idx++) {
    if (FONT12_DATA[font_idx].txt[0] == s[0] &&
        FONT12_DATA[font_idx].txt[1] == s[1] &&
        FONT12_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT12_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT12_NULL.dat[i * ROW_BYTE + (j >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(x + j, y + i, fc);
}

static void my_lcd_show_hanz14(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 14,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT12_SIZE; font_idx++) {
    if (FONT14_DATA[font_idx].txt[0] == s[0] &&
        FONT14_DATA[font_idx].txt[1] == s[1] &&
        FONT14_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT14_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT14_NULL.dat[i * ROW_BYTE + (j >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(x + j, y + i, fc);
}

static void my_lcd_show_hanz16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 16,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT16_SIZE; font_idx++) {
    if (FONT16_DATA[font_idx].txt[0] == s[0] &&
        FONT16_DATA[font_idx].txt[1] == s[1] &&
        FONT16_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT16_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT16_NULL.dat[i * ROW_BYTE + ((j) >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(x + j, y + i, fc);
}

void lcd_show_font(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc,
                   FontSize_t fs) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  while (*s != 0) {
    switch (fs) {
      case FontSize12: {
        my_lcd_show_hanz12(x, y, s, fc, bc);
        x += 12;
        break;
      }
      case FontSize14: {
        my_lcd_show_hanz14(x, y, s, fc, bc);
        x += 14;
        break;
      }
      case FontSize16: {
        my_lcd_show_hanz16(x, y, s, fc, bc);
        x += 16;
        break;
      }
      default:
        break;
    }
    s += 3;
  }

  xSemaphoreGive(buf_mutex_handle);
}

static void lcd_show_char(uint16_t x, uint16_t y, uint8_t num, uint16_t fc,
                          uint16_t bc, uint8_t sizey) {
  uint8_t temp, sizex, t;
  uint16_t i, TypefaceNum;  // 一个字符所占字节大小
  uint16_t x0 = x;
  sizex       = sizey / 2;
  TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
  num         = num - ' ';

  if (fc != bc) lcd_fill_unmutex(x, y, x + sizex - 1, y + sizey - 1, bc);

  for (i = 0; i < TypefaceNum; i++) {
    if (sizey == 12)
      temp = ascii_1206[num][i];  // 调用6x12字体
    else if (sizey == 16)
      temp = ascii_1608[num][i];  // 调用8x16字体
    else if (sizey == 24)
      temp = ascii_2412[num][i];  // 调用12x24字体
    else if (sizey == 32)
      temp = ascii_3216[num][i];  // 调用16x32字体
    else
      return;
    for (t = 0; t < 8; t++) {
      if (temp & (0x01 << t)) LCD_BUF_SET(x, y, fc);
      x++;
      if ((x - x0) == sizex) {
        x = x0;
        y++;
        break;
      }
    }
  }
}

void lcd_show_str(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc,
                  uint16_t bc, uint8_t sizey) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  while (*p != '\0') {
    lcd_show_char(x, y, *p, fc, bc, sizey);
    x += sizey / 2;
    p++;
  }

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_show_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                  uint16_t fc, uint16_t bc, uint8_t sizey) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  uint8_t t, temp;
  uint8_t enshow = 0;
  uint8_t sizex  = sizey / 2;
  for (t = 0; t < len; t++) {
    temp = (uint16_t)(num / powf(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        lcd_show_char(x + t * sizex, y, ' ', fc, bc, sizey);
        continue;
      } else
        enshow = 1;
    }
    lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, sizey);
  }

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc,
                    uint16_t bc, uint8_t sizey) {
  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  uint8_t t, temp, sizex;
  uint16_t num1;
  sizex = sizey / 2;
  num1  = num * 100;
  for (t = 0; t < len; t++) {
    temp = (uint16_t)(num1 / pow(10, len - t - 1)) % 10;
    if (t == (len - 2)) {
      lcd_show_char(x + (len - 2) * sizex, y, '.', fc, bc, sizey);
      t++;
      len += 1;
    }
    lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, sizey);
  }

  xSemaphoreGive(buf_mutex_handle);
}

void lcd_show_pic(uint16_t x, uint16_t y, uint16_t height, uint16_t width,
                  const uint8_t *pic) {
  uint32_t k = 0;

  xSemaphoreTake(buf_mutex_handle, portMAX_DELAY);

  for (size_t i = 0; i < height; i++)
    for (size_t j = 0; j < width; j++) {
      LCD_BUF_SET(x + j, y + i, *(uint16_t *)((uint32_t)pic + k));
      k += 2;
    }

  xSemaphoreGive(buf_mutex_handle);
}

void DMA0_Channel5_IRQHandler() {
  if (dma_interrupt_flag_get(DMA0, DMA_CH5, DMA_INT_FLAG_FTF)) {
    xSemaphoreGiveFromISR(lcd_flash_compl_sem_handle, NULL);
    dma_interrupt_flag_clear(DMA0, DMA_CH5, DMA_INT_FLAG_FTF);
  }
}
