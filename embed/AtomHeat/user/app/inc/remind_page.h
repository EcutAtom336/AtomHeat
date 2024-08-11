#ifndef _REMIND_H_
#define _REMIND_H_

typedef enum {
  RemindTypeNone,
  RemindTypeVl,
  RemindTypeSlant,
  RemindTypeFinish,
  RemindTypeMax
} RemindType_t;

/**
 * @brief 提示页相关初始化
 *
 */
void remind_page_init();

/**
 * @brief 启动提示页
 *
 * @param remind 提示信息
 */
void remind_page_start(RemindType_t remind);

/**
 * @brief 停止提示页
 *
 */
void remind_page_stop();

#endif