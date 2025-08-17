#include "include.h"
#include "api.h"

typedef struct {
    u32 anov_thres;
    u8  sta;
    u8  stable_cnt;
    u16 cnt;
} te_cali_cb_t;

tk_cb_t tk_cb;
tk_cb_t tk1_cb;
tk_cb_t te_cb;
te_cali_cb_t te_cali_cb;
u16 tkcnt_tmr[6];       //每个通路的实时TKCNT
u8  tk_ch_temp;
#if USER_TKEY_DEBUG_EN
typedef struct {
    u16 tk_buf[16];
    u16 te_buf[16];
    char spp_buf[160];
    u8  tk_idx;
    u8  ch;
volatile u8 tk_print_sta;
    u32 ticks;
} spp_tkcb_t;
spp_tkcb_t spp_tkcb AT(.tkey_buf.debug);

AT(.rodata.tkey)
const char tkcnt_str[] = "[TK] State(%01x), TKBCNT(%03x):\n";
AT(.rodata.tkey)
const char tecnt_str[] = "[TE] State(%01x), TEBCNT(%03x):\n";

AT(.rodata.tkey)
const char tkbcnt_str[] = "TKBCNT: %x\n";
AT(.rodata.tkey)
const char tkpend_str[] = "TK ERR: %x\n";
AT(.rodata.tkey)
const char tebcnt_str[] = "TEBCNT: %x\n";
AT(.rodata.tkey)
const char tepend_str[] = "TE ERR: %x\n";

void spp_inpcon_tx(void)
{
    int c_len = 0;

    if (tk_pnd_cb.flag & BIT(0)) {
        tk_pnd_cb.flag &= ~BIT(0);
        c_len += sprintf(spp_tkcb.spp_buf+c_len, tepend_str, tk_pnd_cb.te_exp);
    }
    if (tk_pnd_cb.flag & BIT(1)) {
        tk_pnd_cb.flag &= ~BIT(1);
        c_len += sprintf(spp_tkcb.spp_buf+c_len, tebcnt_str, tk_pnd_cb.tebcnt);
    }
    if (tk_pnd_cb.flag & BIT(2)) {
        tk_pnd_cb.flag &= ~BIT(2);
        c_len += sprintf(spp_tkcb.spp_buf+c_len, tkpend_str, tk_pnd_cb.tk_exp);
    }
    if (tk_pnd_cb.flag & BIT(3)) {
        tk_pnd_cb.flag &= ~BIT(3);
        c_len += sprintf(spp_tkcb.spp_buf+c_len, tkbcnt_str, tk_pnd_cb.tkbcnt);
    }
    if (c_len) {
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tkcb.spp_buf, c_len);
    }
}

//通过蓝牙SPP打印输出, 避免UART飞线干扰
void spp_tkcnt_tx(void)
{
    u16 tk_buf[8], te_buf[8];
    int c_len = 0, i;

    if (spp_tkcb.tk_print_sta) {
        memset(spp_tkcb.spp_buf, 0, sizeof(spp_tkcb.spp_buf));
        memcpy(tk_buf, &spp_tkcb.tk_buf[(spp_tkcb.tk_print_sta - 1)*8], 16);
        memcpy(te_buf, &spp_tkcb.te_buf[(spp_tkcb.tk_print_sta - 1)*8], 16);
        spp_tkcb.tk_print_sta = 0;
        if (tk_cb.ch != 0xff) {
            c_len += sprintf(spp_tkcb.spp_buf + c_len, tkcnt_str, tkey_get_key(), (u16)(TKBCNT & 0xffff));
            for (i = 0; i < 8; i++) {
                c_len += sprintf(spp_tkcb.spp_buf + c_len, "%04x ", tk_buf[i] & 0xffff);
            }
            c_len += sprintf(spp_tkcb.spp_buf + c_len, "\n");
        }
#if USER_TKEY_INEAR
        if (te_cb.ch != 0xff) {
            c_len += sprintf(spp_tkcb.spp_buf + c_len, tecnt_str, tkey_is_inear(), (u16)(TEBCNT & 0xffff));
            for (i = 0; i < 8; i++) {
                c_len += sprintf(spp_tkcb.spp_buf + c_len, "%04x ", te_buf[i] & 0xffff);
            }
            c_len += sprintf(spp_tkcb.spp_buf + c_len, "\n");
        }
#endif
        //printf((char *)spp_tkcb.spp_buf);
        bt_spp_tx(SPP_SERVICE_CH0, (uint8_t *)spp_tkcb.spp_buf, c_len);
    }
}

