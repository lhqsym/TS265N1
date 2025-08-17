#include "include.h"
#include "func.h"
#include "func_bt.h"

u16 get_user_def_vol_msg(u8 func_sel)
{
    u16 msg = NO_MSG;

    if(func_sel == UDK_VOL_UP) {            //VOL+
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? KU_VOL_UP : KU_VOL_DOWN;
        } else
#endif
        {
            msg = KU_VOL_UP;
        }
    } else if(func_sel == UDK_VOL_DOWN) {   //VOL-
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? KU_VOL_DOWN : KU_VOL_UP;
        }else
#endif
        {
            msg = KU_VOL_DOWN;
        }
    }
    return msg;
}

void user_def_track_msg(u16 msg)
{
    if (msg == KU_PREV) {
        bt_music_prev();
    } else {
        bt_music_next();
    }
}

#ifdef KEY_LSISR_RSWITCHMODE

void user_def_sisrORswitchmode_msg(u16 msg)
{
    if(msg == UDK_SIRI){
        bt_hfp_siri_switch();

    }else {
        // bool low_latency = bt_is_low_latency();
        // if (low_latency) {
        //     bsp_res_play(TWS_RES_SW_MUSIC_MODE);
        // } else {
        //     bsp_res_play(TWS_RES_SW_GAME_MODE);
        // }
        #ifdef ANC_SWITCH
                msg_enqueue(EVT_KEY_ANC_SWITCH);
        #endif
        bt_ctl_nr_sta_change();                         //发消息通知手机
    }


}

#endif




///检查USER_DEF按键配置功能
bool user_def_func_is_ready(u8 func_sel)
{
    if (func_sel == UDK_NONE) {
        return false;
    }
    if ((func_sel <= UDK_NEXT) || (func_sel == UDK_PLAY_PAUSE)) {
        if ((f_bt.disp_status != BT_STA_CONNECTED) && (f_bt.disp_status != BT_STA_PLAYING)) {
            return false;
        }
    }
    return true;
}

///检查USER_DEF按键消息处理
bool user_def_key_msg(u8 func_sel)
{

#if SWETZ_TIME_3S
    if(sys_cb.tiem_3s != 4){
            printf("sys_cb.tiem:%d\r\n",sys_cb.tiem_3s);
            return NO_MSG;
    }

    
#endif

    u16 msg = NO_MSG;
    printf("def_key_func_sel:%d\r\n",func_sel);

#if SWETZ_DU_TEST
    if(bt_nor_is_connected()){
         if(func_sel == UDK_PREV || func_sel == UDK_NEXT || func_sel == UDK_SIRI|| func_sel == UDK_VOL_UP || func_sel == UDK_VOL_DOWN){
                wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
            }
    }
    if(func_sel == UDK_NR){
                wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
        }
    
#endif

#if SWETZ_CALL_STA_CLSOE_ANC
        if(bt_nor_is_connected() && f_bt.disp_status >= BT_STA_INCOMING){
                if(func_sel == UDK_NR){
                        return NO_MSG;
                }
        }
#endif

    if (!user_def_func_is_ready(func_sel)) {
        return false;
    }
    printf("TEST============\r\n");
    if (func_sel == UDK_REDIALING) {
        if(bsp_res_play(TWS_RES_REDIALING) == RES_ERR_INVALID) {
            bt_call_redial_last_number();               //回拨电话
        }
    } else if (func_sel == UDK_SIRI) {                  //SIRI
    printf("UDK_SIRI\r\n");
#ifdef KEY_LSISR_RSWITCHMODE
    if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
                msg = func_bt_tws_get_channel()? UDK_SIRI : UDK_NR;
}{

                user_def_sisrORswitchmode_msg(msg);
}
             

#else
    printf("UDK_SIRI\n\r");
    bt_hfp_siri_switch();
#endif
       
    } else if (func_sel == UDK_NR) {                    //NR
        printf("UDK_NR\r\n");
#ifdef ANC_SWITCH
 //   if(bt_tws_is_connected() == 1){
      
         msg_enqueue(EVT_KEY_ANC_SWITCH);
          bt_ctl_nr_sta_change();                         //发消息通知手机
   // }
      
#endif
        // bt_ctl_nr_sta_change();                         //发消息通知手机

    } else if (func_sel == UDK_PREV) {                  //PREV
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? KU_PREV : KU_NEXT;
        } else
