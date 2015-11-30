//;DrvRtWMode ?
//;rtgetstate 712 or 7f2 ?
//;write to sa 0!?
//;pages on a mother mrt
//; check address written to (r)tmk400/ta and enable/disable
//; options todo:
//;  - enable/disable rtreset on a single RT in MRT (+mrtreset)
//;  - automatic/manual rtenable
// можно добавить отмену DrvRtWMode во флаговом режиме, с отложенным до выключения режима
//;***************************************************************************;
//;*      TMKNLL v7.06                                                       *;
//;*      - for TMKLL4.OBJ v7.06 for DOS.                                    *;
//;*      - for TMKLL4P.OBJ v7.06 for DPMI.                                  *;
//;*      - for TMK1553B.SYS v4.06 for Windows NT4.                          *;
//;*      - for 1553Bwdm.sys v4.06 for Windows 98/ME/2K/XP.                  *;
//;*      - for TMK1553B v4.06 for Linux.                                    *;
//;*      - for TMK1553B v4.06 for QNX4.                                     *;
//;*      - for TMK1553B v4.06 for QNX6.                                     *;
//;*      ELCUS, 1995, 2011.                                                 *;
//;***************************************************************************;
//; проверить работу с памятью rtputflags, rtgetflags
//; можно убрать get_dis_irq/rest_irq в rtgetstate(MRTX) как в rtpeek
//6.00
// - ASM->C
// - TA minimal support
//6.01
// - added __ for internal vars
// - bits = 0 for TA in rtgetanswbits
//6.02
// - TA timer support (w/o user timer interrupts)
// - QNX4 support (w/o MRTX)
// - LINUX support
// - shared irq processing fixed (DOS/QNX4)
// - SMP support for TMKX, MRTX, TA serves:
//     TMKX writes in irq TMK_CtrlPort (RT mode)
//     MRTX writes in irq TMK_CtrlPort, TMK_AddrPort, TMK_DataPort
//     TA writes in irq TA_ADDR, TA_TIMCR, TA_MODE1
// - TA1-PCI4 support
//6.03
// - 80186/80188 support (DOS)
// - QNX4 support with MRTX (used tmkisr.c)
//6.04
// - QNX4 support for VME cards
// - commented __tmkDrvType[num] = 0xFFFF; (in tmkconfig on irq error)
// - TA timeout control support (tmktimeout function)
// - TA mt def/get mode functions
// - tmk def/get dac functions for TH6-PCI
// - rtsetanswbits bug for xTMK400 cards fixed
// - bcdeflink with CX_NOSIG fixed
// - GET_DIS_IRQ_SMP/REST_IRQ_SMP put out of ...PokeTA/...PeekTA
// - added Virtual RTs reset in bcreset (mrtreset)
//7.00
// - MRTA, MRTAI support
//7.01
// - QNX6 support (with separate tmkisr.c)
// - mbcinit, mbcpreparex, mbcstartx, mbcalloc, mbcfree functions
// - MRTA bcreset fixed (possible fake tester interrupt)
// - wrong type bug fixed (in __RT_DIS_MASK, __RT_BRC_MASK)
//7.02
// - updated tmkisr.c for MRTA and QNX6
// - bcstop for TA cards fixed
// - tmktimer single loop fixed
//7.03
// - mrtdefmaxnrt function
// - check of unitialized driver in tmkdone
// - TMKX/MRTX RT double interrupt reset
// - SMP support for TMKX,MRTX,TA,MRTA for Windows (__tmkIrqSpinLockSMP)
// - SMP IRQ for MRTX,MRTA fixed (DrvRtPoke/PeekIrqMRTX, tmkisr.c)
// - added state/timetag copy for brc cmds in MRTA
// - TMK1553B_NOCONFIGH option for LINUX added
//7.04
// - DOS TA timer (other) interrupt restore fixed
// - DrvFlagMode->DrvFlagModeTA reworked
// - TA/MRTA flags off bug fixed
//7.05
// - MRTX(I)/MRTA(I) support for DOS
// - small TMK_INT_OTHER fix (pEvD->awEvData[2] = 0;) in tmkisr.c
//7.06
// - DPMI (DOS 32 bit) support
// - PCI.C for DOS reworked into asm
// - DefIrqAL -> SetIrqAL/ResetIrqAL (DOS,DPMI)
// - pEvD->nInt = 0; added for brc MRT irq when not fWriteSA (here & tmkisr.c)
// - fixed (m)rtgetstate fake active bit for old cmd on TA/MRTA
// - fixed (m)rtgetstate/rtbusy fake active bit when lock|flag on TA/MRTA
// - fixed rtputcmddata/rtgetcmddata (supported by __rtRxDataCmd, DIRQLTmksInt1)

#ifndef NOT_INCLUDE_DEFS
#include "tmkndefs.h"
#endif

//#ifndef STATIC_TMKNUM
//#define DYNAMIC_TMKNUM
//#endif

#if defined(DOS) || defined(QNX4)
typedef struct
{
#if defined(DOS) && !defined(DOS32)
  int nInt;
  U16 empty;
#else
  int long nInt;
#endif
  U16 wMode;
  U16 awEvData[3];
//  PEPROCESS hProc;
  U16 empty1;
  U16 empty2;
} TListEvD, *pTListEvD;
#endif

#define VOID void

#define BOOL char

#ifdef __cplusplus
  #define __CPPARGS ...
  #define __INLINE inline
#else
  #define __CPPARGS void
  #define __INLINE
#endif

//#define INLINE inline

#ifdef DOS

/****************************************************************************/
/*    PCI.C v1.1 provides API to PCI BIOS. ELCUS, 2001, 2011.               */
/****************************************************************************/

#ifdef DOS32
#include <i86.h> /* regs, sregs, int86x() int386x() */
#else
#include <dos.h>
#endif //def DOS32

#define PCI_BIOS_PRESENT 0xB101
#define FIND_PCI_DEVICE 0xB102
#define FIND_PCI_CLASS_CODE 0xB103
#define GENERATE_SPECIAL_CYCLE 0xB106
#define READ_CONFIG_BYTE 0xB108
#define READ_CONFIG_WORD 0xB109
#define READ_CONFIG_DWORD 0xB10A
#define WRITE_CONFIG_BYTE 0xB10B
#define WRITE_CONFIG_WORD 0xB10C
#define WRITE_CONFIG_DWORD 0xB10D
#define GET_IRQ_ROUTING_OPTIONS 0xB10E
#define SET_PCI_IRQ 0xB10F

#define SUCCESSFUL 0

//unsigned int bcdVersion;
//unsigned int wMechanism;
//int nMaxPciBus;
//int __pciBiosPresent;

unsigned short __pciBusDevFun;

#ifdef DOS32
#define regs_w regs.w
#define int86 int386
#else
#define regs_w regs.x
#endif

#ifdef DOS32
int pciPciBiosPresent(void)
{
  union REGS regs;

  regs_w.ax = PCI_BIOS_PRESENT;
  int86(0x1A, &regs, &regs);
  if (regs.x.cflag == 0 && regs.h.ah == 0 &&
      regs.h.dl == 'P' && regs.h.dh == 'C')
  {
//    bcdVersion = regs.x.bx;
//    wMechanism = (unsigned)regs.h.al;
//    nMaxPciBus = (int)regs.h.cl;
    return (//__pciBiosPresent =
            1);
  }
  else
    return (//__pciBiosPresent =
             0);
}

int pciFindPciDevice(unsigned short wDeviceID, unsigned short wVendorID, int short nIndex)
{
  union REGS regs;

  regs_w.ax = FIND_PCI_DEVICE;
  regs_w.cx = wDeviceID;
  regs_w.dx = wVendorID;
  regs_w.si = nIndex;
  int86(0x1A, &regs, &regs);
  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  {
    __pciBusDevFun = regs_w.bx;
    return 1;
  }
  else
    return 0;
}

unsigned char pciReadConfigByte(unsigned short wBusDevFun, unsigned short wReg)
{
  union REGS regs;

  regs_w.ax = READ_CONFIG_BYTE;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  int86(0x1A, &regs, &regs);
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  return regs.h.cl;
}

unsigned short pciReadConfigWord(unsigned short wBusDevFun, unsigned short wReg)
{
  union REGS regs;

  regs_w.ax = READ_CONFIG_WORD;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  int86(0x1A, &regs, &regs);
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  return regs_w.cx;
}

unsigned long pciReadConfigDword(unsigned short wBusDevFun, unsigned short wReg)
{
  union REGS regs;

  regs_w.ax = READ_CONFIG_DWORD;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  int86(0x1A, &regs, &regs);
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  return regs.x.ecx;
}

void pciWriteConfigByte(unsigned short wBusDevFun, unsigned short wReg, unsigned char bData)
{
  union REGS regs;

  regs_w.ax = WRITE_CONFIG_BYTE;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  regs.h.cl = bData;
  int86(0x1A, &regs, &regs);
}

void pciWriteConfigWord(unsigned short wBusDevFun, unsigned short wReg, unsigned short wData)
{
  union REGS regs;

  regs_w.ax = WRITE_CONFIG_WORD;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  regs_w.cx = wData;
  int86(0x1A, &regs, &regs);
}

void pciWriteConfigDword(unsigned short wBusDevFun, unsigned short wReg, unsigned long dwData)
{
  union REGS regs;

  regs_w.ax = WRITE_CONFIG_DWORD;
  regs_w.bx = wBusDevFun;
  regs_w.di = wReg;
  regs.x.ecx = dwData;
  int86(0x1A, &regs, &regs);
}

#else //notDOS32

int pciPciBiosPresent(void)
{
  asm {
    mov  ax, PCI_BIOS_PRESENT;
    int  1Ah;
    jc   pPBP_0;
    or   ah, ah;
    jnz  pPBP_0;
    cmp  dx, 4350h; //"CP";
    jne  pPBP_0;
//    mov  bcdVersion, bx;
//    mov  wMechanism, ax; // because ah == 0
//    xor  ch, ch;
//    mov  nMaxPciBus, cx;
    mov  ax, 1;
    jmp  short pPBP_Exit;
  }
  pPBP_0:
  asm {
    xor  ax, ax;
  }
  pPBP_Exit:
//  asm {
//  mov  __pciBiosPresent, ax;
//  }
  return _AX;
}

int pciFindPciDevice(unsigned short wDeviceID, unsigned short wVendorID, int short nIndex)
{
  asm {
    mov  ax, FIND_PCI_DEVICE;
    mov  cx, wDeviceID;
    mov  dx, wVendorID;
    mov  si, nIndex;
    int  1Ah;
    jc   pFPD_0;
    or   ah, ah;
    jnz  pFPD_0;
    mov  __pciBusDevFun, bx;
    mov  ax, 1;
    jmp  short pFPD_Exit;
  }
  pFPD_0:
  asm {
    xor  ax, ax;
  }
  pFPD_Exit:
  return _AX;
}

unsigned char pciReadConfigByte(unsigned short wBusDevFun, unsigned short wReg)
{
  asm {
    mov  ax, READ_CONFIG_BYTE;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    int  1Ah;
  }
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  return _CL;
}

unsigned short pciReadConfigWord(unsigned short wBusDevFun, unsigned short wReg)
{
  asm {
    mov  ax, READ_CONFIG_WORD;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    int  1Ah;
  }
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
  return _CX;
}

unsigned long pciReadConfigDword(unsigned short wBusDevFun, unsigned short wReg)
{
  asm {
    mov  ax, READ_CONFIG_DWORD;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    int  1Ah;
//  if (regs.x.cflag == 0 && regs.h.ah == SUCCESSFUL)
    mov  dx, cx;
    db   066h, 0c1h, 0e9h, 010h; // shr ecx, 16
    xchg dx, cx;
  }
  return ((unsigned long)_DX << 16) | _CX;
}

void pciWriteConfigByte(unsigned short wBusDevFun, unsigned short wReg, unsigned char bData)
{
  asm {
    mov  ax, WRITE_CONFIG_BYTE;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    mov  cl, bData;
    int  1Ah;
  }
}

void pciWriteConfigWord(unsigned short wBusDevFun, unsigned short wReg, unsigned short wData)
{
  asm {
    mov  ax, WRITE_CONFIG_WORD;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    mov  cx, wData;
    int  1Ah;
  }
}

void pciWriteConfigDword(unsigned short wBusDevFun, unsigned short wReg, unsigned long dwData)
{
  asm {
    mov  ax, WRITE_CONFIG_DWORD;
    mov  bx, wBusDevFun;
    mov  di, wReg;
    mov  cx, word ptr dwData+2;
    db   066h, 0c1h, 0e1h, 010h; // shl ecx, 16
    mov  cx, word ptr dwData;
    int  1Ah;
  }
}
#endif //def DOS32
#endif //def DOS

#ifdef QNX4

#include <sys/kernel.h>
#include <sys/osinfo.h>
#include <sys/pci.h>

#ifdef QNX4VME

#include <sys/_vme.h>

extern int errno;

int __vmeDev;
volatile unsigned short *__vmeWin;

char __tmkClassName[24] = "Tmk1553b Driver Class";

#endif //def QNX4VME

pid_t far tmkInterruptServiceRoutine();

typedef struct {
  unsigned busnum;
  unsigned devfuncnum;
} __TPciBusDevFun;

__TPciBusDevFun __PciBusDevFun, *__pciBusDevFun;

int pciPciBiosPresent(void)
{
  unsigned lastbus, version, hardware;
  struct _osinfo info;

  if (qnx_osinfo(0, &info) == -1 ||
      !(info.sflags & _PSF_PCI_BIOS) ||   // we have a PCI BIOS
      _CA_PCI_BIOS_Present(&lastbus, &version, &hardware) != PCI_SUCCESS)
  {
    return 0;
  }
//  if (lastbus > 0x40)  // KLUDGE FOR AMI BIOS it reports 66 buses
//    lastbus = 0;
  __pciBusDevFun = &__PciBusDevFun;
  return 1;
}

int pciFindPciDevice(unsigned wDeviceID, unsigned wVendorID, int nIndex)
{
  return (_CA_PCI_Find_Device(wDeviceID, wVendorID, nIndex, &__PciBusDevFun.busnum, &__PciBusDevFun.devfuncnum) == PCI_SUCCESS);
}

unsigned char pciReadConfigByte(__TPciBusDevFun *pciBusDevFun, unsigned wReg)
{
  unsigned char Byte;

  _CA_PCI_Read_Config_Byte(pciBusDevFun->busnum, pciBusDevFun->devfuncnum, wReg, 1, &Byte);
  return Byte;
}

unsigned short pciReadConfigWord(__TPciBusDevFun *pciBusDevFun, unsigned wReg)
{
  unsigned short Word;

  _CA_PCI_Read_Config_Word(pciBusDevFun->busnum, pciBusDevFun->devfuncnum, wReg, 1, (char*)&Word);
  return Word;
}

unsigned long pciReadConfigDword(__TPciBusDevFun *pciBusDevFun, unsigned wReg)
{
  unsigned long Dword;

  _CA_PCI_Read_Config_DWord(pciBusDevFun->busnum, pciBusDevFun->devfuncnum, wReg, 1, (char*)&Dword);
  return Dword;
}
#endif //def QNX4

#ifdef LINUX

#ifndef TMK1553B_NOCONFIGH
#include <linux/config.h>
#endif
#ifdef CONFIG_SMP
#define __SMP__
#endif
#ifdef CONFIG_64BIT
#define __64BIT__
#endif
#include <asm/io.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#endif
#endif //def LINUX

#ifdef QNX6

#ifdef CONFIG_SMP
#define __SMP__
#endif
#include <sys/neutrino.h>

#endif //def QNX6

#define CFG_SUB_VID     0x2C
#define CFG_SUB_DID     0x2E
#define CFG_ADDR1       0x14
#define CFG_IRQ         0x3C

#define PLX_VID         0x10B5
#define PLX_DID         0x9030
#define ELCUS_VID       0xE1C5
#define TX1PCI_ID       0x0001
#define TX6PCI_ID       0x0002
#define TX1PCI2_ID      0x0003
#define TX6PCI2_ID      0x0004
#define TA1PCI_ID       0x0005
#define TA1PCI4_ID      0x0006
#define TA1PCI32RT_ID   0x0007

#define IN_DELAY        1
#define OUT_DELAY       1

#define TMK_MIN_TYPE    2
#define TMK_MAX_TYPE    12

#define TMK400          2
#define TMKMPC          3
#define RTMK400         4
#define TMKX            5
#define TMKXI           6
#define MRTX            7
#define MRTXI           8
#define TA              9
#define TAI             10
#define MRTA            11
#define MRTAI           12

#ifndef NTMK
#define NTMK            8
#endif
#ifndef NRT
#define NRT             32
#endif

#ifndef DRV_MAX_BASE
#define DRV_MAX_BASE    1023
#endif

#define TMKXV_VectPort  0x2
#define TAV_VectPort    0x22

#define TMKH_DacPort    0x2
#define TMKX_StopPort   0x4

#define TMK_BasePort    0x2
#define TMK_StartPort   0x4
#define TMK_ResetPort   0x6
#define TMK_ModePort    0x8
#define TMK_AddrPort    0xA
#define TMK_CtrlPort    0xC
#define TMK_StatePort   0xC
#define TMK_DataPort    0xE

#define TMKMPC_BasePort         0xC
#define TMKMPC_StartPort        0xA
#define TMKMPC_ResetPort        0x8
#define TMKMPC_ModePort         0x6
#define TMKMPC_AddrHPort        0x4
#define TMKMPC_AddrLPort        0xE
#define TMKMPC_CtrlHPort        0x3
#define TMKMPC_CtrlLPort        0x2
#define TMKMPC_StateHPort       0x3
#define TMKMPC_StateLPort       0x2
#define TMKMPC_DataHPort        0x1
#define TMKMPC_DataLPort        0x0

#define TA_BASE(i)     (__tmkPortsAddr1[i])
#define TA_IRQ(i)      (__tmkPortsAddr1[i]+2)
#define AdrRCod_Dec(i) (__tmkPortsAddr1[i]+4)
#define TA_RESET(i)    (__tmkPortsAddr1[i]+6)
#define TA_MODE1(i)    (__tmkPortsAddr1[i]+8)
#define TA_ADDR(i)     (__tmkPortsAddr1[i]+0xA)
#define TA_MODE2(i)    (__tmkPortsAddr1[i]+0xC)
#define TA_DATA(i)     (__tmkPortsAddr1[i]+0xE)
#define TA_TIMER1(i)   (__tmkPortsAddr1[i]+0x10)
#define TA_TIMER2(i)   (__tmkPortsAddr1[i]+0x12)
#define AdrRAO(i)      (__tmkPortsAddr1[i]+0x14)
#define TA_TIMCR(i)    (__tmkPortsAddr1[i]+0x18)
#define TA_LCW(i)      (__tmkPortsAddr1[i]+0x1A)
#define TA_MSGA(i)     (__tmkPortsAddr1[i]+0x1C)

#define MRTA_ADDR2(i)  (__tmkPortsAddr1[i]+4)
#define MRTA_SW(i)     (__tmkPortsAddr1[i]+0x16)

#define MRTA_RT_ON 0x0020
#define MRTA_BRC_PAGE 0x003F

#define TA_RT_DATA_INT_BLK 0x0010
#define TA_IRQ_EN 0x0400
#define TA_TXRX_EN 0x03C0
#define TA_BS_MODE_DATA_EN 0x0002
#define TA_FIFO_RESET 0x0004
#define TA_RTMT_START 0x0008

#define TA_BC_START 0x8000
#define TA_BC_STOP 0x2000
#define TA_STOP_ON_EXC 0x000C

#define __TA_14US 0x0000
#define __TA_18US 0x1000
#define __TA_26US 0x2000
#define __TA_63US 0x3000

//Mask RRG1
#define RRG1_WORK_Reset     0xC000
#define RRG1_WORK_RT        0x4000
#define RRG1_WORK_MCO       0x8000
#define RRG1_WORK_MCL       0xC000
#define RRG1_PAUSE_Reset    0x3000
#define RRG1_PAUSE_18       0x1000
#define RRG1_PAUSE_26       0x2000
#define RRG1_PAUSE_63       0x3000
#define RRG1_Dis_MCO        0x0800
#define RRG1_En_IRQ         0x0400
#define RRG1_Dis_BUS_A_td   0x0200
#define RRG1_Dis_BUS_B_td   0x0100
#define RRG1_Dis_BUS_A_rd   0x0080
#define RRG1_Dis_BUS_B_rd   0x0040
#define RRG1_Dis_BS         0x0020
#define RRG1_Dis_IRQ_CW     0x0010
#define RRG1_Start_RT       0x0008
#define RRG1_Reset_FIFO     0x0004
#define RRG1_Dis_tr_BS      0x0002
#define RRG1_Dis_tr_LCW_BS  0x0001

//Mask RRG2
//BC
#define RRG2_BC_Start            0x8000
#define RRG2_BC_En_IRQ_er        0x4000
#define RRG2_BC_END_Stop_IRQ     0x2000
#define RRG2_BC_En_Restart       0x1000
#define RRG2_BC_En_2_Restart     0x0800
#define RRG2_BC_1Res_Activ_Bus   0x0400
#define RRG2_BC_2Res_Activ_Bus   0x0200
#define RRG2_BC_Dis_Flag_Gen     0x0100
#define RRG2_BC_Goto_Rezerv      0x0040
#define RRG2_BC_Dis_Answ_Bit     0x0020
#define RRG2_BC_Mask_Rez_Bit     0x0010
#define RRG2_BC_Stop_IRQ_AnswBit 0x8
#define RRG2_BC_Stop_IRQ_Error   0x4
#define RRG2_BC_Mode_Mask_BC     0x0001
//RT MCO MCL
#define RRG2_RT_ADR                         0xF800
#define RRG2_RT_EN_GRUP                     0x0010
#define RRG2_RT_DIS_IRQ_ERR                 0x0400
#define RRG2_RT_SET_BIT_SR                  0x0100
#define RRG2_RT_RESET_BIT_SR_TR_VECTOR_WORD 0x0080
#define RRG2_RT_SET_BIT_BS                  0x8
#define RRG2_RT_SET_BIT_SF                  0x4
#define RRG2_RT_SET_BIT_TF                  0x1
#define RRG2_RT_EN_BUS_CTRL                 0x2
#define RRG2_RT_SET_INSTR_BIT               0x0200
#define RRG2_MTW_TEST_BUS                   0x40
#define RRG2_MTW_TEST_SYNC                  0x20

#define BCMT_MODE_L     0x0000

#define TMK400_INT1_MASK        0x40
#define TMK400_INT2_MASK        0x80
#define TMKMPC_INT1_MASK        0x8000
#define RTMK400_INT1_MASK       0x02
#define RTMK400_INT2_MASK       0x01

#define TMK400CWBitsMask        0x0060

#define RT_CLEAR_INT    0x03E0
#define RTAnswBitsMask  0x001F

#define DRV_HBIT_MODE           0x0001
#define DRV_FLAG_MODE           0x0002
#define DRV_BRCST_MODE          0x0004
#define RTMK400_HBIT_MODE       0x0020
#define RTMK400_FLAG_MODE       0x0040
#define RTMK400_BRCST_MODE      0x0080
//;TMKX_HBIT_MODE  0000001000000000b
#define TMKX_HBIT_MODE  0x0000
#define TMKX_FLAG_MODE  0x0400
#define TMKX_BRCST_MODE 0x0100
//;MRTX_HBIT_MODE  0000001000000000b
#define MRTX_HBIT_MODE  0x0000
#define MRTX_FLAG_MODE  0x0400
#define MRTX_BRCST_MODE 0x0100
#define TA_HBIT_MODE    0x0200
#define TA_BRCST_MODE   0x0010

#define TX_RT_DATA_INT_BLK 0x2000
#define GENER1_BLK      0x0004
#define GENER2_BLK      0x4000

#if NRT == 0
#ifdef MRTX
#undef MRTX
#endif
#ifdef MRTXI
#undef MRTXI
#endif
#ifdef MRTA
#undef MRTA
#endif
#ifdef MRTAI
#undef MRTAI
#endif
#endif

#ifdef DOS32
//#pragma pack(4);
int __DataBegin = 0;
int DrvCodeBegin()
{
  return 0x1234;
}

int __fTmkDeep = 0;
U16 __ftmkInt1[NTMK];
#endif //def DOS32

BOOL __tmkIsUserType[TMK_MAX_TYPE+1+1] = 
{
  0, 0,
#ifdef TMK400
  1, 1, 1, //TMK400, TMKMPC, RTMK000
#else
  0, 0, 0,
#endif
#ifdef TMKX
  1, 1, //TMKX, TMKXI
#else
  0, 0,
#endif
#ifdef MRTX
  1, 1, //MRTX, MRTXI
#else
  0, 0,
#endif
#ifdef TA
  1, 1, //TA, TAI
#else
  0, 0,
#endif
#ifdef MRTA
  1, 1, //MRTA, MRTAI
#else
  0, 0,
#endif
  0
};

#define __TMK400 0
#define __TMKMPC 1
#define __RTMK400 2
#define __TMKX 3
#define __TA 4

#if !defined(MRTX) && !defined(MRTA)
#define DRV_MAX_TYPE 4
#endif
#if defined(MRTX) && !defined(MRTA)
#define __MRTX 5
#define  DRV_MAX_TYPE 5
#endif
#if !defined(MRTX) && defined(MRTA)
#define __MRTA 5
#define  DRV_MAX_TYPE 5
#endif
#if defined(MRTX) && defined(MRTA)
#define __MRTX 5
#define __MRTA 6
#define  DRV_MAX_TYPE 6
#endif

U16 __tmkUser2DrvType[TMK_MAX_TYPE+1] = 
{
  0xFFFF,
  0xFFFF,
  __TMK400,  //TMK400
  __TMKMPC,  //TMKMPC
  __RTMK400, //RTMK400
  __TMKX,    //TMKX
  __TMKX,    //TMKXI
#ifdef MRTX
  __MRTX,    //MRTX
  __MRTX,    //MRTXI
#else
  0xFFFF,
  0xFFFF,
#endif
  __TA,      //TA
  __TA,      //TAI
#ifdef MRTA
  __MRTA,    //MRTA
  __MRTA     //MRTAI
#else
  0xFFFF,
  0xFFFF,
#endif
};

//        PUBLIC  tmkError
//        PUBLIC  __rtDisableMask

//;        PUBLIC  _tmkListErr

//#define tmkError tmkError_

BOOL __tmkFirstTime = 1;

U16 __tmkTimerCtrl[NTMK]; // DUP(0)
U16 __tmkTimeOut[NTMK]; //DUP(0)

U16 __tmkHWVer[NTMK]; // DUP(0)
//unsigned AdrTab = 1023;
#define AdrTab 1023

BOOL __FLAG_MODE_ON[NTMK+NRT]; // DUP(0)

U16 __BC_RT_FLAG[NTMK+NRT][32];
U16 __RT_BC_FLAG[NTMK+NRT][32];


//int fTmkEventSet[NTMK]; // DUP(0)
U16 __wInDelay[NTMK]; // DUP(1)
U16 __wOutDelay[NTMK]; // DUP(1)
int __tmkMaxNumber = NTMK-1;

#ifdef STATIC_TMKNUM
int tmkError = 0;
#else
int tmkError[NTMK+NRT]; // DUP(0)
#endif

#ifdef STATIC_TMKNUM
int volatile __tmkNumber = 0;
#endif
//;tmkRealNumber2  DD      0
int __amrtNumber[NTMK+NRT]; // DUP(0)

#if defined(DOS) || defined(QNX4)
int volatile __tmkSaveNumber;
int volatile __tmkInIrqNumber;

#if NRT > 0
#define MRT_READ_BRC_DATA 1L
#define MRT_WRITE_BRC_DATA 1L
U32 adwTmkOptions[NTMK+NRT];

U16 awBrcRxState[NTMK][32*4];
U16 awBrcRxBuf[NTMK][32*32];
#endif //NRT

TListEvD __aEvData[NTMK+NRT];
#endif //def DOS
#ifdef WIN95
//;__tmkSaveNumber   DD      ?
//;__tmkInIrqNumber  DD      ?
//;_tmkListErr     U32[NTMK]; //!!! DUP(0)
#endif

U16 __tmkPortsAddr1[NTMK];
U16 __tmkPortsAddr2[NTMK];
U16 __tmkDrvType[NTMK+NRT]; // DUP(0xFFFF)
U16 __tmkUserType[NTMK+NRT];
U16 __tmkMode[NTMK+NRT]; // DUP(0xFFFF)
U16 __tmkRAMSize[NTMK];
#if defined(DOS) || defined(QNX4)
U16 __tmkPci[NTMK];// DUP(0)
#endif

#if NRT > 0
int __mrtMaxNRT = 0;
int __mrtMinRT[NTMK]; // DUP(0)
int __mrtNRT[NTMK]; // DUP(1)
U16 __mrtCtrl0[NTMK+NRT];
U16 __mrtCtrl1[NTMK+NRT];
U16 __mrtMask0[NTMK+NRT];
U16 __mrtMask1[NTMK+NRT];
U32 __dmrtRT[NTMK]; //DUP(0L)
U32 __dmrtBrc[NTMK]; //DUP(0L)
U08 __mrtA2RT[NTMK][32];
U16 __mrtLastBrcTxRT[NTMK]; //DUP(0xFF)
#endif //NRT

BOOL __tmkRAMInWork[NTMK]; // DUP(0)
unsigned __tmkRAMAddr[NTMK]; // DUP(0)
#if defined(DOS) || defined(QNX4)
#ifdef RAMwoCLI
!!! not ready yet
unsigned __tmkRAMRestored[NTMK]; //!!! DUP(0)
#endif //def RAMwoCLI
BOOL __tmkSaveRAMInWork = 0;
unsigned __tmkSaveRAMAddr = 0;
unsigned __nFoundDev;
#endif //def DOS

//;                IRPC    N, 0123
//;VTMK&N&_Int1_Desc DD &N& //;VPICD_IRQ_Descriptor <255, VPICD_Opt_Read_Hw_IRR OR VPICD_OPT_REF_DATA, OFFSET32 DrvTmksInt1>
//;VTMK&N&_Int2_Desc DD &N& //;VPICD_IRQ_Descriptor <255, VPICD_Opt_Read_Hw_IRR OR VPICD_OPT_REF_DATA, OFFSET32 DrvTmksInt2>
//;                ENDM
//;
//;tmkDrvInt1Desc  LABEL   DWORD
//;                IRPC    N, 0123
//;                DD      VTMK&N&_Int1_Desc
//;                ENDM
//;tmkDrvInt2Desc  LABEL   DWORD
//;                IRPC    N, 0123
//;                DD      VTMK&N&_Int2_Desc
//;                ENDM
//;
//;tmkIRQ1Handle   U32 [NTMK]; //!!! DUP(0)
//;tmkIRQ2Handle   U32 [NTMK]; //!!! DUP(0)

#if defined(DOS)

#if (NTMK > 0)
RETIR TYPIR DrvTmk0Int1(__CPPARGS);
#endif
#if NTMK > 1
RETIR TYPIR DrvTmk1Int1(__CPPARGS);
#endif
#if NTMK > 2
RETIR TYPIR DrvTmk2Int1(__CPPARGS);
#endif
#if NTMK > 3
RETIR TYPIR DrvTmk3Int1(__CPPARGS);
#endif
#if NTMK > 4
RETIR TYPIR DrvTmk4Int1(__CPPARGS);
#endif
#if NTMK > 5
RETIR TYPIR DrvTmk5Int1(__CPPARGS);
#endif
#if NTMK > 6
RETIR TYPIR DrvTmk6Int1(__CPPARGS);
#endif
#if NTMK > 7
RETIR TYPIR DrvTmk7Int1(__CPPARGS);
#endif

RETIR (TYPIR *tmkDrvInt1[NTMK])(__CPPARGS) = {
#if NTMK > 0
  DrvTmk0Int1
#endif
#if NTMK > 1
  , DrvTmk1Int1
#endif
#if NTMK > 2
  , DrvTmk2Int1
#endif
#if NTMK > 3
  , DrvTmk3Int1
#endif
#if NTMK > 4
  , DrvTmk4Int1
#endif
#if NTMK > 5
  , DrvTmk5Int1
#endif
#if NTMK > 6
  , DrvTmk6Int1
#endif
#if NTMK > 7
  , DrvTmk7Int1
#endif
#if NTMK > 8
Not Ready Yet
#endif
};

#ifdef DOS
void (TYPIR *tmkOldInt1[NTMK])(__CPPARGS);
#endif

#endif //def DOS

#ifdef DOS32

#if (NTMK > 0)
RETIR TYPIR rmDrvTmk0Int1(__CPPARGS);
#endif
#if NTMK > 1
RETIR TYPIR rmDrvTmk1Int1(__CPPARGS);
#endif
#if NTMK > 2
RETIR TYPIR rmDrvTmk2Int1(__CPPARGS);
#endif
#if NTMK > 3
RETIR TYPIR rmDrvTmk3Int1(__CPPARGS);
#endif
#if NTMK > 4
RETIR TYPIR rmDrvTmk4Int1(__CPPARGS);
#endif
#if NTMK > 5
RETIR TYPIR rmDrvTmk5Int1(__CPPARGS);
#endif
#if NTMK > 6
RETIR TYPIR rmDrvTmk6Int1(__CPPARGS);
#endif
#if NTMK > 7
RETIR TYPIR rmDrvTmk7Int1(__CPPARGS);
#endif

RETIR (TYPIR *tmkDrvInt1RM[NTMK])(__CPPARGS) = {
#if NTMK > 0
  rmDrvTmk0Int1
#endif
#if NTMK > 1
  , rmDrvTmk1Int1
#endif
#if NTMK > 2
  , rmDrvTmk2Int1
#endif
#if NTMK > 3
  , rmDrvTmk3Int1
#endif
#if NTMK > 4
  , rmDrvTmk4Int1
#endif
#if NTMK > 5
  , rmDrvTmk5Int1
#endif
#if NTMK > 6
  , rmDrvTmk6Int1
#endif
#if NTMK > 7
  , rmDrvTmk7Int1
#endif
#if NTMK > 8
Not Ready Yet
#endif
};

U16 rmtmkDrvInt1[NTMK][2]; //seg:off
U16 rmtmkOldInt1[NTMK][2]; //seg:off
//#pragma pack(1);
struct rmcs {
  U32 rm_EDI;
  U32 rm_ESI;
  U32 rm_EBP;
  U32 rm_Reserved;
// rm_BX           LABEL   WORD
  U32 rm_EBX;
  U32 rm_EDX;
  U32 rm_ECX;
// rm_AX           LABEL   WORD
  U32 rm_EAX;
  U16 rm_Flags;
  U16 rm_ES;
  U16 rm_DS;
  U16 rm_FS;
  U16 rm_GS;
  U16 rm_IP;
  U16 rm_CS;
  U16 rm_SP;
  U16 rm_SS;
} __rmCallStruc;
//#pragma pack(4);
U16 volatile __SaveES;
#endif //def DOS32

#ifdef QNX4
int __tmkiid[NTMK];
#endif

BOOL __tmkStarted[NTMK]; // DUP(0)

U16 __bcControls1[NTMK];
U16 __bcControls[NTMK];
U16 __bcBus[NTMK]; // DUP(0)
#if defined(DOS) || defined(QNX4)
U16 volatile __bcSaveBase;
#endif //def DOS
U16 __bcMaxBase[NTMK]; // DUP(0)
#if DRV_MAX_BASE > 255
U16 __mtMaxBase[NTMK]; // DUP(0)
#endif
U16 __bcBaseBus[NTMK];
U16 __bcBasePC[NTMK];
U16 __bcAW1Pos[NTMK];
U16 __bcAW2Pos[NTMK];
//U16 bcAW1[NTMK];
//U16 bcAW2[NTMK];
BOOL __bcXStart[NTMK];

U08 __bcExt2StdResult[] = 
{
  0x00, 0x02, 0x08, 0x08, 
  0x80, 0x01, 0x02, 0x20, 
  0x04, 0x06, 0x0C, 0x0C, 
  0x84, 0x05, 0x06, 0x24
};

//;bcStd2ExtResult DB      00h, 05h, 01h, 05h
//;                DB      08h, 0Dh, 09h, 0Dh
//;                DB      03h, 05h, 01h, 05h
//;                DB      0Bh, 0Dh, 09h, 0Dh
//;                DB      07h, 05h, 01h, 05h
//;                DB      0Fh, 0Dh, 09h, 0Dh
//;                DB      03h, 05h, 01h, 05h
//;                DB      0Bh, 0Dh, 09h, 0Dh
//;                DB      07h, 05h, 01h, 05h
//;                DB      0Fh, 0Dh, 09h, 0Dh
//;                DB      03h, 05h, 01h, 05h
//;                DB      0Bh, 0Dh, 09h, 0Dh
//;                DB      07h, 05h, 01h, 05h
//;                DB      0Fh, 0Dh, 09h, 0Dh
//;                DB      03h, 05h, 01h, 05h
//;                DB      0Bh, 0Dh, 09h, 0Dh


#ifdef WIN95
U32 hTmkEvent[NTMK]; //!!! DUP(0)
//;hIrqEvent       U32 [NTMK]; //!!! DUP(0)
//;hCurVM          U32 [NTMK]; //!!! DUP(0)
//;hSysVM          DD      0
//;EXTERN _hSysVM: DWORD
//;EXTERN _ahlEvData: DWORD

//;EXTERN _lpwOut: DWORD
//;EXTERN _VTMK_BC_Event_Callback: DWORD
#endif

void FARIR retfLabel(void);

#if defined(DOS) || defined(QNX4)
RETIR FARIR retfLabel1(U16 arg1);
RETIR FARIR retfRLabel1(U16 arg1);
RETIR FARIR retfRLabel2(U16 arg1, U16 arg2);
RETIR FARIR retfRLabel3(U16 arg1, U16 arg2, U16 arg3);
#endif

#if defined(DOS) || defined (QNX4)
RETIR (FARIR *tmkUserNormBC[NTMK])(U16, U16, U16); // DUP(retfRLabel)
RETIR (FARIR *tmkUserExcBC[NTMK])(U16, U16, U16); // DUP(retfRLabel)
RETIR (FARIR *tmkUserSigBC[NTMK])(U16); // DUP(retfLabel)
RETIR (FARIR *tmkUserXBC[NTMK])(U16, U16); // DUP(retfRLabel)

RETIR (FARIR *tmkUserSigMT[NTMK])(U16); // DUP(retfLabel)
RETIR (FARIR *tmkUserXMT[NTMK])(U16, U16); // DUP(retfRLabel)
#endif// def DOS

