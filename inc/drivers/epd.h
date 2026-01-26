#ifndef EPD_H
#define EPD_H
#include <stdint.h>

#define PSR_CMD     0x00
#define PWR_CMD     0x01
#define POF_CMD     0x02
#define PFS_CMD     0x03
#define PON_CMD     0x04
#define PMES_CMD    0x05
#define BTST_CMD    0x06
#define DSLP_CMD    0x07

#define DTM1_CMD    0x10
#define DSP_CMD     0x11
#define DRF_CMD     0x12
#define DTM2_CMD    0x13
#define D_SPI_CMD   0x15
#define AUTO_CMD    0x17

#define LUTC_CMD    0x20
#define LUTWW_CMD   0x21
#define LUTR_CMD    0x22
#define LUTW_CMD    0x23
#define LUTK_CMD    0x24
#define B_LUT_CMD   0x25
#define LUTOPT_CMD  0x2A
#define KWOPT_CMD   0x2B

#define PLL_CMD     0x30

#define TSC_CMD     0x40
#define TSE_CMD     0x41
#define TSW_CMD     0x42
#define TSR_CMD     0x43
#define PBC_CMD     0x44

#define CDI_CMD     0x50
#define LPD_CMD     0x51
#define EVS_CMD     0x52

#define TCON_CMD    0x60
#define TRES_CMD    0x61
#define GSST_CMD    0x65

#define REV_CMD     0x70
#define FLG_CMD     0x71

#define AMV_CMD     0x80
#define VV_CMD      0x81
#define VDCS_CMD    0x82

#define PTL_CMD     0x90
#define PTIN_CMD    0x91
#define PTOUT_CMD   0x92

#define PGM_CMD     0xA0
#define APG_CMD     0xA1
#define ROTP_CMD    0xA2

#define CCSET_CMD   0xE0
#define PWS_CMD     0xE3
#define LVSEL_CMD   0xE4
#define TSSET_CMD   0xE5
#define TSBDRY_CMD  0xE7

typedef enum {
    EPD_STATUS_OK,
    EPD_STATUS_ERR
} EPD_Status_t;

EPD_Status_t EPD_Init_bcme2835(void);
EPD_Status_t EPD_Init(void);
EPD_Status_t EPD_Init_4g(void);
EPD_Status_t EPD_DisplayInit(void);
EPD_Status_t EPD_Clear(void);
EPD_Status_t EPD_DisplayImage(const uint8_t* image);
EPD_Status_t EPD_DisplayImage_4g(const uint8_t* image);
void EPD_EnableTemperatureSensor();
void EPD_ReadTemperatureSensor();
void EPD_WaitUntilIdle(void);
void EPD_Reset(void);
void EPD_Sleep(void);
void EPD_Close(void);
void EPD_Update(void);

#endif // EPD_H