#endif
        {

            msg = KU_PREV;
        }
        user_def_track_msg(msg);
    } else if (func_sel == UDK_NEXT) {                  //NEXT
#if BT_TWS_EN
        if(xcfg_cb.user_def_lr_en && xcfg_cb.bt_tws_en) {
            msg = func_bt_tws_get_channel()? KU_NEXT : KU_PREV;
        } else
#endif
        {
            msg = KU_NEXT;
        }
        user_def_track_msg(msg);
    } else if (func_sel == UDK_MODE) {                  //MODE
        func_message(KU_MODE);
    } else if (func_sel == UDK_PHOTO) {
        return bsp_bt_hid_photo(HID_KEY_VOL_UP);        //拍照
    } else if (func_sel == UDK_HOME) {
        return bt_hid_consumer(HID_KEY_IOS_HOME);       //IOS Home按键功能
    } else if (func_sel == UDK_LANG) {
        func_bt_switch_voice_lang();                    //中英文切换
    } else if (func_sel == UDK_PLAY_PAUSE) {
        bt_music_play_pause();
    } else if (func_sel == UDK_DUT) {                  //CBT 测试模式
        if(func_cb.sta != FUNC_BT_DUT){
            func_cb.sta = FUNC_BT_DUT;
            sys_cb.discon_reason = 0;
        }
    } else if (func_sel == UDK_LOW_LATENCY) {
        bool low_latency = bt_is_low_latency();
        if (low_latency) {
#if SWETZ_WARNING_TONE 
        bsp_res_play(TWS_RES_SW_MUSIC_MODE);
#else 
        bsp_res_play(TWS_RES_MUSIC_MODE);
#endif
            
        } else {
#if SWETZ_WARNING_TONE 
        bsp_res_play(TWS_RES_SW_GAME_MODE);
#else     
        bsp_res_play(TWS_RES_GAME_MODE);
#endif            
        }
    }else {                                            //VOL+, VOL-
        func_message(get_user_def_vol_msg(func_sel));
    }
    return true;
}

bool user_def_lkey_tone_is_enable(u8 func_sel)
{
    if (!xcfg_cb.user_def_kl_tone_en) {
        return false;
    }
    return user_def_func_is_ready(func_sel);
}

#if BT_HID_MANU_EN
//双击VOL-, VOL+的功能处理
void bt_hid_vol_msg(u8 sel)
{
    if (sel == 1) {
        bsp_bt_hid_photo(HID_KEY_VOL_UP);
    } else if (sel == 2) {
        bsp_bt_hid_photo(HID_KEY_VOL_DOWN);
    } else if (sel == 3) {
        bsp_bt_hid_tog_conn();
    }
}
#endif

bool bt_tws_pair_mode(u8 method)
{
#if BT_TWS_EN
    if ((xcfg_cb.bt_tws_en) && (xcfg_cb.bt_tws_pair_mode == method) && (!bt_nor_is_connected())) {
        if(bt_tws_is_connected()) {
            bt_tws_disconnect();
        } else {
            bt_tws_search_slave(15000);
        }
        return true;
    }
#endif
    return false;
}