U16 __rtControls[NTMK+NRT]; // DUP(0)
//MRTA(realnum) - MODE1
U16 __rtControls1[NTMK+NRT]; // DUP(0)
//MRTA(realnum) - MODE2
//MRTA(num) - SW
U16 __rtPagePC[NTMK+NRT];
U16 __rtPageBus[NTMK+NRT];
U16 __rtAddress[NTMK+NRT]; //DUP(00FFh)
//;rtMaskAddr      U16 [NTMK+NRT]; DUP(001Fh)
//;rtMaskBrc       U16 [NTMK+NRT]; //!!! DUP(0)
U16 __rtMaxPage[NTMK+NRT]; // DUP(0)
#if defined(DOS) || defined(QNX4)
U16 volatile __rtSaveMode;
U16 volatile __rtSaveSubAddr;
#endif //def DOS
U16 __rtMode[NTMK+NRT]; // DUP(0)
U16 __rtSubAddr[NTMK+NRT]; // DUP(0)
U16 __hm400Page[NTMK+NRT]; // DUP(0)
#if NRT > 0
#ifdef MRTA
U16 __hm400Page2[NTMK+NRT]; // DUP(0) current ADDR2
U16 __hm400Page0[NTMK+NRT]; // DUP(0) starting page
#endif //def MRTA
#endif
U16 __rtRxDataCmd[NTMK+NRT][5];
U16 __rtDisableMask[NTMK+NRT];
U16 __rtBRCMask[NTMK+NRT];
BOOL __rtEnableOnAddr[NTMK+NRT]; // DUP(1)
U16 __RT_DIS_MASK0[TMK_MAX_TYPE+1] = 
{
  0,       //; none
  0,       //; none
  0x001F,  //; TMK400
  0x001F,  //; RTMK400
  0x001F,  //; TMKMPC
  0xF800,  //; TMKX
  0xF800,  //; TMKXI
  0x001F,  //; MRTX
  0x001F,  //; MRTXI
  0xF800,  //; TA
  0xF800,  //; TAI
  0x001F,  //; MRTA
  0x001F   //; MRTAI
};
U16 __RT_BRC_MASK0[TMK_MAX_TYPE+1] = 
{
  0,       //; none
  0,       //; none
  0xFFFF,  //; TMK400
  0xFF7F,  //; RTMK400
  0xFFFF,  //; TMKMPC
  0xFEFF,  //; TMKX
  0xFEFF,  //; TMKXI
  0xFEFF,  //; MRTX
  0xFEFF,  //; MRTXI
  0xFFEF,  //; TA
  0xFFEF,  //; TAI
  0x7FFF,  //; MRTA
  0x7FFF   //; MRTAI
};

U16 __RT_DIS_MASK[DRV_MAX_TYPE+1];
U16 __RT_BRC_MASK[DRV_MAX_TYPE+1];

#if defined(DOS) || defined(QNX4)
RETIR (FARIR *tmkUserCmdRT[NTMK+NRT])(U16); // DUP(retfRLabel)
RETIR (FARIR *tmkUserErrRT[NTMK+NRT])(U16); // DUP(retfRLabel)
RETIR (FARIR *tmkUserDataRT[NTMK+NRT])(U16); // DUP(retfRLabel)
void DrvMaskTmk(int num);
void DrvUnmaskTmk(int num);
RETIR DrvTmksInt1(int num);
#endif //def DOS

#ifdef DOS32
U08 __tmkPic20Base = 0x08;
U08 __tmkPicA0Base = 0x70;
#endif //def DOS32

#if defined(DOS) || defined(QNX4)
#ifdef MASKTMKS
#if NRT > 0
!!! NOT SUPPORTED
#endif //NRT
#ifdef DOS32
!!! NOT SUPPORTED
#endif //def DOS32
#ifndef CPU188
U08 tmkSaveMask21 = 0;
U08 tmkAllMask21 = 0;
U08 tmkSaveMaskA1 = 0;
U08 tmkAllMaskA1 = 0;
#else //def CPU188
U16 tmkSaveMask21 = 0;
U16 tmkAllMask21 = 0;
#endif //ndef CPU188
#endif //def MASKTMKS

#ifndef CPU188
U08 __tmkMask21[NTMK]; // DUP(0)
U08 __tmkMaskA1[NTMK]; // DUP(0)
#else //def CPU188
U16 __tmkMask21[NTMK]; // DUP(0)
#endif //ndef CPU188
U08 __tmkIrq1[NTMK]; // DUP(0xFF)
BOOL __tmkIrqShared[NTMK]; // DUP(0)
U16 __tmkIrqPort[NTMK];
U16 __tmkIrqBit[NTMK];
#ifdef QNX4VME
U08 __tmkIrqLevel[NTMK];
#endif //def QNX4VME
#endif //def DOSQNX4

#ifdef WIN95
U08 __tmkIrq1[NTMK];
#endif

U16 __mtCW[NTMK];
#if DRV_MAX_BASE > 255
U16 __bcLinkBaseN[NTMK][DRV_MAX_BASE+1];
U16 __bcLinkCCN[NTMK][DRV_MAX_BASE+1];
#else
U16 __bcLinkWN[NTMK][DRV_MAX_BASE+1];
#endif
////U32 bcLinkWPtr[NTMK];
////                DD      bcLinkW0
////                IRPC    N, 1234567
////                DD      bcLinkW&N&
////                ENDM
U16 __bcCmdWN[NTMK][DRV_MAX_BASE+1];
////U32 bcCmdWPtr[NTMK];
////                DD      bcCmdW0
////                IRPC    N, 1234567
////                DD      bcCmdW&N&
////                ENDM

#ifdef NMBCID
int __mbcAlloc[NMBCID];
int __mbci[NMBCID];
U16 __mbcBase[NMBCID][NTMK];
U16 __mbcTmkN[NMBCID][NTMK];
U16 __mbcPort[NMBCID][NTMK];
U16 __mbcData[NMBCID][NTMK];
U16 __mbcPort0[NMBCID][NTMK];
U16 __mbcData0[NMBCID][NTMK];
#endif //NMBCID

char __TMKLL_Ver[] = "TMKNLL v7.06";
#ifdef CPU188
char __ch186[] = "-188";
#endif
#ifdef RAMwoCLI
char __chRAMwoCLI[] = ".a";
#else
char __chRAMwoCLI[] = ".b";
#endif
#ifdef MASKTMKS
char __chMASKTMKS[] = ".a";
#else
char __chMASKTMKS[] = ".b";
#endif

//;_DATA  ENDS
//;VXD_LOCKED_DATA_ENDS

#ifdef DOS
/*
void __INLINE GET_DIS_IRQ()
{
  asm pushf;
  asm cli;
}

void __INLINE REST_IRQ()
{
  asm popf;
}
*/
#define IRQ_FLAGS
#ifdef DOS32
void GET_DIS_IRQ();
#pragma aux GET_DIS_IRQ = \
" push    eax " \
" mov     eax, 0900h " \
" int     31h " \
" xchg    eax, [esp] " \
__parm [] __modify [esp];
/*
" pushfd " \
" cli " \
*/
void REST_IRQ();
#pragma aux REST_IRQ = \
" xchg    eax, [esp] " \
" int     31h " \
" pop     eax " \
__parm [] __modify [esp];
/*
" popfd " \
*/
#else //notDOS32
#define GET_DIS_IRQ() { \
  asm pushf; \
  asm cli; \
}
#define REST_IRQ() { \
  asm popf; \
}
#endif //def DOS32
#define GET_MUTEX GET_DIS_IRQ()
#define REST_MUTEX REST_IRQ()
#define GET_DIS_IRQ_SMP()
#define REST_IRQ_SMP()

#else //notDOS
/*
_inline void GET_DIS_IRQ()
{
  __asm
  {
    pushfd
    cli
  }
}

_inline void REST_IRQ()
{
  __asm
  {
    popfd
  }
}
*/
#ifdef QNX4
#define IRQ_FLAGS
#pragma aux GET_DIS_IRQ = "pushfd" "cli" __parm [] __modify [esp];
void GET_DIS_IRQ();
#pragma aux REST_IRQ = "popfd" __parm [] __modify [esp];
void REST_IRQ();
#define GET_MUTEX
#define REST_MUTEX
#define GET_DIS_IRQ_SMP()
#define REST_IRQ_SMP()

#else
#ifdef LINUX
/*
#define IRQ_FLAGS
#define GET_DIS_IRQ() { \
  asm volatile ( \
  "pushfl\n\t" \
  "cli\n\t" \
  : \
  : \
  : "esp"); \
}
#define REST_IRQ() { \
  asm volatile ( \
  "popfl\n\t" \
  : \
  : \
  : "esp"); \
}
*/
//#define IRQ_FLAGS unsigned irq_flags
//#define GET_DIS_IRQ() { save_flags(irq_flags); cli(); }
//#define REST_IRQ() { restore_flags(irq_flags); }
#define IRQ_FLAGS
extern spinlock_t tmkIrqSpinLock;
#define GET_DIS_IRQ() { spin_lock_irq(&tmkIrqSpinLock); }
#define REST_IRQ() { spin_unlock_irq(&tmkIrqSpinLock); }
#define GET_MUTEX
#define REST_MUTEX
#ifdef __SMP__
#define GET_DIS_IRQ_SMP() { spin_lock_irq(&tmkIrqSpinLock); }
#define REST_IRQ_SMP() { spin_unlock_irq(&tmkIrqSpinLock); }
#else
#define GET_DIS_IRQ_SMP()
#define REST_IRQ_SMP()
#endif //def __SMP__

#else
#ifdef QNX6
#define IRQ_FLAGS
/*#define GET_DIS_IRQ() { \
  asm ("pushfl"); \
  asm ("cli"); \
}
#define REST_IRQ() { \
  asm ("popfl"); \
}*/
extern intrspin_t  tmkIrqSpinLock; 
#define GET_DIS_IRQ(){InterruptLock(&tmkIrqSpinLock);}
#define REST_IRQ(){InterruptUnlock(&tmkIrqSpinLock);}
#define GET_MUTEX
#define REST_MUTEX
#ifdef __SMP__
#define GET_DIS_IRQ_SMP(){InterruptLock(&tmkIrqSpinLock);}
#define REST_IRQ_SMP(){InterruptUnlock(&tmkIrqSpinLock);}
#else
#define GET_DIS_IRQ_SMP()
#define REST_IRQ_SMP()
#endif

#else // WIN etc.
#define IRQ_FLAGS
volatile unsigned long __tmkIrqSpinLockSMP = 1;
//#define GET_DIS_IRQ() { \
//  __asm {pushfd}; \
//  __asm {cli}; \
//}
//#define REST_IRQ() { \
//  __asm {popfd}; \
//}
#define GET_MUTEX
#define REST_MUTEX
//#define GET_DIS_IRQ_SMP()
//#define REST_IRQ_SMP()

#define GET_DIS_IRQ() \
{ \
  unsigned long _eiptmp, _irqflags; \
  { \
    __asm {pushfd}; \
    __asm {cli}; \
    __asm {pop eax}; \
    __asm {mov _irqflags, eax}; \
    while (1) \
    { \
      __asm {lock dec __tmkIrqSpinLockSMP}; \
      __asm {pushfd}; \
      __asm {pop eax}; \
      __asm {mov _eiptmp, eax}; \
      if ((_eiptmp & 0x80) == 0) \
        break; \
      while (__tmkIrqSpinLockSMP <= 0) \
      { \
        __asm {rep nop}; \
      } \
    } \
  }

#define REST_IRQ() \
  { \
  __tmkIrqSpinLockSMP = 1; \
  __asm {mov eax, _irqflags}; \
  __asm {push eax}; \
  __asm {popfd}; \
  } \
}

#define GET_DIS_IRQ_SMP() GET_DIS_IRQ()

#define REST_IRQ_SMP() REST_IRQ()

#endif //def QNX6
#endif //def LINUX
#endif //def QNX4
#endif //def DOS

/*
#define GET_DIS_IRQ { \
  __asm \
  { \
     pushfd \
     cli \
  } \
}
#define REST_IRQ { \
  __asm \
  { \
    popfd \
  } \
}
*/

#ifdef DOS

#ifdef DOS32
#define outpb outp
#define inpb inp
#else
#define outpw outport
#define inpw inport
#define outpb outportb
#define inpb inportb
#endif

#else
#ifdef LINUX

#define outpw(port, data) outw(data, port)
#define inpw(port) inw(port)
#define outpb(port, data) outb(data, port)
#define inpb(port) inb(port)

#else
#ifdef QNX4VME

#define outpw myoutpw
#define inpw myinpw
#define inpb inp
#define outpb outp

//#pragma off (check_stack);

void myoutpw(U16 addr, U16 data)
{
//!!! works with single window only!!!
  *(__vmeWin + ((unsigned)addr>>1)) = (U16)(((data>>8) & 0x00FF) | ((data<<8) & 0xFF00));
}
//MYOUT           MACRO
//                xchg    al, ah
//                mov     [edx], ax
//                xchg    al, ah
//                ENDM

U16 myinpw(U16 addr)
{
  register data;
//!!! works with single window only!!!
  data = *(__vmeWin + ((unsigned)addr>>1));
  return (U16)(((data>>8) & 0x00FF) | ((data<<8) & 0xFF00));
}
//MYIN            MACRO
//                movzx   eax, word ptr [edx]
//                xchg    al, ah
//                ENDM

//#pragma on (check_stack);

#else
/*
#pragma warning(disable:4035)

unsigned __inline inpb(unsigned port)
{
  __asm
  {
    xor eax, eax
    mov edx, port
    in al, dx
  }
}

unsigned __inline inpw(unsigned port)
{
  __asm
  {
    xor eax, eax
    mov edx, port
    in ax, dx
  }
}

#pragma warning(default:4035)

void __inline outpb(unsigned port, unsigned data)
{
  __asm
  {
    mov eax, data
    mov edx, port
    out dx, al
  }
}

void __inline outpw(unsigned port, unsigned data)
{
  __asm
  {
    mov eax, data
    mov edx, port
    out dx, ax
  }
}
*/

#define inpb inp
#define outpb outp

//void ___outpw(unsigned port, unsigned data) { if (port < 0x8000 || port > 0xF000) { __asm { int 3 }; } outpw(port, data); }
//unsigned ___inpw(unsigned port) { if (port < 0x8000 || port > 0xF000) { __asm { int 3 }; } return inpw(port); }

#endif //def QNX4VME
#endif //def LINUX
#endif //def DOS

#define DrvBcDefBaseTA(realnum, base) { \
  __bcBasePC[realnum] = base; \
}

__INLINE
void DrvBcPokeTA(int realnum, unsigned pos, unsigned data)
{
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  outpw(TA_DATA(realnum), data);
////  DrvBcPoke(realnum, TA, pos, data);
}

//PokeTA(PeekTA()) does not work in macro
//#define DrvBcPokeTA(realnum, pos, data) { \
//  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos); \
//  outpw(TA_DATA(realnum), data); \
//}

//__INLINE
void DrvBcDefBase(int realnum, unsigned type, unsigned base);
//__INLINE
//void DrvBcDefBaseTA(int realnum, unsigned base);

void DrvBcPoke(int realnum, unsigned type, unsigned pos, unsigned data);
//__INLINE
//void DrvBcPokeTA(int realnum, unsigned pos, unsigned data);

unsigned DrvBcPeek(int realnum, unsigned type, unsigned pos);
//__INLINE
//unsigned DrvBcPeekTA(int realnum, unsigned pos);
//__inline
__INLINE
unsigned DrvBcPeekTA(int realnum, unsigned pos)
{
  unsigned data;
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  data = inpw(TA_DATA(realnum));
  return data;
//  return DrvBcPeek(realnum, TA, pos);
}

//#define DrvBcPeekTA(realnum, pos) \
//  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos), \
//  inpw(TA_DATA(realnum));

void DrvRtPoke(int __tmkNumber, unsigned type, unsigned pos, unsigned data);
//void DrvRtPokeTA(int __tmkNumber, unsigned base, unsigned pos, unsigned data);

unsigned DrvRtPeek(int __tmkNumber, unsigned type, unsigned pos);
//unsigned DrvRtPeekTA(int __tmkNumber, unsigned base, unsigned pos);

__INLINE
void DrvRtPokeTA(int realnum, unsigned base, unsigned pos, unsigned data)
{
  outpw(TA_ADDR(realnum), (base << 6) + pos);
  outpw(TA_DATA(realnum), data);
//  DrvBcPoke(realnum, TA, pos, data);
}


//#define DrvRtPokeTA(realnum, base, pos, data) { \
//  outpw(TA_ADDR(realnum), (base << 6) + pos); \
//  outpw(TA_DATA(realnum), data); \
//}

__INLINE
unsigned DrvRtPeekTA(int realnum, unsigned base, unsigned pos)
{
  unsigned data;
  outpw(TA_ADDR(realnum), (base << 6) + pos);
  data = inpw(TA_DATA(realnum));
  return data;
//  return DrvBcPeek(realnum, TA, pos);
}

//#define DrvRtPeekTA(realnum, base, pos) \
//  outpw(TA_ADDR(realnum), (base << 6) + pos), \
//  inpw(TA_DATA(realnum));

void DrvRtWMode(int num, unsigned type, unsigned mode);
void DrvFlagModeTA(int num, int m);

void (FARIR *tmkUserErrors)(void) = retfLabel;

#define DEF_VAR(type, var) type var

#define GET_VAR(varloc, varpar) varloc = varpar

#define GET_RealNum (__amrtNumber[__tmkNumber])
#define GET_RealNumOf(num) (__amrtNumber[num])
#define PUT_RealNum(num) {}

#define PUSH_RealNum {}
#define POP_RealNum {}

/*
GET_RealNum2    MACRO   reg
                mov     reg, __tmkNumber2
                movzx   reg, __amrtNumber2[reg]
                ENDM
GET_RealNum2R   MACRO   reg, reg2
                movzx   reg, __amrtNumber2[reg2]
                ENDM

PUT_RealNum2    MACRO   reg
                ENDM
PUSH_RealNum2   MACRO
                ENDM

POP_RealNum2    MACRO
                ENDM
*/



#define SYN_LPT { \
  outpb(0x378, 1); \
  outpb(0x378, 0); \
}

#define INT3 { asm db 0cch; }

#ifdef STATIC_TMKNUM

#define CLRtmkError { \
  tmkError = 0; \
}

#else

#ifdef NTMKLL_CLR_TMK_ERROR
#define CLRtmkError { \
  tmkError[__tmkNumber] = 0; \
}
#else
#define CLRtmkError {}
#endif

#endif //def DOS

/*
CLRtmkError    MACRO
#ifdef   NTMKLL_CLR_TMK_ERROR
                mov     eax, __tmkNumber2
                mov     tmkError[eax*2], 0
#endif
                ENDM
*/

/*
SYN_LPT         MACRO
                push    eax
                push    edx
                mov     edx, 378h
                mov     eax, 1
                out     dx, ax
                mov     eax, 0
                out     dx, ax
                pop     edx
                pop     eax
                ENDM
*/

#define CONVERT_TMKX_SW_BITS(bits, bitst) { \
    bits = bitst & 0x04; \
    if (bitst & 0x01) \
      bits |= 0x02; \
    if (bitst & 0x02) \
      bits |= 0x01; \
    if (bitst & 0x08) \
      bits |= 0x10; \
    if (bitst & 0x10) \
      bits |= 0x08; \
}

#define CONVERT_TA_SW_BITS(bits, bitst) { \
    bits = 0; \
    if (bitst & SREQ) \
      bits |= SREQ_MASK; \
    if (bitst & BUSY) \
      bits |= BUSY_MASK; \
    if (bitst & SSFL) \
      bits |= SSFL_MASK; \
    if (bitst & RTFL) \
      bits |= RTFL_MASK; \
    if (bitst & DNBA) \
      bits |= DNBA_MASK; \
}

#ifdef STATIC_TMKNUM
#define USER_ERROR(err) (tmkError = err)
#define USER_ERROR_R USER_ERROR
#else
#define USER_ERROR(err) (tmkError[__tmkNumber] = err)
#define USER_ERROR_R(err) (err)
#endif

/*
USER_ERROR      MACRO   err
                mov     eax, __tmkNumber2
                mov     tmkError[eax*2], err
                mov     eax, err
//;                call    dword ptr tmkUserErrors
                ENDM

USER_ERROR_R    MACRO   err
                mov     eax, err
//;                call    dword ptr tmkUserErrors
                ENDM
*/

#ifndef NOCHECK
#define CHECK_TMK_REAL_NUMBER(num) { \
  if (num >= NTMK) \
    return USER_ERROR_R(TMK_BAD_NUMBER); \
}
#else
#define CHECK_TMK_REAL_NUMBER(num)
#endif

#ifndef NOCHECK
#define CHECK_TMK_NUMBER(num) { \
  if ((unsigned)num > __tmkMaxNumber) \
    return USER_ERROR_R(TMK_BAD_NUMBER); \
}
#else
#define CHECK_TMK_NUMBER(num)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE(num, ReqMode) { \
  if (__tmkMode[realnum] != ReqMode) \
    return USER_ERROR(TMK_BAD_FUNC); \
}
#else
#define CHECK_TMK_MODE(num, ReqMode)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE_L(num, ReqMode) { \
  if ((__tmkMode[realnum] & 0x00FF) != ReqMode) \
    return USER_ERROR(TMK_BAD_FUNC); \
}
#else
#define CHECK_TMK_MODE_L(num, ReqMode)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE_LN(num, ReqMode) { \
  if ((__tmkMode[realnum] & 0x00FF) != ReqMode) \
  { \
    USER_ERROR(TMK_BAD_FUNC); \
    return; \
  } \
}
#else
#define CHECK_TMK_MODE_LN(num, ReqMode)
#endif

#ifndef NOCHECK
#ifdef DOS
#ifndef CPU188
#define CHECK_IRQ(irq) { \
  if (irq > 0xF) \
    return USER_ERROR(TMK_BAD_IRQ); \
}
#else //def CPU188
#define CHECK_IRQ(irq) { \
  if (irq > 0x6) \
    return USER_ERROR(TMK_BAD_IRQ); \
}
#endif //ndef CPU188
#else
#define CHECK_IRQ(irq)
#endif //def DOS
#else
#define CHECK_IRQ(irq)
#endif

#ifdef QNX4VME
#define CHECK_TMK_TYPE_1(type) { \
  if ((unsigned)(type) != TMKX && (unsigned)(type) != MRTX && (unsigned)(type) != TA) \
    return USER_ERROR_R(TMK_BAD_TYPE); \
}
#else
#ifndef NOCHECK
#define CHECK_TMK_TYPE_1(type) { \
  if ((unsigned)(type) < TMK_MIN_TYPE || (unsigned)(type) > TMK_MAX_TYPE) \
    return USER_ERROR_R(TMK_BAD_TYPE); \
}
#else
#define CHECK_TMK_TYPE_1(type)
#endif
#endif //def QNX4VME

#ifndef NOCHECK
#define CHECK_TMK_TYPE_2(type) { \
  if ((unsigned)(type) > DRV_MAX_TYPE) \
    return USER_ERROR_R(TMK_BAD_TYPE); \
}
#else
#define CHECK_TMK_TYPE_2(type)
#endif

#define CHECK_TMK_TYPE(type)
//;#ifndef NOCHECK
//;                cmp     reg, TMK_MIN_TYPE
//;                jb      &lbl&_TmkBadType
//;                cmp     reg, TMK_MAX_TYPE
//;                jbe     &lbl&_TmkTypeOk
//;&lbl&_TmkBadType:
//;                USER_ERROR(TMK_BAD_TYPE
//;                jmp     &lbl&_Exit
//;#endif

#define CHECK_BC_BASE_BX(num, base) { \
  if (base > __bcMaxBase[num]) \
    return USER_ERROR(BC_BAD_BASE); \
}

#if DRV_MAX_BASE > 255
#define CHECK_BCMT_BASE_BX(num, base) { \
  if ((__tmkMode[num] == BC_MODE && base > __bcMaxBase[num]) || \
      (__tmkMode[num] != BC_MODE && base > __mtMaxBase[num])) \
    return USER_ERROR(BC_BAD_BASE); \
}
#else
#define CHECK_BCMT_BASE_BX CHECK_BC_BASE_BX
#endif

#define CHECK_BC_ADDR(addr) { addr &= 0x3F; }

#define CHECK_BC_LEN(len) { \
  if (len > 64) \
  { \
    USER_ERROR(BC_BAD_LEN); \
    return; \
  } \
}

#define CHECK_BC_BUS(bus) { bus &= 1; }

#define CHECK_BC_CTRL(ctrl) { ctrl &= 0xF; }

#define CHECK_BC_CTRLX(ctrlx) { ctrlx &= 0x803F; }

#define CHECK_MT_CTRLX(ctrlx) { ctrlx &= 0x8030; }

#define CHECK_RT_DIR(dir) { dir &= 0x0400; }

#define CHECK_RT_SUBADDR(sa) { sa &= 0x1F; }

#define CHECK_RT_SUBPOS(pos) { pos &= 0x1F; }

#define CHECK_RT_LEN(len) { \
  if (len > 32) \
  { \
    USER_ERROR(RT_BAD_LEN); \
    return; \
  } \
}

#define CHECK_RT_PAGE_BX(num, page) { \
  if (page > __rtMaxPage[num]) \
    return USER_ERROR(RT_BAD_PAGE); \
}

#define CHECK_RT_ADDRESS(addr) { \
  if (addr > 0x1E) \
    return USER_ERROR(RT_BAD_ADDRESS); \
}

#define CHECK_RT_CMD(cmd) { cmd &= 0x041F; }

#ifdef DOS

#ifdef DOS32
#define IRQ2INT(irq) (((irq) >= 8) ? (irq) - 0x08 + __tmkPicA0Base : (irq) + __tmkPic20Base)
#else
#ifndef CPU188
#define IRQ2INT(irq) (((irq) >= 8) ? (irq) + 0x68 : (irq) + 0x08)
#else //def CPU188
// INT0-INT4 -> 0Ch-10h; INT5-INT6 -> 0Ah-0Bh
#define IRQ2INT(irq) (((irq) >= 5) ? (irq) + 0x05 : (irq) + 0x0C)
#endif //ndef CPU188
#endif //def DOS32

#ifdef MASKTMKS
#ifndef CPU188
#define DrvMaskTmks { \
  tmkSaveMask21 = inpb(0x21); \
  outpb(0x21, tmkSaveMask21 | tmkAllMask21); \
  tmkSaveMaskA1 = inpb(0xA1); \
  outpb(0xA1, tmkSaveMaskA1 | tmkAllMaskA1); \
}
#define DrvUnmaskTmks { \
  outpb(0x21, tmkSaveMask21); \
  outpb(0xA1, tmkSaveMaskA1); \
}
#else //def CPU188
#define DrvMaskTmks { \
  tmkSaveMask21 = inpw(0xFF28); \
  _AX = tmkSaveMask21 | tmkAllMask21; \
  outpb(0xFF28, _AL); \
}
#define DrvUnmaskTmks { \
  _AX = tmkSaveMask21; \
  outpb(0xFF28, _AL); \
}
#endif //ndef CPU188
#endif //def MASKTMKS

#ifndef CPU188
#define DrvEndInt(num) { \
  if (__tmkMaskA1[num]) \
    outpb(0xA0, 0x20); \
  outpb(0x20, 0x20); \
}
#else //def CPU188
#define DrvEndInt(num) { \
  _AX = 0x8000; \
  outpb(0xFF22, _AL); \
}
#endif //ndef CPU188

#endif //def DOS

//;_TEXT  SEGMENT PARA USE32 PUBLIC 'CODE'
//;VXD_LOCKED_CODE_SEG
//                .CODE

void MyUserErrors(unsigned err)
{
#ifdef STATIC_TMKNUM
  tmkError = err;
  tmkUserErrors();
#endif
}

unsigned inpb_d(int num, unsigned port)
{
  unsigned t;
  unsigned data;

  data = inpb(port);
  t = __wInDelay[num];
  while (--t);
  return data;
}

unsigned inpw_d(int num, unsigned port)
{
  unsigned t;
  unsigned data;

  data = inpw(port);
  t = __wInDelay[num];
  while (--t);
  return data;
}

void outpb_d(int num, unsigned port, unsigned data)
{
  unsigned t;

  outpb(port, data);
  t = __wOutDelay[num];
  while (--t);
  return;
}

void outpw_d(int num, unsigned port, unsigned data)
{
  unsigned t;

  outpw(port, data);
  t = __wOutDelay[num];
  while (--t);
  return;
}

#define REP_OUTSWB_D { \
  do \
  { \
    unsigned data; \
    data = *(buf++); \
    /*lodsw*/ \
    ++port; \
    GET_DIS_IRQ(); \
    outpb(port, data >> 8);\
    --port; \
    outpb_d(realnum, port, data); \
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
  } \
  while (--len != 0); \
}

#define REP_INSW { \
  do \
  { \
    GET_DIS_IRQ(); \
    *(buf++) = inpw(port); \
    /*insw*/ \
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
  } \
  while (--len != 0); \
}

#define REP_OUTSW { \
  do \
  { \
    GET_DIS_IRQ(); \
    outpw(port, *(buf++)); \
    /*outsw*/ \
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
  } \
  while (--len != 0); \
}

// can be macro only because of --port !
#define outpwb_d(num, port, data) { \
  GET_DIS_IRQ(); \
  outpb(port, (data) >> 8); \
  --port; \
  outpb_d(num, port, data); \
  REST_IRQ(); \
}

//#define inpwb_d(port)
//;    GET_DIS_IRQ();
//;  al = inpb_d(port);
//;                mov     ah, al
//;                inc     edx
//;                in      al, dx
//;//;  al = inpb_d(port);
//;  REST_IRQ();
//;                xchg    al, ah

//rep_inswb_d                //;case __TMKMPC1:    GET_DIS_IRQ
//;//;                insb
//;                INSB_D
//;                inc     edx
//;//;                insb
//;                INSB_D
//;  REST_IRQ();
//;                dec     edx
//;                inc     __tmkRAMAddr[ebx*2];
//;                loop    case __TMKMPC1
/*
INSB_D          MACRO
                call    insb_1d
                ENDM

insb_1d         PROC    NEAR
                push    ebx
                insb
                movzx   ebx, __wInDelay[ebx]
i_sb_d:         dec     ebx
                jnz     i_sb_d
                pop     ebx
                ret
insb_1d         ENDP
*/

//the function always works as ifndef RAMwoCLI
#define REP_INSW_D { \
  do \
  { \
    int t; \
    GET_DIS_IRQ(); \
    *(buf++) = inpw(port); \
    /*insw*/ \
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
    t = __wInDelay[realnum]; \
    while (--t); \
  } \
  while (--len != 0); \
}

//the function always works as ifndef RAMwoCLI
#define REP_OUTSW_D { \
  do \
  { \
    int t; \
    GET_DIS_IRQ(); \
    outpw(port, *(buf++)); \
    /*outsw*/ \
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
    t = __wOutDelay[realnum]; \
    while (--t); \
  } \
  while (--len != 0); \
}

#ifdef DOS
#ifndef CPU188

void ENABLE_IRQ_AL(int num, U08 irq)
{
  U08 mask;
  if (irq < 8)
  {
    mask = 1 << irq;
    __tmkMask21[num] |= mask;
#ifdef MASKTMKS
    tmkAllMask21 |= mask;
#endif
  }
  else
  {
    mask = 1 << (irq - 8);
    __tmkMaskA1[num] |= mask;
#ifdef MASKTMKS
    tmkAllMaskA1 |= mask;
#endif
  }
}

void DISABLE_IRQ_AL(int num, U08 irq)
{
  U08 mask;
  if (irq < 8)
  {
    mask = ~(1 << irq);
    __tmkMask21[num] &= mask;
#ifdef MASKTMKS
    tmkAllMask21 &= mask;
#endif
  }
  else
  {
    mask = ~(1 << (irq - 8));
    __tmkMaskA1[num] &= mask;
#ifdef MASKTMKS
    tmkAllMaskA1 &= mask;
#endif
  }
}

#else //def CPU188

void ENABLE_IRQ_AL(int num, U08 irq)
{
  U16 mask;
  if (irq < 5)
    mask = 1 << (irq + 4);
  else
    mask = 1 << (irq - 3);
  __tmkMask21[num] |= mask;
#ifdef MASKTMKS
  tmkAllMask21 |= mask;
#endif
}

void DISABLE_IRQ_AL(int num, U08 irq)
{
  U16 mask;
  if (irq < 5)
    mask = ~(1 << (irq + 4));
  else
    mask = ~(1 << (irq - 3));
  __tmkMask21[num] &= mask;
#ifdef MASKTMKS
  tmkAllMask21 &= mask;
#endif
}

#endif //ndef CPU188
#endif //def DOS

////////////////////////////////////////////////////////////////
//void L_SetMask(unsigned adr, unsigned mask, unsigned zero_mask)
//{
//  outpw(adr, (inpw(adr) & ~mask) & (~zero_mask));
//}

//void H_SetMask(unsigned adr, unsigned mask, unsigned zero_mask)
//{
//  outpw(adr, (inpw(adr) | mask) & (~zero_mask));
//}

void FARIR retfLabel()
{
}

#if defined(DOS) || defined(QNX4)
#ifdef QNX4
#define RETURN0 return 0
#else
#define RETURN0
#endif

RETIR FARIR retfRLabel3(U16 arg1, U16 arg2, U16 arg3)
{
  __tmkNumber = __tmkSaveNumber;
  RETURN0;
  arg1;arg2;arg3;
}
RETIR FARIR retfRLabel2(U16 arg1, U16 arg2)
{
  __tmkNumber = __tmkSaveNumber;
  RETURN0;
  arg1;arg2;
}
RETIR FARIR retfRLabel1(U16 arg1)
{
  __tmkNumber = __tmkSaveNumber;
  RETURN0;
  arg1;
}
RETIR FARIR retfLabel1(U16 arg1)
{
  RETURN0;
  arg1;
}
#endif

int FARFN tmkdefdac(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 tmkDacValue)
{
  int realnum;
  unsigned type;
  unsigned port;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(TMK_BAD_FUNC);
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMKH_DacPort;
    outpw(port, tmkDacValue);
    break;
  }
  return 0;
}

int FARFN tmkgetdac(
#ifndef STATIC_TMKNUM
        int __tmkNumber,
#endif
        U16 *tmkDacValue,
        U16 *tmkDacMode
        )
{
  int realnum;
  unsigned type;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    *tmkDacValue = 0;
    *tmkDacMode = 0;
    USER_ERROR(TMK_BAD_FUNC);
    break;
  }
  return 0;
}

U16 FARFN tmktimeout(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 TimeOut)
{
  int realnum;
  unsigned type;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  if (TimeOut != GET_TIMEOUT)
  {
    switch (type)
    {
    case __TA:
#ifdef MRTA
    case __MRTA:
#endif
      if (TimeOut <= 14)
        __tmkTimeOut[realnum] = __TA_14US;
      else if (TimeOut <= 18)
        __tmkTimeOut[realnum] = __TA_18US;
      else if (TimeOut <= 26)
        __tmkTimeOut[realnum] = __TA_26US;
      else
        __tmkTimeOut[realnum] = __TA_63US;
      __bcControls[realnum] = (__bcControls[realnum] & 0xCFFF) | __tmkTimeOut[realnum];
      __rtControls[realnum] = (__rtControls[realnum] & 0xCFFF) | __tmkTimeOut[realnum];
      GET_DIS_IRQ();
      outpw(TA_MODE1(realnum), (inpw(TA_MODE1(realnum)) & 0xCFFF) | __tmkTimeOut[realnum]);
      REST_IRQ();
      break;
    }
  }
  switch (type)
  {
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    switch (__tmkTimeOut[realnum])
    {
    case __TA_63US:
      TimeOut = 63;
      break;
    case __TA_26US:
      TimeOut = 26;
      break;
    case __TA_18US:
      TimeOut = 18;
      break;
    case __TA_14US:
      TimeOut = 14;
      break;
    default:
      TimeOut = 0;
      break;
    }
    break;
  default:
    TimeOut = 0;
    break;
  }
  return TimeOut;
}

#ifdef DOS
U16 FARFN tmkswtimer(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 SwTimerCtrl)
{
  return 0;
}

U32 FARFN tmkgetswtimer(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  return 0L;
}

U32 FARFN tmkgetevtime(
#ifndef STATIC_TMKNUM
        int __tmkNumber 
#endif
        )
{
  return 0L;
}
#endif //def DOS

U16 FARFN tmktimer(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 TimerCtrl)
{
  int realnum;
  unsigned type;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    if (TimerCtrl != GET_TIMER_CTRL)
    {
      if (TimerCtrl != TIMER_RESET)
        __tmkTimerCtrl[realnum] = (TimerCtrl & TIMER_MASK) | TIMER_NOSTOP;
      GET_DIS_IRQ_SMP();
      if (TimerCtrl == TIMER_RESET)
        outpw(TA_TIMCR(realnum), 0);
      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
      REST_IRQ_SMP();
    }
    return __tmkTimerCtrl[realnum];
  }
  return 0;
}

U16 FARFN tmkgettimerl(
#ifndef STATIC_TMKNUM
        int __tmkNumber 
#endif
        )
{
  int realnum;
  unsigned type;
  U16 timer;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    timer = inpw(TA_TIMER2(realnum));
    break;
  default:
    timer = 0;
    break;
  }
  return timer;
}

U32 FARFN tmkgettimer(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  U32 timer;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    GET_DIS_IRQ();
    outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum] | 0x0800);
    timer = (U32)inpw(TA_TIMER1(realnum)) << 16;
    timer |= (U32)inpw(TA_TIMER2(realnum));
    outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    REST_IRQ();
    break;
  default:
    timer = 0L;
    break;
  }
  return timer;
}

U32 FARFN bcgetmsgtime(
#ifndef STATIC_TMKNUM
        int __tmkNumber 
#endif
        )
{
  int realnum;
  unsigned type;
  U32 time;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
    time = (U32)DrvBcPeekTA(realnum, 59) << 16;
    time |= (U32)inpw(TA_DATA(realnum));
    REST_IRQ_SMP();
    break;
  default:
    time = 0L;
    break;
  }
  return time;
}

U32 FARFN rtgetmsgtime(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  int realnum;
  unsigned type;
  U32 time;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
    outpw(TA_ADDR(realnum), ((__rtSubAddr[num] | __hm400Page[num]) << 1) | 59);
    time = (U32)inpw(TA_DATA(realnum)) << 16;
    time |= (U32)inpw(TA_DATA(realnum));
    REST_IRQ_SMP();
    break;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    outpw(TA_ADDR(realnum), ((__hm400Page[num] | __rtSubAddr[num]) << 1) | 59);
    time = (U32)inpw(TA_DATA(realnum)) << 16;
    time |= (U32)inpw(TA_DATA(realnum));
    REST_IRQ_SMP();
    break;
#endif
  default:
    time = 0L;
    break;
  }
  return time;
}

