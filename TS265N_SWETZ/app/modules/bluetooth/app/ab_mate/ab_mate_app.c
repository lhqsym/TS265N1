#include "include.h"
#include "ab_mate_app.h"
#include "ab_mate_timer.h"
#include "ab_mate_ota.h"
#include "ab_mate_tws.h"
#include "ab_mate_profile.h"

#if AB_MATE_APP_EN

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if (BT_HFP_TIME_EN || BT_MAP_EN)
#define AB_MATE_MAP_GET_CNT         3      //bt时间获取尝试次数
static u8 ab_mate_time_get_cnt;

void hfp_at_kick(void);
void bt_map_kick(void);
bool hfp_at_time_get(u8 *buf, u8 len);
bool bt_map_time_get(u8 *buf, u8 len);
#endif

#if AB_MATE_EQ_EN
struct __attribute__((packed)) eq_all_mode_info_st{
    const s8 eq_normal[AB_MATE_EQ_BAND_CNT];
    const s8 eq_pop[AB_MATE_EQ_BAND_CNT];
    const s8 eq_rock[AB_MATE_EQ_BAND_CNT];
    const s8 eq_jazz[AB_MATE_EQ_BAND_CNT];
    const s8 eq_classic[AB_MATE_EQ_BAND_CNT];
    const s8 eq_country[AB_MATE_EQ_BAND_CNT];
};

const struct eq_all_mode_info_st eq_all_mode_info = {
    .eq_normal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .eq_pop = {3, 1, 0, -5, 0, 4, 1, 3, 2, 5},
    .eq_rock = {5, 5, 3, 0, 0, -1, -3, 2, 0, 4},
    .eq_jazz = {5, 3, 0, -1, -3, 0, 3, -3, 2, 3},
    .eq_classic = {4, 4, 3, 0, -3, 0, 2, 0, -3, 4},
    .eq_country = {0, 0, 2, 0, 3, 2, 5, 3, 1, 0},

    // .eq_normal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // .eq_pop = {5, 3, 2, 2, -4, -4, -2, 0, 1, 2},
    // .eq_rock = {-2, 0, 2, 4, -2, -2, 0, 0, 4, 4},
    // .eq_jazz = {0, 0, 0, 4, 4, 4, 0, 2, 3, 4},
    // .eq_classic = {0, 8, 8, 4, 0, 0, 0, 0, 2, 2},
    // .eq_country = {-2, 0, 0, 2, 2, 0, 0, 0, 4, 4},
};

#if AB_MATE_EQ_USE_DEVICE
struct __attribute__((packed)) eq_mode_custom_st{
    s8 eq_custom1[AB_MATE_EQ_BAND_CNT];
    s8 eq_custom2[AB_MATE_EQ_BAND_CNT];
};

static struct eq_mode_custom_st eq_mode_custom = {
    .eq_custom1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .eq_custom2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
#endif
#endif

#if AB_MATE_ANC_EN
static const u8 anc_app_table[] = {APP_ANC_STOP, APP_ANC_START, APP_ANC_TRANSPARENCY};
#endif

#if AB_MATE_ANC_LEVEL
static const s8 anc_level_table[AB_MATE_ANC_TOTAL_LEVEL + 1][2] = {
    {0,      0},            // level 0
    {-5,    -2},            // level 1 -- FF:-1db    FB:-0.4db       //step:0.2db
    {-2,    -3},            // level 2 -- FF:-0.4db  FB:-0.6db
    {5,      10},           // level 3 -- FF: 1db    FB: 2db
    {6,      12},           // level 4 -- FF: 1.2db    FB: 2.4db
};

static const s8 tp_level_table[AB_MATE_TP_TOTAL_LEVEL + 1][2] = {       //step:0.2db
    {0,      0},            // level 0
    {5,     10},            // level 1 -- FF: 1db     FB: 2db
    {-3,    -4},            // level 2 -- FF:-0.6db   FB:-0.8db
    {15,    20},            // level 3 -- FF: 3db     FB: 4db
};
#endif


#if AB_MATE_LANG_EN
static const u8 lang_app_table[] = {APP_LANG_EN, APP_LANG_ZH};
#endif

#if AB_MATE_KEY_EN
static const u8 key_app2local_table[] = {UDK_NONE,          UDK_REDIALING,
                                     UDK_SIRI,          UDK_PREV,
                                     UDK_NEXT,          UDK_VOL_UP,
                                     UDK_VOL_DOWN,      UDK_PLAY_PAUSE,
                                     UDK_LOW_LATENCY,   UDK_NR,
#if ABMATE_CALL_REJECT
                                     UDK_REJECTCALL,    UDK_ANSWER_HANG,                       
#endif

                                                            

                                    };
#if !AB_MATE_KEY_USER_DEF_EN
static const u8 key_local2app_table[] = {APP_KEY_NONE,      APP_KEY_REDIALING,
                                         APP_KEY_SIRI,      APP_KEY_PREV,
                                         APP_KEY_NEXT,      APP_KEY_VOL_UP,
                                         APP_KEY_VOL_DOWN,  APP_KEY_NONE,
                                         APP_KEY_NONE,      APP_KEY_NONE,
                                         APP_KEY_NONE,      APP_KEY_PLAY_PAUSE,
                                         APP_KEY_NONE,      APP_KEY_LOW_LATENCY,
                                         APP_KEY_NONE
                                        };
#endif
#endif

ab_mate_cmd_recv_t ab_mate_cmd_recv AT(.app.buf);
ab_mate_cmd_send_t ab_mate_cmd_send AT(.app.buf);
ab_mate_app_var_t ab_mate_app AT(.app.buf);

soft_timer_p cm_timer;
soft_timer_p vbat_timer;

#if AB_MATE_DEVICE_FIND_EN
soft_timer_p device_find_timer;
#endif

#if AB_MATE_PP_EN
soft_timer_p play_sta_timer;
#endif

#if AB_MATE_BT_STA_EN
soft_timer_p bt_sta_timer;
#endif

#if AB_MATE_CUSTOM_CMD_TONE_EN
soft_timer_p custom_cmd_tone_timer;
#endif

extern u32 fot_head_crc_get(void);

void ab_mate_cm_sync(soft_timer_p timer)
{
    cm_sync();
}

void ab_mate_cm_timer_creat(void)
{
    soft_timer_create(&cm_timer, 3000, TIMER_SINGLE_SHOT, ab_mate_cm_sync);
}

void ab_mate_cm_write(void *buf, u16 addr, u16 size, u8 sync)
{
    cm_write(buf, APP_CM_PAGE(addr), size);
    if(sync == 1){
        cm_sync();  //sync now
    }else if(sync == 2){
        soft_timer_restart(cm_timer,3000); //sync after 3000ms
    }else{
        //not sync
    }
}

void ab_mate_cm_read(void *buf, u16 addr, u16 size)
{
    cm_read(buf, APP_CM_PAGE(addr), size);
}

u8 ab_mate_mtu_get(void)
{
    u16 mtu = 0;

    if(ble_is_connect()){
        mtu = ble_get_gatt_mtu();
    }
#if BT_SPP_EN
    else{
        mtu = get_spp_mtu_size();
    }
#endif
    if(mtu > 0xFF){
        mtu = 0xFF;
    }

    return (u8)mtu;
}

static void ab_mate_data_send_do(uint8_t *packet, uint16_t len)
{
#ifdef AB_MATE_SPP_UUID
    u8 spp_ch = SPP_SERVICE_CH2;
#else
    u8 spp_ch = SPP_SERVICE_CH0;
#endif

    if(ab_mate_app.con_sta == AB_MATE_CON_BLE){
        ab_mate_ble_send_packet(packet, len);
    }else if(ab_mate_app.con_sta == AB_MATE_CON_SPP){
        bt_spp_tx(spp_ch, packet, len);
    }
#if SWETZ 
    TRACE("ab tx:");
    TRACE_R(packet, len);
    
#endif 
}

void ab_mate_data_send(u8* buf, u16 len)
{
    printf("ab_mate_app.con_sta:%d  ab_mate_app.can_send_now:%d\r\n",ab_mate_app.con_sta,ab_mate_app.can_send_now);
    if(ab_mate_app.con_sta ){//&& ab_mate_app.can_send_now){

        u8 mtu = ab_mate_mtu_get() - AB_MATE_HEADER_LEN;
        u8 total_frame = 0;
        u8 cur_frame = 0;
        u8 data_len = len - AB_MATE_HEADER_LEN;
        u8 *p_data = buf;
        u8 send_len;

        if(data_len > mtu){
            total_frame = (data_len / mtu) + (data_len % mtu ? 1 : 0) - 1;
        }

        ab_mate_cmd_send.cmd_head.frame_total = total_frame;

        do{
            send_len = data_len > mtu ? mtu : data_len;
            ab_mate_cmd_send.cmd_head.seq++;
            ab_mate_cmd_send.cmd_head.frame_seq = cur_frame++;
            ab_mate_cmd_send.cmd_head.payload_len = send_len;
            memcpy(p_data, &ab_mate_cmd_send.cmd_head, AB_MATE_HEADER_LEN);
            ab_mate_data_send_do(p_data, send_len + AB_MATE_HEADER_LEN);
            data_len -= send_len;
            p_data += send_len;
        }while(total_frame--);
    }
}

void ab_mate_request_common_response(ab_mate_result_t result)
{
    ab_mate_cmd_send.cmd_head.payload_len = 1;
    ab_mate_cmd_send.payload[0] = result;

    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + 1);
}

void ab_mate_notify_do(u8 cmd,u8* payload,u8 len)
{
    ab_mate_cmd_send.cmd_head.cmd = cmd;
    ab_mate_cmd_send.cmd_head.cmd_type = CMD_TYPE_NOTIFY;
    ab_mate_cmd_send.cmd_head.frame_seq = 0;
    ab_mate_cmd_send.cmd_head.frame_total = 0;
    ab_mate_cmd_send.cmd_head.payload_len = len;
    memcpy(ab_mate_cmd_send.payload,payload,len);
    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + len);
}

void ab_mate_device_info_notify(u8* payload,u8 len)
{
    ab_mate_notify_do(CMD_DEVICE_INFO_NOTIFY, payload, len);
}

#if AB_MATE_DEVICE_FIND_EN
void ab_mate_device_find_do(soft_timer_p timer)
{
    if(ab_mate_app.device_find){
        reset_sleep_delay();
        if (ab_mate_app.find_type <= DEVICE_FIND_START) {
            printf("111\r\n");
           bsp_res_play(TWS_RES_SW_FIND_EAR);
         // wav_res_play(RES_BUF_FIND_EAR_WAV,RES_LEN_FIND_EAR_WAV);
        //    maxvol_tone_play();
        } else {
            printf("222\r\n");
            bsp_set_volume(VOL_MAX);
            bsp_piano_warning_play(WARNING_TONE, TONE_MAX_VOL);
        }
    }
}

void ab_mate_device_find_timer_creat(void)
{
    soft_timer_create(&device_find_timer, 2000, TIMER_REPEATED, ab_mate_device_find_do);
}
#endif

#if AB_MATE_CUSTOM_CMD_TONE_EN
void ab_mate_custom_cmd_tone_play(soft_timer_p timer)
{
    reset_sleep_delay();
    switch (ab_mate_app.custom_tone_type) {
    case TONE_CTRL_COUNTDWON_PLAY:
        bsp_res_play(TWS_RES_RING);
        break;

    default:
        break;
    }
}

void ab_mate_custom_cmd_tone_timer_creat(void)
{
    soft_timer_create(&custom_cmd_tone_timer, 2000, TIMER_REPEATED, ab_mate_custom_cmd_tone_play);
}
#endif

#if AB_MATE_PP_EN
void ab_mate_play_sta_notify(soft_timer_p timer)
{
    if(ab_mate_app.con_sta){
#if AB_MATE_OTA_EN
        if(!ab_mate_ota_is_start())
#endif
        {
            u8 tlv_data[3] = {INFO_PLAY_STA, 1, 0x00};
            tlv_data[2] = ab_mate_app.play_sta;
            ab_mate_device_info_notify(tlv_data,sizeof(tlv_data));
            TRACE("play_sta_report:%d\n",ab_mate_app.play_sta);
        }
    }
}

void ab_mate_play_sta_timer_creat(void)
{
    soft_timer_create(&play_sta_timer, 1000, TIMER_SINGLE_SHOT, ab_mate_play_sta_notify);
}

AT(.text.ab_mate.process)
void ab_mate_play_sta_report_proc(void)
{
    if(!bt_tws_is_slave()){
        u8 play_sta = a2dp_is_playing_fast();
        if(ab_mate_app.play_sta != play_sta){
            ab_mate_app.play_sta = play_sta;
            soft_timer_restart(play_sta_timer,1000);
        }
    }
}
#endif

