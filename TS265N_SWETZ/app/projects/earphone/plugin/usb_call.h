#ifndef _USB_CALL_H
#define _USB_CALL_H

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int channel, void *param);
typedef s32 (*nr_kick_func_t)(s32 *efw0, s32 *efw1, s32 *dfw, s32 *xfw, s16 *hnl, u32 *aeccon2);

void uda_pcm_kick_start(void);
void uda_mic_sync_set(u8 on);
u32 uda_mic_stm_len_get(void);
void usbmic_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void src_var_init(void);
void src0_init(u8 in_sample_rate, u16 out_samples_expected);
void src0_adjust_speed(int speed);
void src0_audio_input(u8 *buf, u32 samples);
void src0_audio_output_callback_set(audio_callback_t callback);
void src0_stop(void);
void uda_nr_alg_init(void);
void uda_nr_alg_exit(void);

void alg_sndp_dm_init(void *alg_cb);
void alg_sndp_dm_exit(void);
s32 bt_sndp_dm_process(s32 *efw0, s32 *efw1, s32 *dfw, s32 *xfw, s16 *hnl, u32 *aeccon2);
void uda_alg_sndpdm_init(void);
void uda_alg_sndpdm_exit(void);
#endif