void bsp_tkey_spp_tx(void)
{
    spp_tkcnt_tx();                     //输出TKCNT
//    if (tick_check_expire(spp_tkcb.ticks, 100)) {
//        spp_tkcb.ticks = tick_get();
//        spp_inpcon_tx();                //输出一些状态
//    }
}

#endif // USER_TKEY_DEBUG_EN

#if USER_TKEY
void tkey_timeout_calibration(tk_cb_t *s, u16 tkcnt);
//中断调用
AT(.com_text.tkey.isr)
void tkey_tkcnt_isr(u32 tk_ch, u16 tkcnt)
{
    tkcnt_tmr[tk_ch] = tkcnt;

    if (tk_ch == te_cb.ch) {
        tkey_bcnt_range_exception(&te_cb, NULL, tkcnt);
#if USER_TKEY_DEBUG_EN
        spp_tkcb.te_buf[spp_tkcb.tk_idx] = tkcnt;
#endif
    } else if (tk_ch == tk_cb.ch) {
        tkey_bcnt_range_exception(&tk_cb, &tk1_cb, tkcnt);
        tkey_timeout_calibration(&tk_cb,tkcnt);
#if USER_TKEY_DEBUG_EN
        spp_tkcb.tk_buf[spp_tkcb.tk_idx] = tkcnt;
#endif
    }
#if USER_TKEY_SLIDE
    else if (tk_ch == tk1_cb.ch) {
        tkey_bcnt_range_exception(&tk1_cb, &tk_cb, tkcnt);
        tkey_timeout_calibration(&tk1_cb,tkcnt);
    }
#endif
#if USER_TKEY_TEMP_EN
    else if (tk_ch == 5) {						//温度通道
    }
#endif
#if USER_TKEY_DEBUG_EN
    if (tk_ch == spp_tkcb.ch) {
        spp_tkcb.tk_idx++;
        if (spp_tkcb.tk_idx == 8) {
            spp_tkcb.tk_print_sta = 1;
        } else if (spp_tkcb.tk_idx >= 16) {
            spp_tkcb.tk_print_sta = 2;
            spp_tkcb.tk_idx = 0;
        }
    }
#endif
}

/*****************************************************************************
* 1. 函数功能：充电时校准触摸的基准值BCNT
* 2. 确保无触摸情况下调用（入仓且合盖可以确保未触摸），min_avg_cnt可以用小值。
* 3. 不能确保有无触摸时被调用，min_avg_cnt * 8ms需要超过10秒以上。
* 4. 此函数不需要频繁调用，建议10秒以上间隔。
******************************************************************************/
AT(.text.bsp.tkey)
void bsp_charge_bcnt_calibration(u32 min_avg_cnt)
{
#if USER_TKEY_INEAR
    if (te_cb.ch != 0xff) {
//        printf("calibration: %d, %d\n", te_cb.anov_cnt, min_avg_cnt);
        if (te_cb.anov_cnt > 100) {
            if ((!te_cali_cb.sta) && (te_cali_cb.stable_cnt < 3)) {
                RTCCON8 |= BIT(1);                      //RI_CHSTOPS = 1, 延时400ms校准
                te_cali_cb.cnt = 120;
                te_cali_cb.anov_thres = 50;             //与延时时间长短有关
                te_cali_cb.sta = 1;
            }
        }
    }
#endif
    if (tk_cb.ch != 0xff) {
        if (tk_cb.anov_cnt > min_avg_cnt) {             //tkcnt是否稳定？
            u16 tkbcnt = TKBCNT & 0xffff;
            if (s_abs((s16)tkbcnt - (s16)tk_cb.avg) > 2) {
                TKBCNT = tk_cb.avg;
            }
        }
    }
}