void ab_mate_vbat_check_proc(soft_timer_p timer)
{
    static u8 local_bat = 0;
    static u8 remote_bat = 0;
    static u8 box_bat = 0;
    u8 update = 0;

#if BT_TWS_EN
    u8 sync = 1;
#endif

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    ab_mate_app.box_vbat = charge_box_get_charge_box_bat_level();
    ab_mate_app.local_vbat = sys_cb.loc_bat;
    if((sys_cb.loc_bat & BIT(7)) && charge_box_bat_is_ready()){
        ab_mate_app.remote_vbat = sys_cb.rem_bat;
        sync = 0;
    }
#else
    ab_mate_app.local_vbat = bsp_get_bat_level();
#endif

    if((local_bat != ab_mate_app.local_vbat) || (box_bat != ab_mate_app.box_vbat)){
        local_bat = ab_mate_app.local_vbat;
        box_bat = ab_mate_app.box_vbat;
#if BT_TWS_EN
        if(sync){
            ab_mate_tws_vbat_sync();
        }
#endif
        update = 1;
    }

#if BT_TWS_EN
    if(remote_bat != ab_mate_app.remote_vbat){
        remote_bat = ab_mate_app.remote_vbat;
        update = 1;
    }
#endif

    if(!ab_mate_ota_is_start() && ab_mate_app.con_sta && update){
        u8 tlv_data[5] = {INFO_POWER, 3, 0x00, 0x00, 0x00};
#if BT_TWS_EN
        if(sys_cb.tws_left_channel){
            tlv_data[2] = local_bat;    //L
            tlv_data[3] = remote_bat;   //R
            tlv_data[4] = box_bat;
        }else
#endif
		{
            tlv_data[2] = remote_bat;   //L
            tlv_data[3] = local_bat;    //R
            tlv_data[4] = box_bat;
        }

        ab_mate_device_info_notify(tlv_data,sizeof(tlv_data));
        TRACE("vbat_report:L %d, R %d, Box %d\n",tlv_data[2],tlv_data[3],tlv_data[4]);
    }
}

void ab_mate_vbat_timer_creat(void)
{
    soft_timer_create(&vbat_timer, 2000, TIMER_REPEATED, ab_mate_vbat_check_proc);
}

#if AB_MATE_VOL_EN
void ab_mate_vol_notify(void)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[3] = {INFO_VOL, 1, 0x00};
        tlv_data[2] = (sys_cb.vol * 100) / VOL_MAX;
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}
#endif

#if AB_MATE_ANC_EN
void ab_mate_anc_mode_notify(void)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[3] = {INFO_ANC, 1, 0x00};
        tlv_data[2] = anc_app_table[sys_cb.anc_user_mode];
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}
#endif

#if AB_MATE_LATENCY_EN
void ab_mate_device_mode_notify(void)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[3] = {INFO_LATENCY_MODE, 1, 0x00};
        tlv_data[2] = ab_mate_app.latency_mode;
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}
#endif

void ab_mate_tws_con_sta_notify(u8 sta)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[6] = {INFO_TWS_STA, 1, 0x00, INFO_TWS_CHANNEL, 1, 0};
        tlv_data[2] = sta;
        tlv_data[5] = func_bt_tws_get_channel();
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}

void ab_mate_tws_channel_notify(void)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[3] = {INFO_TWS_CHANNEL, 1, 0};
        tlv_data[2] = func_bt_tws_get_channel();
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}

#if AB_MATE_EQ_USE_DEVICE
void ab_mate_eq_custom_save(void)
{
    u8 *p_eq = (u8*)&eq_mode_custom;
    u8 offset = (ab_mate_app.eq_info.mode - AB_MATE_EQ_CUSTOM_INDEX) * AB_MATE_EQ_BAND_CNT;
    p_eq += offset ;
    memcpy(p_eq, &ab_mate_app.eq_info.gain, ab_mate_app.eq_info.band_cnt);
    ab_mate_cm_write(p_eq, AB_MATE_CM_EQ_CUSTOM1 + offset, AB_MATE_EQ_BAND_CNT, 2);
}
#endif

void ab_mate_eq_set(u8 *payload,u8 payload_len)
{
#if AB_MATE_EQ_EN
    ab_mate_app.eq_info.band_cnt = payload[0];
    ab_mate_app.eq_info.mode = payload[1];
    memcpy(ab_mate_app.eq_info.gain, &payload[2], ab_mate_app.eq_info.band_cnt);

#if AB_MATE_EQ_USE_DEVICE
    if(ab_mate_app.eq_info.mode >= AB_MATE_EQ_CUSTOM_INDEX){
        ab_mate_eq_custom_save();
    }
#endif

#if BT_TWS_EN
    ab_mate_tws_eq_info_sync();
#endif

    ab_mate_app.do_flag |= FLAG_EQ_SET;

    ab_mate_cm_write(&ab_mate_app.eq_info.mode, AB_MATE_CM_EQ_DATA, 1+AB_MATE_EQ_BAND_CNT, 2);

    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_music_set(u8 *payload,u8 payload_len)
{
    u8 tlv_type = payload[0];
    u8 *tlv_val = &payload[2];
    u8 result = AB_MATE_SUCCESS;

    switch(tlv_type){
        case A2DP_CTL_VOICE:{

#if ABMATE_VOL_SWITCH
            if (tlv_val[0])
            {
                msg_enqueue(KU_VOL_UP);
            }
            else
            {
                msg_enqueue(KU_VOL_DOWN);
            }
#else

            u32 vol,vol_max = VOL_MAX;
            if(sys_cb.incall_flag){
                vol_max = 15;
            }
            vol = tlv_val[0];
            vol = (vol * vol_max) / 100;
           // bsp_set_volume(vol);
            bsp_bt_vol_set(vol);
            ab_mate_app.vol = vol;
#endif            
        } break;

        case A2DP_CTL_PLAY:
            bt_music_play();
            ab_mate_app.play_sta = 1;
            delay_5ms(4);
            break;

        case A2DP_CTL_PASUE:
            bt_music_pause();
            ab_mate_app.play_sta = 0;
            delay_5ms(4);
            break;

        case A2DP_CTL_PREV:
            bt_music_prev();
            break;

        case A2DP_CTL_NEXT:
            bt_music_next();
            break;

        default:
            result = AB_MATE_FAIL;
            break;
    }

    ab_mate_cmd_send.cmd_head.payload_len = 3;
    ab_mate_cmd_send.payload[0] = tlv_type;
    ab_mate_cmd_send.payload[1] = 1;
    ab_mate_cmd_send.payload[2] = result;
    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + ab_mate_cmd_send.cmd_head.payload_len);
}

