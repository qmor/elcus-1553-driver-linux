
/****************************************************************************/
/*      LTMK.H v4.06 for Linux. (c) ELCUS, 2002,2011.                       */
/*      Interface to driver tmk1553b v4.06 for Linux.                       */
/****************************************************************************/

//#ifndef _TMK1553B_
//#define _TMK1553B_

//#define USE_TMK_ERROR

#define TMK_DATA unsigned short
#define TMK_DATA_RET unsigned short
#define WORD unsigned short
#define HANDLE int
#define DWORD unsigned int
#define ULONG unsigned long
#define BOOL int
#define TRUE 1
#define FALSE 0

#ifndef _TMK1553B_MRT
#define _TMK1553B_MRT
#endif

#ifdef _TMK1553B_MRT
#define TMK_VERSION_MIN 0x0403 /* v4.03 */
#define TMK_VERSION 0x0406     /* v4.06 */
#else
#define TMK_VERSION_MIN 0x0403 /* v4.03 */
#define TMK_VERSION 0x0406     /* v4.06 */
#endif

#ifdef _TMK1553B_MRT
#define MAX_TMK_NUMBER (32-1)
#else
#define MAX_TMK_NUMBER (8-1)
#endif //def TMK1553B_MRT

#define MIN_TMK_TYPE 2
#define MAX_TMK_TYPE 12

#define TMK400 2
#define TMKMPC 3
#define RTMK400 4
#define TMKX 5
#define TMKXI 6
#define MRTX 7
#define MRTXI 8
#define TA 9
#define TAI 10
#define MRTA 11
#define MRTAI 12

#define ALL_TMKS 0x00FF

#define GET_TIMEOUT 0xFFFF

#define SWTIMER_OFF   0x0000
#define SWTIMER_ON    0x2400
#define SWTIMER_EVENT 0x8000
#define SWTIMER_RESET 0xFBFF

#define GET_SWTIMER_CTRL 0xFFFF

#define TIMER_RESET 0xFBFF
#define TIMER_OFF   0x0000
#define TIMER_16BIT 0x3400
#define TIMER_32BIT 0x2400
#define TIMER_1US   0x0000
#define TIMER_2US   0x0080
#define TIMER_4US   0x0100
#define TIMER_8US   0x0180
#define TIMER_16US  0x0200
#define TIMER_32US  0x0280
#define TIMER_64US  0x0300
#define TIMER_STOP  0x0380
#define TIMER_SYN   0x0040
#define TIMER_SYND  0x0020
#define TIMER_SA    0x001F

#define TIMER_NOSTOP 0x2000

#define TIMER_MASK  0x37FF
#define TIMER_STEP  0x0380
#define TIMER_BITS  0x3400

#define GET_TIMER_CTRL 0xFFFF

#define DATA_BC_RT 0x00
#define DATA_BC_RT_BRCST 0x08
#define DATA_RT_BC 0x01
#define DATA_RT_RT 0x02
#define DATA_RT_RT_BRCST 0x0A
#define CTRL_C_A 0x03
#define CTRL_C_BRCST 0x0B
#define CTRL_CD_A 0x04
#define CTRL_CD_BRCST 0x0C
#define CTRL_C_AD 0x05

#define BUS_A 0
#define BUS_B 1
#define BUS_1 0
#define BUS_2 1

#define S_ERAO_MASK 0x01
#define S_MEO_MASK 0x02
#define S_IB_MASK 0x04
#define S_TO_MASK 0x08
#define S_EM_MASK 0x10
#define S_EBC_MASK 0x20
#define S_DI_MASK 0x40
#define S_ELN_MASK 0x80
#define S_G1_MASK 0x1000
#define S_G2_MASK 0x2000

#define NWORDS_MASK 0x001F
#define CMD_MASK 0x001F
#define SUBADDR_MASK 0x03E0
#define CI_MASK 0x03E0
#define HBIT_MASK 0x0200
#define RT_DIR_MASK 0x0400
#define ADDRESS_MASK 0xF800
#define RTFL_MASK 0x0001
#define DNBA_MASK 0x0002
#define SSFL_MASK 0x0004
#define BUSY_MASK 0x0008
#define BRCST_MASK 0x0010
#define NULL_MASK 0x00E0
#define SREQ_MASK 0x0100
#define ERROR_MASK 0x0400