AT(.text.func.bt.msg)
void func_bt_message_do(u16 msg)
{
    int klu_flag = 0;
   // printf("bt_message:%d\r\n,msg");
    switch (msg) {
    case KU_PLAY:
    case KU_PLAY_USER_DEF:
    case KU_PLAY_PWR_USER_DEF:
#if SWETZ
            user_def_key_msg(xcfg_cb.user_def_ks_sel);
#else
        if (!bt_nor_is_connected()) {
            bt_tws_pair_mode(3);           //单击PLAY按键手动配对
        } else {
        	user_def_key_msg(xcfg_cb.user_def_ks_sel);
        }
#endif

        
        break;

    case KL_PLAY_PWR_USER_DEF:
        klu_flag = 1;                                                       //长按抬键的时候呼SIRI
    case KL_PLAY_USER_DEF:
    
#if SWETZ_KL_PLAY_USER_DEF_DELAY5S
    if(sys_cb.time_power_def_5s != 5){
            printf("sys_cb.time_power_def_5s:%d\r\n",sys_cb.time_power_def_5s);
            break;
    }
#endif
        printf("-----44444444444444444444-----------55555555555555555555 klu_flag:%d\r\n",klu_flag);
        f_bt.user_kl_flag = 0;
        if (!bt_tws_pair_mode(4)) {                                         //是否长按配对功能
            if (user_def_lkey_tone_is_enable(xcfg_cb.user_def_kl_sel)) {
                bsp_piano_warning_play(WARNING_TONE, 2);                    //长按“滴”一声
            }
            if (klu_flag) {
                f_bt.user_kl_flag = user_def_func_is_ready(xcfg_cb.user_def_kl_sel);     //长按抬键的时候再处理
                printf("444455555\r\n");
            } else {
                printf("3333222\r\n");
                user_def_key_msg(xcfg_cb.user_def_kl_sel);
            }
        }
        break;

        //SIRI, NEXT, PREV在长按抬键的时候响应,避免关机前切歌或呼SIRI了
    case KLU_PLAY_PWR_USER_DEF:
        if (f_bt.user_kl_flag) {
            user_def_key_msg(xcfg_cb.user_def_kl_sel);
            f_bt.user_kl_flag = 0;
        }
        break;

    case KH_PLAY_USER_DEF:
        func_message(get_user_def_vol_msg(xcfg_cb.user_def_kl_sel));
        break;

    case KD_PLAY_USER_DEF:
    case KD_PLAY_PWR_USER_DEF:
        if (user_def_key_msg(xcfg_cb.user_def_kd_sel)) {
#if BT_TWS_EN
        } else if(bt_tws_pair_mode(2)) {
#endif
        } else if (xcfg_cb.user_def_kd_lang_en) {
            func_bt_switch_voice_lang();
        }
        break;

    ///三击按键处理
    case KTH_PLAY_USER_DEF:
    case KTH_PLAY_PWR_USER_DEF:
        user_def_key_msg(xcfg_cb.user_def_kt_sel);
        break;

    ///四击按键处理
    case KFO_PLAY_USER_DEF:
    case KFO_PLAY_PWR_USER_DEF:
        #if ASR_EN
        if(sys_cb.asr_enable) {
            bsp_res_play(TWS_RES_ASR_OFF);
        } else {
            bsp_res_play(TWS_RES_ASR_ON);
        }
        #else
        user_def_key_msg(xcfg_cb.user_def_kfour_sel);
        #endif
        break;

    ///五击按键处理
    case KFI_PLAY_USER_DEF:
    case KFI_PLAY_PWR_USER_DEF:
        if (xcfg_cb.user_def_kfive_sel) {
            user_def_key_msg(xcfg_cb.user_def_kfive_sel);
        }
        break;

    case KU_SIRI:
    case KL_SIRI:
    case KL_HOME:
        if (bt_nor_is_connected()) {
            bt_hfp_siri_switch();
        }
        break;

    case KU_HOME:
        bt_hid_consumer(HID_KEY_IOS_HOME);
        break;

//    case KL_PLAY:
//        if (xcfg_cb.bt_key_discon_en) {
//            bt_disconnect(0);
//            break;
//        }

    case KU_PREV_VOL_DOWN:
    case KU_PREV_VOL_UP:
    case KL_VOL_DOWN_PREV:
    case KL_VOL_UP_PREV:
    case KU_PREV:
        bt_music_prev();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;

    case KU_NEXT:
    case KU_NEXT_VOL_UP:
    case KU_NEXT_VOL_DOWN:
    case KL_VOL_UP_NEXT:
    case KL_VOL_DOWN_NEXT:
        bt_music_next();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;
    case KL_PREV:
        bt_music_rewind();
        break;
    case KLU_PREV:
        bsp_clr_mute_sta();
        bt_music_rewind_end();
        break;
    case KL_NEXT:
        bt_music_fast_forward();
        break;
    case KLU_NEXT:
        bsp_clr_mute_sta();
        bt_music_fast_forward_end();
        break;

    case KD_MODE:
    case KD_MODE_PWR:
        bt_tws_pair_mode(5);
        break;

    case KD_HSF:
        if (bt_nor_is_connected()) {
            if(bsp_res_play(TWS_RES_REDIALING) == RES_ERR_INVALID) {
                bt_call_redial_last_number();       //回拨电话
            }
        }
        break;

#if BT_HID_MANU_EN
    case KD_NEXT_VOL_UP:
    case KD_PREV_VOL_UP:
    case KD_VOL_UP_NEXT:
    case KD_VOL_UP_PREV:
    case KD_VOL_UP:
        bt_hid_vol_msg(3);
        break;

    case KD_PREV_VOL_DOWN:
    case KD_NEXT_VOL_DOWN:
    case KD_VOL_DOWN_PREV:
    case KD_VOL_DOWN_NEXT:
    case KD_VOL_DOWN:
        bt_hid_vol_msg(1);
        break;
#endif

    case MSG_SYS_1S:
        bt_hfp_report_bat();
#if BT_TWS_DBG_EN
        bt_tws_report_dgb();
#endif

#if SWETZ_INCASE_MUTE

    if(f_bt.disp_status >= BT_STA_INCOMING){
            if(bsp_get_mute_sta()){
                    bsp_clr_mute_sta();
                    bsp_sys_unmute();
            }
    }else {
            app_check_mute();
    }
        
#endif
        break;

    case EVT_A2DP_MUSIC_PLAY:
        if (!sbc_is_bypass()) {
            dac_fade_in();
        }
        if (f_bt.pp_2_unmute) {
            f_bt.pp_2_unmute = 0;
            bsp_clr_mute_sta();
        }
        break;

    case EVT_A2DP_MUSIC_STOP:
    case EVT_A2DP_SYNC_CLOSE:
        if (!sbc_is_bypass() && !bsp_res_is_playing()) {
            dac_fade_out();
        }
        break;

    case EVT_KEY_2_UNMUTE:
        bsp_clr_mute_sta();
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.bt.msg)
void func_bt_message(u16 msg)
{
    if(msg){
        func_bt_message_do(msg);
    }
}

#if SWETZ_APP_TEST
void udk_reject_call(void)
{
    if(ab_mate_app.local_key.key_short == 10){
        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
            printf("222222222222222222222222222222222\r\n");
            bt_call_terminate();    //拒接
        }

    }else {


    }


}
    
