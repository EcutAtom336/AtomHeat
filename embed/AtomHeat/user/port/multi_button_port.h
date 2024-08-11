#ifndef _MULTI_BUTTON_PORT_H_
#define _MULTI_BUTTON_PORT_H_

#include <stdint.h>

typedef enum {
  BtnNum1,
  BtnNum2,
  BtnNum3,
  BtnNum4,
  BtnNum5,
  EncoderBtn,
  EncoderP,
  EncoderN,
  BtnMax,
} BtnNum_t;

void multi_button_port_init();

uint8_t multi_button_port_read_btn(uint8_t btn);

#endif