U16 FARFN tmkiodelay(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 IODelay)
{
  int realnum;
  U16 iodelay;

  CLRtmkError;
  realnum = GET_RealNum;
  iodelay = __wInDelay[realnum];
  if (IODelay != GET_IO_DELAY)
  {
    if (IODelay == 0)
      IODelay = 1;
    __wInDelay[realnum] = IODelay;
    __wOutDelay[realnum] = IODelay;
  }
  return iodelay;
}

void FARFN tmkdeferrors(void (FARIR* UserErrors)(void))
{
#ifdef STATIC_TMKNUM
  CLRtmkError;
  tmkUserErrors = UserErrors;
#endif
}

#if defined(DOS) || defined(QNX4)

void FARFN bcdefintsig(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserSigBC)(U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserSigBC[realnum] = UserSigBC;
}

void FARFN bcdefintx(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserXBC)(U16, U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserXBC[realnum] = UserXBC;
}

void FARFN bcdefintnorm(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserNormBC)(U16, U16, U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserNormBC[realnum] = UserNormBC;
}

void FARFN bcdefintexc(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserExcBC)(U16, U16, U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserExcBC[realnum] = UserExcBC;
}

void FARFN rtdefintdata(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserDataRT)(U16))
{
  tmkUserDataRT[__tmkNumber] = UserDataRT;
}

void FARFN rtdefintcmd(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserCmdRT)(U16))
{
  tmkUserCmdRT[__tmkNumber] = UserCmdRT;
}

void FARFN rtdefinterr(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserErrRT)(U16))
{
  tmkUserErrRT[__tmkNumber] = UserErrRT;
}

void FARFN mtdefintsig(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserSigMT)(U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserSigMT[realnum] = UserSigMT;
}

void FARFN mtdefintx(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        RETIR (FARIR* UserXMT)(U16, U16))
{
  int realnum;

  realnum = GET_RealNum;
  tmkUserXMT[realnum] = UserXMT;
}

#endif //def DOS

//;        PUBLIC  tmkdefevent
//;                ALIGN   4
#ifdef STATIC_TMKNUM
//;tmkdefevent     (ebx, TmkEventU32, fEventSetU32
#else
//;tmkdefevent     (ebx, U32 __tmkNumber2, TmkEventU32, fEventSetU32
#endif
//;                mov     ebx, __tmkNumber2
//;                mov     eax, TmkEvent
//;                mov     hTmkEvent[ebx*2], eax
//;                mov     eax, fEventSet
//;                mov     fTmkEventSet[ebx*2], eax
//;                ret
//;tmkdefevent     ENDP

#if NRT > 0
int FARFN rt2mrt(int hTMK)
{
//;#ifdef   STATIC_TMKNUM
//;  CLRtmkError;
//;#endif
  if ((unsigned)hTMK > __tmkMaxNumber)
  {
//;#ifdef   STATIC_TMKNUM
//;                USER_ERROR(TMK_BAD_NUMBER
//;#endif
    return -1;
  }
  else
  {
    return __amrtNumber[hTMK];
  }
}
#endif //NRT

int FARFN tmkselect(int hTMK)
{
#ifndef STATIC_TMKNUM
  int __tmkNumber; // used in CLRtmkError
#endif
//;  CLRtmkError;
  CHECK_TMK_NUMBER(hTMK);
//  CHECK_TMK_TYPE_1(tmkUserType[hTMK]);
  CHECK_TMK_TYPE_2(__tmkDrvType[hTMK]);
  __tmkNumber = hTMK;
  CLRtmkError;
  PUT_RealNum(__amrtNumber[hTMK]);
  return 0;
}

#ifdef STATIC_TMKNUM
#if NRT > 0
//__inline
int FARFN mrtselected()
{
//;  CLRtmkError;
  return GET_RealNum;
}
#endif //NRT

//__inline
int FARFN tmkselected()
{
//;  CLRtmkError;
  return __tmkNumber;
}
#endif

//__inline
int FARFN tmkgetmaxn()
{
//;  CLRtmkError;
//;                mov     eax, NTMK - 1
  return __tmkMaxNumber;
}

#if NRT > 0
//__inline
int FARFN mrtgetmaxn()
{
//;  CLRtmkError;
  return NTMK - 1;
}

int FARFN mrtgetrt0(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  CLRtmkError;
  return __mrtMinRT[GET_RealNum];
}

int FARFN mrtgetnrt(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  CLRtmkError;
  return __mrtNRT[GET_RealNum];
}
#endif //NRT

U16 FARFN tmkgetmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  CLRtmkError;
  return __tmkMode[__tmkNumber];
}

void FARFN tmksetcwbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 SetControl)
{
  int realnum;
  unsigned type;
  unsigned port;
  U08 bits;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1[realnum];
  GET_MUTEX;
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_ModePort - TMK_ModePort;
  case __TMK400:
  case __RTMK400:
    port += TMK_ModePort;
    bits = SetControl & TMK400CWBitsMask;
    if (__tmkMode[realnum] != RT_MODE)
    {
      bits |= __bcControls[realnum];
      __bcControls[realnum] = bits;
    }
    else
    {
      bits |= __rtControls[realnum];
      __rtControls[realnum] = bits;
    }
    outpb_d(realnum, port, bits);
    break;
  }
  REST_MUTEX;
}

void FARFN tmkclrcwbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 ClrControl)
{
  int realnum;
  unsigned type;
  unsigned port;
  U08 bits;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1[realnum];
  GET_MUTEX;
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_ModePort - TMK_ModePort;
  case __TMK400:
  case __RTMK400:
    port += TMK_ModePort;
    bits = ~(ClrControl & TMK400CWBitsMask);
    if (__tmkMode[realnum] != RT_MODE)
    {
      bits &= __bcControls[realnum];
      __bcControls[realnum] = bits;
    }
    else
    {
      bits &= __rtControls[realnum];
      __rtControls[realnum] = bits;
    }
    outpb_d(realnum, port, bits);
    break;
  }
  REST_MUTEX;
}

U16 FARFN tmkgetcwbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  U08 bits;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  if (__tmkMode[realnum] != RT_MODE)
    bits = __bcControls[realnum];
  else
    bits = __rtControls[realnum];
  switch (type)
  {
//  case __TMKX:
//  case __MRTX:
//    bits = 0;
//    break;
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
    bits &= TMK400CWBitsMask;
    break;
  default:
    bits = 0;
    break;
  }
  return (U16)bits;
}

void FARFN bc_def_tldw(U16 TLDW)
{
  TLDW;
}

void FARFN bc_enable_di()
{
}

void FARFN bc_disable_di()
{
}

void rtcreatlink(int num, unsigned sa, unsigned len)
{
  unsigned i;
  unsigned base, next;

  GET_DIS_IRQ_SMP();
  DrvRtPokeTA(num, AdrTab, sa, sa|0x4000);
//  DrvRtPokeTA(num, AdrTab, sa, sa);
  REST_IRQ_SMP();
  for (i = 1; i < len; ++i)
  {
    base = sa + ((i-1) << 6);
    next = base + 64; //sa + (i << 6);
    GET_DIS_IRQ_SMP();
    DrvRtPokeTA(num, base, 63, next|0x4000);
//DrvRtPokeTA(num, base, 63, next);
    REST_IRQ_SMP();
  }
  base = sa + ((i-1) << 6);
  GET_DIS_IRQ_SMP();
  DrvRtPokeTA(num, base, 63, sa|0x4000);
//DrvRtPokeTA(num, base, 63, sa);
  REST_IRQ_SMP();
}

void rtcreattab(int num, unsigned len)
{
  unsigned sa;

  outpw(TA_MSGA(num), AdrTab);

  //for MT
  GET_DIS_IRQ_SMP();
  DrvRtPokeTA(num, AdrTab, 0, 0x4000);
  DrvRtPokeTA(num, 0, 63, 0x4000);
  REST_IRQ_SMP();

  //for RT
  for (sa = 1; sa < 31; ++sa)
  {
    rtcreatlink(num, sa, len);      //rx sa
    rtcreatlink(num, sa|0x20, len); //tx sa
  }
  rtcreatlink(num, 0x1F, len); //All Mode Cmds
  rtcreatlink(num, 0x20, len); //Cmd TX VECTOR
  rtcreatlink(num, 0x3F, len); //Cmd TX BIT
}

#ifdef MRTA
void mrtcreatlink(int num, unsigned addr, unsigned sa)
{
  unsigned base;

  base = 0x0800 | (addr << 6) | sa;
  GET_DIS_IRQ_SMP();
  outpw(MRTA_ADDR2(num), 0);
  DrvRtPokeTA(num, addr, sa, base|0x4000);
  outpw(MRTA_ADDR2(num), base >> 10);
  DrvRtPokeTA(num, base, 63, base|0x4000);
  REST_IRQ_SMP();
}

void mrtcreattab(int num, unsigned addr)
{
  unsigned sa;

  //for RT
  for (sa = 1; sa < 31; ++sa)
  {
    mrtcreatlink(num, addr, sa);      //rx sa
    mrtcreatlink(num, addr, sa|0x20); //tx sa
  }
  mrtcreatlink(num, addr, 0x1F); //All Mode Cmds
  mrtcreatlink(num, addr, 0x20); //Cmd TX VECTOR
  mrtcreatlink(num, addr, 0x3F); //Cmd TX BIT
}
#endif //def MRTA

#if NRT > 0
unsigned DrvMrtaBrcRtOn(int realnum)
{
  return (__dmrtBrc[realnum] != 0L || (__dmrtRT[realnum] & (1L << 31)) != 0L) ? MRTA_RT_ON : 0;
}
#endif

int FARFN bcreset(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned cntr=0;
  unsigned irt;
  unsigned nrt;
  int err; // because of tmkError can be switched off

  CLRtmkError;
  realnum = GET_RealNum;
  __tmkMode[realnum] = BC_MODE;
  __tmkStarted[realnum] = 0;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
    outpb_d(realnum, port, 0);
    port += TMK_ModePort-TMK_ResetPort;
    cntr = BC_MODE;
    outpb_d(realnum, port, cntr);
    break;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_ResetPort;
    outpb_d(realnum, port, 0);
    port += TMKMPC_ModePort-TMKMPC_ResetPort;
    cntr = BC_MODE;
    outpb_d(realnum, port, cntr);
    break;
#ifdef MRTX
  case __MRTX:
    __tmkMode[realnum] = MRT_MODE;
    err = 0;
    if (realnum != __tmkNumber)
    {
      __tmkMode[__tmkNumber] = RT_MODE;
#ifdef STATIC_TMKNUM
      rtenable(RT_DISABLE);
#else
      rtenable(__tmkNumber, RT_DISABLE);
#endif
      __tmkMode[__tmkNumber] = 0xFFFF;
      USER_ERROR(TMK_BAD_FUNC);
#ifdef STATIC_TMKNUM
      err = tmkError;
#else
      err = tmkError[__tmkNumber];
#endif
      irt = __mrtMinRT[realnum];
      nrt = __mrtNRT[realnum];
      do
      {
        if (__tmkMode[irt++] != 0xFFFF)
        {
          __rtDisableMask[__tmkNumber] = __RT_DIS_MASK[type];
          __rtBRCMask[__tmkNumber] = __RT_BRC_MASK[type];
          __rtEnableOnAddr[__tmkNumber] = 1;
          break;
        }
      }
      while (--nrt != 0);
      if (nrt != 0)
        return err;
    }
    __hm400Page[realnum] = 0;
    __rtDisableMask[realnum] = __RT_DIS_MASK[type];
    __rtBRCMask[realnum] = __RT_BRC_MASK[type];
    __rtEnableOnAddr[realnum] = 1;
    __rtControls1[realnum] = MRTX_HBIT_MODE + MRTX_BRCST_MODE;
    __mrtCtrl0[realnum] = 0x1FF8;
    __mrtCtrl1[realnum] = 0x8000 + 0x1FF8;
    __mrtMask0[realnum] = 0x1FF8;
    __mrtMask1[realnum] = 0x1FF8;
    __rtControls[realnum] = TX_RT_DATA_INT_BLK;
    irt = __mrtMinRT[realnum];
    nrt = __mrtNRT[realnum];
    do
    {
      __tmkMode[irt] = 0xFFFF;
      __rtDisableMask[irt] = __RT_DIS_MASK[type];
      __rtBRCMask[irt] = __RT_BRC_MASK[type];
      __rtEnableOnAddr[irt] = 1;
      ++irt;
    }
    while (--nrt != 0);
    port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
    GET_DIS_IRQ();
    outpw(port, 0);
    port += TMK_ModePort-TMK_ResetPort;
    outpw(port, __mrtCtrl0[realnum] | __rtControls[realnum]);
    // | __mrtMask0[realnum]; __rtControls->m__rtControls
    outpw(port, __mrtCtrl1[realnum] | __rtControls[realnum]);
    // | __mrtMask1[realnum]; __rtControls->m__rtControls
    port += TMK_CtrlPort-TMK_ModePort;
    outpw(port, __rtControls1[realnum] & __rtBRCMask[realnum]);
    irt = __mrtMinRT[realnum];
    nrt = __mrtNRT[realnum];
    port += TMK_DataPort-TMK_CtrlPort;
    do
    {                //; simple ram write because of get_dis_irq
      port += TMK_AddrPort-TMK_DataPort;
      outpw(port, __hm400Page[irt]);
      port += TMK_DataPort-TMK_AddrPort;
      outpw(port, 0);            //; status bits (__rtControls) = 0
      ++irt;
    }
    while (--nrt != 0);
    REST_IRQ();
    return err;
#endif
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
    GET_DIS_IRQ_SMP();
    outpw(port, 0);
    REST_IRQ_SMP();
    port += TMK_ModePort-TMK_ResetPort;
    cntr = (BC_MODE >> 7) | (GENER1_BLK + GENER2_BLK);
    outpw(port, cntr);
    break;
  case __TA:
    GET_DIS_IRQ_SMP();
    outpw(TA_RESET(realnum), 0);
    outpw(TA_TIMCR(realnum), 0);
//    if (__tmkTimerCtrl[realnum] & TIMER_BITS)
//      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    cntr = (BC_MODE << 7) | (TA_TXRX_EN + TA_IRQ_EN);
    outpw(TA_MODE1(realnum), cntr | TA_FIFO_RESET);
    outpw(TA_MODE1(realnum), cntr);
    REST_IRQ_SMP();
    __bcControls1[realnum] = TA_STOP_ON_EXC | RRG2_BC_Mask_Rez_Bit;
    outpw(TA_MODE2(realnum), __bcControls1[realnum]);
    break;
#ifdef MRTA
  case __MRTA:
    __tmkMode[realnum] = MRT_MODE;
    err = 0;
    if (realnum != __tmkNumber)
    {
      __tmkMode[__tmkNumber] = RT_MODE;
#ifdef STATIC_TMKNUM
      rtenable(RT_DISABLE);
#else
      rtenable(__tmkNumber, RT_DISABLE);
#endif
      __tmkMode[__tmkNumber] = 0xFFFF;
      USER_ERROR(TMK_BAD_FUNC);
#ifdef STATIC_TMKNUM
      err = tmkError;
#else
      err = tmkError[__tmkNumber];
#endif
      irt = __mrtMinRT[realnum];
      nrt = __mrtNRT[realnum];
      do
      {
        if (__tmkMode[irt++] != 0xFFFF)
        {
          __hm400Page0[__tmkNumber] = 0;
          __hm400Page[__tmkNumber] = 0;
          __hm400Page2[__tmkNumber] = 0;
          __rtDisableMask[__tmkNumber] = __RT_DIS_MASK[type];
          __rtBRCMask[__tmkNumber] = __RT_BRC_MASK[type];
          __rtEnableOnAddr[__tmkNumber] = 1;

          break;
        }
      }
      while (--nrt != 0);
      if (nrt != 0)
        return err;
    }
    __dmrtRT[realnum] = 0L;
    __dmrtBrc[realnum] = 0L;
    __hm400Page0[realnum] = 0;
    __hm400Page[realnum] = 0;
    __hm400Page2[realnum] = 0;
    __rtDisableMask[realnum] = __RT_DIS_MASK[type];
    __rtBRCMask[realnum] = __RT_BRC_MASK[type];
    __rtEnableOnAddr[realnum] = 1;
    __rtControls1[realnum] = TA_HBIT_MODE | TA_BRCST_MODE;
    __FLAG_MODE_ON[realnum] = 0;
    __rtControls[realnum] = TA_RT_DATA_INT_BLK + TA_TXRX_EN + TA_BS_MODE_DATA_EN + TA_IRQ_EN;
    irt = __mrtMinRT[realnum];
    nrt = __mrtNRT[realnum];
    do
    {
      __tmkMode[irt] = 0xFFFF;
      __hm400Page0[irt] = 0;
      __hm400Page[irt] = 0;
      __hm400Page2[irt] = 0;
      __rtDisableMask[irt] = __RT_DIS_MASK[type];
      __rtBRCMask[irt] = __RT_BRC_MASK[type];
      __rtEnableOnAddr[irt] = 1;
      __rtControls1[irt] = 0xF800 | TA_HBIT_MODE | TA_BRCST_MODE;
      __FLAG_MODE_ON[irt] = 0;
      ++irt;
    }
    while (--nrt != 0);
    GET_DIS_IRQ_SMP();
    outpw(TA_RESET(realnum), 0);
    __mrtLastBrcTxRT[realnum] = 0;
    outpw(TA_TIMCR(realnum), 0);
//    if (__tmkTimerCtrl[num] & TIMER_BITS)
//      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[num]);
    outpw(TA_MODE1(realnum), __rtControls[realnum] | TA_FIFO_RESET);
    REST_IRQ_SMP();
    outpw(TA_MODE2(realnum), __rtControls1[realnum]); // & __rtBRCMask[num]) | __rtDisableMask[num]);
    mrtcreattab(realnum, 31);
    for (irt = 0; irt <= 31; ++irt)
    {
      outpw(MRTA_SW(realnum), irt << 11);
      __mrtA2RT[realnum][irt] = 0;
    }
    return err;
#endif
  }
  __tmkTimeOut[realnum] = 0;
  __tmkTimerCtrl[realnum] = 0;
  __bcControls[realnum] = cntr;
  __bcBus[realnum] = 0;
  return 0;
}

int FARFN rtreset(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned cntr=0;
  unsigned mask;
  unsigned mask05, mask16;
  unsigned irt;
  unsigned nrt;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  if (__tmkMode[num] != RT_MODE)
  {
    __rtDisableMask[num] = __RT_DIS_MASK[type];
    __rtBRCMask[num] = __RT_BRC_MASK[type];
  }
  __tmkMode[num] = RT_MODE;
  switch (type)
  {
  case __TMK400:
    __hm400Page[num] = 0;
    port = __tmkPortsAddr1[num] + TMK_ResetPort;
    outpb_d(num, port, 0);
    port += TMK_AddrPort-TMK_ResetPort;
    outpw_d(num, port, 0);
    port += TMK_CtrlPort-TMK_AddrPort;
    outpw_d(num, port, __rtAddress[num] | __rtDisableMask[num]);
    port += TMK_ModePort-TMK_CtrlPort;
    cntr = RT_MODE;
    outpb_d(num, port, cntr);
    __tmkTimeOut[num] = 0;
    __tmkTimerCtrl[num] = 0;
    break;
  case __RTMK400:
    __hm400Page[num] = 0;
    port = __tmkPortsAddr1[num] + TMK_ResetPort;
    outpb_d(num, port, 0);
    GET_MUTEX;
    __rtControls1[num] &= RTMK400_HBIT_MODE | RTMK400_BRCST_MODE; //| RTMK400_FLAG_MODE
    REST_MUTEX;
    port -= TMK_ResetPort;
    outpb_d(num, port, (__rtControls1[num] & __rtBRCMask[num]) | __rtAddress[num] | __rtDisableMask[num]);
    port += TMK_AddrPort;
    outpw_d(num, port, 0);
    port += TMK_ModePort-TMK_AddrPort;
    cntr = RT_MODE;
    outpb_d(num, port, cntr);
    __tmkTimeOut[num] = 0;
    __tmkTimerCtrl[num] = 0;
    break;
  case __TMKMPC:
    __hm400Page[num] = 0;
    port = __tmkPortsAddr1[num] + TMKMPC_ResetPort;
    outpb_d(num, port, 0);
    port += TMKMPC_AddrHPort-TMKMPC_ResetPort;
    outpb_d(num, port, 0);
    port += TMKMPC_ModePort-TMKMPC_AddrHPort;
    cntr = RT_MODE;
    outpb_d(num, port, cntr);
    __tmkTimeOut[num] = 0;
    __tmkTimerCtrl[num] = 0;
    break;
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    __tmkMode[realnum] = MRT_MODE;
    __tmkTimeOut[realnum] = 0;
    __tmkTimerCtrl[realnum] = 0;
    cntr = 0;
    if (num == realnum)
      break;
    mask = __rtDisableMask[num] << 3;
    mask05 = 0x1F << 3;
    if (__hm400Page[num] & (1 << 11))
    {
      mask <<= 5;
      mask05 <<= 5;
    }
    mask16 = 0;
    if (__hm400Page[num] & (1 << 12))
    {
      mask16 = ~mask16;
    }
/*
    ecx <<= (31-11);      //; 11->31
    int ecx >>= 31;           //; fill ecx with sign
    cl &= 5;             //; even->0 / odd->5
    eax <<= cl;           //; eax  new mask in place
    edx <<= cl;           //; edx  mask in place

    ecx = __hm400Page[num];
    ecx <<= (30-11);      //; 12->31, 11->30
    int ecx >>= 31;           //; fill ecx with sign (mask0/mask1)
*/
    __mrtMask1[realnum] &= ~(mask05 & mask16);
    __mrtMask1[realnum] |= mask & mask16;
    mask16 = ~mask16;
    __mrtMask0[realnum] &= ~(mask05 & mask16);
    __mrtMask0[realnum] |= mask & mask16;
/*
    edx &= ecx;
    edx = ~edx;
    __mrtMask1[realnum] &= dx;
    edx = eax;
    edx &= ecx;
    __mrtMask1[realnum] |= dx;
    ecx = ~ecx;
                 pop     edx
    edx &= ecx;
    edx = ~edx;
    __mrtMask0[realnum] &= dx;
    edx = eax;
    edx &= ecx;
    __mrtMask0[realnum] |= dx;
*/
    irt = __mrtMinRT[realnum];
    nrt = __mrtNRT[realnum] + irt - 1;
    mask = __rtBRCMask[irt];
    do
    {
      mask |= __rtBRCMask[++irt];
    }
    while (irt != nrt);
    __rtBRCMask[realnum] = mask;

    irt = __mrtMinRT[realnum];
    nrt = __mrtNRT[realnum];
    do
    {
      if (irt != num)
        if (__tmkMode[irt] != 0xFFFF)
          break;
        ++irt;
    }
    while (--nrt != 0);
    port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
    GET_DIS_IRQ();
    if (nrt == 0)
    {
      outpw(port, 0);
      __rtControls[realnum] = TX_RT_DATA_INT_BLK;  //; m__rtControls
    }
    port += TMK_ModePort-TMK_ResetPort;
    outpw(port, __mrtCtrl0[realnum] | __mrtMask0[realnum] | __rtControls[realnum]);
                                                    //; m__rtControls
    outpw(port, __mrtCtrl1[realnum] | __mrtMask1[realnum] | __rtControls[realnum]);
                                                    //; m__rtControls
    port += TMK_CtrlPort-TMK_ModePort;
    __rtControls1[realnum] &= MRTX_BRCST_MODE; //| MRTX_FLAG_MODE;
    outpw(port, __rtControls1[realnum] & __rtBRCMask[realnum]);
    __rtMode[realnum] = 0;
                //; simple ram write because of get_dis_irq
    port += TMK_AddrPort-TMK_CtrlPort;
    outpw(port, __hm400Page[num]);
    port += TMK_DataPort-TMK_AddrPort;
    outpw(port, 0);            //; status bits (__rtControls) = 0
    REST_IRQ();
//    cntr = 0; // above
    break;
#endif
  case __TMKX:
    __hm400Page[num] = 0;
    port = __tmkPortsAddr1[num] + TMK_ResetPort;
    GET_DIS_IRQ();
    outpw(port, 0);
    port += TMK_ModePort-TMK_ResetPort;
    cntr = ((RT_MODE >> 7) + TX_RT_DATA_INT_BLK + GENER1_BLK + GENER2_BLK);
    outpw(port, cntr);
    port += TMK_CtrlPort-TMK_ModePort;
    __rtControls1[num] &= 0xF800 | TMKX_BRCST_MODE; //| TMKX_FLAG_MODE;
                     //; Восстановление адреса ОУ и режимов
    outpw(port, (__rtControls1[num] & __rtBRCMask[num]) | __rtDisableMask[num]);
    REST_IRQ();
    __tmkTimeOut[num] = 0;
    __tmkTimerCtrl[num] = 0;
    break;
  case __TA:
    __hm400Page[num] = 0;
    GET_DIS_IRQ_SMP();
    outpw(TA_RESET(num), 0);
    outpw(TA_TIMCR(num), 0);
//    if (__tmkTimerCtrl[num] & TIMER_BITS)
//      outpw(TA_TIMCR(num), __tmkTimerCtrl[num]);
    cntr = (RT_MODE << 7) + TA_RT_DATA_INT_BLK + TA_IRQ_EN + TA_TXRX_EN + TA_BS_MODE_DATA_EN;
    outpw(TA_MODE1(num), cntr | TA_FIFO_RESET);
    REST_IRQ_SMP();
    __rtControls1[num] &= 0xF800 | TA_HBIT_MODE | TA_BRCST_MODE;
                     //; Восстановление адреса ОУ и режимов
    outpw(TA_MODE2(num), (__rtControls1[num])); // & __rtBRCMask[num]) | __rtDisableMask[num]);

    if (__rtDisableMask[num]) // first rtreset from any other mode
    {
      rtcreattab(num, 1);
      __FLAG_MODE_ON[num] = 0;
    }
    else
    {
      DrvFlagModeTA(num, 0);
      DrvRtWMode(num, __TA, 0);
      cntr |= TA_RTMT_START;
    }
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(num), cntr);
    REST_IRQ_SMP();
    __tmkTimeOut[num] = 0;
    __tmkTimerCtrl[num] = 0;
    break;
#ifdef MRTA
  case __MRTA:
    realnum = GET_RealNum;
    __tmkMode[realnum] = MRT_MODE;
    __tmkTimeOut[realnum] = 0;
    __tmkTimerCtrl[realnum] = 0;
    cntr = 0;
    if (num == realnum)
      break;

    cntr = TA_RT_DATA_INT_BLK + TA_TXRX_EN + TA_BS_MODE_DATA_EN + TA_IRQ_EN;
    if (__dmrtRT[realnum] == 0L)
    {
      GET_DIS_IRQ_SMP();
      outpw(TA_RESET(realnum), 0);
      __mrtLastBrcTxRT[realnum] = 0;
      outpw(TA_TIMCR(realnum), 0);
//    if (__tmkTimerCtrl[num] & TIMER_BITS)
//      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[num]);
      outpw(TA_MODE1(realnum), cntr | TA_FIFO_RESET);
      REST_IRQ_SMP();
      mrtcreattab(realnum, 31);
    }
    if (__dmrtRT[realnum] != 0L)
      cntr |= TA_RTMT_START;
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(realnum), cntr);
    REST_IRQ_SMP();
    __rtControls1[realnum] &= TA_HBIT_MODE | TA_BRCST_MODE;
                     //; Восстановление режимов
    outpw(TA_MODE2(realnum), (__rtControls1[realnum])); // & __rtBRCMask[num]) | __rtDisableMask[num]);
    outpw(MRTA_SW(realnum), 0xF800 | __rtControls1[realnum] | DrvMrtaBrcRtOn(realnum));
    __rtControls1[num] &= 0xF800 | TA_HBIT_MODE | TA_BRCST_MODE | MRTA_RT_ON;
                     //; Восстановление адреса ОУ и режимов
    if (__rtControls1[num] & MRTA_RT_ON) //!__rtDisableMask[num]
    {
      __rtControls1[num] &= ~MRTA_RT_ON;
      outpw(MRTA_SW(realnum), __rtControls1[num]);
      DrvFlagModeTA(num, 0);
      DrvRtWMode(num, __MRTA, 0);
      __rtControls1[num] |= MRTA_RT_ON;
      outpw(MRTA_SW(realnum), __rtControls1[num]);
    }
    break;
#endif
  }
  __rtControls[num] = cntr;
  __rtMode[num] = 0;
  __rtPagePC[num] = 0;
  __rtPageBus[num] = 0;
  return 0;
}

int FARFN mtreset(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned cntr=0;

  CLRtmkError;
  realnum = GET_RealNum;
  __tmkMode[realnum] = MT_MODE;
  __tmkStarted[realnum] = 0;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    __tmkMode[realnum] = 0xFFFF;
    return USER_ERROR(TMK_BAD_FUNC);
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
    GET_DIS_IRQ_SMP();
    outpw(port, 0);
    REST_IRQ_SMP();
    port += TMK_ModePort-TMK_ResetPort;
    cntr = ((BC_MODE >> 7) | (GENER1_BLK + GENER2_BLK));
           //MT_MODE SHR 7
    outpw(port, cntr);
    break;
  case __TA:
    GET_DIS_IRQ_SMP();
    outpw(TA_RESET(realnum), 0);
    outpw(TA_TIMCR(realnum), 0);
//    if (__tmkTimerCtrl[realnum] & TIMER_BITS)
//      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    cntr = (MT_MODE << 7) | (TA_TXRX_EN + TA_IRQ_EN);
    outpw(TA_MODE1(realnum), cntr | TA_FIFO_RESET);
    outpw(TA_MODE1(realnum), cntr);
    REST_IRQ_SMP();
    __bcControls1[realnum] = 0xF800; // disable RT
    outpw(TA_MODE2(realnum), __bcControls1[realnum]);
    break;
  }
  __tmkTimeOut[realnum] = 0;
  __tmkTimerCtrl[realnum] = 0;
  __bcControls[realnum] = cntr;
  return 0;
}

int FARFN mtdefmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 __mtModeBits)
{
  int realnum;
  unsigned type;
//  unsigned port;
  unsigned bits, bitst;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  bitst = __mtModeBits;
  bits = 0;
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TA:
    if (bitst & DRV_HBIT_MODE)
      bits |= TA_HBIT_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TA_BRCST_MODE;
    GET_MUTEX;
    bits |= __bcControls1[realnum] & 0xFDEF;
    __bcControls1[realnum] = bits;
    REST_MUTEX;
//    bits &= __rtBRCMask[num];
//    bits |= __rtDisableMask[num];
    outpw(TA_MODE2(realnum), bits);
    break;
  }
  return 0;
}

U16 FARFN mtgetmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned bits=0, bitst;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
#ifdef MRTA
  case __MRTA:
#endif
    USER_ERROR(BC_BAD_FUNC);
    bits = 0;
    break;
  case __TA:
    bitst = __bcControls1[realnum]; //inpw(TA_MODE2(num));
    bits = 0;
    if (bitst & TA_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & TA_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    break;
  }
  return bits;
}

int FARFN bcdefirqmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcIrqModeBits)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned bits;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
    bits = bcIrqModeBits & 0xC004;
    GET_MUTEX;
    bits |= __bcControls[realnum] & 0x0003;
    __bcControls[realnum] = bits;
    port = __tmkPortsAddr1[realnum] + TMK_ModePort;
    outpw(port, bits);
    REST_MUTEX;
    break;
  case __TA:
    bits = (((bcIrqModeBits & TMK_IRQ_OFF) ^ TMK_IRQ_OFF) >> 5);
    GET_MUTEX;
    bits |= __bcControls[realnum] & ~TA_IRQ_EN;
    __bcControls[realnum] = bits;
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(realnum), bits);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
  }
  return 0;
}

U16 FARFN bcgetirqmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned bits=0;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    USER_ERROR(BC_BAD_FUNC);
    bits = 0;
    break;
  case __TMKX:
    bits = __bcControls[realnum] & 0xC004;
    break;
  case __TA:
    bits = GENER1_BLK | GENER2_BLK;
    if (!(inpw(TA_MODE1(realnum)) & RRG1_En_IRQ))
      bits |= TMK_IRQ_OFF;
    break;
  }
  return bits;
}

int FARFN bcstart(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned code;
  unsigned base;
  unsigned basepc;
  unsigned len;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE(realnum, BC_MODE);
//;                mov     edx, ebx
//;                //;VMMcall Get_Sys_VM_Handle
//;                xchg    edx, ebx
//;                mov     hSysVM, edx
//;                mov     hCurVM[ebx*2], edx
//;//;
//;                mov     eax, 5                         //; number of milliseconds
//;                mov     edx, ebx                       //; reference data
//;                mov     esi, OFFSET32 _VTMK_BC_Event_Callback //; callback procedure
//;                //;VMMcall Set_Global_Time_Out
//;
//;//;                mov     [TimeOut], esi                  //; time-out handle
//;                jmp     bs_Ok
//;//;
  code = bcCtrlCode;
  CHECK_BC_CTRL(code);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  base = Base;
  CHECK_BC_BASE_BX(realnum, base);
  if (__tmkStarted[realnum])
  {
    switch (type)
    {
    case __TMK400:
    case __RTMK400:
      port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
      outpb_d(realnum, port, 0);
      port += TMK_ModePort-TMK_ResetPort;
      outpb_d(realnum, port, __bcControls[realnum]);
      break;
    case __TMKMPC:
      port = __tmkPortsAddr1[realnum] + TMKMPC_ResetPort;
      outpb_d(realnum, port, 0);
      port += TMKMPC_ModePort-TMKMPC_ResetPort;
      outpb_d(realnum, port, __bcControls[realnum]);
      break;
#ifdef MRTX
    case __MRTX:
#endif
    case __TMKX:
      port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
      GET_DIS_IRQ_SMP();
      outpw(port, 0);
      REST_IRQ_SMP();
      port += TMK_ModePort-TMK_ResetPort;
      outpw(port, __bcControls[realnum]);
      break;
#ifdef MRTX
    case __MRTA:
#endif
    case __TA:
      GET_DIS_IRQ_SMP();
      outpw(TA_RESET(realnum), 0);
      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
      outpw(TA_MODE1(realnum), __bcControls[realnum]);
      REST_IRQ_SMP();
      outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      break;
    }
  }
  __tmkStarted[realnum] = 1;
  __bcBaseBus[realnum] = base;
  basepc = __bcBasePC[realnum];
  if (base == basepc)
    basepc = -1;
  __bcXStart[realnum] = 0;

  __bcAW1Pos[realnum] = 0;
  __bcAW2Pos[realnum] = 0;
  switch (code)
  {
  case DATA_RT_RT_BRCST:
    __bcAW1Pos[realnum] = 2;
    break;
  case DATA_BC_RT:
    len = __bcCmdWN[realnum][base] & 0x1F;
////                mov     ecx, bcCmdWPtr[ebx*2]
////                movzx   eax, word ptr [ecx+eax*2]
////                and     eax, 1Fh
    if (len == 0)
      len = 0x20;
    __bcAW2Pos[realnum] = len + 1;
    break;
  case DATA_RT_BC:
    __bcAW1Pos[realnum] = 1;
    break;
  case DATA_RT_RT:
    len = __bcCmdWN[realnum][base] & 0x1F;
////                mov     ecx, bcCmdWPtr[ebx*2]
////                movzx   eax, word ptr [ecx+eax*2]
////                and     eax, 1Fh
    if (len == 0)
      len = 0x20;
    __bcAW1Pos[realnum] = 2;
    __bcAW2Pos[realnum] = len + 3;
    break;
  case CTRL_C_A:
    __bcAW2Pos[realnum] = 1;
    break;
  case CTRL_CD_A:
    __bcAW2Pos[realnum] = 2;
    break;
  case CTRL_C_AD:
    __bcAW1Pos[realnum] = 1;
    break;
  }

  switch (type)
  {
#ifdef MRTX
  case __MRTX:
    return USER_ERROR(BC_BAD_FUNC);
#endif
#ifdef MRTA
  case __MRTA:
    return USER_ERROR(BC_BAD_FUNC);
#endif
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_BasePort;
    outpb_d(realnum, port, base);
    port += TMK_ModePort-TMK_BasePort;
    GET_MUTEX;
    code |= __bcControls[realnum] & 0x00F0;
    __bcControls[realnum] = code;
    outpb_d(realnum, port, code);
    REST_MUTEX;
    port += TMK_StartPort-TMK_ModePort;
    outpb_d(realnum, port, 0);
    break;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_BasePort;
    outpb_d(realnum, port, base);
    port += TMKMPC_ModePort-TMKMPC_BasePort;
    GET_MUTEX;
    code |= __bcControls[realnum] & 0x00F0;
    __bcControls[realnum] = code;
    outpb_d(realnum, port, code);
    REST_MUTEX;
    port += TMKMPC_StartPort-TMKMPC_ModePort;
    outpb_d(realnum, port, 0);
    break;
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
    base <<= 1;
    base |= __bcBus[realnum];
    base <<= 5;
    if (__tmkMode[realnum] != MT_MODE)
      outpw(port, base | code);
    else
    {
      if ((__bcControls[realnum] & (MT_MODE >> 7)) != 0)
        outpw(port, base | code);
      else
      {
        port += TMK_ModePort-TMK_CtrlPort;
        GET_DIS_IRQ();
        __bcControls[realnum] |= MT_MODE >> 7;
        outpw(port, __bcControls[realnum]);
        port += TMK_CtrlPort-TMK_ModePort;
        outpw(port, base | code);
        REST_IRQ();
      }
    }
    break;
  case __TA:
    {
      unsigned ContrW;
      ContrW = 0x1D1F;

      if ((code == DATA_RT_RT) || (code == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      ContrW |= __bcBus[realnum] << 7;

      GET_DIS_IRQ_SMP();
      outpw(TA_ADDR(realnum), (base<<6) | 61);
      outpw(TA_DATA(realnum), ContrW);
      outpw(TA_DATA(realnum), 0);
      REST_IRQ_SMP();
      outpw(TA_MSGA(realnum), base & 0x03FF);

      outpw(TA_MODE2(realnum), __bcControls1[realnum] | TA_BC_START);
    }
    break;
  }
//;                mov     esi, OFFSET32 BCIntCallback
//;                mov     esi, OFFSET32 _VTMK_BC_Event_Callback
//;                mov     edx, ebx
//;                push    ebx
//;                //;VMMcall Schedule_Global_Event
//;                pop     ebx
//;                mov     hIrqEvent[ebx*2], esi
  return 0;
}

int FARFN bcstop(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMKX_StopPort;
    outpw(port, 0);
    break;
  case __TA:
    if (__tmkMode[realnum] != MT_MODE) //bcstop
    {
      outpw(TA_MODE2(realnum), __bcControls1[realnum] | TA_BC_STOP);
    }
    else //mtstop
    {
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
      outpw(TA_MODE1(realnum), __bcControls[realnum] &= ~TA_RTMT_START);
//      outpw(TA_MODE1(realnum), inpw(TA_MODE1(realnum)) & 0xFFF7);
      REST_IRQ_SMP();
      REST_MUTEX;
    }
    break;
  }
  return 0;
}

int FARFN bcstartx(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned code;
  unsigned base;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  base = Base;
  CHECK_BCMT_BASE_BX(realnum, base);
  __bcBaseBus[realnum] = base;
  __bcXStart[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
    if (__tmkStarted[realnum])
    {
      port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
      GET_DIS_IRQ_SMP();
      outpw(port, 0);
      REST_IRQ_SMP();
      port += TMK_ModePort-TMK_ResetPort;
      outpw(port, __bcControls[realnum]);
    }
    __tmkStarted[realnum] = 1;
    port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
    base <<= 6;
    if (__tmkMode[realnum] != MT_MODE)
    {
      outpw(port, base | code);
    }
    else
    {
      if ((__bcControls[realnum] & (MT_MODE >> 7)) != 0)
        outpw(port, base | code);
      else
      {
        port += TMK_ModePort-TMK_CtrlPort;
        GET_DIS_IRQ();
        __bcControls[realnum] |= MT_MODE >> 7;
        outpw(port, __bcControls[realnum]);
        port += TMK_CtrlPort-TMK_ModePort;
        outpw(port, base | code);
        REST_IRQ();
      }
    }
    break;
  case __TA:
    if (__tmkMode[realnum] != MT_MODE) //bcstartx
    {
      unsigned ContrW = 0x1D1F;
      unsigned code1;

      if (__tmkStarted[realnum])
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_RESET(realnum), 0);
        outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
        outpw(TA_MODE1(realnum), __bcControls[realnum]);
        REST_IRQ_SMP();
        outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      }
      __tmkStarted[realnum] = 1;
      if (((code&0xf) == DATA_RT_RT) || ((code&0xf) == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      if (code & CX_BUS_B)
        ContrW |= 0x0080;
      if (code & CX_CONT)
        ContrW |= 0x2000;
#if DRV_MAX_BASE < 256
      code1 = __bcLinkWN[realnum][base];
#else
      code1 = __bcLinkCCN[realnum][base];
#endif
      if (code1 & CX_SIG)
        ContrW |= 0x8000;

      GET_DIS_IRQ_SMP();
      outpw(TA_ADDR(realnum), (base<<6) | 61);
      outpw(TA_DATA(realnum), ContrW);
      outpw(TA_DATA(realnum), 0);
      REST_IRQ_SMP();
#ifdef DRV_EMULATE_FIRST_CX_SIG
//emulation is through special base 0x3ff
//also it could be a good (or bad) idea to block irq output 
//and further poll it until it occurs
      if (code & CX_SIG)
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_ADDR(realnum), (0x03FF<<6) | 61);
        port = TA_DATA(realnum);
        outpw(port, 0xA020);
        outpw(port, 0);
        outpw(port, base);
        REST_IRQ_SMP();
        base = 0x03FF;
      }
#endif //def DRV_EMULATE_FIRST_CX_SIG
      outpw(TA_MSGA(realnum), base & 0x03FF);

      outpw(TA_MODE2(realnum), __bcControls1[realnum] | TA_BC_START);
    }
    else //mtstartx
    {
//      ??? what for CX_SIG? ! make nop msg with irq! but for mt?
      unsigned oldbase = __bcBasePC[realnum];

      if (__tmkStarted[realnum])
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_RESET(realnum), 0);
        outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
        GET_MUTEX;
        outpw(TA_MODE1(realnum), __bcControls[realnum] &= ~TA_RTMT_START);
        REST_MUTEX;
        REST_IRQ_SMP();
        outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      }
      __tmkStarted[realnum] = 1;
      __mtCW[realnum] = code;
      DrvBcDefBaseTA(realnum, AdrTab);
      GET_DIS_IRQ_SMP();
      DrvBcPokeTA(realnum, 0, base|0x4000);
      REST_IRQ_SMP();

