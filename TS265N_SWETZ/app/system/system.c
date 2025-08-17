#include "include.h"

xcfg_cb_t xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);
volatile int micl2gnd_flag;
volatile u32 ticks_50ms;
uint8_t cfg_spiflash_speed_up_en = FLASH_SPEED_UP_EN;        //SPI FLASH提速。部份FLASH不支持提速
uint8_t cfg_pmu_vddio_lp_enable = SYS_VDDIO_LP_EN;

void sd_detect(void);
void tbox_uart_isr(void);
void testbox_init(void);
bool exspiflash_init(void);
void ledseg_6c6s_clr(void);
u8 getcfg_buck_mode_en(void);

#if BT_PWRKEY_5S_DISCOVER_EN
AT(.com_text.detect)
void pwrkey_5s_on_check(void)
{
    if (!xcfg_cb.bt_pwrkey_nsec_discover) {
        return;
    }
    if (sys_cb.pwrkey_5s_check) {
        if (PWRKEY_IS_PRESS() || TKEY_IS_PRESS()) {
            if (tick_check_expire(sys_cb.ms_ticks, 1000 * xcfg_cb.bt_pwrkey_nsec_discover)) {
                sys_cb.pwrkey_5s_flag = 1;
                sys_cb.pwrkey_5s_check = 0;
            }
        } else {
            sys_cb.pwrkey_5s_check = 0;
        }
    }
}
#endif // BT_PWRKEY_5S_DISCOVER_EN

#if PWRKEY_2_HW_PWRON
//软开机模拟硬开关，松开PWRKEY就关机。
AT(.com_text.detect)
void pwrkey_2_hw_pwroff_detect(void)
{
    static int off_cnt = 0;
    if (!PWRKEY_2_HW_PWRON) {
        return;
    }

    if (PWRKEY_IS_PRESS()) {
        off_cnt = 0;
    } else {
        if (off_cnt < 10) {
            off_cnt++;
        } else if (off_cnt == 10) {
            //pwrkey已松开，需要关机
            off_cnt = 20;
            sys_cb.pwrdwn_hw_flag = 1;
            sys_cb.poweron_flag = 0;
        }
    }
}
#endif // PWRKEY_2_HW_PWRON




static uint16_t ntc_cval_H = 70;//45°
static uint16_t ntc_cval_H_rec = 77;//42°

static uint16_t ntc_cval_L = 200;//0°
static uint16_t ntc_cval_L_rec = 190;//3°

static uint16_t ntc_fval_H_off = 44;//60°
static uint16_t ntc_fval_L_off = 229;//-15°


#if SWETZ
#if USER_NTC



AT(.com_text.ntc)
u8  sys_check_ntc(void)
{
    if (!xcfg_cb.ntc_en) {
        return  0;
    }

    static int n_off_cnt = 0;
    static int n_cnt = 0;
    u16 current_adc = saradc_get_value8(ADCCH_NTC);

    if((current_adc <= ntc_fval_H_off)
        || (current_adc >= ntc_fval_L_off) 
        ){
            n_off_cnt++;
            n_cnt = 0;
            if(n_off_cnt>5){
                    n_off_cnt = 0;
                    printf("ntc:out range\n");
                    sys_cb.discon_reason = 0;   //不同步关机
                    sys_cb.pwrdwn_tone_en = 1;  
                    func_cb.sta = FUNC_PWROFF;  
                    return 1;
            }

    }else{
         
         n_off_cnt = 0;
    }



    if(current_adc <= ntc_cval_H || current_adc >= ntc_cval_L){
                n_cnt++;
                if(n_cnt > 8){
                        if(sys_cb.charge_sta == 1 && CHARGE_DC_IN()){
                                RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1);     //disable charger function
                                printf("ntc:dis charge\n");
                        }
                        n_cnt = 0;
                }


    }
    else if(current_adc >= ntc_cval_H_rec && current_adc <= ntc_cval_L_rec){
                n_cnt++;
                if(n_cnt > 8){
                        if(sys_cb.charge_sta == 0 && CHARGE_DC_IN()){
                         RTCCON8 = (RTCCON8 & ~BIT(1)) | BIT(6);// enable charger function
                         printf("ntc:normal\n");
                        }
                        n_cnt = 0;
                }

    }else {

        n_cnt = 0;
       
    }
    printf("NTC_current_adc:%d  n_cnt:%d  n_off_cnt:%d  sys_cb.charge_sta:%d CHARGE_DC_IN:%d\r\n",current_adc,n_cnt,n_off_cnt,sys_cb.charge_sta,CHARGE_DC_IN());

    return 0;

}

// u8 sys_ntc_check(void)
// {
//     if (!xcfg_cb.ntc_en) {
//         return  0;
//     }

//     static int n_cnt = 0;
//     if (xcfg_cb.ntc_en) {

//         if (saradc_get_value8(ADCCH_NTC) <= xcfg_cb.ntc_thd_val) {
//             if (n_cnt >= 20) {
// #if SWETZ                
//                 if(sys_cb.charge_sta == 1 && CHARGE_DC_IN()) { //  接触5V并且正在充电
//                     RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1);     //disable charger function
//                     n_cnt = 0;
//                 }else if(CHARGE_DC_IN() && sys_cb.charge_sta == 0){
//                      n_cnt = 0;

//                 }else if(!CHARGE_DC_IN()){
//                     if(saradc_get_value8(ADCCH_NTC) <= xcfg_cb.ntc_thd_val){//判断放电高低温保护
                             
//                              sys_cb.ntc_2_pwrdwn_flag = 1;
//                     }         
                   
//                 }


// #endif


//                // printf("ntc_2_pwrdwn_flag:%d\r\n",sys_cb.ntc_2_pwrdwn_flag);
                
//                 return 1;
//             } else {
//                 n_cnt++;
//             }
//         } else {
//             n_cnt = 0;
// #if SWETZ            
//             if(sys_cb.charge_sta == 0 && CHARGE_DC_IN()){
//                     //恢复充电功能
//                 RTCCON8 |= BIT(6); RTCCON8 &=~BIT(1);
//             }
// #endif
//         }
//     }
//     return 0;
// }
 #endif
#endif


//timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
#if (GUI_SELECT & DISPLAY_LEDSEG)
    gui_scan();                     //7P屏按COM扫描时，1ms间隔
#endif

#if LED_DISP_EN
    port_2led_scan();
#endif // LED_DISP_EN

    plugin_tmr1ms_isr();
#if USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif

    led_scan();
}

// AT(.com_text.str2) 
// const char box[] = "====inbox= %d\n";

