/*
 * tmk1553busb.h -- the tmk1553busb v1.9g usb kernel module.
 * (c) ELCUS, 2008.
 */
#ifndef __TMK1553BUSB__
#define __TMK1553BUSB__

#define DRIVER_VERSION "v1.8"
#define DRIVER_AUTHOR "Elcus"
#define DRIVER_DESC "Driver for ELCUS (http://www.elcus.ru) MIL-STD-1553B USB devices"

#define TMKUSB_VER_HI 0x01
#define TMKUSB_VER_LO 0x08

#define TMKUSB_FWVER_MIN 0x0105

/* Define these values to match your device */
#define TA1_USB_01_VENDOR_ID        0x04b4
#define TA1_USB_01_PRODUCT_ID       0x8613

/* names of the tmk1553b usb modules */
#define TMKUSB_DEVICES_NUM        1
#define TA1_USB_01_ID             0x01
char * tmkusb_device_name[TMKUSB_DEVICES_NUM] = {"TA1-USB-01"};

/* Get a minor range for your devices from the usb maintainer */
#define TMK1553BUSB_MINOR_BASE        192

#define _KEVENTD_PID                  2

#define INT_LOOP_MODE                 0
#define MT_LOOP_MODE                  1
#define SLEEP_LOOP_MODE               2

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Structure to hold all of our device specific stuff */
#include "devstruct.h"

typedef struct
{
  s32 nInt;
  u16 wMode;
  u16 awEvData[3];
} TListEvD, *pTListEvD;

typedef struct
{
  struct list_head ProcListEntry;
  pid_t hProc;
  int openCnt;
  int waitFlag;
} TListProc;

/*
 * Ioctl definitions
 */

/* Use 'l' as magic number */
#define TMK_IOC_MAGIC  'k'

#define TMK_IOC0 0

#define VTMK_tmkconfig 2
#define VTMK_tmkdone 3
#define VTMK_tmkgetmaxn 4
#define VTMK_tmkselect 5
#define VTMK_tmkselected 6
#define VTMK_tmkgetmode 7
#define VTMK_tmksetcwbits 8
#define VTMK_tmkclrcwbits 9
#define VTMK_tmkgetcwbits 10
#define VTMK_tmkwaitevents 11
#define VTMK_tmkgetevd 12

#define VTMK_bcreset 13
#define VTMK_bc_def_tldw 14
#define VTMK_bc_enable_di 15
#define VTMK_bc_disable_di 16
#define VTMK_bcdefirqmode 17
#define VTMK_bcgetirqmode 18
#define VTMK_bcgetmaxbase 19
#define VTMK_bcdefbase 20
#define VTMK_bcgetbase 21
#define VTMK_bcputw 22
#define VTMK_bcgetw 23
#define VTMK_bcgetansw 24
#define VTMK_bcputblk 25
#define VTMK_bcgetblk 26
#define VTMK_bcdefbus 27
#define VTMK_bcgetbus 28
#define VTMK_bcstart 29
#define VTMK_bcstartx 30
#define VTMK_bcdeflink 31
#define VTMK_bcgetlink 32
#define VTMK_bcstop 33
#define VTMK_bcgetstate 34

#define VTMK_rtreset 35
#define VTMK_rtdefirqmode 36
#define VTMK_rtgetirqmode 37
#define VTMK_rtdefmode 38
#define VTMK_rtgetmode 39
#define VTMK_rtgetmaxpage 40
#define VTMK_rtdefpage 41
#define VTMK_rtgetpage 42
#define VTMK_rtdefpagepc 43
#define VTMK_rtdefpagebus 44
#define VTMK_rtgetpagepc 45
#define VTMK_rtgetpagebus 46
#define VTMK_rtdefaddress 47
#define VTMK_rtgetaddress 48
#define VTMK_rtdefsubaddr 49
#define VTMK_rtgetsubaddr 50
#define VTMK_rtputw 51
#define VTMK_rtgetw 52
#define VTMK_rtputblk 53
#define VTMK_rtgetblk 54
#define VTMK_rtsetanswbits 55
#define VTMK_rtclranswbits 56
#define VTMK_rtgetanswbits 57
#define VTMK_rtgetflags 58
#define VTMK_rtputflags 59
#define VTMK_rtsetflag 60
#define VTMK_rtclrflag 61
#define VTMK_rtgetflag 62
#define VTMK_rtgetstate 63
#define VTMK_rtbusy 64
#define VTMK_rtlock 65
#define VTMK_rtunlock 66
#define VTMK_rtgetcmddata 67
#define VTMK_rtputcmddata 68

