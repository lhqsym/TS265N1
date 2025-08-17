#ifndef __APP_ROLE_SWITCH_H_
#define __APP_ROLE_SWITCH_H_


#if SWETZ_SWITCH_BY_BAT
void app_role_switch_by_bat(void);
void app_role_switch_by_inbox(void);
uint16_t app_role_switch_get_user_data(uint8_t *data_ptr);
uint16_t app_role_switch_set_user_data(uint8_t *data_ptr, uint16_t len);
#endif

#endif
