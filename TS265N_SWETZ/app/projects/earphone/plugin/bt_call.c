#include "include.h"

#if BT_SNDP_SMIC_AI_EN
    static sndp_sm_cb_t sndp_sm_cb AT(.sndp_buf);
#elif BT_SNDP_DM_AI_EN
    static sndp_dm_cb_t sndp_dm_cb AT(.sndp_dm_buf);
    uint8_t cfg_micr_phase_en;
#elif BT_SNDP_FBDM_EN
    static sndp_fbdm_cb_t sndp_fbdm_cb AT(.sndp_fbdm_buf);
#elif BT_SCO_SMIC_AI_EN
    static dnn_cb_t dnn_cb AT(.dnn_buf);
#elif BT_SCO_SMIC_AI_PRO_EN
    static dnn_pro_cb_t dnn_pro_cb AT(.dnn_pro_buf);
#elif BT_SCO_AIAEC_DNN_EN
    static dnn_aec_ns_cb_t aiaec_dnn_cb AT(.aiaec_dnn_buf);
#elif BT_SCO_DMIC_AI_EN
    static dmns_cb_t dmns_cb AT(.dmdnn_buf);
    uint8_t cfg_micr_phase_en;
#elif BT_SCO_LDMIC_AI_EN
    static ldmdnn_cb_t ldmdnn_cb AT(.ldmdnn_buf);
    uint8_t cfg_micr_phase_en;
#elif BT_SCO_NR_EN
    static ains_cb_t ains_cb AT(.ains_buf);
#endif

#if BT_SCO_FAR_NR_EN
static peri_nr_cfg_t far_cfg AT(.nr_buf.far);
#endif

void bt_sco_rec_exit(void);

AT(.text.sco.gain.tbl)
const int mic_gain_tbl[32] = {
    SOFT_DIG_P0DB,
    SOFT_DIG_P1DB,
    SOFT_DIG_P2DB,
    SOFT_DIG_P3DB,
    SOFT_DIG_P4DB,
    SOFT_DIG_P5DB,
    SOFT_DIG_P6DB,
    SOFT_DIG_P7DB,
    SOFT_DIG_P8DB,
    SOFT_DIG_P9DB,
    SOFT_DIG_P10DB,
    SOFT_DIG_P11DB,
    SOFT_DIG_P12DB,
    SOFT_DIG_P13DB,
    SOFT_DIG_P14DB,
    SOFT_DIG_P15DB,
    SOFT_DIG_P16DB,
    SOFT_DIG_P17DB,
    SOFT_DIG_P18DB,
    SOFT_DIG_P19DB,
    SOFT_DIG_P20DB,
    SOFT_DIG_P21DB,
    SOFT_DIG_P22DB,
    SOFT_DIG_P23DB,
    SOFT_DIG_P24DB,
    SOFT_DIG_P25DB,
    SOFT_DIG_P26DB,
    SOFT_DIG_P27DB,
    SOFT_DIG_P28DB,
    SOFT_DIG_P29DB,
    SOFT_DIG_P30DB,
    SOFT_DIG_P31DB,
};

void bt_sco_aec_init(u8 *sysclk, aec_cfg_t *aec, alc_cb_t *alc)
{
    if (xcfg_cb.bt_aec_en) {
    #if BT_AEC_EN
        aec->type           = AEC_TYPE_DEFAULT;
        aec->mode           = 1;
        aec->nlp_bypass     = 0;
        aec->nlp_only       = 0;
        aec->nlp_level      = BT_ECHO_LEVEL;
        aec->nlp_part       = 1;
        aec->comforn_level  = 10;
        aec->comforn_floor  = 300;
        aec->comforn_en     = 0;
        aec->xe_add_corr    = 16384;
        aec->upbin          = 63;
        aec->lowbin         = 7;
        aec->diverge_th     = 15;
        aec->echo_th        = 66666;
        aec->gamma          = 27852;
        aec->aggrfact       = 16384;
        aec->depre_mode     = 1;
        aec->mic_ch         = 0;
        aec->qidx           = (16384 * (aec->upbin - aec->lowbin + 1)) >> 15;
        aec->dig_gain       = mic_gain_tbl[xcfg_cb.aec_dig_gain] >> 8;
        aec->ff_mic_ref_en  = BT_AEC_FF_MIC_REF_EN;
        *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
    #endif
    } else {
        aec->type         = AEC_TYPE_NONE;
    }
    aec->far_offset     = xcfg_cb.aec_far_offset;           //声加1+1会用到这个参数，单独放到外面初始化
}

