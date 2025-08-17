#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "mute.h"
#include "param.h"
#include "device.h"
#include "vbat.h"


#if SWETZ_VBAT_CHECK
// #define VOLT_BAT_LEVEL_100 (4153)//4.17
// #define VOLT_BAT_LEVEL_95  (4096)//4.12
// #define VOLT_BAT_LEVEL_90  (4064)//4.08
// #define VOLT_BAT_LEVEL_85  (4022)//4.04
// #define VOLT_BAT_LEVEL_80  (3981)//4.00
// #define VOLT_BAT_LEVEL_75  (3925)//3.95
// #define VOLT_BAT_LEVEL_70  (3876)//3.90-	
// #define VOLT_BAT_LEVEL_65  (3851)//3.87-
// #define VOLT_BAT_LEVEL_60  (3818)//3.84-
// #define VOLT_BAT_LEVEL_55  (3785)//3.81-
// #define VOLT_BAT_LEVEL_50  (3769)//3.79-
// #define VOLT_BAT_LEVEL_45  (3745)//3.77-
// #define VOLT_BAT_LEVEL_40  (3713)//3.73-
// #define VOLT_BAT_LEVEL_35  (3689)//3.71-
// #define VOLT_BAT_LEVEL_30  (3665)//3.69-
// #define VOLT_BAT_LEVEL_25  (3649)//3.67-
// #define VOLT_BAT_LEVEL_20  (3625)//3.65-
// #define VOLT_BAT_LEVEL_15  (3601)//3.62-
// #define VOLT_BAT_LEVEL_10  (3576)//3.60
// #define VOLT_BAT_LEVEL_5   (3527)//3.55




// #define VOLT_BAT_LEVEL_CHARGING_100 (4269)//4.30-
// #define VOLT_BAT_LEVEL_CHARGING_95  (4229)//4.25-
// #define VOLT_BAT_LEVEL_CHARGING_90  (4189)//4.21-
// #define VOLT_BAT_LEVEL_CHARGING_85  (4141)//4.16-
// #define VOLT_BAT_LEVEL_CHARGING_80  (4093)//4.11-
// #define VOLT_BAT_LEVEL_CHARGING_75  (4061)//4.08-
// #define VOLT_BAT_LEVEL_CHARGING_70  (4029)//4.05-
// #define VOLT_BAT_LEVEL_CHARGING_65  (3997)//4.02-
// #define VOLT_BAT_LEVEL_CHARGING_60  (3973)//3.99-
// #define VOLT_BAT_LEVEL_CHARGING_55  (3949)//3.97-
// #define VOLT_BAT_LEVEL_CHARGING_50  (3925)//3.95-
// #define VOLT_BAT_LEVEL_CHARGING_45  (3909)//3.93-
// #define VOLT_BAT_LEVEL_CHARGING_40  (3885)//3.91-
// #define VOLT_BAT_LEVEL_CHARGING_35  (3869)//3.89-
// #define VOLT_BAT_LEVEL_CHARGING_30  (3847)//3.87-
// #define VOLT_BAT_LEVEL_CHARGING_25  (3815)//3.84-
// #define VOLT_BAT_LEVEL_CHARGING_20  (3799)//3.82-
// #define VOLT_BAT_LEVEL_CHARGING_15  (3767)//3.79-
// #define VOLT_BAT_LEVEL_CHARGING_10  (3647)//3.67-
// #define VOLT_BAT_LEVEL_CHARGING_5   (3575)//3.60-

// #define VOLT_BAT_LEVEL_100 (4260)//4.30
// #define VOLT_BAT_LEVEL_95  (4198)//4.23
// #define VOLT_BAT_LEVEL_90  (4141)//4.17
// #define VOLT_BAT_LEVEL_85  (4081)//4.11
// #define VOLT_BAT_LEVEL_80  (4017)//4.05	
// #define VOLT_BAT_LEVEL_75  (3967)//4.00	
// #define VOLT_BAT_LEVEL_70  (4901)//3.93	
// #define VOLT_BAT_LEVEL_65  (3859)//3.89
// #define VOLT_BAT_LEVEL_60  (3819)//3.85
// #define VOLT_BAT_LEVEL_55  (3787)//3.82
// #define VOLT_BAT_LEVEL_50  (3763)//3.79
// #define VOLT_BAT_LEVEL_45  (3731)//3.76
// #define VOLT_BAT_LEVEL_40  (3707)//3.74
// #define VOLT_BAT_LEVEL_35  (3691)//3.72
// #define VOLT_BAT_LEVEL_30  (3667)//3.70
// #define VOLT_BAT_LEVEL_25  (3642)//3.67
// #define VOLT_BAT_LEVEL_20  (3618)//3.65
// #define VOLT_BAT_LEVEL_15  (3590)//3.62
// #define VOLT_BAT_LEVEL_10  (3560)//3.60
// #define VOLT_BAT_LEVEL_5   (3423)//3.45


