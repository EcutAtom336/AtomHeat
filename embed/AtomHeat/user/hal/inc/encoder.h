#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <stdint.h>

typedef void (*EncoderCb_t)(int8_t change_val);

/**
 * @brief 编码器相关初始化
 *
 */
void encoder_init();

/**
 * @brief 添加编码器处理回调（仅支持一个回调）
 *
 * @param cb
 */
void encoder_cb_add(EncoderCb_t cb);

/**
 * @brief 删除编码器处理回调
 *
 */
void encoder_cb_del();

#endif