#if SWETZ_CHECK_INCASE
AT(.com_text.timer)
void swetz_check_inoutbox(void)
{

    bool inbox = CHARGE_INBOX();
  //  printf(box,inbox);
    if (sys_cb.flag_local_in_case){
            if (!inbox){
                    msg_enqueue(EVT_OUT_CASE);
                }
            }else {
                if (inbox){
                    msg_enqueue(EVT_IN_CASE);
                }
            }
            sys_cb.flag_local_in_case = inbox;
           
}
#endif

#if ABMATE_AUTO_ANSWER
void ab_mate_auto_answer_set(u8 auto_answer_tyepe)
{

    if(auto_answer_tyepe == 1){
        if(bt_get_call_indicate() == BT_CALL_INCOMING){
                bt_call_answer_incoming();
        }

    }
    
}
#endif

//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_thread(void)
{
#if SWETZ
    sys_cb.sys_5ms_cnt++;
    if (sys_cb.sys_5ms_cnt == 980) 
    {
#if USER_NTC    
        ntc_gpio_power_supply();
#endif     	
    }
    if (sys_cb.sys_5ms_cnt >= 1000)
    {
        sys_cb.sys_5ms_cnt = 0;        
        msg_enqueue(EVT_SYS_5S);
    }

#endif         
    tmr5ms_cnt++;
    //5ms timer process
    dac_fade_process();
#if !USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif
#if PWRKEY_2_HW_PWRON
    pwrkey_2_hw_pwroff_detect();
#endif

#if (CHARGE_BOX_TYPE == CBOX_SSW)

#if !SWETZ_HEART_OFF
    charge_box_heart_beat_ack();
#endif

#endif
    plugin_tmr5ms_isr();

#if MUSIC_SDCARD_EN
    sd_detect();
#endif // MUSIC_SDCARD_EN



#if USB_SUPPORT_EN
    usb_detect();
#endif // USB_SUPPORT_EN

#if LINEIN_DETECT_EN
    linein_detect();
#endif // LINEIN_DETECT_EN

#if USER_INEAR_DETECT_EN
    earin_detect();
#endif

#if WARNING_WSBC_EN
    warning_dec_check_kick();
#endif

    //20ms timer process
    if ((tmr5ms_cnt % 4) == 0) {
#if CHARGE_EN
        if (xcfg_cb.charge_en) {
            ///入仓自动主从切换
            if (CHARGE_DC_IN()) {
                if (sys_cb.dc_in_filter < CHARGE_DC_IN_FILTER) {
                    sys_cb.dc_in_filter++;
                }
            } else {
                sys_cb.dc_in_filter = 0;
            }
        }
#endif
    }

    //50ms timer process
    if ((tmr5ms_cnt % 10) == 0) {
        ticks_50ms++;
#if BT_PWRKEY_5S_DISCOVER_EN
        pwrkey_5s_on_check();
#endif // BT_PWRKEY_5S_DISCOVER_EN
    }

    //100ms timer process
    if ((tmr5ms_cnt % 20) == 0) {


        lowpwr_tout_ticks();
#if UDE_HID_EN
        if (func_cb.sta == FUNC_USBDEV) {
            ude_tmr_isr();
        }
#endif // UDE_HID_EN
        gui_box_isr();                  //显示控件计数处理

        if (sys_cb.lpwr_cnt > 0) {
            sys_cb.lpwr_cnt++;
        }

        if (sys_cb.key2unmute_cnt) {
            sys_cb.key2unmute_cnt--;
            if (!sys_cb.key2unmute_cnt) {
                msg_enqueue(EVT_KEY_2_UNMUTE);
            }
        }
        dac_fifo_detect();
    }

    //500ms timer process
    if ((tmr5ms_cnt % 100) == 0) {
        sys_cb.cm_times++;
#if FUNC_CLOCK_EN
        msg_enqueue(MSG_SYS_500MS);
#endif // FUNC_CLOCK_EN
        
    }

    //1s timer process
    if ((tmr5ms_cnt % 200) == 0) {

#if SWETZ_CHECK_INCASE
        swetz_check_inoutbox();
#endif
        msg_enqueue(EVT_SYS_1S);
        msg_enqueue(MSG_SYS_1S);
        sys_cb.lpwr_warning_cnt++;



#if SWETZ_APP_TEST
    ab_mate_poweroff_proc();
#endif



#if SWETZ_TIME_3S
    if(sys_cb.tiem_3s != 4){
        sys_cb.tiem_3s++;
    }

    if(sys_cb.tiem_5s != 5){
        sys_cb.tiem_5s++;
    }   
    if(sys_cb.time_power_def_5s != 5){
        sys_cb.time_power_def_5s++;
    }   
#endif

#if SWETZ_EVT_5S
        if(sys_cb.time_cnt > 5){
            sys_cb.time_cnt = 0;
           // msg_enqueue(EVT_SYS_5S);
        }else {
            sys_cb.time_cnt++;
        }

#endif

    }
    //10s timer process
    if ((tmr5ms_cnt % 6000) == 0) {
        msg_enqueue(EVT_MSG_SYS_30S);
        tmr5ms_cnt = 0;
    }
}
static uint8_t bat_level = 0xff;
uint bsp_get_bat_level(void)
{
#if VBAT_DETECT_EN
    //计算方法：level = (实测电压 - 关机电压) / ((满电电压 - 关机电压) / 100)
    u16 bat_off = LPWR_OFF_VBAT * 100 + 2700;
    if (bat_off > sys_cb.vbat) {
        return 0;
    }
#if SWETZ_VBAT_CHECK
    static uint8_t old_bat_level = 0xff;

#if !SWETZ_CLOSE_INCASE_CHECKVBAT   
    if(CHARGE_DC_IN()){
        bat_level = get_bat_level_from_volt_wi_charger(sys_cb.vbat);
        old_bat_level = 0xff;
    }else 
#endif    
    {
        bat_level = get_bat_level_from_volt_wo_charger(sys_cb.vbat);
#if SWETZ_VBAT_CHARGE
        if(old_bat_level == 0xff){
                old_bat_level = bat_level;
        }

        if(bat_level < old_bat_level){
                old_bat_level = bat_level;

        }else {
                bat_level  = old_bat_level;
        }
#endif

    }

    if (bat_level > 100) {
        bat_level = 100;
        }
    return bat_level;

#else
    uint bat_level = (sys_cb.vbat - bat_off) / ((4200 - bat_off) / 100);//((4200 - bat_off) / 100);
    //printf("bat level: %d %d\n", sys_cb.vbat, bat_level);
    if (bat_level > 100) {
        bat_level = 100;
    }
    return bat_level;
#endif


#else
    return 100;
#endif
}