#define SREQ 0x01
#define BUSY 0x02
#define SSFL 0x04
#define RTFL 0x08
#define DNBA 0x10

#define CWB0 0x20
#define CWB1 0x40

#define BC_MODE 0x00
#define RT_MODE 0x80
#define MT_MODE 0x100
#define MRT_MODE 0x280
#define UNDEFINED_MODE 0xFFFF

#define RT_TRANSMIT 0x0400
#define RT_RECEIVE 0x0000

#define RT_ERROR_MASK 0x4000

#define RT_FLAG 0x8000
#define RT_FLAG_MASK 0x8000

#define RT_HBIT_MODE 0x0001
#define RT_FLAG_MODE 0x0002
#define RT_BRCST_MODE 0x0004
#define RT_DATA_BL 0x2000
#define RT_GENER1_BL 0x0004
#define RT_GENER2_BL 0x4000
#define BC_GENER1_BL 0x0004
#define BC_GENER2_BL 0x4000
#define MT_GENER1_BL 0x0004
#define MT_GENER2_BL 0x4000
#define TMK_IRQ_OFF 0x8000

#define CX_CC_MASK 0x000F
#define CX_CONT_MASK 0x0010
#define CX_BUS_MASK 0x0020
#define CX_SIG_MASK 0x8000
#define CX_INT_MASK 0x0020

#define CX_CONT 0x0010
#define CX_STOP 0x0000
#define CX_BUS_0 0x0000
#define CX_BUS_A 0x0000
#define CX_BUS_1 0x0020
#define CX_BUS_B 0x0020
#define CX_NOSIG 0x0000
#define CX_SIG 0x8000
#define CX_INT 0x0000
#define CX_NOINT 0x0020

#define SX_NOERR 0
#define SX_MEO 1
#define SX_TOA 2
#define SX_TOD 3
#define SX_ELN 4
#define SX_ERAO 5
#define SX_ESYN 6
#define SX_EBC 7

#define SX_ERR_MASK 0x0007
#define SX_IB_MASK 0x0008
#define SX_G1_MASK 0x0010
#define SX_G2_MASK 0x0020
#define SX_K2_MASK 0x0100
#define SX_K1_MASK 0x0200
#define SX_SCC_MASK 0x3C00
#define SX_ME_MASK 0x4000
#define SX_BUS_MASK 0x8000

#define SX_BUS_0 0x0000
#define SX_BUS_A 0x0000
#define SX_BUS_1 0x8000
#define SX_BUS_B 0x8000

#define GET_IO_DELAY 0xFFFF

#define RT_ENABLE 0x0000
#define RT_DISABLE 0x001F
#define RT_GET_ENABLE 0xFFFF

#define CW(ADDR,DIR,SUBADDR,NWORDS) (((ADDR)<<11)|(DIR)|((SUBADDR)<<5)|((NWORDS)&0x1F))
#define CWM(ADDR,COMMAND) (((ADDR)<<11)|(CI_MASK)|(COMMAND))
#define CWMC(ADDR,CI,COMMAND) (((ADDR)<<11)|((CI)&0x03E0)|(COMMAND))

/*#define CMD_ILLEGAL 0x000*/
#define CMD_DYNAMIC_BUS_CONTROL 0x400
#define CMD_SYNCHRONIZE 0x401
#define CMD_TRANSMIT_STATUS_WORD 0x402
#define CMD_INITIATE_SELF_TEST 0x403
#define CMD_TRANSMITTER_SHUTDOWN 0x404
#define CMD_OVERRIDE_TRANSMITTER_SHUTDOWN 0x405
#define CMD_INHIBIT_TERMINAL_FLAG_BIT 0x406
#define CMD_OVERRIDE_INHIBIT_TERMINAL_FLAG_BIT 0x407
#define CMD_RESET_REMOTE_TERMINAL 0x408
#define CMD_TRANSMIT_VECTOR_WORD 0x410
#define CMD_SYNCHRONIZE_WITH_DATA_WORD 0x011
#define CMD_TRANSMIT_LAST_COMMAND_WORD 0x412
#define CMD_TRANSMIT_BUILT_IN_TEST_WORD 0x413


