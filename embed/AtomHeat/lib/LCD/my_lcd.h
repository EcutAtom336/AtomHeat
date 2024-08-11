#ifndef _MY_LCD_H_
#define _MY_LCD_H_

#include <stdint.h>

typedef enum {
  FontSize12,
  FontSize14,
  FontSize16,
} FontSize_t;

void lcd_handler_init();

void lcd_refreash_request();

void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
              uint16_t color);

// 图形相关
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                   uint16_t color);

void lcd_draw_multi_line(uint16_t *x, uint16_t *y, uint16_t num,
                         uint16_t color);

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t color);

void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

// 字符相关
void lcd_show_font(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc,
                   FontSize_t fs);

void lcd_show_str(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc,
                  uint16_t bc, uint8_t sizey);

void lcd_show_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                  uint16_t fc, uint16_t bc, uint8_t sizey);

void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc,
                    uint16_t bc, uint8_t sizey);

void lcd_show_pic(uint16_t x, uint16_t y, uint16_t height, uint16_t width,
                  const uint8_t *pic);

#endif