AT(.text.bsp.sys.init)
static void rtc_32k_configure(void)
{
    u32 temp = RTCCON0;

//    //xosc_div768_rtc
//    temp &= ~BIT(6);
//    temp |= BIT(9) | BIT(8);                        //sel xosc_div768_rtc
//    RTCCON0 = temp;
//    RTCCON4 |= BIT(17);                             //xosc24m low power clk enable
//    RTCCON2 = 31249;

    //clk2m_rtc_div32
    temp &= ~BIT(6);
    temp &= ~(BIT(9) | BIT(8));
    temp |= BIT(8);
    temp |= BIT(2) | BIT(0);
    RTCCON0 = temp;
    RTCCON2 = sys_get_rc2m_rtc_clk() / 32 - 1;
}

AT(.text.bsp.sys.init)
bool rtc_init(void)
{
    u32 temp;
    rtc_32k_configure();
    sys_cb.rtc_first_pwron = 0;
    temp = RTCCON0;
    if (temp & BIT(7)) {
        temp &= ~BIT(7);                            //clear first poweron flag
        RTCCON0 = temp;
        sys_cb.rtc_first_pwron = 1;
#if FUNC_CLOCK_EN
        rtc_clock_init();
#endif // FUNC_CLOCK_EN
        printf("rtc 1st pwrup\n");
        return false;
    }

    return true;
}

//UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    //等待uart0发送完成
    if(UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)));
    }

    GPIOEDE  &= ~BIT(13);
    GPIOEPU  &= ~BIT(13);
    GPIOBPU  &= ~(BIT(2) | BIT(3));
    FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA7)
    GPIOADE  |= BIT(7);
    GPIOAPU  |= BIT(7);
    GPIOADIR |= BIT(7);
    GPIOAFEN |= BIT(7);
    GPIOADRV |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA7;           //RX0 Map To TX0, TX0 Map to G1
#elif (UART0_PRINTF_SEL == PRINTF_PB2)
    GPIOBDE  |= BIT(2);
    GPIOBPU  |= BIT(2);
    GPIOBDIR |= BIT(2);
    GPIOBFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB2;           //RX0 Map To TX0, TX0 Map to G2
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    GPIOBDE  |= BIT(3);
    GPIOBPU  |= BIT(3);
    GPIOBDIR |= BIT(3);
    GPIOBFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB3;           //RX0 Map To TX0, TX0 Map to G3
#elif (UART0_PRINTF_SEL == PRINTF_PE7)
    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE7;           //RX0 Map To TX0, TX0 Map to G4
#elif (UART0_PRINTF_SEL == PRINTF_PE0)
    GPIOEDE  |= BIT(0);
    GPIOEPU  |= BIT(0);
    GPIOEDIR |= BIT(0);
    GPIOEFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE0;           //RX0 Map To TX0, TX0 Map to G5
#elif (UART0_PRINTF_SEL == PRINTF_VUSB)
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_VUSB;          //RX0 Map To TX0, TX0 Map to G8
#endif
}

//开user timer前初始化的内容
AT(.text.bsp.sys.init)
static void sys_var_init(void)
{
    u32 rst_reason = sys_cb.rst_reason;
    memset(&sys_cb, 0, sizeof(sys_cb));
    sys_cb.rst_reason = rst_reason;
    sys_cb.loudspeaker_mute = 1;
    sys_cb.ms_ticks = tick_get();
    sys_cb.pwrkey_5s_check = 1;
    xcfg_cb.vol_max = (xcfg_cb.vol_max) ? 32 : 16;
    sys_cb.hfp2sys_mul = (xcfg_cb.vol_max + 2) / 16;
    if (SYS_INIT_VOLUME > xcfg_cb.vol_max) {
        SYS_INIT_VOLUME = xcfg_cb.vol_max;
    }
    if (WARNING_VOLUME > xcfg_cb.vol_max) {
        WARNING_VOLUME = xcfg_cb.vol_max;
    }

    sys_cb.sleep_time = -1L;
    sys_cb.pwroff_time = -1L;
    if (xcfg_cb.sys_sleep_time != 0) {
        sys_cb.sleep_time = (u32)xcfg_cb.sys_sleep_time * 10;   //100ms为单位
    }
    if (xcfg_cb.sys_off_time != 0) {
        sys_cb.pwroff_time = (u32)xcfg_cb.sys_off_time * 10;    //100ms为单位
    }

    sys_cb.sleep_delay = -1L;
    sys_cb.pwroff_delay = -1L;
    sys_cb.sleep_en = 0;
    sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
    sys_cb.led_scan_en = 1;

    if(xcfg_cb.osci_cap == 0 && xcfg_cb.osco_cap == 0) {        //没有过产测时，使用自定义OSC电容
        xcfg_cb.osci_cap = xcfg_cb.uosci_cap;
        xcfg_cb.osco_cap = xcfg_cb.uosco_cap;
    }
    if(xcfg_cb.ft_rf_param_en == 0 && xcfg_cb.bt_rf_param_en) { //使用自定义参数时，不需要微调
        xcfg_cb.bt_rf_pwrdec = 0;
    }

    saradc_var_init();
    key_var_init();
    plugin_var_init();

    msg_queue_init();
    bsp_res_init();

    dev_init(((u8)is_sd_support()) | ((u8)is_usb_support() * 0x02));
#if SD_SOFT_DETECT_EN
    if (SD_IS_SOFT_DETECT()) {
        dev_delay_times(DEV_SDCARD, 3);
    }
#endif

#if MUSIC_SDCARD_EN
    if((xcfg_cb.sddet_iosel == IO_MUX_SDCLK) || (xcfg_cb.sddet_iosel == IO_MUX_SDCMD)) {
        dev_delay_offline_times(DEV_SDCARD, 3); //复用时, 加快拔出检测. 这里拔出检测为3次.
    }
#endif

    sdadc_var_init();
#if SDADC_5CH_EN
    sdadc_set_5ch_en();
#endif

#if (MUSIC_UDISK_EN || MUSIC_SDCARD_EN)
    fs_var_init();
#endif
    music_stream_var_init();
    msc_pcm_out_var_init();

    dac_cb_init((DAC_CH_SEL & 0x0f) | (0x200 * DAC_FAST_SETUP_EN) | (0x400 * DAC_VCM_CAPLESS_EN) \
                | (0x800 * DAC_MAXOUT_EN));

#if ANC_EN
    anc_var_init();
#endif


}