void bt_sco_far_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_SCO_FAR_NR_EN
    nr->nr_cfg_en |= NR_CFG_FAR_EN;
    memset(&far_cfg, 0, sizeof(peri_nr_cfg_t));
    nr->far_nr = &far_cfg;
    far_cfg.level = BT_SCO_FAR_NOISE_LEVEL;
    far_cfg.min_range1 = 30;
    far_cfg.min_range2 = 50;
    far_cfg.min_range3 = 80;
    far_cfg.min_range4 = 120;
    far_cfg.nera_val = 0x7eb8;
#endif

#if BT_SCO_DAC_DRC_EN
    nr->nr_cfg_en |= NR_CFG_DAC_DRC_EN;
    bt_sco_dac_drc_init((u8 *)RES_BUF_EQ_CALL_DAC_DRC, RES_LEN_EQ_CALL_DAC_DRC);
#endif

#if BT_SCO_DAC_DNR_EN
    nr->nr_cfg_en |= NR_CFG_DAC_DNR_EN;
    nr->dac_dnr_thr = BT_SCO_DAC_DNR_THR;
#endif

#if BT_SCO_CALLING_NR_EN
    nr->calling_voice_cnt = BT_SCO_CALLING_VOICE_CNT;
    nr->calling_voice_pow = BT_SCO_CALLING_VOICE_POW;
#endif
}

void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_TRUMPET_NR_EN
    nr->nr_cfg_en |= NR_CFG_TRUMPET_EN;
    trumpet_denoise_init(BT_TRUMPET_NR_LEVEL);
#endif

#if BT_SCO_FADE_EN
    nr->nr_cfg_en |= NR_CFG_SCO_FADE_EN;
#endif

    bt_sco_near_nr_init(sysclk, nr);

    if (!xcfg_cb.bt_nr_en) {
        nr->nr_type = ((nr->nr_type & ~NR_CFG_TYPE_MASK) | NR_TYPE_NONE);
    }
}

void bt_sco_nr_exit(void)
{
    bt_sco_near_nr_exit();
}

#if BT_SNDP_DMIC_EN
void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr)
{
}
#endif

