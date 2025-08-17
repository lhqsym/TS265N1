#include "include.h"

func_cb_t func_cb AT(.buf.func_cb);

#if VBAT_DETECT_EN

void lowpower_warning_do(void)
{
    if (sys_cb.lpwr_warning_times) {        //低电语音提示次数
        if (RLED_LOWBAT_FOLLOW_EN) {
            led_lowbat_follow_warning();
        }

        sys_cb.lowbat_flag = 1;
         //   bsp_res_play(TWS_RES_LOW_BATTERY);
        bsp_piano_warning_play(WARNING_TONE, TONE_LOW_BATTERY);
        printf("lowpower_warning_do---\r\n");
#if SWETZ
        sys_cb.low_power_mark = 1;
#endif
        plugin_lowbat_vol_reduce();         //低电降低音乐音量

        #if 0
        if (RLED_LOWBAT_FOLLOW_EN) {
            while (get_led_sta(1)) {        //等待红灯闪完
                delay_5ms(2);
            }
            led_lowbat_recover();
        }
        #endif

        if (sys_cb.lpwr_warning_times != 0xff) {
            sys_cb.lpwr_warning_times--;
        }
    }
}

void lowpower_switch_to_normal_do(void)
{
    sys_cb.vbat_nor_cnt++;

    if (sys_cb.vbat_nor_cnt > 40) {
        sys_cb.lowbat_flag = 0;
        sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
        plugin_lowbat_vol_recover();    //离开低电, 恢复音乐音量
    }
}

void lowpower_poweroff_do(void)
{
    bsp_piano_warning_play(WARNING_TONE, TONE_LOW_BATTERY);
    printf("lowpower_poweroff_do lowpower_warning_do---\r\n");
    sys_cb.pwrdwn_tone_en = 1;
#if ASYN_SHUTDOWN
    sys_cb.local_bat_level = 0;
    sys_cb.discon_reason = 0;//低电不同步关机
#endif
    func_cb.sta = FUNC_PWROFF;     //低电，进入关机或省电模式
}

AT(.text.func.msg)
void lowpower_vbat_process(void)
{
    int lpwr_vbat_sta = is_lowpower_vbat_warning();

    if (lpwr_vbat_sta == 1) {
      //  bsp_res_play(TWS_RES_LOW_BATTERY);
        lowpower_poweroff_do();
        return;
    }

    if ((func_cb.mp3_res_play == NULL) || (lpwr_vbat_sta != 2)) {
        if ((sys_cb.lowbat_flag) && (sys_cb.vbat > 3800)) {
            lowpower_switch_to_normal_do();
        }
        return;
    }

    //低电提示音播放
    sys_cb.vbat_nor_cnt = 0;
    if (sys_cb.lpwr_warning_cnt > xcfg_cb.lpwr_warning_period) {
        sys_cb.lpwr_warning_cnt = 0;
        lowpower_warning_do();
    }
}
#endif // VBAT_DETECT_EN

void func_volume_up(void)
{
    if (func_cb.sta == FUNC_BT) {
        if (sys_cb.incall_flag) {
            bsp_bt_call_volume_msg(KU_VOL_UP);
        } else {
#if BT_HID_VOL_CTRL_EN
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_UP)) {
                return;
            }
#endif

            bt_music_vol_up();
#if WARNING_MAX_VOLUME

            if (sys_cb.vol == VOL_MAX) {
                maxvol_tone_play();
            }
           
#endif

        }
    } else {
        bsp_set_volume(bsp_volume_inc(sys_cb.vol));
    }

    if (func_cb.set_vol_callback) {
         func_cb.set_vol_callback(1);
    }


}

void func_volume_down(void)
{
    if (func_cb.sta == FUNC_BT) {
        if (sys_cb.incall_flag) {
            bsp_bt_call_volume_msg(KU_VOL_DOWN);
        } else {
#if BT_HID_VOL_CTRL_EN
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_DOWN)) {
                return;
            }
#endif

            bt_music_vol_down();

#if WARNING_MIN_VOLUME
            if (sys_cb.vol == 0) {
                minvol_tone_play();
            }
#endif
        }
    } else {
        bsp_set_volume(bsp_volume_dec(sys_cb.vol));
    }

    if (func_cb.set_vol_callback) {
        func_cb.set_vol_callback(0);
    }
}