#endif


AT(.text.func.btring.msg)
void sfunc_bt_ring_message_do(u16 msg)
{

#if ABMATE_AUTO_ANSWER
    ab_mate_auto_answer_set(ab_mate_app.auto_answer_type);
#endif
    printf("ring_message:0x%4x\r\n",msg);
    switch (msg) {
    case KU_HSF:                //接听
    case KU_PLAY_USER_DEF:
    case KU_PLAY_PWR_USER_DEF:
            printf("KU_HSF\r\n");
#if ABMATE_CALL_REJECT
    if(ab_mate_app.local_key.key_short == 10){
        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
#if SWETZ_DU_TEST
                    wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
#endif
            printf("333333333333333333333333333333333333333\r\n");
            bt_call_terminate();    //拒接
        }

    }else if(ab_mate_app.local_key.key_short == 11){
            
           if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();
        }

    }
#endif


#if !SWETZ
        printf("KU_HSF\r\n");
        if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();
        }
#endif

        break;

    case KL_PLAY_PWR_USER_DEF:
        //PWRKEY松开前不产生KHL_PLAY_PWR消息。按键松开自动清此标志。
        sys_cb.poweron_flag = 1;
        printf("sys_cb.poweron_flag:%d\r\n",sys_cb.poweron_flag);

    case KL_PLAY_USER_DEF:
    printf("KL_PLAY_USER_DEF\r\n");