#if BT_SNDP_SMIC_AI_EN
void bt_sco_sndp_sm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_smic_ai_en) {
        printf("SNDP DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_SM;

    memset(&sndp_sm_cb, 0, sizeof(sndp_sm_cb_t));
    sndp_sm_cb.param_printf = 0;
    sndp_sm_cb.level        = xcfg_cb.bt_sndp_level;
    sndp_sm_cb.dnn_level    = xcfg_cb.bt_sndp_dnn_level;
#ifdef RES_BUF_SPECCFGPARAST_F_BIN
    if (RES_LEN_SPECCFGPARAST_F_BIN > 0) {
        sndp_sm_cb.coef = (const u32 *)RES_BUF_SPECCFGPARAST_F_BIN;
    }
#endif
    bt_sndp_sm_init(&sndp_sm_cb);

    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}
#endif

#if BT_SNDP_FBDM_EN
void bt_sco_sndp_fbdm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_fbdm_en) {
        printf("SNDP FBDM xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_FBDM;
    memset(&sndp_fbdm_cb, 0, sizeof(sndp_fbdm_cb_t));
    sndp_fbdm_cb.param_printf   = 0;
    sndp_fbdm_cb.level          = xcfg_cb.sndp_fbdm_level;
    sndp_fbdm_cb.dnn_level      = xcfg_cb.sndp_fbdm_dnn_level;
    sndp_fbdm_cb.noiserms_th0   = xcfg_cb.sndp_fbdm_noiserms_th0;

#ifdef RES_BUF_SPECCFGPARAST_F_BIN
    if (RES_LEN_SPECCFGPARAST_F_BIN > 0) {
        sndp_fbdm_cb.coef = (const u32 *)RES_BUF_SPECCFGPARAST_F_BIN;
    }
#endif
    bt_sndp_fbdm_init(&sndp_fbdm_cb);

    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SNDP_DM_AI_EN
void bt_sco_sndp_dm_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_sndp_dm_ai_en) {
        printf("SNDP DM xcfg init warning!\n");
        return;
    }
    nr->nr_type             = NR_TYPE_SNDP_DM;

    memset(&sndp_dm_cb, 0, sizeof(sndp_dm_cb_t));
    sndp_dm_cb.param_printf = 0;
    sndp_dm_cb.level        = xcfg_cb.sndp_dm_level;
    sndp_dm_cb.dnn_level    = xcfg_cb.sndp_dm_dnn_level;
    sndp_dm_cb.distance     = xcfg_cb.sndp_dm_distance;
    sndp_dm_cb.degree       = xcfg_cb.sndp_dm_degree;
    sndp_dm_cb.degree1      = xcfg_cb.sndp_dm_degree1;
    sndp_dm_cb.bf_upper     = 128;

    if (sndp_dm_cb.degree1 < sndp_dm_cb.degree) {
        sndp_dm_cb.degree1 = sndp_dm_cb.degree;
    } else {
        sndp_dm_cb.degree1 = sndp_dm_cb.degree1 - sndp_dm_cb.degree;    //相减得出过渡角
    }

    if(xcfg_cb.micr_phase_en == 1){
        cfg_micr_phase_en = 1;
    }

#ifdef RES_BUF_SPECCFGPARAST_F_BIN
    if (RES_LEN_SPECCFGPARAST_F_BIN > 0) {
        sndp_dm_cb.coef = (const u32 *)RES_BUF_SPECCFGPARAST_F_BIN;
    }
#endif
    bt_sndp_dm_init(&sndp_dm_cb);

    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SCO_DMIC_AI_EN
void bt_sco_dmns_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dmns_en) {  //配置上面的DMNS双麦降噪功能未打开
        printf("DMNS xcfg init warning!\n");
    }
	memset(&dmns_cb, 0, sizeof(dmns_cb_t));
	nr->nr_type             = NR_TYPE_DMIC_AI;

	dmns_cb.param_printf           	= 0;
	dmns_cb.distance               	= xcfg_cb.bt_dmns_distance;
	dmns_cb.degree                 	= 0;
	dmns_cb.nt                     	= xcfg_cb.bt_dmns_level;
	dmns_cb.noise_ps_rate          	= 1;
	dmns_cb.low_fre_ns0_range		= 16;        //range:1-200
	dmns_cb.comp_mic_fre_L         	= 0;
	dmns_cb.comp_mic_fre_H         	= 255;
	dmns_cb.prior_opt_idx			= 3;
	dmns_cb.tf_en					= 1;
    dmns_cb.phase_comp_en           = 0;
	dmns_cb.phase_comp_en_B         = 0;
    dmns_cb.low_bf_lim              = 8;
	dmns_cb.tf_len				 	= 256;      //range:1-256
	dmns_cb.tf_norm_en			 	= 1;
	dmns_cb.tf_norm_only_en		 	= 0;
	dmns_cb.opt_limit				= 32767;
	dmns_cb.bf_choose				= 1;
	dmns_cb.white_approach_shape    = 1;
	dmns_cb.wind_sig_choose		 	= 1;        //0时代表副mic受风小  1代表主mic受风小
	dmns_cb.wind_state_lim          = 25000;
	dmns_cb.wind_sup_floor          = 23000;
	dmns_cb.wind_sup_open           = 1;
	dmns_cb.wind_a_pow_en			= 0;

	dmns_cb.exp_range_H			 	= 0;
	dmns_cb.exp_range_L			 	= 0;
	dmns_cb.music_lev			  	= 11;
	dmns_cb.gain_expand			 	= 1024;
	dmns_cb.nn_only				 	= 0;
	dmns_cb.nn_only_len			 	= 16;
	dmns_cb.hi_fre_en 			 	= 1;
	dmns_cb.gain_assign			 	= 16666;
	dmns_cb.sin_gain_post_en		= 1;
	dmns_cb.sin_gain_post_len_f	    = 160;
	dmns_cb.b_frein2_floor		    = 19666;
	dmns_cb.enlarge_v               = 1;
    dmns_cb.mask_floor			    = 1600;
	dmns_cb.mask_floor_hi		    = 160;
	dmns_cb.TCtrigger_sta           = 0;
	dmns_cb.low_grad_prot           = 0;
    dmns_cb.grad_gain_floor         = 600;
	dmns_cb.bfrein3_ns			    = 3;
    dmns_cb.wind_len_used           = 36;

	dmns_cb.Gcoh_thr				= 13666;
	dmns_cb.Gcoh_sum_thr			= 9666;
	dmns_cb.wind_or_en			 	= 1;
	dmns_cb.v_white_en			 	= 1;
    dmns_cb.bfrein_en				= 0;        //if set "1", the "dmdnn_cb->bfrein3_en" must be "0".
	dmns_cb.bfrein3_en		        = 1;        //if set "1", the "dmdnn_cb->bfrein_en" must be "0".
	dmns_cb.gu_value				= 32767;

    bt_dmns_init(&dmns_cb);

    if(xcfg_cb.micr_phase_en == 1){
        cfg_micr_phase_en = 1;
    }

    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SCO_LDMIC_AI_EN
