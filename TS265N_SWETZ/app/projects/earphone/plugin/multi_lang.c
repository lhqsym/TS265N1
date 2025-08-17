#include "include.h"

#if (LANG_SELECT == LANG_EN_ZH)
const res_addr_t mul_lang_tbl[2][RES_IDX_MAX] = {
    {
        [RES_IDX_LANGUAGE   ] = {&RES_BUF(EN_LANGUAGE),      &RES_LEN(EN_LANGUAGE)},
        [RES_IDX_LOW_BATTERY] = {&RES_BUF(EN_LOW_BATTERY),   &RES_LEN(EN_LOW_BATTERY)},
        [RES_IDX_POWERON    ] = {&RES_BUF(EN_POWERON),       &RES_LEN(EN_POWERON)},
        [RES_IDX_POWEROFF   ] = {&RES_BUF(EN_POWEROFF),      &RES_LEN(EN_POWEROFF)},
        [RES_IDX_MAX_VOL    ] = {&RES_BUF(EN_MAX_VOL),       &RES_LEN(EN_MAX_VOL)},
//        [RES_IDX_SDCARD_MODE] = {&RES_BUF(EN_SDCARD_MODE),   &RES_LEN(EN_SDCARD_MODE)},
//        [RES_IDX_USB_MODE   ] = {&RES_BUF(EN_USB_MODE),      &RES_LEN(EN_USB_MODE)},
//        [RES_IDX_AUX_MODE   ] = {&RES_BUF(EN_AUX_MODE),      &RES_LEN(EN_AUX_MODE)},
//        [RES_IDX_CLOCK_MODE ] = {&RES_BUF(EN_CLOCK_MODE),    &RES_LEN(EN_CLOCK_MODE)},
//        [RES_IDX_FM_MODE    ] = {&RES_BUF(EN_FM_MODE),       &RES_LEN(EN_FM_MODE)},
//        [RES_IDX_SPK_MODE   ] = {&RES_BUF(EN_SPK_MODE),      &RES_LEN(EN_SPK_MODE)},
//        [RES_IDX_PC_MODE    ] = {&RES_BUF(EN_PC_MODE),       &RES_LEN(EN_PC_MODE)},
        [RES_IDX_BT_MODE    ] = {&RES_BUF(EN_BT_MODE),       &RES_LEN(EN_BT_MODE)},
        [RES_IDX_CAMERA_MODE] = {&RES_BUF(EN_CAMERA_MODE),   &RES_LEN(EN_CAMERA_MODE)},
        [RES_IDX_LEFT_CH    ] = {&RES_BUF(EN_LEFT_CH),       &RES_LEN(EN_LEFT_CH)},
        [RES_IDX_RIGHT_CH   ] = {&RES_BUF(EN_RIGHT_CH),     &RES_LEN(EN_RIGHT_CH)},
        [RES_IDX_PAIRING    ] = {&RES_BUF(EN_PAIRING),      &RES_LEN(EN_PAIRING)},
        [RES_IDX_WAIT4CONN  ] = {&RES_BUF(EN_WAIT4CONN),    &RES_LEN(EN_WAIT4CONN)},
        [RES_IDX_CONNECTED  ] = {&RES_BUF(EN_CONNECTED),    &RES_LEN(EN_CONNECTED)},
        [RES_IDX_DISCONNECT ] = {&RES_BUF(EN_DISCONNECT),   &RES_LEN(EN_DISCONNECT)},
        [RES_IDX_CAMERA_ON  ] = {&RES_BUF(EN_CAMERA_ON),    &RES_LEN(EN_CAMERA_ON)},
        [RES_IDX_CAMERA_OFF ] = {&RES_BUF(EN_CAMERA_OFF),   &RES_LEN(EN_CAMERA_OFF)},
        [RES_IDX_CALL_HANGUP] = {&RES_BUF(EN_CALL_HANGUP),  &RES_LEN(EN_CALL_HANGUP)},
        [RES_IDX_CALL_REJECT] = {&RES_BUF(EN_CALL_REJECT),  &RES_LEN(EN_CALL_REJECT)},
        [RES_IDX_REDIALING  ] = {&RES_BUF(EN_REDIALING),    &RES_LEN(EN_REDIALING)},
        [RES_IDX_NUM_0      ] = {&RES_BUF(EN_NUM_0),        &RES_LEN(EN_NUM_0)},
        [RES_IDX_NUM_1      ] = {&RES_BUF(EN_NUM_1),        &RES_LEN(EN_NUM_1)},
        [RES_IDX_NUM_2      ] = {&RES_BUF(EN_NUM_2),        &RES_LEN(EN_NUM_2)},
        [RES_IDX_NUM_3      ] = {&RES_BUF(EN_NUM_3),        &RES_LEN(EN_NUM_3)},
        [RES_IDX_NUM_4      ] = {&RES_BUF(EN_NUM_4),        &RES_LEN(EN_NUM_4)},
        [RES_IDX_NUM_5      ] = {&RES_BUF(EN_NUM_5),        &RES_LEN(EN_NUM_5)},
        [RES_IDX_NUM_6      ] = {&RES_BUF(EN_NUM_6),        &RES_LEN(EN_NUM_6)},
        [RES_IDX_NUM_7      ] = {&RES_BUF(EN_NUM_7),        &RES_LEN(EN_NUM_7)},
        [RES_IDX_NUM_8      ] = {&RES_BUF(EN_NUM_8),        &RES_LEN(EN_NUM_8)},
        [RES_IDX_NUM_9      ] = {&RES_BUF(EN_NUM_9),        &RES_LEN(EN_NUM_9)},
        [RES_IDX_MUSIC_MODE ] = {&RES_BUF(EN_MUSIC_MODE),   &RES_LEN(EN_MUSIC_MODE)},
        [RES_IDX_GAME_MODE  ] = {&RES_BUF(EN_GAME_MODE),    &RES_LEN(EN_GAME_MODE)},
    },
    {
        [RES_IDX_LANGUAGE   ] = {&RES_BUF(ZH_LANGUAGE),     &RES_LEN(ZH_LANGUAGE)},
        [RES_IDX_LOW_BATTERY] = {&RES_BUF(ZH_LOW_BATTERY),  &RES_LEN(ZH_LOW_BATTERY)},
        [RES_IDX_POWERON    ] = {&RES_BUF(ZH_POWERON),      &RES_LEN(ZH_POWERON)},
        [RES_IDX_POWEROFF   ] = {&RES_BUF(ZH_POWEROFF),     &RES_LEN(ZH_POWEROFF)},
        [RES_IDX_MAX_VOL    ] = {&RES_BUF(ZH_MAX_VOL),      &RES_LEN(ZH_MAX_VOL)},
//        [RES_IDX_SDCARD_MODE] = {&RES_BUF(ZH_SDCARD_MODE),  &RES_LEN(ZH_SDCARD_MODE)},
//        [RES_IDX_USB_MODE   ] = {&RES_BUF(ZH_USB_MODE),     &RES_LEN(ZH_USB_MODE)},
//        [RES_IDX_AUX_MODE   ] = {&RES_BUF(ZH_AUX_MODE),     &RES_LEN(ZH_AUX_MODE)},
//        [RES_IDX_CLOCK_MODE ] = {&RES_BUF(ZH_CLOCK_MODE),   &RES_LEN(ZH_CLOCK_MODE)},
//        [RES_IDX_FM_MODE    ] = {&RES_BUF(ZH_FM_MODE),      &RES_LEN(ZH_FM_MODE)},
//        [RES_IDX_SPK_MODE   ] = {&RES_BUF(ZH_SPK_MODE),     &RES_LEN(ZH_SPK_MODE)},
//        [RES_IDX_PC_MODE    ] = {&RES_BUF(ZH_PC_MODE),      &RES_LEN(ZH_PC_MODE)},
        [RES_IDX_BT_MODE    ] = {&RES_BUF(ZH_BT_MODE),      &RES_LEN(ZH_BT_MODE)},
        [RES_IDX_CAMERA_MODE] = {&RES_BUF(ZH_CAMERA_MODE),  &RES_LEN(ZH_CAMERA_MODE)},
        [RES_IDX_LEFT_CH    ] = {&RES_BUF(ZH_LEFT_CH),      &RES_LEN(ZH_LEFT_CH)},
        [RES_IDX_RIGHT_CH   ] = {&RES_BUF(ZH_RIGHT_CH),     &RES_LEN(ZH_RIGHT_CH)},
        [RES_IDX_PAIRING    ] = {&RES_BUF(ZH_PAIRING),      &RES_LEN(ZH_PAIRING)},
        [RES_IDX_WAIT4CONN  ] = {&RES_BUF(ZH_WAIT4CONN),    &RES_LEN(ZH_WAIT4CONN)},
        [RES_IDX_CONNECTED  ] = {&RES_BUF(ZH_CONNECTED),    &RES_LEN(ZH_CONNECTED)},
        [RES_IDX_DISCONNECT ] = {&RES_BUF(ZH_DISCONNECT),   &RES_LEN(ZH_DISCONNECT)},
        [RES_IDX_CAMERA_ON  ] = {&RES_BUF(ZH_CAMERA_ON),    &RES_LEN(ZH_CAMERA_ON)},
        [RES_IDX_CAMERA_OFF ] = {&RES_BUF(ZH_CAMERA_OFF),   &RES_LEN(ZH_CAMERA_OFF)},
        [RES_IDX_CALL_HANGUP] = {&RES_BUF(ZH_CALL_HANGUP),  &RES_LEN(ZH_CALL_HANGUP)},
        [RES_IDX_CALL_REJECT] = {&RES_BUF(ZH_CALL_REJECT),  &RES_LEN(ZH_CALL_REJECT)},
        [RES_IDX_REDIALING  ] = {&RES_BUF(ZH_REDIALING),    &RES_LEN(ZH_REDIALING)},
        [RES_IDX_NUM_0      ] = {&RES_BUF(ZH_NUM_0),        &RES_LEN(ZH_NUM_0)},
        [RES_IDX_NUM_1      ] = {&RES_BUF(ZH_NUM_1),        &RES_LEN(ZH_NUM_1)},
        [RES_IDX_NUM_2      ] = {&RES_BUF(ZH_NUM_2),        &RES_LEN(ZH_NUM_2)},
        [RES_IDX_NUM_3      ] = {&RES_BUF(ZH_NUM_3),        &RES_LEN(ZH_NUM_3)},
        [RES_IDX_NUM_4      ] = {&RES_BUF(ZH_NUM_4),        &RES_LEN(ZH_NUM_4)},
        [RES_IDX_NUM_5      ] = {&RES_BUF(ZH_NUM_5),        &RES_LEN(ZH_NUM_5)},
        [RES_IDX_NUM_6      ] = {&RES_BUF(ZH_NUM_6),        &RES_LEN(ZH_NUM_6)},
        [RES_IDX_NUM_7      ] = {&RES_BUF(ZH_NUM_7),        &RES_LEN(ZH_NUM_7)},
        [RES_IDX_NUM_8      ] = {&RES_BUF(ZH_NUM_8),        &RES_LEN(ZH_NUM_8)},
        [RES_IDX_NUM_9      ] = {&RES_BUF(ZH_NUM_9),        &RES_LEN(ZH_NUM_9)},
        [RES_IDX_MUSIC_MODE ] = {&RES_BUF(ZH_MUSIC_MODE),   &RES_LEN(ZH_MUSIC_MODE)},
        [RES_IDX_GAME_MODE  ] = {&RES_BUF(ZH_GAME_MODE),    &RES_LEN(ZH_GAME_MODE)},
    },
};

void multi_lang_init(uint lang_id)
{
}

const res_addr_t *res_get_ring_num(u8 index)
{
    return &mul_lang_tbl[sys_cb.lang_id][index+RES_IDX_NUM_0];
}

#endif //#if (LANG_SELECT == LANG_EN_ZH)
