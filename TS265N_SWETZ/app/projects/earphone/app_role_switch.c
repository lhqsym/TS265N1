#include "include.h"

#if DEBUG_ROLE_SWITCH
#define ROLE_SWITCH_DEBUG(...)              printf(__VA_ARGS__)
#else
#define ROLE_SWITCH_DEBUG(...)
#endif

#if SWETZ_SWITCH_BY_BAT

void app_role_switch_by_bat(void)
{
   
    if (bt_tws_is_connected()
    && (!bt_tws_is_slave())
    && (bt_nor_is_connected())
#if SWETZ_CHECK_INCASE
    && (!sys_cb.flag_peer_in_case)
#endif 
    && (!bsp_res_is_playing())
    && (bsp_res_is_empty())
    && (!f_bt.warning_status)
    && (!ab_mate_ota_is_start())
    && ((f_bt.disp_status == BT_STA_CONNECTED) || (f_bt.disp_status == BT_STA_PLAYING) || (f_bt.disp_status == BT_STA_INCALL))
    && (sys_cb.peer_bat_level != 0xff)
    && (sys_cb.local_bat_level < sys_cb.peer_bat_level)
    && ((sys_cb.peer_bat_level - sys_cb.local_bat_level) >= 10)
   // && (!sys_cb.flag_find_me_left_ongoing)
   // && (!sys_cb.flag_find_me_right_ongoing)
   // && (!is_pwrkey_pressed())
   // && (tick_check_expire(sys_cb.tick_event, 1000))
   // && (message_is_empty())
    && (app_lr_is_idle())
    && (bt_get_scan() != 0x03)
    )
    {
        printf("3AUDIO SWITCH\n");
        bt_tws_switch();
    } 
}

void app_role_switch_by_inbox(void)
{
//     if (bt_tws_is_connected()
//     && (!bt_tws_is_slave())
//     && (bt_nor_is_connected())
//     && (sys_cb.flag_local_in_case)
//     && (!sys_cb.flag_peer_in_case)
//     && (sys_cb.incall_flag)
//     && (!bsp_res_is_playing())
//     && (bsp_res_is_empty())
//     && (!f_bt.warning_status)
//     && (!ab_mate_ota_is_start())
//     // && (!sys_cb.flag_find_me_left_ongoing)
//     // && (!sys_cb.flag_find_me_right_ongoing)
//    // && (!is_pwrkey_pressed())
//     && (tick_check_expire(sys_cb.tick_event, 1000))
//     && (sys_cb.peer_bat_level != 0xff)
//     && (sys_cb.local_bat_level < sys_cb.peer_bat_level)
//     && ((sys_cb.peer_bat_level - sys_cb.local_bat_level) >= 10)
//     && (app_lr_is_idle())
//     )
//     {
//         bt_tws_switch();
//     }

    if (bt_tws_is_connected()
    && (!bt_tws_is_slave())
    && (bt_nor_is_connected())
    && (sys_cb.flag_local_in_case)
    && (!sys_cb.flag_peer_in_case)
    // && (!sys_cb.incall_flag)
    //&& (!bsp_res_is_playing())
    // && (bsp_res_is_empty())
    && (!ab_mate_ota_is_start())
    //&& (tick_check_expire(sys_cb.tick_event, 100))
    // && (app_lr_is_idle())
    )
    {
        
       printf("4AUDIO SWITCH\n");
       bt_tws_switch();

#if SWETZ_SLAVE_FIND_DEV
        if(ab_mate_app.find_type == 1){
                app_lr_send_msg(EVT_FIND_DEV);
                ab_mate_app.find_type = 0;
        }

#endif

    }
    printf("sys_cb.flag_local_in_case:%d sys_cb.flag_peer_in_case:%d\r\n",sys_cb.flag_local_in_case,sys_cb.flag_peer_in_case);
}

uint16_t app_role_switch_get_user_data(uint8_t *data_ptr)
{
    // u8 offset = 0;
    
    // ROLE_SWITCH_DEBUG("role_switch_get_user_data\n");
    // data_ptr[offset++] = app_priv_ble_seq_num_rx;
    // data_ptr[offset++] = app_priv_ble_seq_num_tx;
    
    // return offset;
}

uint16_t app_role_switch_set_user_data(uint8_t *data_ptr, uint16_t len)
{
    // u8 offset = 0;
    
    // ROLE_SWITCH_DEBUG("role_switch_set_user_data\n");
    // app_priv_ble_seq_num_rx = data_ptr[offset++];
    // app_priv_ble_seq_num_tx = data_ptr[offset++];
    
    // return offset;
}

#endif