#define TMK_BAD_0        -1024
#define TMK_BAD_TYPE     (TMK_BAD_0-1)
#define TMK_BAD_IRQ      (TMK_BAD_0-2)
#define TMK_BAD_NUMBER   (TMK_BAD_0-3)
#define BC_BAD_BUS       (TMK_BAD_0-4)
#define BC_BAD_BASE      (TMK_BAD_0-5)
#define BC_BAD_LEN       (TMK_BAD_0-6)
#define RT_BAD_PAGE      (TMK_BAD_0-7)
#define RT_BAD_LEN       (TMK_BAD_0-8)
#define RT_BAD_ADDRESS   (TMK_BAD_0-9)
#define RT_BAD_FUNC      (TMK_BAD_0-10)
#define BC_BAD_FUNC      (TMK_BAD_0-11)
#define TMK_BAD_FUNC     (TMK_BAD_0-12)
#define VTMK_BAD_VERSION (TMK_BAD_0-13)

typedef struct
{
 short nType;
 char szName[10];
 unsigned short wPorts1;
 unsigned short wPorts2;
 unsigned short wIrq1;
 unsigned short wIrq2;
 unsigned short wIODelay;
} TTmkConfigData;

typedef struct
{
  int nInt;
  unsigned short wMode;
  union
  {
    struct
    {
      unsigned short wResult;
      unsigned short wAW1;
      unsigned short wAW2;
    } bc;
    struct
    {
      unsigned short wBase;
      unsigned short wResultX;
    } bcx;
    struct
    {
      unsigned short wStatus;
      unsigned short wCmd;
    } rt;
    struct
    {
      unsigned short wBase;
      unsigned short wResultX;
    } mt;
    struct
    {
      unsigned short wStatus;
    } mrt;
    struct
    {
      unsigned short wRequest;
    } tmk;
  };
} TTmkEventData;

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
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
//#define VTMK_tmkdefevent 11
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

#define VTMK_mrtdefbrcpage 105
#define VTMK_mrtgetbrcpage 106

#define TMK_IOC_MAXNR 106


