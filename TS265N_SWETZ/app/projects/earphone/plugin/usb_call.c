#include "include.h"
#include "usb_call.h"

#if USB_MIC_NR_EN
typedef struct {
    fft_cfg_t fft;
    ifft_cfg_t ifft;
    s16 mmic_buf[480];
    s16 smic_buf[480];
    s32 mmic_fft_buf[512];
    s32 smic_fft_buf[512];
    s16 mmic_outbuf[512];
    nr_kick_func_t nr_kick_func;
} uda_alg_mic_cb_t;

uda_alg_mic_cb_t uda_alg_mic_cb AT(.usb_buf);

#if BT_SNDP_DM_AI_EN
static sndp_dm_cb_t sndp_dm_cb AT(.sndp_dm_buf);
#endif

void uda_pcm_set_nr_kick_func(nr_kick_func_t nr_kick_func)
{
    uda_alg_mic_cb.nr_kick_func = nr_kick_func;
}

AT(.usbdev.com)
void uda_mic_src0_sync(void)
{
    u32 len = uda_mic_stm_len_get();
    if (len < 1440) {
        src0_adjust_speed(-1);
    } else if (len > 2160) {
        src0_adjust_speed(1);
    } else {
        src0_adjust_speed(0);
    }
}


AT(.usbdev.com)
void usb_srcto48k_callback (u8 *ptr, u32 samples, int channel, void *param)
{
    usbmic_sdadc_process(ptr, samples, 0);
}

AT(.usbdev.com)
void uda_pcm_process(void)
{
    fft_cfg_t *f = &uda_alg_mic_cb.fft;
    ifft_cfg_t *fi = &uda_alg_mic_cb.ifft;
    s16* mmic_buf = uda_alg_mic_cb.mmic_buf;
    s16* smic_buf = uda_alg_mic_cb.smic_buf;
    s16* mmic_outbuf = uda_alg_mic_cb.mmic_outbuf;
    s32* mmic_fft_buf = uda_alg_mic_cb.mmic_fft_buf;
    s32* smic_fft_buf = uda_alg_mic_cb.smic_fft_buf;

    f->in_addr = mmic_buf;
    f->out_addr = mmic_fft_buf;
    fft_hw(f);

    f->in_addr = smic_buf;
    f->out_addr = smic_fft_buf;
    fft_hw(f);

    uda_alg_mic_cb.nr_kick_func(mmic_fft_buf, smic_fft_buf, NULL, NULL, NULL, NULL);

    fi->in_addr = mmic_fft_buf;
    fi->out_addr = mmic_outbuf;
    ifft_hw(fi);

    memcpy(mmic_buf, mmic_buf+240, 240 * 2);
    memcpy(smic_buf, smic_buf+240, 240 * 2);

    uda_mic_src0_sync();
    src0_audio_input((u8 *)mmic_outbuf, 240);
}

AT(.usbdev.com)
void usbmic_nr_fill_process(u8 *ptr, u32 samples, int ch_mode)
{
    u16 *ptr16 = (u16 *)ptr;
    s16* mmic_buf = uda_alg_mic_cb.mmic_buf;
    s16* smic_buf = uda_alg_mic_cb.smic_buf;
    if (ch_mode == 0) {
        for (int i = 0; i < 240; i++) {
            mmic_buf[240+i] = ptr16[i];
        }
    } else {
        for (int i = 0; i < 240; i++) {
            mmic_buf[240+i] = ptr16[2*i + 0];
            smic_buf[240+i] = ptr16[2*i + 1];
        }
    }
    uda_pcm_kick_start();
}

AT(.usbdev.com)
void usbmic_data_process(u8 *ptr, u32 samples, int ch_mode)
{
    usbmic_nr_fill_process(ptr, samples, ch_mode);
}

void uda_alg_mic_cb_init(void)
{
    fft_cfg_t *f = &uda_alg_mic_cb.fft;
    ifft_cfg_t *fi = &uda_alg_mic_cb.ifft;
    s16* mmic_buf = uda_alg_mic_cb.mmic_buf;
    s16* smic_buf = uda_alg_mic_cb.smic_buf;

    memset(mmic_buf, 0, sizeof(uda_alg_mic_cb.mmic_buf));
    memset(smic_buf, 0, sizeof(uda_alg_mic_cb.smic_buf));

    memset(f, 0, sizeof(fft_cfg_t));
    memset(fi, 0, sizeof(ifft_cfg_t));

    f->size             = RDFT_512;
	f->input_type       = 0;
    f->window_en        = 1;
    f->isr_en           = 1;

    fi->size            = RDFT_512;
    fi->output_type     = 0;
    fi->window_en       = 1;
    fi->isr_en          = 1;
    fi->overlap_en      = 1;
}

void uda_nr_alg_init(void)
{
    printf("uda_nr_alg_init\n");
    uda_alg_mic_cb_init();

#if BT_SNDP_DM_AI_EN
    uda_alg_sndpdm_init();
#endif
    uda_mic_sync_set(0);

    src0_init(SPR_16000, 240*3);
    src0_audio_output_callback_set(usb_srcto48k_callback);
}

void uda_nr_alg_exit(void)
{
    delay_5ms(1);
#if BT_SNDP_DM_AI_EN
    uda_alg_sndpdm_exit();
#endif
    src0_stop();
}

#if BT_SNDP_DM_AI_EN
void uda_alg_sndpdm_init(void)
{
    if (!xcfg_cb.bt_sndp_dm_ai_en) {
        printf("SNDP DM xcfg init warning!\n");
        return;
    }

    memset(&sndp_dm_cb, 0, sizeof(sndp_dm_cb_t));
    sndp_dm_cb.param_printf = 0;
    sndp_dm_cb.level        = xcfg_cb.sndp_dm_level;
    sndp_dm_cb.dnn_level    = xcfg_cb.sndp_dm_dnn_level;
    sndp_dm_cb.distance     = 70;
    sndp_dm_cb.degree       = xcfg_cb.sndp_dm_degree;
    sndp_dm_cb.degree1      = xcfg_cb.sndp_dm_degree1;
    sndp_dm_cb.bf_upper     = 128;

    if (sndp_dm_cb.degree1 < sndp_dm_cb.degree) {
        sndp_dm_cb.degree1 = sndp_dm_cb.degree;
    } else {
        sndp_dm_cb.degree1 = sndp_dm_cb.degree1 - sndp_dm_cb.degree;    //相减得出过渡角
    }

//    if(xcfg_cb.micr_phase_en == 1){                                   //当前软件不支持反相
//        cfg_micr_phase_en = 1;
//    }

#ifdef RES_BUF_SPECCFGPARAST_F_BIN
    if (RES_LEN_SPECCFGPARAST_F_BIN > 0) {
        sndp_dm_cb.coef = (const u32 *)RES_BUF_SPECCFGPARAST_F_BIN;
    }
#endif

    alg_sndp_dm_init(&sndp_dm_cb);
    printf("dm level: %d %d, distance: %d\n", sndp_dm_cb.level, sndp_dm_cb.dnn_level, sndp_dm_cb.distance);
    printf("degree: %d %d\n", sndp_dm_cb.degree, sndp_dm_cb.degree1);
    printf("alg_sndp_dm_init\n");

    uda_pcm_set_nr_kick_func(bt_sndp_dm_process);

    sys_clk_req(INDEX_VOICE, SYS_120M);
}

void uda_alg_sndpdm_exit(void)
{
    alg_sndp_dm_exit();

    sys_clk_free(INDEX_VOICE);
}
#endif
#endif
