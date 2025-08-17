#ifndef _BSP_IIS_H
#define _BSP_IIS_H

//iis ������������
#define IISCFG_TX        BIT(0)
#define IISCFG_RX        BIT(1)
#define IISCFG_SRC       BIT(2)
#define IISCFG_DMA       BIT(3)
#define IISCFG_ONEWIRE   BIT(4)
#define IISCFG_MASTER    BIT(7)   //master or slave sel: 1 master, 0 slave
//iis�������
#define IISCFG_RAMTX    (IISCFG_TX | IISCFG_DMA)
#define IISCFG_RAMRX    (IISCFG_RX | IISCFG_DMA)
//iis MASK BIT
#define IISCFG_TXMASK   (IISCFG_TX | IISCFG_SRC | IISCFG_DMA)
#define IISCFG_RXMASK   (IISCFG_RX | IISCFG_DMA)
#define IISCFG_TXRXMASK (IISCFG_TX | IISCFG_RX  | IISCFG_SRC | IISCFG_DMA)
#define IRQ_I2S_VECTOR                  27
//***************************************
//IIS��audio�����õ��ļĴ���
//#define IISCON0                 SFR_RW (SFR5_BASE + 0x00*4)
//#define IISBAUD                 SFR_RW (SFR5_BASE + 0x01*4)
//#define IISDMACNT               SFR_RW (SFR5_BASE + 0x02*4)
//#define IISDMAOADR0             SFR_RW (SFR5_BASE + 0x03*4)
//#define IISDMAOADR1             SFR_RW (SFR5_BASE + 0x04*4)
//#define IISDMAIADR0             SFR_RW (SFR5_BASE + 0x05*4)
//#define IISDMAIADR1             SFR_RW (SFR5_BASE + 0x06*4)
#define AUBUF0DATA           SFR_RW (SFR12_BASE + 0x01*4)
#define AUBUF0SIZE              SFR_RW (SFR12_BASE + 0x03*4)
#define AUBUF0FIFOCNT           SFR_RW (SFR12_BASE + 0x04*4)
#define PHASECOMP               SFR_RW (SFR13_BASE + 0x0a*4)
#define AUBUF0CON               SFR_RW (SFR12_BASE + 0x00*4)
#define AU0DMAICON              SFR_RW (SFR13_BASE + 0x18*4)
#define AU0DMAIADR              SFR_RW (SFR13_BASE + 0x19*4)
#define AU0DMAISIZE             SFR_RW (SFR13_BASE + 0x1a*4)

#define SETB(REG,POS)           ((REG) |= (1ul << (POS)))
#define CLRB(REG,POS)           ((REG) &= (~(1ul << (POS))))
#define XORB(REG,POS)           ((REG) ^= (1ul << (POS)))
#define CKB1(REG,POS)            REG & (1ul << POS))     //�����Ӧ��BIT�Ƿ�Ϊ1
#define CKB0(REG,POS)           (!(REG & (1ul << POS)))  //�����Ӧ��BIT�Ƿ�Ϊ1

#define IS_IIS_32BIT()    (IISCON0 & BIT(2))
#define IS_IIS_MASTER()   (CKB0(IISCON0,1))

#define SETB(REG,POS)           ((REG) |= (1ul << (POS)))
#define CLRB(REG,POS)           ((REG) &= (~(1ul << (POS))))
#define XORB(REG,POS)           ((REG) ^= (1ul << (POS)))
#define CKB1(REG,POS)            REG & (1ul << POS))     //�����Ӧ��BIT�Ƿ�Ϊ1
#define CKB0(REG,POS)           (!(REG & (1ul << POS)))  //�����Ӧ��BIT�Ƿ�Ϊ1

#define IS_IIS_32BIT()    (IISCON0 & BIT(2))
#define IS_IIS_MASTER()   (CKB0(IISCON0,1))

typedef void (*isr_t)(void);
isr_t register_isr(int vector, isr_t isr);

typedef enum {  //io_map
   IIS_G1 = 0,  //MCLK_PA4, BCLK_PA5, LRC_PA6, DO_PA7, DI_PB3
   IIS_G2 = 1,  //MCLK_PE4, BCLK_PE5, LRC_PE6, DO_PE7, DI_PE0
   IIS_G3 = 2,  //MCLK_PB4, BCLK_PB0, LRC_PB1, DO_PB2, DI_PB5
}TYPE_IIS_IO;

typedef enum {  //bit_mode
   IIS_16BIT = 0,
   IIS_32BIT = 1,
}TYPE_IIS_BIT;