#define TMK_IOCtmkconfig _IO(TMK_IOC_MAGIC, VTMK_tmkconfig+TMK_IOC0)
#define TMK_IOCtmkdone _IO(TMK_IOC_MAGIC, VTMK_tmkdone+TMK_IOC0)
#define TMK_IOCtmkgetmaxn _IO(TMK_IOC_MAGIC, VTMK_tmkgetmaxn+TMK_IOC0)
#define TMK_IOCtmkselect _IO(TMK_IOC_MAGIC, VTMK_tmkselect+TMK_IOC0)
#define TMK_IOCtmkselected _IO(TMK_IOC_MAGIC, VTMK_tmkselected+TMK_IOC0)
#define TMK_IOCtmkgetmode _IO(TMK_IOC_MAGIC, VTMK_tmkgetmode+TMK_IOC0)
#define TMK_IOCtmksetcwbits _IO(TMK_IOC_MAGIC, VTMK_tmksetcwbits+TMK_IOC0)
#define TMK_IOCtmkclrcwbits _IO(TMK_IOC_MAGIC, VTMK_tmkclrcwbits+TMK_IOC0)
#define TMK_IOCtmkgetcwbits _IO(TMK_IOC_MAGIC, VTMK_tmkgetcwbits+TMK_IOC0)
#define TMK_IOCtmkwaitevents _IOW(TMK_IOC_MAGIC, VTMK_tmkwaitevents+TMK_IOC0, __u64)
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
#define TMK_IOCbcgetansw _IOWR(TMK_IOC_MAGIC, VTMK_bcgetansw+TMK_IOC0, __u32)
#define TMK_IOCbcputblk _IOW(TMK_IOC_MAGIC, VTMK_bcputblk+TMK_IOC0, ULONG[2])
#define TMK_IOCbcgetblk _IOW(TMK_IOC_MAGIC, VTMK_bcgetblk+TMK_IOC0, ULONG[2])
#define TMK_IOCbcdefbus _IO(TMK_IOC_MAGIC, VTMK_bcdefbus+TMK_IOC0)
#define TMK_IOCbcgetbus _IO(TMK_IOC_MAGIC, VTMK_bcgetbus+TMK_IOC0)
#define TMK_IOCbcstart _IO(TMK_IOC_MAGIC, VTMK_bcstart+TMK_IOC0)
#define TMK_IOCbcstartx _IO(TMK_IOC_MAGIC, VTMK_bcstartx+TMK_IOC0)
#define TMK_IOCbcdeflink _IO(TMK_IOC_MAGIC, VTMK_bcdeflink+TMK_IOC0)
#define TMK_IOCbcgetlink _IOR(TMK_IOC_MAGIC, VTMK_bcgetlink+TMK_IOC0, __u32)
#define TMK_IOCbcstop _IO(TMK_IOC_MAGIC, VTMK_bcstop+TMK_IOC0)
#define TMK_IOCbcgetstate _IOR(TMK_IOC_MAGIC, VTMK_bcgetstate+TMK_IOC0, __u32)

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
#define TMK_IOCrtputblk _IOW(TMK_IOC_MAGIC, VTMK_rtputblk+TMK_IOC0, ULONG[2])
#define TMK_IOCrtgetblk _IOW(TMK_IOC_MAGIC, VTMK_rtgetblk+TMK_IOC0, ULONG[2])
#define TMK_IOCrtsetanswbits _IO(TMK_IOC_MAGIC, VTMK_rtsetanswbits+TMK_IOC0)
#define TMK_IOCrtclranswbits _IO(TMK_IOC_MAGIC, VTMK_rtclranswbits+TMK_IOC0)
#define TMK_IOCrtgetanswbits _IO(TMK_IOC_MAGIC, VTMK_rtgetanswbits+TMK_IOC0)
#define TMK_IOCrtgetflags _IOW(TMK_IOC_MAGIC, VTMK_rtgetflags+TMK_IOC0, ULONG[2])
#define TMK_IOCrtputflags _IOW(TMK_IOC_MAGIC, VTMK_rtputflags+TMK_IOC0, ULONG[2])
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
#define TMK_IOCmtputblk _IOW(TMK_IOC_MAGIC, VTMK_mtputblk+TMK_IOC0, ULONG[2])
#define TMK_IOCmtgetblk _IOW(TMK_IOC_MAGIC, VTMK_mtgetblk+TMK_IOC0, ULONG[2])
#define TMK_IOCmtstartx _IO(TMK_IOC_MAGIC, VTMK_mtstartx+TMK_IOC0)
#define TMK_IOCmtdeflink _IO(TMK_IOC_MAGIC, VTMK_mtdeflink+TMK_IOC0)
#define TMK_IOCmtgetlink _IOR(TMK_IOC_MAGIC, VTMK_mtgetlink+TMK_IOC0, __u32)
#define TMK_IOCmtstop _IO(TMK_IOC_MAGIC, VTMK_mtstop+TMK_IOC0)
#define TMK_IOCmtgetstate _IOR(TMK_IOC_MAGIC, VTMK_mtgetstate+TMK_IOC0, __u32)

#define TMK_IOCtmkgetinfo _IOR(TMK_IOC_MAGIC, VTMK_tmkgetinfo+TMK_IOC0, TTmkConfigData)
#define TMK_IOCGetVersion _IO(TMK_IOC_MAGIC, VTMK_GetVersion+TMK_IOC0)

#define TMK_IOCrtenable _IO(TMK_IOC_MAGIC, VTMK_rtenable+TMK_IOC0)