void ab_mate_device_info_query(u8 *payload,u8 payload_len)
{
    u8 read_offset = 0;
    u8 write_offset = 0;
    u8 *buf = ab_mate_cmd_send.payload;
    u8 val_len = 0;

    while(read_offset < payload_len){
        switch(payload[read_offset]){
            case INFO_POWER:
                TRACE("INFO_POWER\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_POWER;
                buf[write_offset++] = 3;
#if (CHARGE_BOX_TYPE == CBOX_SSW)
                ab_mate_app.box_vbat = charge_box_get_charge_box_bat_level();
                ab_mate_app.local_vbat = sys_cb.loc_bat;
#else
                ab_mate_app.local_vbat = bsp_get_bat_level();
#endif
				#if BT_TWS_EN
                if(sys_cb.tws_left_channel){
                    buf[write_offset++] = ab_mate_app.local_vbat;
                    buf[write_offset++] = ab_mate_app.remote_vbat;
                    buf[write_offset++] = ab_mate_app.box_vbat;
                }else
				#endif
				{
                    buf[write_offset++] = ab_mate_app.remote_vbat;
                    buf[write_offset++] = ab_mate_app.local_vbat;
                    buf[write_offset++] = ab_mate_app.box_vbat;
                }
            break;

            case INFO_VERSION:
                TRACE("INFO_VERSION\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_VERSION;
                buf[write_offset++] = sizeof(ab_mate_app.version);
                memcpy(&buf[write_offset],ab_mate_app.version,sizeof(ab_mate_app.version));
                write_offset += sizeof(ab_mate_app.version);
            break;

            case INFO_BT_NAME:
                TRACE("INFO_BT_NAME\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_BT_NAME;
                buf[write_offset++] = strlen(xcfg_cb.bt_name);
                memcpy(&buf[write_offset], xcfg_cb.bt_name, strlen(xcfg_cb.bt_name));
                write_offset += strlen(xcfg_cb.bt_name);
                break;

            case INFO_EQ:
                TRACE("INFO_EQ\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_EQ;
                buf[write_offset++] = sizeof(ab_mate_app.eq_info);
                memcpy(&buf[write_offset], &ab_mate_app.eq_info, sizeof(ab_mate_app.eq_info));
                write_offset += sizeof(ab_mate_app.eq_info);
                break;

            case INFO_KEY:{
                TRACE("INFO_KEY\n");
                ab_mate_key_info_t *key_left;
                ab_mate_key_info_t *key_right;
				#if BT_TWS_EN
                if(sys_cb.tws_left_channel){
                    key_left = &ab_mate_app.local_key;
                    key_right = &ab_mate_app.remote_key;
                }else
				#endif
				{
                    key_left = &ab_mate_app.remote_key;
                    key_right = &ab_mate_app.local_key;
                }

                val_len = payload[read_offset + 1];

                u8 key_info[] = {KEY_LEFT_SHORT, 1, key_left->key_short,
                                 KEY_RIGHT_SHORT, 1, key_right->key_short,
                                 KEY_LEFT_DOUBLE, 1, key_left->key_double,
                                 KEY_RIGHT_DOUBLE, 1, key_right->key_double,
                                 KEY_LEFT_THREE, 1, key_left->key_three,
                                 KEY_RIGHT_THREE, 1, key_right->key_three,
                                 KEY_LEFT_LONG, 1, key_left->key_long,
                                 KEY_RIGHT_LONG, 1, key_right->key_long,
                                };

                buf[write_offset++] = INFO_KEY;
                buf[write_offset++] = sizeof(key_info);
                memcpy(&buf[write_offset],key_info,sizeof(key_info));
                write_offset += sizeof(key_info);
            } break;

            case INFO_VOL:
                TRACE("INFO_VOL\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_VOL_EN
                buf[write_offset++] = INFO_VOL;
                buf[write_offset++] = 1;
                buf[write_offset++] = (sys_cb.vol * 100) / VOL_MAX;
#endif
                break;

            case INFO_PLAY_STA:
                TRACE("INFO_PLAY_STA\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_PP_EN
                buf[write_offset++] = INFO_PLAY_STA;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.play_sta;
#endif
                break;

            case INFO_LATENCY_MODE:
                TRACE("INFO_MODE\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_LATENCY_MODE;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.latency_mode;
                break;

            case INFO_IN_EAR_EN:
                TRACE("INFO_IN_EAR_EN\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_INERA_EN
                buf[write_offset++] = INFO_IN_EAR_EN;
                buf[write_offset++] = 1;
                buf[write_offset++] = sys_cb.in_ear_en;
#endif
                break;

            case INFO_LANGUAGE:
                TRACE("INFO_LANGUAGE\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_LANG_EN
                buf[write_offset++] = INFO_LANGUAGE;
                buf[write_offset++] = 1;
                buf[write_offset++] = lang_app_table[sys_cb.lang_id];
#endif
                break;

            case INFO_AUTO_ANSWER:
                TRACE("INFO_AUTO_ANSWER\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_AUTO_ANSWER;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.auto_answer_type;
                break;

            case INFO_ANC:
                TRACE("INFO_ANC\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_ANC_EN
                buf[write_offset++] = INFO_ANC;
                buf[write_offset++] = 1;
                buf[write_offset++] = anc_app_table[sys_cb.anc_user_mode];
#endif
                break;

            case INFO_TWS_SUPPORT:
                TRACE("INFO_TWS_SUPPORT\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_TWS_SUPPORT;
                buf[write_offset++] = 1;
                buf[write_offset++] = BT_TWS_EN * xcfg_cb.bt_tws_en;
                break;

            case INFO_TWS_STA:
                TRACE("INFO_TWS_STA\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_TWS_STA;
                buf[write_offset++] = 1;
                buf[write_offset++] = bt_tws_is_connected();
                break;

            case INFO_MTU:
                TRACE("INFO_MTU\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_MTU;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_mtu_get();
                ab_mate_app.can_send_now = 1;
#if (BT_HFP_TIME_EN || BT_MAP_EN)
                ab_mate_time_get_cnt = AB_MATE_MAP_GET_CNT;
#endif
                break;

            case INFO_LED:{
                TRACE("INFO_LED\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_LED_EN
                buf[write_offset++] = INFO_LED;
                buf[write_offset++] = 1;
                buf[write_offset++] = sys_cb.led_scan_en;
#endif
            } break;

            case INFO_CRC:{
                TRACE("INFO_CRC:%x\n",ab_mate_app.flash_crc);
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_CRC;
                buf[write_offset++] = 4;
                memcpy(&buf[write_offset],&ab_mate_app.flash_crc,4);
                write_offset += 4;
            } break;

            case INFO_ANC_CUR_LEVEL:{
                TRACE("INFO_ANC_CUR_LEVEL\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_ANC_EN
                buf[write_offset++] = INFO_ANC_CUR_LEVEL;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.anc_cur_level;
#endif
            } break;

            case INFO_TRANSPARENCY_CUR_LEVEL:{
                TRACE("INFO_TRANSPARENCY_CUR_LEVEL\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_ANC_EN
                buf[write_offset++] = INFO_TRANSPARENCY_CUR_LEVEL;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.tp_cur_level;
#endif
            } break;

            case INFO_ANC_TOTAL_LEVEL:{
                TRACE("INFO_ANC_TOTAL_LEVEL\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_ANC_EN
                buf[write_offset++] = INFO_ANC_TOTAL_LEVEL;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.anc_total_level;
#endif
            } break;

            case INFO_TRANSPARENCY_TOTAL_LEVEL:{
                TRACE("INFO_TRANSPARENCY_TOTAL_LEVEL\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_ANC_EN
                buf[write_offset++] = INFO_TRANSPARENCY_TOTAL_LEVEL;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.tp_total_level;
#endif
            } break;

#if AB_MATE_EQ_USE_DEVICE
            case INFO_EQ_ALL_MODE:
                TRACE("INFO_EQ_ALL_MODE\n");
                val_len = payload[read_offset + 1];
                u8 eq_const_mode_cnt = sizeof(eq_all_mode_info) / AB_MATE_EQ_BAND_CNT;
                u8 eq_custom_mode_cnt = sizeof(eq_mode_custom) / AB_MATE_EQ_BAND_CNT;
                buf[write_offset++] = INFO_EQ_ALL_MODE;
                buf[write_offset++] = 1 + (eq_const_mode_cnt + eq_custom_mode_cnt) * (AB_MATE_EQ_BAND_CNT + 1);
                buf[write_offset++] = AB_MATE_EQ_BAND_CNT;
                u8 i;
                u8 *p_eq_info = (u8*)&eq_all_mode_info;
                for(i = 0; i< eq_const_mode_cnt; i++){
                    buf[write_offset++] = i;
                    memcpy(&buf[write_offset], p_eq_info, AB_MATE_EQ_BAND_CNT);
                    write_offset += AB_MATE_EQ_BAND_CNT;
                    p_eq_info += AB_MATE_EQ_BAND_CNT;
                }
                p_eq_info = (u8*)&eq_mode_custom;
                for(i = 0; i< eq_custom_mode_cnt;i++){
                    buf[write_offset++] = AB_MATE_EQ_CUSTOM_INDEX + i;
                    memcpy(&buf[write_offset], p_eq_info, AB_MATE_EQ_BAND_CNT);
                    write_offset += AB_MATE_EQ_BAND_CNT;
                    p_eq_info += AB_MATE_EQ_BAND_CNT;
                }
                break;
#endif

            case INFO_TWS_CHANNEL:
                TRACE("INFO_TWS_CHANNEL\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_TWS_CHANNEL;
                buf[write_offset++] = 1;
                buf[write_offset++] = func_bt_tws_get_channel();
                break;


            case INFO_V3D_AUDIO:
                TRACE("INFO_V3D_AUDIO\n");
                val_len = payload[read_offset + 1];
#if AB_MATE_V3D_AUDIO_EN
                buf[write_offset++] = INFO_V3D_AUDIO;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.v3d_audio_en;
#endif
                break;

#if AB_MATE_BT_ATT_EN
            case INFO_PID:
                TRACE("INFO_PID\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_PID;
                buf[write_offset++] = 2;
                buf[write_offset++] = 0x01; //PID信息
                buf[write_offset++] = 0x00;
                break;
#endif

#if AB_MATE_MULT_DEV_EN
            case INFO_MULT_DEV:
                TRACE("INFO_MULT_DEV\n");
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_MULT_DEV;
                buf[write_offset++] = 1;
                buf[write_offset++] = ab_mate_app.mult_dev.en;
                break;

            case INFO_PAIRED_INFO:{
                TRACE("INFO_PAIRED_INFO\n");
                val_len = payload[read_offset + 1];
                u8 offset = write_offset;
                buf[offset] = INFO_PAIRED_INFO;
                buf[offset+1] = ab_mate_connect_info_get(&buf[offset+2]);
                write_offset += (2+buf[offset+1]);
            }   break;
#endif

            case INFO_DEV_CAP:{
                TRACE("INFO_DEV_CAP\n");
                u16 capacity = 0;
                if(BT_TWS_EN * xcfg_cb.bt_tws_en){
                    capacity |= BIT(0);
                }
//                if(BT_MUSIC_AUDIO_EN){
//                    capacity |= BIT(1);
//                }
                if(AB_MATE_MULT_DEV_EN * xcfg_cb.bt_2acl_en){
                    capacity |= BIT(2);
                }
                if(ANC_EN){
                    capacity |= BIT(3);
                }
                val_len = payload[read_offset + 1];
                buf[write_offset++] = INFO_DEV_CAP;
                buf[write_offset++] = 2;
                memcpy(&buf[write_offset], &capacity, 2);
                write_offset += 2;
            } break;

            default:
                val_len = payload[read_offset + 1];
                break;
        }
        read_offset += (2 + val_len);
    }

    ab_mate_cmd_send.cmd_head.payload_len = write_offset;

   TRACE("ab_mate_device_info_query[%d]:",write_offset);
   TRACE_R(buf,write_offset);

    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + ab_mate_cmd_send.cmd_head.payload_len);
}

void ab_mate_anc_level_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_ANC_LEVEL
    ab_mate_result_t result = AB_MATE_SUCCESS;
    ab_mate_app.anc_cur_level = payload[0];
    if(ab_mate_app.anc_mode == APP_ANC_START){
        ab_mate_cm_write(&ab_mate_app.anc_cur_level,AB_MATE_CM_ANC_LEVEL,1,2);
#if BT_TWS_EN
        if(!bt_tws_is_slave()){
            ab_mate_tws_anc_cur_level_sync();
        }
#endif
        TRACE("anc mic0 level gain:%d,adjust gain:%d\n",anc_level_table[ab_mate_app.anc_cur_level][0],ab_mate_app.anc_adjust_mic0);
        TRACE("anc mic1 level gain:%d,adjust gain:%d\n",anc_level_table[ab_mate_app.anc_cur_level][1],ab_mate_app.anc_adjust_mic1);
        bsp_anc_gain_adjust(0, (anc_level_table[ab_mate_app.anc_cur_level][0] + ab_mate_app.anc_adjust_mic0)); //FF Gain adjust
        bsp_anc_gain_adjust(1, (anc_level_table[ab_mate_app.anc_cur_level][1] + ab_mate_app.anc_adjust_mic1));  //FB Gain adjust
    }else{
        result = AB_MATE_FAIL;
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif

}

void ab_mate_tp_level_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_ANC_LEVEL
    ab_mate_result_t result = AB_MATE_SUCCESS;
    ab_mate_app.tp_cur_level = payload[0];
    if(ab_mate_app.anc_mode == APP_ANC_TRANSPARENCY){
        ab_mate_cm_write(&ab_mate_app.tp_cur_level,AB_MATE_CM_TP_LEVEL,1,2);
#if BT_TWS_EN
        if(!bt_tws_is_slave()){
            ab_mate_tws_tp_cur_level_sync();
        }
#endif
        TRACE("tp mic0 level gain:%d,adjust gain:%d\n",tp_level_table[ab_mate_app.tp_cur_level][0],ab_mate_app.tp_adjust_mic0);
        TRACE("tp mic1 level gain:%d,adjust gain:%d\n",tp_level_table[ab_mate_app.tp_cur_level][1],ab_mate_app.tp_adjust_mic1);
        bsp_anc_gain_adjust(0, (tp_level_table[ab_mate_app.tp_cur_level][0] + ab_mate_app.tp_adjust_mic0)); //FF Gain adjust
        bsp_anc_gain_adjust(1, (tp_level_table[ab_mate_app.tp_cur_level][1] + ab_mate_app.tp_adjust_mic1));  //FB Gain adjust
    }else{
        result = AB_MATE_FAIL;
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif

}

void ab_mate_anc_set(u8 *payload, u8 payload_len)
{
#if ABMATE_ANC_SWITCH
    ab_mate_result_t result = AB_MATE_SUCCESS;
    if (payload[0] < APP_ANC_NUM)
    {
        ab_mate_app.anc_set = payload[0] + 1;
    }
    else
    {
        result = AB_MATE_FAIL;
    }
    ab_mate_request_common_response(result);

#else 
#if AB_MATE_ANC_EN
    ab_mate_result_t result = AB_MATE_SUCCESS;
    ab_mate_app.anc_mode = payload[0];
    if(ab_mate_app.anc_mode != anc_app_table[sys_cb.anc_user_mode]){
        if(ab_mate_app.anc_mode == APP_ANC_STOP){
            sys_cb.anc_user_mode = 0;
            bsp_res_play(TWS_RES_NR_DISABLE);                   //同步播放语言提示音,并切换ANC模式
        }else if(ab_mate_app.anc_mode == APP_ANC_START){
            sys_cb.anc_user_mode = 1;
            bsp_res_play(TWS_RES_ANC);
        }else if(ab_mate_app.anc_mode == APP_ANC_TRANSPARENCY){
            sys_cb.anc_user_mode = 2;
            bsp_res_play(TWS_RES_TRANSPARENCY);
        }else{
            result = AB_MATE_FAIL;
        }
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
#endif



}

void ab_mate_key_set_do(void)
{
#if AB_MATE_KEY_EN
    AB_MATE_KEY_SHORT = key_app2local_table[ab_mate_app.local_key.key_short];
    AB_MATE_KEY_DOUBLE = key_app2local_table[ab_mate_app.local_key.key_double];
    AB_MATE_KEY_THREE = key_app2local_table[ab_mate_app.local_key.key_three];
    AB_MATE_KEY_LONG = key_app2local_table[ab_mate_app.local_key.key_long];

    ab_mate_cm_write(&ab_mate_app.local_key,AB_MATE_CM_KEY_LOCAL,sizeof(ab_mate_key_info_t),2);
#endif
}

void ab_mate_key_set(u8 *payload ,u8 payload_len)
{
    u8 read_offset = 0;
    u8 write_offset = 0;
    u8 *buf = ab_mate_cmd_send.payload;
    u8 val_len = 0;

    ab_mate_key_info_t *key_left;
    ab_mate_key_info_t *key_right;
    ab_mate_key_info_t key_loc;
    ab_mate_key_info_t key_rem;

    memcpy(&key_loc, &ab_mate_app.local_key, sizeof(ab_mate_key_info_t));
    memcpy(&key_rem, &ab_mate_app.remote_key, sizeof(ab_mate_key_info_t));
    #if BT_TWS_EN
    if(sys_cb.tws_left_channel){
        key_left = &ab_mate_app.local_key;
        key_right = &ab_mate_app.remote_key;
    }else
    #endif
    {
        key_left = &ab_mate_app.remote_key;
        key_right = &ab_mate_app.local_key;
    }

    while(read_offset < payload_len){
        switch(payload[read_offset]){
            case KEY_LEFT_SHORT:
                TRACE("KEY_LEFT_SHORT\n");
                val_len = payload[read_offset + 1];
                if(key_left->key_short != payload[read_offset + 2]){
                    key_left->key_short = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_LEFT_SHORT;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_RIGHT_SHORT:
                TRACE("KEY_RIGHT_SHORT\n");
                val_len = payload[read_offset + 1];
                if(key_right->key_short != payload[read_offset + 2]){
                    key_right->key_short = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_RIGHT_SHORT;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_LEFT_DOUBLE:
                TRACE("KEY_LEFT_DOUBLE\n");
                val_len = payload[read_offset + 1];
                if(key_left->key_double != payload[read_offset + 2]){
                    key_left->key_double = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_LEFT_DOUBLE;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_RIGHT_DOUBLE:
                TRACE("KEY_RIGHT_DOUBLE\n");
                val_len = payload[read_offset + 1];
                if(key_right->key_double != payload[read_offset + 2]){
                    key_right->key_double = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_RIGHT_DOUBLE;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_LEFT_THREE:
                TRACE("KEY_LEFT_THREE\n");
                val_len = payload[read_offset + 1];
                if(key_left->key_three != payload[read_offset + 2]){
                    key_left->key_three = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_LEFT_THREE;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_RIGHT_THREE:
                TRACE("KEY_RIGHT_THREE\n");
                val_len = payload[read_offset + 1];
                if(key_right->key_three != payload[read_offset + 2]){
                    key_right->key_three = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_RIGHT_THREE;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_LEFT_LONG:
                TRACE("KEY_LEFT_LONG\n");
                val_len = payload[read_offset + 1];
                if(key_left->key_long != payload[read_offset + 2]){
                    key_left->key_long = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_LEFT_LONG;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            case KEY_RIGHT_LONG:
                TRACE("KEY_RIGHT_LONG\n");
                val_len = payload[read_offset + 1];
                if(key_right->key_long != payload[read_offset + 2]){
                    key_right->key_long = payload[read_offset + 2];
                }
                buf[write_offset++] = KEY_RIGHT_LONG;
                buf[write_offset++] = 1;
                buf[write_offset++] = AB_MATE_SUCCESS;
            break;

            default:
                val_len = payload[read_offset + 1];
                break;
        }
        read_offset += (2 + val_len);
    }

    if(memcmp(&key_loc, &ab_mate_app.local_key, sizeof(ab_mate_key_info_t))){
        ab_mate_key_set_do();
    }
#if BT_TWS_EN
    if(memcmp(&key_rem, &ab_mate_app.remote_key, sizeof(ab_mate_key_info_t))){
        ab_mate_tws_key_info_sync();
    }
#endif

    ab_mate_cmd_send.cmd_head.payload_len = write_offset;

    //TRACE("ab_mate_key_set:");
    //print_r(buf,write_offset);

    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + ab_mate_cmd_send.cmd_head.payload_len);
}

void ab_mate_language_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_LANG_EN
    ab_mate_result_t result = AB_MATE_SUCCESS;
    u8 lang = payload[0];

    if(sys_cb.lang_id != lang){
        func_bt_switch_voice_lang();
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_mode_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_LATENCY_EN

    ab_mate_result_t result = AB_MATE_SUCCESS;
    u8 mode = payload[0];   
    
    if(ab_mate_app.latency_mode != mode){
        if(mode == DEVICE_MODE_NORMAL){
#if ABMATE_MODE_SWITCH
            bsp_res_play(TWS_RES_SW_MUSIC_MODE);
#else 
            bsp_res_play(TWS_RES_MUSIC_MODE);
#endif
           
        }else if(mode == DEVICE_MODE_GAME){
#if ABMATE_MODE_SWITCH
            bsp_res_play(TWS_RES_SW_GAME_MODE);
#else 
            bsp_res_play(TWS_RES_GAME_MODE);
#endif
     
        }else{
            result = AB_MATE_FAIL;
        }
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_device_find(u8 *payload, u8 payload_len)
{
#if AB_MATE_DEVICE_FIND_EN
    ab_mate_result_t result = AB_MATE_SUCCESS;
    ab_mate_app.find_type = payload[0];

    switch (ab_mate_app.find_type) {
    case DEVICE_FIND_STOP:
        ab_mate_app.device_find = 0;
#if SWETZ_FIND_SYNC
        app_lr_send_notification(LR_NOTIFY_FIND_SYNC, 1, &ab_mate_app.find_type);
#endif
        soft_timer_stop(device_find_timer);
        TRACE("find stop\n");
        break;

    case DEVICE_FIND_START:
        ab_mate_app.device_find = 1;
#if SWETZ_FIND_SYNC
        app_lr_send_notification(LR_NOTIFY_FIND_SYNC, 1, &ab_mate_app.find_type);
#endif
        soft_timer_start(device_find_timer);
    TRACE("find start\n");
        break;

    case DEVICE_FIND_START_L:
    case DEVICE_FIND_STOP_L:
    case DEVICE_FIND_START_R:
    case DEVICE_FIND_STOP_R:
        TRACE("find side\n");
        #if AB_MATE_DEVICE_FIND_EN && BT_TWS_EN
        ab_mate_tws_device_find_sync();
        #endif
        ab_mate_device_find_side();
        break;

    default:
        result = AB_MATE_FAIL;
        break;
    }
    ab_mate_request_common_response(result);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

#if ABMATE_AUTO_ANSWER
void ab_mate_auto_answer(u8 *payload, u8 payload_len)
{
    ab_mate_result_t result = AB_MATE_SUCCESS;
    ab_mate_app.auto_answer_type = payload[0];

    memory_save_flag(PARAM_AUTO_ANSWER_ADDR,ab_mate_app.auto_answer_type);
    app_lr_send_notification(LR_NOTIFY_AUTO_ANSWER_STATUS, 1, &ab_mate_app.auto_answer_type);

    ab_mate_request_common_response(result);
}

#endif


void ab_mate_device_find_side(void)
{
#if AB_MATE_DEVICE_FIND_EN
    static u8 vol_bkp;
    bool start, execute;
    u8 type = ab_mate_app.find_type;
#if BT_TWS_EN
    u8 tws_left_channel = sys_cb.tws_left_channel;
#else
    u8 tws_left_channel = 0;
#endif
    start = (type == DEVICE_FIND_START_L || type == DEVICE_FIND_START_R);   //开始/停止
    execute = (tws_left_channel && (type == DEVICE_FIND_START_L || type == DEVICE_FIND_STOP_L)) || \
                (!tws_left_channel && (type == DEVICE_FIND_START_R || type == DEVICE_FIND_STOP_R));  //是否查找本机

    if (execute) {
        if (start) {
            if (!ab_mate_app.device_find) {
                vol_bkp = sys_cb.vol;
                ab_mate_app.device_find = 1;
                soft_timer_start(device_find_timer);
                TRACE("find side start\n");
            }
        } else {
            if (ab_mate_app.device_find) {
                ab_mate_app.device_find = 0;
                soft_timer_stop(device_find_timer);
                bsp_set_volume(vol_bkp);
                bsp_bt_vol_change();
                TRACE("find side stop\n");
            }
        }
    }
#endif
}

void ab_mate_power_off_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_POWER_OFF_EN

    u8 time_min = payload[0];
    printf("time_min:%d\r\n",time_min);
    if(time_min == 0){
        printf("AB_MATE_POWER_OFF_EN\r\n");
        ab_mate_app.poweroff_time = 1;
    }else if(time_min == 0xff){
        ab_mate_app.poweroff_time = 0xffff;

    }else{
        ab_mate_app.poweroff_time = time_min * 60;//time_min * 60 * 2;

    }
   // printf("ab_mate_app.poweroff_time:%d\r\n,ab_mate_app.poweroff_time");
#if SWETZ_POWERTIME_SYNC
    
        app_lr_send_notification(LR_NOTIFY_POWERTIME_SYNC, 1, &time_min);

#endif


    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_device_reset(void)
{

#if SWETZ_VOL_RESET
    ab_mate_app.auto_answer_type = 0;
    memory_save_flag(PARAM_AUTO_ANSWER_ADDR,ab_mate_app.auto_answer_type);
    app_lr_send_notification(LR_NOTIFY_AUTO_ANSWER_STATUS, 1, &ab_mate_app.auto_answer_type);    

   
#if SWETZ_RESET_NOT_POWER_TONE
    sys_cb.reset_Not_tone = 1;

    //app_lr_send_notification(LR_NOTIFY_NOT_POWERON_TONE, 1, &sys_cb.reset_Not_tone);
    memory_save_flag(PARAM_NOT_POWER_TONE_ADDR,sys_cb.reset_Not_tone);
    printf("sys_cb.reset_Not_tone:%d\r\n",sys_cb.reset_Not_tone);
    delay_5ms(280);
#endif

#endif

    ab_mate_app.do_flag |= FLAG_DEVICE_RESET;
    
#if BT_TWS_EN
    ab_mate_tws_device_reset_sync();
#endif
    //ab_mate_flag_do();
    ab_mate_request_common_response(AB_MATE_SUCCESS);
}

void ab_mate_in_ear_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_INERA_EN
    u8 en = payload[0];

    if(sys_cb.in_ear_en != en){
        sys_cb.in_ear_en = en;
#if BT_TWS_EN
        ab_mate_tws_in_ear_en_sync();
#endif
        ab_mate_cm_write(&sys_cb.in_ear_en, AB_MATE_CM_IN_EAR, 1, 2);
    }
    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_bt_name_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_BT_NAME_EN
    u8 tws_en = BT_TWS_EN * xcfg_cb.bt_tws_en;

    if(tws_en){
        if((bt_tws_is_connected() == false) && (BT_TWS_PAIR_MODE == 0)){
            ab_mate_request_common_response(AB_MATE_FAIL);
            return;
        }
    }
    if(payload_len > 31){
        payload_len = 31;
    }
    memset(xcfg_cb.bt_name, 0, sizeof(xcfg_cb.bt_name));
    memcpy(xcfg_cb.bt_name, payload, payload_len);
#if BT_TWS_EN
    ab_mate_tws_bt_name_sync();
#endif
    ab_mate_app.do_flag |= FLAG_BT_NAME_SET;
    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_bt_link_info_clear_do(void)
{
    if(bt_nor_is_connected()){
        delay_5ms(2);
        bt_nor_unpair_device();
        printf("bt_nor_unpair_device\r\n");
    }
}

void ab_mate_bt_link_info_clear(void)
{
    ab_mate_request_common_response(AB_MATE_SUCCESS);
    ab_mate_app.do_flag |= FLAG_BT_LINK_INFO_CLEAR;
#if BT_TWS_EN
    ab_mate_tws_link_clear_sync();
#endif
}

void ab_mate_led_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_LED_EN
    if(payload[0]){
        sys_cb.led_scan_en = 1;
    }else{
        sys_cb.led_scan_en = 0;
        bled_set_off();
        rled_set_off();
    }
#if BT_TWS_EN
    ab_mate_tws_led_info_sync();
#endif
    ab_mate_cm_write(&sys_cb.led_scan_en, AB_MATE_CM_LED_EN, 1, 2);
    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

#if AB_MATE_V3D_AUDIO_EN
void ab_mate_v3d_audio_notify(void)
{
    if(ab_mate_app.con_sta){
        u8 tlv_data[3] = {INFO_V3D_AUDIO, 1, 0x00};
        tlv_data[2] = ab_mate_app.v3d_audio_en;
        ab_mate_device_info_notify(tlv_data, sizeof(tlv_data));
    }
}

void ab_mate_v3d_audio_set_do(void)
{
    static u8 clk;
    if(ab_mate_app.v3d_audio_en){
        clk = sys_clk_get_cur();
        if(clk < SYS_60M){
            sys_clk_set(SYS_60M);
        }
        bt_music_audio_set_state(MUSIC_AUDIO_V3D);
    }else{
        bt_music_audio_set_state(MUSIC_AUDIO_NONE);
        sys_clk_set(clk);
    }
    ab_mate_cm_write(&ab_mate_app.v3d_audio_en, AB_MATE_CM_V3D_AUDIO, 1, 2);
}

void ab_mate_v3d_audio_set(u8 *payload, u8 payload_len)
{
    ab_mate_app.v3d_audio_en = payload[0];
#if BT_TWS_EN
    ab_mate_tws_v3d_audio_sync();
#endif
    ab_mate_app.do_flag |= FLAG_V3D_AUDIO_SET;
    ab_mate_request_common_response(AB_MATE_SUCCESS);
}
#endif

void ab_mate_call_ctrl(u8 *payload, u8 payload_len)
{
#if AB_MATE_CALL_EN
    if (payload[0]) {
        bt_call_terminate();
    } else {
        bt_call_answer_incoming();
    }
    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

void ab_mate_mic_ctrl(u8 *payload, u8 payload_len)
{
#if AB_MATE_MIC_EN
    if (payload[0]) {
        audio_path_init(AUDIO_PATH_BTMIC);
        audio_path_start(AUDIO_PATH_BTMIC);
    } else {
        audio_path_exit(AUDIO_PATH_BTMIC);
    }
    ab_mate_request_common_response(AB_MATE_SUCCESS);
#else
    ab_mate_request_common_response(AB_MATE_FAIL);
#endif
}

#if AB_MATE_MULT_DEV_EN

//库里面的weak函数，在外面进行强定义，调用bt_disconnect_address()后将会回调该函数进行地址的获取
uint8_t bt_get_disconnect_addr(uint8_t *bd_addr)
{
    memcpy(bd_addr, ab_mate_app.mult_dev.discon_addr, 6);

    return 1;
}

void ab_mate_mult_dev_addr_encrypt(u8 *bt_addr)
{
    u8 i;
    for(i=0; i<6; i++){
        bt_addr[i] ^= 0xAD;
    }
}
void ab_mate_mult_dev_addr_decrypt(u8 *bt_addr)
{
    ab_mate_mult_dev_addr_encrypt(bt_addr);
}

void ab_mate_mult_dev_en_set_do(void)
{
    u8 bt_set_sacn_val = 0;

    if(ab_mate_app.mult_dev.en){
        cfg_bt_max_acl_link = 2;
        bt_nor_link_ctrl_max_set(cfg_bt_max_acl_link);
        bt_set_sacn_val = 0x02;  //可连接
    }else{
        cfg_bt_max_acl_link = 1;
        bt_nor_link_ctrl_max_set(cfg_bt_max_acl_link);
        bt_set_sacn_val = 0;     //不可发现不可连接
    }
    bt_set_scan_force(bt_set_sacn_val);
    TRACE("mult dev set do, en:%d, max acl_link:%d\n", ab_mate_app.mult_dev.en, cfg_bt_max_acl_link);
    ab_mate_cm_write(&ab_mate_app.mult_dev.en, AB_MATE_CM_MULT_DEV_EN, 1, 2);
}

void ab_mate_mult_dev_discon_do(void)
{
    bt_disconnect_address();
}

void ab_mate_mult_dev_unpair_do(void)
{
    bt_disconnect_address();
    bt_nor_delete_link_info_by_addr(ab_mate_app.mult_dev.discon_addr, 0);
}

int ab_mate_connect_info_get(u8* buf)
{
    u8 bt_addr[6];
    char name[32];
    u8 name_len;
    u8 offset = 0;

    u8 cnt = bt_get_connected_num();
    buf[offset++] = cnt;

    TRACE("ab_mate_connect_info_get:%d\n",cnt);

    for(u8 i=0; i<cnt; i++){
        if(cnt == 1){
            bt_get_link_btaddr(0xff, bt_addr);
        }else{
            bt_get_link_btaddr(i, bt_addr);
        }
        TRACE("bt_addr:");
        TRACE_R(bt_addr, 6);
        memset(name, 0, 32);
        bt_nor_get_link_info_name(bt_addr, name, 32);
        TRACE("name:%s\n",name);
        name_len = strlen(name);
        ab_mate_mult_dev_addr_encrypt(bt_addr);
        buf[offset++] = name_len + 6 + 1; //name + addr + con_sta
        memcpy(&buf[offset], bt_addr, 6);
        offset += 6;
        buf[offset++] = 1;    //con_sta
        memcpy(&buf[offset], name, name_len);
        offset += name_len;
    }
    return offset;
}

int ab_mate_connect_info_set(u8* buf)
{
    u8 bt_addr[6];
    char name[32];
    u8 name_len;
    char name_loc[32];
    u8 offset = 0;

    u8 cnt = buf[offset++];

    TRACE("ab_mate_connect_info_set:%d\n",cnt);

    for(u8 i=0; i<cnt; i++){
        name_len = buf[offset] - 6 - 1;
        offset++;
        memcpy(bt_addr, &buf[offset], 6);
        ab_mate_mult_dev_addr_decrypt(bt_addr);
        offset += 7;
        TRACE("bt_addr:");
        TRACE_R(bt_addr, 6);
        memset(name, 0, 32);
        memcpy(name, &buf[offset], name_len);
        offset += name_len;
        TRACE("name:%s\n",name);
        memset(name_loc, 0, 32);
        bt_nor_get_link_info_name(bt_addr, name_loc, 32);
        if(memcmp(name_loc, name, 32)){
            TRACE("bt_put_link_name\n");
            bt_nor_put_link_name(bt_addr, name);
        }
    }

    return offset;
}

void ab_mate_connect_info_notify(void)
{
    if(ab_mate_app.con_sta){
        ab_mate_cmd_send.payload[0] = INFO_PAIRED_INFO;
        ab_mate_cmd_send.payload[1] = ab_mate_connect_info_get(&ab_mate_cmd_send.payload[2]);
        ab_mate_device_info_notify(ab_mate_cmd_send.payload, ab_mate_cmd_send.payload[1] + 2);
    }
    TRACE("ab_mate_connect_info_notify:\n");
    TRACE_R( ab_mate_cmd_send.payload, ab_mate_cmd_send.payload[1] + 2);
}

void ab_mate_mult_dev_set(u8 *payload, u8 payload_len)
{
    u8 read_offset = 0;
    u8 write_offerst = 0;
    u8 *rsp_buf = ab_mate_cmd_send.payload;
    u8 val_len = 0;
    u8 result = AB_MATE_SUCCESS;

    while(read_offset < payload_len){
        switch(payload[read_offset]){
            case MULT_DEV_CTR_EN:
                TRACE("MULT_DEV_CTR_EN\n");
                val_len = payload[read_offset + 1];
                //printf("val_len:%d\r\n",val_len);
                ab_mate_app.mult_dev.en = payload[read_offset + 2];
                //printf("ab_mate_app.mult_dev.en:%d\r\n",ab_mate_app.mult_dev.en);
                ab_mate_app.do_flag |= FLAG_MULT_DEV_SET;
               // printf("ab_mate_app.do_flag:%d\r\n",ab_mate_app.do_flag);
                result = AB_MATE_SUCCESS;
                break;

            case MULT_DEV_CTR_DISCON:{
                TRACE("MULT_DEV_CTR_DISCON\n");
                val_len = payload[read_offset + 1];
                memcpy(ab_mate_app.mult_dev.discon_addr, &payload[read_offset + 2], 6);
                ab_mate_mult_dev_addr_decrypt(ab_mate_app.mult_dev.discon_addr);
                u8 bt_addr[6];
                for(u8 i=0; i<2; i++){
                    bt_get_link_btaddr(i, bt_addr);
                    if(memcmp(ab_mate_app.mult_dev.discon_addr, bt_addr, 6)){
                        bt_nor_reorder_for_addr(bt_addr);
                    }
                }
                ab_mate_app.do_flag |= FLAG_MULT_DEV_DISCON_SET;
                result = AB_MATE_SUCCESS;
            } break;

            case MULT_DEV_CTR_UNPAIR:
                TRACE("MULT_DEV_CTR_UNPAIR\n");
                val_len = payload[read_offset + 1];
                memcpy(ab_mate_app.mult_dev.discon_addr, &payload[read_offset + 2], 6);
                ab_mate_mult_dev_addr_decrypt(ab_mate_app.mult_dev.discon_addr);
                ab_mate_app.do_flag |= FLAG_MULT_DEV_UNPAIR_SET;
                result = AB_MATE_SUCCESS;
                break;

            case MULT_DEV_CTR_APP_DEV_ADDR:
                TRACE("MULT_DEV_CTR_APP_DEV_ADDR\n");
                val_len = payload[read_offset + 1];
                memcpy(ab_mate_app.mult_dev.app_dev_addr, &payload[read_offset + 2], 6);
                ab_mate_mult_dev_addr_decrypt(ab_mate_app.mult_dev.app_dev_addr);
                result = AB_MATE_SUCCESS;
                break;

            default:
                val_len = payload[read_offset + 1];
                result = AB_MATE_FAIL;
            break;
        }
        rsp_buf[write_offerst++] = payload[read_offset];
        rsp_buf[write_offerst++] = 1;
        rsp_buf[write_offerst++] = result;
        read_offset += (2 + val_len);
    }

    #if BT_TWS_EN
        ab_mate_tws_mult_dev_info_sync();
    #endif

    TRACE("ab_mate_mult_dev_set_response:");
    TRACE_R(ab_mate_cmd_send.payload, write_offerst);
    ab_mate_cmd_send.cmd_head.payload_len = write_offerst;
    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + ab_mate_cmd_send.cmd_head.payload_len);
}
#endif

#if AB_MATE_BT_STA_EN
void ab_mate_bt_sta_proc(soft_timer_p timer)  //bt状态/号码上报
{
    static u8 status = 0;
    static u8 number_len = 0;
    printf("ab_mate_app.con_sta:%d\r\n",ab_mate_app.con_sta);
    if (ab_mate_app.con_sta) {
        u8 sta = bt_get_disp_status();
        u8 len = 4;
        if (sta != status) {
            TRACE("BT_STA_NOTIFY[%d->%d]\n", status, sta);
            status = sta;
            u8 tlv_data[4] = {INFO_BT_STA, 2, sta, 0};
            ab_mate_device_info_notify(tlv_data, len);
            if (sta == BT_STA_INCOMING || sta == BT_STA_OUTGOING) {
                number_len = 0;
            } else {
                bt_redial_init();  //重置号码
            }
        }
        if (number_len == 0 && (sta == BT_STA_INCOMING || sta == BT_STA_OUTGOING)) { //上报号码
            const char *number = bt_get_last_call_number(bt_tws_is_connected() ? 1 : 0);
            if (number) {
                u8 tlv_data[16] = {INFO_BT_STA, 2, sta, 0};
                memcpy(&tlv_data[4], number, 12);
                number_len = min(11, strlen(number));
                tlv_data[1] += number_len;
                tlv_data[3] += number_len;
                len += number_len;
                ab_mate_device_info_notify(tlv_data, len);
            }
        }
    } else {
        status = 0; //连接断开，保证回连后立即上报bt状态
    }
    //printf("------------------------------------------------status:%d\r\n",status);
}

void ab_mate_bt_sta_timer_creat(void)
{
    soft_timer_create(&bt_sta_timer, 500, TIMER_REPEATED, ab_mate_bt_sta_proc);
}
#endif // AB_MATE_BT_STA_EN

#if AB_MATE_CUSTOM_CMD_EN
void ab_mate_custom_cmd_notify(u8* payload, u8 len)
{
    ab_mate_notify_do(CMD_CUSTOM, payload, len);
}

void ab_mate_music_info_notify(u8 type, u16 data_offset, u8* payload, u8 len)
{
    payload[0] = CUSTOM_CMD_MUSIC_CTRL;
    payload[1] = len + 2;
    payload[2] = type;
    payload[3] = len;
    memcpy(&payload[4], &payload[data_offset], len);

    ab_mate_custom_cmd_notify(payload, len + 4);
}

#if (BT_HFP_TIME_EN || BT_MAP_EN)
void ab_mate_time_info_notify(u8 type)
{
    u8 len = 0;
    u8 payload[16] = {CUSTOM_CMD_TIME_SYNC, len + 2, type, len};
    switch (type) {
    case CMD_TIME_TIMEZONE:
        len = 1;
        break;

    case CMD_TIME_TIMESTAMP_UTC_GMT:
        len = 4;
        break;

    case CMD_TIME_LOCALTIME:
        len = 7;
#if BT_HFP_TIME_EN
        if (!hfp_at_time_get(&payload[4], len)) {
#endif
#if BT_MAP_EN
            if (!bt_map_time_get(&payload[4], len)) {
                return;
            }
#else
            return;
#endif
#if BT_HFP_TIME_EN
        }
#endif
        break;
    }
    payload[1] = len + 2;
    payload[3] = len;
    ab_mate_time_get_cnt = 0;
    ab_mate_custom_cmd_notify(payload, len + 4);
}

static void ab_mate_get_real_time_process(void)
{
    static u32 map_ticks = 1;
    if (tick_check_expire(map_ticks, 5000)) {
        map_ticks = tick_get();
        if(bt_nor_is_connected()) {
            if (!bt_tws_is_slave() && ab_mate_time_get_cnt) {
                printf("---map timer---\r\n");
                #if BT_HFP_TIME_EN
                    hfp_at_kick();
                #endif
                #if BT_MAP_EN
                    bt_map_kick();
                #endif
                ab_mate_time_get_cnt--;
            }
        } else {
            ab_mate_time_get_cnt = AB_MATE_MAP_GET_CNT;
        }
    }
}
#endif //(BT_HFP_TIME_EN || BT_MAP_EN)

void ab_mate_hid_ctrl(u8 *payload, u8 payload_len)
{
    u8 hid_cmd = payload[0];

    switch (hid_cmd) {
        case CMD_HID_SLIDE_UP:
            msg_enqueue(EVT_HID_SLIDE_UP);
            break;
        case CMD_HID_SLIDE_DOWN:
            msg_enqueue(EVT_HID_SLIDE_DOWN);
            break;
        case CMD_HID_SLIDE_LEFT:
            break;
        case CMD_HID_SLIDE_RIGHT:
            break;
        case CMD_HID_P:
             msg_enqueue(EVT_HID_P);
            break;
        case CMD_HID_PP:
            msg_enqueue(EVT_HID_PP);
            break;
        case CMD_HID_TAKE_PIC:
            msg_enqueue(EVT_HID_TAKE_PIC);
            break;
    }

    ab_mate_request_common_response(0);
}

void ab_mate_custom_request_receive_proc(u8 *payload, u8 payload_len)
{
    u8 custom_cmd = payload[0];
    switch (custom_cmd) {
        case CUSTOM_CMD_HID_CTRL:
            ab_mate_hid_ctrl(&payload[2], payload[1]);
            break;
        case CUSTOM_CMD_SIRI:
            bt_hfp_siri_switch();
            ab_mate_request_common_response(0);
            break;
        case CUSTOM_CMD_TONE_CTRL:
            ab_mate_cmd_send.cmd_head.payload_len = 3;
            ab_mate_cmd_send.payload[0] = payload[0];   //tlv-type
            ab_mate_cmd_send.payload[1] = 1;    //tlv-length
#if AB_MATE_CUSTOM_CMD_TONE_EN
            ab_mate_app.custom_tone_type = payload[2];
            switch (ab_mate_app.custom_tone_type) {
            case TONE_CTRL_COUNTDWON_PLAY:
                soft_timer_start(custom_cmd_tone_timer);
                break;

            case TONE_CTRL_COUNTDWON_STOP:
                soft_timer_stop(custom_cmd_tone_timer);
                break;
            }
            ab_mate_cmd_send.payload[2] = AB_MATE_SUCCESS;   //tlv-value
#else
            ab_mate_cmd_send.payload[2] = AB_MATE_FAIL;   //tlv-value
#endif
            ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + 3);
            break;
#if AB_MATE_CUSTOM_CMD_CALL_EN
        case CUSTOM_CMD_PHONE_CALL:
            if(bt_nor_is_connected() && !bt_tws_is_slave()) {
                printf("call:%s\n", &payload[2]);

				void hfp_at_dial(char *number);
                hfp_at_dial((char *)(&payload[2]));
            }
            ab_mate_request_common_response(0);
            break;
#endif
    }
}

#endif // AB_MATE_CUSTOM_CMD_EN


#if ABMATE_ANC_SWITCH
void ab_mate_anc_new_set(u8 *payload, u8 payload_len)
{
#if AB_MATE_ANC_EN
    u8 read_offset = 0;
    u8 write_offset = 0;
    u8 *buf = ab_mate_cmd_send.payload;
    u8 val_len = 0;

    while(read_offset < payload_len)
    {
        switch(payload[read_offset])
        {
            case 0x01://设置降噪模式
            printf("TEST22============\r\n");
            val_len = payload[read_offset + 1];
            buf[write_offset++] = 0x01;
            buf[write_offset++] = 1;
            if ((val_len == 1) && (payload[read_offset + 2] <= APP_ANC_TRANSPARENCY) && f_bt.disp_status < BT_STA_INCOMING)
            {
                printf("TEST66============\r\n");
                buf[write_offset++] = AB_MATE_SUCCESS;
                ab_mate_app.anc_set = payload[read_offset + 2] + 1;
              //  ab_mate_app.anc_mode_notify_device = FIIL_ANC_MODE_NOTIFY_BOX;
            }
            else
            {
                buf[write_offset++] = AB_MATE_FAIL;
            }
            break;

             case 0x02://设置降噪深度
             printf("TEST22============\r\n");
            // val_len = payload[read_offset + 1];
            // buf[write_offset++] = 0x02;
            // buf[write_offset++] = 1;
            // if ((val_len == 1) && (payload[read_offset + 2] < FIIL_ANC_LEVEL_NUM))
            // {
            //     buf[write_offset++] = AB_MATE_SUCCESS;
            //     if (ab_mate_app.fiil_anc_level != payload[read_offset + 2])
            //     {
            //         if (payload[read_offset + 2] == FIIL_ANC_LEVEL_HIGH)
            //         {
            //             bsp_res_play(TWS_RES_TONE_SILENCE_ANC_LEVEL_HIGH);
            //         }
            //         else if (payload[read_offset + 2] == FIIL_ANC_LEVEL_MIDDLE)
            //         {
            //             bsp_res_play(TWS_RES_TONE_SILENCE_ANC_LEVEL_MIDDLE);
            //         }
            //         else if (payload[read_offset + 2] == FIIL_ANC_LEVEL_LOW)
            //         {
            //             bsp_res_play(TWS_RES_TONE_SILENCE_ANC_LEVEL_LOW);
            //         }
            //     }
            // }
            // else
            // {
            //     buf[write_offset++] = AB_MATE_FAIL;
            // }
             break;

            default:
            val_len = payload[read_offset + 1];
            break;
        }
        read_offset += (2 + val_len);
    }

    ab_mate_cmd_send.cmd_head.payload_len = write_offset;
    ab_mate_data_send((u8*)&ab_mate_cmd_send, AB_MATE_HEADER_LEN + ab_mate_cmd_send.cmd_head.payload_len);
#endif
}
#endif


AT(.com_text.ab_mate)
bool ab_mate_system_need_wakeup(void)
{
    return (ab_mate_app.wakeup == 1);
}

void ab_mate_request_receive_proc(u8 cmd,u8 *payload,u8 payload_len)
{
    ab_mate_cmd_send.cmd_head.cmd = cmd;
    ab_mate_cmd_send.cmd_head.cmd_type = CMD_TYPE_RESPONSE;
    ab_mate_cmd_send.cmd_head.frame_seq = 0;
    ab_mate_cmd_send.cmd_head.frame_total = 0;
    printf("ab_mate_cmd:0x%x\r\n",cmd);
    switch(cmd){
        case CMD_EQ_SET:
            ab_mate_eq_set(payload, payload_len);
            break;

        case CMD_MUSIC_SET:
            ab_mate_music_set(payload, payload_len);
            break;

        case CMD_KEY_SET:
            ab_mate_key_set(payload, payload_len);
            break;

        case CMD_POWER_OFF_SET:

            ab_mate_power_off_set(payload, payload_len);
            break;

        case CMD_DEVICE_RESET:
            ab_mate_device_reset();
            break;

        case CMD_MODE_SET:
            ab_mate_mode_set(payload, payload_len);
            break;

        case CMD_IN_EAR_SET:
            ab_mate_in_ear_set(payload, payload_len);
            break;

        case CMD_DEVICE_INFO_GET:
            ab_mate_device_info_query(payload, payload_len);
            break;

        case CMD_LANGUAGE_SET:
            ab_mate_language_set(payload, payload_len);
            break;

        case CMD_DEVICE_FIND:
            ab_mate_device_find(payload,payload_len);
            break;

        case CMD_AUTO_ANSWER_SET:
#if ABMATE_AUTO_ANSWER
            ab_mate_auto_answer(payload,payload_len);
#endif

            break;

        case CMD_ANC_SET:
            ab_mate_anc_set(payload, payload_len);
            break;

        case CMD_BT_NAME_SET:
            ab_mate_bt_name_set(payload, payload_len);
            break;

        case CMD_LED_SET:
            ab_mate_led_set(payload, payload_len);
            break;

        case CMD_BT_LINK_INFO_CLEAR:
            ab_mate_bt_link_info_clear();
            break;

        case CMD_ANC_LEVEL_SET:
            ab_mate_anc_level_set(payload, payload_len);
            
            break;

        case CMD_TP_LEVEL_SET:
            ab_mate_tp_level_set(payload, payload_len);
            break;

#if AB_MATE_V3D_AUDIO_EN
        case CMT_V3D_AUDIO_SET:
            ab_mate_v3d_audio_set(payload, payload_len);
            break;
#endif

#if AB_MATE_MULT_DEV_EN
        case CMD_MULT_DEV_SET:
            ab_mate_mult_dev_set(payload, payload_len);
            break;
#endif

#if ABMATE_ANC_SWITCH
        case CMD_ANC_NEW:
        printf("CMD_ANC_NEW======\r\n");
        ab_mate_anc_new_set(payload, payload_len);
        break;
#endif

        case CMD_CALL_CTRL:
            ab_mate_call_ctrl(payload, payload_len);
            break;

        case CMD_MIC_CTRL:
            ab_mate_mic_ctrl(payload, payload_len);
            break;

#if AB_MATE_CUSTOM_CMD_EN
        case CMD_CUSTOM:
            ab_mate_custom_request_receive_proc(payload, payload_len);
            break;
#endif

        default:
            break;
    }
}

void ab_mate_receive_proc_do(void)
{
    switch(ab_mate_cmd_recv.cmd_head.cmd_type){
        case CMD_TYPE_REQUEST:
            ab_mate_request_receive_proc(ab_mate_cmd_recv.cmd_head.cmd, ab_mate_cmd_recv.payload, ab_mate_cmd_recv.total_len);
            break;

        case CMD_TYPE_RESPONSE:
            break;

        case CMD_TYPE_NOTIFY:
            break;

        default:
            break;
    }
}

bool ab_mate_receive_proc(u8 *data,u16 len, u8 con_type)
{
    // TRACE("ab_mate_receive_proc[%d]:",len);
    // TRACE_R(data,8);

    ab_mate_cmd_head_t* cmd_head = (ab_mate_cmd_head_t*)data;
    u8 *p_data = &data[AB_MATE_PAYLOAD_POS];
    u8 payload_len = cmd_head->payload_len;

    if(cmd_head->cmd_type > CMD_TYPE_NOTIFY){
        return false;
    }

    if(payload_len != (len - AB_MATE_HEADER_LEN)){
        return false;
    }

    if(ab_mate_cmd_recv.next_header_seq != cmd_head->seq){
        TRACE("--->header_seq_err:%d,%d\n",ab_mate_cmd_recv.next_header_seq,cmd_head->seq);
        ab_mate_cmd_recv.next_header_seq = cmd_head->seq;
#if AB_MATE_OTA_EN
        if(ab_mate_ota_is_start()){
            ab_mate_ota_seq_err_notify();
            ab_mate_cmd_recv.next_header_seq++;
            return false;
        }
#endif
    }

    if(ab_mate_app.con_sta == AB_MATE_CON_NONE){
        ab_mate_app.con_sta = con_type;
#if BT_TWS_EN
        ab_mate_tws_connect_sta_sync();
#endif
        if(ab_mate_app.con_sta == AB_MATE_CON_SPP){
            ab_mate_disable_adv_for_spp_connect();
        }
    }

    ab_mate_app.wakeup = 1;

    ab_mate_cmd_recv.next_header_seq++;

    if(cmd_head->frame_total == 0){
        if(cmd_head->cmd >= CMD_OTA_REQ && cmd_head->cmd <= CMD_OTA_STA){
            //TRACE("ab_mate_receive[%d]:",len);
            //TRACE_R(data,len);
#if AB_MATE_OTA_EN
            ab_mate_ota_proc(cmd_head->cmd, p_data, payload_len);
#endif
        }else{
            TRACE("ab_mate_receive[%d]:",len);
            TRACE_R(data,len);
            memcpy(&ab_mate_cmd_recv.cmd_head, cmd_head, AB_MATE_HEADER_LEN);
            memcpy(&ab_mate_cmd_recv.payload, p_data, payload_len);
            ab_mate_cmd_recv.total_len = payload_len;
            ab_mate_receive_proc_do();
        }
    }else{
        if(ab_mate_cmd_recv.next_frame_seq != cmd_head->frame_seq){
            TRACE("--->frame_seq_err\n");
            ab_mate_cmd_recv.next_frame_seq = 0;
            return false;
        }
        ab_mate_cmd_recv.next_frame_seq++;
        memcpy(&ab_mate_cmd_recv.payload[ab_mate_cmd_recv.recv_len], p_data, payload_len);
        ab_mate_cmd_recv.recv_len += payload_len;
        if(cmd_head->frame_seq == cmd_head->frame_total){
            memcpy(&ab_mate_cmd_recv.cmd_head, cmd_head, AB_MATE_HEADER_LEN);
            ab_mate_cmd_recv.total_len = ab_mate_cmd_recv.recv_len;
            ab_mate_cmd_recv.next_frame_seq = 0;
            ab_mate_cmd_recv.recv_len = 0;
            ab_mate_receive_proc_do();
        }
    }

    return true;
}

AT(.com_rodata.bat)
const char powertine[] = "powertine: %d\n";

AT(.com_text.ab_mate)
void ab_mate_poweroff_proc(void)
{
    
#if AB_MATE_POWER_OFF_EN
    if(ab_mate_app.poweroff_time == 0xffff){
        reset_pwroff_delay();
    }else if(ab_mate_app.poweroff_time){
        printf(powertine,ab_mate_app.poweroff_time);
        ab_mate_app.poweroff_time--;
        reset_pwroff_delay();
        if(ab_mate_app.poweroff_time == 0){
            sys_cb.discon_reason = 0xff;   //同步关机
            sys_cb.pwrdwn_tone_en = 1;
            func_cb.sta = FUNC_PWROFF;
        }
    }
   printf(powertine,ab_mate_app.poweroff_time);
#endif
}
#if AB_MATE_EQ_EN && AB_MATE_EQ_FOR_IDX_EN
void ab_mate_eq_overall_gain_set(void)
{
    switch(ab_mate_app.eq_info.mode){
    case 0:     //normal
        music_set_eq_overall_gain(0);
        break;
    case 1:     //pop
        music_set_eq_overall_gain(0);
        break;
    case 2:     //rock
        music_set_eq_overall_gain(0);
        break;
    case 3:     //jazz
        music_set_eq_overall_gain(-1);
        break;
    case 4:     //classic
        music_set_eq_overall_gain(-5);
        break;
    case 5:     //country
        music_set_eq_overall_gain(0);
        break;
    default:
        music_set_eq_overall_gain(0);
        break;
    }
}
#endif
u8 ab_mate_eq_set_do(void)
{
    printf("ab_mate_app.eq_info.mode:%d f_bt.disp_status:%d \r\n",ab_mate_app.eq_info.mode,f_bt.disp_status);
    
#if AB_MATE_EQ_EN
#if AB_MATE_EQ_USE_RES
    if(ab_mate_app.eq_info.mode < AB_MATE_EQ_RES_CNT){
#if SWETZ_EQ_GAIN_RESET
    for(u8 i=0; i<ab_mate_app.eq_info.band_cnt; i++){
        WDT_CLR();
        music_set_eq_for_index(i,0);
    }
    music_set_eq_overall_gain(0);
#endif

        while (!tick_check_expire(ab_mate_app.tick, 30)) {     //连续两次EQ设置相隔30ms，连续短时间设置容易失败
            delay_5ms(1);
        }
        sys_cb.eq_app_total_gain_en = 0;
        music_set_eq_by_num(ab_mate_app.eq_info.mode);
        ab_mate_app.tick = tick_get();
        return 1;
    }
#endif  
#if AB_MATE_EQ_FOR_IDX_EN
    for(u8 i=0; i<ab_mate_app.eq_info.band_cnt; i++){
        WDT_CLR();
        music_set_eq_for_index(i,(int)ab_mate_app.eq_info.gain[i]);
    }
    sys_cb.eq_app_total_gain_en = 1;
    ab_mate_eq_overall_gain_set();

    while (!tick_check_expire(ab_mate_app.tick, 30)) {          //连续两次EQ设置相隔30ms，连续短时间设置容易失败
        delay_5ms(1);
    }

    music_set_eq_for_index_do();
#endif
    ab_mate_app.tick = tick_get();

    return 1;
#endif
    return 0;
}

void ab_mate_key_init(void)
{
#if AB_MATE_KEY_EN
    if (ab_mate_app.cm_flag == AB_MATE_CM_TAG) {
        ab_mate_cm_read(&ab_mate_app.local_key, AB_MATE_CM_KEY_LOCAL, sizeof(ab_mate_key_info_t));
        ab_mate_cm_read(&ab_mate_app.remote_key, AB_MATE_CM_KEY_REMOTE, sizeof(ab_mate_key_info_t));
    } else {
        ab_mate_key_info_t *key_left;
        ab_mate_key_info_t *key_right;
        #if BT_TWS_EN
        if(sys_cb.tws_left_channel){
            key_left = &ab_mate_app.local_key;
            key_right = &ab_mate_app.remote_key;
        }else
        #endif
        {
            key_left = &ab_mate_app.remote_key;
            key_right = &ab_mate_app.local_key;
        }
#if AB_MATE_KEY_USER_DEF_EN
        key_left->key_short = KEY_SHORT_LEFT_DEF;
        key_right->key_short = KEY_SHORT_RIGHT_DEF;
        key_left->key_double = KEY_DOUBLE_LEFT_DEF;
        key_right->key_double = KEY_DOUBLE_RIGHT_DEF;
        key_left->key_three = KEY_THREE_LEFT_DEF;
        key_right->key_three = KEY_THREE_RIGHT_DEF;
#if SWETZ_APP_TEST
        key_left->key_four  =  KEY_FOUR_LEFT_DEF;
        key_right->key_four  =  KEY_FOUR_RIGHT_DEF;
#endif
        key_left->key_long = KEY_LONG_LEFT_DEF;
        key_right->key_long = KEY_LONG_RIGHT_DEF;
#else
        key_left->key_short = key_local2app_table[AB_MATE_KEY_SHORT];
        key_right->key_short = key_local2app_table[AB_MATE_KEY_SHORT];
        key_left->key_double = key_local2app_table[AB_MATE_KEY_DOUBLE];
        key_right->key_double = key_local2app_table[AB_MATE_KEY_DOUBLE];
        key_left->key_three = key_local2app_table[AB_MATE_KEY_THREE];
        key_right->key_three = key_local2app_table[AB_MATE_KEY_THREE];
        key_left->key_long = key_local2app_table[AB_MATE_KEY_LONG];
        key_right->key_long = key_local2app_table[AB_MATE_KEY_LONG];
#endif

        ab_mate_cm_write(&ab_mate_app.local_key, AB_MATE_CM_KEY_LOCAL, sizeof(ab_mate_key_info_t), 0);
        ab_mate_cm_write(&ab_mate_app.remote_key, AB_MATE_CM_KEY_REMOTE, sizeof(ab_mate_key_info_t), 0);
    }
#if AB_MATE_KEY_USER_DEF_EN
    AB_MATE_KEY_SHORT = key_app2local_table[ab_mate_app.local_key.key_short];
    AB_MATE_KEY_DOUBLE = key_app2local_table[ab_mate_app.local_key.key_double];
    AB_MATE_KEY_THREE = key_app2local_table[ab_mate_app.local_key.key_three];
#if SWETZ_APP_TEST
    AB_MATE_KEY_FOUR = key_app2local_table[ab_mate_app.local_key.key_four];
#endif
    AB_MATE_KEY_LONG = key_app2local_table[ab_mate_app.local_key.key_long];
#endif
#endif
}




void ab_mate_mode_set_do(void)
{
#if AB_MATE_LATENCY_EN
    bool low_latency = bt_is_low_latency();

    if(ab_mate_app.latency_mode == DEVICE_MODE_GAME){
        if(low_latency == false){
            bt_low_latency_enable();
        }
    }else if(ab_mate_app.latency_mode == DEVICE_MODE_NORMAL){
        if(low_latency == true){
            bt_low_latency_disable();
        }
    }
#endif
}

void ab_mate_anc_set_do(void)
{
#if AB_MATE_ANC_EN
    bsp_anc_set_mode(sys_cb.anc_user_mode);
#endif
}

void ab_mate_anc_init(void)
{
#if AB_MATE_ANC_EN
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        bsp_param_read(&sys_cb.anc_user_mode, PARAM_ANC_NR_STA, 1);
        #if AB_MATE_ANC_LEVEL
            ab_mate_cm_read(&ab_mate_app.anc_cur_level, AB_MATE_CM_ANC_LEVEL, 1);
            ab_mate_cm_read(&ab_mate_app.tp_cur_level, AB_MATE_CM_TP_LEVEL, 1);
        #endif
    }else{
        sys_cb.anc_user_mode = 0;
        bsp_param_write(&sys_cb.anc_user_mode, PARAM_ANC_NR_STA, 1);
        #if AB_MATE_ANC_LEVEL
            ab_mate_app.anc_cur_level = 0;
            ab_mate_cm_write(&ab_mate_app.anc_cur_level, AB_MATE_CM_ANC_LEVEL, 1, 0);
            ab_mate_app.tp_cur_level = 0;
            ab_mate_cm_write(&ab_mate_app.tp_cur_level, AB_MATE_CM_TP_LEVEL, 1, 0);
        #endif
    }
    ab_mate_app.anc_mode = anc_app_table[sys_cb.anc_user_mode];
    #if AB_MATE_ANC_LEVEL
        ab_mate_app.anc_total_level = AB_MATE_ANC_TOTAL_LEVEL;
        ab_mate_app.tp_total_level = AB_MATE_TP_TOTAL_LEVEL;
        bsp_param_read((u8*)&ab_mate_app.anc_adjust_mic0, PARAM_ANC_MIC0_VAL, 1);
        bsp_param_read((u8*)&ab_mate_app.anc_adjust_mic1, PARAM_ANC_MIC1_VAL, 1);
        bsp_param_read((u8*)&ab_mate_app.tp_adjust_mic0, PARAM_ANC_TP_MIC0_VAL, 1);
        bsp_param_read((u8*)&ab_mate_app.tp_adjust_mic1, PARAM_ANC_TP_MIC1_VAL, 1);
    #endif
#endif
}

void ab_mate_eq_init(void)
{

#if AB_MATE_EQ_EN
    ab_mate_app.eq_info.band_cnt = AB_MATE_EQ_BAND_CNT;

    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        ab_mate_cm_read(&ab_mate_app.eq_info.mode, AB_MATE_CM_EQ_DATA, 1 + AB_MATE_EQ_BAND_CNT);
#if AB_MATE_EQ_USE_DEVICE
        ab_mate_cm_read(&eq_mode_custom, AB_MATE_CM_EQ_CUSTOM1, sizeof(eq_mode_custom));
#endif
    }else{
        // ab_mate_app.eq_info.mode = 0;
        // memcpy(ab_mate_app.eq_info.gain, eq_all_mode_info.eq_normal, AB_MATE_EQ_BAND_CNT);
#if SWETZ    
        ab_mate_app.eq_info.mode = 4;
        memcpy(ab_mate_app.eq_info.gain, eq_all_mode_info.eq_classic, AB_MATE_EQ_BAND_CNT);  
#endif      
        ab_mate_cm_write(&ab_mate_app.eq_info.mode, AB_MATE_CM_EQ_DATA, 1 + AB_MATE_EQ_BAND_CNT, 0);
#if AB_MATE_EQ_USE_DEVICE
        memset(&eq_mode_custom, 0, sizeof(eq_mode_custom));
        ab_mate_cm_write(&eq_mode_custom, AB_MATE_CM_EQ_CUSTOM1, sizeof(eq_mode_custom), 0);
#endif
    }
#endif
}

void ab_mate_bt_name_init(void)
{
#if AB_MATE_BT_NAME_EN
    u8 bt_name_len;
    u8 bt_name_tag;

    ab_mate_cm_read(&bt_name_tag, AB_MATE_CM_BT_NAME_FLAG, 1);

    if((ab_mate_app.cm_flag == AB_MATE_CM_TAG) && (bt_name_tag == AB_MATE_BT_NAME_TAG)){
        ab_mate_cm_read(&bt_name_len, AB_MATE_CM_BT_NAME_LEN, 1);
        memset(xcfg_cb.bt_name, 0, sizeof(xcfg_cb.bt_name));
        ab_mate_cm_read(xcfg_cb.bt_name, AB_MATE_CM_BT_NAME, bt_name_len);
    }
#endif
}

void ab_mate_latency_mode_init(void)
{
#if AB_MATE_LATENCY_EN
    #if AB_MATE_LATENCY_USE_CM
        if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
            ab_mate_cm_read(&ab_mate_app.latency_mode, AB_MATE_CM_MODE, 1);
        }else{
            ab_mate_app.latency_mode = DEVICE_MODE_NORMAL;
            ab_mate_cm_write(&ab_mate_app.latency_mode, AB_MATE_CM_MODE, 1, 0);
        }
    #else
        ab_mate_app.latency_mode = DEVICE_MODE_NORMAL;
    #endif
#endif
}

void ab_mate_lang_init(void)
{
#if AB_MATE_LANG_EN
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        param_lang_id_read();
    }else{
        if (xcfg_cb.lang_id == 2) {
            sys_cb.lang_id = 0;             //出厂默认英文
        } else if (xcfg_cb.lang_id == 3) {
            sys_cb.lang_id = 1;             //出厂默认中文
        } else {
            sys_cb.lang_id = xcfg_cb.lang_id;
        }
        param_lang_id_write();
    }
#endif
}

void ab_mate_in_ear_init(void)
{
#if AB_MATE_INERA_EN
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        ab_mate_cm_read(&sys_cb.in_ear_en, AB_MATE_CM_IN_EAR, 1);
    }else{
        sys_cb.in_ear_en = 1;
        ab_mate_cm_write(&sys_cb.in_ear_en, AB_MATE_CM_IN_EAR, 1, 0);
    }
#endif
}

void ab_mate_led_init(void)
{
#if AB_MATE_LED_EN
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        ab_mate_cm_read(&sys_cb.led_scan_en, AB_MATE_CM_LED_EN, 1);
    }else{
        sys_cb.led_scan_en = 1;
        ab_mate_cm_write(&sys_cb.led_scan_en, AB_MATE_CM_LED_EN, 1, 0);
    }
#endif
}

#if AB_MATE_V3D_AUDIO_EN
void ab_mate_v3d_audio_init(void)
{
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        ab_mate_cm_read(&ab_mate_app.v3d_audio_en, AB_MATE_CM_V3D_AUDIO, 1);
        if(ab_mate_app.v3d_audio_en){
            ab_mate_app.do_flag |= FLAG_V3D_AUDIO_SET;
        }
    }else{
        ab_mate_app.v3d_audio_en = 0;
        ab_mate_cm_write(&ab_mate_app.v3d_audio_en, AB_MATE_CM_V3D_AUDIO, 1, 0);
    }
}
#endif

#if AB_MATE_MULT_DEV_EN
void ab_mate_mult_dev_init(void)
{
    if(ab_mate_app.cm_flag == AB_MATE_CM_TAG){
        ab_mate_cm_read(&ab_mate_app.mult_dev.en, AB_MATE_CM_MULT_DEV_EN, 1);
        TRACE("app mate init--------->mult_dev.en:%d\n", ab_mate_app.mult_dev.en);
        ab_mate_app.do_flag |= FLAG_MULT_DEV_SET;
    }else{
        ab_mate_cm_write(&ab_mate_app.mult_dev.en, AB_MATE_CM_MULT_DEV_EN, 1, 0);
    }

}
#endif
void ab_mate_device_reset_do(void)
{
    u8 bt_name_tag = 0xFF;
    ab_mate_cm_write((u8*)&bt_name_tag, AB_MATE_CM_BT_NAME_FLAG, 1, 2);

    ab_mate_app.cm_flag = 0xFFFF;
    ab_mate_cm_write((u8*)&ab_mate_app.cm_flag, AB_MATE_CM_FLAG, 2, 1);

    

    ab_mate_bt_link_info_clear_do();
    sys_cb.vol = SYS_INIT_VOLUME;
    param_sys_vol_write();
    cm_sync();
    printf("ab_mate_device_reset_do\r\n");
    delay_5ms(6);
    WDT_RST();
}

void ab_mate_var_init(void)
{
	static u8 init_flag = 0;

    char *version = SW_VERSION;

    memset(&ab_mate_cmd_recv, 0, sizeof(ab_mate_cmd_recv_t));
    memset(&ab_mate_cmd_send, 0, sizeof(ab_mate_cmd_send_t));
    memset(&ab_mate_app, 0, sizeof(ab_mate_app_var_t));

    ab_mate_cmd_send.cmd_head.seq = 0xf;

    ab_mate_app.version[0] = version[5] - 0x30;
    ab_mate_app.version[1] = version[3] - 0x30;
    ab_mate_app.version[2] = version[1] - 0x30;
    ab_mate_app.version[3] = 0;
#if AB_MATE_OTA_EN
    ab_mate_app.flash_crc = fot_head_crc_get();
#endif
#if AB_MATE_BT_NAME_EN
    memcpy(ab_mate_app.bt_name, xcfg_cb.bt_name, 32);
#endif

    ab_mate_cm_read((u8*)&ab_mate_app.cm_flag, AB_MATE_CM_FLAG, 2);
	#if BT_TWS_EN
    tws_lr_xcfg_sel();
	#endif
    ab_mate_key_init();
#if AB_MATE_OTA_EN
    ab_mate_ota_init();
#endif
    ab_mate_eq_init();
    ab_mate_anc_init();
    ab_mate_bt_name_init();
    ab_mate_latency_mode_init();
    ab_mate_lang_init();
    ab_mate_in_ear_init();
    ab_mate_led_init();
#if AB_MATE_V3D_AUDIO_EN
    ab_mate_v3d_audio_init();
#endif
#if AB_MATE_MULT_DEV_EN
    ab_mate_mult_dev_init();
#endif
    if(init_flag == 0){
        init_flag = 1;
    }else{
        ab_mate_app.init_flag = 1;
    }
}

void ab_mate_init_do(void)
{
    soft_timer_init();
    ab_mate_cm_timer_creat();
#if AB_MATE_PP_EN
    ab_mate_play_sta_timer_creat();
#endif

#if AB_MATE_DEVICE_FIND_EN
    ab_mate_device_find_timer_creat();
#endif
#if AB_MATE_BT_STA_EN
    ab_mate_bt_sta_timer_creat();
    soft_timer_start(bt_sta_timer);
#endif
#if AB_MATE_CUSTOM_CMD_TONE_EN
    ab_mate_custom_cmd_tone_timer_creat();
#endif
    ab_mate_vbat_timer_creat();
    soft_timer_start(vbat_timer);


    ab_mate_mode_set_do();
    ab_mate_anc_set_do();

    if(!sco_is_connected()){
        ab_mate_eq_set_do();
    }

    if(ab_mate_app.cm_flag != AB_MATE_CM_TAG){
        ab_mate_app.cm_flag = AB_MATE_CM_TAG;
        ab_mate_cm_write((u8*)&ab_mate_app.cm_flag, AB_MATE_CM_FLAG, 2, 1);
    }
    ab_mate_app.init_flag = 1;
}

AT(.text.ab_mate.process)
void ab_mate_device_info_update(void)
{
#if AB_MATE_ANC_EN
    if(ab_mate_app.anc_mode != anc_app_table[sys_cb.anc_user_mode]){
        ab_mate_app.anc_mode = anc_app_table[sys_cb.anc_user_mode];
        ab_mate_anc_mode_notify();
    }
#endif

#if AB_MATE_LATENCY_EN
    if(ab_mate_app.latency_mode != bt_is_low_latency()){
        ab_mate_app.latency_mode = bt_is_low_latency();
    #if AB_MATE_LATENCY_USE_CM
        ab_mate_cm_write(&ab_mate_app.latency_mode, AB_MATE_CM_MODE, 1, 2);
    #endif
        ab_mate_device_mode_notify();
    }
#endif

#if AB_MATE_VOL_EN
    if(ab_mate_app.vol != sys_cb.vol){
        ab_mate_app.vol = sys_cb.vol;
        ab_mate_vol_notify();
    }
#endif

#if AB_MATE_V3D_AUDIO_EN
    u8 v3d_audio_en = (bt_music_audio_get_state() == MUSIC_AUDIO_NONE) ? 0 : 1;
    if(ab_mate_app.v3d_audio_en != v3d_audio_en){
        if(!(ab_mate_app.do_flag & FLAG_V3D_AUDIO_SET)){
            ab_mate_app.v3d_audio_en = v3d_audio_en;
            ab_mate_v3d_audio_notify();
        }
    }
#endif
}

AT(.text.ab_mate.flag)
void ab_mate_flag_do(void)
{
    if(ab_mate_app.do_flag & FLAG_EQ_SET){
        if(!sco_is_connected()){
            ab_mate_app.do_flag &= ~FLAG_EQ_SET;
            ab_mate_eq_set_do();
        }
    }

    if(ab_mate_app.do_flag & FLAG_DEVICE_RESET){
        ab_mate_app.do_flag &= ~FLAG_DEVICE_RESET;
        ab_mate_device_reset_do();
    }

    if(ab_mate_app.do_flag & FLAG_BT_LINK_INFO_CLEAR){
        ab_mate_app.do_flag &= ~FLAG_BT_LINK_INFO_CLEAR;
        ab_mate_bt_link_info_clear_do();
    }

    if(ab_mate_app.do_flag & FLAG_BT_NAME_SET){
        ab_mate_app.do_flag &= ~FLAG_BT_NAME_SET;
        u8 name_len = strlen(xcfg_cb.bt_name);
        u8 name_tag = AB_MATE_BT_NAME_TAG;
        bt_set_stack_local_name(xcfg_cb.bt_name);
        ab_mate_cm_write(xcfg_cb.bt_name, AB_MATE_CM_BT_NAME, name_len, 2);
        ab_mate_cm_write(&name_len, AB_MATE_CM_BT_NAME_LEN, 1, 2);
        ab_mate_cm_write(&name_tag, AB_MATE_CM_BT_NAME_FLAG, 1, 2);
    }

    if(ab_mate_app.do_flag & FLAG_BT_ROLE_SWITCH){
        ab_mate_app.do_flag &= ~FLAG_BT_ROLE_SWITCH;
        delay_5ms(2);
        if(bt_tws_is_slave()){

        }else{
            ab_mate_update_ble_adv_bt_sta(1, 1);
            if(ab_mate_app.con_sta == AB_MATE_CON_SPP){
                ab_mate_disable_adv_for_spp_connect();
            }
        }
    }

#if AB_MATE_V3D_AUDIO_EN
    if(ab_mate_app.do_flag & FLAG_V3D_AUDIO_SET){
        ab_mate_app.do_flag &= ~FLAG_V3D_AUDIO_SET;
        ab_mate_v3d_audio_set_do();
    }
#endif

#if AB_MATE_MULT_DEV_EN
    if(ab_mate_app.do_flag & FLAG_MULT_DEV_SET){
        ab_mate_app.do_flag &= ~FLAG_MULT_DEV_SET;
        ab_mate_mult_dev_en_set_do();
    }
    if(ab_mate_app.do_flag & FLAG_MULT_DEV_DISCON_SET){
        ab_mate_app.do_flag &= ~FLAG_MULT_DEV_DISCON_SET;
        ab_mate_mult_dev_discon_do();
    }
    if(ab_mate_app.do_flag & FLAG_MULT_DEV_UNPAIR_SET){
        ab_mate_app.do_flag &= ~FLAG_MULT_DEV_UNPAIR_SET;
        ab_mate_mult_dev_unpair_do();
    }
#endif

    if(ab_mate_app.do_flag & FLAG_TWS_STA_NOTIFY){
        ab_mate_app.do_flag &= ~FLAG_TWS_STA_NOTIFY;
        ab_mate_tws_con_sta_notify(bt_tws_is_connected());
    }
    printf("-------------------------------------------------RESTE\r\n");
}

AT(.text.ab_mate.process)
void ab_mate_process(void)
{
    soft_timer_run();

#if AB_MATE_PP_EN
    ab_mate_play_sta_report_proc();
#endif

    if(ab_mate_app.do_flag){
        ab_mate_flag_do();
    }

    ab_mate_device_info_update();

#if AB_MATE_OTA_EN
    ab_mate_ota_process();
#endif

#if (BT_HFP_TIME_EN || BT_MAP_EN)
    ab_mate_get_real_time_process();
#endif

    ab_mate_app.wakeup = 0;
#if SWETZ


#if ABMATE_ANC_SWITCH
    if (ab_mate_app.anc_set)
    {
        if ((!bsp_res_is_playing()) && (bsp_res_is_empty()))
        {
            u8 anc_mode = ab_mate_app.anc_set - 1;

            if (sys_cb.anc_user_mode != anc_mode)
            {
                if (anc_mode == APP_ANC_STOP)
                {
                    bsp_res_play(TWS_RES_SW_ANC_OFF);
                }
                else if (anc_mode == APP_ANC_START)
                {
                    bsp_res_play(TWS_RES_SW_ANC_ON);
                }
                else if (anc_mode == APP_ANC_TRANSPARENCY)
                {
                    bsp_res_play(TWS_RES_ANC_OPEN);
                }
            }
            ab_mate_app.anc_set = 0;
        }
    }
#endif






#endif
}

void ab_mate_connect_proc(u8 sync)
{
    if(ab_mate_app.con_sta == AB_MATE_CON_NONE){
        memset(&ab_mate_cmd_recv, 0, sizeof(ab_mate_cmd_recv_t));
        memset(&ab_mate_cmd_send, 0, sizeof(ab_mate_cmd_send_t));

        ab_mate_cmd_send.cmd_head.seq = 0xf;

        ab_mate_app.can_send_now = 0;
    }
}

void ab_mate_disconnect_proc(u8 sync)
{
    if(ab_mate_app.con_sta){
#if AB_MATE_OTA_EN
        ab_mate_ota_disconnect_callback();
#endif

#if AB_MATE_DEVICE_FIND_EN
        if(ab_mate_app.device_find){
            ab_mate_app.device_find = 0;
            soft_timer_stop(device_find_timer);
        }
#endif

        ab_mate_app.con_sta = AB_MATE_CON_NONE;
        ab_mate_app.can_send_now = 0;
#if BT_TWS_EN
        if(sync){
            ab_mate_tws_connect_sta_sync();
        }
#endif
    }
}

void ab_mate_ble_connect_callback(void)
{
#if !AB_MATE_BT_ATT_EN
    ab_mate_connect_proc(1);
    ble_update_conn_param(AB_MATE_CON_INTERVAL, 0, 400);
#endif
#if AB_MATE_ADV0_EN
    ble_adv0_set_ctrl(0);
    // ble_adv_dis(); // can't perform successful
#endif
}

void ab_mate_ble_disconnect_callback(void)
{
#if !AB_MATE_BT_ATT_EN
    if(ab_mate_app.con_sta == AB_MATE_CON_BLE){
        ab_mate_disconnect_proc(1);
    }
#endif
#if AB_MATE_ADV0_EN
    ble_adv0_set_ctrl(1);
#endif
}

#if AB_MATE_BT_ATT_EN
void bt_latt_connect_callback(uint16_t conn_handle)
{
    printf("[BT]: latt conn\n");
    ab_mate_connect_proc(1);
}

void bt_latt_disconnect_callback(uint16_t conn_handle)
{
    printf("[BT]: latt disconn\n");
    if(ab_mate_app.con_sta == AB_MATE_CON_BLE){
        ab_mate_disconnect_proc(1);
    }
}
#endif

WEAK void ab_mate_disable_adv_for_spp_connect(void)
{
    if(ble_get_status() == LE_STA_ADVERTISING){
        printf("--------------ble_adv_dis\r\n");
        ble_adv_dis();
    }
    printf("0000112");
}

WEAK void ab_mate_enable_adv_for_spp_disconnect(void)
{
    if((ble_get_status() != LE_STA_CONNECTION) && !bt_tws_is_slave()){
        ble_adv_en();
        printf("0000112");
    }
}

void ab_mate_spp_connect_callback(void)
{
    ab_mate_connect_proc(1);
}

void ab_mate_spp_disconnect_callback(void)
{
    if(ab_mate_app.con_sta == AB_MATE_CON_SPP){
        ab_mate_enable_adv_for_spp_disconnect();
        ab_mate_disconnect_proc(1);
    }
}

#if BT_TWS_EN
void ab_mate_tws_connect_callback(void)
{
    if(bt_tws_is_slave() == 0) {
        ab_mate_tws_info_all_sync();
    }
#if AB_MATE_ADV0_EN
    else {
        ble_adv0_set_ctrl(0);
    }
#endif

    ab_mate_app.do_flag |= FLAG_TWS_STA_NOTIFY;
}

void ab_mate_tws_disconnect_callback(void)
{
    ab_mate_app.remote_vbat = 0;

#if AB_MATE_OTA_EN
	ab_mate_ota_disconnect_callback();
#endif
    if(!bt_tws_is_slave()){
        if(!bt_nor_is_connected()){
            ab_mate_update_ble_adv_bt_sta(0, 1);
            printf("ab_mate_update_ble_adv_bt_sta-----0\r\\n");
            
        }

    }


#if !GFPS_EN
    if(!ab_mate_app.con_sta && !ble_is_connect()){
        ble_adv_en();
    }
#endif

    ab_mate_app.do_flag |= FLAG_TWS_STA_NOTIFY;
}

void ab_mate_tws_role_switch_callback(u8 role)
{
    ab_mate_app.do_flag |= FLAG_BT_ROLE_SWITCH;

    if(role){
       TRACE("-->role switch to slave\n");
    }else{
       TRACE("-->role switch to master\n");
    }
}

#endif

void ab_mate_bt_connect_callback(void)
{
    ab_mate_update_ble_adv_bt_sta(1, 1);
    printf("ab_mate_bt_connect_callback\r\n");
#if AB_MATE_MULT_DEV_EN
    ab_mate_connect_info_notify();
#if BT_TWS_EN
    ab_mate_tws_mult_dev_info_sync();
#endif
#endif
}

void ab_mate_bt_disconnect_callback(u8 *packet)
{

        if(!bt_nor_is_connected()){
        ab_mate_update_ble_adv_bt_sta(0, 1);
        printf("ab_mate_update_ble_adv_bt_sta\r\n");
    }
#if !AB_MATE_CUSTOM_CMD_EN
    ab_mate_update_ble_adv_bt_sta(0, 1);
#if AB_MATE_MULT_DEV_EN
    ab_mate_connect_info_notify();
#if BT_TWS_EN
    ab_mate_tws_mult_dev_info_sync();
#endif
    if((ab_mate_app.con_sta == AB_MATE_CON_BLE) &&
        !memcmp(&packet[2], ab_mate_app.mult_dev.app_dev_addr, 6)){
        ble_disconnect();
        memset(ab_mate_app.mult_dev.app_dev_addr, 0, 6);
    }
#else
    if(ab_mate_app.con_sta == AB_MATE_CON_BLE){
        ble_disconnect();
    }
#endif
#endif  //AB_MATE_CUSTOM_CMD_EN
}

uint16_t ab_mate_ble_role_switch_get_data(uint8_t *data_ptr)
{
    return 0;
}

uint16_t ab_mate_ble_role_switch_set_data(uint8_t *data_ptr, uint16_t len)
{
    return 0;
}

uint16_t role_switch_get_user_data(uint8_t *data_ptr)
{
    u8 offset = 0;

    if(ab_mate_app.can_send_now){
        data_ptr[offset++] = ab_mate_app.con_sta;
        data_ptr[offset++] = ab_mate_cmd_recv.next_header_seq;
        data_ptr[offset++] = ab_mate_cmd_send.cmd_head.seq;
		data_ptr[offset++] = ab_mate_app.device_find;
		memcpy(&data_ptr[offset], &ab_mate_app.local_key, sizeof(ab_mate_key_info_t));
		offset += sizeof(ab_mate_key_info_t);

        TRACE_R(data_ptr,offset);
    }

	ab_mate_disconnect_proc(0);

    return offset;
}

uint16_t role_switch_set_user_data(uint8_t *data_ptr, uint16_t len)
{
    u8 offset = 0;

    if(!ab_mate_app.con_sta && len){
        ab_mate_app.con_sta = data_ptr[offset++];
        ab_mate_connect_proc(0);

        ab_mate_app.rem_con_sta = 0;
        ab_mate_app.can_send_now = 1;
        ab_mate_cmd_recv.next_header_seq = data_ptr[offset++];
        ab_mate_cmd_send.cmd_head.seq = data_ptr[offset++];
		ab_mate_app.device_find = data_ptr[offset++];
		memcpy(&ab_mate_app.remote_key, &data_ptr[offset], sizeof(ab_mate_key_info_t));
		offset += sizeof(ab_mate_key_info_t);

        if(ab_mate_app.device_find){
            soft_timer_start(device_find_timer);
        }

        TRACE_R(data_ptr,offset);
    }

    return offset;
}

void ab_mate_bt_evt_notice(uint evt, void *params)
{
#if BT_TWS_EN
    u8 *packet = params;
#endif
    switch(evt){
        case BT_NOTICE_INIT_FINISH:
            ble_set_adv_interval(AB_MATE_ADV_INTERVAL);
#if AB_MATE_ADV0_EN
            ble_adv0_set_ctrl(1);
#endif
            break;

        case BT_NOTICE_DISCONNECT:
            ab_mate_bt_disconnect_callback(params);
            break;

        case BT_NOTICE_CONNECTED:
            ab_mate_bt_connect_callback();
            break;

#if BT_TWS_EN
        case BT_NOTICE_TWS_DISCONNECT:
            ab_mate_tws_disconnect_callback();
            break;

        case BT_NOTICE_TWS_CONNECTED:
            ab_mate_tws_connect_callback();
            break;

        case BT_NOTICE_TWS_ROLE_CHANGE:
            ab_mate_tws_role_switch_callback(packet[0]);
            break;
#endif
    }
}

void ab_mate_enter_sleep(void)
{
    if(ble_get_status() == LE_STA_ADVERTISING){
        printf("0000112");
        ble_set_adv_interval(AB_MATE_ADV_SLEEP_INTERVAL);
    }else if(ble_get_status() == LE_STA_CONNECTION){
        printf("0000112");
        if (!ab_mate_app.update_param_flag) {
            ble_update_conn_param(AB_MATE_CON_SLEEP_INTERVAL, 0, 400);
            ab_mate_app.update_param_flag = 1;
        }
    }
}

void ab_mate_exit_sleep(void)
{
    if(ble_get_status() == LE_STA_ADVERTISING){
        printf("0000112");
        ble_set_adv_interval(AB_MATE_ADV_INTERVAL);
    }
}

#endif