AT(.text.bsp.charge)
void tkey_proc_calibration_in_charge(void)
{
    u16 cali_val;
    u16 cur_tebcnt = TEBCNT & 0xffff;

    if (te_cali_cb.sta) {
        if (te_cali_cb.cnt) {
            te_cali_cb.cnt--;
        } else {
            cali_val = te_cb.avg;
            RTCCON8 &= ~BIT(1);                         //RI_CHSTOPS = 0;
            te_cali_cb.sta = 0;
            if (te_cb.anov_cnt > te_cali_cb.anov_thres) {
                if (s_abs((s16)cur_tebcnt - (s16)cali_val) > 2) {
                    if (tk_ch_temp != 0xff) {
                        te_cb.temp_tkcnt = tkcnt_tmr[tk_ch_temp];
                    }
                    TEBCNT = cali_val;
//                    printf("tebcnt calibration: %x\n", cali_val);
                } else {
                    te_cali_cb.stable_cnt++;
                }
            } else {
                te_cali_cb.stable_cnt = 0;
            }
        }
    }
}

AT(.text.bsp.tkey)
void tkey_stop_calibration_in_charge(void)
{
    if (te_cb.ch != 0xff) {
        if (te_cali_cb.sta) {
            RTCCON8 &= ~BIT(1);                         //RI_CHSTOPS = 0;
        }
        memset(&te_cali_cb, 0, sizeof(te_cali_cb));
        bsp_tebcnt_temp_calibration_stop();
    }
}

AT(.text.bsp.tkey)
void bsp_tebcnt_temp_calibration_start(void)
{
    if ((te_cb.ch != 0xff) && (tk_ch_temp != 0xff)){
        te_cb.range_en = 0;
        TKCON &= ~BIT(29);
        te_cb.temp_tkcnt = tkcnt_tmr[tk_ch_temp];
        te_cb.tick = tick_get();
//        printf("calibration start\n");
    }
}

AT(.text.bsp.tkey)
void bsp_tebcnt_temp_calibration(void)
{
    if ((te_cb.ch != 0xff) && (tk_ch_temp != 0xff)) {
        u16 cur_tkcnt = tkcnt_tmr[tk_ch_temp];
        te_cb.tick = tick_get();
        if (te_temp_bcnt_calibration(cur_tkcnt, te_cb.temp_tkcnt)) {
            te_cb.temp_tkcnt = cur_tkcnt;
        }
    }
}

AT(.text.bsp.tkey)
void bsp_tebcnt_temp_calibration_stop(void)
{
    if ((te_cb.ch != 0xff) && (tk_ch_temp != 0xff)) {
        bsp_tebcnt_temp_calibration();
        delay_5ms(2);
        TKCON |= BIT(29);
        te_cb.range_en = 1;
//        printf("calibration stop: %x\n", TEBCNT & 0xffff);
    }
}

void bsp_tkey_channel_set(void)
{
    int i;
    for (i = 0; i < 5; i++) {
        tkey_ch_t *ch = (tkey_ch_t *)tkey_cfg.key[i];
        if (ch) {
            if (ch->type == 2) {
                te_cb.ch = i;
            } else if (ch->type == 1) {
            	if (tk_cb.ch == 0xff) {
                   tk_cb.ch = i;
                } else if (tk1_cb.ch == 0xff) {
                   tk1_cb.ch = i;
                }
            }
#if USER_TKEY_DEBUG_EN
            spp_tkcb.ch = i;
#endif
        }
    }
#if USER_TKEY_TEMP_EN
	tk_ch_temp = 5;				//temp ch5
#endif
}

AT(.com_text.tkey)
bool tkey_is_pressed(void)
{
    if (tk_cb.ch == 0xff) {
        return false;
    }
    if (!tkey_get_key()) {
        return false;
    }
#if USER_TKEY_SLIDE
    if (tk1_cb.ch != 0xff) {
        if (tk1_cb.tkey_sta || tk_cb.tkey_sta) {
            return false;
        }
        if (tk1_cb.slide || tk_cb.slide) {
            return false;
        }
    }
#endif
    return true;
}

#if USER_TKEY_SLIDE
AT(.com_text.tkey)
void tkey_slide_msg(u8 silde_up)
{
    if (silde_up == 0) {
        msg_enqueue(KU_VOL_UP);
    } else {
        msg_enqueue(KU_VOL_DOWN);
    }
}
#endif

AT(.com_text.tkey)
u8 bsp_tkey_scan(void)
{
    u8 key = NO_KEY;

    if (tkey_is_pressed()) {
        key = (USER_TKEY_SOFT_PWR_EN) ? KEY_PLAY_PWR_USER_DEF : KEY_PLAY_USER_DEF;
    }
    return key;
}

#endif // USER_TKEY