#if SWETZ_SPP_CMD
AT(.text.func.msg)
static void spp_at_cmd_process(void)
{
	char spp_tx_buffer[80];
    u8 *ptr = eq_rx_buf;

    memset(spp_tx_buffer,0,sizeof(spp_tx_buffer));
    if (memcmp((char *)ptr, "version", strlen("version")) == 0)
    {
       // u8* cc_version = app_param_get_cc_version();

		sprintf(spp_tx_buffer, "+version:hs V%d.%d.%d",SW_VERSION[1]-'0', SW_VERSION[3]-'0', SW_VERSION[5]-'0');
		bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

    else if (memcmp((char *)ptr, "battery", strlen("battery")) == 0)
    {
            sprintf(spp_tx_buffer, "+battery:vbat[%d] local_bat_level[%d] peer_bat_level[%d]",sys_cb.vbat,sys_cb.local_bat_level,sys_cb.peer_bat_level);
            bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }
    
    else if (memcmp((char *)ptr, "poweroff", strlen("poweroff")) == 0)
    {
        sprintf(spp_tx_buffer, "+poweroff");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        delay_5ms(100);
        sys_cb.discon_reason = 0xff;//同步关机
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;
    }

    else if (memcmp((char *)ptr, "fixed_addr", strlen("fixed_addr")) == 0)
    {
        u8 fixed_addr[6];

        bt_get_fixed_bd_addr(&fixed_addr[0]);
        sprintf(spp_tx_buffer, "+fixed_addr:%02x%02x%02x%02x%02x%02x", fixed_addr[0], fixed_addr[1], fixed_addr[2], fixed_addr[3], fixed_addr[4], fixed_addr[5]);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

    else if (memcmp((char *)ptr, "master_addr", strlen("master_addr")) == 0)
    {
        u8 bt_addr[6];

        memset(&bt_addr[0], 0, 6);
        bt_get_master_addr(&bt_addr[0]);
        sprintf(spp_tx_buffer, "+master_addr:%02x%02x%02x%02x%02x%02x", bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3], bt_addr[4], bt_addr[5]);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

    else if (memcmp((char *)ptr, "qr_addr", strlen("qr_addr")) == 0)
    {
        u8 bt_addr[6];

        memset(&bt_addr[0], 0, 6);
        bt_get_qr_addr(&bt_addr[0]);
        sprintf(spp_tx_buffer, "+qr_addr:%02x%02x%02x%02x%02x%02x", bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3], bt_addr[4], bt_addr[5]);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

    else if (memcmp((char *)ptr, "xcfg_addr", strlen("xcfg_addr")) == 0)
    {
        u8 bt_addr[6];

        memcpy(bt_addr, xcfg_cb.bt_addr, 6);
        bt_get_qr_addr(&bt_addr[0]);
        sprintf(spp_tx_buffer, "+xcfg_addr:%02x%02x%02x%02x%02x%02x", bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3], bt_addr[4], bt_addr[5]);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

    else if (memcmp((char *)ptr, "clear_all", strlen("clear_all")) == 0)
    {
        sprintf(spp_tx_buffer, "+clear_all");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
      //  app_lr_send_notification(LR_NOTIFY_CLEAR_TWS_AG_INFO, 0, NULL);
        delay_5ms(100);
        bt_nor_unpair_device();
        bt_tws_unpair_device();
        bt_clr_master_addr();
    }

    else if (memcmp((char *)ptr, "factory_reset", strlen("factory_reset")) == 0)
    {
        sprintf(spp_tx_buffer, "+factory_reset:null");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        //ab_mate_device_reset();
        // ab_mate_app.do_flag |= FLAG_FACTORY_RESET;
        // ab_mate_tws_factory_reset_info_sync();
    }
#if USER_NTC
    else if (memcmp((char *)ptr, "ntc", strlen("ntc")) == 0)
    {
         ntc_gpio_power_supply();
         sprintf(spp_tx_buffer, "+ntc:%d", saradc_get_value8(ADCCH_NTC));
         bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }
#endif
    else if (memcmp((char *)ptr, "channel", strlen("channel")) == 0)
    {
        sprintf(spp_tx_buffer, "+channel:%d", sys_cb.tws_left_channel);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }
#if ANC_EN
    else if (memcmp((char *)ptr, "anc0", strlen("anc0")) == 0)
    {
        sprintf(spp_tx_buffer, "+anc0");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        bsp_res_play(TWS_RES_SW_ANC_OFF);
       // sys_cb.flag_online_test_ongoing = true;
    }

    else if (memcmp((char *)ptr, "anc1", strlen("anc1")) == 0)
    {
        sprintf(spp_tx_buffer, "+anc1");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        bsp_res_play(TWS_RES_SW_ANC_ON);
      //  sys_cb.flag_online_test_ongoing = true;
    }
    else if (memcmp((char *)ptr, "anc2", strlen("anc2")) == 0)
    {
        sprintf(spp_tx_buffer, "+anc2");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        bsp_res_play(TWS_RES_ANC_OPEN);
      //  sys_cb.flag_online_test_ongoing = true;
    }
    else if (memcmp((char *)ptr, "anc_status", strlen("anc_status")) == 0)
    {
        sprintf(spp_tx_buffer, "+anc_status:user[%d]", sys_cb.anc_user_mode);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
    }

#endif

    else if (memcmp((char *)ptr, "dut", strlen("dut")) == 0)
    {
        sprintf(spp_tx_buffer, "+dut");
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));
        delay_5ms(100);
        if(func_cb.sta != FUNC_BT_DUT)
        {
            func_cb.sta = FUNC_BT_DUT;
            sys_cb.discon_reason = 0;
        }
    }



    else if (memcmp((char *)ptr, "chip", strlen("chip")) == 0)
    {
        int chip_ver = VERSIONID;
        // uint rtccon3 = RTCCON3 & ~BIT(11);
        sprintf(spp_tx_buffer, "+chip:%d",chip_ver);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tx_buffer, strlen(spp_tx_buffer));

        qtest_cb.pdn_boat_flag = 1;
        // rtccon3 &= ~(BIT(14) | BIT(12) | BIT(10) | BIT(9) | BIT(8));      //touch key long press, inbox, wk pin, RTC one second, RTC alarm wakeup disable
        // rtccon3 |= BIT(11);
        sys_cb.discon_reason = 0;//不同步关耳机
        // sfunc_pwrdown(1);


        func_cb.sta = FUNC_PWROFF;
    }



}
#endif

AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();
    vusb_reset_clr();

#if VBAT_DETECT_EN
    lowpower_vbat_process();
#endif // VBAT_DETECT_EN

#if QTEST_EN
    if(QTEST_IS_ENABLE()) {
        qtest_process();
    }
#endif

#if BT_BACKSTAGE_EN
    if (func_cb.sta != FUNC_BT) {
        func_bt_warning();
        uint status = bt_get_status();
#if BT_BACKSTAGE_PLAY_DETECT_EN
        if (status >= BT_STA_PLAYING) {
#else
        if (status > BT_STA_PLAYING) {
#endif
            func_cb.sta_break = func_cb.sta;
            func_cb.sta = FUNC_BT;
        }
    }
#endif

#if PWRKEY_2_HW_PWRON
    //PWRKEY模拟硬开关关机处理
    if (sys_cb.pwrdwn_hw_flag) {
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;
        sys_cb.pwrdwn_hw_flag = 0;
    }
#endif


#if !SWETZ
#if USER_NTC
    if (sys_cb.ntc_2_pwrdwn_flag) {
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;
        sys_cb.ntc_2_pwrdwn_flag = 0;
    }
#endif
#endif

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_process();
    }
#endif // CHARGE_EN

#if SYS_KARAOK_EN
    karaok_process();
#endif

#if ANC_ALG_EN
    anc_alg_process();
#endif // ANC_ALG_EN

#if ANC_TOOL_EN
    anc_tool_process();
#endif // ANC_TOOL_EN

#if BT_MUSIC_EFFECT_EN
    music_effect_func_process();
#endif // BT_MUSIC_EFFECT_EN

#if ANC_MAX_VOL_DIS_FB_EN
    bsp_anc_max_vol_dis_fb_process();
#endif // ANC_MAX_VOL_DIS_FB_EN

    if(f_bt.bt_is_inited) {
        bt_thread_check_trigger();
        bsp_res_process();
        app_process();
        le_popup_process();
    }

    gsensor_process();

    sys_run_loop();
}





#if SWETZ_CHECK_INCASE
static void lr_notify_in_case_info(void)
{
    u8 in_case = sys_cb.flag_local_in_case;
    app_lr_send_notification(LR_NOTIFY_IN_CASE_STATUS, 1, &in_case);
}
#endif
#if BT_TWS_DBG_EN
extern u8 tws_dbg_ind[24];
#endif

#if SWETZ_SLAVE_FIND_DEV
extern soft_timer_p device_find_timer;
#endif
#if SWETZ_CHANGE_BLE
static u8 mark;
#endif
//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    switch (msg) {
#if BT_TWS_EN
        case EVT_BT_UPDATE_STA:
            f_bt.disp_update = 1;    //刷新显示
            break;
#endif
        case KL_NEXT_VOL_UP:
        case KH_NEXT_VOL_UP:
        case KL_PREV_VOL_UP:
        case KH_PREV_VOL_UP:
        case KL_VOL_UP:
        case KH_VOL_UP:
        case KU_VOL_UP_NEXT:
        case KU_VOL_UP_PREV:
        case KU_VOL_UP:
        case KU_VOL_UP_DOWN:
            func_volume_up();
            break;

        case KLU_VOL_UP:
        case KLU_NEXT_VOL_UP:
        case KL_PREV_VOL_DOWN:
        case KH_PREV_VOL_DOWN:
        case KL_NEXT_VOL_DOWN:
        case KH_NEXT_VOL_DOWN:
        case KU_VOL_DOWN_PREV:
        case KU_VOL_DOWN_NEXT:
        case KU_VOL_DOWN:
        case KL_VOL_DOWN:
        case KH_VOL_DOWN:
        case KL_VOL_UP_DOWN:
        case KH_VOL_UP_DOWN:
            func_volume_down();
            break;

        //长按PP/POWER软关机(通过PWROFF_PRESS_TIME控制长按时间)
        case KLH_POWER:
        case KLH_MODE_PWR:
        case KLH_PLAY_PWR_USER_DEF:
            if(xcfg_cb.bt_tswi_kpwr_en) {       //按键关机是否主从切换
                if(sys_cb.discon_reason == 0xff) {
                    sys_cb.discon_reason = 0;   //不同步关机
                }
            }
            sys_cb.pwrdwn_tone_en = 1;
            func_cb.sta = FUNC_PWROFF;
            break;
        case KU_MODE:
        case KU_MODE_PWR:
            func_cb.sta = FUNC_NULL;
            break;