//      DrvBcDefBaseTA(realnum, base);
//      GET_DIS_IRQ_SMP();
//      DrvBcPokeTA(realnum, ADR_SIG_WORD, code);
//      REST_IRQ_SMP();
      DrvBcDefBaseTA(realnum, oldbase);
      outpw(TA_MSGA(realnum), AdrTab);
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
      outpw(TA_MODE1(realnum), __bcControls[realnum] |= TA_RTMT_START);
      REST_IRQ_SMP();
      REST_MUTEX;
    }
    break;
  }
  return 0;
}

int FARFN bcdeflink(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned code;
  unsigned base;
#if DRV_MAX_BASE < 256
  unsigned link;
#endif
  unsigned oldbase;
  unsigned code1;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  base = Base;
  CHECK_BCMT_BASE_BX(realnum, base);
#if DRV_MAX_BASE < 256
  link = (base << 6) | code;
  code1 = __bcLinkWN[realnum][__bcBasePC[realnum]];
  __bcLinkWN[realnum][__bcBasePC[realnum]] = link;
////                movzx   edx, __bcBasePC[ebx]
////                mov     edi, bcLinkWPtr[ebx*2]
////                mov     [edi+edx*2], ax
#else
  __bcLinkBaseN[realnum][__bcBasePC[realnum]] = base;
  code1 = __bcLinkCCN[realnum][__bcBasePC[realnum]];
  __bcLinkCCN[realnum][__bcBasePC[realnum]] = code;
#endif
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
#if DRV_MAX_BASE < 256
    DrvBcPoke(realnum, type, 0x3F, link);
#else
    DrvBcPoke(realnum, type, 0x3F, (base << 6) | code);
#endif
    break;
  case __TA:
    oldbase = __bcBasePC[realnum];
    if (__tmkMode[realnum] != MT_MODE) //bcdeflink
    {
      unsigned ContrW = 0x1D1F;

      if (((code&0xf) == DATA_RT_RT) || ((code&0xf) == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      if (code & CX_BUS_B)
        ContrW |= 0x0080;
      if (code & CX_CONT)
        ContrW |= 0x2000;

      GET_DIS_IRQ_SMP();

      if ((code1 ^ code) & CX_SIG) //code1CX_SIG != codeCX_SIG
      {
        if (code & CX_SIG)
        {
          DrvBcPokeTA(realnum, 61, DrvBcPeekTA(realnum, 61) | 0x8000);
        }
        else
        {
          DrvBcPokeTA(realnum, 61, DrvBcPeekTA(realnum, 61) & ~0x8000);
        }
      }
//      DrvBcPokeTA(realnum, 61, ((DrvBcPeekTA(realnum, 61) & 0x7FFF) | (code & 0x8000)));
      DrvBcPokeTA(realnum, 63, base);

      DrvBcDefBaseTA(realnum, base);
#if DRV_MAX_BASE < 256
      code1 = __bcLinkWN[realnum][base];
#else
      code1 = __bcLinkCCN[realnum][base];
#endif
      if (code1 & CX_SIG)
        ContrW |= 0x8000;
      outpw(TA_ADDR(realnum), (base<<6) | 61);
      outpw(TA_DATA(realnum), ContrW);
//      DrvBcPokeTA(realnum, 61, ((DrvBcPeekTA(realnum, 61) & 0x8000) | ContrW));
      outpw(TA_DATA(realnum), 0);

      REST_IRQ_SMP();
      DrvBcDefBaseTA(realnum, oldbase);
    }
    else //mtdeflink
    { //!!!todo: 0x4000 only if CX_STOP | CX_INT
      GET_DIS_IRQ_SMP();
      DrvBcPokeTA(realnum, 63, base|0x4000);
      REST_IRQ_SMP();
    }
    break;
  }
  return 0;
}

U32 FARFN bcgetlink(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
#if DRV_MAX_BASE < 256
  unsigned link;
#endif
  unsigned base;


  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
#if DRV_MAX_BASE < 256
  link = __bcLinkWN[realnum][__bcBasePC[realnum]];
////                movzx   eax, __bcBasePC[ebx]
////                mov     edi, bcLinkWPtr[ebx*2]
////                movzx   eax, word ptr [edi+eax*2]
  base = (link >> 6) & __bcMaxBase[realnum];
  link &= 0x803F;
  return ((U32)link << 16) | (U32)base;
#else
  base = __bcBasePC[realnum];
  return ((U32)__bcLinkCCN[realnum][base] << 16) |
          (U32)__bcLinkBaseN[realnum][base];
#endif
}

U32 FARFN bcgetstate(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned stat=0;
  unsigned base=0;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMK_ModePort;
    stat = inpw(port);
    base = (stat >> 6) & __bcMaxBase[realnum];
    stat &= 0x003F;
    break;
  case __TA:
//!!! mtgetstate?
    base = (inpw(TA_BASE(realnum))) & 0x3FF;
    stat = 0;
    break;
  }
  return ((U32)stat << 16) | (U32)base;
}

U16 FARFN bcgetmaxbase(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
#if DRV_MAX_BASE > 255
  if (__tmkMode[realnum] == BC_MODE)
    return __bcMaxBase[realnum];
  else
    return __mtMaxBase[realnum];
#else
  return __bcMaxBase[realnum];
#endif
}

U16 FARFN bcgetbase(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __bcBasePC[realnum];
}

/*
//__inline
__INLINE
void DrvBcDefBaseTA(int realnum, unsigned base)
{
  __bcBasePC[realnum] = base;
  return;
}
*/

//__inline
//__INLINE
void DrvBcDefBase(int realnum, unsigned type, unsigned base)
{
  unsigned port;

  __bcBasePC[realnum] = base;
  switch (type)
  {
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    return;
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    outpw_d(realnum, port, base);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_AddrHPort;
    outpb_d(realnum, port, base);
    return;
  }
  return;
}

int FARFN bcdefbase(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 BasePC)
{
  int realnum;
  unsigned type;
  unsigned base;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  base = BasePC;
  CHECK_BCMT_BASE_BX(realnum, base);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  DrvBcDefBase(realnum, type, base);
  return 0;
}

void DrvBcPoke(int realnum, unsigned type, unsigned pos, unsigned data)
// write to pos in current base
{
  unsigned port;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    outpw_d(realnum, port, data);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataHPort-TMKMPC_AddrLPort;
    outpwb_d(realnum, port, data);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    pos |= __bcBasePC[realnum] << 6;
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    GET_DIS_IRQ();
    outpw(port, data);
    ++__tmkRAMAddr[realnum];
    REST_IRQ();
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
//  case __TA:
//    pos |= __bcBasePC[realnum] << 6;
//    outpw(TA_ADDR(realnum), pos);
//    outpw(TA_DATA(realnum), data);
//    return;
  }
  return;
}

//__INLINE
//void DrvBcPokeTA(int realnum, unsigned pos, unsigned data)
//{
//  DrvBcPoke(realnum, TA, pos, data);
//}

/*
__INLINE
void DrvBcPokeTA(int realnum, unsigned pos, unsigned data)
{
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  outpw(TA_DATA(realnum), data);
}
*/

unsigned DrvBcPeek(int realnum, unsigned type, unsigned pos)
// read from pos in current base
{
  unsigned save_rama, save_ramiw;
  unsigned port;
  unsigned data=0;

  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = inpw_d(realnum, port);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    break; //    return data;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    data = inpw_d(realnum, port);
//    data = inpwb_d(port);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    break; //    return data;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    pos |= __bcBasePC[realnum] << 6;
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    GET_DIS_IRQ();
    data = inpw(port);
    ++__tmkRAMAddr[realnum];
    REST_IRQ();
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    break; //    return data;
//  case __TA:
//    pos |= __bcBasePC[realnum] << 6;
//    outpw(TA_ADDR(realnum), pos);
//    data = inpw(TA_DATA(realnum));
//    break;
  }
  return data;
}

//__inline
//__INLINE
//unsigned DrvBcPeekTA(int realnum, unsigned pos)
//{
//  return DrvBcPeek(realnum, TA, pos);
//}

/*
__INLINE
unsigned DrvBcPeekTA(int realnum, unsigned pos)
{
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  return inpw(TA_DATA(realnum));
}
*/

U32 FARFN bcgetansw(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned aw1pos, aw2pos;
  unsigned aw1, aw2;
  unsigned code;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  code = bcCtrlCode;
  CHECK_BC_CTRL(code);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  aw2pos = 0;
  switch (code)
  {
  case DATA_BC_RT:
    aw1pos = __bcCmdWN[realnum][__bcBasePC[realnum]] & 0x1F;
////                movzx   eax, __bcBasePC[ebx]
////                mov     esi, bcCmdWPtr[ebx*2]
////                movzx   eax, word ptr [esi+eax*2]
////                and     eax, 1Fh
    if (aw1pos == 0)
      aw1pos = 0x20;
    ++aw1pos;
    break;
  case DATA_RT_BC:
  case CTRL_C_A:
  case CTRL_C_AD:
    aw1pos = 1;
    break;
  case DATA_RT_RT:
    aw2pos = __bcCmdWN[realnum][__bcBasePC[realnum]] & 0x1F;
////                movzx   eax, __bcBasePC[ebx]
////                mov     esi, bcCmdWPtr[ebx*2]
////                movzx   edx, word ptr [esi+eax*2]
////                and     edx, 1Fh
    if (aw2pos == 0)
      aw2pos = 0x20;
    aw2pos += 3;
  case DATA_RT_RT_BRCST:
  case CTRL_CD_A:
    aw1pos = 2;
    break;
  default:
    aw1pos = 0;
    break;
  }
  aw2 = (aw2pos == 0) ? 0xFFFF : DrvBcPeek(realnum, type, aw2pos);
  aw1 = (aw1pos == 0) ? 0xFFFF : DrvBcPeek(realnum, type, aw1pos);
  return ((U32)aw2 << 16) | (U32)aw1;
}


U16 FARFN mtgetsw(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned RetData;
  unsigned SW;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMKX:
    return (U16)DrvBcPeek(realnum, type, 0x3E);
  case __TA:
    GET_DIS_IRQ_SMP();
    SW = DrvBcPeekTA(realnum, 58);
    REST_IRQ_SMP();
    RetData = SW & 0x7;
    RetData |= (SW & 0x3C0) << 4;
    if (SW & 0x8)
      RetData |= 0x8000;
    if (SW & 0x30)
      RetData |= 0x8;
// ???
    if (SW & 0x37)
      RetData |= 0x4000;
    return (U16)RetData;
  }
  return 0xFFFF;
}

void FARFN bcputw(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcPos, U16 bcData)
{
  int realnum;
  unsigned type;
  unsigned pos;
  unsigned data;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_LN(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  data = bcData;
  if (pos == 0)
  {
    __bcCmdWN[realnum][__bcBasePC[realnum]] = data;
////                movzx   eax, __bcBasePC[ebx]
////                mov     edi, bcCmdWPtr[ebx*2]
////                mov     [edi+eax*2], dx
  }
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  DrvBcPoke(realnum, type, pos, data);
}

U16 FARFN bcgetw(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcPos)
{
  int realnum;
  unsigned type;
  unsigned pos;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  return (U16)DrvBcPeek(realnum, type, pos);
}

void FARFN bcputblk(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcPos, VOID FARDT *pcBufAddr, U16 bcLen)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_LN(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  buf = (U16 FARDT*)pcBufAddr;
  len = bcLen;
  CHECK_BC_LEN(len);
  if (len == 0)
    return;
  if (pos == 0)
  {
    __bcCmdWN[realnum][__bcBasePC[realnum]] = *(U16 FARDT*)buf;
////                movzx   eax, __bcBasePC[ebx]
////                mov     edi, bcCmdWPtr[ebx*2]
////                movzx   edx, word ptr [esi]
////                mov     [edi+eax*2], dx
  }
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    REP_OUTSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    pos |= __bcBasePC[realnum] << 6;
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

void FARFN bcgetblk(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 bcPos, VOID FARDT *pcBufAddr, U16 bcLen)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_LN(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = bcLen;
  CHECK_BC_LEN(len);
  if (len == 0)
    return;
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[realnum] + TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    REP_INSW_D;
//    REP_INSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    pos |= __bcBasePC[realnum] << 6;
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

int FARFN bcdefbus(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Bus)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned bus;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  bus = Bus;
  switch (type)
  {
  case __RTMK400:
    CHECK_BC_BUS(bus);
    port = __tmkPortsAddr1[realnum] + TMK_ModePort;
    __bcBus[realnum] = bus;
    bus <<= 4;
    GET_MUTEX;
    bus |= __bcControls[realnum] & 0x00EF;
    __bcControls[realnum] = bus;
    outpb_d(realnum, port, bus);
    REST_MUTEX;
    break;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    CHECK_BC_BUS(bus)
    __bcBus[realnum] = bus;
    break;
  case __TMK400:
  case __TMKMPC:
    if (bus == 0)
      break;
    return USER_ERROR(BC_BAD_BUS);
  }
  return 0;
}

U16 FARFN bcgetbus(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __bcBus[realnum];
}

void DrvRtDefSubAddr(int num, unsigned type, unsigned sa_shl5)
{
  unsigned port;

  __rtSubAddr[num] = sa_shl5;
  switch (type)
  {
  case __TMKX:
#ifdef MRTX
  case __MRTX:
#endif
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    return;
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[num] + TMK_AddrPort;
    sa_shl5 |= __hm400Page[num];
    sa_shl5 >>= 6;
    outpw_d(num, port, sa_shl5);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[num] + TMKMPC_AddrHPort;
//;                or      ax, __hm400Page[ebx];
    sa_shl5 >>= 6;
    outpb_d(num, port, sa_shl5);
    return;
  }
  return;
}

//__inline
__INLINE
unsigned DrvRtGetBaseTA(int num)
{
  return __rtSubAddr[num] >> 5;
}

#if NRT > 0
void FARFN mrtdefbrcsubaddr0(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned sa;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
#ifdef MRTX
  case __MRTX:
    sa = 0x1F << 5;
    break;
#endif
#ifdef MRTA
// check with DrvFlagModeTA!!!
//  case __MRTA:
//    sa = mrtBrcSubAddr0[realnum]; //& 0xFFE0
//    break;
#endif
  default:
    return;
  }
  DrvRtDefSubAddr(realnum, type, sa);
  if (realnum != __tmkNumber)
  {
    DrvRtDefSubAddr(__tmkNumber, type, sa);
                //; call    DrvRtWMode
  }
  return;
}
#endif //NRT

void FARFN rtdefsubaddr(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Dir, U16 SubAddr)
{
  int num;
  unsigned type;
  unsigned sa;

  CLRtmkError;
  num = __tmkNumber;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  sa <<= 5;
  CHECK_RT_DIR(Dir);
  sa |= Dir;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  DrvRtDefSubAddr(num, type, sa);
  DrvRtWMode(num, type, sa);
  return;
}

U16 FARFN rtgetsubaddr(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned sa;

  CLRtmkError;
  num = __tmkNumber;
  sa = __rtSubAddr[num];
  return ((sa >> 5) & 0x001F) | (sa & 0x0400);
}

void FARFN rtgetblk(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  CHECK_RT_LEN(len);
  if (len == 0)
    return;
  pos += __rtSubAddr[num];
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    REP_INSW_D;
//    REP_INSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    REST_IRQ_SMP();
#endif
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

#ifdef MRTA
void FARFN __rtgetblkmrta(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  if (len == 0)
    return;
  pos += __rtSubAddr[num] << 1;
  pos |= __hm400Page[num] << 1;
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
//  case __MRTA:
  GET_DIS_IRQ_SMP();
  outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
  REST_IRQ_SMP();
//  case __TA:
  port += TMK_AddrPort;
  GET_DIS_IRQ_SMP();
  __tmkRAMAddr[realnum] = pos;
  outpw(port, pos);
  REST_IRQ_SMP();
  port += TMK_DataPort-TMK_AddrPort;
  REP_INSW;
  if (save_ramiw)
  {
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
  }
  else
    __tmkRAMInWork[realnum] = 0;
 return;
}
#endif //def MRTA

void FARFN rtputblk(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  CHECK_RT_LEN(len);
  if (len == 0)
    return;
  pos += __rtSubAddr[num];
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    REP_OUTSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    REST_IRQ_SMP();
#endif
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

#ifdef MRTA
void FARFN __rtputblkmrta(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned port;
  unsigned pos;
  unsigned len;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  if (len == 0)
    return;
  pos += __rtSubAddr[num] << 1;
  pos |= __hm400Page[num] << 1;
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
//  case __MRTA:
  GET_DIS_IRQ_SMP();
  outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
  REST_IRQ_SMP();
//  case __TA:
  port += TMK_AddrPort;
  GET_DIS_IRQ_SMP();
  __tmkRAMAddr[realnum] = pos;
  outpw(port, pos);
  REST_IRQ_SMP();
  port += TMK_DataPort-TMK_AddrPort;
  REP_OUTSW;
  if (save_ramiw)
  {
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
  }
  else
    __tmkRAMInWork[realnum] = 0;
  return;
}
#endif //def MRTA

U16 FARFN rtgetw(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos)
{
  int num;
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned data=0;
  unsigned save_rama, save_ramiw;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  pos += __rtSubAddr[num];
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = inpw_d(realnum, port);
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return data;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataLPort-TMKMPC_AddrLPort;
    data = inpw_d(realnum, port);
//    data = inpwb_d(port);
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return data;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    REST_IRQ_SMP();
#endif
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = inpw(port);
    REST_IRQ_SMP();
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return data;
  }
  __tmkRAMInWork[realnum] = 0;
  return data;
}

void FARFN rtputw(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPos, U16 pcData)
{
  int num;
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned data;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  pos += __rtSubAddr[num];
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = pcData;
    outpw_d(realnum, port, data);
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_DataHPort-TMKMPC_AddrLPort;
    data = pcData;
    outpwb_d(realnum, port, data);
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    REST_IRQ_SMP();
#endif
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = pcData;
    outpw(port, data);
    REST_IRQ_SMP();
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

void FARFN rtsetanswbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 SetControl)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum] + TMK_ModePort;
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_ModePort-TMK_ModePort;
  case __TMK400:
  case __RTMK400:
    bits = SetControl & RTAnswBitsMask;
    GET_MUTEX;
    bits |= __rtControls[realnum];
    __rtControls[realnum] = bits;
    outpb_d(realnum, port, bits);
    break;
#ifdef MRTX
  case __MRTX:
    CONVERT_TMKX_SW_BITS(bits, SetControl);
//;                and     ecx, RTAnswBitsMask
    bits <<= 3;
    GET_MUTEX;
    bits |= __rtControls[num];    //; __rtControls
    __rtControls[num] = bits;
    GET_DIS_IRQ();
                //; simple ram write because of get_dis_irq
    port += TMK_AddrPort-TMK_ModePort;
    outpw(port, __hm400Page[num]);
    port += TMK_DataPort-TMK_AddrPort;
    outpw(port, bits);
    REST_IRQ();
    break;
#endif
  case __TMKX:
    CONVERT_TMKX_SW_BITS(bits, SetControl);
//;                and     ecx, RTAnswBitsMask
    bits <<= 3;
    GET_MUTEX;
    bits |= __rtControls[realnum];
    __rtControls[realnum] = bits;
    outpw(port, bits);
    break;
  case __TA:
    CONVERT_TA_SW_BITS(bits, SetControl);
    GET_MUTEX;
    bits |= __rtControls1[realnum];
    __rtControls1[realnum] = bits;
    outpw(TA_MODE2(realnum), bits);
    break;
//    H_SetMask(TA_MODE2(realnum), bits, 0);
//    return;
#ifdef MRTA
  case __MRTA:
    CONVERT_TA_SW_BITS(bits, SetControl);
    GET_MUTEX;
    bits |= __rtControls1[num];
    __rtControls1[num] = bits;
    outpw(MRTA_SW(realnum), bits);
    break;
#endif
  }
  REST_MUTEX;
  return;
}

void FARFN rtclranswbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 ClrControl)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum] + TMK_ModePort;
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_ModePort-TMK_ModePort;
  case __TMK400:
  case __RTMK400:
    bits = ~(ClrControl & RTAnswBitsMask);
    GET_MUTEX;
    bits &= __rtControls[realnum];
    __rtControls[realnum] = bits;
    outpb_d(realnum, port, bits);
    break;
#ifdef MRTX
  case __MRTX:
    CONVERT_TMKX_SW_BITS(bits, ClrControl);
//;                and     ecx, RTAnswBitsMask
    bits = ~(bits << 3);
    GET_MUTEX;
    bits &= __rtControls[num];    //; __rtControls
    __rtControls[num] = bits;
    GET_DIS_IRQ();
                //; simple ram write because of get_dis_irq
    port += TMK_AddrPort-TMK_ModePort;
    outpw(port, __hm400Page[num]);
    port += TMK_DataPort-TMK_AddrPort;
    outpw(port, bits);
    REST_IRQ();
    break;
#endif
  case __TMKX:
    CONVERT_TMKX_SW_BITS(bits, ClrControl);
//;                and     ecx, RTAnswBitsMask
    bits = ~(bits << 3);
    GET_MUTEX;
    bits &= __rtControls[realnum];
    __rtControls[realnum] = bits;
    outpw(port, bits);
    break;
  case __TA:
    CONVERT_TA_SW_BITS(bits, ClrControl);
    bits = ~bits;
    GET_MUTEX;
    bits &= __rtControls1[realnum];
    __rtControls1[realnum] = bits;
    outpw(TA_MODE2(realnum), bits);
    break;
//    L_SetMask(TA_MODE2(realnum), bits, 0);
//    return;
#ifdef MRTA
  case __MRTA:
    CONVERT_TA_SW_BITS(bits, ClrControl);
    bits = ~bits;
    GET_MUTEX;
    bits &= __rtControls1[num];
    __rtControls1[num] = bits;
    outpw(MRTA_SW(realnum), bits);
    break;
#endif
  }
  REST_MUTEX;
  return;
}

U16 FARFN rtgetanswbits(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned bits=0, bitst;

  CLRtmkError;
  num = __tmkNumber; //???
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
    bits = __rtControls[num];
    break;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    bitst = __rtControls[num] >> 3;
    CONVERT_TMKX_SW_BITS(bits, bitst);
    break;
  case __TA:
    bitst = inpw(TA_MODE2(num));
    bits = 0;
    if (bitst & SREQ_MASK)
      bits |= SREQ;
    if (bitst & BUSY_MASK)
      bits |= BUSY;
    if (bitst & SSFL_MASK)
      bits |= SSFL;
    if (bitst & RTFL_MASK)
      bits |= RTFL;
    if (bitst & DNBA_MASK)
      bits |= DNBA;
    break;
#ifdef MRTA
  case __MRTA:
//!!! bitst = __rtControls1[num];
    realnum = GET_RealNum;
    GET_DIS_IRQ();
    outpw(TA_LCW(realnum), __rtControls1[num]);
    bitst = inpw(MRTA_SW(realnum));
    REST_IRQ();
    bits = 0;
    if (bitst & SREQ_MASK)
      bits |= SREQ;
    if (bitst & BUSY_MASK)
      bits |= BUSY;
    if (bitst & SSFL_MASK)
      bits |= SSFL;
    if (bitst & RTFL_MASK)
      bits |= RTFL;
    if (bitst & DNBA_MASK)
      bits |= DNBA;
    break;
#endif
  }
  bits &= RTAnswBitsMask;
  return bits;
}

int FARFN rtdefirqmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtIrqModeBits)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
    return USER_ERROR(RT_BAD_FUNC);
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    bits = rtIrqModeBits & 0x2000;
    GET_MUTEX;
    bits |= __rtControls[realnum] & 0xDFFF; //; m__rtControls
    __rtControls[realnum] = bits;
    bits |= __mrtCtrl0[realnum] | __mrtMask0[realnum];
    port = __tmkPortsAddr1[realnum] + TMK_ModePort;
    outpw(port, bits);
    REST_MUTEX;
    break;
#endif
  case __TMKX:
    bits = rtIrqModeBits & 0xE004;
    GET_MUTEX;
    bits |= __rtControls[num] & 0x1FFB;
    __rtControls[num] = bits;
    port = __tmkPortsAddr1[num] + TMK_ModePort;
    outpw(port, bits);
    REST_MUTEX;
    break;
  case __TA:
    bits = 0;
    if (!(rtIrqModeBits & TMK_IRQ_OFF))
      bits |= TA_IRQ_EN;
    if (rtIrqModeBits & RT_DATA_BL)
      bits |= TA_RT_DATA_INT_BLK;
    GET_MUTEX;
    bits |= __rtControls[num] & ~(TA_IRQ_EN | TA_RT_DATA_INT_BLK);
    __rtControls[num] = bits;
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(num), bits);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
#ifdef MRTA
  case __MRTA: //!!!it's possible to join with TA if use num as realnum
    realnum = GET_RealNum;
    bits = 0;
    if (!(rtIrqModeBits & TMK_IRQ_OFF))
      bits |= TA_IRQ_EN;
    if (rtIrqModeBits & RT_DATA_BL)
      bits |= TA_RT_DATA_INT_BLK;
    GET_MUTEX;
    bits |= __rtControls[realnum] & ~(TA_IRQ_EN | TA_RT_DATA_INT_BLK);
    __rtControls[realnum] = bits;
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(realnum), bits);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
#endif
  }
  return 0;
}

U16 FARFN rtgetirqmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned bits=0;
  unsigned bitst;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
  case __RTMK400:
    USER_ERROR(RT_BAD_FUNC);
    return 0;
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    bits = __rtControls[realnum] & 0x2000;    //; m__rtControls
    break;
#endif
  case __TMKX:
    bits = __rtControls[num] & 0xE004;
    break;
  case __TA:
    bitst = inpw(TA_MODE1(num));
    bits = RT_GENER1_BL|RT_GENER2_BL;
    if (!(bitst & TA_IRQ_EN))
      bits |= TMK_IRQ_OFF;
    if (bitst & TA_RT_DATA_INT_BLK)
      bits |= RT_DATA_BL;
    break;
#ifdef MRTA
  case __MRTA: //!!!it's possible to join with TA if use num as realnum
    realnum = GET_RealNum;
    bitst = inpw(TA_MODE1(realnum));
    bits = RT_GENER1_BL|RT_GENER2_BL;
    if (!(bitst & TA_IRQ_EN))
      bits |= TMK_IRQ_OFF;
    if (bitst & TA_RT_DATA_INT_BLK)
      bits |= RT_DATA_BL;
    break;
#endif
  }
  return bits;
}

void DrvFlagModeTA(int num, int m)
{
#ifdef MRTA
  unsigned type;
#endif
  int realnum;
  unsigned tablea;
  unsigned sa;
  unsigned i;

#ifdef MRTA
  type = __tmkDrvType[num];
  if (type == __MRTA)
  {
    realnum = GET_RealNumOf(num);
    tablea = __rtAddress[num];
  }
  else
#endif
  {
    realnum = num;
    tablea = AdrTab;
  }
  if (__FLAG_MODE_ON[num] != m)
  {
    if (m)
    {
      if (__rtMode[num] & 0x0800)
      {
  // disable rtlock if switch flag mode on
        sa = (__rtMode[num] >> 5) & 0x3F;
        GET_DIS_IRQ_SMP();
#ifdef MRTA
        if (type == __MRTA)
          outpw(MRTA_ADDR2(realnum), 0);
#endif
        DrvRtPokeTA(realnum, tablea, sa, (DrvRtPeekTA(realnum, tablea, sa) & 0x7FFF));
        REST_IRQ_SMP();
      }
#ifdef STATIC_TMKNUM
      rtputflags(__BC_RT_FLAG[num], RT_RECEIVE, 1, 30);
      rtputflags(__RT_BC_FLAG[num], RT_TRANSMIT, 1, 30);
#else
      rtputflags(num, __BC_RT_FLAG[num], RT_RECEIVE, 1, 30);
      rtputflags(num, __RT_BC_FLAG[num], RT_TRANSMIT, 1, 30);
#endif
    }
    else
    {
#ifdef STATIC_TMKNUM
      rtgetflags(__BC_RT_FLAG[num], RT_RECEIVE, 1, 30);
      rtgetflags(__RT_BC_FLAG[num], RT_TRANSMIT, 1, 30);
#else
      rtgetflags(num, __BC_RT_FLAG[num], RT_RECEIVE, 1, 30);
      rtgetflags(num, __RT_BC_FLAG[num], RT_TRANSMIT, 1, 30);
#endif

#ifdef MRTA
      if (type == __MRTA)
      {
        GET_DIS_IRQ_SMP();
        outpw(MRTA_ADDR2(realnum), 0);
        REST_IRQ_SMP();
      }
#endif
      for (i = 1; i < 31; ++i)
      {
        sa = i;
        GET_DIS_IRQ_SMP();
        DrvRtPokeTA(realnum, tablea, sa, (DrvRtPeekTA(realnum, tablea, sa) & 0x7FFF));
        sa |= 0x0020;
        DrvRtPokeTA(realnum, tablea, sa, (DrvRtPeekTA(realnum, tablea, sa) & 0x7FFF));
        REST_IRQ_SMP();
      }

      if (__rtMode[num] & 0x0800)
      {
  // enable rtlock if switch flag mode off
        sa = (__rtMode[num] >> 5) & 0x3F;
        GET_DIS_IRQ_SMP();
//#ifdef MRTA
//        if (type == __MRTA)
//          outpw(MRTA_ADDR2(realnum), 0);
//#endif
        DrvRtPokeTA(realnum, tablea, sa, (DrvRtPeekTA(realnum, tablea, sa) | 0x8000));
        REST_IRQ_SMP();
      }
    }

#ifdef MRTA
    if (type == __MRTA)
    {
      GET_DIS_IRQ_SMP();
      outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
      REST_IRQ_SMP();
    }
#endif
    for (i = 1; i < 31; ++i)
    {
#ifdef MRTA
      if (type == __MRTA)
        sa = (__hm400Page[num] >> 5) | i;
      else
#endif
        sa = i;
      if (m)
      {
        GET_DIS_IRQ_SMP();
        DrvRtPokeTA(realnum, sa, 63, DrvRtPeekTA(realnum, sa, 63) | 0x8000);
        sa |= 0x0020;
        DrvRtPokeTA(realnum, sa, 63, DrvRtPeekTA(realnum, sa, 63) | 0x8000);
        REST_IRQ_SMP();
      }
      else
      {
        GET_DIS_IRQ_SMP();
        DrvRtPokeTA(realnum, sa, 63, DrvRtPeekTA(realnum, sa, 63) & 0x7FFF);
        sa |= 0x0020;
        DrvRtPokeTA(realnum, sa, 63, DrvRtPeekTA(realnum, sa, 63) & 0x7FFF);
        REST_IRQ_SMP();
      }
    }

    __FLAG_MODE_ON[num] = m;
  }
}

int FARFN rtdefmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 __rtModeBits)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits, bitst;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  bitst = __rtModeBits;
  bits = 0;
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
    return USER_ERROR(RT_BAD_FUNC);
  case __RTMK400:
    if (bitst & DRV_HBIT_MODE)
      bits |= RTMK400_HBIT_MODE;
    if (bitst & DRV_FLAG_MODE)
      bits |= RTMK400_FLAG_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= RTMK400_BRCST_MODE;
    port = __tmkPortsAddr1[num];
    __rtControls1[num] = bits;
    bits &= __rtBRCMask[num];
    bits |= __rtAddress[num];
    bits |= __rtDisableMask[num];
    outpb_d(num, port, bits);
    break;
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    if (bitst & DRV_FLAG_MODE)
      bits |= MRTX_FLAG_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= MRTX_BRCST_MODE;
    GET_MUTEX;
    bits |= __rtControls1[realnum] & 0x007F;
    __rtControls1[realnum] = bits;
    REST_MUTEX;
    bits &= __rtBRCMask[realnum];
    port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
//;   num = __tmkNumber;
    GET_DIS_IRQ_SMP();
    outpw(port, bits);
    REST_IRQ_SMP();
    break;
#endif
  case __TMKX:
    if (bitst & DRV_FLAG_MODE)
      bits |= TMKX_FLAG_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TMKX_BRCST_MODE;
    GET_MUTEX;
    bits |= __rtControls1[num] & 0xF87F;
    __rtControls1[num] = bits;
    REST_MUTEX;
    bits &= __rtBRCMask[num];
    bits |= __rtDisableMask[num];
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    GET_DIS_IRQ_SMP();
    outpw(port, bits);
    REST_IRQ_SMP();
    break;
  case __TA:
    if (bitst & DRV_HBIT_MODE)
      bits |= TA_HBIT_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TA_BRCST_MODE;
    GET_MUTEX;
    bits |= __rtControls1[num] & 0xFDEF;
    __rtControls1[num] = bits;
    REST_MUTEX;
//    bits &= __rtBRCMask[num];
//    bits |= __rtDisableMask[num];
    outpw(TA_MODE2(num), bits);

    if (bitst & DRV_FLAG_MODE)
      DrvFlagModeTA(num, 1);
    else
      DrvFlagModeTA(num, 0);
    break;
#ifdef MRTA
  case __MRTA:
    realnum = GET_RealNum;
    if (bitst & DRV_HBIT_MODE)
      bits |= TA_HBIT_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TA_BRCST_MODE;
    GET_MUTEX;
    bits |= __rtControls1[realnum] & 0xFDEF;
    __rtControls1[realnum] = bits;
    REST_MUTEX;
    if (__hm400Page0[num] != 0)
    {
      outpw(TA_MODE2(realnum), bits);
      if (__rtControls1[realnum] & TA_BRCST_MODE)
        __dmrtBrc[realnum] |= 1L << __rtAddress[num];
      outpw(MRTA_SW(realnum), 0xF800 | __rtControls1[realnum] | DrvMrtaBrcRtOn(realnum));

      if (bitst & DRV_FLAG_MODE)
        DrvFlagModeTA(num, 1);
      else
        DrvFlagModeTA(num, 0);
    }
    else
    {
      if (bitst & DRV_FLAG_MODE)
        __FLAG_MODE_ON[num] = 1;
      else
        __FLAG_MODE_ON[num] = 0;
    }
    break;
#endif
  }
  return 0;
}