#ifdef _TMK1553B_MRT
#define TMK_IOCmrtgetmaxn _IO(TMK_IOC_MAGIC, VTMK_mrtgetmaxn+TMK_IOC0)
#define TMK_IOCmrtconfig _IO(TMK_IOC_MAGIC, VTMK_mrtconfig+TMK_IOC0)
#define TMK_IOCmrtselected _IO(TMK_IOC_MAGIC, VTMK_mrtselected+TMK_IOC0)
#define TMK_IOCmrtgetstate _IO(TMK_IOC_MAGIC, VTMK_mrtgetstate+TMK_IOC0)
#define TMK_IOCmrtdefbrcsubaddr0 _IO(TMK_IOC_MAGIC, VTMK_mrtdefbrcsubaddr0+TMK_IOC0)
#define TMK_IOCmrtreset _IO(TMK_IOC_MAGIC, VTMK_mrtreset+TMK_IOC0)
#endif //def _TMK1553B_MRT

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

#ifdef _TMK1553B_MRT
#define TMK_IOCmrtdefbrcpage _IO(TMK_IOC_MAGIC, VTMK_mrtdefbrcpage+TMK_IOC0)
#define TMK_IOCmrtgetbrcpage _IO(TMK_IOC_MAGIC, VTMK_mrtgetbrcpage+TMK_IOC0)
#endif //def _TMK1553B_MRT

#ifndef _TMK1553B_
#ifdef USE_TMK_ERROR
extern int tmkError;
#endif
#endif

/*****************Definitions***************************/

int TmkOpen(void);
void TmkClose(void);

int tmkgetmaxn(void);
int tmkconfig(int tmkNumber);
int tmkdone(int tmkNumber);
int tmkselect(int tmkNumber);
int tmkselected(void);
TMK_DATA_RET tmkgetmode(void);
void tmksetcwbits(TMK_DATA tmkSetControl);
void tmkclrcwbits(TMK_DATA tmkClrControl);
TMK_DATA_RET tmkgetcwbits(void);

/*
void bcdefintnorm(void (* UserNormBC)(TMK_DATA, TMK_DATA, TMK_DATA));
void bcdefintexc(void (* UserExcBC)(TMK_DATA, TMK_DATA, TMK_DATA));
void bcdefintx(void (* UserXBC)(TMK_DATA, TMK_DATA));
void bcdefintsig(void (* UserSigBC)(TMK_DATA));
*/
int bcreset(void);
//void bc_def_tldw(TMK_DATA wTLDW);
//void bc_enable_di(void);
//void bc_disable_di(void);
int bcdefirqmode(TMK_DATA bcIrqMode);
TMK_DATA_RET bcgetirqmode(void);
TMK_DATA_RET bcgetmaxbase(void);
int bcdefbase(TMK_DATA bcBasePC);
TMK_DATA_RET bcgetbase(void);
void bcputw(TMK_DATA bcAddr, TMK_DATA bcData);
TMK_DATA_RET bcgetw(TMK_DATA bcAddr);
DWORD bcgetansw(TMK_DATA bcCtrlCode);
void bcputblk(TMK_DATA bcAddr, void *pcBuffer, TMK_DATA cwLength);
void bcgetblk(TMK_DATA bcAddr, void *pcBuffer, TMK_DATA cwLength);
int bcdefbus(TMK_DATA bcBus);
TMK_DATA_RET bcgetbus(void);
int bcstart(TMK_DATA bcBase, TMK_DATA bcCtrlCode);
int bcstartx(TMK_DATA bcBase, TMK_DATA bcCtrlCode);
int bcdeflink(TMK_DATA bcBase, TMK_DATA bcCtrlCode);
DWORD bcgetlink(void);
TMK_DATA_RET bcstop(void);
DWORD bcgetstate(void);