#if EQ_MODE_EN
        case KU_EQ:
            sys_set_eq();
            break;
#endif // EQ_MODE_EN

        case KU_MUTE:
            // if (sys_cb.mute) {
            //     bsp_sys_unmute();
            // } else {
            //     bsp_sys_mute();
            // }
            break;

#if SYS_KARAOK_EN
        case KU_VOICE_RM:
            karaok_voice_rm_switch();
            break;
#if SYS_MAGIC_VOICE_EN || HIFI4_PITCH_SHIFT_EN
        case KL_VOICE_RM:
            magic_voice_switch();
            break;
#endif
#endif

#if ANC_EN
        case KU_ANC:
        case KD_ANC:
        case KL_ANC:
            sys_cb.anc_user_mode++;
            if (sys_cb.anc_user_mode > 2) {
                sys_cb.anc_user_mode = 0;
            }
            printf("sys_cb.anc_user_mode++\r\n");
            bsp_anc_set_mode(sys_cb.anc_user_mode);
            break;
#endif

        case MSG_SYS_500MS:
    sys_cb.local_bat_level = bsp_get_bat_level();
    app_lr_send_notification(LR_NOTIFY_BATTERY_LEVEL, 1, &sys_cb.local_bat_level);
    
   // printf("500ms sys_cb.local_bat_level:%d  sys_cb.peer_bat_level:%d\r\n",sys_cb.local_bat_level,sys_cb.peer_bat_level);
            break;



#if SWETZ_EVT_5S
        case EVT_SYS_5S:
#if USER_NTC
        sys_check_ntc();
      //  ntc_gpio_power_down();
        msg_queue_detach(EVT_SYS_5S,0);//避免消息队列长时间堵的情况下，连续检查NTC时NTC没有供电的问题
#endif


        printf("disp status %d,tws conn %d,tws slave %d, nor conn %d, scan %d,  ancmode:%d sys_cb.vol:%d\n\r",
        f_bt.disp_status,bt_tws_is_connected(),bt_tws_is_slave(),bt_nor_is_connected(), bt_get_scan(),
        sys_cb.anc_user_mode,sys_cb.vol);
        //app_priv_ble_notify_bt_status_and_vol(f_bt.disp_status, sys_cb.vol, sys_cb.hfp_vol);
       if (!bt_tws_is_slave()){
        u8 bt_addr[6];
        char name[32];
        u8 disc_index = 0;
        bt_get_link_btaddr(disc_index, bt_addr);
        bt_nor_get_link_info_name(bt_addr, name, 32);
        printf("disc device:%s\n",name);
       }




      //  printf("NTC val:%d  ntc_thd_val:%d \r\n",saradc_get_value8(ADCCH_NTC),xcfg_cb.ntc_thd_val);
        sys_cb.local_bat_level= bsp_get_bat_level();
        printf("sys_cb.vbat:%d local_bat_level:%d  charge_sta:%d \r\n",sys_cb.vbat,sys_cb.local_bat_level,sys_cb.charge_sta);
#if TWS_LR
       // u8 bat_level = bsp_get_bat_level();
        app_lr_send_notification(LR_NOTIFY_BATTERY_LEVEL, 1, &sys_cb.local_bat_level);
        printf("                  peer_bat_level:%d\r\n",sys_cb.peer_bat_level);
#endif

#if SWETZ_SWITCH_BY_BAT
        app_role_switch_by_bat();
#endif

    uint8_t mac_buf[6];
    bt_get_local_bd_addr(mac_buf);
    printf("mac_buf:%x %x %x %x %x %x \r\n",mac_buf[0],mac_buf[1],mac_buf[2],mac_buf[3],mac_buf[4],mac_buf[5]);
     //   u8 bt_addr[6];

    //     memset(&bt_addr[0], 0, 6);
    //     bt_get_master_addr(&bt_addr[0]);
    //     printf("mac_buf:%x %x %x %x %x %x \r\n",bt_addr[0],bt_addr[1],bt_addr[2],bt_addr[3],bt_addr[4],bt_addr[5]);
#if !SWETZ_APP_TEST
       // printf("ab_mate_app.poweroff_time:%d\r\n",ab_mate_app.poweroff_time);
       printf("ab_mate_app.local_key.key_short:%d\r\n",ab_mate_app.local_key.key_short);
       printf("ab_mate_app.local_key.key_double:%d\r\n",ab_mate_app.local_key.key_double);
       printf("ab_mate_app.local_key.key_three:%d\r\n",ab_mate_app.local_key.key_three);
       printf("ab_mate_app.local_key.key_four:%d\r\n",ab_mate_app.local_key.key_four);
       printf("ab_mate_app.local_key.key_long:%d\r\n",ab_mate_app.local_key.key_long);
       //printf("get_wav_res_digvol:%d\r\n",get_wav_res_digvol());
       printf("ab_mate_app.eq_info.mode:%d\r\n",ab_mate_app.eq_info.mode);
       printf("ab_mate_app.auto_answer_type:%d\r\n",ab_mate_app.auto_answer_type);
       printf("ab_mate_app.mult_dev.en:%d\r\n",ab_mate_app.mult_dev.en);
