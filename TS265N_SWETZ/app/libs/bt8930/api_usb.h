#ifndef _API_USB_H
#define _API_USB_H

enum {
    USBCHK_ONLY_HOST,
    USBCHK_ONLY_DEVICE,
    USBCHK_OTG,
};

enum {
    USB_DISCONNECT,
    USB_UDISK_CONNECTED,
    USB_PC_CONNECTED,
};


//USB Device HID Report id
#define REPORT_ID1              0x01
#define REPORT_ID2              0x02
//USB Device HID Comsumer usage id
#define UDE_HID_PLAYPAUSE       BIT(0)                          //0xcd
#define UDE_HID_VOLUP           BIT(1)                          //0xe9
#define UDE_HID_VOLDOWN         BIT(2)                          //0xea
#define UDE_HID_RESV            BIT(3)                          //0xcf
#define UDE_HID_HOME            BIT(4)                          //0x40
#define UDE_HID_NEXTFILE        BIT(5)                          //0xb5
#define UDE_HID_PREVFILE        BIT(6)                          //0xb6
#define UDE_HID_MUTE            BIT(7)                          //0xe2

void usb_init(void);                                            //��ʼ��ʹ��
void usb_disable(void);                                         //�ر�
u8 usbchk_connect(u8 mode);                             		//usb�ɼ�ⷽʽ
u8 usb_connect(void);                                   		//usb�¼�ⷽʽ
void udisk_remove(void);                                        //�Ƴ�USB�豸
void udisk_insert(void);                                        //����USB�豸ʹ��
void udisk_invalid(void);                                       //usb״̬��ԭ
void usb_bc_init(u8 set);                                       //usb bc����
void udh_dset_valid(u32 set_val);                               //����usb hidʹ��,set_val:��λ��

u8 get_device_addr(void);                                       //��ȡ��������Ľӿڵ�ַ���
void udisk_suspend(void);                                       //����USB�豸
void udisk_resume(void);                                        //�ָ�������豸

void usb_device_enter(u8 enum_type);                            //USB_DEV����ʹ�� enum_type��UDE_STORAGE_EN...
void usb_device_exit(void);                                     //�˳�USB_DEV
void usb_device_process(void);                                  //USB�豸��ѭ��
void ude_tmr_isr(void);                                         //USB�ж�
void pc_remove(void);                                           //USB�ӻ��˳�

bool usb_device_hid_send(u8 report_id, u16 hid_val, u16 rls_time);  //usb hid����, rls_time: 0-disable, 1~0xffff-�Զ��ͷ�ʱ��,��λ5ms
bool ude_hid_buf_send(u8 *buf, u8 len);					        //����hid buff

u8 ude_cfgval_get(void);                                        //��ȡUSBö�ٿ�ʼ��־
u8 get_mobile_sta(void);                                        //��ȡ����״̬,1-��׿�ֻ�

void uda_get_spk_mute(u8 *buf);                                 //��ȡspeaker mute
void uda_set_spk_mute(u8 val);                                  //����speaker mute
u16 uda_get_spk_volume(u8 ch);                                  //��ȡspeaker 0-L,1-R,2-LR����ֵ 
u8 uda_get_spk_spr(void);                                       //��ȡspeaker��ǰ������
u8 uad_get_spk_bits(void);                                      //��ȡspeaker��ǰbits����

void uda_get_mic_mute(u8 *buf);                                 //��ȡmic mute
void uda_set_mic_mute(u8 val);                                  //����mic mute
u8 uda_get_mic_spr(void);                                       //��ȡmic��ǰ������
u16 uda_get_mic_volume(u8 ch);                                  //��ȡmic����
u8 uad_get_mic_bits(void);                                      //��ȡmic��ǰbits����
void set_mic_disframe(u16 discard_time);                        //����mic����ʱ�������ݵ�ʱ��
#endif // _API_USB_H