#define VTMK_mtreset 69
#define VTMK_mtdefirqmode 70
#define VTMK_mtgetirqmode 71
#define VTMK_mtgetmaxbase 72
#define VTMK_mtdefbase 73
#define VTMK_mtgetbase 74
#define VTMK_mtputw 75
#define VTMK_mtgetw 76
#define VTMK_mtgetsw 77
#define VTMK_mtputblk 78
#define VTMK_mtgetblk 79
#define VTMK_mtstartx 80
#define VTMK_mtdeflink 81
#define VTMK_mtgetlink 82
#define VTMK_mtstop 83
#define VTMK_mtgetstate 84

#define VTMK_tmkgetinfo 85
#define VTMK_GetVersion 86

#define VTMK_rtenable 87

#define VTMK_mrtgetmaxn 88
#define VTMK_mrtconfig 89
#define VTMK_mrtselected 90
#define VTMK_mrtgetstate 91
#define VTMK_mrtdefbrcsubaddr0 92
#define VTMK_mrtreset 93
#define VTMK_tmktimer 94
#define VTMK_tmkgettimer 95
#define VTMK_tmkgettimerl 96
#define VTMK_bcgetmsgtime 97
#define VTMK_mtgetmsgtime 98
#define VTMK_rtgetmsgtime 99

#define VTMK_tmkgethwver 100
#define VTMK_tmkgetevtime 101
#define VTMK_tmkswtimer 102
#define VTMK_tmkgetswtimer 103

#define VTMK_tmktimeout 104

#define VTMK_MT_Start 112
#define VTMK_MT_GetMessage 113
#define VTMK_MT_Stop 114

#define TMK_IOC_MAXNR 114

#define TMK_IOCtmkconfig _IO(TMK_IOC_MAGIC, VTMK_tmkconfig+TMK_IOC0)
#define TMK_IOCtmkdone _IO(TMK_IOC_MAGIC, VTMK_tmkdone+TMK_IOC0)
#define TMK_IOCtmkgetmaxn _IO(TMK_IOC_MAGIC, VTMK_tmkgetmaxn+TMK_IOC0)
#define TMK_IOCtmkselect _IO(TMK_IOC_MAGIC, VTMK_tmkselect+TMK_IOC0)
#define TMK_IOCtmkselected _IO(TMK_IOC_MAGIC, VTMK_tmkselected+TMK_IOC0)
#define TMK_IOCtmkgetmode _IO(TMK_IOC_MAGIC, VTMK_tmkgetmode+TMK_IOC0)
#define TMK_IOCtmksetcwbits _IO(TMK_IOC_MAGIC, VTMK_tmksetcwbits+TMK_IOC0)
#define TMK_IOCtmkclrcwbits _IO(TMK_IOC_MAGIC, VTMK_tmkclrcwbits+TMK_IOC0)
#define TMK_IOCtmkgetcwbits _IO(TMK_IOC_MAGIC, VTMK_tmkgetcwbits+TMK_IOC0)
#define TMK_IOCtmkwaitevents _IOW(TMK_IOC_MAGIC, VTMK_tmkwaitevents+TMK_IOC0, u64)
//#define TMK_IOCtmkdefevent _IO(TMK_IOC_MAGIC, VTMK_tmkdefevent+TMK_IOC0)
#define TMK_IOCtmkgetevd _IOR(TMK_IOC_MAGIC, VTMK_tmkgetevd+TMK_IOC0, TTmkEventData)