#endif
       // printf("ab_mate_app.mult_dev.en:%d  cfg_bt_max_acl_link:%d \r\n",ab_mate_app.mult_dev.en,cfg_bt_max_acl_link);
        //printf("ble_adv0_get_adv_en:%d\r\n",ble_adv0_get_adv_en());
       // printf("ab_mate_app.auto_answer_type:%d\r\n",ab_mate_app.auto_answer_type);
        
if(!bt_tws_is_slave()){
             sys_cb.bt_master_status = bt_get_disp_status();
             app_lr_send_notification(LR_NOTIFY_BT_STATUS, 1, &sys_cb.bt_master_status);
        }
      //  printf("sys_cb.bt_master_status:%d\r\n",sys_cb.bt_master_status);
        
        // printf("ab_mate_app.find_type:%d\r\n",ab_mate_app.find_type);  
        // printf("sys_cb.reset_Not_tone:%d\r\n",sys_cb.reset_Not_tone); 
        // printf("sys_cb.flag_local_in_case:%d  sys_cb.flag_peer_in_case:%d \r\n",sys_cb.flag_local_in_case,sys_cb.flag_peer_in_case);

        // printf("sys_cb.time_power_def_5s:%d tiem_3s:%d tiem_5s:%d \r\n",sys_cb.time_power_def_5s,sys_cb.tiem_3s,sys_cb.tiem_5s);
        // printf("bt_nor_get_link_info:%d\r\n",bt_nor_get_link_info(NULL));
        // printf("ble_is_connect:%d\r\n",ble_is_connect());
        
        // printf("ble_get_status:%d\r\n",ble_get_status());


#if SWETZ_TEST
        // bt_is_scan_ctrl();
        bt_set_scan(0x03);
        
        printf("5S_bt_get_scan:0x%2x bt_is_scan_ctrl:%d\r\n",bt_get_curr_scan(),bt_is_scan_ctrl());
#endif
            break;
#endif

    case EVT_SYS_1S:
            printf("==========================================bsp_get_mute_sta:%d\r\n",bsp_get_mute_sta());
            printf("==========================================CHARGE_INBOX:%d\r\n",CHARGE_INBOX());
            #if BLE_POPU_TEST
#if SWETZ_TEST_1
     printf("bt_get_scan:%2x f_bt.disp_status:%d\r\n",bt_get_scan(),f_bt.disp_status);
#endif
#if SWETZ_BLE
        if(!bt_tws_is_slave()){
            if(!ble_is_connect() && f_bt.disp_status >= BT_STA_INCOMING && (!ble_get_status())){
#if SWETZ_CHANGE_BLE                
                ab_mate_update_ble_adv_bt_call_sta(1,1);
#endif
                btstack_ble_adv_en();
                mark = 1;
                printf("btstack_ble_adv_en\r\n");
            }
#if  SWETZ_BLE   
        if(f_bt.disp_status < BT_STA_INCOMING && (!bt_tws_is_slave()) && mark == 1){
                ab_mate_update_ble_adv_bt_call_sta(0,1);
                mark = 0;
        }
#endif
            
        }
#endif
                update_broadcast_mode();
#if BT_TWS_DBG_EN
    // u8 *ind = tws_dbg_ind + 3;
    // s8 *rssi = (s8 *)(ind + 8);

    // if(sys_cb.tws_left_channel){
    //         ind[19] = 1;   //0=right, 1=left
    //         bt_tws_get_link_rssi(rssi, 1);
    // printf("rssi_left_tws:%d  rssi_right_tws:%d rssi_left_phone:%d rssi_right_phone:%d \r\n",(s8)tws_dbg_ind[11],(s8)tws_dbg_ind[12],(s8)tws_dbg_ind[13],(s8)tws_dbg_ind[14]);
    // printf("per_left_tws:%d  per_right_tws:%d per_left_phone:%d per_right_phone:%d \r\n",(s8)tws_dbg_ind[15],(s8)tws_dbg_ind[16],(s8)tws_dbg_ind[17],(s8)tws_dbg_ind[18]);
    // }else {
    //         ind[19] = 0;   //0=right, 1=left
    //         bt_tws_get_link_rssi(rssi, 0);
    // printf("rssi_left_tws:%d  rssi_right_tws:%d rssi_left_phone:%d rssi_right_phone:%d \r\n",(s8)tws_dbg_ind[11],(s8)tws_dbg_ind[12],(s8)tws_dbg_ind[13],(s8)tws_dbg_ind[14]);
    // printf("per_left_tws:%d  per_right_tws:%d per_left_phone:%d per_right_phone:%d \r\n",(s8)tws_dbg_ind[15],(s8)tws_dbg_ind[16],(s8)tws_dbg_ind[17],(s8)tws_dbg_ind[18]);        
    // }
   


#endif
               // printf("=======sys_cb.vol:%d\r\n",sys_cb.vol);
            #endif
            break;
#if SWETZ_SWITCH_BY_BAT
        case EVT_INBOX_CHANGED:
            app_role_switch_by_inbox();
            break;
#endif