AT(.text.bsp.sys.init)
static void sys_io_init(void)
{
    //全部设置成模拟GPIO，防止漏电。使用时，自行配置对应数字GPIO
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F; //MCP FLASH

    uart0_mapping_sel();        //调试UART IO选择或关闭

#if !USB_BC_EN
    usb_bc_init(1);
#endif

#if LINEIN_DETECT_EN
    LINEIN_DETECT_INIT();
#endif // LINEIN_DETECT_EN

#if MUSIC_SDCARD_EN
    SD_DETECT_INIT();
#endif // MUSIC_SDCARD_EN

    LOUDSPEAKER_MUTE_INIT();

#if USER_INEAR_DET_OPT
    INEAR_OPT_PORT_INIT()
#endif
}

void xosc_get_cfg_cap(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

#if  0 //port_int_example
AT(.com_text)
const char strisr0[] = ">>[0x%X]_[0x%X]\n";
const char strisr1[] = "portisr->";
AT(.com_text.timer)
void port_isr(void)
{
    printf(strisr0,WKUPEDG,WKUPCPND);
    if (WKUPEDG & (BIT(6) << 16)) {
        WKUPCPND = (BIT(6) << 16);  //CLEAR PENDING
        printf(strisr1);
    }

}

void port_int_example(void)     //sys_set_tmr_enable(1, 1); 前调用 测试OK
{
    GPIOFDE |= BIT(0);  GPIOFDIR |= BIT(0); GPIOFFEN &= ~BIT(0);
    GPIOFPU |= BIT(0);
    sys_irq_init(IRQ_PORT_VECTOR,0, port_isr);
    PORTINTEN |= BIT(21);
    PORTINTEDG |= BIT(21);  //falling edge;
    WKUPEDG |= BIT(6);     //falling edge
    WKUPCON = BIT(6) | BIT(16);  //falling edge wake iput //wakeup int en

    printf("PORTINTEN = 0x%X, PORTINTEDG = 0x%X  WKUPEDG = 0x%X, WKUPCON = 0x%X\n", PORTINTEN, PORTINTEDG, WKUPEDG, WKUPCON);
    WDT_CLR();
    while(1) {
//       printf("WKUPEDG = 0x%X\n", WKUPEDG);
//       printf("GPIOF = 0x%X\n", GPIOF);
//       delay_ms(500);
    }
}
#endif

AT(.text.bsp.power)
bool power_off_check(void)
{
#if CHARGE_EN
    u16 charge_cnt = 0;
#endif

    u32 pwron_press_nms;
    int pwrkey_pressed_flag, ticks = 0, up_cnt = 0;
    u8 restart_chk_en = 1;

    pwrkey_pressed_flag = 0;
    pwron_press_nms = PWRON_PRESS_TIME;
    if (pwron_press_nms == 0) {
        pwron_press_nms = 15;                                           //bootloader 80ms + 15ms, 最小开机时间在100ms左右
    }

    //要等PWRKEY开关释放后再次按下才能重新开机, 否则充电过程中5分钟关机, 低电关机等异常
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
        restart_chk_en = 0;
        sys_cb.poweron_flag = 0;
    }

    while (1) {
        WDT_CLR();
        delay_ms(5);
        if ((bsp_key_pwr_scan() & K_PWR_MASK) == K_PWR) {
            up_cnt = 0;
            if (restart_chk_en) {
                if (!pwrkey_pressed_flag) {
                    ticks = tick_get();
                    pwrkey_pressed_flag = 1;
                    sys_cb.ms_ticks = tick_get();                                       //记录PWRKEY按键按下的时刻
                    sys_cb.pwrkey_5s_check = 1;
                }
                if (!sys_cb.poweron_flag) {
                    if (tick_check_expire(ticks, pwron_press_nms)) {                    //长按开机时间配置
                        sys_cb.poweron_flag = 1;
                        sys_cb.pwrdwn_hw_flag = 0;                                      //清PWRKEY硬开关的关机标志
                    }
                }
            }
        } else {
            if (up_cnt < 3) {
                up_cnt++;
            }
            if (up_cnt == 3) {
                up_cnt = 10;
                sys_cb.poweron_flag = 0;
                pwrkey_pressed_flag = 0;
                restart_chk_en = 1;
            }
        }

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
			charge_cnt++;
			if (charge_cnt > 20) {
                charge_cnt = 0;
                charge_detect(0);
            }
        }
#endif // CHARGE_EN

        if (sys_cb.poweron_flag) {
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
#if LINEIN_2_PWRDOWN_EN
            if (dev_is_online(DEV_LINEIN)) {
                continue;
            }
#endif // LINEIN_2_PWRDOWN_EN
            //长按PP/POWER开机
            gui_display(DISP_POWERON);
            led_power_up();
            dac_restart();
            bsp_change_volume(sys_cb.vol);
#if WARNING_POWER_ON
            #if WARNING_WSBC_EN
            wsbc_res_play(RES_BUF_POWERON, RES_LEN_POWERON);
            #else
#if SWETZ_WARNING_TONE
            mp3_res_play(RES_BUF_SW_POWERON_MP3,RES_LEN_SW_POWERON_MP3);
            printf("22RES_BUF_SW_POWERON_MP3\r\n");
#else
            mp3_res_play(RES_BUF_POWERON, RES_LEN_POWERON);
#endif

            #endif
#endif //

            if (PWRON_ENTER_BTMODE_EN) {
                func_cb.sta = FUNC_BT;
                if (dev_is_online(DEV_UDISK)) {
                    sys_cb.cur_dev = DEV_UDISK;
                } else {
                    sys_cb.cur_dev = DEV_SDCARD;
                }
            } else {
                if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_UDISK) || dev_is_online(DEV_SDCARD1)) {
                    func_cb.sta = FUNC_MUSIC;
                    if (dev_is_online(DEV_UDISK)) {
                        sys_cb.cur_dev = DEV_UDISK;
                    } else if (dev_is_online(DEV_SDCARD1)) {
                        sys_cb.cur_dev = DEV_SDCARD1;
                    } else {
                        sys_cb.cur_dev = DEV_SDCARD;
                    }
                } else {
#if FUNC_BT_EN
                    func_cb.sta = FUNC_BT;
#elif FUNC_CLOCK_EN
                    func_cb.sta = FUNC_CLOCK;
#else
                    func_cb.sta = FUNC_IDLE;
#endif
                }
            }
            return true;
        } else {
            if (CHARGE_DC_IN()) {
                continue;
            } else {
                return false;
            }
        }
    }
}

AT(.text.bsp.power)
static bool power_on_frist_enable(u32 rtccon9)
{
    if (!PWRON_FRIST_BAT_EN) {
        return false;
    }

    if (rtccon9 & BIT(4)) {                                        //charge inbox wakeup
        return false;
    }
    if (rtccon9 & BIT(2)) {                                        //WKO wakeup不能直接开机
        return false;
    }
    if (rtccon9 & BIT(6)) {                                        //TK Wakeup pending
        return false;
    }

    if (CHARGE_DC_IN() && (CHARGE_DC_NOT_PWRON) && (xcfg_cb.charge_en)) {   //VUSB充电禁止开机
        return false;
    }
    return true;
}