void bt_sco_ldmdnn_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dmns_en) {  //配置上面的DMNS双麦降噪功能未打开
        printf("DMNS xcfg init warning!\n");
    }
	memset(&ldmdnn_cb, 0, sizeof(ldmdnn_cb_t));
	nr->nr_type             = NR_TYPE_DMIC_AI;

	ldmdnn_cb.param_printf           	= 0;
	ldmdnn_cb.distance               	= xcfg_cb.bt_dmns_distance;
	ldmdnn_cb.degree                 	= 0;
	ldmdnn_cb.nt                     	= xcfg_cb.bt_dmns_level;
	ldmdnn_cb.noise_ps_rate          	= 1;
	ldmdnn_cb.low_fre_ns0_range		    = 16;        //range:1-200
	ldmdnn_cb.comp_mic_fre_L         	= 0;
	ldmdnn_cb.comp_mic_fre_H         	= 256;
	ldmdnn_cb.prior_opt_idx			    = 3;
	ldmdnn_cb.tf_en					    = 1;
	ldmdnn_cb.tf_len				 	= 256;      //range:1-256
	ldmdnn_cb.tf_norm_en			 	= 0;
	ldmdnn_cb.tf_norm_only_en		 	= 0;
	ldmdnn_cb.bf_choose				    = 1;
	ldmdnn_cb.wind_sig_choose		 	= 1;        //0时代表副mic受风小  1代表主mic受风小
	ldmdnn_cb.wind_state_lim            = 25000;
	ldmdnn_cb.wind_sup_floor            = 23000;
	ldmdnn_cb.wind_sup_open             = 1;
	ldmdnn_cb.wind_a_pow_en			    = 0;
	ldmdnn_cb.wind_len_used             = 36;       //要小于
	ldmdnn_cb.Ftrust                    = 64;       //用前多少个点来确定入射角 不能超过FTRUST

	ldmdnn_cb.exp_range_H			 	= 0;
	ldmdnn_cb.exp_range_L			 	= 0;
	ldmdnn_cb.music_lev			  	    = 11;
	ldmdnn_cb.gain_expand			 	= 1024;
	ldmdnn_cb.nn_only				 	= 0;
	ldmdnn_cb.nn_only_len			 	= 16;
	ldmdnn_cb.hi_fre_en 			 	= 1;
	ldmdnn_cb.gain_assign			 	= 16666;
	ldmdnn_cb.sin_gain_post_en		    = 1;
	ldmdnn_cb.sin_gain_post_len_f	    = 160;
	ldmdnn_cb.b_frein2_floor		    = 19666;
	ldmdnn_cb.enlarge_v                 = 1;
    ldmdnn_cb.mask_floor			    = 1600;
	ldmdnn_cb.mask_floor_hi		        = 160;

	ldmdnn_cb.bfrein3_ns		        = 3;

	ldmdnn_cb.Gcoh_thr				    = 13666;
	ldmdnn_cb.Gcoh_sum_thr			    = 9666;
	ldmdnn_cb.wind_or_en			 	= 1;
	ldmdnn_cb.v_white_en			 	= 1;
	ldmdnn_cb.gu_value				    = 32767;

    bt_ldmdnn_init(&ldmdnn_cb);

    if(xcfg_cb.micr_phase_en == 1){
        cfg_micr_phase_en = 1;
    }

    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