U16 FARFN rtgetmode(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned bits=0, bitst;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __TMKMPC:
    USER_ERROR(RT_BAD_FUNC);
    return 0;
  case __RTMK400:
    bitst = __rtControls1[num];
    bits = 0;
    if (bitst & RTMK400_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & RTMK400_FLAG_MODE)
      bits |= DRV_FLAG_MODE;
    if (bitst & RTMK400_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    break;
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    bitst = __rtControls1[realnum];
    bits = 0;
    if (bitst & MRTX_FLAG_MODE)
      bits |= DRV_FLAG_MODE;
    if (bitst & MRTX_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    break;
#endif
  case __TMKX:
    bitst = __rtControls1[num];
    bits = 0;
    if (bitst & TMKX_FLAG_MODE)
      bits |= DRV_FLAG_MODE;
    if (bitst & TMKX_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    break;
  case __TA:
    bitst = __rtControls1[num]; //inpw(TA_MODE2(num));
    bits = 0;
    if (bitst & TA_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & TA_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    if (__FLAG_MODE_ON[num])
      bits |= DRV_FLAG_MODE;
    break;
#ifdef MRTA
  case __MRTA:
    realnum = GET_RealNum;
    bitst = __rtControls1[realnum];
    bits = 0;
    if (bitst & TA_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & TA_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    if (__FLAG_MODE_ON[num])
      bits |= DRV_FLAG_MODE;
    break;
#endif
  }
  return bits;
}

#if NRT > 0
int FARFN mrtdefbrcpage(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 BrcPage)
{
  int num;
  unsigned type;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
  case __TA:
    return USER_ERROR(RT_BAD_FUNC);
#ifdef MRTA
  case __MRTA:
    page = BrcPage;
    if (page > 0)
      return USER_ERROR(RT_BAD_PAGE);
    __rtPagePC[num] =  __rtMaxPage[num] + 1;
//    __rtPageBus[num] =   __rtMaxPage[num] + 1;
    __hm400Page2[num] = MRTA_BRC_PAGE >> 4;
    __hm400Page[num] = (U16)(MRTA_BRC_PAGE << 11);
    break;
#endif
  }
  return 0;
}

U16 FARFN mrtgetbrcpage(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
//  int num;

  CLRtmkError;
//  num = __tmkNumber;
  return 0;
}
#endif //NRT

int FARFN rtdefpage(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtPage)
{
  int num;
  unsigned type;
  unsigned port;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  page = rtPage;
#ifdef MRTA
  if (type == __MRTA)
  {
    if (page > (__rtMaxPage[num] + 1))
      return USER_ERROR(RT_BAD_PAGE);
  }
  else
#endif
  {
    CHECK_RT_PAGE_BX(num, page);
  }
  __rtPagePC[num] = page;
  __rtPageBus[num] = page;
  switch (type)
  {
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
  case __TA:
    break;
  case __TMK400:
  case __RTMK400:
    page <<= 5;
    port = __tmkPortsAddr1[num] + TMK_AddrPort;
    outpw_d(num, port, page);
    __hm400Page[num] = page << 6;
    break;
  case __TMKX:
    page <<= 11;
    __hm400Page[num] = page;
    page >>= 3;
    GET_MUTEX;
    page |= __rtControls[num] & 0xE0FF;
    __rtControls[num] = page;
    REST_MUTEX;
    port = __tmkPortsAddr1[num] + TMK_ModePort;
    outpw(port, page);
    break;
#ifdef MRTA
  case __MRTA:
    if (page <= __rtMaxPage[num])
    { //std page 0
      __hm400Page2[num] = __hm400Page0[num] >> 4;
      __hm400Page[num] = __hm400Page0[num] << 11;
    }
    else
    { //brc page 0
      __hm400Page2[num] = MRTA_BRC_PAGE >> 4;
      __hm400Page[num] = (U16)(MRTA_BRC_PAGE << 11);
    }
    break;
#endif
  }
  return 0;
}

U16 FARFN rtgetpage(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber;
  return __rtPagePC[num];
}

U16 FARFN rtgetmaxpage(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber;
  return __rtMaxPage[num];
}

int FARFN rtdefpagepc(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 PagePC)
{
  int num;
  unsigned type;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  page = PagePC;
#ifdef MRTA
  if (type == __MRTA)
  {
    if (page > (__rtMaxPage[num] + 1))
      return USER_ERROR(RT_BAD_PAGE);
  }
  else
#endif
  {
    CHECK_RT_PAGE_BX(num, page);
  }
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
  case __TA:
    return USER_ERROR(RT_BAD_FUNC);
  case __TMKX:
    __rtPagePC[num] = page;
    __hm400Page[num] = page << 11;
    break;
#ifdef MRTA
  case __MRTA:
    __rtPagePC[num] = page;
    if (page <= __rtMaxPage[num])
    { //std page 0
      __hm400Page2[num] = __hm400Page0[num] >> 4;
      __hm400Page[num] = __hm400Page0[num] << 11;
    }
    else
    { //brc page 0
      __hm400Page2[num] = MRTA_BRC_PAGE >> 4;
      __hm400Page[num] = (U16)(MRTA_BRC_PAGE << 11);
    }
    break;
#endif
  }
  return 0;
}

int FARFN rtdefpagebus(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 PageBus)
{
  int num;
  unsigned type;
  unsigned port;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber;
  page = PageBus;
  CHECK_RT_PAGE_BX(num, page);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
  case __TA:
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(RT_BAD_FUNC);
  case __TMKX:
    __rtPageBus[num] = page;
    page <<= 8;
    GET_MUTEX;
    page |= __rtControls[num] & 0xE0FF;
    __rtControls[num] = page;
    REST_MUTEX;
    port = __tmkPortsAddr1[num] + TMK_ModePort;
    outpw(port, page);
    break;
  }
  return 0;
}

U16 FARFN rtgetpagepc(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber;
  return __rtPagePC[num];
}

U16 FARFN rtgetpagebus(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber;
  return __rtPageBus[num];
}

U16 FARFN rtenable(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 rtEnDis)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned mask, maskbrc;
  unsigned mask05, mask16;
  unsigned irt, nrt;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (rtEnDis)
  {
  case RT_GET_ENABLE:
    return (__rtDisableMask[num]) ? RT_DISABLE : RT_ENABLE;
  case RT_ENABLE:
  case RT_DISABLE:
    mask = rtEnDis;
    maskbrc = (unsigned)(-1);
    if (mask)
    {
      mask = __RT_DIS_MASK[type];
      maskbrc = __RT_BRC_MASK[type];
    }
    if (mask == __rtDisableMask[num])
      break;
    __rtDisableMask[num] = mask;
    __rtBRCMask[num] = maskbrc;
    switch (type)
    {
    case __TMK400:
      port = __tmkPortsAddr1[num] + TMK_CtrlPort;
      mask |= __rtAddress[num] | __rtMode[num];
      outpw_d(num, port, mask);
      break;
    case __RTMK400:
      port = __tmkPortsAddr1[num];
      mask |= (__rtControls1[num] & maskbrc) | __rtAddress[num];
      outpb_d(num, port, mask);
      break;
#ifdef MRTX
    case __MRTX:
      realnum = GET_RealNum;
      if (num == realnum)
        break;
      mask <<= 3;
      mask05 = 0x1F << 3;
      if (__hm400Page[num] & (1 << 11))
      {
        mask <<= 5;
        mask05 <<= 5;
      }
      mask16 = 0;
      if (__hm400Page[num] & (1 << 12))
      {
        mask16 = ~mask16;
      }
      __mrtMask1[realnum] &= ~(mask05 & mask16);
      __mrtMask1[realnum] |= mask & mask16;
      mask16 = ~mask16;
      __mrtMask0[realnum] &= ~(mask05 & mask16);
      __mrtMask0[realnum] |= mask & mask16;

      irt = __mrtMinRT[realnum];
      nrt = __mrtNRT[realnum] + irt - 1;
      mask = __rtBRCMask[irt];
      do
      {
        mask |= __rtBRCMask[++irt];
      }
      while (irt != nrt);
      __rtBRCMask[realnum] = mask;

      port = __tmkPortsAddr1[realnum] + TMK_ModePort;
      outpw(port, __mrtCtrl0[realnum] | __mrtMask0[realnum] | __rtControls[realnum]);
                                                      //; m__rtControls
      outpw(port, __mrtCtrl1[realnum] | __mrtMask1[realnum] | __rtControls[realnum]);
                                                      //; m__rtControls
      port += TMK_CtrlPort-TMK_ModePort;
      GET_DIS_IRQ_SMP();
      outpw(port, __rtControls1[num] & __rtBRCMask[num]);
      REST_IRQ_SMP();
      break;
#endif
    case __TMKX:
      port = __tmkPortsAddr1[num] + TMK_CtrlPort;
      GET_DIS_IRQ_SMP();
      outpw(port, (__rtControls1[num] & maskbrc) | mask);
      REST_IRQ_SMP();
                   //; Восстановление адреса ОУ
      break;
    case __TA:
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
      outpw(TA_MODE1(num), __rtControls[num] & ~TA_RTMT_START);
      outpw(TA_MODE2(num), (__rtControls1[num])); // & maskbrc) | mask;
      __rtControls[num] = (__rtControls[num] & ~TA_RTMT_START) | (TA_RTMT_START & (maskbrc>>1));
      outpw(TA_MODE1(num), __rtControls[num]);
      REST_IRQ_SMP();
      REST_MUTEX;
      break;
#ifdef MRTA
    case __MRTA:
      realnum = GET_RealNum;
      if (num == realnum)
        break;
      if (mask)
      {
        __dmrtRT[realnum] &= ~(1L << __rtAddress[num]);
        __dmrtBrc[realnum] &= ~(1L << __rtAddress[num]);
        GET_MUTEX;
        __rtControls1[num] &= ~MRTA_RT_ON;
        outpw(MRTA_SW(realnum), __rtControls1[num]);
        REST_MUTEX;
        if (DrvMrtaBrcRtOn(realnum) == 0)
          outpw(MRTA_SW(realnum), 0xF800);
        if (__dmrtRT[realnum] == 0L && __dmrtBrc[realnum] == 0L)
        {
          GET_MUTEX;
          GET_DIS_IRQ_SMP();
          __rtControls[realnum] &= ~TA_RTMT_START;
          outpw(TA_MODE1(realnum), __rtControls[realnum]);
          REST_IRQ_SMP();
          REST_MUTEX;
        }
      }
      else
      {
        __dmrtRT[realnum] |= 1L << __rtAddress[num];
        if (__rtControls1[num] & TA_BRCST_MODE)
          __dmrtBrc[realnum] |= 1L << __rtAddress[num];
        GET_MUTEX;
        __rtControls1[num] |= MRTA_RT_ON;
        outpw(MRTA_SW(realnum), __rtControls1[num]);
        REST_MUTEX;
        outpw(MRTA_SW(realnum), 0xF800 | __rtControls1[realnum] | DrvMrtaBrcRtOn(realnum));
        if ((__rtControls[realnum] & TA_RTMT_START) == 0 && (__dmrtRT[realnum] != 0L || __dmrtBrc[realnum] != 0L))
        {
          GET_MUTEX;
          GET_DIS_IRQ_SMP();
          __rtControls[realnum] |= TA_RTMT_START;
          outpw(TA_MODE1(realnum), __rtControls[realnum]);
          REST_IRQ_SMP();
          REST_MUTEX;
        }
        DrvRtWMode(num, __MRTA, 0);
      }
      break;
#endif
    case __TMKMPC:
      break;
    }
    break;
  default:
    return USER_ERROR(RT_BAD_FUNC);
  }
  return 0;
}

int FARFN rtdefaddress(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Address)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned rtaddr;
  unsigned mask;
  unsigned mask05, mask16;
  unsigned irt, nrt;
  unsigned shift;
#if NRT > 0
  int fEnable = 0;
  int rtctrl;
#endif

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  rtaddr = Address;
  CHECK_RT_ADDRESS(rtaddr);
  __rtAddress[num] = rtaddr;
  if (__rtDisableMask[num])
  {
    if (__rtEnableOnAddr[num])
    {
#if NRT > 0
      fEnable = 1;
#endif
      __rtDisableMask[num] = 0;  //; RT_ENABLE
      __rtBRCMask[num] = 0xFFFF;
    }
  }
  switch (type)
  {
  case __TMKMPC:
    return USER_ERROR(RT_BAD_FUNC);
  case __TMK400:
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    rtaddr |= __rtDisableMask[num] | __rtMode[num];
    outpw_d(num, port, rtaddr);
    break;
  case __RTMK400:
    rtaddr |= __rtControls1[num];
    rtaddr &= __rtBRCMask[num];
    rtaddr |= __rtDisableMask[num];
    port = __tmkPortsAddr1[num];
    outpb_d(num, port, rtaddr);
    break;
#ifdef MRTX
  case __MRTX:
    realnum = GET_RealNum;
    if (num == realnum)
      break;
    if (fEnable) //; became enabled?
    {
      mask = __rtDisableMask[num] << 3;
      mask05 = 0x1F << 3;
      if (__hm400Page[num] & (1 << 11))
      {
        mask <<= 5;
        mask05 <<= 5;
      }
      mask16 = 0;
      if (__hm400Page[num] & (1 << 12))
      {
        mask16 = ~mask16;
      }
      __mrtMask1[realnum] &= ~(mask05 & mask16);
      __mrtMask1[realnum] |= mask & mask16;
      mask16 = ~mask16;
      __mrtMask0[realnum] &= ~(mask05 & mask16);
      __mrtMask0[realnum] |= mask & mask16;

      irt = __mrtMinRT[realnum];
      nrt = __mrtNRT[realnum] + irt - 1;
      mask = __rtBRCMask[irt];
      do
      {
        mask |= __rtBRCMask[++irt];
      }
      while (irt != nrt);
      if (mask != __rtBRCMask[realnum])
      {
        __rtBRCMask[realnum] = mask;
        port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
        mask &= __rtControls1[realnum];
        GET_DIS_IRQ_SMP();
        outpw(port, mask);
        REST_IRQ_SMP();
      }
    }

    irt = __hm400Page[num] >> 11;
//    shift = (irt & 1) ? 8 : 3;
    shift = irt & 1;
    shift = (shift << 2) + shift + 3;
    mask = ~(0x1F << shift);
    rtaddr <<= shift;
    GET_MUTEX;
    if ((irt & 2) == 0)
    {
      mask &= __mrtCtrl0[realnum];
      rtaddr |= mask;
      __mrtCtrl0[realnum] = rtaddr;
      rtaddr |= __mrtMask0[realnum];
    }
    else
    {
      mask &= __mrtCtrl1[realnum];
      rtaddr |= mask;
      __mrtCtrl1[realnum] = rtaddr;
      rtaddr |= __mrtMask1[realnum];
    }
    rtaddr |= __rtControls[realnum];      //; m__rtControls
    port = __tmkPortsAddr1[realnum] + TMK_ModePort;
    outpw(port, rtaddr);
    REST_MUTEX;
    break;
#endif
  case __TMKX:
    rtaddr <<= 11;
    GET_MUTEX;
    rtaddr |= __rtControls1[num] & 0x057F;
    __rtControls1[num] = rtaddr;
    REST_MUTEX;
    rtaddr &= __rtBRCMask[num];
    rtaddr |= __rtDisableMask[num];
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    GET_DIS_IRQ_SMP();
    outpw(port, rtaddr);
    REST_IRQ_SMP();
    break;
  case __TA:
    rtaddr <<= 11;
    GET_MUTEX;
    GET_DIS_IRQ_SMP();
    outpw(TA_MODE1(num), __rtControls[num] & ~TA_RTMT_START);
    rtaddr |= __rtControls1[num] & 0x07FF;
    __rtControls1[num] = rtaddr;
    __rtControls[num] = (__rtControls[num] & ~TA_RTMT_START) | (TA_RTMT_START & (__rtBRCMask[num]>>1));
//    rtaddr &= __rtBRCMask[num];
//    rtaddr |= __rtDisableMask[num];
    outpw(TA_MODE2(num), rtaddr);
    outpw(TA_MODE1(num), __rtControls[num]);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
#ifdef MRTA
  case __MRTA:
    realnum = GET_RealNum;
    if (num == realnum)
      break;
//    if (__dmrtRT[realnum] & (1L << rtaddr))
//      USER_ERROR(RT_BAD_ADDRESS)
    rtctrl = __rtControls1[num] | __FLAG_MODE_ON[num]; // previous ctrl
    rtaddr = rtctrl >> 11;   // previous addr
    if (rtaddr != 31 && rtaddr != __rtAddress[num])
    {
      __dmrtRT[realnum] &= ~(1L << rtaddr);
      __dmrtBrc[realnum] &= ~(1L << rtaddr);
      __mrtA2RT[realnum][rtaddr] = 0;
      GET_MUTEX;
      __rtControls1[num] &= ~(MRTA_RT_ON | TA_BRCST_MODE);
      outpw(MRTA_SW(realnum), __rtControls1[num]);
      REST_MUTEX;
      if (DrvMrtaBrcRtOn(realnum) == 0)
        outpw(MRTA_SW(realnum), 0xF800);
      DrvFlagModeTA(num, 0);
    }
    if (rtaddr != __rtAddress[num]) // aka rtreset (+ all settings after) for new RT !
    {
      rtaddr = __rtAddress[num];
      __mrtA2RT[realnum][rtaddr] = num;
      __hm400Page0[num] = 0x20 | rtaddr;
      __hm400Page2[num] = __hm400Page0[num] >> 4; //rtdefpage(0)
      __hm400Page[num] = __hm400Page0[num] << 11;
      mrtcreattab(realnum, rtaddr);
      if (rtctrl & 1) // flag mode?
        DrvFlagModeTA(num, 1);
      DrvRtWMode(num, __MRTA, 0);
      if (__rtDisableMask[num])
      {
        __rtControls1[num] = ((rtctrl & 0x07FF) | (rtaddr << 11)) & ~MRTA_RT_ON;
        __dmrtRT[realnum] &= ~(1L << rtaddr);
        __dmrtBrc[realnum] &= ~(1L << rtaddr);
      }
      else
      {
        __rtControls1[num] = (rtctrl & 0x07FF) | (rtaddr << 11) | MRTA_RT_ON;
        __dmrtRT[realnum] |= 1L << rtaddr;
        if (__rtControls1[num] & TA_BRCST_MODE)
          __dmrtBrc[realnum] |= 1L << rtaddr;
      }
      GET_MUTEX;
      outpw(MRTA_SW(realnum), __rtControls1[num]);
      REST_MUTEX;
      outpw(MRTA_SW(realnum), 0xF800 | __rtControls1[realnum] | DrvMrtaBrcRtOn(realnum));
      if ((__rtControls[realnum] & TA_RTMT_START) == 0 && (__dmrtRT[realnum] != 0L || __dmrtBrc[realnum] != 0L))
      {
        GET_MUTEX;
        GET_DIS_IRQ_SMP();
        __rtControls[realnum] |= TA_RTMT_START;
        outpw(TA_MODE1(realnum), __rtControls[realnum]);
        REST_IRQ_SMP();
        REST_MUTEX;
      }
    }
    break;
#endif
  }
  return 0;
}

U16 FARFN rtgetaddress(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned type;
  unsigned port;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TMKMPC:
    USER_ERROR(RT_BAD_FUNC);
    return 0xFFFF;
  case __TMK400:
    port = __tmkPortsAddr2[num];
    return inpb_d(num, port) & 0x1F;
  case __RTMK400:
    port = __tmkPortsAddr1[num];
    return (inpb_d(num, port) >> 2) & 0x1F;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
#ifdef MRTA
  case __MRTA:
#endif
    return __rtAddress[num];
  case __TA:
    return inpw(TA_MODE2(num)) >> 11;
  }
  return __rtAddress[num];
}

void FARFN rtgetflags(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
         VOID FARDT *pcBufAddr, U16 Dir, U16 FlagMin, U16 FlagMax)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  unsigned dir;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  pos = FlagMin;
  CHECK_RT_SUBADDR(pos);
  len = FlagMax;
  CHECK_RT_SUBADDR(len);
  len = len + 1 - pos;
  if ((int)len <= 0)
    return;
  dir = Dir;
  CHECK_RT_DIR(dir);
  pos |= dir;
  __rtSubAddr[num] = dir;
  pos |= __hm400Page[num];
  buf = (U16 FARDT*)pcBufAddr;
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    outpw_d(realnum, port, pos >> 6);
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_AddrHPort-TMKMPC_AddrLPort;
    outpb_d(realnum, port, pos >> 6);
    port += TMKMPC_DataLPort-TMKMPC_AddrHPort;
    REP_INSW_D;
//    REP_INSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_INSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
#ifdef MRTA
  case __MRTA:
#endif
  case __TA:
    pos = FlagMin;
    do
    {
#ifdef STATIC_TMKNUM
      *(buf++) = rtgetflag(dir, pos);
#else
      *(buf++) = rtgetflag(realnum, dir, pos);
#endif
      ++pos;
    }
    while (--len);
    break;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

//__EXTERN void __FAR rtgetflags(void __FAR *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
//__EXTERN void __FAR rtputflags(void __FAR *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
void FARFN rtputflags(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
         VOID FARDT *pcBufAddr, U16 Dir, U16 FlagMin, U16 FlagMax)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned len;
  unsigned dir;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  pos = FlagMin;
  CHECK_RT_SUBADDR(pos);
  len = FlagMax;
  CHECK_RT_SUBADDR(len);
  len = len + 1 - pos;
  if ((int)len <= 0)
    return;
  dir = Dir;
  CHECK_RT_DIR(dir);
  pos |= dir;
  __rtSubAddr[num] = dir;
  pos |= __hm400Page[num];
  buf = (U16 FARDT*)pcBufAddr;
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    outpw_d(realnum, port, pos >> 6);
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW_D;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_AddrHPort-TMKMPC_AddrLPort;
    outpb_d(realnum, port, pos >> 6);
    port += TMKMPC_DataLPort-TMKMPC_AddrHPort;
    REP_OUTSWB_D;
    if (!save_ramiw)
      break;
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    REST_IRQ_SMP();
    port += TMK_DataPort-TMK_AddrPort;
    REP_OUTSW;
    if (!save_ramiw)
      break;
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
#ifdef MRTA
  case __MRTA:
#endif
  case __TA:
    pos = FlagMin;
    do
    {
#ifdef STATIC_TMKNUM
      rtputflag(dir, pos, *(buf++));
#else
      rtputflag(realnum, dir, pos, *(buf++));
#endif
      ++pos;
    }
    while (--len);
    break;
  }
  __tmkRAMInWork[realnum] = 0;
  return;
}