static bool power_on_check_do_pre(u32 rtccon9)
{
    bool ret = false;
     //第一次上电是否直接开机
    if (power_on_frist_enable(rtccon9)) {
        ret = true;
    }
    if(sys_cb.sw_rst_flag == SW_RST_FLAG){
        ret = true;
    }
    if (sys_cb.rst_reason & BIT(19)) {                         //is wko 10s reset pending
        if (PWRKEY_IS_PRESS()) {
            sys_cb.poweron_flag = 1;
        }
        ret = true;                                     //长按PWRKEY 10S复位后直接开机。
    }
#if QTEST_EN
    if(qtest_get_mode()){
        ret = true;
    }
#endif

#if IODM_TEST_EN
    if (cm_read8(PARAM_RST_FLAG) == RST_FLAG_MAGIC_VALUE) {
        cm_write8(PARAM_RST_FLAG, 0);
        cm_sync();
        printf("iodm rst power_on\n");
        ret = true;
    }
#endif
#if USER_PWRKEY
    if ((!xcfg_cb.user_pwrkey_en) && (!PWRKEY_2_HW_PWRON) && (!sys_cb.tkey_pwrdwn_en)) {
        ret = true;
    }
#endif
    return ret;
}

AT(.text.bsp.power)
static void power_on_check_do(void)
{
    int pwrkey_pressed_flag = 0;
    int up_cnt = 0, ticks = 0;
    u32 pwron_press_nms;
    u8 chbox_sta = 1;                               //默认offline

#if USER_NTC 
    u16 ntc_cnt = 0;
#endif 


    u32 rtccon9 = RTCCON9;                          //wakeup pending
    printf("power_on_check_do: %08x\n", rtccon9);

    RTCCON9 = 0xffff;                               //Clr pending
    RTCCON10 = BIT(10) | BIT(1) | BIT(0);           //Clr pending
    CRSTPND = 0x1ff0000;                            //clear reset pending
    LVDCON &= ~(0x1f << 8);                         //clear software reset
    RTCCON13 &= ~BIT(16);                           //wko pin0 low level wakeup

    if (power_on_check_do_pre(rtccon9)) {
        return;
    }

#if CHARGE_EN
    if (charge_power_on_pre(rtccon9)) {
        return;
    }
#endif

#if USER_PWRKEY
    if (PWRKEY_IS_PRESS() || TKEY_IS_PRESS()) {                         //PWRKEY是否按下
        pwrkey_pressed_flag = 1;
        ticks = sys_cb.ms_ticks;
    }
#endif // USER_PWRKEY
    pwron_press_nms = PWRON_PRESS_TIME;
    if (pwron_press_nms == 0) {
        pwron_press_nms = 15;                                           //最小开机时间在100ms左右
    }

    while (1) {
        WDT_CLR();
#if CHARGE_LOW_POWER_EN
        delay_us(350);
#else
        delay_5ms(1);
#endif
        bsp_key_scan_do();
        if ((bsp_key_pwr_scan() & K_PWR_MASK) == K_PWR) {
            up_cnt = 0;
            if (!pwrkey_pressed_flag) {
                ticks = tick_get();
                sys_cb.ms_ticks = ticks;                                //记录PWRKEY按键按下的时刻
                pwrkey_pressed_flag = 1;
            }
            if (!sys_cb.poweron_flag) {
                if (tick_check_expire(ticks, pwron_press_nms)) {        //长按开机时间配置
                    sys_cb.poweron_flag = 1;
                }
            }
        } else {
            if (up_cnt < 3) {
                up_cnt++;
            }
            if (up_cnt == 3) {
                up_cnt = 10;
                pwrkey_pressed_flag = 0;
                sys_cb.poweron_flag = 0;
            }
        }

#if LINEIN_2_PWRDOWN_EN
        linein_detect();
#endif // LINEIN_2_PWRDOWN_EN

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
            chbox_sta = charge_charge_on_process();
            if (chbox_sta == 2) {
                break;                  //充电仓拿起开机
            }
        }
#endif // CHARGE_EN

        if (sys_cb.poweron_flag) {
#if VBAT_DETECT_EN
            if (sys_cb.vbat <= 2950) {  //电压小于2.95v不开机
                continue;
            }
#endif
            if ((CHARGE_DC_NOT_PWRON) && CHARGE_DC_IN()) {
                continue;
            }
#if LINEIN_2_PWRDOWN_EN
            if (dev_is_online(DEV_LINEIN)) {
                continue;
            }
#endif // LINEIN_2_PWRDOWN_EN
            break;
        } else {
            //PWKKEY已松开, 不在充电仓或未充电直接进行关机
            if ((!pwrkey_pressed_flag) && (chbox_sta)) {
                if ((!SOFT_POWER_ON_OFF) || ((!sys_cb.wko_pwrkey_en) && (!USER_TKEY_SOFT_PWR_EN))) {
                    break;                          //没有按键软开关机功能，不在充电状态直接开机
                }
#if CHARGE_EN
                charge_exit();
#endif
                unlock_code_charge();
                bsp_saradc_exit();
                sfunc_pwrdown(1);
            }
        }

#if USER_NTC 
        ntc_cnt++;
        if (ntc_cnt == 980)
        {
            ntc_gpio_power_supply();
        }
        if (ntc_cnt >= 1000)
        {
            ntc_cnt = 0;
            sys_check_ntc();
            ntc_gpio_power_down();
        }
#endif   


    }
#if CHARGE_EN
     charge_exit();
#endif
}

AT(.text.bsp.power)
void power_on_check(void)
{
    lock_code_charge();
    power_on_check_do();
#if CHARGE_BOX_EN
    charge_box_reinit();
#endif
    unlock_code_charge();
}

#if SWETZ
extern void touchkey_init();
#endif

#if SWETZ_TOUCHINIT
void touchkey_init(void)
{
    // GPIOEFEN &= ~BIT(4);   //Pe3作为GPIO使用
    // GPIOEDE  |= BIT(4);    //Pe3设置为数字IO
    // GPIOEDIR &= ~BIT(4);   //Pe3方向设置为输出
    // GPIOESET |= BIT(4);
  // GPIOE |= BIT(4);       //Pe3输出高, 等效于GPIOeSET = BIT(3);
  // GPIOEDRV  |= BIT(4);
    // GPIOEPU &= ~BIT(4);
    // GPIOEPU200K &= ~BIT(4);
    // GPIOEPU300 &= ~BIT(4);
    GPIOBDE |= BIT(5);//数字I0使能:0为模拟I0，1为数字IO
    GPIOBDIR |= BIT(5);//控制I0的方向:8为输出，1为输入.
    GPIOBFEN &= ~BIT(5);//0:当作通用GPIO使用 //1:当作其它功能性IO
    GPIOBPU200K |= BIT(5);//200K上拉使能
}


