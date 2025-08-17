#include "include.h"
#include "api.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN


//���������Ļص�����, setting_type: 0=ͬ������, 1=�ֻ���������, 2=������������
bool dev_vol_set_cb(uint8_t dev_vol, uint8_t media_index, uint8_t setting_type)
{
    if(setting_type & BIT(3)) {
        sys_cb.hfp_vol = dev_vol;
    } else {
        sys_cb.vol = a2dp_vol_conver(dev_vol);
    }

    TRACE("dev_vol_set_cb: %d(%x, %d, %d)\n", sys_cb.vol, dev_vol, media_index, setting_type);

#if BT_TWS_EN
    //TWSʱ������alarm��ʱ��������δʵ����������
    bt_tws_req_alarm_vol(dev_vol, setting_type);
#else
    if(setting_type & BIT(3)) {
        msg_enqueue(EVT_HFP_SET_VOL);
    } else {
        msg_enqueue(EVT_A2DP_SET_VOL);
    }
#endif

    return true;
}

#if BT_TWS_EN
//alarm��ʱ����ת����DAC����������ʾ��ǰ����
uint16_t dev_vol_req_cb(uint8_t dev_vol, uint8_t setting_type, bool remote)
{
    uint8_t vol_set;
    uint8_t feat = (setting_type & 0x7);
    bool hfp_type = (bool)(setting_type & BIT(3));

    if(hfp_type) {
        vol_set = bsp_bt_get_hfp_vol(dev_vol);
        if(remote){
            sys_cb.hfp_vol = dev_vol;
        }
    } else {
        vol_set = a2dp_vol_conver(dev_vol);
        if(remote){
            sys_cb.vol = vol_set;
        }
#if BT_MUSIC_EFFECT_DBB_EN || BT_MUSIC_EFFECT_DYEQ_EN || BT_MUSIC_EFFECT_XDRC_EN
        sys_cb.vol = vol_set;
#endif
    }

    TRACE("dev_vol_set_req: %d(%x, %d, %d)\n", vol_set, dev_vol, remote, setting_type);

    if(feat != 0) {
        msg_enqueue(EVT_TWS_SET_VOL);
    }
    return bsp_volume_convert(vol_set) | (hfp_type<<15);
}

//alarm��ʱ������ͬ������DAC����
AT(.com_text.dev_vol)
void dev_vol_alarm_cb(uint16_t params)
{
    uint16_t dac_vol = params & 0x7fff;
    bool hfp_type = (bool)(params & 0x8000);
    if (!bsp_res_is_vol_busy()) {
        if((hfp_type && !(sys_cb.incall_flag & INCALL_FLAG_SCO))
            || (!hfp_type && (sys_cb.incall_flag & INCALL_FLAG_SCO))){
            return;
        }
#if BT_MUSIC_EFFECT_SOFT_VOL_EN
#if BT_MUSIC_EFFECT_DBB_EN
        if (music_effect_get_state_real(MUSIC_EFFECT_DBB)) {
#elif BT_MUSIC_EFFECT_DYEQ_EN
        if (music_effect_get_state_real(MUSIC_EFFECT_DYEQ)) {
#elif BT_MUSIC_EFFECT_XDRC_EN
        if (music_effect_get_state_real(MUSIC_EFFECT_XDRC)) {
#endif
            msg_enqueue(EVT_SOFT_VOL_SET);
            return;
        }
#endif // BT_MUSIC_EFFECT_SOFT_VOL_EN
#if ANC_ALG_DYVOL_FF_LOCAL_VOL_EN
        bsp_anc_alg_dyvol_gain_cal(dac_vol);
#else
        dac_vol_set(dac_vol);
#endif // ANC_ALG_DYVOL_FF_LOCAL_VOL_EN
    }
}
#endif