void FARFN rtsetflag(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned pos;
  unsigned data;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  sa = __rtSubAddr[num];
  switch (type)
  {
  default:
    pos = ((sa >> 5) | sa) & 0x041F;
    DrvRtPoke(num, type, pos, 0x8000);
    DrvRtDefSubAddr(num, type, sa);
    break;
  case __TA:
    sa >>= 5;
    if (!__FLAG_MODE_ON[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG[num][sa&0x1f] = 0x8000;
      else
        __BC_RT_FLAG[num][sa&0x1f] = 0x8000;
    }
    else
    {
      GET_DIS_IRQ_SMP();
      data = DrvRtPeekTA(num, AdrTab, sa);
      if (sa & 0x20)
        data &= 0x7FFF;
      else
        data |= 0x8000;
      DrvRtPokeTA(num, AdrTab, sa, data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#ifdef MRTA
  case __MRTA:
    sa >>= 5;
    if (!__FLAG_MODE_ON[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG[num][sa&0x1f] = 0x8000;
      else
        __BC_RT_FLAG[num][sa&0x1f] = 0x8000;
    }
    else
    {
      int realnum;
      realnum = GET_RealNum;
      GET_DIS_IRQ_SMP();
      outpw(MRTA_ADDR2(realnum), 0);
      data = DrvRtPeekTA(realnum, __rtAddress[num], sa);
      if (sa & 0x20)
        data &= 0x7FFF;
      else
        data |= 0x8000;
      DrvRtPokeTA(realnum, __rtAddress[num], sa, data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#endif
  }
  return;
}

void FARFN rtclrflag(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned pos;
  unsigned data;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  sa = __rtSubAddr[num];
  switch (type)
  {
  default:
    pos = ((sa >> 5) | sa) & 0x041F;
    DrvRtPoke(num, type, pos, 0);
    DrvRtDefSubAddr(num, type, sa);
    break;
  case __TA:
    sa >>= 5;
    if (!__FLAG_MODE_ON[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG[num][sa&0x1f] = 0x0000;
      else
        __BC_RT_FLAG[num][sa&0x1f] = 0x0000;
    }
    else
    {
      GET_DIS_IRQ_SMP();
      data = DrvRtPeekTA(num, AdrTab, sa);
      if (sa & 0x20)
        data |= 0x8000;
      else
        data &= 0x7FFF;
      DrvRtPokeTA(num, AdrTab, sa, data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#ifdef MRTA
  case __MRTA:
    sa >>= 5;
    if (!__FLAG_MODE_ON[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG[num][sa&0x1f] = 0x0000;
      else
        __BC_RT_FLAG[num][sa&0x1f] = 0x0000;
    }
    else
    {
      int realnum;
      realnum = GET_RealNum;
      GET_DIS_IRQ_SMP();
      outpw(MRTA_ADDR2(realnum), 0);
      data = DrvRtPeekTA(realnum, __rtAddress[num], sa);
      if (sa & 0x20)
        data |= 0x8000;
      else
        data &= 0x7FFF;
      DrvRtPokeTA(realnum, __rtAddress[num], sa, data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#endif
  }
  return;
}

void FARFN rtputflag(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Dir, U16 SubAddr, U16 Flag)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;
  unsigned pos;
  unsigned data;

  CLRtmkError;
  num = __tmkNumber;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  dir = Dir;
  CHECK_RT_DIR(dir);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  default:
    pos = sa | dir;
    DrvRtPoke(num, type, pos, Flag);
    DrvRtDefSubAddr(num, type, sa);
    break;
  case __TA:
    if (!__FLAG_MODE_ON[num])
    {
      if (dir)
        __RT_BC_FLAG[num][sa] = Flag;
      else
        __BC_RT_FLAG[num][sa] = Flag;
    }
    else
    {
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
      data = DrvRtPeekTA(num, AdrTab, pos);
      if (Flag & 0x8000)
      {
        if (dir)
          data &= 0x7FFF;
        else
          data |= 0x8000;
      }
      else
      {
        if (dir)
          data |= 0x8000;
        else
          data &= 0x7FFF;
      }
      DrvRtPokeTA(num, AdrTab, pos, data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#ifdef MRTA
  case __MRTA:
    if (!__FLAG_MODE_ON[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG[num][sa&0x1f] = 0x0000;
      else
        __BC_RT_FLAG[num][sa&0x1f] = 0x0000;
    }
    else
    {
      int realnum;
      realnum = GET_RealNum;
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
      outpw(MRTA_ADDR2(realnum), 0);
      data = DrvRtPeekTA(realnum, __rtAddress[num], pos);
      if (Flag & 0x8000)
      {
        if (dir)
          data &= 0x7FFF;
        else
          data |= 0x8000;
      }
      else
      {
        if (dir)
          data |= 0x8000;
        else
          data &= 0x7FFF;
      }
      DrvRtPokeTA(realnum, __rtAddress[num], pos, data);
      outpw(MRTA_ADDR2(realnum), __hm400Page0[num] >> 4);
      DrvRtPokeTA(realnum, (__hm400Page0[num] << 6) | pos, 58, Flag & 0x7FFF);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
#endif
  }
  return;
}

U16 FARFN rtgetflag(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Dir, U16 SubAddr)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;
  unsigned pos;
  unsigned flag;

  CLRtmkError;
  num = __tmkNumber;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  dir = Dir;
  CHECK_RT_DIR(dir);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  default:
    pos = sa | dir;
    flag = DrvRtPeek(num, type, pos);
    break;
  case __TA:
    if (!__FLAG_MODE_ON[num])
    {
      if (dir)
        flag = __RT_BC_FLAG[num][sa];
      else
        flag = __BC_RT_FLAG[num][sa];
    }
    else
    {
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
      flag = DrvRtPeekTA(num, AdrTab, pos);
      if (dir)
      {
        flag = (~flag) & 0x8000;
      }
      else
      {
        flag = flag & 0x8000;
      }
      flag |= DrvRtPeekTA(num, pos, 58) & 0x07FF;
      REST_IRQ_SMP();
    }
    break;
#ifdef MRTA
  case __MRTA:
    if (!__FLAG_MODE_ON[num])
    {
      if (dir)
        flag = __RT_BC_FLAG[num][sa];
      else
        flag = __BC_RT_FLAG[num][sa];
    }
    else
    {
      int realnum;
      realnum = GET_RealNum;
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
      outpw(MRTA_ADDR2(realnum), 0);
      flag = DrvRtPeekTA(realnum, __rtAddress[num], pos);
      if (dir)
      {
        flag = (~flag) & 0x8000;
      }
      else
      {
        flag = flag & 0x8000;
      }
      outpw(MRTA_ADDR2(realnum), __hm400Page0[num] >> 4);
      flag |= DrvRtPeekTA(realnum, (__hm400Page0[num] << 6) | pos, 58) & 0x07FF;
      REST_IRQ_SMP();
    }
    break;
#endif
  }
  DrvRtDefSubAddr(num, type, (sa << 5) | dir);
  return flag;
}

int FARFN rtbusy(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned state=0, statex;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1[realnum];
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_StateLPort-TMK_StatePort;
  case __TMK400:
  case __RTMK400:
    port += TMK_StatePort;
    state = inpw_d(realnum, port);
    break;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_StatePort;
    state = inpw(port);
    do
    {
      statex = state;
      state = inpw(port);
    }
    while (statex != state);
    break;
  case __TA:
    state = inpw(TA_BASE(realnum));
    if (((state & 0x8000) == 0) && ((state & 0x03FF) == (DrvRtGetBaseTA(realnum) & 0x03FF)))
      state >>= 2;
    else
      state = 0;
    break;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ();
    outpw(TA_LCW(realnum), __rtControls1[__tmkNumber]);
    state = inpw(TA_BASE(realnum));
    REST_IRQ();
    if (((state & 0x8000) == 0) && ((state & 0x0FFF) == (((__rtSubAddr[__tmkNumber] | __hm400Page[__tmkNumber]) >> 5) | 0x0800)))
      state >>= 2;
    else
      state = 0;
    break;
#endif
  }
  return (state >> 11) & 1;
}

#if NRT > 0
U16 FARFN mrtgetstate(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned state=0, statex;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1[realnum];
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_StateLPort-TMK_StatePort;
  case __TMK400:
  case __RTMK400:
    port += TMK_StatePort;
    state = inpw_d(realnum, port) & 0x5FFF;
    break;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_StatePort;
    state = inpw(port);
    do
    {
      statex = state;
      state = inpw(port);
    }
    while (statex != state);
    break;
  case __TA:
    GET_DIS_IRQ();
    state = inpw(TA_LCW(realnum)) & 0x7FF;
    statex = inpw(TA_BASE(realnum));
    if (statex & 0x2000)
      state = inpw(TA_LCW(realnum)) & 0x7FF;
    REST_IRQ();
    if (((statex & 0x8000) == 0) && ((statex&0x03FF) == (DrvRtGetBaseTA(realnum)&0x03FF)))
      state |= (statex & 0x2000) >> 2;
    break;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ();
    outpw(TA_LCW(realnum), __rtControls1[__tmkNumber]);
    state = inpw(TA_LCW(realnum)) & 0x7FF;
    statex = inpw(TA_BASE(realnum));
    if (statex & 0x2000)
      state = inpw(TA_LCW(realnum)) & 0x7FF;
    REST_IRQ();
    if (((statex & 0x8000) == 0) && ((statex & 0x0FFF) == (((__rtSubAddr[__tmkNumber] | __hm400Page[__tmkNumber]) >> 5) | 0x0800)))
      state |= (statex & 0x2000) >> 2;
    break;
#endif
  }
  return state;
}
#endif //NRT

U16 FARFN rtgetstate(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned state=0, statex;
  unsigned pos;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  port = __tmkPortsAddr1[realnum];
  switch (type)
  {
  case __TMKMPC:
    port += TMKMPC_StateLPort-TMK_StatePort;
  case __TMK400:
  case __RTMK400:
    port += TMK_StatePort;
    state = inpw_d(realnum, port) & 0x5FFF;
    break;
#ifdef MRTX
  case __MRTX:
//    num = __tmkNumber;
    port += TMK_StatePort;
    state = inpw(port);
    do
    {
      statex = state;
      state = inpw(port);
    }
    while (statex != state);
    if ((state & 0x8000) == 0)
    {
      if (((state >> 1) & 0x1800) != __hm400Page[num])
      {
        pos = __hm400Page[num] | 0x07F2;
        port += TMK_AddrPort-TMK_StatePort;
        GET_DIS_IRQ();
        outpw(port, pos);
        port += TMK_DataPort-TMK_AddrPort;
        state = inpw(port);
        REST_IRQ();
        state &= 0x07FF;
      }
    }
    state &= 0xCFFF;
    break;
#endif
  case __TMKX:
    port += TMK_StatePort;
    state = inpw(port);
    do
    {
      statex = state;
      state = inpw(port);
    }
    while (statex != state);
    break;
  case __TA:
    GET_DIS_IRQ();
    state = inpw(TA_LCW(realnum)) & 0x7FF;
    statex = inpw(TA_BASE(realnum));
    if (statex & 0x2000)
      state = inpw(TA_LCW(realnum)) & 0x7FF;
    REST_IRQ();
    if (((statex & 0x8000) == 0) && ((statex&0x03FF) == (DrvRtGetBaseTA(realnum)&0x03FF)))
      state |= (statex & 0x2000) >> 2;
    break;
#ifdef MRTA
  case __MRTA:
    GET_DIS_IRQ();
    outpw(TA_LCW(realnum), __rtControls1[num]);
    state = inpw(TA_LCW(realnum)) & 0x7FF;
    statex = inpw(TA_BASE(realnum));
    if (statex & 0x2000)
      state = inpw(TA_LCW(realnum)) & 0x7FF;
    REST_IRQ();
    if (((statex & 0x8000) == 0) && ((statex & 0x0FFF) == (((__rtSubAddr[num] | __hm400Page[num]) >> 5) | 0x0800)))
      state |= (statex & 0x2000) >> 2;
    break;
#endif
  }
  return state;
}

void FARFN rtlock(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Dir, U16 SubAddr)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;

  CLRtmkError;
  num = __tmkNumber;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  sa <<= 5;
  dir = Dir;
  CHECK_RT_DIR(dir);
  sa |= dir;
  DrvRtDefSubAddr(num, type, sa);
  DrvRtWMode(num, type, sa | 0x0800);
  return;
}

void FARFN rtunlock(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned type;

  CLRtmkError;
  num = __tmkNumber;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  DrvRtWMode(num, type, __rtSubAddr[num]);
  return;
}

U16 FARFN rtgetlock(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned mode;

  CLRtmkError;
  num = __tmkNumber;   //; tmkRealNumber2 ???
  mode = __rtMode[num];
  return (mode & 0xFC00) | ((mode >> 5) & 0x001F);
}

U16 FARFN rtgetcmddata(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 BusCommand)
{
  int num;
  unsigned type;
  DEF_VAR(unsigned, cmd);

  CLRtmkError;
  num = __tmkNumber;
  GET_VAR(cmd, BusCommand);
  CHECK_RT_CMD(cmd);
  cmd |= 0x03E0;
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
#ifdef MRTX
  case __MRTX:
    cmd |= 0x0400; //; 7F0h
#endif
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
  case __TMKX:
    return DrvRtPeek(num, type, cmd);
  case __TA:
// need to mantain an array of cmddatas for full compatibility
  {
    unsigned data;
    switch (cmd)
    {
    case 0x7F0: //0x410
      GET_DIS_IRQ_SMP();
      data = DrvRtPeekTA(num, 0x20, 0);
      REST_IRQ_SMP();
      break;
    case 0x7F3: //0x413
      GET_DIS_IRQ_SMP();
      data = DrvRtPeekTA(num, 0x3F, 0);
      REST_IRQ_SMP();
      break;
    case 0x3F1: //0x011
    case 0x3F4: //0x014
    case 0x3F5: //0x015
      data = __rtRxDataCmd[num][cmd-0x3F1];
      break;
    default:
      data = 0;
    }
    return (U16)data;
  }
#ifdef MRTA
  case __MRTA:
// need to mantain an array of cmddatas for full compatibility
  {
    unsigned data;
    int realnum;
    realnum = GET_RealNum;
    switch (cmd)
    {
    case 0x7F0: //0x410
      GET_DIS_IRQ_SMP();
      // page depend on std/brc (programmer can use any page)
      outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
      data = DrvRtPeekTA(realnum, (__hm400Page[num] >> 5) | 0x20, 0);
      REST_IRQ_SMP();
      break;
    case 0x7F3: //0x413
      GET_DIS_IRQ_SMP();
      // page depend on std/brc (programmer can use any page)
      outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
      data = DrvRtPeekTA(realnum, (__hm400Page[num] >> 5) | 0x3F, 0);
      REST_IRQ_SMP();
      break;
    case 0x3F1: //0x011
    case 0x3F4: //0x014
    case 0x3F5: //0x015
      data = __rtRxDataCmd[num][cmd-0x3F1];
      break;
    default:
      data = 0;
    }
    return (U16)data;
  }
#endif
  }
  return 0;
}

void FARFN rtputcmddata(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 BusCommand, U16 rtData)
{
  int num;
  unsigned type;
  DEF_VAR(unsigned, cmd);

  CLRtmkError;
  num = __tmkNumber;
  GET_VAR(cmd, BusCommand);
  CHECK_RT_CMD(cmd);
  cmd |= 0x03E0;        //;or      ax, 7F0h
  type = __tmkDrvType[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
#ifdef MRTX
  case __MRTX:
    cmd |= 0x0400; //; 7F0h
#endif
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
  case __TMKX:
    DrvRtPoke(num, type, cmd, rtData);
    break;
  case __TA:
// need to mantain an array of cmddatas for full compatibility
    switch (cmd)
    {
    case 0x7F0: //0x410
      GET_DIS_IRQ_SMP();
      DrvRtPokeTA(num, 0x20, 0, rtData);
      REST_IRQ_SMP();
      break;
    case 0x7F3: //0x413
      GET_DIS_IRQ_SMP();
      DrvRtPokeTA(num, 0x3F, 0, rtData);
      REST_IRQ_SMP();
      break;
    case 0x3F1: //0x011
    case 0x3F4: //0x014
    case 0x3F5: //0x015
      __rtRxDataCmd[num][cmd-0x3F1] = rtData;
      break;
    }
    break;
#ifdef MRTA
  case __MRTA:
// need to mantain an array of cmddatas for full compatibility
    {
    int realnum;
    realnum = GET_RealNum;
    switch (cmd)
    {
    case 0x7F0: //0x410
      GET_DIS_IRQ_SMP();
      // page depend on std/brc (programmer should use std page)
      outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
      DrvRtPokeTA(realnum,(__hm400Page[num] >> 5) | 0x20, 0, rtData);
      REST_IRQ_SMP();
      break;
    case 0x7F3: //0x413
      GET_DIS_IRQ_SMP();
      // page depend on std/brc (programmer should use std page)
      outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
      DrvRtPokeTA(realnum,(__hm400Page[num] >> 5) | 0x3F, 0, rtData);
      REST_IRQ_SMP();
      break;
    case 0x3F1: //0x011
    case 0x3F4: //0x014
    case 0x3F5: //0x015
      __rtRxDataCmd[num][cmd-0x3F1] = rtData;
      break;
    }
    break;
    }
#endif
  }
  return;
}

void DrvRtWMode(int __tmkNumber, unsigned type, unsigned mode)
{
  int num, realnum;
  unsigned port;
  unsigned sa;
  IRQ_FLAGS;

  realnum = GET_RealNum;
  if (mode == __rtMode[realnum])
    return;
  num = __tmkNumber;
  __rtMode[num] = mode;
  switch (type)
  {
  case __TMK400:
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    outpw_d(num, port, mode | __rtAddress[num] | __rtDisableMask[num]);
    return;
  case __RTMK400:
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    outpw_d(num, port, mode);
    return;
  case __TMKMPC:
    port = __tmkPortsAddr1[num] + TMKMPC_CtrlHPort;
    outpwb_d(num, port, mode);
    return;
#ifdef MRTX
  case __MRTX:
    __rtMode[realnum] = mode;
    port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
    mode >>= 5;
    if ((mode & 0x1F) == 0 || (mode & 0x1F) == 0x1F)
      mode = 1;
    GET_MUTEX;
    mode |= __rtControls1[realnum] & 0x0500;
    __rtControls1[realnum] = mode;
    REST_MUTEX;
    GET_DIS_IRQ_SMP();
    outpw(port, mode & __rtBRCMask[realnum]);
    REST_IRQ_SMP();
    return;
#endif
  case __TMKX:
    port = __tmkPortsAddr1[num] + TMK_CtrlPort;
    mode >>= 5;
    GET_MUTEX;
    mode |= __rtControls1[num] & 0xFD00;
    __rtControls1[num] = mode;
    REST_MUTEX;
    GET_DIS_IRQ_SMP();
    outpw(port, (mode & __rtBRCMask[num]) | __rtDisableMask[num]);
    REST_IRQ_SMP();
    return;
  case __TA:
    if (__FLAG_MODE_ON[num])
      return;
    sa = (mode >> 5) & 0x3F;
    GET_DIS_IRQ_SMP();
    DrvRtPokeTA(num, AdrTab, sa, (DrvRtPeekTA(num, AdrTab, sa) & 0x7FFF) | ((mode << 4) & 0x8000));
//    sa = DrvRtPeekTA(num, AdrTab, sa);
    REST_IRQ_SMP();
    return;
#ifdef MRTA
  case __MRTA:
    if (__FLAG_MODE_ON[num])
      return;
    sa = (mode >> 5) & 0x3F;
    GET_DIS_IRQ_SMP();
    outpw(MRTA_ADDR2(realnum), 0);
    DrvRtPokeTA(realnum, __rtAddress[num], sa, (DrvRtPeekTA(realnum, __rtAddress[num], sa) & 0x7FFF) | ((mode << 4) & 0x8000));
//    sa = DrvRtPeekTA(num, __rtAddress[num], sa);
//    outpw(MRTA_ADDR2(realnum), __hm400Page2[num]);
    REST_IRQ_SMP();
    return;
#endif
  }
  return;
}

void DrvRtPoke(int __tmkNumber, unsigned type, unsigned pos, unsigned data)
// not for TA boards!
// write to pos in current page, update current subaddr
{
  int register num, realnum;
  unsigned port;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  num = __tmkNumber;
  __rtSubAddr[num] = pos & 0x07E0;
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  port = __tmkPortsAddr1[realnum];
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    outpw_d(realnum, port, pos >> 6);
    port += TMK_DataPort-TMK_AddrPort;
    outpw_d(realnum, port, data);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_AddrHPort-TMKMPC_AddrLPort;
    outpb_d(realnum, port, pos >> 6);
    port += TMKMPC_DataHPort-TMKMPC_AddrHPort;
    outpwb_d(realnum, port, data);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    return;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    outpw(port, data);
    REST_IRQ_SMP();
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      return;
    }
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    return;
//  case __TA:
//  ???
//    break;
//  case __MRTA:
//  ???
//    break;
  }
  return;
}

unsigned DrvRtPeek(int __tmkNumber, unsigned type, unsigned pos)
// not for TA boards!
// read from pos in current page, update current subaddr
{
  int register num, realnum;
  unsigned data=0;
  unsigned port;
  unsigned save_rama, save_ramiw;

  num = __tmkNumber;
  __rtSubAddr[num] = pos & 0x07E0;
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  port = __tmkPortsAddr1[realnum];
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port += TMK_AddrPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    outpw_d(realnum, port, pos >> 6);
    port += TMK_DataPort-TMK_AddrPort;
    data = inpw_d(realnum, port);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    break; //return data;
  case __TMKMPC:
    port += TMKMPC_AddrLPort;
    __tmkRAMAddr[realnum] = pos;
    outpb_d(realnum, port, pos);
    port += TMKMPC_AddrHPort-TMKMPC_AddrLPort;
    outpb_d(realnum, port, pos >> 6);
    port += TMKMPC_DataLPort-TMKMPC_AddrHPort;
    data = inpw_d(realnum, port);
//    data = inpwb_d(port);
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMKMPC_AddrLPort-TMKMPC_DataLPort;
    __tmkRAMAddr[realnum] = save_rama;
    outpb_d(realnum, port, save_rama);
    break; //return data;
#ifdef MRTX
  case __MRTX:
#endif
  case __TMKX:
    port += TMK_AddrPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = pos;
    outpw(port, pos);
    port += TMK_DataPort-TMK_AddrPort;
    data = inpw(port);
    REST_IRQ_SMP();
    if (!save_ramiw)
    {
      __tmkRAMInWork[realnum] = 0;
      break; //return data;
    }
    port += TMK_AddrPort-TMK_DataPort;
    GET_DIS_IRQ_SMP();
    __tmkRAMAddr[realnum] = save_rama;
    outpw(port, save_rama);
    REST_IRQ_SMP();
    break; //return data;
//  case __TA:
//  ???
//    break;
//  case __MRTA:
//  ???
//    break;
  }
  return data;
}

#ifdef MRTX
void DrvRtPokeIrqMRTX(int __tmkNumber, unsigned pos, unsigned data)
{
  int register num, realnum;
  unsigned port;
  unsigned save_rama, save_ramiw;

  num = __tmkNumber;
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
  __tmkRAMAddr[realnum] = pos;
  outpw(port, pos);
  port += TMK_DataPort-TMK_AddrPort;
  outpw(port, data);
  if (!save_ramiw)
  {
    __tmkRAMInWork[realnum] = 0;
    return;
  }
  port += TMK_AddrPort-TMK_DataPort;
  __tmkRAMAddr[realnum] = save_rama;
  outpw(port, save_rama);
  return;
}

unsigned DrvRtPeekIrqMRTX(int __tmkNumber, unsigned pos)
{
  int register num, realnum;
  unsigned data;
  unsigned port;
  unsigned save_rama, save_ramiw;

  num = __tmkNumber;
  pos |= __hm400Page[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr[realnum];
  save_ramiw = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 1;
  port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
  __tmkRAMAddr[realnum] = pos;
  outpw(port, pos);
  port += TMK_DataPort-TMK_AddrPort;
  data = inpw(port);
  if (!save_ramiw)
  {
    __tmkRAMInWork[realnum] = 0;
    return data;
  }
  port += TMK_AddrPort-TMK_DataPort;
  __tmkRAMAddr[realnum] = save_rama;
  outpw(port, save_rama);
  return data;
}
#endif //def MRTX

#if defined(DOS) || defined(QNX4)
int SetupPCI(int num)
{
  unsigned subdev;
  int iDev;
  unsigned port;
  unsigned data;

  if (!pciPciBiosPresent())
    return USER_ERROR(TMK_PCI_ERROR);
  __nFoundDev = 0;
  iDev = 0;
  while (1)
  {
    if (!pciFindPciDevice(PLX_DID, PLX_VID, iDev++))
      return USER_ERROR(TMK_PCI_ERROR);
    if (pciReadConfigWord(__pciBusDevFun, CFG_SUB_VID) == ELCUS_VID)
    {
      switch (pciReadConfigWord(__pciBusDevFun, CFG_SUB_DID))
      {
      case TX1PCI_ID:
      case TX6PCI_ID:
      case TX1PCI2_ID:
      case TX6PCI2_ID:
      case TA1PCI_ID:
      case TA1PCI4_ID:
      case TA1PCI32RT_ID:
        ++__nFoundDev;
        if (__nFoundDev != __tmkPortsAddr1[num])
          continue;
        subdev = __tmkPortsAddr2[num];
        if (subdev == 0xFFFF)
          subdev = 1;
        if (subdev < 1 || subdev > 4)
          return USER_ERROR(TMK_BAD_NUMBER);
        port = pciReadConfigWord(__pciBusDevFun, CFG_ADDR1 + (subdev<<2)) & 0xFFFE;
        if (port == 0)
          return USER_ERROR(TMK_BAD_NUMBER);
        __tmkPortsAddr1[num] = port;
        __tmkPortsAddr2[num] = pciReadConfigWord(__pciBusDevFun, CFG_ADDR1) & 0xFFFE;
        __tmkIrq1[num] = pciReadConfigByte(__pciBusDevFun, CFG_IRQ);
        port += TMK_AddrPort;
        data = inpw(port);
        outpw(port, ~data);
        data ^= inpw(port);
        if (data == 0xFFFF)
        {
        // IrqTmk for TA1-PCI
          __tmkIrqPort[num] = port + (TMK_ResetPort-TMK_AddrPort);
          __tmkIrqBit[num] = 0xE000;
        }
        else if (data == 0x3FFF)
        {
        // IrqTmk for TE1-PCI2
          __tmkIrqPort[num] = port;
          __tmkIrqBit[num] = 0x8000;
        }
        else //if (data == 0 || data == 0x7FFF)
        {
        // IrqPlx for TE1-PCI
          __tmkIrqPort[num] = __tmkPortsAddr2[num] + 0x4C;
          __tmkIrqBit[num] = 0x0004;
        }
        outpw(port, 0);
        break;
      default:
        continue;
      }
      break;
    }
  }
  return 0;
}
#endif //def DOS

int ExamTmkRAM(int num, unsigned type)
{
  unsigned pos, size;
  unsigned port;
  unsigned save_idelay, save_odelay;

#if DRV_MAX_BASE > 255
  __mtMaxBase[num] = 0;
#endif
  switch (type)
  {
  case __TMK400:
    __tmkRAMSize[num] = 8;
    __bcMaxBase[num] = 127;
    __rtMaxPage[num] = 3;
    break;
  case __RTMK400:
    save_idelay = __wInDelay[num];
    save_odelay = __wOutDelay[num];
    __wInDelay[num] = 32;
    __wOutDelay[num] = 32;
    port = __tmkPortsAddr1[num] + TMK_AddrPort;
    outpw_d(num, port, 60 >> 6);
    outpb_d(num, port, 60);
    port += TMK_DataPort-TMK_AddrPort;
    outpw_d(num, port, 0);
    size = 0x0800;
    do
    {
      port += TMK_AddrPort-TMK_DataPort;
      pos = size + 60;
      outpw_d(num, port, pos >> 6);
      outpb_d(num, port, pos);
      port += TMK_DataPort-TMK_AddrPort;
      outpw_d(num, port, pos);
      port += TMK_AddrPort-TMK_DataPort;
      pos -= size;
      outpw_d(num, port, pos >> 6);
      outpb_d(num, port, pos);
      port += TMK_DataPort-TMK_AddrPort;
      if (inpw_d(num, port) != 0)
        break;
      size <<= 1;
    }
    while ((size & 0x8000) == 0);
    size >>= 10;
    __tmkRAMSize[num] = size;
    size <<= 4;
    --size;
    __bcMaxBase[num] = (size <= DRV_MAX_BASE) ? size : DRV_MAX_BASE;
    size >>= 5;
    __rtMaxPage[num] = size;
    __wOutDelay[num] = save_odelay;
    __wInDelay[num] = save_idelay;
    break;
  case __TMKMPC:
    __tmkRAMSize[num] = 2;
    __bcMaxBase[num] = 31;
    __rtMaxPage[num] = 0;
    break;
#ifdef MRTX
  case __MRTX:
    __tmkRAMSize[num] = 8;
    __bcMaxBase[num] = 0;
    __rtMaxPage[num] = 0;
    __mrtNRT[num] = 4;
    if (__mrtMaxNRT > 0 && __mrtMaxNRT < __mrtNRT[num])
      __mrtNRT[num] = __mrtMaxNRT;
    break;
#endif
  case __TMKX:
    port = __tmkPortsAddr1[num] + TMK_AddrPort;
    outpw(port, 60);
    port += TMK_DataPort-TMK_AddrPort;
    outpw(port, 0);
    size = 0x0800;
    do
    {
      port += TMK_AddrPort-TMK_DataPort;
      pos = size + 60;
      outpw(port, pos);
      port += TMK_DataPort-TMK_AddrPort;
      outpw(port, pos);
      port += TMK_AddrPort-TMK_DataPort;
      pos -= size;
      outpw(port, pos);
      port += TMK_DataPort-TMK_AddrPort;
      if (inpw(port) != 0)
        break;
      size <<= 1;
    }
    while ((size & 0x8000) == 0);
    size >>= 10;
    __tmkRAMSize[num] = size;
    size <<= 4;
    --size;
#if DRV_MAX_BASE > 255
    __mtMaxBase[num] = 
#endif
    __bcMaxBase[num] = (size <= DRV_MAX_BASE) ? size : DRV_MAX_BASE;
    size >>= 5;
    __rtMaxPage[num] = size;
    break;
  case __TA:
    __tmkRAMSize[num] = 64;
    __bcMaxBase[num] = (1023 <= DRV_MAX_BASE) ? 1023 : DRV_MAX_BASE;
#if DRV_MAX_BASE > 255
    __mtMaxBase[num] = (511 <= DRV_MAX_BASE) ? 511 : DRV_MAX_BASE;
#endif
    __rtMaxPage[num] = 0;
    break;
#ifdef MRTA
  case __MRTA:
    __tmkRAMSize[num] = 256;
    __bcMaxBase[num] = 0;
    __rtMaxPage[num] = 0;
    __mrtNRT[num] = 31;
    if (__mrtMaxNRT > 0 && __mrtMaxNRT < __mrtNRT[num])
      __mrtNRT[num] = __mrtMaxNRT;
    break;
#endif
  }
  return 0;
}

#if defined(DOS) || defined(QNX4)

#ifdef DOS
#ifdef DOS32

#define LOCK_RAM 0x600
#define UNLOCK_RAM 0x601

#define GET_OLD_PM_VECTOR 1
#define SET_NEW_PM_VECTOR 2
#define GET_OLD_RM_VECTOR 4
#define SET_NEW_RM_VECTOR 8
#define ALLOCATE_RM_CALLBACK 16

int getvect_pm(unsigned intr, void (TYPIR __far **ProcA)(__CPPARGS))
{
  union REGS r;

  r.x.eax = 0x0204;
  r.h.bl = intr;
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  *ProcA = MK_FP(r.x.ecx, r.x.edx);
  return 0;
}

int setvect_pm(unsigned intr, void (TYPIR __far *Proc)(__CPPARGS))
{
  union REGS r;

  r.x.eax = 0x0205;
  r.h.bl = intr;
  r.x.ecx = FP_SEG(*Proc);
  r.x.edx = FP_OFF(*Proc);
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  return 0;
}

int getvect_rm(unsigned intr, U16 *rmProcA)
{
  union REGS r;

  r.x.eax = 0x0200;
  r.h.bl = intr;
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  rmProcA[0] = r.w.dx;
  rmProcA[1] = r.w.cx;
  return 0;
}

int setvect_rm(unsigned intr, U16 *rmProcA)
{
  union REGS r;

  r.x.eax = 0x0201;
  r.h.bl = intr;
  r.w.dx = rmProcA[0];
  r.w.cx = rmProcA[1];
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  return 0;
}

int alloc_rm_callback(void (TYPIR __far *ProcRM)(__CPPARGS), U16 *rmProcA)
{
  union REGS r;
  struct SREGS s;

  r.x.eax = 0x0303;
  s.ds = FP_SEG(*ProcRM);
  r.x.esi = FP_OFF(*ProcRM);
  s.es = FP_SEG(&__rmCallStruc);
  r.x.edi = FP_OFF(&__rmCallStruc);
  int386x(0x31, &r, &r, &s);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  rmProcA[0] = r.w.dx;
  rmProcA[1] = r.w.cx;
  return 0;
}

int unalloc_rm_callback(U16 *rmProcA)
{
  union REGS r;

  r.x.eax = 0x0304;
  r.w.dx = rmProcA[0];
  r.w.cx = rmProcA[1];
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return TMK_DPMI_ERROR;
  return 0;
}

#define DPMI_IRQ_OK (GET_OLD_PM_VECTOR | SET_NEW_PM_VECTOR | GET_OLD_RM_VECTOR | ALLOCATE_RM_CALLBACK | SET_NEW_RM_VECTOR)

int SetIrqAL(int num, unsigned irq)
{
  irq = IRQ2INT(irq);
  if (getvect_pm(irq, &tmkOldInt1[num]) == 0)
    __ftmkInt1[num] |= GET_OLD_PM_VECTOR;
  if ( setvect_pm(irq, tmkDrvInt1[num]) == 0)
    __ftmkInt1[num] |= SET_NEW_PM_VECTOR;
  if (getvect_rm(irq, rmtmkOldInt1[num]) == 0)
    __ftmkInt1[num] |= GET_OLD_RM_VECTOR;
  if (alloc_rm_callback(tmkDrvInt1RM[num], rmtmkDrvInt1[num]) == 0)
    __ftmkInt1[num] |= ALLOCATE_RM_CALLBACK;
  if (setvect_rm(irq, rmtmkDrvInt1[num]) == 0)
    __ftmkInt1[num] |= SET_NEW_RM_VECTOR;
  return (__ftmkInt1[num] == DPMI_IRQ_OK) ? 0 : TMK_DPMI_ERROR;
}

void ResetIrqAL(int num, unsigned irq)
{
  irq = IRQ2INT(irq);
  if (__ftmkInt1[num] & SET_NEW_PM_VECTOR)
  {
    setvect_pm(irq, tmkOldInt1[num]);
    __ftmkInt1[num] &= ~(GET_OLD_PM_VECTOR | SET_NEW_PM_VECTOR);
  }
  if (__ftmkInt1[num] & SET_NEW_RM_VECTOR)
  {
    setvect_rm(irq, rmtmkOldInt1[num]);
    __ftmkInt1[num] &= ~(GET_OLD_RM_VECTOR | SET_NEW_RM_VECTOR);
  }
  if (__ftmkInt1[num] & ALLOCATE_RM_CALLBACK)
  {
    unalloc_rm_callback(rmtmkDrvInt1[num]);
    __ftmkInt1[num] &= ~ALLOCATE_RM_CALLBACK;
  }
}
#else //notDOS32
int SetIrqAL(int num, unsigned irq)
{
  irq = IRQ2INT(irq);
  tmkOldInt1[num] = getvect(irq);
  setvect(irq, tmkDrvInt1[num]);
  return 0;
}
void ResetIrqAL(int num, unsigned irq)
{
  irq = IRQ2INT(irq);
  setvect(irq, tmkOldInt1[num]);
}
#endif //def DOS32
#endif //def DOS

int SetTmkIrqs(int num)
{
  U08 irq;
  int i;
  int res = 0;

  irq = __tmkIrq1[num];
  CHECK_IRQ(irq);
  for (i = 0; i < NTMK; ++i)
  {
    if (i == num)
      continue;
//    if (__tmkDrvType[i] == 0xFFFF)
//      continue;
    if (irq == __tmkIrq1[i])
    {
      for (i = 0; i < NTMK; ++i)
      {
//        if (__tmkDrvType[i] == 0xFFFF)
//          continue;
        if (irq != __tmkIrq1[i])
          continue;
        __tmkIrqShared[i] = 1;
      }
      return 0;
    }
  }
#ifdef DOS
  res = SetIrqAL(num, irq);
  if (!res)
    ENABLE_IRQ_AL(num, irq);
#endif //def DOS
#ifdef QNX4
#ifdef QNX4VME
  if (__vmeDev != -1)
    __tmkiid[num] = _VME_hint_attach(__vmeDev, __tmkIrqLevel[num], irq, tmkInterruptServiceRoutine, FP_SEG(&tmkError));
  else
    __tmkiid[num] = -1;
#else
  __tmkiid[num] = qnx_hint_attach(irq, tmkInterruptServiceRoutine, FP_SEG(&tmkError));
//  __tmkiid[num] = qnx_hint_attach(irq, tmkDrvInt1[num], FP_SEG(&tmkError));
#endif //def QNX4VME
  if (__tmkiid[num] == -1)
    res = TMK_BAD_IRQ;
#endif //def QNX4
  return res;
}

int ResetTmkIrqs(int num)
{
  U08 irq;
  int i;

  irq = __tmkIrq1[num];
  CHECK_IRQ(irq);
  for (i = 0; i < NTMK; ++i)
  {
    if (i == num)
      continue;
//    if (__tmkDrvType[i] == 0xFFFF)
//      continue;
    if (irq == __tmkIrq1[i])
      break;
  }
  if (i == NTMK)
  {
#ifdef DOS
    DISABLE_IRQ_AL(num, irq);
    ResetIrqAL(num, irq);
#endif// def DOS
#ifdef QNX4
    if (__tmkiid[num] != -1)
#ifdef QNX4VME
      if (__vmeDev != -1)
        _VME_hint_detach(__vmeDev, irq);
#else
      qnx_hint_detach(__tmkiid[num]);
#endif //def QNX4VME
    __tmkiid[num] = -1;
#endif //def QNX4
  }
  __tmkIrq1[num] = 0xFF;
  __tmkIrqShared[num] = 0;
  return 0;
}

#endif //def DOS

#ifdef WIN95
//;                ALIGN   4
//;SetTmkIrqs      PROC    NEAR USES edi
//;                cli
//;                jmp     STI_Ok
//;                movzx   eax, byte ptr __tmkIrq1[ebx];
//;                CHECK_IRQ STI1
//;STI1_IrqOk:     mov     edi, tmkDrvInt1Desc[ebx*2];
//;                //;mov     [edi.VID_IRQ_Number], ax
//;                //;mov     [edi.VID_Hw_Int_Ref], ebx
//;                //;VxDcall VPICD_Virtualize_IRQ
//;                jc      STI_VPICD_Err
//;                mov     tmkIRQ1Handle[ebx*2], eax
//;//;                call    ENABLE_IRQ_AL
//;                //;VxDcall VPICD_Physically_Unmask
//;  type = __tmkDrvType[ebx];
//;  switch (type)    STI, edi
//;STI_RTMK:
//;                movzx   eax, byte ptr tmkIrq2[ebx];
//;                CHECK_IRQ STI2
//;STI2_IrqOk:     mov     edi, tmkDrvInt2Desc[ebx*2];
//;                //;mov     [edi.VID_IRQ_Number], ax
//;                //;mov     [edi.VID_Hw_Int_Ref], ebx
//;                //;VxDcall VPICD_Virtualize_IRQ
//;                jc      STI_VPICD_Err
//;                mov     tmkIRQ2Handle[ebx*2], eax
//;//;                call    ENABLE_IRQ_AL
//;                //;VxDcall VPICD_Physically_Unmask
//;                jmp     STI_Ok
//;
//;STI_VPICD_Err:  USER_ERROR(TMK_BAD_IRQ
//;                jmp     STI_Exit
//;
//;STI_RTMK1:
//;STI_TMK400:
//;STI_RTMK400:
//;STI_TMKMPC:
//;STI_TMKX:
//;
//;STI_Ok:         xor     eax, eax
//;STI1_Exit:
//;STI2_Exit:
//;STI_Exit:       sti
//;    return;
//;SetTmkIrqs      ENDP
//;
//;                ALIGN   4
//;ResetTmkIrqs    PROC    NEAR USES edi
//;                jmp     RTI_Ok
//;                movzx   eax, byte ptr __tmkIrq1[ebx];
//;                CHECK_IRQ RTI1
//;RTI1_IrqOk:     mov     eax, tmkIRQ1Handle[ebx*2];
//;                or      eax, eax
//;                jz      RTI1_Ok
//;//;                call    DISABLE_IRQ_AL
//;                //;VxDcall VPICD_Physically_Mask
//;                //;VxDcall VPICD_Force_Default_Behavior
//;                mov     tmkIRQ1Handle[ebx*2], 0
//;RTI1_Ok:
//;  type = __tmkDrvType[ebx];
//;  switch (type)    RTI, edi
//;RTI_RTMK:
//;                movzx   eax, byte ptr tmkIrq2[ebx];
//;                CHECK_IRQ RTI2
//;RTI2_IrqOk:     mov     eax, tmkIRQ2Handle[ebx*2];
//;                or      eax, eax
//;                jz      RTI2_Ok
//;//;                call    DISABLE_IRQ_AL
//;                //;VxDcall VPICD_Physically_Mask
//;                //;VxDcall VPICD_Force_Default_Behavior
//;                mov     tmkIRQ2Handle[ebx*2], 0
//;RTI2_Ok:
//;
//;RTI_RTMK1:
//;RTI_TMK400:
//;RTI_RTMK400:
//;RTI_TMKMPC:
//;RTI_TMKX:
//;
//;RTI_Ok:         xor     eax, eax
//;RTI1_Exit:
//;RTI2_Exit:
//;RTI_Exit:
//;    return;
//;ResetTmkIrqs    ENDP
#endif //def WIN95

void DrvInitAll(void)
{
  int i;

  for (i = 0; i < NTMK; ++i)
  {
#ifdef DOS32
    __ftmkInt1[NTMK] = 0;
#endif //def DOS32
    __tmkTimeOut[i] = 0;
    __tmkTimerCtrl[i] = 0;
    __tmkHWVer[i] = 0;
//      fTmkEventSet[i] = 0;
    __wInDelay[i] = 1;
    __wOutDelay[i] = 1;
#if NRT > 0
    __mrtMinRT[i] = 0;
    __mrtNRT[i] = 1;
    __dmrtRT[i] = 0L;
    __dmrtBrc[i] = 0L;
#endif //NRT
    __tmkRAMInWork[i] = 0;
    __tmkRAMAddr[i] = 0;
    __tmkStarted[i] = 0;
    __bcBus[i] = 0;
    __bcMaxBase[i] = 0;
#if DRV_MAX_BASE > 255
    __mtMaxBase[i] = 0;
#endif
#if defined(DOS) || defined(QNX4)
    __tmkPci[i] = 0;
    __tmkMask21[i] = 0;
#ifndef CPU188
    __tmkMaskA1[i] = 0;
#endif //ndef CPU188
    __tmkIrq1[i] = 0xFF;
    __tmkIrqShared[i] = 0;
    __tmkIrqPort[i] = 0;
    __tmkIrqBit[i] = 0;
    tmkUserNormBC[i] = retfRLabel3;
    tmkUserExcBC[i] = retfRLabel3;
    tmkUserSigBC[i] = retfLabel1;
    tmkUserXBC[i] = retfRLabel2;
    tmkUserSigMT[i] = retfLabel1;
    tmkUserXMT[i] = retfRLabel2;
#endif// def DOS
#ifdef QNX4
    __tmkiid[i] = -1;
#endif //def QNX4
#ifdef QNX4VME
    __vmeDev = -1;
    __vmeWin = (void*) -1;
    __tmkIrqLevel[i] = 0;
#endif //def QNX4VME
  }
  for (i = 0; i < (NTMK + NRT); ++i)
  {
#ifdef STATIC_TMKNUM
    tmkError = 0;
#else
    tmkError[i] = 0;
#endif
    __amrtNumber[i] = 0;
    __tmkDrvType[i] = 0xFFFF;
    __tmkUserType[i] = 0xFFFF;
    __tmkMode[i] = 0xFFFF;
    __rtControls[i] = 0;
    __rtControls1[i] = 0;
    __rtAddress[i] = 0x00FF;
    __rtMaxPage[i] = 0;
    __rtMode[i] = 0;
    __rtSubAddr[i] = 0;
    __hm400Page[i] = 0;
#ifdef MRTA
    __hm400Page0[i] = 0;
    __hm400Page2[i] = 0;
#endif
    __rtEnableOnAddr[i] = 1;
    __FLAG_MODE_ON[i] = 0;
#if defined(DOS) || defined(QNX4)
    tmkUserCmdRT[i] = retfRLabel1;
    tmkUserErrRT[i] = retfRLabel1;
    tmkUserDataRT[i] = retfRLabel1;
#if NRT > 0
    adwTmkOptions[i] = MRT_READ_BRC_DATA | MRT_WRITE_BRC_DATA;
#endif //NRT
#endif
  }
#ifdef NMBCID
  for (i = 0; i < NMBCID; ++i)
  {
    __mbcAlloc[i] = 0;
  }
#endif //def NMBCID
}

void DrvInitTmk(int num, unsigned type)
{
  unsigned i;
  unsigned port;
#ifdef QNX4VME
  unsigned led;
#endif //def QNX4VME

  switch (type)
  {
  case __TMK400:
  case __RTMK400:
    port = __tmkPortsAddr1[num] + TMK_ResetPort;
    outpb_d(num, port, 0);
    break;
  case __TMKMPC:
    port = __tmkPortsAddr1[num] + TMKMPC_ResetPort;
    outpb_d(num, port, 0);
    break;
#ifdef MRTX
  case __MRTX:
//    break;
#endif
  case __TMKX:
    port = __tmkPortsAddr1[num] + TMK_ResetPort;
    GET_DIS_IRQ_SMP();
    outpw(port, 0);
    REST_IRQ_SMP();
#ifdef QNX4VME
    port += TMKXV_VectPort - TMK_ResetPort;
    outpw(port, (U16)__tmkIrq1[num]);
#endif //def QNX4VME
    break;
#ifdef MRTA
  case __MRTA:
//    break;
    for (i = 0; i <= 31; ++i)
      outpw(MRTA_SW(num), i << 11);
#endif
  case __TA:
    GET_DIS_IRQ_SMP();
    outpw(TA_RESET(num), 0);
#if NRT > 0
    __mrtLastBrcTxRT[num] = 0;
#endif
    outpw(TA_MODE1(num), TA_FIFO_RESET);
//    outpw(TA_MODE1(num), 0);
    REST_IRQ_SMP();
    port = TA_IRQ(num);
    for (i = 0; i < 16; ++i)
      __tmkHWVer[num] |= ((inpw(port) & 0x1000) >> 12) << i;
#ifdef QNX4VME
    port = __tmkPortsAddr1[num] + TAV_VectPort;
    led = ((__tmkIrqLevel[num] & 7) << 12);
//    if (__tmkIrq1[num] & 1)
//      led |= 0x0400;
//    else
//      led |= 0x0800;
    outpw(port, (U16)(__tmkIrq1[num] | led));
#endif //def QNX4VME
    break;
  }
}

U16 FARFN tmkgethwver(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __tmkHWVer[realnum];
}

#if NRT > 0
void mrtdefmaxnrt(int mrtMaxNrt)
{
  __mrtMaxNRT = mrtMaxNrt;
}
#endif //NRT

#ifdef DOS32
int DrvLockRam(unsigned lock_op);

void savees(void);
#pragma aux savees = \
"mov __SaveES, es" \
__parm [] __modify [];
#endif //def DOS32

#ifdef QNX4VME
int FARFN tmkconfig(int hTMK, U16 wType, U16 PortsAddr1, U16 PortsAddr2, U08 Irq1, U08 Irq2, char *pTmkName)
//tmkconfig(hTMK, wType, PortsAddr, DeviceNumber, IrqVect, IrqLev, pTmkName);
#else
int FARFN tmkconfig(int hTMK, U16 wType, U16 PortsAddr1, U16 PortsAddr2, U08 Irq1, U08 Irq2)
#endif //def QNX4VME
{
#ifndef STATIC_TMKNUM
  int __tmkNumber; // fake __tmkNumber
#endif
  int num;
  unsigned type;
  int res;
  unsigned irt, nrt;

  if (__tmkFirstTime)
  {
    __tmkFirstTime = 0;
    DrvInitAll();
  }
//;  CLRtmkError;
#ifdef DOS32
  ++__fTmkDeep;
  if (__fTmkDeep == 1)
  {
/*
                ASSUME  ds:_TEXT
                mov     SaveDS, ds
                mov     SaveES, es
                ASSUME  ds:_DATA
*/
    savees();
    if (DrvLockRam(LOCK_RAM))
    {
      res = USER_ERROR_R(TMK_DPMI_ERROR);
      goto err;
    }
  }
#endif //def DOS32
  num = hTMK;
  CHECK_TMK_REAL_NUMBER(num);
  if (__tmkDrvType[num] != 0xFFFF)
  {
    res = USER_ERROR_R(TMK_BAD_TYPE);
    goto err;
  }
  __tmkNumber = num;
  CLRtmkError;
  PUT_RealNum(num);
  __tmkMode[num] = UNDEFINED_MODE;
  type = wType;
  CHECK_TMK_TYPE_1(type); // special if QNX4VME
#if defined(DOS) || defined(QNX4)
  __tmkPci[num] = 0;
  switch (type)
  {
  case TMKXI:
  case TAI:
#ifdef MRTXI
  case MRTXI:
#endif
#ifdef MRTAI
  case MRTAI:
#endif
    __tmkPci[num] = 1;
    break;
  }
#endif //def DOS
  __tmkUserType[num] = type;
  type = __tmkUser2DrvType[type];
  __tmkDrvType[num] = type;
  __RT_DIS_MASK[type] = __RT_DIS_MASK0[__tmkUserType[num]];
  __RT_BRC_MASK[type] = __RT_BRC_MASK0[__tmkUserType[num]];
  __rtDisableMask[num] = __RT_DIS_MASK[type];
  __rtBRCMask[num] = __RT_BRC_MASK[type];
  __rtEnableOnAddr[num] = 1;
  __tmkPortsAddr1[num] = PortsAddr1;
  __tmkPortsAddr2[num] = PortsAddr2;
#ifdef WIN95
  __tmkIrq1[num] = Irq1;
#endif
#if defined(DOS) || defined(QNX4)
  __tmkIrq1[num] = Irq1;
  if (__tmkPci[num])
  {
    res = SetupPCI(num);
    if (res)
      goto err;
  }
#endif //def DOS
#ifdef QNX4VME
  __tmkIrqLevel[num] = Irq2;
  __vmeDev = _VME_device_attach(pTmkName, __tmkClassName);
  if (__vmeDev == -1)
    return USER_ERROR_R(errno);
  __vmeWin = _VME_window_attach(__vmeDev, _ACC_A16, PortsAddr1, 0x200);
  if (__vmeWin == (void*) -1)
    return USER_ERROR_R(errno);
  if (PortsAddr2 > 3)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  __tmkPortsAddr1[num] = PortsAddr2 << 7;
#endif //def QNX4VME
  DrvInitTmk(num, type);
  res = ExamTmkRAM(num, type);
  if (res)
    goto err;
#if defined(DOS) || defined(QNX4)
  res = SetTmkIrqs(num);
  if (res)
  {
    ResetTmkIrqs(num);
//    __tmkDrvType[num] = 0xFFFF;
    goto err;
  }
#endif //def DOS
#ifdef WIN95 //???
//;                call    SetTmkIrqs
//;                or      eax, eax
//;                jz        case Cont_1
//;                push    eax
//;                call    ResetTmkIrqs
//;                mov     __tmkDrvType[ebx], 0xFFFF
//;                pop     eax
//;                jmp       case Exit
#endif
  switch (type)
  {
  case __RTMK400:
    __rtControls1[num] = RTMK400_HBIT_MODE + RTMK400_BRCST_MODE;
    break;
#ifdef MRTX
  case __MRTX:
    __rtControls1[num] = TMKX_HBIT_MODE + TMKX_BRCST_MODE;
    __mrtCtrl0[num] = 0x1FF8;
    __mrtCtrl1[num] = 0x8000 + 0x1FF8;
    __mrtMask0[num] = 0x1FF8;
    __mrtMask1[num] = 0x1FF8;
    __rtControls[num] = TX_RT_DATA_INT_BLK;
    break;
#endif
  case __TMKX:
    __rtControls1[num] = TMKX_HBIT_MODE + TMKX_BRCST_MODE;
    break;
  case __TMK400:
  case __TMKMPC:
    break;
#ifdef MRTA
  case __MRTA:
#endif
  case __TA:
    __rtControls1[num] = TA_HBIT_MODE + TA_BRCST_MODE;
    __FLAG_MODE_ON[num] = 0;
//  ???
    break;
  }
  __amrtNumber[num] = num;
#if NRT > 0
  switch (type)
  {
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
#if defined(MRTX) || defined(MRTA)
    nrt = __mrtNRT[num];
    irt = __mrtMinRT[num];
    if (irt == 0)
    {
      irt = __tmkMaxNumber;
      __tmkMaxNumber += nrt;
      while (__tmkMaxNumber >= (NTMK + NRT))
      {
        --__tmkMaxNumber;
        --__mrtNRT[num];
        --nrt;
      }
      if (nrt <= 0)
      {
        res = USER_ERROR_R(TMK_BAD_NUMBER);
        goto err;
      }
      ++irt;
      __mrtMinRT[num] = irt;
    }
    __tmkNumber = irt;
    do
    {
      __amrtNumber[irt] = num;
      __tmkDrvType[irt] = type;
      __rtDisableMask[irt] = __RT_DIS_MASK[type];
      __rtBRCMask[irt] = __RT_BRC_MASK[type];
      __rtEnableOnAddr[irt] = 1;
#ifdef MRTX
      if (type == __MRTX)
        __hm400Page[irt] = (irt - __mrtMinRT[num]) << 11;
#endif
      ++irt;
    }
    while (--nrt != 0);
    break;
#endif
  }
#endif
#ifdef DOS
  DrvUnmaskTmk(num);
#endif //def DOS
  Irq2;
  return 0;
  err:
#ifdef DOS32
  if (__fTmkDeep != 0)
  {
    if (--__fTmkDeep == 0)
      DrvLockRam(UNLOCK_RAM);
  }
#endif //def DOS32
  return res;
}

#if NRT > 0 && defined(DOS)
U32 FARFN mrtconfig(int hTMK, U16 wType, U16 PortsAddr1, U16 PortsAddr2, U08 Irq1, U08 Irq2)
{
  int iRt, iRt0, nRt;
  int i;
  U16 aw0[30];
  U32 res;

  switch (wType)
  {
#ifdef MRTX
  case MRTX:
  case MRTXI:
#endif
#ifdef MRTA
  case MRTA:
  case MRTAI:
#endif
    if (tmkconfig(hTMK, wType, PortsAddr1, PortsAddr2, Irq1, Irq2))
    {
      res = 0L;
      break;
    }
    // tmkconfig selects first RT
    tmkselect(hTMK);
    if (bcreset())
    {
      res = 0L;
      break;
    }
    for (i = 0; i < 30; ++i)
      aw0[i] = 0;
    rtputflags(aw0, RT_RECEIVE, 1, 30);
    iRt0 = mrtgetrt0();
    nRt = mrtgetnrt();
    for (iRt = iRt0+nRt-1; iRt >= iRt0; --iRt)
    {
      tmkselect(iRt);
      rtreset();
      rtputflags(aw0, RT_RECEIVE, 1, 30);
    }
    tmkselect(iRt0); // ??? select first RT
    res = (((U32)nRt<<16) | (U32)iRt0);
    break;
  default:
    res = 0L;
    break;
  }
  return res;
}
#endif //NRT DOS

int FARFN tmkdone(int hTMK)
{
  int num;
  unsigned type;
  int ntmk;
  int irt, nrt;

#ifdef DOS
  CLRtmkError;
#endif
  if (__tmkFirstTime)
    return 0;
  num = hTMK;
  if (num != ALL_TMKS)
    ntmk = num;
  else
  {
    num = 0;
    ntmk = NTMK;
  }
  do
  {
    type = __tmkDrvType[num];
    if ((unsigned)type == 0xFFFF)
      continue;
//;    CHECK_TMK_TYPE(type);
#ifdef DOS
    DrvMaskTmk(num);
#endif
#if defined(DOS) || defined(QNX4)
    ResetTmkIrqs(num);
#endif //def DOSQNX4
#ifdef QNX4VME
    if (__vmeDev != -1 && __vmeWin != (void*) -1)
      _VME_window_detach(__vmeDev, _ACC_A16);
    __vmeWin = (void*) -1;
    if (__vmeDev != -1)
      _VME_device_detach(__vmeDev);
    __vmeDev = -1;
#endif //def QNX4VME
    __tmkDrvType[num] = 0xFFFF;
//    tmkUserType[num] = 0xFFFF;
#ifdef DOS32
    if (__fTmkDeep != 0)
    {
      if (--__fTmkDeep == 0)
        DrvLockRam(UNLOCK_RAM);
    }
#endif //def DOS32
    __rtDisableMask[num] = __RT_DIS_MASK[type];
    __rtBRCMask[num] = __RT_BRC_MASK[type];
    __rtEnableOnAddr[num] = 1;
#if NRT > 0
    switch (type)
    {
#ifdef MRTX
    case __MRTX:
      __rtControls1[num] = MRTX_HBIT_MODE+MRTX_BRCST_MODE;
      __mrtCtrl0[num] = 0x1FF8;
      __mrtCtrl1[num] = 0x8000+0x1FF8;
      __mrtMask0[num] = 0x1FF8;
      __mrtMask1[num] = 0x1FF8;
      __rtControls[num] = TX_RT_DATA_INT_BLK;
      irt = __mrtMinRT[num];
      nrt = __mrtNRT[num];
      while (nrt > 0)
      {
//;        __amrtNumber2[irt] = 0xFFFF;
        __tmkDrvType[irt] = 0xFFFF;
        __rtDisableMask[irt] = __RT_DIS_MASK[type];
        __rtBRCMask[irt] = __RT_BRC_MASK[type];
        __rtEnableOnAddr[irt] = 1;
        ++irt;
        --nrt;
      }
      break;
#endif
#ifdef MRTA
    case __MRTA:
      __rtControls1[num] = TA_HBIT_MODE+TA_BRCST_MODE;
      __FLAG_MODE_ON[num] = 0;
      irt = __mrtMinRT[num];
      nrt = __mrtNRT[num];
      while (nrt > 0)
      {
//;        __amrtNumber2[irt] = 0xFFFF;
        if (__rtAddress[irt] > 0 && __rtAddress[irt] < 31)
        {
          __dmrtRT[irt] &= ~(1L << __rtAddress[irt]);
          __dmrtBrc[irt] &= ~(1L << __rtAddress[irt]);
        }
        __rtAddress[irt] = 0x00FF;
        __tmkDrvType[irt] = 0xFFFF;
        __rtDisableMask[irt] = __RT_DIS_MASK[type];
        __rtBRCMask[irt] = __RT_BRC_MASK[type];
        __rtEnableOnAddr[irt] = 1;
        ++irt;
        --nrt;
      }
      break;
#endif
    }
#endif
  }
  while (++num < ntmk);
  return 0;
}

#ifdef DOS

#ifndef CPU188

void DrvMaskTmk(int num)
{
  outpb(0x21, inpb(0x21) | __tmkMask21[num]);
  outpb(0xA1, inpb(0xA1) | __tmkMaskA1[num]);
}

void DrvUnmaskTmk(int num)
{
  outpb(0x21, inpb(0x21) & ~__tmkMask21[num]);
  outpb(0xA1, inpb(0xA1) & ~__tmkMaskA1[num]);
}

#else //def CPU188

void DrvMaskTmk(int num)
{
  _AX = inpw(0xFF28) | __tmkMask21[num];
  outpb(0xFF28, _AL);
}

void DrvUnmaskTmk(int num)
{
  _AX = inpw(0xFF28) & ~__tmkMask21[num];
  outpb(0xFF28, _AL);
}

#endif //ndef CPU188

void FARFN tmkdefirq(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Irq)
{
  CLRtmkError;
#ifdef MASKTMKS
  U08 irq;
  irq = Irq;
  CHECK_IRQ(irq);
#ifndef CPU188
  if (irq < 8)
    tmkAllMask21 |= 1 << irq;
  else
    tmkAllMaskA1 |= 1 << (irq - 8);
#else //def CPU188
  if (irq < 5)
    tmkAllMask21 |= 1 << (irq + 4);
  else
    tmkAllMask21 |= 1 << (irq - 3);
#endif //ndef CPU188
#else
  Irq;
#endif //def MASKTMKS
  return;
}

void FARFN tmkundefirq(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 Irq)
{
  CLRtmkError;
#ifdef MASKTMKS
  U08 irq;
  irq = Irq;
  CHECK_IRQ(irq);
#ifndef CPU188
  if (irq < 8)
    tmkAllMask21 &= ~(1 << irq);
  else
    tmkAllMaskA1 &= ~(1 << (irq - 8));
#else //def CPU188
  if (irq < 5)
    tmkAllMask21 &= ~(1 << (irq + 4));
  else
    tmkAllMask21 &= ~(1 << (irq - 3));
#endif //ndef CPU188
#else
  Irq;
#endif //def MASKTMKS
  return;
}
#endif //def DOS

#if defined(DOS)
void FARFN bcrestore(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned type;
  unsigned port;

  num = __tmkInIrqNumber;
  type = __tmkDrvType[num];
  DrvBcDefBase(num, type, __bcSaveBase);
  __tmkRAMInWork[num] = __tmkSaveRAMInWork;
  if (__tmkSaveRAMInWork)
  {
    __tmkRAMAddr[num] = __tmkSaveRAMAddr;
    port = __tmkPortsAddr1[num] + TMK_AddrPort;
    switch (type)
    {
    case __TMKMPC:
      port += TMKMPC_AddrLPort-TMK_AddrPort;
    case __TMK400:
    case __RTMK400:
      outpb_d(num, port, __tmkSaveRAMAddr);
      break;
    case __TMKX:
#ifdef MRTX
    case __MRTX:
#endif
      outpw(port, __tmkSaveRAMAddr);
      break;
//    case __TA:
//    ???
//      break;
//    case __MRTA:
//    ???
//      break;
    }
  }
  __tmkNumber = __tmkSaveNumber;
  return;
}

void FARFN rtrestore(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned port;

  num = __tmkInIrqNumber;
  realnum = GET_RealNumOf(num);
  type = __tmkDrvType[num];
  DrvRtDefSubAddr(num, type, __rtSaveSubAddr);
  DrvRtWMode(num, type, __rtSaveMode);
  __tmkRAMInWork[realnum] = __tmkSaveRAMInWork;
  if (__tmkSaveRAMInWork)
  {
    __tmkRAMAddr[realnum] = __tmkSaveRAMAddr;
    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    switch (type)
    {
    case __TMKMPC:
      port += TMKMPC_AddrLPort-TMK_AddrPort;
    case __TMK400:
    case __RTMK400:
      outpb_d(realnum, port, __tmkSaveRAMAddr);
      break;
    case __TMKX:
#ifdef MRTX
    case __MRTX:
#endif
      outpw(port, __tmkSaveRAMAddr);
      break;
//    case __TA:
//    ???
//      break;
//    case __MRTA:
//    ???
//      break;
    }
  }
  __tmkNumber = __tmkSaveNumber;
  return;
}

void FARFN tmkrestore(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  __tmkInIrqNumber = __tmkNumber = __tmkSaveNumber;
  _disable();
#ifdef MASKTMKS
  DrvUnmaskTmks();
#endif //def MASKTMKS
  if (__tmkMode[__tmkNumber] & RT_MODE) // RT, MRT
    rtrestore();
  else
    bcrestore();
  return;
}

void FARFN tmksave(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num, realnum;

  num = __tmkNumber;
  realnum = GET_RealNum;
  __tmkSaveNumber = num;
  __tmkSaveRAMAddr = __tmkRAMAddr[realnum];
  __tmkSaveRAMInWork = __tmkRAMInWork[realnum];
  __tmkRAMInWork[realnum] = 0;
  if (__tmkMode[num] & RT_MODE) // RT, MRT
  {
    __rtSaveSubAddr = __rtSubAddr[num];
    __rtSaveMode = __rtMode[num];
  }
  else
  {
    __bcSaveBase = __bcBasePC[realnum];
  }
#ifdef MASKTMKS
  DrvMaskTmks();
#endif //def MASKTMKS
  return;
}

#ifdef QNX4
#define RETQNX4 return
#else
#define RETQNX4
#endif

#ifdef DOS32
void restes(void);
#pragma aux restes = \
"mov es, __SaveES" \
__parm [] __modify [];
#endif //def DOS32

#if NTMK > 0
RETIR TYPIR DrvTmk0Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(0);
}
#endif
#if NTMK > 1
RETIR TYPIR DrvTmk1Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(1);
}
#endif
#if NTMK > 2
RETIR TYPIR DrvTmk2Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(2);
}
#endif
#if NTMK > 3
RETIR TYPIR DrvTmk3Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(3);
}
#endif
#if NTMK > 4
RETIR TYPIR DrvTmk4Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(4);
}
#endif
#if NTMK > 5
RETIR TYPIR DrvTmk5Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(5);
}
#endif
#if NTMK > 6
RETIR TYPIR DrvTmk6Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(6);
}
#endif
#if NTMK > 7
RETIR TYPIR DrvTmk7Int1(__CPPARGS)
{
  RETQNX4 DrvTmksInt1(7);
}
#endif