void bt_sco_near_nr_dft_init(u8 *sysclk, nr_cb_t *nr)
{
    nr->nr_type     = NR_TYPE_NONE;
    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;

#if BT_SCO_NR_EN
    nr->nr_type     = NR_TYPE_AINS;
    memset(&ains_cb, 0, sizeof(ains_cb_t));
    ains_cb.nt              = BT_SCO_NR_LEVEL;
    ains_cb.prior_opt_idx   = 10;
    ains_cb.ns_ps_rate		= 1;
    ains_cb.trumpet_en		= 0;
	ains_cb.nt_post         = 0;    //0-6 >0才起效 0为不开gain指数化 开的话默认为3
	ains_cb.exp_range	    = 90;
	bt_ains_init(&ains_cb);
#endif
}

#if BT_SCO_SMIC_AI_EN
void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_AI;
    memset(&dnn_cb, 0, sizeof(dnn_cb_t));

	dnn_cb.param_printf           	= 0;
	dnn_cb.nt                     	= BT_SCO_SMIC_AI_LEVEL;
	dnn_cb.nt_post                	= 0;
	dnn_cb.exp_range_H			   	= 1;
	dnn_cb.exp_range_L			   	= 0;
	dnn_cb.noise_ps_rate          	= 1;
	dnn_cb.prior_opt_idx	       	= 3;
	dnn_cb.prior_opt_ada_en	   		= 1;
	dnn_cb.wind_level			   	= 0;
	dnn_cb.wind_range			   	= 0;
	dnn_cb.low_fre_range          	= 16;
	dnn_cb.low_fre_range0         	= 0;
	dnn_cb.mask_floor			   	= 1600;
	dnn_cb.mask_floor_r		   		= 0;
	dnn_cb.music_lev			   	= 11;
	dnn_cb.comforN_level		   	= 1;
	dnn_cb.gain_expand			   	= 1024;
	dnn_cb.nn_only				   	= 0;
	dnn_cb.nn_only_len			   	= 16;
	dnn_cb.gain_assign			   	= 16666;
	dnn_cb.sin_gain_post_en	   		= 0;
	dnn_cb.sin_gain_post_len	   	= 128;
	dnn_cb.sin_gain_post_len_f	   	= 256;

	bt_dnn_init(&dnn_cb);
    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}
#endif

#if BT_SCO_SMIC_AI_PRO_EN
void bt_sco_dnn_pro_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN_PRO xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_PRO_AI;
    memset(&dnn_pro_cb, 0, sizeof(dnn_pro_cb_t));

	dnn_pro_cb.param_printf           	= 0;
	dnn_pro_cb.gain_floor          	    = 1100;
//	dnn_pro_cb.nt_post                	= 0;
//	dnn_pro_cb.exp_range_H			   	= 1;
//	dnn_pro_cb.exp_range_L			   	= 0;
	dnn_pro_cb.noise_ps_rate          	= 1;
	dnn_pro_cb.prior_opt_idx	       	= 10;
	dnn_pro_cb.prior_opt_ada_en	   		= 1;
