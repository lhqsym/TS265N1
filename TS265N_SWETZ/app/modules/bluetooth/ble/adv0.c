/*
 *  adv0.c
 *  BLE通道0广播相关
 */
#include "include.h"

//adv0配置变量，如果使用PUBLIC地址，需要或上BLE_ADV0_ADDR_PUBIC_BIT
const uint8_t cfg_ble_adv0_en = (LE_ADV0_EN*BLE_ADV0_EN_BIT) | BLE_ADV0_ADDR_PUBIC_BIT;

#if (LE_ADV0_EN && !AB_MATE_ADV0_EN)

#if LE_WIN10_POPUP
const uint8_t window_adv_data[] = {
    // 0x1E, //Lenght
    // 0xff, //Vendor Defined Flag
    // 0x06, 0x00,//Mircosoft Vendor ID
    // 0x03,//Mircosoft Beacon ID
    // 0x01,//Mircosoft Beacon Sub Scenario
    // 0x80,//Reserved RSSI Byte
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//BR/EDR Address(little-endian )
    // 0x18, 0x04, 0x24,//Classic Device lcon appearance
    // //'B','T','8','8','9','2',//Display Name
    // 'B', 'l', 'u', 'e','t','r','u','m','-','B','T','8','9','2','A'


    // Service UUID (0x4A40)
    0x03, 0x03, 0x40, 0x4A, // AD Structure: 16-bit UUID (0x4A40)

    // Manufacturer Specific Data
    0x14, // Length (21 bytes)
    0xFF, // AD Type: Manufacturer Specific Data
    0xCA, 0x08, // Company Identifier Code (0x0958, ZTE)


    0x01, // packet_type: 1 (配对广播包)
    0x01, // num_of_address: 1 (单设备)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // address: 11:22:33:44:55:66 (little-endian)
    0x01, 0x07,  // type: 主设备类型 (0x01), 子设备类型 (0x01)
    0x01, // bond_state: 0x01 (未绑定)
    0x01, // pairing_state: 0 (非配对模式)
    0x01, // connection_state: 0x01 (未连接)
    0xff, // battery_left: 50% (0x32)
    0xff, // battery_right: 60% (0x3C)
    0xff,  // battery_case: 80% (0x50)
    0x00,    //保留字节 (0x00)
    0x02, 0x01, 0x02,

};
#endif // LE_WIN10_POPUP

//-------------------------------------------------------------------
//------------------libbtstack.a回调函数-----------------------------
//-------------------------------------------------------------------

//设置adv0 PUBIC广播地址
void ble_adv0_get_local_bd_addr(u8 *addr)
{
    bt_get_local_bd_addr(addr);
}

//tws时通常由主耳广播，该回调函数决定是否广播
//返回值: true=广播, false=不广播
bool ble_adv0_get_tws_role(void) {
#if (CHARGE_BOX_TYPE == CBOX_SSW)
    if(!charge_box_popup_is_enable()) {
        return true;
    }
#endif
    if(bt_tws_is_slave()) {
        return false;//true;
    }
    return false;//false;
}

//主从切换前回调，获取主耳广播相关数据
//u8 ble_adv0_get_tws_loc_var(uint8_t *ptr)
//{
//    ptr[0] = sys_cb.loc_house_state & 0x03;
//    ptr[1] = sys_cb.loc_house_bat;
//    ptr[2] = sys_cb.loc_bat;
//    return 3;
//}

//主从切换后回调，广播相关数据设置到新主耳
//void ble_adv0_set_tws_rem_var(uint8_t *ptr)
//{
//    sys_cb.rem_house_state  = ptr[0] & 0x03;
//    sys_cb.rem_house_bat    = ptr[1];
//    sys_cb.rem_bat          = ptr[2];
//}

uint8_t bat_level_show_for_popup(uint8_t bat_real_level)
{
    uint8_t bat_show;

    if (bat_real_level < 15)
    {
        bat_show = 10;
    }
    else if (bat_real_level < 25)
    {
        bat_show = 20;
    }
    else if (bat_real_level < 35)
    {
        bat_show = 30;
    }
    else if (bat_real_level < 45)
    {
        bat_show = 40;
    }
    else if (bat_real_level < 55)
    {
        bat_show = 50;
    }
    else if (bat_real_level < 65)
    {
        bat_show = 60;
    }
    else if (bat_real_level < 75)
    {
        bat_show = 70;
    }
    else if (bat_real_level < 85)
    {
        bat_show = 80;
    }
    else if (bat_real_level < 95)
    {
        bat_show = 90;
    }
    else
    {
        bat_show = 100;
    }
    return bat_show;
}


 uint8_t bond_state = 0x01;    // 初始未绑定
 uint8_t pairing_state = 0;
 uint8_t connection_state = 0x01;