#if ABMATE_CALL_REJECT
    // if(ab_mate_app.local_key.key_long == 10){
    //     if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
    //         bt_call_terminate();    //拒接
    //     }

    //     break;
    // }else if(ab_mate_app.local_key.key_long == 11){
    //        if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
    //         bt_call_answer_incoming();
    //     }           

    // }else 
    if(ab_mate_app.local_key.key_short == 10 || ab_mate_app.local_key.key_double == 10 || ab_mate_app.local_key.key_three == 10){


    }else if(ab_mate_app.local_key.key_long == 11){
           if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();
        }          

    }else if(!CHARGE_INBOX()){

#if SWETZ_KL_PLAY_USER_DEF_DELAY5S
    if(sys_cb.time_power_def_5s != 5){
            printf("sys_cb.time_power_def_5s:%d\r\n",sys_cb.time_power_def_5s);
            break;
    }
#endif


        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
#if SWETZ_DU_TEST
                    wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
#endif      
            printf("CHARGE_INBOX:%d\r\n",CHARGE_INBOX());
            printf("888888888888888888888888888888888\r\n");
            bt_call_terminate();    //拒接
            printf("ab_mate_app.local_key.key_long:%d\r\n",ab_mate_app.local_key.key_long);
        }

    }

#endif

        // if (user_def_key_msg(xcfg_cb.user_def_kl_sel)) {
        //     break;

        // }
        
    break;
    
    case KL_HSF:
        printf("KL_HSF\r\n");
        if(ab_mate_app.local_key.key_short == 10 || ab_mate_app.local_key.key_double == 10 || ab_mate_app.local_key.key_three == 10){

            
        }else{
            if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
#if SWETZ_DU_TEST
                    wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
#endif
                printf("4444444444444444444444444444444444444\r\n");
                bt_call_terminate();    //拒接
            }

        }

        break;

    case KLH_PLAY_PWR_USER_DEF:
        //ring不响应关机消息，解决关机时间1.5时长按拒接偶尔触发关机的问题。
        break;

    case KTH_HSF:
        printf("KTH_HSF\r\n");
        break;

  
    case KD_PLAY_USER_DEF:
    case KD_PLAY_PWR_USER_DEF:
        printf("KD_PLAY_USER_DEF\r\n");
#if ABMATE_CALL_REJECT
    if(ab_mate_app.local_key.key_double == 10){
       
        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
#if SWETZ_DU_TEST
            wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
#endif            
printf("5555555555555555555555555555555555555555555\r\n");
            bt_call_terminate();    //拒接
        }

    }else if(ab_mate_app.local_key.key_short == 11 || ab_mate_app.local_key.key_long == 11 || ab_mate_app.local_key.key_three == 11){
        
    }else 
#endif


#if SWETZ
   {      
           if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();
        }
}
#else
        if (user_def_key_msg(xcfg_cb.user_def_kd_sel)) {
            break;
        }
        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
            bt_call_terminate();    //拒接
        }

#endif

        break;

    ///三击按键处理
    case KTH_PLAY_USER_DEF:
    case KTH_PLAY_PWR_USER_DEF:
        printf("KTH_PLAY_USER_DEF\r\n");