#define VOLT_BAT_LEVEL_100 (4227)//
#define VOLT_BAT_LEVEL_95  (4170)//
#define VOLT_BAT_LEVEL_90  (4113)//
#define VOLT_BAT_LEVEL_85  (4053)//
#define VOLT_BAT_LEVEL_80  (3999)//
#define VOLT_BAT_LEVEL_75  (3943)//
#define VOLT_BAT_LEVEL_70  (3904)//
#define VOLT_BAT_LEVEL_65  (3871)//
#define VOLT_BAT_LEVEL_60  (3835)//
#define VOLT_BAT_LEVEL_55  (3800)//
#define VOLT_BAT_LEVEL_50  (3782)//
#define VOLT_BAT_LEVEL_45  (3755)//

#define VOLT_BAT_LEVEL_40  (3737)//
#define VOLT_BAT_LEVEL_35  (3707)//
#define VOLT_BAT_LEVEL_30  (3680)//
#define VOLT_BAT_LEVEL_25  (3663)//
#define VOLT_BAT_LEVEL_20  (3632)//
#define VOLT_BAT_LEVEL_15  (3568)//
#define VOLT_BAT_LEVEL_10  (3300)//
#define VOLT_BAT_LEVEL_5   (3200)//


// #define VOLT_BAT_LEVEL_CHARGING_100 (4350)
// #define VOLT_BAT_LEVEL_CHARGING_95  (4325)
// #define VOLT_BAT_LEVEL_CHARGING_90  (4295)
// #define VOLT_BAT_LEVEL_CHARGING_85  (4235)
#define VOLT_BAT_LEVEL_CHARGING_80  (4337)//4.38
#define VOLT_BAT_LEVEL_CHARGING_75  (4280)//4.32
#define VOLT_BAT_LEVEL_CHARGING_70  (4231)//4.26
#define VOLT_BAT_LEVEL_CHARGING_65  (4167)//4.20
#define VOLT_BAT_LEVEL_CHARGING_60  (4110)//4.14
#define VOLT_BAT_LEVEL_CHARGING_55  (4061)//4.09
#define VOLT_BAT_LEVEL_CHARGING_50  (4020)//4.05
#define VOLT_BAT_LEVEL_CHARGING_45  (3980)//4.01
#define VOLT_BAT_LEVEL_CHARGING_40  (3948)//3.98
#define VOLT_BAT_LEVEL_CHARGING_35  (3916)//3.95
#define VOLT_BAT_LEVEL_CHARGING_30  (3892)//3.92
#define VOLT_BAT_LEVEL_CHARGING_25  (3867)//3.90
#define VOLT_BAT_LEVEL_CHARGING_20  (3851)//3.88
#define VOLT_BAT_LEVEL_CHARGING_15  (3819)//3.85
#define VOLT_BAT_LEVEL_CHARGING_10  (3786)//3.82
#define VOLT_BAT_LEVEL_CHARGING_5   (3745)//3.78

#endif


#define VOL_MAX                         xcfg_cb.vol_max   //�����������

typedef struct {

#if SWETZ
    u8  tiem_3s;
    u8  tiem_5s;
    u8  time_power_def_5s;
    u8  time_cnt;
    u8  time_ble;
    u32 bt_con_titck;
    u8  reset_Not_tone;
    u8  return_link;
    u8  ancmode_temp;
#endif  
    u32 rst_reason;
    u8  play_mode;
    u8  db_level;
    u8  vol;
    u8  hfp_vol;
    u8  eq_mode;
    u8  cur_dev;
    u8  lang_id;
    u8  lpwr_warning_cnt;
    u8  lpwr_warning_times;     //�����͵������0xff��ʾһֱ��
    u8  vbat_nor_cnt;
    s8  gain_offset;            //���ڶ�̬�޸�������
    u8  hfp2sys_mul;            //ϵͳ������HFP�����������ֻ�HFP����ֻ��16����
    u8  lpwr_cnt;               //�͵����
    u8 dac_sta_bck;
#if BT_TWS_EN
#if (CHARGE_BOX_TYPE == CBOX_SSW)
    u8 loc_house_state;         //bit0=is_open, bit1=key_state
    u8 rem_house_bat;
    u8 rem_bat;
    u8 rem_house_state;         //bit0=is_open, bit1=key_state
    u8 loc_house_bat;
    u8 loc_bat;
    u8 popup_en;
#endif
    u8 tws_left_channel;        //TWS��������־.
    u8 name_suffix_en;          //TWS�����Ƿ����Ӻ�׺����
    u8 tws_force_channel;       //1: �̶�������, 2:�̶�������
    u8 vusb_force_channel;      //VUSB UART�̶�������
#if USER_INEAR_DETECT_EN
    u8 rem_ear_sta;             //�Զ������״̬��0Ϊ�����1Ϊ����
    u8 loc_ear_sta;				//���������״̬��0Ϊ�����1Ϊ����
#endif // USER_INEAR_DETECT_EN
#endif // BT_TWS_EN
    u8 vusb_uart_flag;          //vusb uart�򿪱�־
    u8 discon_reason;
#if CHARGE_LOW_POWER_EN
    u32 charge_timr0_pr;
#endif
volatile u8  incall_flag;

    u8 fmrx_type;
    u8 inbox_wken_flag;
    u8 outbox_pwron_flag;
    u16 kh_vol_msg;
    u16 vbat;
    u32 sleep_time;
    u32 pwroff_time;
    u32 sleep_delay;
    u32 pwroff_delay;
    u32 sleep_wakeup_time;
    u32 ms_ticks;               //msΪ��λ
    u32 rand_seed;
    u8  tws_res_brk;
    u8  tws_res_wait;
    u8  wav_sysclk_bak;
    u8 wko_pwrkey_en;
    u8 tkey_pwrdwn_en;
    u8 sw_rst_flag;

    u8 sleep_counter;       //sleepmode����ʱ����

volatile u8  cm_times;
volatile u8  loudspeaker_mute;  //����MUTE��־
volatile u8  pwrkey_5s_check;   //�Ƿ���п�������5S��PWRKEY���
volatile u8  pwrkey_5s_flag;    //����5��PWRKEY������־
volatile u8  charge_sta;        //0:���رգ� 1����翪���� 2������
volatile u8  charge_bled_flag;  //charge�������Ƴ�����־
volatile u8  ch_bled_cnt;       //charge����������ʱ�����
volatile u8  micl_en;           //MICL enable working��־
volatile u8  poweron_flag;      //pwrkey������־
volatile u8  pwrdwn_tone_en;    //POWER DOWN�Ƿ񲥷���ʾ��
volatile u8  key2unmute_cnt;    //����������ʱ��mute
volatile u8  pwrdwn_hw_flag;    //ģ��Ӳ���أ��ػ���־

volatile u8  dc_in_filter;      //DC IN filter

#if REC_FAST_PLAY
    u16 rec_num;                //��¼���µ�¼���ļ����
    u32 ftime;
#endif

#if SYS_KARAOK_EN
    u8 echo_level;              //����ȼ�
    u8 echo_delay;              //������
    u8 mic_vol;                 //MIC����
    u8 music_vol;               //MUSIC����
    u8 rec_msc_vol;             //KARAOK¼��ʱMUSIC����������
    u8 magic_type;
    u8 magic_level;
#endif

#if ANC_EN || TINY_TRANSPARENCY_EN
    u8 anc_user_mode;           //ancģʽ, 0:�ر�, 1:ANC, 2:ͨ͸
    u8 ttp_start;
    u8 ttp_sta;
#endif

#if ANC_EN
    u8 anc_start;
    u8 anc_init;
    s8 adjust_val[8];           //mic����ֵ
	u32 mic_gain[4];            //mic��ʼֵ
#endif

#if !SWETZ
#if USER_NTC
    u8  ntc_2_pwrdwn_flag;
#endif
#endif


#if ANC_ALG_EN
    volatile u8 anc_alg_en;
    u32 anc_alg_type_bak;
#endif // ANC_ALG_EN
    ///λ������Ҫ������Ҫ���жϸ�ֵ�ı����� �����ʹ��λ�����������ٶ���λ������
    u8  rtc_first_pwron  : 1,   //RTC�Ƿ��һ���ϵ�
        mute             : 1,   //ϵͳMUTE���Ʊ�־
        cm_factory       : 1,   //�Ƿ��һ��FLASH�ϵ�
        cm_vol_change    : 1,   //���������Ƿ���Ҫ���µ�FLASH
        bthid_mode       : 1,   //����HID����ģʽ��־
        port2led_en      : 1,   //1��IO��������
        voice_evt_brk_en : 1,   //������ʾ��ʱ��U�̡�SD����LINEIN�Ȳ����¼��Ƿ�������Ӧ.
        karaok_init      : 1;   //karaok��ʼ����־

    u8  sleep_en             : 1,   //���ڿ����Ƿ����sleep
		lowbat_flag          : 1,
        hfp_karaok_en        : 1,   //ͨ��ģʽ�Ƿ��K�蹦�ܡ�
        eq_app_total_gain_en : 1;   //���ڿ����Ƿ�ʹ��app eq������
    u8 led_scan_en;
#if USER_INEAR_DETECT_EN
    u8 in_ear_en;
#endif
#if BT_MAP_EN
    u8 map_retry;
#endif
#if ABP_EN
    u8 abp_mode;
#endif
#if ASR_EN
    u8 asr_enable;
#endif
#if FUNC_USBDEV_EN
    u8  ude_mic_spr;            //usb mic spr
#endif
#if ANC_ALG_DYVOL_FF_EN
    s8 dyvol_local;
    s8 dyvol_real;
    s8 dyvol_overlap;
#if BT_TWS_EN
    s8 dyvol_remote;
#endif // BT_TWS_EN
#endif // ANC_ALG_DYVOL_FF_EN

#if SWETZ

    u32 tick_event;
    u8  low_power_mark;
    u8  uart_poweroff_flag;
#endif
#if TWS_LR
    u8 peer_bat_level;
    u8 local_bat_level;
    u8 bt_master_status;
    u8 bt_master_link;
#endif

#if SWETZ_CHECK_INCASE
    volatile u8 flag_local_in_case;
    volatile u8 flag_peer_in_case;
#endif
#if SWETZ
    u8 ntc_discharge_out_normal_range;
#endif

    volatile u16 sys_5ms_cnt;
} sys_cb_t;
extern sys_cb_t sys_cb;