//设置广播内容格式，返回内容长度（最长31byte）
uint8_t ble_adv0_get_adv_data_cb(uint8_t *adv_data)
{
//    //示例
//    adv_data[0] = 0x09,         //[0]   len
//    adv_data[1] = 0xff,         //[1]   data_type
//    adv_data[2] = 0x42,         //[2:3] comany_id
//    adv_data[3] = 0x06,         //[2:3] comany_id
//    adv_data[4] = 0x00,         //[7]   headset_type
//    adv_data[5] = 0x00,         //[20]  color
//    adv_data[6] = 0x00,         //[20]  state
//    adv_data[7] = 100,          //[16]  battery_left
//    adv_data[8] = 100,          //[17]  battery_right
//    adv_data[9] = 100 ,         //[18]  battery_house
//    return 10;
#if LE_WIN10_POPUP
    uint8_t mac_buf[6];
    static uint8_t bt_status;
    memcpy(adv_data,window_adv_data,sizeof(window_adv_data));
    bt_get_local_bd_addr(mac_buf);
    // adv_data[7] = mac_buf[5];
    // adv_data[8] = mac_buf[4];
    // adv_data[9] = mac_buf[3];
    // adv_data[10]= mac_buf[2];
    // adv_data[11]= mac_buf[1];
    // adv_data[12]= mac_buf[0];

    adv_data[10] = mac_buf[0];
    adv_data[11] = mac_buf[1];
    adv_data[12] = mac_buf[2];
    adv_data[13]= mac_buf[3];
    adv_data[14]= mac_buf[4];
    adv_data[15]= mac_buf[5];

if(!bt_tws_is_slave()){
    sys_cb.bt_master_link = bt_nor_get_link_info(NULL);
    app_lr_send_notification(LR_NOTIFY_BT_LINK, 1, &sys_cb.bt_master_link);

}
        if(sys_cb.bt_master_link) {
            adv_data[18] = 0x02;
            }else{
            adv_data[18] = 0x01;
        }




if(!bt_tws_is_slave()){
             sys_cb.bt_master_status = bt_get_disp_status();
             app_lr_send_notification(LR_NOTIFY_BT_STATUS, 1, &sys_cb.bt_master_status);
        }    
         

          //  printf("----sys_cb.bt_master_status:%d\r\n",sys_cb.bt_master_status);

            if(sys_cb.bt_master_status < BT_STA_CONNECTED){
                adv_data[19] = 0x01;
            }else{
                adv_data[19] = 0x00;
            }


            if(sys_cb.bt_master_status  < BT_STA_CONNECTING){
                adv_data[20] = 0x01;
            }
            else if(sys_cb.bt_master_status == BT_STA_CONNECTING){
               if(bt_tws_is_connected()){
                    adv_data[20] = 0x04;
               }else {
                    adv_data[20] = 0x01;
               }
                
            }
            else if(sys_cb.bt_master_status > BT_STA_CONNECTING){
                adv_data[20] = 0x02;
            }

    
    bond_state = adv_data[18];
    pairing_state = adv_data[19];
    connection_state = adv_data[20];

    sys_cb.local_bat_level = bsp_get_bat_level();
    app_lr_send_notification(LR_NOTIFY_BATTERY_LEVEL, 1, &sys_cb.local_bat_level);
    // printf("888sys_cb.local_bat_level:%d  sys_cb.peer_bat_level:%d\r\n,",sys_cb.local_bat_level,sys_cb.peer_bat_level);
    // printf("8888-ab_mate_app.local_vbat:%d  ab_mate_app.remote_vbat:%d\r\n,",ab_mate_app.local_vbat,ab_mate_app.remote_vbat);
    if(sys_cb.tws_left_channel){
    adv_data[21] = (ab_mate_app.local_vbat & 0x7F);//bat_level_show_for_popup(sys_cb.local_bat_level);//sys_cb.local_bat_level;ab_mate_app.local_vbat
    adv_data[22] = (ab_mate_app.remote_vbat & 0x7F);//bat_level_show_for_popup(sys_cb.peer_bat_level);//sys_cb.peer_bat_level; ab_mate_app.remote_vbat
    printf("1111-ab_mate_app.local_vbat:%d  ab_mate_app.remote_vbat:%d ab_mate_app.box_vbat:%d\r\n,",adv_data[21],adv_data[22],ab_mate_app.box_vbat);
    }
    else {
    adv_data[21] = (ab_mate_app.remote_vbat & 0x7F);//bat_level_show_for_popup(sys_cb.peer_bat_level);//sys_cb.local_bat_level;
    adv_data[22] = (ab_mate_app.local_vbat & 0x7F);//bat_level_show_for_popup(sys_cb.local_bat_level);//sys_cb.peer_bat_level; 
     printf("1111-ab_mate_app.remote_vbat:%d  ab_mate_app.local_vbat:%d ab_mate_app.box_vbat:%d\r\n,",adv_data[21],adv_data[22],ab_mate_app.box_vbat);
    }


    adv_data[23] = ab_mate_app.box_vbat;//charge_box_get_charge_box_bat_level();//bsp_get_bat_level();

  //  printf("adv_data[21]:%d  adv_data[22]:%d adv_data[23]:%d \r\n",adv_data[21],adv_data[22],adv_data[23]);
    return sizeof(window_adv_data);
#else
    return 0;
#endif // LE_WIN10_POPUP
}

