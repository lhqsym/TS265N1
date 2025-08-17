/*****************************************************************************
 * Module    : Config
 * File      : swetz_event.h
 * Function  : 用户自定义配置
 *****************************************************************************/

#ifndef SWETZ_EVENT_H
#define SWETZ_EVENT_H
#include "../swetz_typedef.h"

#define  USER_EVENT_MAX              10
#define  USER_EVENT_ERROR_NUM        USER_EVENT_MAX

typedef enum
{
    USER_EVENT_NONE = 0,
    USER_EVENT_I2C_TEST,
    USER_EVENT_SPI_TEST,
}user_event_e;

typedef struct
{
    user_event_e user_event;/*Event id*/
    swetz_uint16 user_timer_set_cnt;/*Timer set count num*/
    swetz_uint16 user_timer_current_cnt;/*Timer current count num*/
}user_event_stru;

void clear_user_event_list(void);
void remove_user_event_in_list(user_event_e user_event_remove);
void user_event_set(user_event_stru event);
user_event_e user_event_get(bool timer_cnt_enable);

#endif // SWETZ_EVENT_H