extern volatile int micl2gnd_flag;
extern volatile u32 ticks_50ms;


void sys_init(void);
void sys_update_init(void);
void timer1_irq_init(void);
void bsp_sys_mute(void);
void bsp_sys_unmute(void);
void bsp_clr_mute_sta(void);
#if LOUDSPEAKER_MUTE_EN
void bsp_loudspeaker_mute(void);
void bsp_loudspeaker_unmute(void);
#else
#define bsp_loudspeaker_mute()
#define bsp_loudspeaker_unmute()
#endif
void uart0_mapping_sel(void);
void linein_detect(void);
bool linein_micl_is_online(void);
void get_usb_chk_sta_convert(void);
uint bsp_get_bat_level(void);
void sd_soft_cmd_detect(u32 check_ms);

#if SWETZ_BAT_SHOW_PHONE
uint8_t app_bat_level_show_for_phone(uint8_t bat_real_level);
#endif

#if SWETZ_VBAT_CHECK
static uint8_t get_bat_level_from_volt_wo_charger(uint16_t volt);
static uint8_t get_bat_level_from_volt_wi_charger(uint16_t volt);
#endif


#if USER_NTC 
extern u8 ntc_status;
u8 sys_check_ntc(void);
#if (NTC_GPIO_POWER == IO_PE4)
#define ntc_gpio_power_supply()     \
    do {                            \
        GPIOEFEN &= ~BIT(4);        \
        GPIOEDE |= BIT(4);          \
        GPIOEDIR &= ~BIT(4);        \
        GPIOESET |= BIT(4);         \
    } while (0)
#define ntc_gpio_power_down()       \
    do {                            \
        GPIOECLR |= BIT(4);         \
    } while (0)
#else
#define ntc_gpio_power_supply()
#define ntc_gpio_power_down()
#endif
#endif

#define ntc_is_ready_for_sleep()    (!sys_cb.ntc_discharge_out_normal_range)

#if ABMATE_AUTO_ANSWER
void ab_mate_auto_answer_set(u8 auto_answer_tyepe);
#endif
#endif // __SYSTEM_H