//	dnn_pro_cb.wind_level			   	= 0;
//	dnn_pro_cb.wind_range			   	= 0;
	dnn_pro_cb.low_fre_range          	= 15;
	dnn_pro_cb.music_lev			   	= 11;
	dnn_pro_cb.nn_only				   	= 0;
	dnn_pro_cb.nn_only_len			   	= 16;
	dnn_pro_cb.gain_assign			   	= 16666;
	dnn_pro_cb.sin_gain_post_en	   		= 0;
	dnn_pro_cb.sin_gain_post_len	   	= 128;
	dnn_pro_cb.sin_gain_post_len_f	   	= 256;

	bt_dnn_pro_init(&dnn_pro_cb);
    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SCO_AIAEC_DNN_EN
void bt_sco_aiaec_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_AI;
    memset(&aiaec_dnn_cb, 0, sizeof(dnn_aec_ns_cb_t));

	aiaec_dnn_cb.param_printf           	= 0;
	aiaec_dnn_cb.nt                     	= BT_SCO_AIAEC_DNN_LEVEL;
//	aiaec_dnn_cb.exp_range_H			   	= 1;
//	aiaec_dnn_cb.exp_range_L			   	= 0;
	aiaec_dnn_cb.noise_ps_rate          	= 1;
	aiaec_dnn_cb.prior_opt_idx	       	    = 10;
	aiaec_dnn_cb.prior_opt_ada_en           = 1;
	aiaec_dnn_cb.wind_level			   	    = 0;
	aiaec_dnn_cb.wind_range			   	    = 0;
	aiaec_dnn_cb.low_fre_range          	= 15;
	aiaec_dnn_cb.mask_floor			   	    = 600;
//	aiaec_dnn_cb.mask_floor_r		   		= 0;
	aiaec_dnn_cb.music_lev			   	    = 11;
//	aiaec_dnn_cb.gain_expand			   	= 1024;
	aiaec_dnn_cb.nn_only				   	= 0;
	aiaec_dnn_cb.nn_only_len			   	= 16;
	aiaec_dnn_cb.gain_assign                = 32767;
	aiaec_dnn_cb.sin_gain_post_en           = 0;
	aiaec_dnn_cb.sin_gain_post_len          = 128;
	aiaec_dnn_cb.sin_gain_post_len_f        = 0;
    aiaec_dnn_cb.gamma                      = 27852;
	aiaec_dnn_cb.nlp_en                     = 1;
	aiaec_dnn_cb.nlp_level                  = 3;
    aiaec_dnn_cb.smooth_en				    = 1;
    aiaec_dnn_cb.far_post_thr				= 1024;//Q8
    aiaec_dnn_cb.dtd_post_thr				= 6000000;//Q15
    aiaec_dnn_cb.echo_gain_floor			= 100;
    aiaec_dnn_cb.dtd_smooth				    = 29491;
	aiaec_dnn_cb.single_floor				= 0;
	aiaec_dnn_cb.gain_assign_nlp		    = 26666;
	aiaec_dnn_cb.gain_st_thr		        = 12000;
	aiaec_dnn_cb.intensity					= 36;
    nr->rfu[0]                              = 1;    //bypass hw_aec nlp

    bt_aiaec_dnn_init(&aiaec_dnn_cb);
    *sysclk = *sysclk < SYS_100M ? SYS_100M : *sysclk;
}
#endif

#if BT_SCO_NR_USER_SMIC_EN || BT_SCO_NR_USER_DMIC_EN
AT(.bt_voice.sco_nr.user)
void bt_nr_user_proc_do(s16 *mic1, s16 *mic2, nr_cb_t *nr_cb)
{
    //define your nr proc func here, and define nr_input by yourself

    //if you need aec ref signal, you can refer to ↓↓↓
//    if (!bt_sco_far_cache_read((u8*)&nr_input[nr_cb->nr_samples*2], nr_cb->nr_samples*2)) {
//        memset(&nr_input[nr_samples*2], 0, nr_samples * 2);
//    }
}

AT(.bt_voice.sco_nr.user)
void bt_nr_user_init(void *alg_cb, nr_cb_t *nr_cb)
{
    //define your nr init func here
    printf("nr user\n");
}

void bt_sco_nr_user_init(u8 *sysclk, nr_cb_t *nr)
{
    nr->nr_type = NR_TYPE_USER;

    *sysclk = *sysclk < SYS_60M ? SYS_60M : *sysclk;
}
#endif