#if BLE_POPU_TEST
// 更新广播模式
void update_broadcast_mode(void) 
{
#if !SWETZ
    static battery_temp = 0xff;
    if(battery_temp == 0xff){
            battery_temp = sys_cb.local_bat_level;
    }
    


    // 不广播的场景：听音乐、打电话和低延迟模式时不广播
    if(sys_cb.bt_master_status == BT_STA_PLAYING || sys_cb.bt_master_status == BT_STA_INCALL || bt_is_low_latency() || sys_cb.lowbat_flag == 1){ 
        // 例外情况：电量变化5%时需要广播10秒

        if ((battery_temp - sys_cb.local_bat_level) >= 5 || sys_cb.low_power_mark == 1) {
            battery_temp = sys_cb.local_bat_level;
            ble_adv0_set_ctrl(1);
            ble_adv0_set_ctrl(2);
            sys_cb.low_power_mark = 0;
            sys_cb.time_ble = 10;//计时10s
        } else {
            if(sys_cb.time_ble == 0){
                 ble_adv0_set_ctrl(2);
                 ble_adv0_set_ctrl(0);//关广播    
                 printf("ble_adv0 is close\r\n");
            }else {
                 ble_adv0_set_ctrl(1);
                 ble_adv0_set_ctrl(2);
            }
           
        }
         printf("battery_temp:%d    diff:%d\r\n",battery_temp,battery_temp - sys_cb.local_bat_level);
        return;
    }


    
    // 确定广播模式
    bool need_low_latency = (sys_cb.bt_master_status == BT_STA_IDLE) || 
                           (pairing_state == 1) ||
                           ((sys_cb.bt_master_status == BT_STA_CONNECTED) && 
                            (
                                ((tick_get()) - sys_cb.bt_con_titck) < 10000
                            ));

    if (need_low_latency) {
        ble_adv0_set_intv(176);
        ble_adv0_set_ctrl(2);
    } else {
        ble_adv0_set_intv(1760);
        ble_adv0_set_ctrl(2);
    }
    printf("need_low_latency:%d\r\n",need_low_latency);
    //     if (tick_check_expire(0, 1000)) {
    //     printf("need_low_latency:%d  battery_temp:%d    diff:%d\r\n",need_low_latency,battery_temp,battery_temp - sys_cb.local_bat_level);
    // }

    if(!ble_adv0_get_adv_en()){
       
        ble_adv0_set_ctrl(1);
         printf("ble_adv0 is open\r\n");
    }
#else 

#if SWETZ_BLE_PUPO

        
        if(!bt_tws_is_slave()){
             sys_cb.bt_master_status = bt_get_disp_status();
             
             app_lr_send_notification(LR_NOTIFY_BT_STATUS, 1, &sys_cb.bt_master_status);
             app_lr_send_notification(LR_NOTIFY_LINK_RETURN, 1, &sys_cb.return_link);
        }  

        if((bt_nor_get_link_info(NULL) && sys_cb.return_link == 1) || (sys_cb.bt_master_status >= BT_STA_CONNECTED)){
                    if(ble_adv0_get_adv_en()){
                            ble_adv0_set_ctrl(0);
                    }

        }else if(sys_cb.bt_master_status < BT_STA_CONNECTED){
                    if(!ble_adv0_get_adv_en()){
                            ble_adv0_set_ctrl(1);
                    }
                            
        }
       // printf("sys_cb.return_link:%d sys_cb.bt_master_status:%d   ble_adv0_get_adv_en:%d\r\n",sys_cb.return_link,sys_cb.bt_master_status,ble_adv0_get_adv_en());
        
        
        // if(ble_adv0_get_adv_en()){
        //         ble_adv0_set_ctrl(2);
        // }

#endif        

#endif

}
#endif


#if LE_ADV0_CON_EN
//设置广播内容格式，返回内容长度（最长31byte）
uint8_t ble_adv0_get_scan_rsp_data_cb(uint8_t *scan_rsp_data)
{
    return 0;
}

AT(.text.bb.ble.adv0)
void ble_adv0_get_type_cb(uint8_t *adv_type)
{
    *adv_type = 0;  //0=可连接，3=不可连接
}
#endif

//-------------------------------------------------------------------
//----------------------APP调用的api函数-----------------------------
//-------------------------------------------------------------------
//更新广播
void ble_adv0_update_adv(void)
{
    ble_adv0_set_ctrl(2);
#if LE_ADV0_3BAT_EN
    if(bt_tws_is_slave()) {
        bt_tws_sync_setting();  //副耳的数据同步到主耳，由主耳来广播
    } else {
    }
#endif
}

#if LE_ADV0_CON_EN
//更新扫描响应
void ble_adv0_update_scan_rsp(void)
{
    ble_adv0_set_ctrl(3);
}
#endif

#endif //LE_ADV0_EN