#define TMK_IOCbcreset _IO(TMK_IOC_MAGIC, VTMK_bcreset+TMK_IOC0)
//#define TMK_IOCbc_def_tldw _IO(TMK_IOC_MAGIC, VTMK_bc_def_tldw+TMK_IOC0)
//#define TMK_IOCbc_enable_di _IO(TMK_IOC_MAGIC, VTMK_bc_enable_di+TMK_IOC0)
//#define TMK_IOCbc_disable_di _IO(TMK_IOC_MAGIC, VTMK_bc_disable_di+TMK_IOC0)
#define TMK_IOCbcdefirqmode _IO(TMK_IOC_MAGIC, VTMK_bcdefirqmode+TMK_IOC0)
#define TMK_IOCbcgetirqmode _IO(TMK_IOC_MAGIC, VTMK_bcgetirqmode+TMK_IOC0)
#define TMK_IOCbcgetmaxbase _IO(TMK_IOC_MAGIC, VTMK_bcgetmaxbase+TMK_IOC0)
#define TMK_IOCbcdefbase _IO(TMK_IOC_MAGIC, VTMK_bcdefbase+TMK_IOC0)
#define TMK_IOCbcgetbase _IO(TMK_IOC_MAGIC, VTMK_bcgetbase+TMK_IOC0)
#define TMK_IOCbcputw _IO(TMK_IOC_MAGIC, VTMK_bcputw+TMK_IOC0)
#define TMK_IOCbcgetw _IO(TMK_IOC_MAGIC, VTMK_bcgetw+TMK_IOC0)
#define TMK_IOCbcgetansw _IOR(TMK_IOC_MAGIC, VTMK_bcgetansw+TMK_IOC0, u32)
#define TMK_IOCbcputblk _IOW(TMK_IOC_MAGIC, VTMK_bcputblk+TMK_IOC0, u64)
#define TMK_IOCbcgetblk _IOW(TMK_IOC_MAGIC, VTMK_bcgetblk+TMK_IOC0. u64)
#define TMK_IOCbcdefbus _IO(TMK_IOC_MAGIC, VTMK_bcdefbus+TMK_IOC0)
#define TMK_IOCbcgetbus _IO(TMK_IOC_MAGIC, VTMK_bcgetbus+TMK_IOC0)
#define TMK_IOCbcstart _IO(TMK_IOC_MAGIC, VTMK_bcstart+TMK_IOC0)
#define TMK_IOCbcstartx _IO(TMK_IOC_MAGIC, VTMK_bcstartx+TMK_IOC0)
#define TMK_IOCbcdeflink _IO(TMK_IOC_MAGIC, VTMK_bcdeflink+TMK_IOC0)
#define TMK_IOCbcgetlink _IOR(TMK_IOC_MAGIC, VTMK_bcgetlink+TMK_IOC0, u32)
#define TMK_IOCbcstop _IO(TMK_IOC_MAGIC, VTMK_bcstop+TMK_IOC0)
#define TMK_IOCbcgetstate _IOR(TMK_IOC_MAGIC, VTMK_bcgetstate+TMK_IOC0, u32)