#if SWETZ_SPP_CMD
        case EVT_SPP_AT_CMD:
            spp_at_cmd_process();

            break;
#endif




#if SWETZ_CHECK_INCASE
        case EVT_IN_CASE:
#if SWETZ_TIME_3S
            sys_cb.tiem_3s = 0;
#endif
            lr_notify_in_case_info();
            printf("         in case\r\n");
            
#if SWETZ_SWITCH_BY_BAT
            msg_enqueue(EVT_INBOX_CHANGED);
#endif

#if SWETZ_INCASE_AUTO_ANCOFF

           // msg_enqueue(EVT_AUTO_ANCOFF);
#endif

#endif


            break;

        case EVT_OUT_CASE:
            lr_notify_in_case_info();
            printf("         out case\r\n");
#if SWETZ_SWITCH_BY_BAT
          //  msg_enqueue(EVT_INBOX_CHANGED);
#endif
#if SWETZ_INCASE_AUTO_ANCOFF
          //  msg_enqueue(EVT_AUTO_ANCON);
#endif
#if SWETZ_TIME_3S
            sys_cb.tiem_3s = 0;
#endif
            break;


#if SWETZ_INCASE_AUTO_ANCOFF
            case EVT_AUTO_ANCOFF:
         //   memory_save_flag(PARAM_ANC_SWITCH_ADDR,sys_cb.anc_user_mode);
            // if(!bt_tws_is_slave()){
                bsp_anc_set_mode(0);
            if(!bt_tws_is_slave()){
                     bt_ctl_nr_sta_change();                         //发消息通知手机
            }


            //     app_lr_send_notification(LR_NOTIFY_SLAVE_SYNC_INFO,1,&sys_cb.anc_user_mode);
            // }else {

            //     app_lr_send_msg(EVT_AUTO_ANCOFF);

            // }


            break;
#endif

#if SWETZ_INCASE_AUTO_ANCOFF
            case EVT_AUTO_ANCON:
           // sys_cb.anc_user_mode = memory_read_flag(PARAM_ANC_SWITCH_ADDR);
            // printf("power on anc_user_mode:%d\r\n",sys_cb.anc_user_mode);
            // if(sys_cb.anc_user_mode == APP_ANC_START || sys_cb.anc_user_mode == APP_ANC_TRANSPARENCY){
            //         bsp_anc_set_mode(sys_cb.anc_user_mode);
            // }
            break;
#endif

#if MUSIC_UDISK_EN
        case EVT_UDISK_INSERT:
            if (dev_is_online(DEV_UDISK)) {
                if (dev_udisk_activation_try(0)) {
                    sys_cb.cur_dev = DEV_UDISK;
                    func_cb.sta = FUNC_MUSIC;
                }
            }
            break;
#endif // MUSIC_UDISK_EN

#if MUSIC_SDCARD_EN
        case EVT_SD_INSERT:
            if (dev_is_online(DEV_SDCARD)) {
                sys_cb.cur_dev = DEV_SDCARD;
                func_cb.sta = FUNC_MUSIC;
            }
            break;
#endif // MUSIC_SDCARD_EN

#if FUNC_USBDEV_EN
        case EVT_PC_INSERT:
            if (dev_is_online(DEV_USBPC)) {
                func_cb.sta = FUNC_USBDEV;
            }
            break;

        case EVT_UDE_SET_VOL:
            printf("ude set db: %d\n", sys_cb.db_level);
            bsp_change_volume_db(sys_cb.db_level);
            break;
#endif // FUNC_USBDEV_EN

#if LINEIN_DETECT_EN
        case EVT_LINEIN_INSERT:
            if (dev_is_online(DEV_LINEIN)) {
#if LINEIN_2_PWRDOWN_EN
                sys_cb.pwrdwn_tone_en = LINEIN_2_PWRDOWN_TONE_EN;
                func_cb.sta = FUNC_PWROFF;
#else
                func_cb.sta = FUNC_AUX;
#endif // LINEIN_2_PWRDOWN_EN
            }
            break;
#endif // LINEIN_DETECT_EN

        case EVT_A2DP_SET_VOL:
        case EVT_TWS_INIT_VOL:
            if((sys_cb.incall_flag & INCALL_FLAG_SCO) == 0) {
                if(!bsp_res_is_vol_busy()) {
                    bsp_change_volume(sys_cb.vol);
                }
            }
            //no break
        case EVT_TWS_SET_VOL:
            if (sys_cb.incall_flag == 0) {
                printf("SET_VOL: %d\n", sys_cb.vol);


                if(msg != EVT_TWS_INIT_VOL) {   //TWS同步音量，不需要显示
                    gui_box_show_vol();
                }
            }
            //no break
        case EVT_DEV_SAVE_VOL:
            sys_cb.cm_times = 0;
            sys_cb.cm_vol_change = 1;
            break;

#if BT_MUSIC_EFFECT_SOFT_VOL_EN
        case EVT_SOFT_VOL_SET:
            bsp_change_volume(sys_cb.vol);
            break;
#endif // BT_MUSIC_EFFECT_SOFT_VOL_EN
#if ABP_EN
        case EVT_ABP_SET:
            if (sys_cb.abp_mode == 0) {
                abp_stop();
            } else {
                abp_start(sys_cb.abp_mode - 1);
            }
            break;