#ifdef DOS32
//rmCallStruc.rm_IP == rmCallStruc+2aH
//rmCallStruc.rm_CS == rmCallStruc+2cH
//rmCallStruc.rm_Flags == rmCallStruc+20H
//rmCallStruc.rm_SP == rmCallStruc+2eH
void rmDrvTmksInt(void);
#pragma aux rmDrvTmksInt = \
" mov bx, ds" \
" mov ds, [esp+0xC]" /*original ds saved by Watcom*/ \
/*no need for cld here because Watcom places cld himself*/ \
/*" cld"*/ \
" lodsw" \
" mov es:[edi].0x2A, ax" \
" lodsw" \
" mov es:[edi].0x2C, ax" \
" lodsw" \
" mov es:[edi].0x20, ax" \
" add word ptr es:[edi].0x2E, 6" \
" mov ds, bx" \
__parm [] __modify [ebx esi];

#if NTMK > 0
RETIR TYPIR rmDrvTmk0Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(0);
}
#endif
#if NTMK > 1
RETIR TYPIR rmDrvTmk1Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(1);
}
#endif
#if NTMK > 2
RETIR TYPIR rmDrvTmk2Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(2);
}
#endif
#if NTMK > 3
RETIR TYPIR rmDrvTmk3Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(3);
}
#endif
#if NTMK > 4
RETIR TYPIR rmDrvTmk4Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(4);
}
#endif
#if NTMK > 5
RETIR TYPIR rmDrvTmk5Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(5);
}
#endif
#if NTMK > 6
RETIR TYPIR rmDrvTmk6Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(6);
}
#endif
#if NTMK > 7
RETIR TYPIR rmDrvTmk7Int1(__CPPARGS)
{
  rmDrvTmksInt();
  DrvTmksInt1(7);
}
#endif
#endif //def DOS32

#ifdef MASKTMKS
#define DRVMASKTMKS(num) { \
  if (!__tmkIrqShared[num]) \
  { \
    DrvMaskTmks; \
    DrvEndInt(num); \
  } \
}
#define DRVUNMASKTMKS(num) { \
  disable(); \
  if (__tmkIrqShared[num]) \
    DrvEndInt(num); \
  DrvUnMaskTmks; \
}
#else
#define DRVMASKTMKS
#define DRVUNMASKTMKS
#endif //def MASKTMKS

#ifdef QNX4
#define GETRES proxy = 
#define RETRES proxy
#else
#define GETRES
#define RETRES
#endif //def QNX4

RETIR IncDpcData(int num)
{
  pTListEvD pEvD;
  unsigned bcAW1, bcAW2;
#ifdef QNX4
  int proxy = 0;
#endif //def QNX4
#if NRT > 0
#define MAX_TMK_NUMBER (NTMK-1)
  int hTmk, hTmkI;
  pTListEvD pEvD0;
#ifndef DYNAMIC_TMKNUM
  int hTmkSave;
#endif
  unsigned short wSaveSA, wSaveLock;
  int fWriteSA;
  unsigned short wSA, nWords;
  int nInt, hTmk0, hTmk1, hTmkT;
  unsigned short wMask;
  int nInDpcType;
#endif //NRT

#ifndef DYNAMIC_TMKNUM
#define HTMK
#define HTMK__
#else
#define HTMK hTmk
#define HTMK__ hTmk, 
#endif

  pEvD = &(__aEvData[num]);
  switch (pEvD->wMode)
  {
  case BC_MODE:
    __bcSaveBase = __bcBasePC[num];
    switch (pEvD->nInt)
    {
    case 1:
      DrvBcDefBase(num, __tmkDrvType[num], __bcBaseBus[num]);
      DRVMASKTMKS;
      GETRES tmkUserNormBC[num](0, 0xFFFF, 0xFFFF);
      break;
    case 2:
      DrvBcDefBase(num, __tmkDrvType[num], __bcBaseBus[num]);
      bcAW1 = 0xFFFF;
      bcAW2 = 0xFFFF;
      if (__bcAW1Pos[num])
        bcAW1 = DrvBcPeek(num, __tmkDrvType[num], __bcAW1Pos[num]);
      if (__bcAW2Pos[num])
      {
        if (__bcAW1Pos[num])
          bcAW2 = DrvBcPeek(num, __tmkDrvType[num], __bcAW2Pos[num]);
        else
          bcAW1 = DrvBcPeek(num, __tmkDrvType[num], __bcAW2Pos[num]);
      }
      DRVMASKTMKS;
      GETRES tmkUserExcBC[num](pEvD->awEvData[0], bcAW1, bcAW2);
      break;
    case 3:
      DrvBcDefBase(num, __tmkDrvType[num], __bcBaseBus[num]);
      DRVMASKTMKS;
      GETRES tmkUserXBC[num](__bcBaseBus[num], pEvD->awEvData[0]);
      break;
    case 4:
      DRVMASKTMKS;
      GETRES tmkUserSigBC[num](pEvD->awEvData[0]);
      __tmkNumber = __tmkSaveNumber;
      break;
    }
    break;
  case RT_MODE:
    __rtSaveSubAddr = __rtSubAddr[num];
    __rtSaveMode = __rtMode[num];
    switch (pEvD->nInt)
    {
    case 1:
      DRVMASKTMKS;
      GETRES tmkUserCmdRT[num](pEvD->awEvData[0]);
      break;
    case 2:
      DRVMASKTMKS;
      GETRES tmkUserErrRT[num](pEvD->awEvData[0]);
      break;
    case 3:
      DRVMASKTMKS;
      GETRES tmkUserDataRT[num](pEvD->awEvData[0]);
      break;
    }
    break;
  case MT_MODE:
    __bcSaveBase = __bcBasePC[num];
    switch (pEvD->nInt)
    {
    case 3:
      DrvBcDefBase(num, __tmkDrvType[num], __bcBaseBus[num]);
      DRVMASKTMKS;
      GETRES tmkUserXMT[num](__bcBaseBus[num], pEvD->awEvData[0]);
      break;
    case 4:
      DRVMASKTMKS;
      GETRES tmkUserSigMT[num](pEvD->awEvData[0]);
      __tmkNumber = __tmkSaveNumber;
      break;
    }
    break;
#if NRT > 0
  case MRT_MODE:
// No DRVMASKTMKS because MASKTMKS should be undefined!
  #ifndef DYNAMIC_TMKNUM
    hTmkSave = tmkselected();
  #endif
    hTmk = num;
    if (hTmk <= MAX_TMK_NUMBER)
    {
      hTmkI = hTmk;
      nInDpcType = __tmkDrvType[num]; //MRTzI->__MRTz //aTmkConfig[hTmk].nType;
    }
    else
    {
      hTmkI = rt2mrt(hTmk);
      nInDpcType = __tmkDrvType[num]; //MRTzI->__MRTz //aTmkConfig[hTmkI].nType; 
    }

    if (hTmk <= MAX_TMK_NUMBER)// &&
//        (nInDpcType == MRTX || nInDpcType == MRTXI ||
//         nInDpcType == MRTA || nInDpcType == MRTAI))
    {
        {

          if (pEvD->nInt == 4)
          {
            hTmk0 = (int)(pEvD->awEvData[1]);
            hTmkT = hTmk0 >> 8; // transmitting RT (if exist)
            hTmk0 &= 0x00FF;    // min receiving RT
            hTmk1 = hTmk0 + (int)(pEvD->awEvData[2]) - 1; // max receiving RT
            wMask = 0xFFFF;

            if (pEvD->awEvData[0] & 0x4000)
            {
              nInt = 2;
              wMask = 0x47FF;
            }
            else if (((pEvD->awEvData[0] & 0x03E0) == 0x0000) ||
                     ((pEvD->awEvData[0] & 0x03E0) == 0x03E0))
            {
              nInt = 1;
              wMask = 0x041F;
            }
            else
            {
              nInt = 3;
              wMask = 0x07FF;
            }
            pEvD->awEvData[1] = (unsigned short)nInt;

            if (nInt == 3 && (adwTmkOptions[hTmk] & MRT_READ_BRC_DATA))
            {
              unsigned short wSavePage = 0xFFFF;
  #ifndef DYNAMIC_TMKNUM
              tmkselect(hTmk);
  #endif
              wSA = pEvD->awEvData[0] & 0x03E0;
              nWords = pEvD->awEvData[0] & 0x1F;
              if (nWords == 0)
                nWords = 32;
              if (nInDpcType == __MRTX) //(nInDpcType == MRTX || nInDpcType == MRTXI)
              {
                mrtdefbrcsubaddr0(HTMK); // save ???
                rtgetblk(HTMK__ 0, &(awBrcRxBuf[hTmk][wSA]), nWords);
                if (rtgetmode(HTMK) & RT_FLAG_MODE)
                {
                  rtdefsubaddr(HTMK__ RT_RECEIVE, wSA>>5);
                  rtclrflag(HTMK); // clrflag in mrt aka rt0
                    // the flag may be cleared later if rt0 is used
                    // (subj to optimize)
                    // but if rt0 isn't used we have to have the flag cleared!
                    // also the cleared flag is checked below before writing
                    // saved awBrcRxBuf data into rt0 (so if we do not clear 
                    // it here we need a workaround for rt0
                }
              }
              else //(nInDpcType == MRTA || nInDpcType == MRTAI)
              {
                wSavePage = rtgetpage(HTMK);
                mrtdefbrcpage(HTMK__ 0);
                rtdefsubaddr(HTMK__ RT_RECEIVE, wSA>>5);
                __rtgetblkmrta(HTMK__ 0, &(awBrcRxBuf[hTmk][wSA]), nWords);
                __rtgetblkmrta(HTMK__ 58, &(awBrcRxState[hTmk][wSA>>3]), 3);
                // rtgetblk does not accept position 58
//                if (rtgetmode(HTMK) & RT_FLAG_MODE)
//                {
//                  rtclrflag(HTMK);
                    // We may need clrflag in the mrt brc page to be able to 
                    // receive new brc data if the brc page uses flags. 
                    // Actually it could be better (but more complex) to 
                    // clear the flag when saved awBrcRxBuf data are written 
                    // into all RT pages.
                    // But now we do not use flags in the brc page at all so 
                    // it always can receive data.
//                }
                rtdefpage(HTMK__ wSavePage);
                wSavePage = 0xFFFF;
              }
              if (wSavePage != 0xFFFF)
                rtdefpage(HTMK__ wSavePage);
  #ifndef DYNAMIC_TMKNUM
              tmkselect(hTmkSave);
  #endif
            }

            while (hTmk0 <= hTmk1)
            {
              if (!__rtDisableMask[hTmk0] && (hTmk0 != hTmkT))
              {
                pEvD0 = &(__aEvData[hTmk0]);
                pEvD0->nInt = (nInt == 2) ? 2 : 4;
                pEvD0->wMode = (nInt == 2) ? RT_MODE : MRT_MODE; //RT_MODE;
                pEvD0->awEvData[0] = pEvD->awEvData[0] & wMask;
                pEvD0->awEvData[1] = (unsigned short)nInt;
                __tmkInIrqNumber = __tmkNumber = hTmk0;
                IncDpcData(hTmk0);
////                fTaskletSchedule = 1;
              }
              else
                __tmkNumber = __tmkSaveNumber;
              ++hTmk0;
            }
          } //if (pEvD->nInt == 4)
          else
          {
            hTmk0 = (int)(pEvD->awEvData[1]);
            pEvD0 = &(__aEvData[hTmk0]);
            pEvD0->nInt = pEvD->nInt;
            pEvD0->wMode = RT_MODE;
            pEvD0->awEvData[0] = pEvD->awEvData[0];
            __tmkInIrqNumber = __tmkNumber = hTmk0;
            IncDpcData(hTmk0);
////            fTaskletSchedule = 1;
          }
        }
    }

    if (hTmk > MAX_TMK_NUMBER)
//        && ((adwTmkOptions[hTmk] & MRT_WRITE_BRC_DATA) ||
//         (adwTmkOptions[hTmk] & RT_NO_BRC_IRQS)))
    {
      {

        if (pEvD->nInt == 4)
        {
//          if (adwTmkOptions[hTmk] & RT_NO_BRC_IRQS)
          pEvD->nInt = (int)(pEvD->awEvData[1]);
          pEvD->wMode = RT_MODE;
          if ((int)(pEvD->awEvData[1]) == 3 &&
              (adwTmkOptions[hTmk] & MRT_WRITE_BRC_DATA))
          {
            unsigned short wSavePage = 0xFFFF;
#ifndef DYNAMIC_TMKNUM
            tmkselect(hTmk);
#endif
            if (rtgetirqmode(HTMK) & RT_DATA_BL)
            {
              pEvD->nInt = 0;
            }

            if (nInDpcType == __MRTA) //(nInDpcType == MRTA || nInDpcType == MRTAI)
            {
              wSavePage = rtgetpage(HTMK);
              rtdefpage(HTMK__ 0);
            }
            wSaveSA = rtgetsubaddr(HTMK);  // how about saves of mrt data ???
            wSaveLock = rtgetlock(HTMK);
            wSA = (pEvD->awEvData[0] >> 5) & 0x1F;
            if (rtgetmode(HTMK) & RT_FLAG_MODE)
            {
              fWriteSA = !(rtgetflag(HTMK__ RT_RECEIVE, wSA) & RT_FLAG_MASK);
            }
            else
            {
              fWriteSA = !(wSaveLock == (wSA | 0x0800));
            }

            if (fWriteSA)
            {
              rtdefsubaddr(HTMK__ RT_RECEIVE, wSA);
              nWords = pEvD->awEvData[0] & 0x1F;
              if (nWords == 0)
                nWords = 32;
              if (nInDpcType == __MRTX) //(nInDpcType == MRTX || nInDpcType == MRTXI)
                rtputblk(HTMK__ 0, &(awBrcRxBuf[hTmkI][wSA<<5]), nWords);
              else // (nInDpcType == MRTA || nInDpcType == MRTAI)
              {
                __rtputblkmrta(HTMK__ 0, &(awBrcRxBuf[hTmkI][wSA<<5]), nWords);
                __rtputblkmrta(HTMK__ 58, &(awBrcRxState[hTmkI][wSA<<2]), 3);
                // rtputblk does not accept position 58
              }
            }
            else
            {
              pEvD->nInt = 0;
            }

            if (rtgetmode(HTMK) & RT_FLAG_MODE)
            {
              if (fWriteSA)
              {
                rtputflag(HTMK__ RT_RECEIVE, wSA, pEvD->awEvData[0] | RT_FLAG_MASK); // rtsetflag(HTMK);
              }
              rtdefsubaddr(HTMK__ wSaveSA, wSaveSA);
            }
            else
            {
              if (fWriteSA)
              {
                if (wSaveLock & 0x0800)
                  rtlock(HTMK__ wSaveLock, wSaveLock);
                else
                  rtdefsubaddr(HTMK__ wSaveSA, wSaveSA);
              }
            }
            if (wSavePage != 0xFFFF)
              rtdefpage(HTMK__ wSavePage);
#ifndef DYNAMIC_TMKNUM
            tmkselect(hTmkSave);
#endif
          }
          if (pEvD->nInt)
            IncDpcData(hTmk);
          else
            __tmkNumber = __tmkSaveNumber;
        }
      }
    }
    break;
#endif //NRT
  default: //UNDEFINED_MODE
    DRVMASKTMKS;
    __tmkNumber = __tmkSaveNumber;
    break;
  }
  __tmkSaveNumber = __tmkNumber; // track tmkselect in handlers 
  DRVUNMASKTMKS;
  return RETRES;
}

RETIR DrvTmksInt1(int num)
{
  int hTmk, hTmk0, hTmk1, nmin, nmax;
  int nInt;
  U16 wMask;
  pTListEvD pEvD, pEvD0;
  unsigned intr;
#ifdef MRTA
  unsigned long saved;
#else
  unsigned saved;
#endif
  U08 irq;
#ifdef QNX4
  int proxy = 0;
  IRQ_FLAGS;
#endif

#ifdef DOS32
  restes();
#endif //def DOS32
  __tmkSaveNumber = __tmkNumber;
  if (__tmkIrqShared[num])
  {
    nmin = 0;
    nmax = NTMK - 1;
  }
  else
  {
    nmin = nmax = num;
  }
  irq = __tmkIrq1[num];
  for (hTmk = nmin; hTmk <= nmax; ++hTmk)
  {
    if (irq != __tmkIrq1[hTmk])
      continue;
    if (__tmkIrqBit[hTmk] != 0 && (inpw(__tmkIrqPort[hTmk]) & __tmkIrqBit[hTmk]) == 0)
      continue;
    __tmkInIrqNumber = __tmkNumber = hTmk;
    __tmkSaveRAMAddr = __tmkRAMAddr[hTmk];
    __tmkSaveRAMInWork = __tmkRAMInWork[hTmk];
    __tmkRAMInWork[hTmk] = 0;
    saved = DIRQLTmkSave(hTmk);
#ifdef QNX4
    GET_DIS_IRQ();
#endif
    do
    {
      pEvD = &(__aEvData[hTmk]);

      intr = DIRQLTmksInt1(hTmk, pEvD);

      if (intr & TMK_INT_SAVED)
      {
        GETRES IncDpcData(hTmk);
//        fTaskletSchedule = 1;
      }
      if (intr & TMK_INT_OTHER) //TMK_INT_TIMER | TMK_INT_BUSJAM | TMK_INT_FIFO_OVF | TMK_INT_GEN1 | TMK_INT_GEN2...
      {
        pEvD = &(__aEvData[hTmk]);
        pEvD->nInt = 5;
        pEvD->wMode = UNDEFINED_MODE;
        pEvD->awEvData[0] = (unsigned short) (intr & TMK_INT_OTHER);
//        *((ULONG*)(&(pEvD->awEvData[1]))) = tmkgettimer(m_Unit+hTmk);
        pEvD->awEvData[1] = 0;
        pEvD->awEvData[2] = 0;
        GETRES IncDpcData(hTmk);
//        fTaskletSchedule = 1;
      }
    }
    while (intr & TMK_INT_MORE);
#ifdef QNX4
    REST_IRQ();
#endif
    DIRQLTmkRestore(hTmk, saved);
  }
#ifdef DOS
#ifndef MASKTMKS
  DrvEndInt(num);
#endif
#endif
  return RETRES;
}
#endif //def DOS

#ifdef MRTA
unsigned long DIRQLTmkSave(int hTMK)
{
  if (__tmkDrvType[hTMK] == __TA)
    return (unsigned long)inpw(__tmkPortsAddr1[hTMK] + TMK_AddrPort);
  else if (__tmkDrvType[hTMK] == __MRTA)
    return (((unsigned long)inpw(MRTA_ADDR2(hTMK)) << 16) + (unsigned long)inpw(TA_ADDR(hTMK)));
  else
    return 0L;
}

void DIRQLTmkRestore(int hTMK, unsigned long Saved)
{

  if (__tmkDrvType[hTMK] == __TA)
    outpw(__tmkPortsAddr1[hTMK] + TMK_AddrPort, Saved);
  else if (__tmkDrvType[hTMK] == __MRTA)
  {
    outpw(MRTA_ADDR2(hTMK), (unsigned)(Saved >> 16));
    outpw(TA_ADDR(hTMK), (unsigned)Saved);
  }
  return;
}
#else
unsigned DIRQLTmkSave(int hTMK)
{
  if (__tmkDrvType[hTMK] == __TA)
    return inpw(__tmkPortsAddr1[hTMK] + TMK_AddrPort);
  else
    return 0;
}

void DIRQLTmkRestore(int hTMK, unsigned Saved)
{
  if (__tmkDrvType[hTMK] == __TA)
    outpw(__tmkPortsAddr1[hTMK] + TMK_AddrPort, Saved);
  return;
}
#endif //def MRTA

unsigned DIRQLTmksInt1(int hTMK, void *pEvData)
{
  int num;
  unsigned type;
  unsigned t;
  U16 FARDT *pevd;
  unsigned mode;
  unsigned port;
  unsigned intd=0;
  unsigned nint=0;
  unsigned intm;
  unsigned irt, nrt;
  unsigned pos, data;
  unsigned res;
  unsigned vp;
  unsigned msgBase;
  unsigned msgSW;
  unsigned state58, state59, state60;

  num = hTMK;
  pevd = (U16 FARDT*)pEvData;
  type = __tmkDrvType[num];
  mode = __tmkMode[num];
  res = TMK_INT_SAVED;
  if (type == __TA)
  {
    vp = inpw(TA_IRQ(num)) & 0xE7FF;
    if (vp & 0x4000) // Timer Overflow
    {
//      if (!ts_TimerLoop[num])
//      if ((inpw(TA_TIMCR(num) & 0x2000) == 0)
      if (inpw(TA_RESET(num)) & 0x4000)
      {
        outpw(TA_TIMCR(num), inpw(TA_TIMCR(num) & ~0x0400));
//        outpw(TA_TIMCR(num), ts_TimerCtrl[num] & ~TS_TIMER_ENABLE);
//        outpw(TA_TIMCR(num), ts_TimerCtrl[num]);
      }
    }
    if (vp & 0x2000) // Bus Self Jum
    {
      if (inpw(TA_RESET(num)) & 0x2000)
      {
//      ts_IrqEn = 0;
        outpw(TA_MODE1(num), inpw(TA_MODE1(num))&~0x0400);
      }
    }
    if (vp & 0x8000) // FIFO Not Empty == TMK_INT_MORE
    {
      msgBase = vp & 0x03FF;
      outpw(TA_ADDR(num), (msgBase << 6) + 58);
      msgSW = inpw(TA_DATA(num));
//      if ((ts_msgMode[ts_iLastInt] == TS_MT_MODE) && ((msgSW & 0x000F) == 0x000F))
//        ts_msgMode[ts_iLastInt] = TS_MW_MODE;
//      msgTH = inpw(TA_DATA(num));
//      msgTL = inpw(TA_DATA(num));
//      msgCW = inpw(TA_DATA(num));
//      if (mode != BC_MODE)
//      {
//        determine actual base mode here
//      }
      pevd[2] = mode; //pevd->4 = mode; //[esi+4] //; wMode
      pevd[5] = 0; //pevd->10 = 0; //[esi+10]  //; bc.wAW2, others.dummy
      switch (mode)
      {
      case BC_MODE:
        if ((msgSW & 0x0200) == 0)
        {
          intd = inpw(TA_MSGA(num)); // maybe __bcLinkBaseN?
          intd &= 0x03FF;          //; bcx.wBase
          nint = 4;                //; nInt = 4
        }
        else
        {
          __tmkStarted[num] = 0;
          intd = msgSW & 0x0007;   //; bcx.wResultX
          if (msgSW & 0x0030)
            intd |= 0x0008;
          if (__bcXStart[num] & 1)
          {
            pevd[4] = msgBase; //pevd->8 = (intd >> 6) & __bcMaxBase[num]; //[esi+8] //; bcx.wBase
  //;          __bcBaseBus[num] = intd;
            nint = 3;                //; nInt = 3
          }
          else
          {
            intd = __bcExt2StdResult[intd]; //; bc.wResult
            nint = 1;                //; nInt = 1
            if (intd)
            {
              ++nint;                   //; nInt = 2
              pevd[5] = 1; //pevd->10 = 1; //[esi+10] //; bc.wAW2
            }
          }
        }
        break;
      case RT_MODE:
        intd = msgSW;
        nint = 2;               //; nInt = 2

        if (__tmkHWVer[num] < 9)
        {
          if (intd & 0x2000)
            intd = (intd & 0x1FFF) | 0x4000;   //; rt.wStatus
        }
        if (intd & 0x4000)
        {
          intd &= 0x47FF; //0x5FFF;
        }
        else if ((intd & 0x03E0) == 0 || (intd & 0x03E0) == 0x03E0)
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
          switch (intd)
          {
          case 0x011: //syn w data
          case 0x014: //sel tx shd
          case 0x015: //ovr sel tx shd
            outpw(TA_ADDR(num), (msgBase << 6));
            __rtRxDataCmd[num][intd-0x11] = inpw(TA_DATA(num));
            break;
          }
        }
        else
        {
          ++nint;                   //; nInt = 3
          intd &= 0x07FF;            //; rt.wCmd
        }
        break;
      case MT_MODE:
//          asm db 0cch;
        if ((__mtCW[num] & 0x10) == 0 || //CX_STOP
           ((__mtCW[num] & 0x20) == 0 && (msgSW & 0x37))) //CX_INT
        {
//          asm db 0cch;
          outpw(TA_MODE1(num), inpw(TA_MODE1(num)) & 0xFFF7); //mtstop
          __tmkStarted[num] = 0;
          pevd[4] = msgBase; //pevd->8 = intd; //[esi+8]//; mt.wBase
          intd = (msgSW & 0x0007) |
                ((msgSW & 0x03C0) << 4) |
                ((msgSW & 0x0008) << 12);   //; mt.wResultX
          if (msgSW & 0x0030)
            intd |= 0x0008;
          nint = 3;                //; nInt = 3
        }
        else
        {
#if DRV_MAX_BASE > 255
          if (__bcLinkCCN[num][msgBase] & 0x8000) //CX_SIG
          {
            intd = __bcLinkBaseN[num][msgBase];
//            intd &= __bcMaxBase[num];  //; mt.wBase
            nint = 4;                //; nInt = 4
          }
#else
          if (__bcLinkWN[num][msgBase] & 0x8000) //CX_SIG
          {
            intd = (__bcLinkWN[num][msgBase] >> 6) & __bcMaxBase[num];
//            intd &= __bcMaxBase[num];  //; mt.wBase
            nint = 4;                //; nInt = 4
          }
#endif
          else
          {
            nint = 0;
            intd = 0;
            res = 0; // no TMK_INT_SAVED
          }
#if DRV_MAX_BASE > 255
          __mtCW[num] = __bcLinkCCN[num][msgBase];
#else
          __mtCW[num] = __bcLinkWN[num][msgBase];
#endif
        }
        break;
      }
    }
    else
    {
      nint = 0;
      intd = 0;
      res = 0; // no TMK_INT_SAVED
    }
//    res |= (vp & 0x0640);
    res |= (vp & 0xE400);
  }