#if ABMATE_CALL_REJECT
    if(ab_mate_app.local_key.key_three == 10){
        if(bsp_res_play(TWS_RES_CALL_REJECT) == RES_ERR_INVALID) {
#if SWETZ_DU_TEST
            wav_res_play(RES_BUF_KEY_TONE_WAV, RES_LEN_KEY_TONE_WAV);
#endif         
printf("6666666666666666666666666666666666666\r\n");   
            bt_call_terminate();    //拒接
        }

    }else if(ab_mate_app.local_key.key_three == 11){
        if(bsp_res_play(TWS_RES_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();
        }   
        
    }else 
#endif
{
        user_def_key_msg(xcfg_cb.user_def_kt_sel);
}
        
        break;

	    ///四击按键处理
    case KFO_PLAY_USER_DEF:
    case KFO_PLAY_PWR_USER_DEF:
        user_def_key_msg(xcfg_cb.user_def_kfour_sel);
        break;

    ///五击按键处理
    case KFI_PLAY_USER_DEF:
    case KFI_PLAY_PWR_USER_DEF:
        user_def_key_msg(xcfg_cb.user_def_kfive_sel);
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
#if SWETZ_INCASE_MUTE
      //  app_check_mute();
#endif
        break;

        //屏蔽来电响铃调音量
    case KL_PREV_VOL_UP:
    case KH_PREV_VOL_UP:
    case KL_NEXT_VOL_UP:
    case KH_NEXT_VOL_UP:
    case KLU_NEXT_VOL_UP:
    case KU_VOL_UP_PREV:
    case KU_VOL_UP_NEXT:
    case KU_VOL_UP:
    case KL_VOL_UP:
    case KH_VOL_UP:
    case KLU_VOL_UP:

    case KL_PREV_VOL_DOWN:
    case KH_PREV_VOL_DOWN:
    case KL_NEXT_VOL_DOWN:
    case KH_NEXT_VOL_DOWN:
    case KU_VOL_DOWN_PREV:
    case KU_VOL_DOWN_NEXT:
    case KU_VOL_DOWN:
    case KL_VOL_DOWN:
    case KH_VOL_DOWN:

    case KU_VOL_UP_DOWN:
    case KL_VOL_UP_DOWN:
    case KH_VOL_UP_DOWN:
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.btring.msg)
void sfunc_bt_ring_message(u16 msg)
{
    if(msg){
        sfunc_bt_ring_message_do(msg);
    }
}

void sfunc_bt_call_message_do(u16 msg)
{
    u8 call_status;

    switch (msg) {
    case KU_HOME:
    case KL_HOME:
        if (bt_get_siri_status()) {
            printf("77777777777777777777777777777777777\r\n");
            bt_call_terminate();                        //结束SIRI
        }
        break;

    ///挂断当前通话，或者结束当前通话并接听第2路通话
    case KU_HSF:
    case KU_PLAY_USER_DEF:
    case KU_PLAY_PWR_USER_DEF:
    printf("CALL_KU_HSF\r\n");
#if  ABMATE_CALL_REJECT
    if(ab_mate_app.local_key.key_short == 11){
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();                  //接听第2路通话
        } else {
            printf("9999999999999999999999999999999999999\r\n");
            bt_call_terminate();                        //挂断当前通话
        }     

    }
#endif

#if !SWETZ
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();                  //接听第2路通话
        } else {
            bt_call_terminate();                        //挂断当前通话
        }
#endif        
        break;

    ///拒接第2路通话, 或私密接听切换
    case KL_PLAY_PWR_USER_DEF:
        sys_cb.poweron_flag = 1;                        //PWRKEY松开前不产生KHL_PLAY_PWR消息。按键松开自动清此标志。
    case KL_PLAY_USER_DEF:
        printf("-----2222222222222-----------33333333333333333\r\n");
        if (user_def_key_msg(xcfg_cb.user_def_kl_sel)) {
            break;
        }
    case KL_HSF:
        printf("KL_HSF\r\n");
#if ABMATE_CALL_REJECT        
        if(ab_mate_app.local_key.key_long == 11){
                call_status = bt_get_call_indicate();
                if(call_status == BT_CALL_INCOMING) {
                    bt_call_answer_incoming();                  //接听第2路通话
                } else {
                    printf("000000000000000000000000000000000\r\n");
                    bt_call_terminate();                        //挂断当前通话
                } 
        
        }else