#define TMK_IOCrtreset _IO(TMK_IOC_MAGIC, VTMK_rtreset+TMK_IOC0)
#define TMK_IOCrtdefirqmode _IO(TMK_IOC_MAGIC, VTMK_rtdefirqmode+TMK_IOC0)
#define TMK_IOCrtgetirqmode _IO(TMK_IOC_MAGIC, VTMK_rtgetirqmode+TMK_IOC0)
#define TMK_IOCrtdefmode _IO(TMK_IOC_MAGIC, VTMK_rtdefmode+TMK_IOC0)
#define TMK_IOCrtgetmode _IO(TMK_IOC_MAGIC, VTMK_rtgetmode+TMK_IOC0)
#define TMK_IOCrtgetmaxpage _IO(TMK_IOC_MAGIC, VTMK_rtgetmaxpage+TMK_IOC0)
#define TMK_IOCrtdefpage _IO(TMK_IOC_MAGIC, VTMK_rtdefpage+TMK_IOC0)
#define TMK_IOCrtgetpage _IO(TMK_IOC_MAGIC, VTMK_rtgetpage+TMK_IOC0)
#define TMK_IOCrtdefpagepc _IO(TMK_IOC_MAGIC, VTMK_rtdefpagepc+TMK_IOC0)
#define TMK_IOCrtdefpagebus _IO(TMK_IOC_MAGIC, VTMK_rtdefpagebus+TMK_IOC0)
#define TMK_IOCrtgetpagepc _IO(TMK_IOC_MAGIC, VTMK_rtgetpagepc+TMK_IOC0)
#define TMK_IOCrtgetpagebus _IO(TMK_IOC_MAGIC, VTMK_rtgetpagebus+TMK_IOC0)
#define TMK_IOCrtdefaddress _IO(TMK_IOC_MAGIC, VTMK_rtdefaddress+TMK_IOC0)
#define TMK_IOCrtgetaddress _IO(TMK_IOC_MAGIC, VTMK_rtgetaddress+TMK_IOC0)
#define TMK_IOCrtdefsubaddr _IO(TMK_IOC_MAGIC, VTMK_rtdefsubaddr+TMK_IOC0)
#define TMK_IOCrtgetsubaddr _IO(TMK_IOC_MAGIC, VTMK_rtgetsubaddr+TMK_IOC0)
#define TMK_IOCrtputw _IO(TMK_IOC_MAGIC, VTMK_rtputw+TMK_IOC0)
#define TMK_IOCrtgetw _IO(TMK_IOC_MAGIC, VTMK_rtgetw+TMK_IOC0)
#define TMK_IOCrtputblk _IOW(TMK_IOC_MAGIC, VTMK_rtputblk+TMK_IOC0, u64)
#define TMK_IOCrtgetblk _IOW(TMK_IOC_MAGIC, VTMK_rtgetblk+TMK_IOC0, u64)
#define TMK_IOCrtsetanswbits _IO(TMK_IOC_MAGIC, VTMK_rtsetanswbits+TMK_IOC0)
#define TMK_IOCrtclranswbits _IO(TMK_IOC_MAGIC, VTMK_rtclranswbits+TMK_IOC0)
#define TMK_IOCrtgetanswbits _IO(TMK_IOC_MAGIC, VTMK_rtgetanswbits+TMK_IOC0)
#define TMK_IOCrtgetflags _IOW(TMK_IOC_MAGIC, VTMK_rtgetflags+TMK_IOC0, u64)
#define TMK_IOCrtputflags _IOW(TMK_IOC_MAGIC, VTMK_rtputflags+TMK_IOC0, u64)
#define TMK_IOCrtsetflag _IO(TMK_IOC_MAGIC, VTMK_rtsetflag+TMK_IOC0)
#define TMK_IOCrtclrflag _IO(TMK_IOC_MAGIC, VTMK_rtclrflag+TMK_IOC0)
#define TMK_IOCrtgetflag _IO(TMK_IOC_MAGIC, VTMK_rtgetflag+TMK_IOC0)
#define TMK_IOCrtgetstate _IO(TMK_IOC_MAGIC, VTMK_rtgetstate+TMK_IOC0)
#define TMK_IOCrtbusy _IO(TMK_IOC_MAGIC, VTMK_rtbusy+TMK_IOC0)
#define TMK_IOCrtlock _IO(TMK_IOC_MAGIC, VTMK_rtlock+TMK_IOC0)
#define TMK_IOCrtunlock _IO(TMK_IOC_MAGIC, VTMK_rtunlock+TMK_IOC0)
#define TMK_IOCrtgetcmddata _IO(TMK_IOC_MAGIC, VTMK_rtgetcmddata+TMK_IOC0)
#define TMK_IOCrtputcmddata _IO(TMK_IOC_MAGIC, VTMK_rtputcmddata+TMK_IOC0)

#define TMK_IOCmtreset _IO(TMK_IOC_MAGIC, VTMK_mtreset+TMK_IOC0)
#define TMK_IOCmtdefirqmode _IO(TMK_IOC_MAGIC, VTMK_mtdefirqmode+TMK_IOC0)
#define TMK_IOCmtgetirqmode _IO(TMK_IOC_MAGIC, VTMK_mtgetirqmode+TMK_IOC0)
#define TMK_IOCmtgetmaxbase _IO(TMK_IOC_MAGIC, VTMK_mtgetmaxbase+TMK_IOC0)
#define TMK_IOCmtdefbase _IO(TMK_IOC_MAGIC, VTMK_mtdefbase+TMK_IOC0)
#define TMK_IOCmtgetbase _IO(TMK_IOC_MAGIC, VTMK_mtgetbase+TMK_IOC0)
#define TMK_IOCmtputw _IO(TMK_IOC_MAGIC, VTMK_mtputw+TMK_IOC0)
#define TMK_IOCmtgetw _IO(TMK_IOC_MAGIC, VTMK_mtgetw+TMK_IOC0)
#define TMK_IOCmtgetsw _IO(TMK_IOC_MAGIC, VTMK_mtgetsw+TMK_IOC0)
#define TMK_IOCmtputblk _IOW(TMK_IOC_MAGIC, VTMK_mtputblk+TMK_IOC0, u64)
#define TMK_IOCmtgetblk _IOW(TMK_IOC_MAGIC, VTMK_mtgetblk+TMK_IOC0, u64)
#define TMK_IOCmtstartx _IO(TMK_IOC_MAGIC, VTMK_mtstartx+TMK_IOC0)
#define TMK_IOCmtdeflink _IO(TMK_IOC_MAGIC, VTMK_mtdeflink+TMK_IOC0)
#define TMK_IOCmtgetlink _IOR(TMK_IOC_MAGIC, VTMK_mtgetlink+TMK_IOC0, u32)
#define TMK_IOCmtstop _IO(TMK_IOC_MAGIC, VTMK_mtstop+TMK_IOC0)
#define TMK_IOCmtgetstate _IOR(TMK_IOC_MAGIC, VTMK_mtgetstate+TMK_IOC0, u32)