#endif

#if SWETZ_BAT_SHOW_PHONE
uint8_t app_bat_level_show_for_phone(uint8_t bat_real_level)
{
    uint8_t bat_show;

    if (bat_real_level < 15)
    {
        bat_show = 0;
    }
    else if (bat_real_level < 25)
    {
        bat_show = 1;
    }
    else if (bat_real_level < 35)
    {
        bat_show = 2;
    }
    else if (bat_real_level < 45)
    {
        bat_show = 3;
    }
    else if (bat_real_level < 55)
    {
        bat_show = 4;
    }
    else if (bat_real_level < 65)
    {
        bat_show = 5;
    }
    else if (bat_real_level < 75)
    {
        bat_show = 6;
    }
    else if (bat_real_level < 85)
    {
        bat_show = 7;
    }
    else if (bat_real_level < 95)
    {
        bat_show = 8;
    }
    else
    {
        bat_show = 9;
    }
    return bat_show;
}

#endif


AT(.text.bsp.sys.init)
void sys_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
        WDT_RST();
        while(1);
    }

    // io init
    sys_io_init();

    // var init
    sys_var_init();

    // power init
    pmu_init(getcfg_buck_mode_en());

    //audio pll init
    adpll_init(DAC_OUT_SPR);

    // clock init
    sys_clk_set(SYS_CLK_SEL);

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    //晶振配置
    xosc_init();

    plugin_init();

    if (POWKEY_10S_RESET) {
        WKO_10SRST_EN(0);                                   //10s reset source select  0: wko pin press, 1: touch key press
    } else {
        WKO_10SRST_DIS();
    }

    led_init();


#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_init();
    }
#endif
    key_init();

#if QTEST_EN
    if(QTEST_IS_ENABLE()) {
        qtest_init();
    }
#endif

#if HUART_EN
    if(xcfg_cb.huart_en){
        bsp_huart_init();
    }
#endif // HUART_EN

#if SWETZ_VBAT_CHECK
        app_bat_level_init();
#endif


#if UART1_EN
    if (xcfg_cb.uart1_en) {
        bsp_uart1_init(9600);
    }
#elif UART2_EN
    if (xcfg_cb.uart2_en) {
        bsp_uart2_init(9600);
    }
#endif

    bt_init();

    power_on_check();               //在key_init之后
    gui_init();
#if PWM_RGB_EN
    pwm_init();
#endif // PWM_RGB_EN

    gsensor_init();

    en_auto_pwroff();

    /// enable user timer for display & dac
    sys_set_tmr_enable(1, 1);

    led_power_up();
    gui_display(DISP_POWERON);

#if ANC_EQ_RES2_EN
    copy_res2flash();
#endif

    if (bsp_dac_off_for_bt_conn()) {
        dac_init();
        func_bt_init();
    } else {
        func_bt_init();
        dac_init();
    }

    codecs_pcm_init();

#if TINY_TRANSPARENCY_EN
    bsp_ttp_init();
#endif

#if ABMATE_AUTO_ANSWER
    ab_mate_app.auto_answer_type = memory_read_flag(PARAM_AUTO_ANSWER_ADDR);
    app_lr_send_notification(LR_NOTIFY_AUTO_ANSWER_STATUS, 1, &ab_mate_app.auto_answer_type);
#endif


    app_init_do();

#if SWETZ_MAX_LINK_1
    if(ab_mate_app.mult_dev.en == 0){
            cfg_bt_max_acl_link = 1;
            bt_nor_link_ctrl_max_set(cfg_bt_max_acl_link);
    }
#endif


    bsp_change_volume(sys_cb.vol);

#if BLE_POPU_TEST
       // printf("33--sys_cb.local_bat_level:%d sys_cb.peer_bat_level:%d\r\n",sys_cb.local_bat_level,sys_cb.peer_bat_level);
      //  ble_adv0_set_ctrl(1);
#endif

#if WARNING_POWER_ON
    if ((xcfg_cb.bt_outbox_voice_pwron_en) || (!sys_cb.outbox_pwron_flag)) {
        #if WARNING_WSBC_EN
        wsbc_res_play(RES_BUF_POWERON, RES_LEN_POWERON);
        #else
#if SWETZ_WARNING_TONE

#if SWETZ_RESET_NOT_POWER_TONE
     sys_cb.reset_Not_tone = memory_read_flag(PARAM_NOT_POWER_TONE_ADDR);
     printf("sys_cb.reset_Not_tone:%d\r\n",sys_cb.reset_Not_tone);

     if(sys_cb.reset_Not_tone == 1){
            sys_cb.reset_Not_tone = 0;
            memory_save_flag(PARAM_NOT_POWER_TONE_ADDR,sys_cb.reset_Not_tone);
            mp3_res_play(RES_BUF_MUTE_MP3,RES_LEN_MUTE_MP3);
            printf("NOT TONE\r\n");
            
     }else {
            mp3_res_play(RES_BUF_SW_POWERON_MP3,RES_LEN_SW_POWERON_MP3);
            printf("33RES_BUF_SW_POWERON_MP3\r\n");

     }
#else
             mp3_res_play(RES_BUF_SW_POWERON_MP3,RES_LEN_SW_POWERON_MP3);
            printf("33RES_BUF_SW_POWERON_MP3\r\n");
#endif

#else
     mp3_res_play(RES_BUF_POWERON, RES_LEN_POWERON);
#endif



        #endif
    }



    sys_cb.outbox_pwron_flag = 0;
#endif //

    if (PWRON_ENTER_BTMODE_EN) {
        func_cb.sta = FUNC_BT;
        if (dev_is_online(DEV_UDISK)) {
            sys_cb.cur_dev = DEV_UDISK;
        } else {
            sys_cb.cur_dev = DEV_SDCARD;
        }
    } else {
#if SD_SOFT_DETECT_EN
        sd_soft_detect_poweron_check();
#endif

#if FUNC_MUSIC_EN
        if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_UDISK) || dev_is_online(DEV_SDCARD1)) {
            func_cb.sta = FUNC_MUSIC;
            if (dev_is_online(DEV_UDISK)) {
                sys_cb.cur_dev = DEV_UDISK;
            } else if (dev_is_online(DEV_SDCARD1)) {
                sys_cb.cur_dev = DEV_SDCARD1;
            } else {
                sys_cb.cur_dev = DEV_SDCARD;
            }
        } else