#endif // ABP_EN

        case EVT_BT_SCAN_START:
            if (bt_get_status() < BT_STA_SCANNING) {
                //printf("bt_scan_enable\r\n");
                bt_scan_enable();
            }
            break;
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
        case EVT_ONLINE_SET_EQ:
            bsp_eq_parse_cmd();
            break;
#endif
#if ANC_EN
        case EVT_ONLINE_SET_ANC:
            bsp_anc_parse_cmd();
            break;
#endif

#if ENC_DBG_EN
        case EVT_ONLINE_SET_ENC:
            bsp_enc_parse_cmd();
            break;
#endif

#if BT_MUSIC_EFFECT_DBG_EN
        case EVT_ONLINE_SET_EFFECT:
            bsp_effect_parse_cmd();
            break;
#endif

#if ANC_ALG_DBG_EN
        case EVT_ONLINE_SET_ANC_ALG:
            bsp_anc_alg_parse_cmd();
            break;
#endif // ANC_ALG_DBG_EN

#if ANC_TOOL_EN
        case EVT_ONLINE_SET_ANC_TOOL:
            bsp_anc_tool_parse_cmd();
            break;
#endif // ANC_TOOL_EN

#if SYS_KARAOK_EN
        case EVT_ECHO_LEVEL:
//            printf("echo level:%x\n", sys_cb.echo_level);
    #if SYS_ECHO_EN
            bsp_echo_set_level();
    #endif
            break;

        case EVT_MIC_VOL:
//            printf("mic vol:%x\n", sys_cb.mic_vol);
            bsp_karaok_set_mic_volume();
            break;

        case EVT_MUSIC_VOL:
//            printf("music vol:%x\n", sys_cb.music_vol);
            bsp_karaok_set_music_volume();
            break;
#endif
#if LANG_SELECT == LANG_EN_ZH
        case EVT_BT_SET_LANG_ID:
            param_lang_id_write();
            param_sync();
            break;
#endif

#if EQ_MODE_EN
        case EVT_BT_SET_EQ:
            music_set_eq_by_num(sys_cb.eq_mode);
            break;
#endif

#if CHARGE_BOX_EN && (UART0_PRINTF_SEL != PRINTF_VUSB)
        //耳机入仓关机
        case EVT_CHARGE_INBOX:
            printf("EVT_CHARGE_INBOX\r\n");
            if(sys_cb.discon_reason == 0xff) {
                sys_cb.discon_reason = 0;   //不同步关机
            }
            sys_cb.pwrdwn_tone_en = 0;
            charge_box_inbox_wakeup_enable();
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

#if QTEST_EN
        case EVT_QTEST_PICKUP_PWROFF:
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

        case EVT_HFP_SET_VOL:
            if(sys_cb.incall_flag & INCALL_FLAG_SCO){
                bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
                sys_cb.cm_times = 0;
                sys_cb.cm_vol_change = 1;
            }
            break;
#ifdef KEY_TONE
        case EVT_KEY_PRESS:
                printf("EVT_KEY_PRESS\r\n");
                wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
            break;
#endif

#ifdef ANC_SWITCH
        case EVT_KEY_ANC_SWITCH:
        if(!bt_tws_is_slave()){
            printf("sys_cb.anc_user_mode:%d\r\n",sys_cb.anc_user_mode);
            if (sys_cb.anc_user_mode == APP_ANC_STOP)
            {
                bsp_res_play(TWS_RES_SW_ANC_ON);
            }
            else if (sys_cb.anc_user_mode == APP_ANC_START)
            {
                bsp_res_play(TWS_RES_ANC_OPEN);
            }
            else if (sys_cb.anc_user_mode == APP_ANC_TRANSPARENCY)
            {
                bsp_res_play(TWS_RES_SW_ANC_OFF);
            }
            else
            {
                bsp_res_play(TWS_RES_SW_ANC_ON);
            }


        }else {
                        app_lr_send_msg(EVT_KEY_ANC_SWITCH);
        }

            break;
#endif

#if SWETZ_SLAVE_FIND_DEV
        case EVT_FIND_DEV:
                printf("45255-------------------\r\n");
                ab_mate_app.find_type = 1;
                ab_mate_app.device_find = 1;
                soft_timer_start(device_find_timer);
            break;
#endif  

#if DAC_DNC_EN
        case EVT_DNC_START:
            if (bsp_res_is_playing()) {
                msg_enqueue(EVT_DNC_START);
            } else {
                #if ANC_EN
                if (sys_cb.anc_user_mode == 0)
                #endif
                {
                    dac_dnc_start();
                }
            }
            break;

        case EVT_DNC_STOP:
            if (bsp_res_is_playing()) {
                msg_enqueue(EVT_DNC_STOP);
            } else {
                dac_dnc_stop();
            }
            break;
#endif

#if IODM_TEST_EN
        case EVT_IODM_TEST:
            iodm_reveice_data_deal();
            break;
#endif