typedef enum {  //data_mode
   IIS_DATA_LEFT_JUSTIFIED = 0,   //left-justified mode (data delay 0 clock after WS change)
   IIS_DATA_NORMAL = 1,           //IIS normal mode  (data delay 1 clock after WS change)
}TYPE_IIS_DATA_FORMAT;

//iisһ���п������ó�8���÷�
typedef enum {  //iis_mode
    //���� src ������
   IIS_MASTER_SRCTX = (IISCFG_MASTER |IISCFG_TX |IISCFG_SRC),
   //���� DMA ������
   IIS_MASTER_RAMTX = (IISCFG_MASTER |IISCFG_TX |IISCFG_DMA),
   //���� DMA �շ�����ͬʱ
   IIS_MASTER_RAMTX_RAMRX = (IISCFG_MASTER | IISCFG_TX | IISCFG_RX | IISCFG_DMA),
   //���� SRC������ DMA������
   IIS_MASTER_SRCTX_RAMRX = (IISCFG_MASTER | IISCFG_TX | IISCFG_SRC | IISCFG_RX | IISCFG_DMA),
   //���� DMA������
   IIS_MASTER_RAMRX =  (IISCFG_MASTER | IISCFG_RX | IISCFG_DMA),
   //���� DMA������ (����ģʽ��DO���ó�DI)
   IIS_MASTER_RAMRX_ONEWIRE =  (IISCFG_MASTER | IISCFG_RX | IISCFG_DMA |IISCFG_ONEWIRE),
   //�ӻ� DMA������
   IIS_SLAVE_RAMRX = IISCFG_RAMRX,
   //�ӻ� DMA������ (����ģʽ��DO���ó�DI)
   IIS_SLAVE_RAMRX_ONEWIRE = (IISCFG_RAMRX | IISCFG_ONEWIRE),
   //�ӻ� DMA������
   IIS_SLAVE_RAMTX = IISCFG_RAMTX,
   //�ӻ� DMA �շ�����ͬʱ
   IIS_SLAVE_RAMTX_RAMRX = (IISCFG_TX | IISCFG_RX | IISCFG_DMA),
}TYPE_IIS_MODE;

typedef enum{  //fsָMCLK/LRC  // MCLK = fs * LRC
    IIS_MCLK_64FS  = 64,
    IIS_MCLK_128FS = 128,
    IIS_MCLK_256FS = 256,
}TYPE_MCLK_SEL;

typedef enum{
    IIS_SPR_48000 = 48000,
    IIS_SPR_44100 = 44100,
    IIS_SPR_16000 = 16000,
}TYPE_SPR_SEL;

typedef enum{
    IIS_MCLK_OUT_DIS = 0,
    IIS_MCLK_OUT_EN =  1,  //ֻ��MASTERģʽ�ſ������MCLK
}TYPE_MCLK_OUT_SEL;

typedef struct {
    u16 samples;
    u16 dmabuf_len;
    u8* dmabuf_ptr;       //TX_RXͬʱ����ʱǰһ����TX,��һ�����RX, ���ֻ��TX��RX,��ȫ������TX��RX
    void(*iis_isr_rx_callbck)(void *buf, u32 samples, bool iis_32bit);   //rx_dma����һ��DMA�����ж�,���Դ�buf��ȡ�����յ�����
    void(*iis_isr_tx_callbck)(void *buf, u32 samples, bool iis_32bit);   //tx_dma����һ��DMA�����ж�,Ҫ����buf����������,�Ա���һ�η���
    u8* txbuf_start_addr;
    u8* rxbuf_start_addr;
    volatile u32 txbuf_idx;
    volatile u32 rxbuf_idx;
}iis_dma_cfg_t;

typedef struct {
    u8 mode;
    u8 iomap        : 2;
    u8 bit_mode     : 1;
    u8 data_mode    : 1;
    u8 mclk_out_en  : 1;
    u8 dma_en       : 1;
    u16 mclk_sel;
    u16 spr_sel;
    //DMA_CFG
    iis_dma_cfg_t   dma_cfg;
}iis_cfg_t;

typedef struct IIS_MODE_STR_T {
    u8 iis_mode;
    const char * iis_str_info;
}IIS_MODE_INFO_TBL;

u32 iis_tx_dma_addr_inc(void);
u32 iis_rx_dma_addr_inc(void);
void iis_irq_init(void);

//API�ӿں���
void iis_cfg_init(iis_cfg_t *cfg);
void iis_start(void);
void iis_stop(void);

void bt_aec_process(u8 *ptr, u32 samples, int ch_mode);
void bt_sco_tx_process(u8 *ptr, u32 samples, int ch_mode);
void bsp_iis_sco_init(void);
#endif