#ifdef MRTA
  else if (type == __MRTA)
  {
    vp = inpw(TA_IRQ(num)) & 0xEFFF;
    if (vp & 0x4000) // Timer Overflow
    {
//      if (!ts_TimerLoop[num])
//      if ((inpw(TA_TIMCR(num) & 0x2000) == 0)
      if (inpw(TA_RESET(num)) & 0x4000)
      {
        outpw(TA_TIMCR(num), inpw(TA_TIMCR(num) & ~0x0400));
//        outpw(TA_TIMCR(num), ts_TimerCtrl[num] & ~TS_TIMER_ENABLE);
//        outpw(TA_TIMCR(num), ts_TimerCtrl[num]);
      }
    }
    if (vp & 0x2000) // Bus Self Jum
    {
      if (inpw(TA_RESET(num)) & 0x2000)
      {
//      ts_IrqEn = 0;
        outpw(TA_MODE1(num), inpw(TA_MODE1(num))&~0x0400);
      }
    }
    if (vp & 0x8000) // FIFO Not Empty == TMK_INT_MORE
    {
      msgBase = vp & 0x0FFF;
      outpw(MRTA_ADDR2(num), msgBase >> 10);
      outpw(TA_ADDR(num), (msgBase << 6) + 58);
      msgSW = inpw(TA_DATA(num));
      pevd[2] = mode; //pevd->4 = mode; //[esi+4] //; wMode
      pevd[5] = 0; //pevd->10 = 0; //[esi+10]  //; bc.wAW2, others.dummy

//      case MRT_MODE:
      intd = msgSW;

//      if (__tmkHWVer[num] < 9)
      {
        if (intd & 0x2000)
          intd = (intd & 0x1FFF) | 0x4000;   //; rt.wStatus
      }

      if ((intd & 0x1000) == 0 || ((intd & 0x1400) == 0x1400 && !((intd & 0x03E0) == 0 || (intd & 0x03E0) == 0x03E0)))       //; !brc ?
      {
        __mrtLastBrcTxRT[num] = 0;
        pevd[4] = __mrtA2RT[num][(msgBase & 0x7FF) >> 6]; //pevd->8 = (intd >> 12) + __mrtMinRT[num];//[esi+8] //; rt number
        nint = 2;                //; nInt = 2
        if (intd & 0x4000)
        {
          intd &= 0x47FF; //0x5FFF;
        }
        else if ((intd & 0x03E0) == 0 || (intd & 0x03E0) == 0x03E0)
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
          switch (intd)
          {
          case 0x011: //syn w data
          case 0x014: //sel tx shd
          case 0x015: //ovr sel tx shd
            outpw(TA_ADDR(num), (msgBase << 6));
            __rtRxDataCmd[pevd[4]][intd-0x11] = inpw(TA_DATA(num));
            break;
          }
        }
        else
        {
          if ((intd & 0x1400) == 0x1400)
            __mrtLastBrcTxRT[num] = pevd[4];
          ++nint;                   //; nInt = 3
          intd &= 0x07FF;            //; rt.wCmd
        }
      }
      else // brc
      {
        pevd[4] = __mrtMinRT[num]; //pevd->8 = __mrtMinRT[num];//[esi+8]      //; rt number
        pevd[5] = __mrtNRT[num]; //pevd->10 = __mrtNRT[num];//[esi+10]
        intm = intd;
        if ((intm & 0x4000) == 0)  //; !error ?
        {
          if ((intm & 0x03E0) == 0 || (intm & 0x03E0) == 0x03E0) // cmd ?
          {
            intm &= 0x041F;
//            outpw(MRTA_ADDR2(num), msgBase >> 10); //already written
//            outpw(TA_ADDR(num), msgBase << 6);
            pos = MRTA_BRC_PAGE >> 4;
//            outpw(MRTA_ADDR2(num), MRTA_BRC_PAGE >> 4); //already written
            data = 0; // suppress untitialized data warning
            switch (intm)
            {
            case 0x011: //syn w data
            case 0x014: //sel tx shd
            case 0x015: //ovr sel tx shd
              outpw(TA_ADDR(num), (U16)(MRTA_BRC_PAGE << 12) | (0x1F << 6));
              data = inpw(TA_DATA(num));
              break;
            }
            outpw(TA_ADDR(num), (U16)(MRTA_BRC_PAGE << 12) | (0x1F << 6) | 58);
            state58 = inpw(TA_DATA(num));
            state59 = inpw(TA_DATA(num));
            state60 = inpw(TA_DATA(num));
//!!! move to DPC? !!!
            irt = __mrtMinRT[num];
            nrt = 0;
            do
            {
              if ((__rtDisableMask[irt] & 0xFFFF) == 0)
              {
                if (pos != (__hm400Page0[irt] >> 4))
                {
                  pos = __hm400Page0[irt] >> 4;
                  outpw(MRTA_ADDR2(num), pos);
                }
                switch (intm)
                {
                case 0x011: //syn w data
                case 0x014: //sel tx shd
                case 0x015: //ovr sel tx shd
//                  outpw(TA_ADDR(num), (__hm400Page0[irt] << 12) | (0x1F << 6));
//                  outpw(TA_DATA(num), data); // actually, this write is not used further
                  __rtRxDataCmd[irt][intm-0x11] = data;
                  break;
                }
                outpw(TA_ADDR(num), (__hm400Page0[irt] << 12) | (0x1F << 6) | 58);
                outpw(TA_DATA(num), state58);
                outpw(TA_DATA(num), state59);
                outpw(TA_DATA(num), state60);
              }
              ++irt;
            }
            while (++nrt <= __mrtNRT[num]);
          }
          else // !cmd
          {
            pevd[4] |= __mrtLastBrcTxRT[num] << 8; // this rt on mrt is brc transmitter
          }
        }
        nint = 4;                //; nInt = 4
        __mrtLastBrcTxRT[num] = 0;
      }
    }
    else
    {
      nint = 0;
      intd = 0;
      res = 0; // no TMK_INT_SAVED
    }
    res |= (vp & 0xE000);
  }
#endif //def MRTA
  else
  {
    pevd[2] = mode; //pevd->4 = mode; //[esi+4] //; wMode
    pevd[5] = 0; //pevd->10 = 0; //[esi+10]  //; bc.wAW2, others.dummy
    switch (mode)
    {
    case MT_MODE:
//      switch (type)
//      {
//      case __TMKX:
        port = __tmkPortsAddr1[num] + TMK_ModePort;
        intd = inpw(port);
        if ((intd & 0x8000) == 0)
        {
          intd >>= 6;
          intd &= __bcMaxBase[num];  //; mt.wBase
          nint = 4;                //; nInt = 4
        }
        else
        {
          __tmkStarted[num] = 0;
          intd >>= 6;
          intd &= __bcMaxBase[num];  //;
  //;        __bcBaseBus[num] = intd //;
          pevd[4] = intd; //pevd->8 = intd; //[esi+8]//; mt.wBase
          port += TMK_StatePort-TMK_ModePort;
          intd = inpw(port);       //; mt.wResultX
          nint = 3;                //; nInt = 3
        }
//        break;
//      }
      break;
    case BC_MODE:
      switch (type)
      {
      case __TMK400:
        t = __wInDelay[num];
        while (--t != 0);
        __tmkStarted[num] = 0;
        port = __tmkPortsAddr2[num];
        intm = inpb_d(num, port);
        port = __tmkPortsAddr1[num] + TMK_StatePort;
        intd = inpw_d(num, port);
        intd >>= 10;              //; bc.wResult
        nint = 1;                 //; nInt = 1
        if (intm & TMK400_INT1_MASK)
        {
          ++nint;                 //; nInt = 2
          pevd[5] = 1; //pevd->10 = 1; //[esi+10]//; bc.wAW2
        }
        break;
      case __RTMK400:
        t = __wInDelay[num];
        while (--t != 0);
        __tmkStarted[num] = 0;
        port = __tmkPortsAddr1[num];
        intm = inpb_d(num, port);
        port += TMK_StatePort;
        intd = inpw_d(num, port);
        intd >>= 10;              //; bc.wResult
        nint = 1;                 //; nInt = 1
        if (intm & RTMK400_INT1_MASK)
        {
          ++nint;                 //; nInt = 2
          pevd[5] = 1; //pevd->10 = 1; //[esi+10]//; bc.wAW2
        }
        break;
      case __TMKMPC:
        t = __wInDelay[num];
        while (--t != 0);
        __tmkStarted[num] = 0;
        port = __tmkPortsAddr1[num] + TMKMPC_StateLPort;
        intd = inpb_d(num, port);
        intd &= 0x003F;           //; bc.wResult
        nint = 1;                 //; nInt = 1
        if (intd)
        {
          ++nint;                 //; nInt = 2
          pevd[5] = 1; //pevd->10 = 1; //[esi+10]//; bc.wAW2
        }
        break;
      case __TMKX:
        port = __tmkPortsAddr1[num] + TMK_ModePort;
        intd = inpw(port);
        if ((intd & 0x8000) == 0)
        {
          intd >>= 6;
          intd &= __bcMaxBase[num];    //; bcx.wBase
          nint = 4;                //; nInt = 4
        }
        else
        {
          __tmkStarted[num] = 0;
          port += TMK_StatePort-TMK_ModePort;
          intd = inpw(port);
          if (__bcXStart[num] & 1)
          {
            pevd[4] = (intd >> 6) & __bcMaxBase[num]; //pevd->8 = (intd >> 6) & __bcMaxBase[num]; //[esi+8] //; bcx.wBase
  //;          __bcBaseBus[num] = intd;
            intd &= 0x003F;            //; bcx.wResultX
            nint = 3;                //; nInt = 3
          }
          else
          {
            intd = __bcExt2StdResult[intd & 0xF] | ((intd << 10) & 0xC000);
                                     //; bc.wResult
            nint = 1;                //; nInt = 1
            if (intd)
            {
              ++nint;                   //; nInt = 2
              pevd[5] = 1; //pevd->10 = 1; //[esi+10] //; bc.wAW2
            }
          }
        }
        break;
      }
      break;
    case RT_MODE:
      switch (type)
      {
      case __TMK400:
        t = __wInDelay[num];
        while (--t != 0);
        port = __tmkPortsAddr2[num];
        intm = inpb_d(num, port);
        port = __tmkPortsAddr1[num] + TMK_StatePort;       //; TMK_CtrlPort;
        intd = inpw_d(num, port);
        outpw_d(num, port, __rtAddress[num] | RT_CLEAR_INT);
        outpw_d(num, port, __rtAddress[num] | __rtMode[num]);//; don't mask because irq
        nint = 2;                //; nInt = 2
        if (intm & TMK400_INT1_MASK)
          intd &= 0x5FFF;            //; rt.wStatus
        else
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
        }
        break;
      case __RTMK400:
        t = __wInDelay[num];
        while (--t != 0);
        port = __tmkPortsAddr1[num];
        intm = inpb_d(num, port);
        port += TMK_StatePort;      //;TMK_CtrlPort;
        intd = inpw_d(num, port);
        outpw_d(num, port, RT_CLEAR_INT);
        outpw_d(num, port, __rtMode[num]);
        nint = 2;                //; nInt = 2
        if (intm & RTMK400_INT1_MASK)
          intd &= 0x5FFF;            //; rt.wStatus
        else
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
        }
        break;
      case __TMKMPC:
        t = __wInDelay[num];
        while (--t != 0);
        port = __tmkPortsAddr1[num] + TMKMPC_StateLPort;   //; TMKMPC_CtrlLPort;
        intd = inpw_d(num, port);
        ++port;
        outpb(port, RT_CLEAR_INT >> 8);
        --port;
        outpb_d(num, port, RT_CLEAR_INT);
        mode = __rtMode[num];
        ++port;
        outpb(port, mode >> 8);
        --port;
        outpb_d(num, port, mode);
        nint = 2;                //; nInt = 2
        if (intd & TMKMPC_INT1_MASK)
          intd &= 0x5FFF;            //; rt.wStatus
        else
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
        }
        break;
      case __TMKX:
        port = __tmkPortsAddr1[num] + TMK_StatePort;       //; TMK_CtrlPort;
        intd = inpw(port);
        do
        {
          intm = intd;
          intd = inpw(port);
        }
        while (intm != intd);
        outpw(port, __rtControls1[num] | (RT_CLEAR_INT >> 5));
        outpw(port, __rtControls1[num] | (RT_CLEAR_INT >> 5));
        outpw(port, __rtControls1[num]); //; don't mask because irq
        nint = 2;               //; nInt = 2
        if (intd & 0x7000)
          intd &= 0x7FFF;           //; rt.wStatus
        else if ((intd & 0x03E0) == 0 || (intd & 0x03E0) == 0x03E0)
        {
          --nint;                   //; nInt = 1
          intd &= 0x041F;            //; rt.wCmd
        }
        else
        {
          ++nint;                   //; nInt = 3
          intd &= 0x07FF;            //; rt.wCmd
        }
        break;
      }
      break;
#if NRT > 0
    case MRT_MODE:
  //;    switch (type)
  //;    {
#ifdef MRTX
  //;    case __MRTX:
        port = __tmkPortsAddr1[num] + TMK_StatePort;       //; MRT_ModePort;
        intd = inpw(port);
        do
        {
          intm = intd;
          intd = inpw(port);
        }
        while (intm != intd);
        outpw(port, __rtControls1[num] | (RT_CLEAR_INT >> 5));
        outpw(port, __rtControls1[num] | (RT_CLEAR_INT >> 5));
        outpw(port, __rtControls1[num]);//; don't mask because irq
        if ((intd & 0x8000) == 0)       //; !brc ?
        {
          pevd[4] = (intd >> 12) + __mrtMinRT[num]; //pevd->8 = (intd >> 12) + __mrtMinRT[num];//[esi+8] //; rt number
          nint = 2;                //; nInt = 2
          if ((intd & 0x4000) == 0)
          {
            if ((intd & 0x03E0) == 0 || (intd & 0x03E0) == 0x03E0)
            {
              --nint;                   //; nInt = 1
              intd &= 0x041F;            //; rt.wCmd
            }
            else
            {
              ++nint;                   //; nInt = 3
              intd &= 0x07FF;            //; rt.wCmd
            }
          }
        }
        else
        {
          pevd[4] = __mrtMinRT[num]; //pevd->8 = __mrtMinRT[num];//[esi+8]      //; rt number
          pevd[5] = __mrtNRT[num]; //pevd->10 = __mrtNRT[num];//[esi+10]
          intm = intd;
  //;                  push    __tmkNumber
          PUSH_RealNum;
  //;                  push    tmkRealNumber
          PUT_RealNum(num);
  //;                  mov     tmkRealNumber, num
          irt = __mrtMinRT[num];
          nrt = 1;
          do
          {
            if ((__rtDisableMask[++irt] & 0xFFFF) == 0)
            {
  //;                mov     __tmkNumber, irt
              DrvRtPokeIrqMRTX(irt, 0x07F2, intm | 0xF800); //; last cmd register
            }
          }
          while  (++nrt < __mrtNRT[num]);
          if ((intm & 0x4000) == 0)  //; !error ?
          {
            if ((intm & 0x03E0) == 0 || (intm & 0x03E0) == 0x03E0) // cmd ?
            {
              intm &= 0x041F;
              if (intm == 0x11 || intm == 0x14 || intm == 0x15)
              {
                irt = __mrtMinRT[num];
                nrt = 1;
                pos = intm | 0x07E0;
  //;                mov     __tmkNumber2, irt
                data = DrvRtPeekIrqMRTX(irt, pos);
                do
                {
                  if ((__rtDisableMask[++irt] & 0xFFFF) == 0)
                  {
  //;                mov     __tmkNumber2, irt
                    DrvRtPokeIrqMRTX(irt, pos, data);
                  }
                }
                while (++nrt < __mrtNRT[num]);
              }
            }
          }
          POP_RealNum;
      //;                pop     tmkRealNumber2
      //;                pop     __tmkNumber2
          nint = 4;                //; nInt = 4
        }
  //;      break;
#endif
  //;    }
      break;
#endif
    default:
  //    case BC_MRTX:
  //    case RT_MRTX:
  //;    case MT_TMK400:
  //;    case MT_RTMK400:
  //;    case MT_TMKMPC:
  //;    case MT_MRTX:
  //;    case MR_TMK400:
  //;    case MR_RTMK400:
  //;    case MR_TMKMPC:
  //;    case MR_TMKX:
      port = __tmkPortsAddr1[num];
      nint = 0;
      intd = 0;
      res = 0; // no TMK_INT_SAVED
      switch (type)
      {
      case __TMKMPC:
        port += TMKMPC_ResetPort;
        outpb(port, 0);
        break;
      case __TMK400:
      case __RTMK400:
      case __TMKX:
#ifdef MRTX
      case __MRTX:
#endif
      case __TA:
#ifdef MRTA
      case __MRTA:
#endif
        port += TMK_ResetPort;
        outpw(port, 0);
        break;
      }
      break;
    }
  }
  pevd[0] = nint; //pevd->0 = nint; //[esi]  //; nInt
  pevd[1] = 0;
  pevd[3] = intd; //pevd->6 = intd; //[esi+6]//; look at tmkgetevd OutBuf[3];
  return res;
}

void DpcIExcBC(int hTMK, void *pEvData)
{
  int num;
  unsigned type;
  unsigned pos;
  unsigned base;
  unsigned basepc;
  U16 FARDT *pevd;
  unsigned bcAW1, bcAW2;

  num = hTMK;
  if (__tmkMode[num] != BC_MODE)
    return;
  pevd = (U16 FARDT*)pEvData;
  basepc = __bcBasePC[num];
  base = __bcBaseBus[num];
  type = __tmkDrvType[num];
  DrvBcDefBase(num, type, base);
  bcAW1 = 0xFFFF;
  bcAW2 = 0xFFFF;
  pos = __bcAW1Pos[num];
  if (pos == 0)
    pos = __bcAW2Pos[num];
  if (pos != 0)
  {
    bcAW1 = DrvBcPeek(num, type, pos);
    if (pos != __bcAW2Pos[num])
    {
      pos = __bcAW2Pos[num];
      if (pos != 0)
        bcAW2 = DrvBcPeek(num, type, pos);
    }
  }
//  pos = __bcAW1Pos[num];
//  if (pos != 0)
//    bcAW1[num] = DrvBcPeek(num, type, pos);
//  pos = __bcAW2Pos[num];
//  if (pos != 0)
//  {
//    if (__bcAW1Pos[num] == 0)
//      bcAW1[num] = DrvBcPeek(num, type, pos);
//    else
//      bcAW2[num] = DrvBcPeek(num, type, pos);
//  }
  if (basepc != base)
    DrvBcDefBase(num, type, basepc);
  pevd[4] = bcAW1; //pevd->8 = bcAW1[num];
  pevd[5] = bcAW2; //pevd->10 = bcAW2[num];
  return;
}

#ifdef NMBCID

U16 FARFN mbcalloc()
{
  unsigned i;
  for (i = 0; i < NMBCID; ++i)
  {
    if (__mbcAlloc[i] == 0)
    {
      __mbcAlloc[i] = 1;
      return (U16)i;
    }
  }
  return MBC_ALLOC_FAIL;
}

int FARFN mbcfree(U16 mbcId)
{
  if (mbcId >= NMBCID || __mbcAlloc[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  __mbcAlloc[mbcId] = 0;
  return 0;
}

int FARFN mbcinit(U16 mbcId)
{
  if (mbcId >= NMBCID || __mbcAlloc[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  __mbci[mbcId] = 0;
  return 0;
}

int FARFN mbcpreparex(
#ifndef STATIC_TMKNUM
        int __tmkNumber, 
#endif
        U16 mbcId, U16 bcBase, U16 bcCtrlCode, U16 mbcDelay)
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned code;
  unsigned base;
  int i;
  IRQ_FLAGS;

  if (mbcId >= NMBCID || __mbcAlloc[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE(realnum, BC_MODE);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  type = __tmkDrvType[realnum];
  CHECK_TMK_TYPE(type);
  base = bcBase;
  CHECK_BCMT_BASE_BX(realnum, base);
  for (i = 0; i < __mbci[mbcId]; ++i)
  {
    if (__mbcTmkN[mbcId][i] == realnum)
      break;
  }
  __mbcTmkN[mbcId][i] = realnum;
  __mbcBase[mbcId][i] = base;
  switch (type)
  {
  case __TMK400:
  case __RTMK400:
  case __TMKMPC:
#ifdef MRTX
  case __MRTX:
#endif
#ifdef MRTA
  case __MRTA:
#endif
    return USER_ERROR(BC_BAD_FUNC);
  case __TMKX:
    port = __tmkPortsAddr1[realnum] + TMK_CtrlPort;
    base <<= 6;
    __mbcPort0[mbcId][i] = 0;
    __mbcData0[mbcId][i] = 0;
    __mbcPort[mbcId][i] = port;
    __mbcData[mbcId][i] = base | code;
    if (i == __mbci[mbcId])
      ++__mbci[mbcId];
    break;
  case __TA:
    {
      unsigned ContrW = 0x1D1F;
      unsigned code1;

      if (((code&0xf) == DATA_RT_RT) || ((code&0xf) == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      if (code & CX_BUS_B)
        ContrW |= 0x0080;
      if (code & CX_CONT)
        ContrW |= 0x2000;
#if DRV_MAX_BASE < 256
      code1 = __bcLinkWN[realnum][base];
#else
      code1 = __bcLinkCCN[realnum][base];
#endif
      if (code1 & CX_SIG)
        ContrW |= 0x8000;

      GET_DIS_IRQ_SMP();
      outpw(TA_ADDR(realnum), (base<<6) | 61);
      outpw(TA_DATA(realnum), ContrW);
      outpw(TA_DATA(realnum), mbcDelay);
      REST_IRQ_SMP();
#ifdef DRV_EMULATE_FIRST_CX_SIG
//emulation is through special base 0x3ff
//also it could be a good (or bad) idea to block irq output 
//and further poll it until it occurs
      if (code & CX_SIG)
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_ADDR(realnum), (0x03FF<<6) | 61);
        port = TA_DATA(realnum);
        outpw(port, 0xA020);
        outpw(port, 0);
        outpw(port, base);
        REST_IRQ_SMP();
        base = 0x03FF;
      }
#endif //def DRV_EMULATE_FIRST_CX_SIG
      __mbcPort0[mbcId][i] = TA_MSGA(realnum);
      __mbcData0[mbcId][i] = base & 0x03FF;
      __mbcPort[mbcId][i] = TA_MODE2(realnum);
      __mbcData[mbcId][i] = __bcControls1[realnum] | TA_BC_START;
      if (i == __mbci[mbcId])
        ++__mbci[mbcId];
    }
    break;
  }
  return 0;
 
}

int FARFN mbcstartx(U16 mbcId)
{
  int realnum;
  unsigned type;
  unsigned port;
  register int i, ni;
#ifndef STATIC_TMKNUM
  int __tmkNumber;
#endif
  IRQ_FLAGS;

  if (mbcId >= NMBCID || __mbcAlloc[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  ni = (int)__mbci[mbcId];
  for (i = 0; i < ni; ++i)
  {
    realnum = __mbcTmkN[mbcId][i];
    __tmkNumber = realnum;
    CLRtmkError;
    type = __tmkDrvType[realnum];
//    CHECK_TMK_TYPE(type);
    __bcBaseBus[realnum] = __mbcBase[mbcId][i];
    __bcXStart[realnum] = 1;
    switch (type)
    {
    case __TMK400:
    case __RTMK400:
    case __TMKMPC:
#ifdef MRTX
    case __MRTX:
#endif
#ifdef MRTA
    case __MRTA:
#endif
      return USER_ERROR(BC_BAD_FUNC);
    case __TMKX:
      if (__tmkStarted[realnum])
      {
        port = __tmkPortsAddr1[realnum] + TMK_ResetPort;
        GET_DIS_IRQ_SMP();
        outpw(port, 0);
        REST_IRQ_SMP();
        port += TMK_ModePort-TMK_ResetPort;
        outpw(port, __bcControls[realnum]);
      }
      __tmkStarted[realnum] = 1;
      break;
    case __TA:
      if (__tmkStarted[realnum])
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_RESET(realnum), 0);
        outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
        outpw(TA_MODE1(realnum), __bcControls[realnum]);
        REST_IRQ_SMP();
        outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      }
      __tmkStarted[realnum] = 1;
      break;
    }
  }
  for (i = 0; i < ni; ++i)
  {
    if ((port = __mbcPort0[mbcId][i]) != 0)
      outpw(port, __mbcData0[mbcId][i]);
  }
  GET_DIS_IRQ();
  for (i = 0; i < ni; ++i)
  {
    outpw(__mbcPort[mbcId][i], __mbcData[mbcId][i]);
  }
  REST_IRQ();
  return 0;
}

#endif //def NBCID

#ifdef DOS32
void tmkOpenDPMI()
{
  union REGS r;

  r.x.eax = 0x0400;
  int386(0x31, &r, &r);
  __tmkPic20Base = r.h.dh;
  __tmkPicA0Base = r.h.dl;
}

int __DataEnd = 0;
int DrvCodeEnd();

int DrvLockRam1(int lock_op, unsigned long addr, unsigned long len)
{
  union REGS r;

  r.x.eax = lock_op;
  r.x.ecx = addr & 0xFFFF;
  r.x.ebx = (addr >> 16) & 0xFFFF;
  r.x.edi = len & 0xFFFF;
  r.x.esi = (len >> 16) & 0xFFFF;
  int386(0x31, &r, &r);
  return (r.x.cflag) ? TMK_DPMI_ERROR : 0;
}

int DrvLockRam(int lock_op)
{
  unsigned long data1_addr, data1_len, data2_addr, data2_len, code_addr, code_len;
  int res = 0;

// 16-bit driver data??? Real order should be checked in .OBJ and .MAP!!!
  data1_addr = FP_OFF(&__bcCmdWN);
  data1_len = FP_OFF(&__pciBusDevFun) + sizeof(__pciBusDevFun) - data1_addr;
// 32-bit and 8-bit driver data???
  data2_addr = FP_OFF(&__DataBegin);
  data2_len = FP_OFF(&__DataEnd) + sizeof(__DataEnd) - data2_addr;
// driver code
  code_addr = FP_OFF(&DrvCodeBegin);
  code_len = FP_OFF(&DrvCodeEnd) - code_addr;
  res |= DrvLockRam1(lock_op, data1_addr, data1_len);
  res |= DrvLockRam1(lock_op, data2_addr, data2_len);
  res |= DrvLockRam1(lock_op, code_addr, code_len);
  return res;
}

int DrvCodeEnd()
{
  return 0x5678;
}
#endif //def DOS32

/*
#ifdef WIN95

//;eax - IRQHandle, ebx - VMHandle, edx - Ref_Data = hTMK*2
//;BeginProc       DrvTmksInt1, High_Freq
        PUBLIC  DrvTmksInt1
                ALIGN   4
DrvTmksInt1     PROC    C,
                hTMKU32
//;                push    esi
//;                push    eax
//;                mov     ebx, edx
                push    ebx
                push    ecx
                push    edx
                push    edi
                push    esi
                mov     edx, hTMK
                mov     ebx, edx
                mov     eax, __tmkNumber2
                mov     __tmkSaveNumber, eax
  type = __tmkDrvType[ebx];
                mov     esi, edi
                mov     __tmkInIrqNumber, ebx
                mov     __tmkNumber2, ebx
                movzx   eax, __tmkMode[ebx];
                cmp     eax, BC_MODE
                je      I1BC
                cmp     eax, RT_MODE
                je      I1RT
I1MT:
                movzx   eax, __bcBasePC[ebx];
                mov     __bcSaveBase, ax
//;  switch (type)    I1MT, esi
//;I1MT_TMK400:
//;I1MT_RTMK400:
//;I1MT_TMKMPC:
I1MT_TMKX:
    port = __tmkPortsAddr1[realnum];
    port += TMK_ModePort;
    = inpw(port);
//;     = inpw_d(port);
                test    eax, 8000h
                jnz     I1MT_TMKX_1
                shr     eax, 6
                and     eax, 0FFh
                jmp     ISigMT
I1MT_TMKX_1:    add     edx, TMK_StatePort-TMK_ModePort;
    = inpw(port);
//;     = inpw_d(port);
                movzx   edx, ax
                shr     edx, 6
                and     edx, 0FFh
                mov     __bcBaseBus[ebx], dx
                and     eax, 003Fh
                mov     bcResult[ebx], ax
                jmp     IXMT
I1BC:
                movzx   eax, __bcBasePC[ebx];
                mov     __bcSaveBase, ax
  switch (type)    I1BC, esi
I1BC_TMK400:
                movzx   edx, __tmkPortsAddr2[ebx];
  al = inpb_d(port);
                movzx   ecx, al
    port = __tmkPortsAddr1[realnum];
    port += TMK_StatePort;
     = inpw_d(port);
                shr     eax, 10
                mov     bcResult[ebx], ax
                test    ecx, TMK400_INT1_MASK
                jz      INormBC
                jmp     IExcBC
I1BC_RTMK400:
    port = __tmkPortsAddr1[realnum];
  al = inpb_d(port);
                movzx   ecx, al
    port += TMK_StatePort;
     = inpw_d(port);
                shr     eax, 10
                mov     bcResult[ebx], ax
                test    ecx, RTMK400_INT1_MASK
                jz      INormBC
                jmp     IExcBC
I1BC_TMKMPC:
    port = __tmkPortsAddr1[realnum];
    port += TMKMPC_StateLPort;
  al = inpb_d(port);
                and     eax, 003Fh        //;xor     ah, ah
                mov     bcResult[ebx], ax
                                          //;or      al, al
                jz      INormBC
                jmp     IExcBC
I1BC_TMKX:
    port = __tmkPortsAddr1[realnum];
    port += TMK_ModePort;
    = inpw(port);
//;     = inpw_d(port);
                test    eax, 8000h
                jnz     I1BC_TMKX_1
                shr     eax, 6
                and     eax, 0FFh
                jmp     ISigBC
I1BC_TMKX_1:    add     edx, TMK_StatePort-TMK_ModePort;
    = inpw(port);
//;     = inpw_d(port);
                mov     edx, eax
                test    __bcXStart[ebx], 1
                jz      I1BC_TMKX_Std
I1BC_TMKX_Ext:  shr     edx, 6
                and     edx, 0FFh
                mov     __bcBaseBus[ebx], dx
                and     eax, 003Fh
                mov     bcResult[ebx], ax
                jmp     IXBC
I1BC_TMKX_Std:  mov     esi, eax
                and     esi, 000Fh
                movzx   eax, __bcExt2StdResult[esi];
                shl     edx, 10
                and     edx, 0C000h
                or      eax, edx
                mov     bcResult[ebx], ax
                jz      INormBC
                jmp     IExcBC

I1RT:           movzx   eax, __rtSubAddr[ebx];
                mov     __rtSaveSubAddr, ax
                movzx   eax, __rtMode[ebx];
                mov     __rtSaveMode, ax
  switch (type)    I1RT, esi
I1RT_TMK400:
                movzx   edx, __tmkPortsAddr2[ebx];
  al = inpb_d(port);
                movzx   ecx, al
    port = __tmkPortsAddr1[realnum];
    port += TMK_StatePort;       //; TMK_CtrlPort;
     = inpw_d(port);
                mov     rtStatus, ax
                mov     eax, RT_CLEAR_INT
                or      ax, __rtAddress[ebx];
    outpw_d(port, ax);
                movzx   eax, __rtMode[ebx];
                or      ax, __rtAddress[ebx];
    outpw_d(port, ax);
                test    ecx, TMK400_INT1_MASK
                jz      ICmdRT
                mov     edx, 5FFFh
                jmp     IBadRT
I1RT_RTMK400:
    port = __tmkPortsAddr1[realnum];
  al = inpb_d(port);
                movzx   ecx, al
    port += TMK_StatePort;      //;TMK_CtrlPort;
     = inpw_d(port);
                mov     rtStatus, ax
                mov     eax, RT_CLEAR_INT
    outpw_d(port, ax);
                movzx   eax, __rtMode[ebx];
    outpw_d(port, ax);
                test    ecx, RTMK400_INT1_MASK
                jz      ICmdRT
                mov     edx, 5FFFh
                jmp     IBadRT
I1RT_TMKMPC:
    port = __tmkPortsAddr1[realnum];
    port += TMKMPC_StateLPort;   //; TMKMPC_CtrlLPort;
  al = inpb_d(port);
                movzx   ecx, al
                inc     edx
  al = inpb_d(port);
                mov     ch, al
                mov     al, HIGH RT_CLEAR_INT
    outpb_d(port, al);
                dec     edx
                mov     al, LOW RT_CLEAR_INT
    outpb_d(port, al);
                movzx   eax, __rtMode[ebx];
                xchg    al, ah
                inc     edx
    outpb_d(port, al);
                mov     al, ah
                dec     edx
    outpb_d(port, al);
                mov     rtStatus, cx
                test    ecx, TMKMPC_INT1_MASK
                jz      ICmdRT
                mov     edx, 5FFFh
                jmp     IBadRT
I1RT_TMKX:
    port = __tmkPortsAddr1[realnum];
    port += TMK_StatePort;       //; TMK_CtrlPort;
    = inpw(port);
//;     = inpw_d(port);
                movzx   ecx, ax
                mov     rtStatus, ax
                mov     eax, RT_CLEAR_INT SHR 5
                or      ax, __rtControls1[ebx];
    outpw(port, ax);
//;    outpw_d(port, ax);
                movzx   eax, __rtMode[ebx];
                or      ax, __rtControls1[ebx];
    outpw(port, ax);
//;    outpw_d(port, ax);
                test    ecx, 7000h
                jnz     I1RT_TMKX_1
                and     ecx, 03E0h
                jz      ICmdRT
                xor     ecx, 03E0h
                jz      ICmdRT
                jmp     IDataRT
I1RT_TMKX_1:    mov     edx, 7FFFh
                jmp     IBadRT

INormBC:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 1 //; IntNum
                mov     word ptr [eax+4], BC_MODE
                movzx   edx, bcResult[ebx];
                mov     [eax+6], dx
                jmp     DrvIntExit
IExcBC:
                movzx   eax, __bcBaseBus[ebx];
                call    DrvBcDefBase
IExc_AW1:       mov     bcAW1[ebx], 0xFFFF
                mov     bcAW2[ebx], 0xFFFF
                movzx   eax, __bcAW1Pos[ebx];
                mov     esi, eax
                or      eax, eax
                jz      IExc_AW2
                call    DrvBcPeek
                mov     bcAW1[ebx], ax
IExc_AW2:       movzx   eax, __bcAW2Pos[ebx];
                or      eax, eax
                jz      IExcBC_User
                call    DrvBcPeek
                or      esi, esi
                jnz     IExc_AW2_2
IExc_AW2_1:     mov     bcAW1[ebx], ax
                jmp     IExcBC_User
IExc_AW2_2:     mov     bcAW2[ebx], ax
IExcBC_User:
                movzx   eax, __bcSaveBase
                call    DrvBcDefBase
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 2 //; IntNum
                mov     word ptr [eax+4], BC_MODE
                movzx   edx, bcResult[ebx];
                mov     word ptr [eax+6], dx
                movzx   edx, bcAW1[ebx];
                mov     word ptr [eax+8], dx
                movzx   edx, bcAW2[ebx];
                mov     word ptr [eax+10], dx
                jmp     DrvIntExit
ICmdRT:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 1 //; IntNum
                mov     word ptr [eax+4], RT_MODE
                movzx   edx, rtStatus
                and     edx, 41Fh
                mov     [eax+6], dx
                jmp     DrvIntExit
IBadRT:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 2 //; IntNum
                mov     word ptr [eax+4], RT_MODE
                and     dx, rtStatus
                mov     [eax+6], dx
                jmp     DrvIntExit
IDataRT:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 3 //; IntNum
                mov     word ptr [eax+4], RT_MODE
                movzx   edx, rtStatus
                and     edx, 7FFh
                mov     [eax+6], dx
                jmp     DrvIntExit
IXBC:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 3 //; IntNum
                mov     word ptr [eax+4], BC_MODE
                movzx   edx, bcResult[ebx];
                mov     [eax+6], dx
                movzx   edx, __bcBaseBus[ebx];
                mov     [eax+8], dx
                jmp     DrvIntExit
IXMT:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 3 //; IntNum
                mov     word ptr [eax+4], MT_MODE
                movzx   edx, bcResult[ebx];
                mov     [eax+6], dx
                movzx   edx, __bcBaseBus[ebx];
                mov     [eax+8], dx
                jmp     DrvIntExit
ISigBC:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     edx, eax
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 4 //; IntNum
                mov     word ptr [eax+4], BC_MODE
                mov     [eax+6], dx
                jmp     DrvIntExit
ISigMT:
//;                SYN_LPT
                cmp     hTmkEvent[ebx*2], 0
                jz      DrvNoTmkEvent
                mov     edx, eax
                mov     esi, _ahlEvData[ebx*2];
                //;VMMcall List_Allocate
                jc      ErrListAlloc
                //;VMMcall List_Attach_Tail
                mov     dword ptr [eax], 4 //; IntNum
                mov     word ptr [eax+4], MT_MODE
                mov     [eax+6], dx
                jmp     DrvIntExit
ErrListAlloc:
                mov     _tmkListErr[ebx*2], 1
DrvIntExit:
//;                mov     esi, OFFSET32 TmkIntCallback
                mov     esi, OFFSET TmkIntCallback
                mov     edx, ebx
                push    ebx
//;                //;VMMcall Schedule_Global_Event
                //;mov     eax, High_Pri_Device_Boost
//;                mov     ebx, hCurVM[ebx*2];
                mov     ebx, _hSysVM
//;                mov     ebx, hSysVM
                //;mov     ecx, PEF_Always_Sched
                //;VMMcall Call_Priority_VM_Event
                pop     ebx
                mov     hIrqEvent[ebx*2], esi
DrvNoTmkEvent:
                mov     eax, __tmkSaveNumber
                mov     __tmkNumber2, eax
                pop     eax
                //;VxDcall VPICD_Phys_EOI
                pop     esi
                clc
    return;
//;EndProc         DrvTmksInt1
DrvTmksInt1     ENDP

//;BeginProc       TmkIntCallback, High_Freq
//;TmkIntCallback  PROC
TmkIntCallback:
                mov     hIrqEvent[edx*2], 0
//;                SYN_LPT
//;                mov     al, "0"
//;                //;VMMcall Out_Debug_Chr
//;                mov     eax, edx
//;                add     al, "0"
//;                //;VMMcall Out_Debug_Chr
                mov     ebx, edx
                mov     eax, hTmkEvent[edx*2];
                or      eax, eax
                jz      TIC_2
                cmp     fTmkEventSet[edx*2], 0
                je      TIC_1
                //;VxDcall _VWIN32_SetWin32Event
//;                mov     al, "1"
//;                //;VMMcall Out_Debug_Chr
//;                mov     al, 0Dh
//;                //;VMMcall Out_Debug_Chr
//;                mov     al, 0Ah
//;                //;VMMcall Out_Debug_Chr
    return;
TIC_1:          //;VxDcall _VWIN32_PulseWin32Event
//;                mov     al, "2"
//;                //;VMMcall Out_Debug_Chr
TIC_2:
//;                mov     al, "3"
//;                //;VMMcall Out_Debug_Chr
//;                mov     al, 0Dh
//;                //;VMMcall Out_Debug_Chr
//;                mov     al, 0Ah
//;                //;VMMcall Out_Debug_Chr
    return;
//;EndProc         TmkIntCallback
//;TmkIntCallback  ENDP


//;VXD_LOCKED_CODE_ENDS
//;_TEXT   ENDS

#endif //def WIN95
*/