#endif // FUNC_MUSIC_EN
        {
#if FUNC_AUX_EN
            if (dev_is_online(DEV_LINEIN)) {
                func_cb.sta = FUNC_AUX;
            } else
#endif // FUNC_AUX_EN
            {
                func_cb.sta = FUNC_BT;
            }
        }


    }



#if SYS_MODE_BREAKPOINT_EN
    u8 sta = param_sys_mode_read();
    if (sta != 0 && sta != 0xff) {
        func_cb.sta = sta & 0xf;
        if (func_cb.sta == FUNC_MUSIC) {
            sys_cb.cur_dev = sta >> 4;
        }
    }
#endif // SYS_MODE_BREAKPOINT_EN

#if LINEIN_2_PWRDOWN_EN
    if (dev_is_online(DEV_LINEIN)) {
        sys_cb.pwrdwn_tone_en = LINEIN_2_PWRDOWN_TONE_EN;
        func_cb.sta = FUNC_PWROFF;
    }
#endif // LINEIN_2_PWRDOWN_EN

#if BT_BACKSTAGE_EN
    bsp_bt_init();
#endif

#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
    eq_dbg_init();
#endif // EQ_DBG_IN_UART

#if PLUGIN_SYS_INIT_FINISH_CALLBACK
    plugin_sys_init_finish_callback(); //初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
#endif

#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif

#if BT_MUSIC_EFFECT_EN
    music_effect_init();
#endif // BT_MUSIC_EFFECT_EN
#if SWETZ_TOUCHINIT
    touchkey_init();
#endif
#if SWETZ_TIME_3S
    sys_cb.tiem_3s = 0;  
    sys_cb.tiem_5s = 0;
    sys_cb.time_power_def_5s = 0;
#endif

#if SWETZ
    sys_cb.low_power_mark = 0;
#endif

#if TWS_LR
    app_lr_init();
#endif

#if SWETZ_MEMORY
    sys_cb.anc_user_mode = memory_read_flag(PARAM_ANC_SWITCH_ADDR);
    printf("power on anc_user_mode:%d\r\n",sys_cb.anc_user_mode);
    if(sys_cb.anc_user_mode == APP_ANC_START || sys_cb.anc_user_mode == APP_ANC_TRANSPARENCY){
            bsp_anc_set_mode(sys_cb.anc_user_mode);
    }

#endif



#if SWETZ_BAT_SHOW_PHONE
        sys_cb.local_bat_level = bsp_get_bat_level();
        app_lr_send_notification(LR_NOTIFY_BATTERY_LEVEL, 1, &sys_cb.local_bat_level);
#endif

//printf("999-sys_cb.local_bat_level:%d sys_cb.peer_bat_level:%d\r\n",sys_cb.local_bat_level,sys_cb.peer_bat_level);

#if UART_TO_POWEROFF
    sys_cb.uart_poweroff_flag = 0;
#endif



}


AT(.text.bsp.sys.init)
void sys_update_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    sys_io_init();

    // var init
    sys_var_init();
    sys_cb.lang_id = 0;

    // power init
    pmu_init(getcfg_buck_mode_en());

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    //晶振配置
    xosc_init();

    plugin_init();
    sys_set_tmr_enable(1, 1);

    adpll_init(DAC_OUT_SPR);
    dac_init();
    mp3_res_play(RES_BUF_UPDATE_DONE_MP3, RES_LEN_UPDATE_DONE_MP3);
}

#if SWETZ_VBAT_CHECK
static bool is_update_level_using_charging_time;
static uint8_t charging_update_gauge_count;
static uint8_t bat_wait_stable_cnt;

#define CHARGING_UPDATE_GAUGE_COUNT_NUMBER (12)//12*5=60秒
#define BAT_STABLE_CNT (12)//12*5=60秒

#define SWETZ_DEBUG  0
#if SWETZ_DEBUG

AT(.com_rodata.bat)
const char volt_wi[] = "bat_level: %d\n\r";

#endif
static uint8_t get_bat_level_from_volt_wi_charger(uint16_t volt)
{
    uint8_t level ;

    if (volt <= (LPWR_OFF_VBAT * 100 + 2700))
    {
        level = 0;
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_5)
    {
        level = 5*(volt - (LPWR_OFF_VBAT * 100 + 2700))/(VOLT_BAT_LEVEL_CHARGING_5 - (LPWR_OFF_VBAT * 100 + 2700));
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_10)
    {
        level = 5 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_5)/(VOLT_BAT_LEVEL_CHARGING_10 - VOLT_BAT_LEVEL_CHARGING_5);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_15)
    {
        level = 10 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_10)/(VOLT_BAT_LEVEL_CHARGING_15 - VOLT_BAT_LEVEL_CHARGING_10);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_20)
    {
        level = 15 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_15)/(VOLT_BAT_LEVEL_CHARGING_20 - VOLT_BAT_LEVEL_CHARGING_15);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_25)
    {
        level = 20 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_20)/(VOLT_BAT_LEVEL_CHARGING_25 - VOLT_BAT_LEVEL_CHARGING_20);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_30)
    {
        level = 25 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_25)/(VOLT_BAT_LEVEL_CHARGING_30 - VOLT_BAT_LEVEL_CHARGING_25);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_35)
    {
        level = 30 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_30)/(VOLT_BAT_LEVEL_CHARGING_35 - VOLT_BAT_LEVEL_CHARGING_30);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_40)
    {
        level = 35 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_35)/(VOLT_BAT_LEVEL_CHARGING_40 - VOLT_BAT_LEVEL_CHARGING_35);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_45)
    {
        level = 40 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_40)/(VOLT_BAT_LEVEL_CHARGING_45 - VOLT_BAT_LEVEL_CHARGING_40);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_50)
    {
        level = 45 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_45)/(VOLT_BAT_LEVEL_CHARGING_50 - VOLT_BAT_LEVEL_CHARGING_45);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_55)
    {
        level = 50 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_50)/(VOLT_BAT_LEVEL_CHARGING_55 - VOLT_BAT_LEVEL_CHARGING_50);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_60)
    {
        level = 55 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_55)/(VOLT_BAT_LEVEL_CHARGING_60 - VOLT_BAT_LEVEL_CHARGING_55);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_65)
    {
        level = 60 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_60)/(VOLT_BAT_LEVEL_CHARGING_65 - VOLT_BAT_LEVEL_CHARGING_60);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_70)
    {
        level = 65 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_65)/(VOLT_BAT_LEVEL_CHARGING_70 - VOLT_BAT_LEVEL_CHARGING_65);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_75)
    {
        level = 70 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_70)/(VOLT_BAT_LEVEL_CHARGING_75 - VOLT_BAT_LEVEL_CHARGING_70);
    }
    else if (volt <= VOLT_BAT_LEVEL_CHARGING_80)
    {
        level = 75 + 5*(volt - VOLT_BAT_LEVEL_CHARGING_75)/(VOLT_BAT_LEVEL_CHARGING_80 - VOLT_BAT_LEVEL_CHARGING_75);
    }
    else
    {
        level = 80;
    }
