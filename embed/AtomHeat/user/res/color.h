#ifndef _COLOR_H_
#define _COLOR_H_

#define RGB888_2_RGB565(color)                                                 \
  (((color) >> 8) & 0xF800) + (((color) >> 5) & 0x7E0) + (((color) >> 3) & 0x1F)

#define WHITE 0xFFFF
#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F

#endif