/*
void rtdefintcmd(void (* UserCmdRT)(TMK_DATA));
void rtdefinterr(void (* UserErrRT)(TMK_DATA));
void rtdefintdata(void (* UserDataRT)(TMK_DATA));
*/
int rtreset(void);
int rtdefirqmode(TMK_DATA rtIrqMode);
TMK_DATA_RET rtgetirqmode(void);
int rtdefmode(TMK_DATA rtMode);
TMK_DATA_RET rtgetmode(void);
TMK_DATA_RET rtgetmaxpage(void);
int rtdefpage(TMK_DATA rtPage);
TMK_DATA_RET rtgetpage(void);
int rtdefpagepc(TMK_DATA rtPagePC);
int rtdefpagebus(TMK_DATA rtPageBus);
TMK_DATA_RET rtgetpagepc(void);
TMK_DATA_RET rtgetpagebus(void);
int rtdefaddress(TMK_DATA rtAddress);
TMK_DATA_RET rtgetaddress(void);
void rtdefsubaddr(TMK_DATA rtDir, TMK_DATA rtSubAddr);
TMK_DATA_RET rtgetsubaddr(void);
void rtputw(TMK_DATA rtAddr, TMK_DATA rtData);
TMK_DATA_RET rtgetw(TMK_DATA rtAddr);
void rtputblk(TMK_DATA rtAddr, void *pcBuffer, TMK_DATA cwLength);
void rtgetblk(TMK_DATA rtAddr, void *pcBuffer, TMK_DATA cwLength);
void rtsetanswbits(TMK_DATA rtSetControl);
void rtclranswbits(TMK_DATA rtClrControl);
TMK_DATA_RET rtgetanswbits(void);
void rtgetflags(void *pcBuffer, TMK_DATA rtDir, TMK_DATA rtFlagMin, TMK_DATA rtFlagMax);
void rtputflags(void *pcBuffer, TMK_DATA rtDir, TMK_DATA rtFlagMin, TMK_DATA rtFlagMax);
void rtsetflag(void);
void rtclrflag(void);
TMK_DATA_RET rtgetflag(TMK_DATA rtDir, TMK_DATA rtSubAddr);
TMK_DATA_RET rtgetstate(void);
TMK_DATA_RET rtbusy(void);
void rtlock(TMK_DATA rtDir, TMK_DATA rtSubAddr);
void rtunlock(void);
TMK_DATA_RET rtgetcmddata(TMK_DATA rtBusCommand);
void rtputcmddata(TMK_DATA rtBusCommand, TMK_DATA rtData);

/*
void mtdefintx(void (* UserIntXMT)(TMK_DATA, TMK_DATA));
void mtdefintsig(void (* UserSigMT)(TMK_DATA));
*/
int mtreset(void);
#define mtdefirqmode bcdefirqmode
#define mtgetirqmode bcgetirqmode
#define mtgetmaxbase bcgetmaxbase
#define mtdefbase bcdefbase
#define mtgetbase bcgetbase
#define mtputw bcputw
#define mtgetw bcgetw
TMK_DATA_RET mtgetsw(void);
#define mtputblk bcputblk
#define mtgetblk bcgetblk
#define mtstartx bcstartx
#define mtdeflink bcdeflink
#define mtgetlink bcgetlink
#define mtstop bcstop
#define mtgetstate bcgetstate
TMK_DATA_RET rtenable(TMK_DATA rtEnable);
#ifdef _TMK1553B_MRT
int mrtgetmaxn(void);
DWORD mrtconfig(int tmkNumber);
int mrtselected(void);
TMK_DATA_RET mrtgetstate(void);
void mrtdefbrcsubaddr0(void);
int mrtreset(void);
#endif //def_TMK1553B_MRT

TMK_DATA_RET tmktimer(TMK_DATA tmkTimerCtrl);
DWORD tmkgettimer(void);
TMK_DATA_RET tmkgettimerl(void);
DWORD bcgetmsgtime(void);
#define mtgetmsgtime bcgetmsgtime
DWORD rtgetmsgtime(void);

TMK_DATA_RET tmkgethwver(void);

DWORD tmkgetevtime(void);
TMK_DATA_RET tmkswtimer(TMK_DATA tmkSwTimerCtrl);
DWORD tmkgetswtimer(void);

TMK_DATA_RET tmktimeout(TMK_DATA tmkTimeOut);

#ifdef _TMK1553B_MRT
int mrtdefbrcpage(TMK_DATA rtPage);
TMK_DATA_RET mrtgetbrcpage(void);
#endif //def_TMK1553B_MRT

//#endif