#if ASR_EN
        case EVT_ASR_START:
            if (sys_cb.asr_enable) {
                bsp_asr_start();
            }
            break;

        case EVT_ASR_STOP:
            bsp_asr_stop();
            break;
#endif
#if OPUS_ENC_EN
       case EVT_ENCODE_STOP:
            bsp_opus_encode_stop();
            break;
#endif
#if BT_HID_DOUYIN_EN
        case EVT_HID_SLIDE_UP:
            printf("user_finger_up\n");
            user_finger_up();
            break;
        case EVT_HID_SLIDE_DOWN:
            printf("user_finger_down\n");
            user_finger_down();
            break;
        case EVT_HID_P:
            printf("user_finger_p\n");
            user_finger_p();
            break;
        case EVT_HID_PP:
            printf("user_finger_pp\n");
            user_finger_pp();
            break;
        case EVT_HID_TAKE_PIC:
            printf("bsp_bt_hid_photo\n");
            bsp_bt_hid_photo(HID_KEY_VOL_UP);
            break;
#endif

#if (BT_MAP_EN || BT_HFP_TIME_EN)
        case EVT_BT_MAP_DONE:
        case EVT_HFP_TIME_DONE:
            ab_mate_time_info_notify(CMD_TIME_LOCALTIME);
            break;
#endif
    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        param_hfp_vol_write();
        param_sys_vol_write();
        cm_sync();
    }

#if SD_SOFT_DETECT_EN
    sd_soft_cmd_detect(120);
#endif




}

#if SWETZ_INCASE_MUTE
AT(.text.func.msg)
void app_check_mute(void)
{
    static u32 tick = 0;
    static uint8_t cnt = 0;
    static uint8_t cnt2 = 0;
    if (CHARGE_INBOX())
    {
        cnt++;
        if(cnt >= 3){
               cnt = 0;
            if (!sys_cb.mute)
            {
                bsp_sys_mute();
            }

         }

    }
    else
    {
        cnt2++;
        if(cnt2 >= 3){
            cnt2 = 0;
            if (sys_cb.mute)
            {
                bsp_sys_unmute();
            }

       }

    }


    // if (tick_check_expire(tick, 5000)) {
    //     tick = tick_get();
    //     cnt = 0;
    // }

}
#endif

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
    if (sys_cb.cm_vol_change) {
        sys_cb.cm_vol_change = 0;
        param_sys_vol_write();
    }
    param_sync();
    gui_box_clear();
    reset_sleep_delay();
    reset_pwroff_delay();
    func_cb.mp3_res_play = NULL;
    func_cb.set_vol_callback = NULL;
    bsp_clr_mute_sta();
    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
#if SYS_KARAOK_EN
    karaok_voice_rm_disable();
    bsp_karaok_echo_reset_buf(func_cb.sta);
#endif
}

AT(.text.func)
void func_exit(void)
{
    u8 func_num;
    u8 funcs_total = get_funcs_total();

    for (func_num = 0; func_num != funcs_total; func_num++) {
        if (func_cb.last == func_sort_table[func_num]) {
            break;
        }
    }
    func_num++;                                     //切换到下一个任务
    if (func_num >= funcs_total) {
        func_num = 0;
    }
    func_cb.sta = func_sort_table[func_num];        //新的任务
#if SYS_MODE_BREAKPOINT_EN
    param_sys_mode_write(func_cb.sta);
#endif // SYS_MODE_BREAKPOINT_EN
}

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);

    func_bt_chk_off();
    while (1) {
        func_enter();
        switch (func_cb.sta) {
#if FUNC_MUSIC_EN
        case FUNC_MUSIC:
            func_music();
            break;
#endif // FUNC_MUSIC_EN

#if FUNC_CLOCK_EN
        case FUNC_CLOCK:
            func_clock();
            break;
#endif // FUNC_CLOCK_EN

#if FUNC_BT_EN
        case FUNC_BT:
            func_bt();
            break;
#endif

#if FUNC_BTHID_EN
        case FUNC_BTHID:
            func_bthid();
            break;
#endif // FUNC_BTHID_EN

#if FUNC_AUX_EN
        case FUNC_AUX:
            func_aux();
            break;
#endif // FUNC_AUX_EN

#if FUNC_USBDEV_EN
        case FUNC_USBDEV:
            func_usbdev();
            break;
#endif

#if FUNC_SPEAKER_EN
        case FUNC_SPEAKER:
            func_speaker();
            break;
#endif // FUNC_SPEAKER_EN

#if FUNC_IDLE_EN
        case FUNC_IDLE:
            func_idle();
            break;
#endif // FUNC_IDLE_EN

#if FUNC_BT_DUT_EN
        case FUNC_BT_DUT:
            func_bt_dut();
            break;
#endif
#if FUNC_BT_FCC_EN
        case FUNC_BT_FCC:
            func_bt_fcc();
            break;
#endif
#if IODM_TEST_EN
        case FUNC_BT_IODM:
            func_bt_iodm();
            break;
#endif

        case FUNC_CHARGE:
            func_charge();
            break;

        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwrdwn_tone_en);
            break;

        default:
            func_exit();
            break;
        }
    }
}