#define TMK_IOCtmkgetinfo _IOR(TMK_IOC_MAGIC, VTMK_tmkgetinfo+TMK_IOC0, TTmkConfigData)
#define TMK_IOCGetVersion _IO(TMK_IOC_MAGIC, VTMK_GetVersion+TMK_IOC0)

#define TMK_IOCrtenable _IO(TMK_IOC_MAGIC, VTMK_rtenable+TMK_IOC0)
#define TMK_IOCmrtgetmaxn _IO(TMK_IOC_MAGIC, VTMK_mrtgetmaxn+TMK_IOC0)
#define TMK_IOCmrtconfig _IO(TMK_IOC_MAGIC, VTMK_mrtconfig+TMK_IOC0)
#define TMK_IOCmrtselected _IO(TMK_IOC_MAGIC, VTMK_mrtselected+TMK_IOC0)
#define TMK_IOCmrtgetstate _IO(TMK_IOC_MAGIC, VTMK_mrtgetstate+TMK_IOC0)
#define TMK_IOCmrtdefbrcsubaddr0 _IO(TMK_IOC_MAGIC, VTMK_mrtdefbrcsubaddr0+TMK_IOC0)
#define TMK_IOCmrtreset _IO(TMK_IOC_MAGIC, VTMK_mrtreset+TMK_IOC0)

#define TMK_IOCtmktimer _IO(TMK_IOC_MAGIC, VTMK_tmktimer+TMK_IOC0)
#define TMK_IOCtmkgettimer _IOR(TMK_IOC_MAGIC, VTMK_tmkgettimer+TMK_IOC0, __u32)
#define TMK_IOCtmkgettimerl _IO(TMK_IOC_MAGIC, VTMK_tmkgettimerl+TMK_IOC0)
#define TMK_IOCbcgetmsgtime _IOR(TMK_IOC_MAGIC, VTMK_bcgetmsgtime+TMK_IOC0, __u32)
#define TMK_IOCmtgetmsgtime _IOR(TMK_IOC_MAGIC, VTMK_mtgetmsgtime+TMK_IOC0, __u32)
#define TMK_IOCrtgetmsgtime _IOR(TMK_IOC_MAGIC, VTMK_rtgetmsgtime+TMK_IOC0, __u32)

#define TMK_IOCtmkgethwver _IO(TMK_IOC_MAGIC, VTMK_tmkgethwver+TMK_IOC0)

#define TMK_IOCtmkgetevtime _IOR(TMK_IOC_MAGIC, VTMK_tmkgetevtime+TMK_IOC0, __u32)
#define TMK_IOCtmkswtimer _IO(TMK_IOC_MAGIC, VTMK_tmkswtimer+TMK_IOC0)
#define TMK_IOCtmkgetswtimer _IOR(TMK_IOC_MAGIC, VTMK_tmkgetswtimer+TMK_IOC0, __u32)

#define TMK_IOCtmktimeout _IO(TMK_IOC_MAGIC, VTMK_tmktimeout+TMK_IOC0)

#define TMK_IOCMT_Start _IO(TMK_IOC_MAGIC, VTMK_MT_Start+TMK_IOC0)
#define TMK_IOCMT_GetMessage _IOR(TMK_IOC_MAGIC, VTMK_MT_GetMessage+TMK_IOC0, __u32)
#define TMK_IOCMT_Stop _IO(TMK_IOC_MAGIC, VTMK_MT_Stop+TMK_IOC0)

#endif