void tkey_press_timeout_clr(void)
{
    tk_pnd_cb.flag = 0;
    tk_pnd_cb.tk_exp = 0;
}

void tkey_press_timeout_process(void)
{
    if ((tk_pnd_cb.flag & BIT(2)) && (tk_pnd_cb.tk_exp & BIT(4))) {
        TKBCNT = tkcnt_tmr[tk_cb.ch];
    }
}

void bsp_tkey_var_init(void)
{
    memset(&tk_cb, 0, sizeof(tk_cb));
    memset(&tk1_cb, 0, sizeof(tk_cb));
    memset(&te_cb, 0, sizeof(te_cb));
    memset(&te_cali_cb, 0, sizeof(te_cali_cb));
#if USER_TKEY_DEBUG_EN
    memset(&spp_tkcb, 0, sizeof(spp_tkcb));
    spp_tkcb.ch = 0xff;
#endif
    tk_cb.limit = 28;
    tk_cb.range_thresh = TKRTHRESH;
    tk_cb.ch = 0xff;
    tk_cb.fil_except = FIL_EXCEPT;
    tk_cb.bcnt_sfr = (psfr_t)(&TKBCNT);
    tk_cb.range_en = 0;                                 //软件range校准是否使能
    tk_cb.to_cnt = TO_EXCEPT;                           //10秒
    tk_cb.slide_interval_min = 10;
    tk_cb.slide_interval_max = 250;
    memcpy(&tk1_cb, &tk_cb, sizeof(tk_cb_t));

    te_cb.te_flag = 1;
    te_cb.limit = 30;
    te_cb.range_thresh = TERTHRESH - 2;
    te_cb.ch = 0xff;
    te_cb.bcnt_sfr = (psfr_t)(&TEBCNT);
    te_cb.fil_except = EAR_FIL_EXCEPT;
    te_cb.range_en = 1;                                 //软件range校准是否使能

    tk_ch_temp = 0xff;
}

void bsp_tkey_init(void)
{
    bsp_tkey_var_init();
#if USER_TKEY
    int res;
    tkey_cfg_t tkey_cfg_v;

    memcpy(&tkey_cfg_v, &tkey_cfg, sizeof(tkey_cfg_t));
    bsp_tkey_channel_set();
#if USER_TKEY_TEMP_EN
    if (tk_ch_temp != 0xff) {
        tkey_cfg_v.key[tk_ch_temp] = &tkey_temp_cali_ch;
    }
#endif
    {   ///重新TK TIMEOUT的配置值
        u32 i, dlypr = 0, to_exp, pto_exp;
        u32 pr_us;
        for (i = 0; i < 6; i++) {
            tkey_ch_t *ch = (tkey_ch_t *)tkey_cfg_v.key[i];
            if (ch != NULL) {
                dlypr += ch->dlypr;
            }
        }
        pr_us = (1000000 * 2048) / sys_get_rc2m_rtc_clk();
        to_exp = TO_EXCEPT*8*1000 / (pr_us*dlypr);                  //根据所有通道PR计算timeout exception的配置值
        pto_exp = ((PTO_EXCEPT << 8)*8*1000 / (pr_us*dlypr)) >> 8;  //根据所有通道PR计算按下timerout的配置值
        if (pto_exp > 15) {
            pto_exp = 15;
        }
        tkey_cfg_v.pto_except = pto_exp;
        tkey_cfg_v.to_except = to_exp;
        tk_cb.to_cnt = to_exp;
        //printf("tkdlypr: %d, %d, %d\n", dlypr, to_exp, pto_exp);
    }

    res = tkey_init((void *)&tkey_cfg_v, sys_cb.rtc_first_pwron);
    if (tk_cb.ch != 0xff) {
        if (!res || sys_cb.inbox_wken_flag || (RTCCON9 & BIT(4)) || (!(RTCCON9 & BIT(6)))) {
            delay_5ms(5);
            TKBCNT = tkcnt_tmr[tk_cb.ch];
        }
    }
    if (te_cb.ch != 0xff) {
        delay_5ms(4);
        TEBCNT = tkcnt_tmr[te_cb.ch];
    }
#else
    tkey_sw_reset();
#endif // USER_TKEY
}

bool bsp_tkey_wakeup_en(void)
{
#if USER_TKEY
    if (tkey_cfg.key[0] != NULL) {		//ch0跟wko同一个io
        return true;
    } else
#endif
    {
        return false;
    }
}