#if SWETZ_DEBUG    
    printf(volt_wi,level);
#endif    
    return level;
}

static uint8_t get_bat_level_from_volt_wo_charger(uint16_t volt)
{
    uint8_t level;

    if (volt >= VOLT_BAT_LEVEL_100)
    {
        level = 100;
    }
    else if (volt >= VOLT_BAT_LEVEL_95)
    {
        level = 95 + 5*(volt - VOLT_BAT_LEVEL_95)/(VOLT_BAT_LEVEL_100 - VOLT_BAT_LEVEL_95);
    }
    else if (volt >= VOLT_BAT_LEVEL_90)
    {
        level = 90 + 5*(volt - VOLT_BAT_LEVEL_90)/(VOLT_BAT_LEVEL_95 - VOLT_BAT_LEVEL_90);
    }
    else if (volt >= VOLT_BAT_LEVEL_85)
    {
        level = 85 + 5*(volt - VOLT_BAT_LEVEL_85)/(VOLT_BAT_LEVEL_90 - VOLT_BAT_LEVEL_85);
    }
    else if (volt >= VOLT_BAT_LEVEL_80)
    {
        level = 80 + 5*(volt - VOLT_BAT_LEVEL_80)/(VOLT_BAT_LEVEL_85 - VOLT_BAT_LEVEL_80);
    }
    else if (volt >= VOLT_BAT_LEVEL_75)
    {
        level = 75 + 5*(volt - VOLT_BAT_LEVEL_75)/(VOLT_BAT_LEVEL_80 - VOLT_BAT_LEVEL_75);
    }
    else if (volt >= VOLT_BAT_LEVEL_70)
    {
        level = 70 + 5*(volt - VOLT_BAT_LEVEL_70)/(VOLT_BAT_LEVEL_75 - VOLT_BAT_LEVEL_70);
    }
    else if (volt >= VOLT_BAT_LEVEL_65)
    {
        level = 65 + 5*(volt - VOLT_BAT_LEVEL_65)/(VOLT_BAT_LEVEL_70 - VOLT_BAT_LEVEL_65);
    }
    else if (volt >= VOLT_BAT_LEVEL_60)
    {
        level = 60 + 5*(volt - VOLT_BAT_LEVEL_60)/(VOLT_BAT_LEVEL_65 - VOLT_BAT_LEVEL_60);
    }
    else if (volt >= VOLT_BAT_LEVEL_55)
    {
        level = 55 + 5*(volt - VOLT_BAT_LEVEL_55)/(VOLT_BAT_LEVEL_60 - VOLT_BAT_LEVEL_55);
    }
    else if (volt >= VOLT_BAT_LEVEL_50)
    {
        level = 50 + 5*(volt - VOLT_BAT_LEVEL_50)/(VOLT_BAT_LEVEL_55 - VOLT_BAT_LEVEL_50);
    }
    else if (volt >= VOLT_BAT_LEVEL_45)
    {
        level = 45 + 5*(volt - VOLT_BAT_LEVEL_45)/(VOLT_BAT_LEVEL_50 - VOLT_BAT_LEVEL_45);
    }
    else if (volt >= VOLT_BAT_LEVEL_40)
    {
        level = 40 + 5*(volt - VOLT_BAT_LEVEL_40)/(VOLT_BAT_LEVEL_45 - VOLT_BAT_LEVEL_40);
    }
    else if (volt >= VOLT_BAT_LEVEL_35)
    {
        level = 35 + 5*(volt - VOLT_BAT_LEVEL_35)/(VOLT_BAT_LEVEL_40 - VOLT_BAT_LEVEL_35);
    }
    else if (volt >= VOLT_BAT_LEVEL_30)
    {
        level = 30 + 5*(volt - VOLT_BAT_LEVEL_30)/(VOLT_BAT_LEVEL_35 - VOLT_BAT_LEVEL_30);
    }
    else if (volt >= VOLT_BAT_LEVEL_25)
    {
        level = 25 + 5*(volt - VOLT_BAT_LEVEL_25)/(VOLT_BAT_LEVEL_30 - VOLT_BAT_LEVEL_25);
    }
    else if (volt >= VOLT_BAT_LEVEL_20)
    {
        level = 20 + 5*(volt - VOLT_BAT_LEVEL_20)/(VOLT_BAT_LEVEL_25 - VOLT_BAT_LEVEL_20);
    }
    else if (volt >= VOLT_BAT_LEVEL_15)
    {
        level = 15 + 5*(volt - VOLT_BAT_LEVEL_15)/(VOLT_BAT_LEVEL_20 - VOLT_BAT_LEVEL_15);
    }
    else if (volt >= VOLT_BAT_LEVEL_10)
    {
        level = 10 + 5*(volt - VOLT_BAT_LEVEL_10)/(VOLT_BAT_LEVEL_15 - VOLT_BAT_LEVEL_10);
    }
    else if (volt >= VOLT_BAT_LEVEL_5)
    {
        level = 5 + 5*(volt - VOLT_BAT_LEVEL_5)/(VOLT_BAT_LEVEL_10 - VOLT_BAT_LEVEL_5);
    }
    else if (volt >= VOLT_BAT_LEVEL_5)
    {
        level = 5 + 5*(volt - VOLT_BAT_LEVEL_5)/(VOLT_BAT_LEVEL_10 - VOLT_BAT_LEVEL_5);
    }
    else if (volt >= (LPWR_OFF_VBAT * 100 + 2700))
    {
        level = 5*(volt - (LPWR_OFF_VBAT * 100 + 2700))/(VOLT_BAT_LEVEL_5 - (LPWR_OFF_VBAT * 100 + 2700));
    }
    else
    {
        level = 0;
    }
    return level;
}



void app_bat_level_init(void)
{
    uint8_t bat_level_temp = get_bat_level_from_volt_wo_charger(sys_cb.vbat);

    if (bat_level == 0xff)
    {
        //第一次接上电池
        if (sys_cb.rtc_first_pwron)
        {
            bat_level = bat_level_temp;
        }

    }
}


#endif