#endif        
        {
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            printf("..........................................\r\n");
            bt_call_terminate();                        //拒接第2路通话
        }

        }

        break;

    ///保持当前通话并接通第2路通话，或者2路通话切换
    case KD_PLAY_PWR_USER_DEF:
    case KD_PLAY_USER_DEF:
        if (user_def_key_msg(xcfg_cb.user_def_kd_sel)) {
            break;
        }
    case KD_HSF:

#if SWETZ
#if ABMATE_CALL_REJECT
        if(ab_mate_app.local_key.key_short == 11 || ab_mate_app.local_key.key_long == 11 || ab_mate_app.local_key.key_three == 11){

        }else 
#endif        
        {
                call_status = bt_get_call_indicate();
                if(call_status == BT_CALL_INCOMING) {
                    bt_call_answer_incoming();                  //接听第2路通话
                } else {
                    printf("285222222222222222222222222222\r\n");
                    bt_call_terminate();                        //挂断当前通话
                }
        }

#else 
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();                  //接听第2路通话
        } else if(call_status >= BT_CALL_ACTIVE) {
            bt_call_swap();                             //切换两路通话或切换私密通话
        }
#endif    

        break;

    ///三击按键处理
    case KTH_PLAY_USER_DEF:
    case KTH_PLAY_PWR_USER_DEF:
#if ABMATE_CALL_REJECT        
    if(ab_mate_app.local_key.key_three == 11){
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();                  //接听第2路通话
        } else {
            printf("***********************************\r\n");
            bt_call_terminate();                        //挂断当前通话
        }   

    }else
#endif    
    {
        user_def_key_msg(xcfg_cb.user_def_kt_sel);
    }

        
        break;

    ///四击按键处理
    case KFO_PLAY_USER_DEF:
    case KFO_PLAY_PWR_USER_DEF:
        user_def_key_msg(xcfg_cb.user_def_kfour_sel);
        break;

    ///五击按键处理
    case KFI_PLAY_USER_DEF:
    case KFI_PLAY_PWR_USER_DEF:
        user_def_key_msg(xcfg_cb.user_def_kfive_sel);
        break;

    case EVT_HFP_SET_VOL:
        if(sys_cb.incall_flag & INCALL_FLAG_SCO) {
            bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
            sys_cb.cm_times = 0;
            sys_cb.cm_vol_change = 1;
        }
        break;

    case EVT_A2DP_MUSIC_PLAY:
        dac_fade_in();
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
#if SWETZ_INCASE_MUTE
            if(f_bt.disp_status >= BT_STA_INCOMING){
            if(bsp_get_mute_sta()){
                    bsp_clr_mute_sta();
                    bsp_sys_unmute();
            }
    }else {
            app_check_mute();
    }
#endif
        break;

    default:
        func_message(msg);
        break;
    }
}

void sfunc_bt_call_message(u16 msg)
{
    if(msg){
        sfunc_bt_call_message_do(msg);
    }
}

#if FUNC_BTHID_EN

AT(.text.func.bt.msg)
void func_bthid_message_do(u16 msg)
{
    switch (msg) {
    case KU_PLAY:
    case KU_PLAY_USER_DEF:
    case KU_PLAY_PWR_USER_DEF:
        if (bt_get_status() < BT_STA_DISCONNECTING) {
            bt_connect();
        } else {
            bsp_bt_hid_photo(HID_KEY_VOL_UP);
            mp3_res_play(RES_BUF_TAKE_PHOTO_MP3,RES_LEN_TAKE_PHOTO_MP3);
        }
        break;

    case KL_PLAY:
        bt_disconnect(0);
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.bt.msg)
void func_bthid_message(u16 msg)
{
    if(msg){
        func_bthid_message_do(msg);
    }
}
#endif
