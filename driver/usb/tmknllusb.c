//;***************************************************************************;
//;*      Based onTMKNLL v7.02                                               *;
//;*      ELCUS, 1995, 2008.                                                 *;
//;***************************************************************************;
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
// - QNX6 support
// - mbcinit, mbcpreparex, mbcstartx, mbcalloc, mbcfree functions
// - MRTA bcreset fixed (possible fake tester interrupt)
// - wrong type bug fixed (in __RT_DIS_MASK, __RT_BRC_MASK)
//7.02
// - updated tmkisr.c for MRTA and QNX6
// - bcstop for TA cards fixed
// - tmktimer single loop fixed
#include "config.h"

#define WRITE_RG	0
#define READ_RG		1
#define WRITE_MEM	2
#define READ_MEM	3
#define MOD_RG_AND	4
#define MOD_RG_OR	5
#define MOD_MEM_AND	6
#define MOD_MEM_OR	7
#define WRITE_PARAM	10
#define READ_PARAM	11

#define PARAM_MODE		1
#define PARAM_BCBASEBUS		2
#define PARAM_BCXSTART		4
#define PARAM_MTCW		5
#define PARAM_BCAW1		6
#define PARAM_BCAW2		7
#define PARAM_BCAW1POS		8
#define PARAM_BCAW2POS		9
#define PARAM_BCLINKBASEN	10
#define PARAM_BCLINKCCN		11

#ifndef NOT_INCLUDE_DEFS
#include "tmkndefs.h"
#endif

#define VOID void

#define BOOL char

#ifdef __cplusplus
  #define __CPPARGS ...
  #define __INLINE inline
#else
  #define __CPPARGS
  #define __INLINE
#endif

#ifdef LINUX
#ifndef TMK1553B_NOCONFIGH
#include <linux/config.h>
#endif 
#ifdef CONFIG_SMP
#define __SMP__
#endif
#include <asm/io.h>
#endif //def LINUX

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

#define TA_BASE(i)     (__tmkPortsAddr1_usb[i])
#define TA_IRQ(i)      (__tmkPortsAddr1_usb[i]+2)
#define AdrRCod_Dec(i) (__tmkPortsAddr1_usb[i]+4)
#define TA_RESET(i)    (__tmkPortsAddr1_usb[i]+6)
#define TA_MODE1(i)    (__tmkPortsAddr1_usb[i]+8)
#define TA_ADDR(i)     (__tmkPortsAddr1_usb[i]+0xA)
#define TA_MODE2(i)    (__tmkPortsAddr1_usb[i]+0xC)
#define TA_DATA(i)     (__tmkPortsAddr1_usb[i]+0xE)
#define TA_TIMER1(i)   (__tmkPortsAddr1_usb[i]+0x10)
#define TA_TIMER2(i)   (__tmkPortsAddr1_usb[i]+0x12)
#define TA_RTA(i)      (__tmkPortsAddr1_usb[i]+0x14)
#define TA_TIMCR(i)    (__tmkPortsAddr1_usb[i]+0x18)
#define TA_LCW(i)      (__tmkPortsAddr1_usb[i]+0x1A)
#define TA_MSGA(i)     (__tmkPortsAddr1_usb[i]+0x1C)

#define MRTA_ADDR2(i)  (__tmkPortsAddr1_usb[i]+4)
#define MRTA_SW(i)     (__tmkPortsAddr1_usb[i]+0x16)

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

BOOL __tmkIsUserType_usb[TMK_MAX_TYPE+1+1] =
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

U16 __tmkUser2DrvType_usb[TMK_MAX_TYPE+1] =
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

BOOL __tmkFirstTime_usb = 1;

U16 __tmkTimerCtrl_usb[NTMK]; // DUP(0)
U16 __tmkTimeOut_usb[NTMK]; //DUP(0)

U16 __tmkHWVer_usb[NTMK]; // DUP(0)
#define AdrTab 1023

BOOL __FLAG_MODE_ON_usb[NTMK+NRT]; // DUP(0)

U16 __BC_RT_FLAG_usb[NTMK+NRT][32];
U16 __RT_BC_FLAG_usb[NTMK+NRT][32];

U16 __wInDelay_usb[NTMK]; // DUP(1)
U16 __wOutDelay_usb[NTMK]; // DUP(1)
int __tmkMaxNumber_usb = NTMK-1;

#ifdef STATIC_TMKNUM
int tmkError_usb = 0;
#else
int tmkError_usb[NTMK+NRT]; // DUP(0)
#endif

#ifdef STATIC_TMKNUM
int volatile __tmkNumber_usb = 0;
#endif
int __amrtNumber_usb[NTMK+NRT]; // DUP(0)

U16 __tmkPortsAddr1_usb[NTMK];
U16 __tmkPortsAddr2_usb[NTMK];
U16 __tmkDrvType_usb[NTMK+NRT]; // DUP(0xFFFF)
U16 __tmkUserType_usb[NTMK+NRT];
U16 __tmkMode_usb[NTMK+NRT]; // DUP(0xFFFF)
U16 __tmkRAMSize_usb[NTMK];

#if NRT > 0
int __mrtMinRT_usb[NTMK]; // DUP(0)
int __mrtNRT_usb[NTMK]; // DUP(1)
U16 __mrtCtrl0_usb[NTMK+NRT];
U16 __mrtCtrl1_usb[NTMK+NRT];
U16 __mrtMask0_usb[NTMK+NRT];
U16 __mrtMask1_usb[NTMK+NRT];
U32 __dmrtRT_usb[NTMK]; //DUP(0L)
U32 __dmrtBrc_usb[NTMK]; //DUP(0L)
U08 __mrtA2RT_usb[NTMK][32];
U16 __mrtLastBrcTxRT_usb[NTMK]; //DUP(0xFF)
#endif //NRT

BOOL __tmkRAMInWork_usb[NTMK]; // DUP(0)
unsigned __tmkRAMAddr_usb[NTMK]; // DUP(0)

BOOL __tmkStarted_usb[NTMK]; // DUP(0)

U16 __bcControls1_usb[NTMK];
U16 __bcControls_usb[NTMK];
U16 __bcBus_usb[NTMK]; // DUP(0)
U16 __bcMaxBase_usb[NTMK]; // DUP(0)
U16 __mtMaxBase_usb[NTMK]; // DUP(0)
U16 __bcBaseBus_usb[NTMK];
U16 __bcBasePC_usb[NTMK];
U16 __bcAW1Pos_usb[NTMK];
U16 __bcAW2Pos_usb[NTMK];
//U16 bcAW1[NTMK];
//U16 bcAW2[NTMK];
BOOL __bcXStart_usb[NTMK];

U08 __bcExt2StdResult_usb[] =
{
  0x00, 0x02, 0x08, 0x08,
  0x80, 0x01, 0x02, 0x20,
  0x04, 0x06, 0x0C, 0x0C,
  0x84, 0x05, 0x06, 0x24
};

void FARIR retfLabel_usb(void);

U16 __rtControls_usb[NTMK+NRT]; // DUP(0)
//MRTA(realnum) - MODE1
U16 __rtControls1_usb[NTMK+NRT]; // DUP(0)
//MRTA(realnum) - MODE2
//MRTA(num) - SW
U16 __rtPagePC_usb[NTMK+NRT];
U16 __rtPageBus_usb[NTMK+NRT];
U16 __rtAddress_usb[NTMK+NRT]; //DUP(00FFh)
//;rtMaskAddr      U16 [NTMK+NRT]; DUP(001Fh)
//;rtMaskBrc       U16 [NTMK+NRT]; //!!! DUP(0)
U16 __rtMaxPage_usb[NTMK+NRT]; // DUP(0)
U16 __rtMode_usb[NTMK+NRT]; // DUP(0)
U16 __rtSubAddr_usb[NTMK+NRT]; // DUP(0)
U16 __hm400Page_usb[NTMK+NRT]; // DUP(0)
#if NRT > 0
#ifdef MRTA
U16 __hm400Page2_usb[NTMK+NRT]; // DUP(0) current ADDR2
U16 __hm400Page0_usb[NTMK+NRT]; // DUP(0) starting page
#endif //def MRTA
#endif
U16 __rtDisableMask_usb[NTMK+NRT];
U16 __rtBRCMask_usb[NTMK+NRT];
BOOL __rtEnableOnAddr_usb[NTMK+NRT]; // DUP(1)
U16 __RT_DIS_MASK0_usb[TMK_MAX_TYPE+1] =
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
U16 __RT_BRC_MASK0_usb[TMK_MAX_TYPE+1] =
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

U16 __RT_DIS_MASK_usb[DRV_MAX_TYPE+1];
U16 __RT_BRC_MASK_usb[DRV_MAX_TYPE+1];

U16 __mtCW_usb[NTMK];
U16 __rtGap_usb[NTMK];
U16 __bcLinkBaseN_usb[NTMK][DRV_MAX_BASE+1];
U16 __bcLinkCCN_usb[NTMK][DRV_MAX_BASE+1];
////U32 bcLinkWPtr[NTMK];
////                DD      bcLinkW0
////                IRPC    N, 1234567
////                DD      bcLinkW&N&
////                ENDM
U16 __bcCmdWN_usb[NTMK][DRV_MAX_BASE+1];
////U32 bcCmdWPtr[NTMK];
////                DD      bcCmdW0
////                IRPC    N, 1234567
////                DD      bcCmdW&N&
////                ENDM

#ifdef NMBCID
int __mbcAlloc_usb[NMBCID];
int __mbci_usb[NMBCID];
U16 __mbcBase_usb[NMBCID][NTMK];
U16 __mbcTmkN_usb[NMBCID][NTMK];
U16 __mbcPort_usb[NMBCID][NTMK];
U16 __mbcData_usb[NMBCID][NTMK];
U16 __mbcPort0_usb[NMBCID][NTMK];
U16 __mbcData0_usb[NMBCID][NTMK];
#endif //NMBCID

char __TMKLL_Ver_usb[] = "TMKNLL v7.02";
#ifdef CPU188
char __ch186_usb[] = "-188";
#endif
#ifdef RAMwoCLI
char __chRAMwoCLI_usb[] = ".a";
#else
char __chRAMwoCLI_usb[] = ".b";
#endif
#ifdef MASKTMKS
char __chMASKTMKS_usb[] = ".a";
#else
char __chMASKTMKS_usb[] = ".b";
#endif

//;_DATA  ENDS
//;VXD_LOCKED_DATA_ENDS

#ifdef LINUX
#define IRQ_FLAGS
#define GET_DIS_IRQ() { /*spin_lock_irq(&tmkIrqSpinLock);*/ }
#define REST_IRQ() { /*spin_unlock_irq(&tmkIrqSpinLock);*/ }
#define GET_MUTEX
#define REST_MUTEX
#ifdef __SMP__
#define GET_DIS_IRQ_SMP() { /*spin_lock_irq(&tmkIrqSpinLock);*/ }
#define REST_IRQ_SMP() { /*spin_unlock_irq(&tmkIrqSpinLock);*/ }
#else
#define GET_DIS_IRQ_SMP()
#define REST_IRQ_SMP()
#endif //def __SMP__
#endif

#ifdef LINUX
#define outpw(port, data) Error//fakeout(port, data)
#define inpw(port) Error//fakein(port)
#define outpb(port, data) Error//fakeout(data, port)
#define inpb(port) Error//fakein(port)
#endif //def LINUX
/*#ifdef LINUX
#define outpw(port, data) outw(data, port)
#define inpw(port) inw(port)
#define outpb(port, data) outb(data, port)
#define inpb(port) inb(port)
#endif //def LINUX*/

#define DrvBcDefBaseTA(realnum, base) { \
  __bcBasePC_usb[realnum] = base; \
}

/*__INLINE
void DrvBcPokeTA(int realnum, unsigned pos, unsigned data)
{
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  outpw(TA_DATA(realnum), data);
} */

void DrvBcDefBase_usb(int realnum, unsigned type, unsigned base);
void DrvBcPoke_usb(int realnum, unsigned type, unsigned pos, unsigned data);

unsigned DrvBcPeek_usb(int realnum, unsigned type, unsigned pos);

/*__INLINE
unsigned DrvBcPeekTA(int realnum, unsigned pos)
{
  unsigned data;
  outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + pos);
  data = inpw(TA_DATA(realnum));
  return data;
} */

void DrvRtPoke_usb(int __tmkNumber, unsigned type, unsigned pos, unsigned data);

unsigned DrvRtPeek_usb(int __tmkNumber, unsigned type, unsigned pos);

/*__INLINE
void DrvRtPokeTA(int realnum, unsigned base, unsigned pos, unsigned data)
{
  outpw(TA_ADDR(realnum), (base << 6) + pos);
  outpw(TA_DATA(realnum), data);
}

__INLINE
unsigned DrvRtPeekTA(int realnum, unsigned base, unsigned pos)
{
  unsigned data;
  outpw(TA_ADDR(realnum), (base << 6) + pos);
  data = inpw(TA_DATA(realnum));
  return data;
} */

void DrvRtWMode_usb(int num, unsigned type, unsigned mode);
void DrvFlagMode_usb(int num, int m);

void (FARIR *tmkUserErrors_usb)(void) = retfLabel_usb;

#define DEF_VAR(type, var) type var

#define GET_VAR(varloc, varpar) varloc = varpar

#define GET_RealNum (__amrtNumber_usb[__tmkNumber_usb])
#define PUT_RealNum(num) {}

#define PUSH_RealNum {}
#define POP_RealNum {}

#define INT3 { asm db 0cch; }

#ifdef STATIC_TMKNUM

#define CLRtmkError { \
  tmkError_usb = 0; \
}

#else

#ifdef NTMKLL_CLR_TMK_ERROR
#define CLRtmkError { \
  tmkError_usb[__tmkNumber_usb] = 0; \
}
#else
#define CLRtmkError {}
#endif

#endif

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
#define USER_ERROR(err) (tmkError_usb = err)
#define USER_ERROR_R USER_ERROR
#else
#define USER_ERROR(err) (tmkError_usb[__tmkNumber_usb] = err)
#define USER_ERROR_R(err) (err)
#endif

#ifndef NOCHECK
#define CHECK_TMK_DEVICE(num) { \
  if (minor_table[num] == NULL) \
    return USER_ERROR(TMK_BAD_NUMBER); \
}
#define CHECK_TMK_DEVICEN(num) { \
  if (minor_table[num] == NULL) \
  { \
    USER_ERROR(TMK_BAD_NUMBER); \
    return 0;\
  } \
}
#define CHECK_TMK_DEVICEV(num) { \
  if (minor_table[num] == NULL) \
  { \
    USER_ERROR(TMK_BAD_NUMBER); \
    return;\
  } \
}
#else
#define CHECK_TMK_DEVICE(num)
#define CHECK_TMK_DEVICEN(num)
#define CHECK_TMK_DEVICEV(num)
#endif

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
  if ((unsigned)num > __tmkMaxNumber_usb) \
    return USER_ERROR_R(TMK_BAD_NUMBER); \
}
#else
#define CHECK_TMK_NUMBER(num)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE(num, ReqMode) { \
  if (__tmkMode_usb[realnum] != ReqMode) \
    return USER_ERROR(TMK_BAD_FUNC); \
}
#else
#define CHECK_TMK_MODE(num, ReqMode)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE_L(num, ReqMode) { \
  if ((__tmkMode_usb[realnum] & 0x00FF) != ReqMode) \
    return USER_ERROR(TMK_BAD_FUNC); \
}
#else
#define CHECK_TMK_MODE_L(num, ReqMode)
#endif

#ifndef NOCHECK
#define CHECK_TMK_MODE_LN(num, ReqMode) { \
  if ((__tmkMode_usb[realnum] & 0x00FF) != ReqMode) \
  { \
    USER_ERROR(TMK_BAD_FUNC); \
    return; \
  } \
}
#else
#define CHECK_TMK_MODE_LN(num, ReqMode)
#endif

#ifndef NOCHECK
#define CHECK_IRQ(irq)
#else
#define CHECK_IRQ(irq)
#endif

#ifndef NOCHECK
#define CHECK_TMK_TYPE_1(type) { \
  if ((unsigned)(type) < TMK_MIN_TYPE || (unsigned)(type) > TMK_MAX_TYPE) \
    return USER_ERROR_R(TMK_BAD_TYPE); \
}
#else
#define CHECK_TMK_TYPE_1(type)
#endif

#ifndef NOCHECK
#define CHECK_TMK_TYPE_2(type) { \
  if ((unsigned)(type) > DRV_MAX_TYPE) \
    return USER_ERROR_R(TMK_BAD_TYPE); \
}
#else
#define CHECK_TMK_TYPE_2(type)
#endif

#define CHECK_TMK_TYPE(type)

#define CHECK_BC_BASE_BX(num, base) { \
  if (base > __bcMaxBase_usb[num]) \
    return USER_ERROR(BC_BAD_BASE); \
}

#define CHECK_BCMT_BASE_BX(num, base) { \
  if ((__tmkMode_usb[num] == BC_MODE && base > __bcMaxBase_usb[num]) || \
      (__tmkMode_usb[num] != BC_MODE && base > __mtMaxBase_usb[num])) \
    return USER_ERROR(BC_BAD_BASE); \
}

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
  if (page > __rtMaxPage_usb[num]) \
    return USER_ERROR(RT_BAD_PAGE); \
}

#define CHECK_RT_ADDRESS(addr) { \
  if (addr > 0x1E) \
    return USER_ERROR(RT_BAD_ADDRESS); \
}

#define CHECK_RT_CMD(cmd) { cmd &= 0x041F; }

void MyUserErrors_usb(unsigned err)
{
#ifdef STATIC_TMKNUM
  tmkError_usb = err;
  tmkUserErrors_usb();
#endif
}

/*unsigned inpb_d(int num, unsigned port)
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

//the function always works as ifndef RAMwoCLI
#define REP_INSW_D { \
  do \
  { \
    int t; \
    GET_DIS_IRQ(); \
    *(buf++) = inpw(port); \
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
    ++__tmkRAMAddr[realnum]; \
    REST_IRQ(); \
    t = __wOutDelay[realnum]; \
    while (--t); \
  } \
  while (--len != 0); \
}*/

void FARIR retfLabel_usb()
{
}

int FARFN tmkdefdac_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 tmkDacValue)
{
  int realnum;
  unsigned type;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    return USER_ERROR(TMK_BAD_FUNC);
  }
  return 0;
}

int FARFN tmkgetdac_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 *tmkDacValue,
        U16 *tmkDacMode
        )
{
  int realnum;
  unsigned type;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    *tmkDacValue = 0;
    *tmkDacMode = 0;
    USER_ERROR(TMK_BAD_FUNC);
    break;
  }
  return 0;
}

U16 rtgap_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Gap)
{
  int realnum;
  unsigned type;
  u16 buf_out[7];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  if (Gap != GET_RT_GAP)
  {
    switch (type)
    {
    case __TA:
      if (__tmkHWVer_usb[realnum] < 15)
        Gap = RT_GAP_DEFAULT;
      if (Gap > RT_GAP_OPT1)
        Gap = RT_GAP_OPT1;
      __rtGap_usb[realnum] = Gap;
      GET_DIS_IRQ();
//      outpw(TA_RTA(realnum), (inpw(TA_RTA(realnum)) & (~1 << 11)) | (__rtGap[realnum] << 11));
      buf_out[0] = MOD_RG_AND;
      buf_out[1] = TA_RTA(realnum);
      buf_out[2] = (~1 << 11);
      buf_out[3] = (__rtGap_usb[realnum] << 11);
      buf_out[4] = READ_RG;
      buf_out[5] = TA_BASE(realnum);
      buf_out[6] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      REST_IRQ();
      break;
    }
  }
//  switch (type)
//  {
//  case __TA:
//#ifdef MRTA
//  case __MRTA:
//#endif
    Gap = __rtGap_usb[realnum];
//    break;
//  default:
//    Gap = RT_GAP_DEFAULT;
//    break;
//  }
  return Gap;
}

U16 FARFN tmktimeout_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 TimeOut)
{
  int realnum;
  unsigned type;
  u16 buf_out[7];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  if (TimeOut != GET_TIMEOUT)
  {
    switch (type)
    {
    case __TA:
      if (TimeOut <= 14)
        __tmkTimeOut_usb[realnum] = __TA_14US;
      else if (TimeOut <= 18)
        __tmkTimeOut_usb[realnum] = __TA_18US;
      else if (TimeOut <= 26)
        __tmkTimeOut_usb[realnum] = __TA_26US;
      else
        __tmkTimeOut_usb[realnum] = __TA_63US;
      __bcControls_usb[realnum] = (__bcControls_usb[realnum] & 0xCFFF) | __tmkTimeOut_usb[realnum];
      __rtControls_usb[realnum] = (__rtControls_usb[realnum] & 0xCFFF) | __tmkTimeOut_usb[realnum];
      GET_DIS_IRQ();
//      outpw(TA_MODE1(realnum), (inpw(TA_MODE1(realnum)) & 0xCFFF) | __tmkTimeOut[realnum]);
      buf_out[0] = MOD_RG_AND;
      buf_out[1] = TA_MODE1(realnum);
      buf_out[2] = 0xCFFF;
      buf_out[3] = __tmkTimeOut_usb[realnum];
      buf_out[4] = READ_RG;
      buf_out[5] = TA_BASE(realnum);
      buf_out[6] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      REST_IRQ();
      break;
    }
  }
  switch (type)
  {
  case __TA:
    switch (__tmkTimeOut_usb[realnum])
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

U16 FARFN tmktimer_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 TimerCtrl)
{
  int realnum;
  unsigned type;
  u16 buf_out[9];
  u16 buf_in[4];
  int ptr = 0;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
    if (TimerCtrl != GET_TIMER_CTRL)
    {
      if (TimerCtrl != TIMER_RESET)
        __tmkTimerCtrl_usb[realnum] = (TimerCtrl & TIMER_MASK) | TIMER_NOSTOP;
      GET_DIS_IRQ_SMP();
      if (TimerCtrl == TIMER_RESET)
      {
//        outpw(TA_TIMCR(realnum), 0);
        buf_out[ptr] = WRITE_RG;
        buf_out[ptr + 1] = TA_TIMCR(realnum);
        buf_out[ptr + 2] = 0;
        ptr += 3;
      }
//      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
      buf_out[ptr] = WRITE_RG;
      buf_out[ptr + 1] = TA_TIMCR(realnum);
      buf_out[ptr + 2] = __tmkTimerCtrl_usb[realnum];
      buf_out[ptr + 3] = READ_RG;
      buf_out[ptr + 4] = TA_BASE(realnum);
      buf_out[ptr + 5] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      REST_IRQ_SMP();
    }
    return __tmkTimerCtrl_usb[realnum];
  }
  return 0;
}

U16 FARFN tmkgettimerl_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  U16 timer;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
//    timer = inpw(TA_TIMER2(realnum));
    buf_out[0] = READ_RG;
    buf_out[1] = TA_TIMER2(realnum);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    timer = buf_in[2];
    break;
  default:
    timer = 0;
    break;
  }
  return timer;
}

U32 FARFN tmkgettimer_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  U32 timer;
  u16 buf_out[11];
  u16 buf_in[7];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ();
//    outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum] | 0x0800);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_TIMCR(realnum);
    buf_out[2] = __tmkTimerCtrl_usb[realnum] | 0x0800;
//    timer = (U32)inpw(TA_TIMER1(realnum)) << 16;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_TIMER1(realnum);
//    timer |= (U32)inpw(TA_TIMER2(realnum));
    buf_out[5] = READ_RG;
    buf_out[6] = TA_TIMER2(realnum);
//    outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    buf_out[7] = WRITE_RG;
    buf_out[8] = TA_TIMCR(realnum);
    buf_out[9] = __tmkTimerCtrl_usb[realnum];
    buf_out[10] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);

    timer = ((u32)buf_in[2]) << 16 | (u32)buf_in[5];

    REST_IRQ();
    break;
  default:
    timer = 0L;
    break;
  }
  return timer;
}

U32 FARFN bcgetmsgtime_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  U32 time;
  u16 buf_out[8];
  u16 buf_in[7];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
////    time = (U32)DrvBcPeekTA(realnum, 59) << 16;
//    outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 59);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_ADDR(realnum);
    buf_out[2] = (__bcBasePC_usb[realnum] << 6) + 59;
//    time = ((U32)inpw(TA_DATA(realnum)) << 16);
    buf_out[3] = READ_RG;
    buf_out[4] = TA_DATA(realnum);
//    time |= (U32)inpw(TA_DATA(realnum))
    buf_out[5] = READ_RG;
    buf_out[6] = TA_DATA(realnum);
    buf_out[7] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    time = ((u32)buf_in[2]) << 16 | (u32)buf_in[5];
    REST_IRQ_SMP();
    break;
  default:
    time = 0L;
    break;
  }
  return time;
}

U32 FARFN rtgetmsgtime_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  int realnum;
  unsigned type;
  U32 time;
  u16 buf_out[8];
  u16 buf_in[7];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
//    outpw(TA_ADDR(realnum), ((__rtSubAddr[num] | __hm400Page[num]) << 1) | 59);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_ADDR(realnum);
    buf_out[2] = ((__rtSubAddr_usb[num] | __hm400Page_usb[num]) << 1) | 59;
//    time = (U32)inpw(TA_DATA(realnum)) << 16;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_DATA(realnum);
//    time |= (U32)inpw(TA_DATA(realnum));
    buf_out[5] = READ_RG;
    buf_out[6] = TA_DATA(realnum);
    buf_out[7] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    time = ((u32)buf_in[2]) << 16 | (u32)buf_in[5];
    REST_IRQ_SMP();
    break;
  default:
    time = 0L;
    break;
  }
  return time;
}

void FARFN tmkdeferrors_usb(void (FARIR* UserErrors_usb)(void))
{
#ifdef STATIC_TMKNUM
  CLRtmkError;
  tmkUserErrors_usb = UserErrors_usb;
#endif
}

int FARFN tmkselect_usb(int hTMK)
{
#ifndef STATIC_TMKNUM
  int __tmkNumber; // used in CLRtmkError
#endif
//;  CLRtmkError;
  CHECK_TMK_NUMBER(hTMK);
//  CHECK_TMK_TYPE_1(tmkUserType[hTMK]);
  CHECK_TMK_TYPE_2(__tmkDrvType_usb[hTMK]);
  __tmkNumber = hTMK;
  CLRtmkError;
  PUT_RealNum(__amrtNumber_usb[hTMK]);
  return 0;
}

#ifdef STATIC_TMKNUM
//__inline
int FARFN tmkselected_usb()
{
//;  CLRtmkError;
  return __tmkNumber_usb;
}
#endif

//__inline
int FARFN tmkgetmaxn_usb()
{
//;  CLRtmkError;
//;                mov     eax, NTMK - 1
  return __tmkMaxNumber_usb;
}

U16 FARFN tmkgetmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  CLRtmkError;
  return __tmkMode_usb[__tmkNumber_usb];
}

void FARFN tmksetcwbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 SetControl)
{
  int realnum;
  unsigned type;
  unsigned port;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1_usb[realnum];
  GET_MUTEX;
  switch (type)
  {
    default:
    break;
  }
  REST_MUTEX;
}

void FARFN tmkclrcwbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 ClrControl)
{
  int realnum;
  unsigned type;
  unsigned port;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  port = __tmkPortsAddr1_usb[realnum];
  GET_MUTEX;
  switch (type)
  {
    default:
    break;
  }
  REST_MUTEX;
}

U16 FARFN tmkgetcwbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  U08 bits;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  if (__tmkMode_usb[realnum] != RT_MODE)
    bits = __bcControls_usb[realnum];
  else
    bits = __rtControls_usb[realnum];
  switch (type)
  {
  default:
    bits = 0;
    break;
  }
  return (U16)bits;
}

void FARFN bc_def_tldw_usb(U16 TLDW)
{
//  TLDW;
}

void FARFN bc_enable_di_usb()
{
}

void FARFN bc_disable_di_usb()
{
}

void rtcreatlink_usb(int num, unsigned sa, unsigned len)
{
  unsigned i;
  unsigned base, next;
  u16 *buf_out;
  int ptr = 6;

  if(minor_table[num] == NULL)
    return;
  buf_out = kmalloc(sizeof(u16) * (((len?len:1) - 1) * 6 + 13), GFP_KERNEL);
  if (buf_out == NULL)
    return;

  GET_DIS_IRQ_SMP();
////  DrvRtPokeTA(num, AdrTab, sa, sa|0x4000);
//  outpw(TA_ADDR(num), (AdrTab << 6) + sa);
  buf_out[0] = WRITE_RG;
  buf_out[1] = TA_ADDR(num);
  buf_out[2] = (AdrTab << 6) + sa;
//  outpw(TA_DATA(num), sa|0x4000);
  buf_out[3] = WRITE_RG;
  buf_out[4] = TA_DATA(num);
  buf_out[5] = sa|0x4000;
  REST_IRQ_SMP();
  for (i = 1; i < len; ++i)
  {
    base = sa + ((i-1) << 6);
    next = base + 64; //sa + (i << 6);
    GET_DIS_IRQ_SMP();
////    DrvRtPokeTA(num, base, 63, next|0x4000);
//    outpw(TA_ADDR(num), (base << 6) + 63);
    buf_out[ptr] = WRITE_RG;
    buf_out[ptr + 1] = TA_ADDR(num);
    buf_out[ptr + 2] = (base << 6) + 63;
//    outpw(TA_DATA(num), next|0x4000);
    buf_out[ptr + 3] = WRITE_RG;
    buf_out[ptr + 4] = TA_DATA(num);
    buf_out[ptr + 5] = next|0x4000;
    REST_IRQ_SMP();
    ptr += 6;
  }
  base = sa + ((i-1) << 6);
  GET_DIS_IRQ_SMP();
////  DrvRtPokeTA(num, base, 63, sa|0x4000);
//  outpw(TA_ADDR(num), (base << 6) + 63);
  buf_out[ptr] = WRITE_RG;
  buf_out[ptr + 1] = TA_ADDR(num);
  buf_out[ptr + 2] = (base << 6) + 63;
//  outpw(TA_DATA(num), sa|0x4000);
  buf_out[ptr + 3] = WRITE_RG;
  buf_out[ptr + 4] = TA_DATA(num);
  buf_out[ptr + 5] = sa|0x4000;
  buf_out[ptr + 6] = 0xFFFF;
  Block_out_in(minor_table[num], buf_out, NULL);
  REST_IRQ_SMP();
  kfree(buf_out);
}

void rtcreattab_usb(int num, unsigned len)
{
  unsigned sa;
  u16 buf_out[16];
  u16 buf_in[4];

  if(minor_table[num] == NULL)
    return;
//  outpw(TA_MSGA(num), AdrTab);
  buf_out[0] = WRITE_RG;
  buf_out[1] = TA_MSGA(num);
  buf_out[2] = AdrTab;
  //for MT
  GET_DIS_IRQ_SMP();
////  DrvRtPokeTA(num, AdrTab, 0, 0x4000);
//  outpw(TA_ADDR(num), (AdrTab << 6) + 0);
  buf_out[3] = WRITE_RG;
  buf_out[4] = TA_ADDR(num);
  buf_out[5] = AdrTab << 6;
//  outpw(TA_DATA(num), 0x4000);
  buf_out[6] = WRITE_RG;
  buf_out[7] = TA_DATA(num);
  buf_out[8] = 0x4000;
////  DrvRtPokeTA(num, 0, 63, 0x4000);
//  outpw(TA_ADDR(num), (0 << 6) + 63);
  buf_out[9] = WRITE_RG;
  buf_out[10] = TA_ADDR(num);
  buf_out[11] = 63;
//  outpw(TA_DATA(num), 0x4000);
  buf_out[12] = WRITE_RG;
  buf_out[13] = TA_DATA(num);
  buf_out[14] = 0x4000;
  buf_out[15] = 0xFFFF;
  Block_out_in(minor_table[num], buf_out, NULL);
  REST_IRQ_SMP();

  //for RT
  for (sa = 1; sa < 31; ++sa)
  {
    rtcreatlink_usb(num, sa, len);      //rx sa
    rtcreatlink_usb(num, sa|0x20, len); //tx sa
  }
  rtcreatlink_usb(num, 0x1F, len); //All Mode Cmds
  rtcreatlink_usb(num, 0x20, len); //Cmd TX VECTOR
  rtcreatlink_usb(num, 0x3F, len); //Cmd TX BIT

  buf_out[0] = READ_RG;
  buf_out[1] = TA_BASE(num);
  buf_out[2] = 0xFFFF;
  Block_out_in(minor_table[num], buf_out, buf_in);
}

int FARFN bcreset_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned cntr = 0;
  u16 buf_out[21];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  __tmkMode_usb[realnum] = BC_MODE; //to device
  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_MODE;
  buf_out[2] = BC_MODE;
  __tmkStarted_usb[realnum] = 0;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
//    outpw(TA_RESET(realnum), 0);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_RESET(realnum);
    buf_out[5] = 0;
//    outpw(TA_TIMCR(realnum), 0);
    buf_out[6] = WRITE_RG;
    buf_out[7] = TA_TIMCR(realnum);
    buf_out[8] = 0;
////    if (__tmkTimerCtrl[realnum] & TIMER_BITS)
////      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    cntr = (BC_MODE << 7) | (TA_TXRX_EN + TA_IRQ_EN);
//    outpw(TA_MODE1(realnum), cntr | TA_FIFO_RESET);
    buf_out[9] = WRITE_RG;
    buf_out[10] = TA_MODE1(realnum);
    buf_out[11] = cntr | TA_FIFO_RESET;
//    outpw(TA_MODE1(realnum), cntr);
    buf_out[12] = WRITE_RG;
    buf_out[13] = TA_MODE1(realnum);
    buf_out[14] = cntr;
    REST_IRQ_SMP();
    __bcControls1_usb[realnum] = TA_STOP_ON_EXC | RRG2_BC_Mask_Rez_Bit;
//    outpw(TA_MODE2(realnum), __bcControls1[realnum]);
    buf_out[15] = WRITE_RG;
    buf_out[16] = TA_MODE2(realnum);
    buf_out[17] = __bcControls1_usb[realnum];
    buf_out[18] = READ_RG;
    buf_out[19] = TA_BASE(realnum);
    buf_out[20] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  __tmkTimeOut_usb[realnum] = 0;
  __tmkTimerCtrl_usb[realnum] = 0;
  __bcControls_usb[realnum] = cntr;
  __bcBus_usb[realnum] = 0;
  return 0;
}

int FARFN rtreset_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned cntr = 0;
  u16 buf_out[16];
  u16 buf_in[4];
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(num);
  if (__tmkMode_usb[num] != RT_MODE)
  {
    __rtDisableMask_usb[num] = __RT_DIS_MASK_usb[type];
    __rtBRCMask_usb[num] = __RT_BRC_MASK_usb[type];
  }
  __tmkMode_usb[num] = RT_MODE; //to device
  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_MODE;
  buf_out[2] = RT_MODE;
  switch (type)
  {
  case __TA:
    __hm400Page_usb[num] = 0;
    GET_DIS_IRQ_SMP();
//    outpw(TA_RESET(num), 0);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_RESET(num);
    buf_out[5] = 0;
//    outpw(TA_TIMCR(num), 0);
    buf_out[6] = WRITE_RG;
    buf_out[7] = TA_TIMCR(num);
    buf_out[8] = 0;
////    if (__tmkTimerCtrl[num] & TIMER_BITS)
////      outpw(TA_TIMCR(num), __tmkTimerCtrl[num]);
    cntr = (RT_MODE << 7) + TA_RT_DATA_INT_BLK + TA_IRQ_EN + TA_TXRX_EN + TA_BS_MODE_DATA_EN;
//    outpw(TA_MODE1(num), cntr | TA_FIFO_RESET);
    buf_out[9] = WRITE_RG;
    buf_out[10] = TA_MODE1(num);
    buf_out[11] = cntr | TA_FIFO_RESET;
    REST_IRQ_SMP();
    __rtControls1_usb[num] &= 0xF800 | TA_HBIT_MODE | TA_BRCST_MODE;
//    outpw(TA_MODE2(num), (__rtControls1[num])); // & __rtBRCMask[num]) | __rtDisableMask[num]);
    buf_out[12] = WRITE_RG;
    buf_out[13] = TA_MODE2(num);
    buf_out[14] = __rtControls1_usb[num];
    buf_out[15] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, NULL);

    if (__rtDisableMask_usb[num]) // first rtreset from any other mode
    {
      rtcreattab_usb(num, 1);
      __FLAG_MODE_ON_usb[num] = 0;
    }
    else
    {
      DrvFlagMode_usb(num, 0);
      DrvRtWMode_usb(num, __TA, 0);
      cntr |= TA_RTMT_START;
    }
    GET_DIS_IRQ_SMP();
//    outpw(TA_MODE1(num), cntr);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE1(num);
    buf_out[2] = cntr;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(num);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);

    REST_IRQ_SMP();
    __tmkTimeOut_usb[num] = 0;
    __tmkTimerCtrl_usb[num] = 0;
    __rtGap_usb[num] = 0;
    break;
  }
  __rtControls_usb[num] = cntr;
  __rtMode_usb[num] = 0;
  __rtPagePC_usb[num] = 0;
  __rtPageBus_usb[num] = 0;
  return 0;
}

int FARFN mtreset_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned cntr = 0;
  u16 buf_out[21];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  __tmkMode_usb[realnum] = MT_MODE; //to device
  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_MODE;
  buf_out[2] = MT_MODE;
  __tmkStarted_usb[realnum] = 0;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
//    outpw(TA_RESET(realnum), 0);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_RESET(realnum);
    buf_out[5] = 0;
//    outpw(TA_TIMCR(realnum), 0);
    buf_out[6] = WRITE_RG;
    buf_out[7] = TA_TIMCR(realnum);
    buf_out[8] = 0;
////    if (__tmkTimerCtrl[realnum] & TIMER_BITS)
////      outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
    cntr = (MT_MODE << 7) | (TA_TXRX_EN + TA_IRQ_EN);
//    outpw(TA_MODE1(realnum), cntr | TA_FIFO_RESET);
    buf_out[9] = WRITE_RG;
    buf_out[10] = TA_MODE1(realnum);
    buf_out[11] = cntr | TA_FIFO_RESET;
//    outpw(TA_MODE1(realnum), cntr);
    buf_out[12] = WRITE_RG;
    buf_out[13] = TA_MODE1(realnum);
    buf_out[14] = cntr;
    REST_IRQ_SMP();
    __bcControls1_usb[realnum] = 0xF800; // disable RT
//    outpw(TA_MODE2(realnum), __bcControls1[realnum]);
    buf_out[15] = WRITE_RG;
    buf_out[16] = TA_MODE2(realnum);
    buf_out[17] = __bcControls1_usb[realnum];
    buf_out[18] = READ_RG;
    buf_out[19] = TA_BASE(realnum);
    buf_out[20] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  __tmkTimeOut_usb[realnum] = 0;
  __tmkTimerCtrl_usb[realnum] = 0;
  __bcControls_usb[realnum] = cntr;
  return 0;
}

int FARFN mtdefmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 __mtModeBits)
{
  int realnum;
  unsigned type;
//  unsigned port;
  unsigned bits, bitst;
  u16 buf_out[6];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  bitst = __mtModeBits;
  bits = 0;
  switch (type)
  {
  case __TA:
    if (bitst & DRV_HBIT_MODE)
      bits |= TA_HBIT_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TA_BRCST_MODE;
    GET_MUTEX;
    bits |= __bcControls1_usb[realnum] & 0xFDEF;
    __bcControls1_usb[realnum] = bits;
    REST_MUTEX;
////    bits &= __rtBRCMask[num];
////    bits |= __rtDisableMask[num];
//    outpw(TA_MODE2(realnum), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE2(realnum);
    buf_out[2] = bits;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  return 0;
}

U16 FARFN mtgetmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned bits = 0, bitst;

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    bitst = __bcControls1_usb[realnum]; //inpw(TA_MODE2(num));
    bits = 0;
    if (bitst & TA_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & TA_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    break;
  }
  return bits;
}

int FARFN bcdefirqmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 bcIrqModeBits)
{
  int realnum;
  unsigned type;
  unsigned bits;
  u16 buf_out[6];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    bits = (((bcIrqModeBits & TMK_IRQ_OFF) ^ TMK_IRQ_OFF) >> 5);
    GET_MUTEX;
    bits |= __bcControls_usb[realnum] & ~TA_IRQ_EN;
    __bcControls_usb[realnum] = bits;
    GET_DIS_IRQ_SMP();
//    outpw(TA_MODE1(realnum), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE1(realnum);
    buf_out[2] = bits;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
  }
  return 0;
}

U16 FARFN bcgetirqmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned bits = 0;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
    bits = GENER1_BLK | GENER2_BLK;
    buf_out[0] = READ_RG;
    buf_out[1] = TA_MODE1(realnum);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
//    if (!(inpw(TA_MODE1(realnum)) & RRG1_En_IRQ))
    if (!(buf_in[2] & RRG1_En_IRQ))
      bits |= TMK_IRQ_OFF;
    break;
  }
  return bits;
}

int FARFN bcstart_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned code;
  unsigned base;
  unsigned basepc;
  unsigned len;
  u16 buf_out[24];
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE(realnum, BC_MODE);
  code = bcCtrlCode;
  CHECK_BC_CTRL(code);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  base = Base;
  CHECK_BC_BASE_BX(realnum, base);
  CHECK_TMK_DEVICE(realnum);
/*  if (__tmkStarted[realnum])
  {
    switch (type)
    {
#ifdef MRTA
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
  }*/
//  __tmkStarted_usb[realnum] = 1;
  __bcBaseBus_usb[realnum] = base; //to device
  basepc = __bcBasePC_usb[realnum];
  if (base == basepc)
    basepc = -1;
  __bcXStart_usb[realnum] = 0; //to device

  __bcAW1Pos_usb[realnum] = 0;  //to device
  __bcAW2Pos_usb[realnum] = 0; //to device
  switch (code)
  {
  case DATA_RT_RT_BRCST:
    __bcAW1Pos_usb[realnum] = 2;
    break;
  case DATA_BC_RT:
    len = __bcCmdWN_usb[realnum][base] & 0x1F;
    if (len == 0)
      len = 0x20;
    __bcAW2Pos_usb[realnum] = len + 1;
    break;
  case DATA_RT_BC:
    __bcAW1Pos_usb[realnum] = 1;
    break;
  case DATA_RT_RT:
    len = __bcCmdWN_usb[realnum][base] & 0x1F;
    if (len == 0)
      len = 0x20;
    __bcAW1Pos_usb[realnum] = 2;
    __bcAW2Pos_usb[realnum] = len + 3;
    break;
  case CTRL_C_A:
    __bcAW2Pos_usb[realnum] = 1;
    break;
  case CTRL_CD_A:
    __bcAW2Pos_usb[realnum] = 2;
    break;
  case CTRL_C_AD:
    __bcAW1Pos_usb[realnum] = 1;
    break;
  }

  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_BCBASEBUS;
  buf_out[2] = base;
  buf_out[3] = WRITE_PARAM;
  buf_out[4] = PARAM_BCXSTART;
  buf_out[5] = 0;
  buf_out[6] = WRITE_PARAM;
  buf_out[7] = PARAM_BCAW1POS;
  buf_out[8] = __bcAW1Pos_usb[realnum];
  buf_out[9] = WRITE_PARAM;
  buf_out[10] = PARAM_BCAW2POS;
  buf_out[11] = __bcAW2Pos_usb[realnum];

  switch (type)
  {
  case __TA:
    {
      unsigned ContrW;
      ContrW = 0x1D1F;

      if ((code == DATA_RT_RT) || (code == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      ContrW |= __bcBus_usb[realnum] << 7;

      GET_DIS_IRQ_SMP();
//      outpw(TA_ADDR(realnum), (base<<6) | 61);
//      outpw(TA_DATA(realnum), ContrW);
//      outpw(TA_DATA(realnum), 0);
      buf_out[12] = WRITE_MEM;
      buf_out[13] = 2;
      buf_out[14] = (base<<6) | 61;
      buf_out[15] = ContrW;
      buf_out[16] = 0;
      REST_IRQ_SMP();
//      outpw(TA_MSGA(realnum), base & 0x03FF);
      buf_out[17] = WRITE_RG;
      buf_out[18] = TA_MSGA(realnum);
      buf_out[19] = base & 0x03FF;

//      outpw(TA_MODE2(realnum), __bcControls1[realnum] | TA_BC_START);
      buf_out[20] = WRITE_RG;
      buf_out[21] = TA_MODE2(realnum);
      buf_out[22] = __bcControls1_usb[realnum] | TA_BC_START;
      buf_out[23] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, NULL);
    }
    break;
  }
  return 0;
}

int FARFN bcstop_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  u16 buf_out[6];
  u16 buf_in[5];

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    if (__tmkMode_usb[realnum] != MT_MODE) //bcstop
    {
//      outpw(TA_MSGA(realnum), inpw(TA_MSGA(realnum)));
/*      buf_out[0] = 4;
      buf_out[1] = TA_MSGA(realnum);
      buf_out[2] = 0xFFFF;
      buf_out[3] = 0;
      buf_out[4] = 0xFFFF;*/
      buf_out[0] = WRITE_RG;
      buf_out[1] = TA_MODE2(realnum);
      buf_out[2] = __bcControls1_usb[realnum] | TA_BC_STOP;
      buf_out[3] = READ_RG;
      buf_out[4] = TA_BASE(realnum);
      buf_out[5] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
    }
    else //mtstop
    {
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
//      outpw(TA_MODE1(realnum), __bcControls[realnum] &= ~TA_RTMT_START);
      __bcControls_usb[realnum] &= ~TA_RTMT_START;
      buf_out[0] = WRITE_RG;
      buf_out[1] = TA_MODE1(realnum);
      buf_out[2] = __bcControls_usb[realnum];
      buf_out[3] = READ_RG;
      buf_out[4] = TA_BASE(realnum);
      buf_out[5] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
////      outpw(TA_MODE1(realnum), inpw(TA_MODE1(realnum)) & 0xFFF7);
      REST_IRQ_SMP();
      REST_MUTEX;
    }
    break;
  }
  return 0;
}

int FARFN bcstartx_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned code;
  unsigned base;
  u16 buf_out[24];
  int ptr = 11;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  base = Base;
  CHECK_BCMT_BASE_BX(realnum, base);
  CHECK_TMK_DEVICE(realnum);
  __bcBaseBus_usb[realnum] = base; //to device
  __bcXStart_usb[realnum] = 1; //to device
  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_BCBASEBUS;
  buf_out[2] = base;
  buf_out[3] = WRITE_PARAM;
  buf_out[4] = PARAM_BCXSTART;
  buf_out[5] = 1;
  switch (type)
  {
  case __TA:
    if (__tmkMode_usb[realnum] != MT_MODE) //bcstartx
    {
      unsigned ContrW = 0x1D1F;
      unsigned code1;

/*      if (__tmkStarted[realnum])
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_RESET(realnum), 0);
        outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
        outpw(TA_MODE1(realnum), __bcControls[realnum]);
        REST_IRQ_SMP();
        outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      }*/
//      __tmkStarted[realnum] = 1;

      if (((code&0xf) == DATA_RT_RT) || ((code&0xf) == DATA_RT_RT_BRCST))
        ContrW |= 0x0040;
      if (code & CX_BUS_B)
        ContrW |= 0x0080;
      if (code & CX_CONT)
        ContrW |= 0x2000;
      code1 = __bcLinkCCN_usb[realnum][base];
      if (code1 & CX_SIG)
        ContrW |= 0x8000;

      GET_DIS_IRQ_SMP();
//      outpw(TA_ADDR(realnum), (base<<6) | 61);
//      outpw(TA_DATA(realnum), ContrW);
//      outpw(TA_DATA(realnum), 0);
      buf_out[6] = WRITE_MEM;
      buf_out[7] = 2;
      buf_out[8] = (base<<6) | 61;
      buf_out[9] = ContrW;
      buf_out[10] = 0;
      REST_IRQ_SMP();
#ifdef DRV_EMULATE_FIRST_CX_SIG
//emulation is through special base 0x3ff
//also it could be a good (or bad) idea to block irq output
//and further poll it until it occurs
      if (code & CX_SIG)
      {
        GET_DIS_IRQ_SMP();
//        outpw(TA_ADDR(realnum), (0x03FF<<6) | 61);
//        port = TA_DATA(realnum);
//        outpw(port, 0xA020);
//        outpw(port, 0);
//        outpw(port, base);
        buf_out[ptr] = WRITE_MEM;
        buf_out[ptr + 1] = 3;
        buf_out[ptr + 2] = (0x03FF<<6) | 61;
        buf_out[ptr + 3] = 0xA020;
        buf_out[ptr + 4] = 0;
        buf_out[ptr + 5] = base;
        ptr += 6
        REST_IRQ_SMP();
        base = 0x03FF;
      }
#endif //def DRV_EMULATE_FIRST_CX_SIG
//      outpw(TA_MSGA(realnum), base & 0x03FF);
      buf_out[ptr] = WRITE_RG;
      buf_out[ptr + 1] = TA_MSGA(realnum);
      buf_out[ptr + 2] = base & 0x03FF;

//      outpw(TA_MODE2(realnum), __bcControls1[realnum] | TA_BC_START);
      buf_out[ptr + 3] = WRITE_RG;
      buf_out[ptr + 4] = TA_MODE2(realnum);
      buf_out[ptr + 5] = __bcControls1_usb[realnum] | TA_BC_START;
      buf_out[ptr + 6] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, NULL);
    }
    else //mtstartx
    {
//      ??? what for CX_SIG? ! make nop msg with irq! but for mt?
      unsigned oldbase = __bcBasePC_usb[realnum];

/*      if (__tmkStarted[realnum])
      {
        GET_DIS_IRQ_SMP();
        outpw(TA_RESET(realnum), 0);
        outpw(TA_TIMCR(realnum), __tmkTimerCtrl[realnum]);
        GET_MUTEX;
        outpw(TA_MODE1(realnum), __bcControls[realnum] &= ~TA_RTMT_START);
        REST_MUTEX;
        REST_IRQ_SMP();
        outpw(TA_MODE2(realnum), __bcControls1[realnum]);
      }*/
//      __tmkStarted[realnum] = 1;
      __mtCW_usb[realnum] = code; //to device
      buf_out[6] = WRITE_PARAM;
      buf_out[7] = PARAM_MTCW;
      buf_out[8] = code;
      DrvBcDefBaseTA(realnum, AdrTab);
      GET_DIS_IRQ_SMP();
////      DrvBcPokeTA(realnum, 0, base|0x4000);
//      outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 0);
      buf_out[9] = WRITE_RG;
      buf_out[10] = TA_ADDR(realnum);
      buf_out[11] = __bcBasePC_usb[realnum] << 6;
//      outpw(TA_DATA(realnum), base|0x4000);
      buf_out[12] = WRITE_RG;
      buf_out[13] = TA_DATA(realnum);
      buf_out[14] = base|0x4000;
      REST_IRQ_SMP();

////      DrvBcDefBaseTA(realnum, base);
////      GET_DIS_IRQ_SMP();
////      DrvBcPokeTA(realnum, ADR_SIG_WORD, code);
////      REST_IRQ_SMP();
      DrvBcDefBaseTA(realnum, oldbase);
//      outpw(TA_MSGA(realnum), AdrTab);
      buf_out[15] = WRITE_RG;
      buf_out[16] = TA_MSGA(realnum);
      buf_out[17] = AdrTab;
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
//      outpw(TA_MODE1(realnum), __bcControls[realnum] |= TA_RTMT_START);
      buf_out[18] = WRITE_RG;
      buf_out[19] = TA_MODE1(realnum);
      buf_out[20] = __bcControls_usb[realnum] |= TA_RTMT_START;
      buf_out[21] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, NULL);
      REST_IRQ_SMP();
      REST_MUTEX;
    }
    break;
  }
  return 0;
}

int FARFN bcdeflink_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Base, U16 bcCtrlCode)
{
  int realnum;
  unsigned type;
  unsigned code;
  unsigned base;
  unsigned oldbase;
  unsigned code1;
  u16 buf_out[27];
  u16 buf_in[4];
  int ptr = 10;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  base = Base;
  CHECK_BCMT_BASE_BX(realnum, base);
  CHECK_TMK_DEVICE(realnum);
  __bcLinkBaseN_usb[realnum][__bcBasePC_usb[realnum]] = base; //to device
  buf_out[0] = WRITE_PARAM;
  buf_out[1] = PARAM_BCLINKBASEN;
  buf_out[2] = 1;
  buf_out[3] = __bcBasePC_usb[realnum];
  buf_out[4] = base;
  code1 = __bcLinkCCN_usb[realnum][__bcBasePC_usb[realnum]];
  __bcLinkCCN_usb[realnum][__bcBasePC_usb[realnum]] = code; //to device
  buf_out[5] = WRITE_PARAM;
  buf_out[6] = PARAM_BCLINKCCN;
  buf_out[7] = 1;
  buf_out[8] = __bcBasePC_usb[realnum];
  buf_out[9] = code;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    oldbase = __bcBasePC_usb[realnum];
    if (__tmkMode_usb[realnum] != MT_MODE) //bcdeflink
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
////          DrvBcPokeTA(realnum, 61, DrvBcPeekTA(realnum, 61) | 0x8000);
//          outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 61);
//          data = inpw(TA_DATA(realnum));
//          outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 61);
//          outpw(TA_DATA(realnum), data | 0x8000);
            buf_out[ptr] = MOD_MEM_AND;
            buf_out[ptr + 1] = 1;
            buf_out[ptr + 2] = (__bcBasePC_usb[realnum] << 6) + 61;
            buf_out[ptr + 3] = 0xFFFF;
            buf_out[ptr + 4] = 0x8000;
        }
        else
        {
////          DrvBcPokeTA(realnum, 61, DrvBcPeekTA(realnum, 61) & ~0x8000);
//          outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 61);
//          data = inpw(TA_DATA(realnum));
//          outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 61);
//          outpw(TA_DATA(realnum), data & ~0x8000);
            buf_out[ptr] = MOD_MEM_AND;
            buf_out[ptr + 1] = 1;
            buf_out[ptr + 2] = (__bcBasePC_usb[realnum] << 6) + 61;
            buf_out[ptr + 3] = (u16)(~0x8000);
            buf_out[ptr + 4] = 0x0000;
        }
        ptr += 5;
      }
////      DrvBcPokeTA(realnum, 63, base);
//      outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 63);
//      outpw(TA_DATA(realnum), base);
      buf_out[ptr] = WRITE_MEM;
      buf_out[ptr + 1] = 1;
      buf_out[ptr + 2] = (__bcBasePC_usb[realnum] << 6) + 63;
      buf_out[ptr + 3] = base;

      DrvBcDefBaseTA(realnum, base);
      code1 = __bcLinkCCN_usb[realnum][base];
      if (code1 & CX_SIG)
        ContrW |= 0x8000;
//      outpw(TA_ADDR(realnum), (base<<6) | 61);
//      outpw(TA_DATA(realnum), ContrW);
//      outpw(TA_DATA(realnum), 0);
      buf_out[ptr + 4] = WRITE_MEM;
      buf_out[ptr + 5] = 2;
      buf_out[ptr + 6] = (base<<6) | 61;
      buf_out[ptr + 7] = ContrW;
      buf_out[ptr + 8] = 0;
      buf_out[ptr + 9] = READ_RG;
      buf_out[ptr + 10] = TA_BASE(realnum);
      buf_out[ptr + 11] = 0xFFFF;

      REST_IRQ_SMP();
      DrvBcDefBaseTA(realnum, oldbase);
      Block_out_in(minor_table[realnum], buf_out, buf_in);
    }
    else //mtdeflink
    { //!!!todo: 0x4000 only if CX_STOP | CX_INT
      GET_DIS_IRQ_SMP();
////      DrvBcPokeTA(realnum, 63, base|0x4000);
//      outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 63);
//      outpw(TA_DATA(realnum), base|0x4000);
      buf_out[10] = WRITE_MEM;
      buf_out[11] = 1;
      buf_out[12] = (__bcBasePC_usb[realnum] << 6) + 63;
      buf_out[13] = base|0x4000;
      buf_out[14] = READ_RG;
      buf_out[15] = TA_BASE(realnum);
      buf_out[16] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      REST_IRQ_SMP();
    }
    break;
  }
  return 0;
}

U32 FARFN bcgetlink_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned base;


  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  base = __bcBasePC_usb[realnum];
  return ((U32)__bcLinkCCN_usb[realnum][base] << 16) |
          (U32)__bcLinkBaseN_usb[realnum][base];
}

U32 FARFN bcgetstate_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned stat = 0;
  unsigned base = 0;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  switch (type)
  {
  case __TA:
//!!! mtgetstate?
    buf_out[0] = READ_RG;
    buf_out[1] = TA_BASE(realnum);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
//    base = (inpw(TA_BASE(realnum))) & 0x3FF;
    base = buf_in[2] & 0x3FF;
    stat = 0;
    break;
  }
  return ((U32)stat << 16) | (U32)base;
}

U16 FARFN bcgetmaxbase_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  if (__tmkMode_usb[realnum] == BC_MODE)
    return __bcMaxBase_usb[realnum];
  else
    return __mtMaxBase_usb[realnum];
}

U16 FARFN bcgetbase_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __bcBasePC_usb[realnum];
}

//__inline
//__INLINE
void DrvBcDefBase_usb(int realnum, unsigned type, unsigned base)
{
  __bcBasePC_usb[realnum] = base;
  switch (type)
  {
  case __TA:
    return;
  }
  return;
}

int FARFN bcdefbase_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  DrvBcDefBase_usb(realnum, type, base);
  return 0;
}

void DrvBcPoke_usb(int realnum, unsigned type, unsigned pos, unsigned data)
{
  unsigned port;
  unsigned save_rama, save_ramiw;
  u16 buf_out[12];
  u16 buf_in[4];
  IRQ_FLAGS;

  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos |= __bcBasePC_usb[realnum] << 6;
    port = __tmkPortsAddr1_usb[realnum] + TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    buf_out[0] = WRITE_RG;
    buf_out[1] = port;
    buf_out[2] = pos;
    port += TMK_DataPort-TMK_AddrPort;
//    outpw(port, data);
    buf_out[3] = WRITE_RG;
    buf_out[4] = port;
    buf_out[5] = data;
    if (!save_ramiw)
    {
      buf_out[6] = READ_RG;
      buf_out[7] = TA_BASE(realnum);
      buf_out[8] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      REST_IRQ_SMP();
      __tmkRAMInWork_usb[realnum] = 0;
      return;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
//    outpw(port, save_rama);
    buf_out[6] = WRITE_RG;
    buf_out[7] = port;
    buf_out[8] = save_rama;
    buf_out[9] = READ_RG;
    buf_out[10] = TA_BASE(realnum);
    buf_out[11] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    REST_IRQ_SMP();
    return;
  }
  return;
}

unsigned DrvBcPeek_usb(int realnum, unsigned type, unsigned pos)
{
  unsigned save_rama, save_ramiw;
  unsigned port;
  u16 buf_out[9];
  u16 buf_in[4];

  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos |= __bcBasePC_usb[realnum] << 6;
    port = __tmkPortsAddr1_usb[realnum] + TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    buf_out[0] = WRITE_RG;
    buf_out[1] = port;
    buf_out[2] = pos;
    port += TMK_DataPort-TMK_AddrPort;
//    data = inpw(port);
    buf_out[3] = READ_RG;
    buf_out[4] = port;
    REST_IRQ_SMP();
    if (!save_ramiw)
    {
      buf_out[5] = 0xFFFF;
      __tmkRAMInWork_usb[realnum] = 0;
      break; //return data;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[5] = WRITE_RG;
    buf_out[6] = port;
    buf_out[7] = save_rama;
    buf_out[8] = 0xFFFF;
    REST_IRQ_SMP();
    break; //    return data;
  }
  Block_out_in(minor_table[realnum], buf_out, buf_in);
  return buf_in[2];
}

U32 FARFN bcgetansw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  aw2pos = 0;
  switch (code)
  {
  case DATA_BC_RT:
    aw1pos = __bcCmdWN_usb[realnum][__bcBasePC_usb[realnum]] & 0x1F;
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
    aw2pos = __bcCmdWN_usb[realnum][__bcBasePC_usb[realnum]] & 0x1F;
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
  aw2 = (aw2pos == 0) ? 0xFFFF : DrvBcPeek_usb(realnum, type, aw2pos); //todo:join with aw1
  aw1 = (aw1pos == 0) ? 0xFFFF : DrvBcPeek_usb(realnum, type, aw1pos);
  return ((U32)aw2 << 16) | (U32)aw1;
}

U16 FARFN mtgetsw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned RetData;
  unsigned SW;
  u16 buf_out[6];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
//    outpw(TA_ADDR(realnum), (__bcBasePC[realnum] << 6) + 58);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_ADDR(realnum);
    buf_out[2] = (__bcBasePC_usb[realnum] << 6) + 58;
//    data = inpw(TA_DATA(realnum));
    buf_out[3] = READ_RG;
    buf_out[4] = TA_DATA(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
////    SW = DrvBcPeekTA(realnum, 58);
    SW = buf_in[2];
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

void FARFN bcputw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  CHECK_TMK_DEVICEV(realnum);
  data = bcData;
  if (pos == 0)
  {
    __bcCmdWN_usb[realnum][__bcBasePC_usb[realnum]] = data;
  }
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  DrvBcPoke_usb(realnum, type, pos, data);
}

U16 FARFN bcgetw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(realnum);
  return (U16)DrvBcPeek_usb(realnum, type, pos);
}

void FARFN bcputblk_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 bcPos, VOID FARDT *pcBufAddr, U16 bcLen)
{
  int realnum;
  unsigned type;
  unsigned pos;
  unsigned len;
  unsigned maxlen;
  unsigned rep;
  int ptr;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  u16 * buf_out;
  u16 buf_in[4];
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_LN(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  buf = (U16 FARDT*)pcBufAddr;
  len = bcLen;
  CHECK_BC_LEN(len);
  CHECK_TMK_DEVICEV(realnum);
  if (len == 0)
    return;
  if (pos == 0)
  {
    __bcCmdWN_usb[realnum][__bcBasePC_usb[realnum]] = *(U16 FARDT*)buf;
  }
  maxlen = (minor_table[realnum]->ep2_maxsize) / 2 - 6;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    buf_out = kmalloc(sizeof(u16) * ((len>maxlen?maxlen:len) + 12), GFP_KERNEL);
    if(buf_out == NULL)
      return;
    pos |= __bcBasePC_usb[realnum] << 6;
//    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos + len;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    REST_IRQ_SMP();
//    port += TMK_DataPort-TMK_AddrPort;
//    REP_OUTSW;
    ptr = 0;
    do
    {
      rep = len>maxlen?maxlen:len;
      buf_out[ptr] = WRITE_MEM;
      buf_out[ptr + 1] = rep;
      buf_out[ptr + 2] = pos;
      pos += rep;
      ptr += 3;
      memcpy(buf_out + ptr, buf, rep * sizeof(u16));
      buf += rep;
      ptr += rep;

      if(len > maxlen)
      {
        buf_out[ptr] = 0xFFFF;
        Block_out_in(minor_table[realnum], buf_out, NULL);
        len -= maxlen;
        ptr = 0;
      }
      else
        len = 0;
      GET_DIS_IRQ();
//      outpw(port, *(buf++));
//      ++__tmkRAMAddr[realnum];
      REST_IRQ();
    }
    while (len != 0);

    if (!save_ramiw)
    {
      buf_out[ptr] = READ_RG;
      buf_out[ptr + 1] = TA_BASE(realnum);
      buf_out[ptr + 2] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      kfree(buf_out);
      break;
    }
//    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[ptr] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, NULL);
    
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_ADDR(realnum);
    buf_out[2] = save_rama;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    kfree(buf_out);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

void FARFN bcgetblk_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 bcPos, VOID FARDT *pcBufAddr, U16 bcLen)
{
  int realnum;
  unsigned type;
  unsigned pos;
  unsigned len;
  unsigned maxlen;
  unsigned rep;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  u16 buf_out[7];
  u16 *buf_in;
  IRQ_FLAGS;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_LN(realnum, BCMT_MODE_L);
  pos = bcPos;
  CHECK_BC_ADDR(pos);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = bcLen;
  CHECK_BC_LEN(len);
  CHECK_TMK_DEVICEV(realnum);
  if (len == 0)
    return;
  maxlen = (minor_table[realnum]->ep2_maxsize) / 2 - 4;
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    buf_in = kmalloc(sizeof(u16) * ((len>maxlen?maxlen:len) + 8), GFP_KERNEL);
    pos |= __bcBasePC_usb[realnum] << 6;
//    port = __tmkPortsAddr1[realnum] + TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos + len;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    REST_IRQ_SMP();
//    port += TMK_DataPort-TMK_AddrPort;
//    REP_INSW;
    do
    {
      rep = len>maxlen?maxlen:len;
      buf_out[0] = READ_MEM;
      if(rep == 1)
        buf_out[1] = rep + 1;
      else
        buf_out[1] = rep;
      buf_out[2] = pos;
      pos += rep;

      if(len > maxlen)
      {
        buf_out[3] = 0xFFFF;
        Block_out_in(minor_table[realnum], buf_out, buf_in);
        len -= maxlen;
        memcpy(buf,buf_in + 3, rep * sizeof(u16));
        buf += rep;
      }
      else
        len = 0;

      GET_DIS_IRQ();
//      *(buf++) = inpw(port);
//      ++__tmkRAMAddr[realnum];
      REST_IRQ();
    }
    while (len != 0);

    if (!save_ramiw)
    {
      buf_out[3] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      memcpy(buf, buf_in + 3, rep * sizeof(u16));
      kfree(buf_in);
      break;
    }
//    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_ADDR(realnum);
    buf_out[5] = save_rama;
    buf_out[6] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    memcpy(buf,buf_in + 3, rep * sizeof(u16));
    REST_IRQ_SMP();
    kfree(buf_in);
    return;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

int FARFN bcdefbus_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Bus)
{
  int realnum;
  unsigned type;
  unsigned bus;

  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_MODE_L(realnum, BCMT_MODE_L);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  bus = Bus;
  switch (type)
  {
  case __TA:
    CHECK_BC_BUS(bus)
    __bcBus_usb[realnum] = bus;
    break;
  }
  return 0;
}

U16 FARFN bcgetbus_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __bcBus_usb[realnum];
}

void DrvRtDefSubAddr_usb(int num, unsigned type, unsigned sa_shl5)
{
  __rtSubAddr_usb[num] = sa_shl5;
  switch (type)
  {
  case __TA:
    return;
  }
  return;
}

__INLINE
unsigned DrvRtGetBaseTA_usb(int num)
{
  return __rtSubAddr_usb[num] >> 5;
}

void FARFN rtdefsubaddr_usb(
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
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  DrvRtDefSubAddr_usb(num, type, sa);
  DrvRtWMode_usb(num, type, sa);
  return;
}

U16 FARFN rtgetsubaddr_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned sa;

  CLRtmkError;
  num = __tmkNumber_usb;
  sa = __rtSubAddr_usb[num];
  return ((sa >> 5) & 0x001F) | (sa & 0x0400);
}

void FARFN rtgetblk_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned type;
  unsigned pos;
  unsigned len;
  unsigned maxlen;
  unsigned rep;
  U16 FARDT *buf;
  unsigned save_rama, save_ramiw;
  u16 buf_out[7];
  u16 *buf_in;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  CHECK_RT_LEN(len);
  if (len == 0)
    return;
  pos += __rtSubAddr_usb[num];
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  maxlen = (minor_table[realnum]->ep2_maxsize) / 2 - 4;
//  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    buf_in = kmalloc(sizeof(u16) * ((len>maxlen?maxlen:len) + 8), GFP_KERNEL);
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
//    port += TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos + len;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    REST_IRQ_SMP();
//    port += TMK_DataPort-TMK_AddrPort;
//    REP_INSW;
    do
    {
      rep = len>maxlen?maxlen:len;
      buf_out[0] = READ_MEM;
      if(rep == 1)
        buf_out[1] = rep + 1;
      else
        buf_out[1] = rep;
      buf_out[2] = pos;
      pos += rep;

      if(len > maxlen)
      {
        buf_out[3] = 0xFFFF;
        Block_out_in(minor_table[realnum], buf_out, buf_in);
        len -= maxlen;
        memcpy(buf,buf_in + 3, rep * sizeof(u16));
        buf += rep;
      }
      else
        len = 0;

      GET_DIS_IRQ();
//      *(buf++) = inpw(port);
//      ++__tmkRAMAddr[realnum];
      REST_IRQ();
    }
    while (len != 0);

    if (!save_ramiw)
    {
      buf_out[3] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      memcpy(buf, buf_in + 3, rep * sizeof(u16));
      kfree(buf_in);
      break;
    }
//    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_ADDR(realnum);
    buf_out[5] = save_rama;
    buf_out[6] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    memcpy(buf,buf_in + 3, rep * sizeof(u16));
    REST_IRQ_SMP();
    kfree(buf_in);
    return;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

void FARFN rtputblk_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtPos, VOID FARDT *pcBufAddr, U16 Len)
{
  int num;
  int realnum;
  unsigned type;
  unsigned pos;
  unsigned len;
  unsigned maxlen;
  unsigned rep;
  int ptr;
  U16 FARDT *buf;
  u16 * buf_out;
  u16 buf_in[4];
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  buf = (U16 FARDT*)pcBufAddr;
  len = Len;
  CHECK_RT_LEN(len);
  if (len == 0)
    return;
  pos += __rtSubAddr_usb[num];
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  maxlen = (minor_table[realnum]->ep2_maxsize) / 2 - 6;
//  port = __tmkPortsAddr1[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    buf_out = kmalloc(sizeof(u16) * ((len>maxlen?maxlen:len) + 12), GFP_KERNEL);
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
//    port += TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos + len;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    REST_IRQ_SMP();
//    port += TMK_DataPort-TMK_AddrPort;
//    REP_OUTSW;
    ptr = 0;
    do
    {
      rep = len>maxlen?maxlen:len;
      buf_out[ptr] = WRITE_MEM;
      buf_out[ptr + 1] = rep;
      buf_out[ptr + 2] = pos;
      pos += rep;
      ptr += 3;
      memcpy(buf_out + ptr, buf, rep * sizeof(u16));
      buf += rep;
      ptr += rep;

      if(len > maxlen)
      {
        buf_out[ptr] = 0xFFFF;
        Block_out_in(minor_table[realnum], buf_out, NULL);
        len -= maxlen;
        ptr = 0;
      }
      else
        len = 0;

      GET_DIS_IRQ();
//      outpw(port, *(buf++));
//      ++__tmkRAMAddr[realnum];
      REST_IRQ();
    }
    while (len != 0);

    if (!save_ramiw)
    {
      buf_out[ptr] = READ_RG;
      buf_out[ptr + 1] = TA_BASE(realnum);
      buf_out[ptr + 2] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      kfree(buf_out);
      break;
    }
//    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[ptr] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, NULL);

    buf_out[0] = READ_RG;
    buf_out[1] = TA_ADDR(realnum);
    buf_out[2] = save_rama;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    REST_IRQ_SMP();
    kfree(buf_out);
    return;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

U16 FARFN rtgetw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtPos)
{
  int num;
  int realnum;
  unsigned type;
  unsigned port;
  unsigned pos;
  unsigned data = 0;
  unsigned save_rama, save_ramiw;
  u16 buf_out[9];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  pos += __rtSubAddr_usb[num];
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1_usb[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  CHECK_TMK_DEVICEN(realnum);
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
    port += TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    buf_out[0] = WRITE_RG;
    buf_out[1] = port;
    buf_out[2] = pos;
    port += TMK_DataPort-TMK_AddrPort;
//    data = inpw(port);
    buf_out[3] = READ_RG;
    buf_out[4] = port;
    REST_IRQ_SMP();
    if (!save_ramiw)
    {
      buf_out[5] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      data = buf_in[2];
      break;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[5] = WRITE_RG;
    buf_out[6] = port;
    buf_out[7] = save_rama;
    buf_out[8] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    data = buf_in[2];
    REST_IRQ_SMP();
    return data;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return data;
}

void FARFN rtputw_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  u16 buf_out[12];
  u16 buf_in[4];
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  pos = rtPos;
  CHECK_RT_SUBPOS(pos);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  pos += __rtSubAddr_usb[num];
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  port = __tmkPortsAddr1_usb[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  CHECK_TMK_DEVICEV(realnum);
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos = ((pos & 0xFFE0) << 1) | (pos & 0x1F);
    port += TMK_AddrPort;
    __tmkRAMAddr_usb[realnum] = pos;
    GET_DIS_IRQ_SMP();
//    outpw(port, pos);
    buf_out[0] = WRITE_RG;
    buf_out[1] = port;
    buf_out[2] = pos;
    port += TMK_DataPort-TMK_AddrPort;
    data = pcData;
//    outpw(port, data);
    buf_out[3] = WRITE_RG;
    buf_out[4] = port;
    buf_out[5] = data;
    REST_IRQ_SMP();
    if (!save_ramiw)
    {
      buf_out[6] = READ_RG;
      buf_out[7] = TA_BASE(realnum);
      buf_out[8] = 0xFFFF;
      Block_out_in(minor_table[realnum], buf_out, buf_in);
      break;
    }
    port += TMK_AddrPort-TMK_DataPort;
    __tmkRAMAddr_usb[realnum] = save_rama;
    GET_DIS_IRQ_SMP();
//    outpw(port, save_rama);
    buf_out[6] = WRITE_RG;
    buf_out[7] = port;
    buf_out[8] = save_rama;
    buf_out[9] = READ_RG;
    buf_out[10] = TA_BASE(realnum);
    buf_out[11] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    REST_IRQ_SMP();
    return;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

void FARFN rtsetanswbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 SetControl)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits;
  u16 buf_out[6];
  u16 buf_in[4];
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  port = __tmkPortsAddr1_usb[realnum] + TMK_ModePort;
  switch (type)
  {
  case __TA:
    CONVERT_TA_SW_BITS(bits, SetControl);
    GET_MUTEX;
    bits |= __rtControls1_usb[realnum];
    __rtControls1_usb[realnum] = bits;
//    outpw(TA_MODE2(realnum), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE2(realnum);
    buf_out[2] = bits;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  REST_MUTEX;
  return;
}

void FARFN rtclranswbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 ClrControl)
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned bits;
  u16 buf_out[6];
  u16 buf_in[4];
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  port = __tmkPortsAddr1_usb[realnum] + TMK_ModePort;
  switch (type)
  {
  case __TA:
    CONVERT_TA_SW_BITS(bits, ClrControl);
    bits = ~bits;
    GET_MUTEX;
    bits &= __rtControls1_usb[realnum];
    __rtControls1_usb[realnum] = bits;
//    outpw(TA_MODE2(realnum), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE2(realnum);
    buf_out[2] = bits;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(realnum);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  REST_MUTEX;
  return;
}

U16 FARFN rtgetanswbits_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned bits = 0, bitst;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb; //???
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (type)
  {
  case __TA:
//    bitst = inpw(TA_MODE2(num));
    buf_out[0] = READ_RG;
    buf_out[1] = TA_MODE2(num);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    bitst = buf_in[2];
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
  }
  bits &= RTAnswBitsMask;
  return bits;
}

int FARFN rtdefirqmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtIrqModeBits)
{
  int num;
  unsigned type;
  unsigned bits;
  u16 buf_out[6];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(num);
  switch (type)
  {
  case __TA:
    bits = 0;
    if (!(rtIrqModeBits & TMK_IRQ_OFF))
      bits |= TA_IRQ_EN;
    if (rtIrqModeBits & RT_DATA_BL)
      bits |= TA_RT_DATA_INT_BLK;
    GET_MUTEX;
    bits |= __rtControls_usb[num] & ~(TA_IRQ_EN | TA_RT_DATA_INT_BLK);
    __rtControls_usb[num] = bits;
    GET_DIS_IRQ_SMP();
//    outpw(TA_MODE1(num), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE1(num);
    buf_out[2] = bits;
    buf_out[3] = READ_RG;
    buf_out[4] = TA_BASE(num);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
  }
  return 0;
}

U16 FARFN rtgetirqmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned bits = 0;
  unsigned bitst;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (type)
  {
  case __TA:
//    bitst = inpw(TA_MODE1(num));
    buf_out[0] = READ_RG;
    buf_out[1] = TA_MODE1(num);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    bitst = buf_in[2];
    bits = RT_GENER1_BL|RT_GENER2_BL;
    if (!(bitst & TA_IRQ_EN))
      bits |= TMK_IRQ_OFF;
    if (bitst & TA_RT_DATA_INT_BLK)
      bits |= RT_DATA_BL;
    break;
  }
  return bits;
}

void DrvFlagMode_usb(int num, int m)
{
  unsigned data;
  unsigned i;
  u16 buf_out[11];

  if (__FLAG_MODE_ON_usb[num] != m)
  {
    if (m)
    {
      if (__rtMode_usb[num] & 0x0800)
      {
  // disable rtlock if switch flag mode on
        data = (__rtMode_usb[num] >> 5) & 0x3F;
        {
          GET_DIS_IRQ_SMP();
////          DrvRtPokeTA(num, AdrTab, data, (DrvRtPeekTA(num, AdrTab, data) & 0x7FFF));
//          outpw(TA_ADDR(num), (AdrTab << 6) + data);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), (AdrTab << 6) + data);
//          outpw(TA_DATA(num), data2 & 0x7FFF);
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + data;
          buf_out[3] = 0x7FFF;
          buf_out[4] = 0;
          buf_out[5] = 0xFFFF;
          Block_out_in(minor_table[num], buf_out, NULL);
          REST_IRQ_SMP();
        }
      }
#ifdef STATIC_TMKNUM
      rtputflags_usb(__BC_RT_FLAG_usb[num], RT_RECEIVE, 1, 30);
      rtputflags_usb(__RT_BC_FLAG_usb[num], RT_TRANSMIT, 1, 30);
#else
      rtputflags_usb(num, __BC_RT_FLAG_usb[num], RT_RECEIVE, 1, 30);
      rtputflags_usb(num, __RT_BC_FLAG_usb[num], RT_TRANSMIT, 1, 30);
#endif
    }
    else
    {
#ifdef STATIC_TMKNUM
      rtgetflags_usb(__BC_RT_FLAG_usb[num], RT_RECEIVE, 1, 30);
      rtgetflags_usb(__RT_BC_FLAG_usb[num], RT_TRANSMIT, 1, 30);
#else
      rtgetflags_usb(num, __BC_RT_FLAG_usb[num], RT_RECEIVE, 1, 30);
      rtgetflags_usb(num, __RT_BC_FLAG_usb[num], RT_TRANSMIT, 1, 30);
#endif
      if (__rtMode_usb[num] & 0x0800)
      {
  // enable rtlock if switch flag mode off
        data = (__rtMode_usb[num] >> 5) & 0x3F;
        {
          GET_DIS_IRQ_SMP();
////          DrvRtPokeTA(num, AdrTab, data, (DrvRtPeekTA(num, AdrTab, data) | 0x8000));
//          outpw(TA_ADDR(num), (AdrTab << 6) + data);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), (AdrTab << 6) + data);
//          outpw(TA_DATA(num), data2 | 0x8000);
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + data;
          buf_out[3] = 0xFFFF;
          buf_out[4] = 0x8000;
          buf_out[5] = 0xFFFF;
          Block_out_in(minor_table[num], buf_out, NULL);
          REST_IRQ_SMP();
        }
      }
    }

    {
      for (i = 1; i < 31; ++i)
      {
        if (m)
        {
          GET_DIS_IRQ_SMP();
////          DrvRtPokeTA(num, i, 63, DrvRtPeekTA(num, i, 63) | 0x8000);
//          outpw(TA_ADDR(num), (i << 6) + 63);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), (i << 6) + 63);
//          outpw(TA_DATA(num), data2 | 0x8000);
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (i << 6) + 63;
          buf_out[3] = 0xFFFF;
          buf_out[4] = 0x8000;
////          DrvRtPokeTA(num, i|0x20, 63, DrvRtPeekTA(num, i|0x20, 63) | 0x8000);
//          outpw(TA_ADDR(num), ((i|0x20) << 6) + 63);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), ((i|0x20) << 6) + 63);
//          outpw(TA_DATA(num), data2 | 0x8000);
          buf_out[5] = MOD_MEM_AND;
          buf_out[6] = 1;
          buf_out[7] = ((i|0x20) << 6) + 63;
          buf_out[8] = 0xFFFF;
          buf_out[9] = 0x8000;
          buf_out[10] = 0xFFFF;
          Block_out_in(minor_table[num], buf_out, NULL);
          REST_IRQ_SMP();
        }
        else
        {
          GET_DIS_IRQ_SMP();
////          DrvRtPokeTA(num, i, 63, DrvRtPeekTA(num, i, 63) & 0x7FFF);
//          outpw(TA_ADDR(num), (i << 6) + 63);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), (i << 6) + 63);
//          outpw(TA_DATA(num), data2 & 0x7FFF);
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (i << 6) + 63;
          buf_out[3] = 0x7FFF;
          buf_out[4] = 0;
////          DrvRtPokeTA(num, i|0x20, 63, DrvRtPeekTA(num, i|0x20, 63) & 0x7FFF);
//          outpw(TA_ADDR(num), ((i|0x20) << 6) + 63);
//          data2 = inpw(TA_DATA(num));
//          outpw(TA_ADDR(num), ((i|0x20) << 6) + 63);
//          outpw(TA_DATA(num), data2 & 0x7FFF);
          buf_out[5] = MOD_MEM_AND;
          buf_out[6] = 1;
          buf_out[7] = ((i|0x20) << 6) + 63;
          buf_out[8] = 0x7FFF;
          buf_out[9] = 0;
          buf_out[10] = 0xFFFF;
          Block_out_in(minor_table[num], buf_out, NULL);
          REST_IRQ_SMP();
        }
      }
    }

    __FLAG_MODE_ON_usb[num] = m;
  }
}

int FARFN rtdefmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 __rtModeBits)
{
  int num;
  unsigned type;
  unsigned bits, bitst;
  u16 buf_out[4];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(num);
  bitst = __rtModeBits;
  bits = 0;
  switch (type)
  {
  case __TA:
    if (bitst & DRV_HBIT_MODE)
      bits |= TA_HBIT_MODE;
    if (bitst & DRV_BRCST_MODE)
      bits |= TA_BRCST_MODE;
    GET_MUTEX;
    bits |= __rtControls1_usb[num] & 0xFDEF;
    __rtControls1_usb[num] = bits;
    REST_MUTEX;
//    bits &= __rtBRCMask[num];
//    bits |= __rtDisableMask[num];

//    outpw(TA_MODE2(num), bits);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE2(num);
    buf_out[2] = bits;
    buf_out[3] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, NULL);

    if (bitst & DRV_FLAG_MODE)
      DrvFlagMode_usb(num, 1);
    else
      DrvFlagMode_usb(num, 0);

    buf_out[0] = READ_RG;
    buf_out[1] = TA_BASE(num);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    break;
  }
  return 0;
}

U16 FARFN rtgetmode_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned bits = 0, bitst;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    bitst = __rtControls1_usb[num]; //inpw(TA_MODE2(num));
    bits = 0;
    if (bitst & TA_HBIT_MODE)
      bits |= DRV_HBIT_MODE;
    if (bitst & TA_BRCST_MODE)
      bits |= DRV_BRCST_MODE;
    if (__FLAG_MODE_ON_usb[num])
      bits |= DRV_FLAG_MODE;
    break;
  }
  return bits;
}

int FARFN rtdefpage_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtPage)
{
  int num;
  unsigned type;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  page = rtPage;
  {
    CHECK_RT_PAGE_BX(num, page);
  }
  __rtPagePC_usb[num] = page;
  __rtPageBus_usb[num] = page;
  switch (type)
  {
  case __TA:
    break;
  }
  return 0;
}

U16 FARFN rtgetpage_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber_usb;
  return __rtPagePC_usb[num];
}

U16 FARFN rtgetmaxpage_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber_usb;
  return __rtMaxPage_usb[num];
}

int FARFN rtdefpagepc_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 PagePC)
{
  int num;
  unsigned type;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  page = PagePC;
  {
    CHECK_RT_PAGE_BX(num, page);
  }
  switch (type)
  {
  case __TA:
    return USER_ERROR(RT_BAD_FUNC);
  }
  return 0;
}

int FARFN rtdefpagebus_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 PageBus)
{
  int num;
  unsigned type;
  unsigned page;

  CLRtmkError;
  num = __tmkNumber_usb;
  page = PageBus;
  CHECK_RT_PAGE_BX(num, page);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
  case __TA:
    return USER_ERROR(RT_BAD_FUNC);
  }
  return 0;
}

U16 FARFN rtgetpagepc_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber_usb;
  return __rtPagePC_usb[num];
}

U16 FARFN rtgetpagebus_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;

  CLRtmkError;
  num = __tmkNumber_usb;
  return __rtPageBus_usb[num];
}

U16 FARFN rtenable_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 rtEnDis)
{
  int num;
  unsigned type;
  unsigned mask, maskbrc;
  u16 buf_out[12];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (rtEnDis)
  {
  case RT_GET_ENABLE:
    return (__rtDisableMask_usb[num]) ? RT_DISABLE : RT_ENABLE;
  case RT_ENABLE:
  case RT_DISABLE:
    mask = rtEnDis;
    maskbrc = (unsigned)(-1);
    if (mask)
    {
      mask = __RT_DIS_MASK_usb[type];
      maskbrc = __RT_BRC_MASK_usb[type];
    }
    if (mask == __rtDisableMask_usb[num])
      break;
    __rtDisableMask_usb[num] = mask;
    __rtBRCMask_usb[num] = maskbrc;
    switch (type)
    {
    case __TA:
      GET_MUTEX;
      GET_DIS_IRQ_SMP();
//      outpw(TA_MODE1(num), __rtControls[num] & ~TA_RTMT_START);
      buf_out[0] = WRITE_RG;
      buf_out[1] = TA_MODE1(num);
      buf_out[2] = __rtControls_usb[num] & ~TA_RTMT_START;
//      outpw(TA_MODE2(num), (__rtControls1[num])); // & maskbrc) | mask;
      buf_out[3] = WRITE_RG;
      buf_out[4] = TA_MODE2(num);
      buf_out[5] = __rtControls1_usb[num];
      __rtControls_usb[num] = (__rtControls_usb[num] & ~TA_RTMT_START) | (TA_RTMT_START & (maskbrc>>1));
//      outpw(TA_MODE1(num), __rtControls[num]);
      buf_out[6] = WRITE_RG;
      buf_out[7] = TA_MODE1(num);
      buf_out[8] = __rtControls_usb[num];
      buf_out[9] = READ_RG;
      buf_out[10] = TA_BASE(num);
      buf_out[11] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
      REST_IRQ_SMP();
      REST_MUTEX;
      break;
    }
    break;
  default:
    return USER_ERROR(RT_BAD_FUNC);
  }
  return 0;
}

int FARFN rtdefaddress_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Address)
{
  int num;
  unsigned type;
  unsigned rtaddr;
  u16 buf_out[12];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  rtaddr = Address;
  CHECK_RT_ADDRESS(rtaddr);
  CHECK_TMK_DEVICE(num);
  __rtAddress_usb[num] = rtaddr;
  if (__rtDisableMask_usb[num])
  {
    if (__rtEnableOnAddr_usb[num])
    {
      __rtDisableMask_usb[num] = 0;  //; RT_ENABLE
      __rtBRCMask_usb[num] = 0xFFFF;
    }
  }
  switch (type)
  {
  case __TA:
    rtaddr <<= 11;
    GET_MUTEX;
    GET_DIS_IRQ_SMP();
//    outpw(TA_MODE1(num), __rtControls[num] & ~TA_RTMT_START);
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_MODE1(num);
    buf_out[2] = __rtControls_usb[num] & ~TA_RTMT_START;
    rtaddr |= __rtControls1_usb[num] & 0x07FF;
    __rtControls1_usb[num] = rtaddr;
    __rtControls_usb[num] = (__rtControls_usb[num] & ~TA_RTMT_START) | (TA_RTMT_START & (__rtBRCMask_usb[num]>>1));
//    rtaddr &= __rtBRCMask[num];
//    rtaddr |= __rtDisableMask[num];

//    outpw(TA_MODE2(num), rtaddr);
    buf_out[3] = WRITE_RG;
    buf_out[4] = TA_MODE2(num);
    buf_out[5] = rtaddr;
//    outpw(TA_MODE1(num), __rtControls[num]);
    buf_out[6] = WRITE_RG;
    buf_out[7] = TA_MODE1(num);
    buf_out[8] = __rtControls_usb[num];
    buf_out[9] = READ_RG;
    buf_out[10] = TA_BASE(num);
    buf_out[11] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    REST_IRQ_SMP();
    REST_MUTEX;
    break;
  }
  return 0;
}

U16 FARFN rtgetaddress_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (type)
  {
  case __TA:
    buf_out[0] = READ_RG;
    buf_out[1] = TA_MODE2(num);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    return buf_in[2] >> 11;
  }
  return __rtAddress_usb[num];
}

void FARFN rtgetflags_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
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
  __rtSubAddr_usb[num] = dir;
  pos |= __hm400Page_usb[num];
  buf = (U16 FARDT*)pcBufAddr;
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  port = __tmkPortsAddr1_usb[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos = FlagMin;
    do
    {
#ifdef STATIC_TMKNUM
      *(buf++) = rtgetflag_usb(dir, pos);
#else
      *(buf++) = rtgetflag_usb(realnum, dir, pos);
#endif
      ++pos;
    }
    while (--len);
    break;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

//__EXTERN void __FAR rtgetflags(void __FAR *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
//__EXTERN void __FAR rtputflags(void __FAR *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
void FARFN rtputflags_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
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
  u16 buf_out[3];
  u16 buf_in[4];
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
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
  __rtSubAddr_usb[num] = dir;
  pos |= __hm400Page_usb[num];
  buf = (U16 FARDT*)pcBufAddr;
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEV(realnum);
  port = __tmkPortsAddr1_usb[realnum];
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  switch (type)
  {
  case __TA:
    pos = FlagMin;
    do
    {
#ifdef STATIC_TMKNUM
      rtputflag_usb(dir, pos, *(buf++));
#else
      rtputflag_usb(realnum, dir, pos, *(buf++));
#endif
      ++pos;
    }
    while (--len);
    buf_out[0] = READ_RG;
    buf_out[1] = TA_BASE(realnum);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    break;
  }
  __tmkRAMInWork_usb[realnum] = 0;
  return;
}

void FARFN rtsetflag_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned sa;
  u16 buf_out[8];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEV(num);
  sa = __rtSubAddr_usb[num];
  switch (type)
  {
/*  default:
    pos = ((sa >> 5) | sa) & 0x041F;
    DrvRtPoke(num, type, pos, 0x8000);
    DrvRtDefSubAddr(num, type, sa);
    break;*/
  case __TA:
    sa >>= 5;
    if (!__FLAG_MODE_ON_usb[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG_usb[num][sa&0x1f] = 0x8000;
      else
        __BC_RT_FLAG_usb[num][sa&0x1f] = 0x8000;
    }
    else
    {
      GET_DIS_IRQ_SMP();
////      data = DrvRtPeekTA(num, AdrTab, sa);
//      outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//      data = inpw(TA_DATA(num));
      if (sa & 0x20)//true
      {
        buf_out[0] = MOD_MEM_AND;
        buf_out[1] = 1;
        buf_out[2] = (AdrTab << 6) + sa;
        buf_out[3] = 0x7FFF;
        buf_out[4] = 0;
//        data &= 0x7FFF;
      }
      else
      {
        buf_out[0] = MOD_MEM_AND;
        buf_out[1] = 1;
        buf_out[2] = (AdrTab << 6) + sa;
        buf_out[3] = 0xFFFF;
        buf_out[4] = 0x8000;
//        data |= 0x8000;
      }
      buf_out[5] = READ_RG;
      buf_out[6] = TA_BASE(num);
      buf_out[7] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
////      DrvRtPokeTA(num, AdrTab, sa, data);
//      outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//      outpw(TA_DATA(num), data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr_usb(num, type, sa);
    }
    break;
  }
  return;
}

void FARFN rtclrflag_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;
  unsigned sa;
  u16 buf_out[8];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEV(num);
  sa = __rtSubAddr_usb[num];
  switch (type)
  {
/*  default:
    pos = ((sa >> 5) | sa) & 0x041F;
    DrvRtPoke(num, type, pos, 0);
    DrvRtDefSubAddr(num, type, sa);
    break;*/
  case __TA:
    sa >>= 5;
    if (!__FLAG_MODE_ON_usb[num])
    {
      if (sa & 0x20)
        __RT_BC_FLAG_usb[num][sa&0x1f] = 0x0000;
      else
        __BC_RT_FLAG_usb[num][sa&0x1f] = 0x0000;
    }
    else
    {
      GET_DIS_IRQ_SMP();
////      data = DrvRtPeekTA(num, AdrTab, sa);
//      outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//      data = inpw(TA_DATA(num));
      if (sa & 0x20)
      {
        buf_out[0] = MOD_MEM_AND;
        buf_out[1] = 1;
        buf_out[2] = (AdrTab << 6) + sa;
        buf_out[3] = 0xFFFF;
        buf_out[4] = 0x8000;
//        data |= 0x8000;
      }
      else
      {
        buf_out[0] = MOD_MEM_AND;
        buf_out[1] = 1;
        buf_out[2] = (AdrTab << 6) + sa;
        buf_out[3] = 0x7FFF;
        buf_out[4] = 0;
//        data &= 0x7FFF;
      }
      buf_out[5] = READ_RG;
      buf_out[6] = TA_BASE(num);
      buf_out[7] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
////      DrvRtPokeTA(num, AdrTab, sa, data);
//      outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//      outpw(TA_DATA(num), data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
  }
  return;
}

void FARFN rtputflag_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Dir, U16 SubAddr, U16 Flag)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;
  unsigned pos;
  u16 buf_out[8];
  u16 buf_in[4];

  CLRtmkError;
  num = __tmkNumber_usb;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  CHECK_TMK_DEVICEV(num);
  dir = Dir;
  CHECK_RT_DIR(dir);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  switch (type)
  {
/*  default:
    pos = sa | dir;
    DrvRtPoke(num, type, pos, Flag);
    DrvRtDefSubAddr(num, type, sa);
    break;*/
  case __TA:
    if (!__FLAG_MODE_ON_usb[num])
    {
      if (dir)
        __RT_BC_FLAG_usb[num][sa] = Flag;
      else
        __BC_RT_FLAG_usb[num][sa] = Flag;
    }
    else
    {
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
////      data = DrvRtPeekTA(num, AdrTab, pos);
//      outpw(TA_ADDR(num), (AdrTab << 6) + pos);
//      data = inpw(TA_DATA(num));
      if (Flag & 0x8000)
      {
        if (dir)
        {
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + pos;
          buf_out[3] = 0x7FFF;
          buf_out[4] = 0;
//          data &= 0x7FFF;
        }
        else
        {
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + pos;
          buf_out[3] = 0xFFFF;
          buf_out[4] = 0x8000;
//          data |= 0x8000;
        }
      }
      else
      {
        if (dir)
        {
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + pos;
          buf_out[3] = 0xFFFF;
          buf_out[4] = 0x8000;
//          data |= 0x8000;
        }
        else
        {
          buf_out[0] = MOD_MEM_AND;
          buf_out[1] = 1;
          buf_out[2] = (AdrTab << 6) + pos;
          buf_out[3] = 0x7FFF;
          buf_out[4] = 0;
//          data &= 0x7FFF;
        }
      }
      buf_out[5] = READ_RG;
      buf_out[6] = TA_BASE(num);
      buf_out[7] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
////      DrvRtPokeTA(num, AdrTab, pos, data);
//      outpw(TA_ADDR(num), (AdrTab << 6) + pos);
//      outpw(TA_DATA(num), data);
      REST_IRQ_SMP();
//      DrvRtDefSubAddr(num, type, sa);
    }
    break;
  }
  return;
}

U16 FARFN rtgetflag_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Dir, U16 SubAddr)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;
  unsigned pos;
  unsigned flag = 0;
  u16 buf_out[7];
  u16 buf_in[11];

  CLRtmkError;
  num = __tmkNumber_usb;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  dir = Dir;
  CHECK_RT_DIR(dir);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (type)
  {
/*  default:
    pos = sa | dir;
    flag = DrvRtPeek(num, type, pos);
    break;*/
  case __TA:
    if (!__FLAG_MODE_ON_usb[num])
    {
      if (dir)
        flag = __RT_BC_FLAG_usb[num][sa];
      else
        flag = __BC_RT_FLAG_usb[num][sa];
    }
    else
    {
      pos = sa | (dir >> 5);
      GET_DIS_IRQ_SMP();
////      flag = DrvRtPeekTA(num, AdrTab, pos);
//      outpw(TA_ADDR(num), (AdrTab << 6) + pos);
//      flag = inpw(TA_DATA(num));
      buf_out[0] = READ_MEM;
      buf_out[1] = 2;
      buf_out[2] = (AdrTab << 6) + pos;
      buf_out[3] = READ_MEM;
      buf_out[4] = 2;
      buf_out[5] = (pos << 6) + 58;
      buf_out[6] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
      flag = buf_in[3];
      if (dir)
      {
        flag = (~flag) & 0x8000;
      }
      else
      {
        flag = flag & 0x8000;
      }
////      flag |= DrvRtPeekTA(num, pos, 58) & 0x07FF;
//      outpw(TA_ADDR(num), (pos << 6) + 58);
//      flag |= inpw(TA_DATA(num)) & 0x07FF;
      flag |= buf_in[8] & 0x07FF;
      REST_IRQ_SMP();
    }
    break;
  }
  DrvRtDefSubAddr_usb(num, type, (sa << 5) | dir);
  return flag;
}

int FARFN rtbusy_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;
  unsigned type;
  unsigned port;
  unsigned state = 0;
  u16 buf_out[3];
  u16 buf_in[4];

  CLRtmkError;
  realnum = GET_RealNum;
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICE(realnum);
  port = __tmkPortsAddr1_usb[realnum];
  switch (type)
  {
  case __TA:
//    state = inpw(TA_BASE(realnum));
    buf_out[0] = READ_RG;
    buf_out[1] = TA_BASE(realnum);
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    state = buf_in[2];
    if (((state & 0x03FF) == (DrvRtGetBaseTA_usb(realnum) & 0x03FF)))
      state >>= 2;
    else
      state = 0;
    break;
  }
  return (state >> 11) & 1;
}

U16 FARFN rtgetstate_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num, realnum;
  unsigned type;
  unsigned port;
  unsigned state = 0, statex;
  u16 buf_out[5];
  u16 buf_in[7];
//  unsigned pos;
  IRQ_FLAGS;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  realnum = GET_RealNum;
  CHECK_TMK_DEVICEN(realnum);
  port = __tmkPortsAddr1_usb[realnum];
  switch (type)
  {
  case __TA:
//    state = inpw(TA_LCW(realnum)) & 0x7FF;
//    statex = inpw(TA_BASE(realnum));
    buf_out[0] = READ_RG;
    buf_out[1] = TA_LCW(realnum);
    buf_out[2] = READ_RG;
    buf_out[3] = TA_BASE(realnum);
    buf_out[4] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);
    state = buf_in[2] & 0x7FF;
    statex = buf_in[5];
    if ((statex&0x03FF) == (DrvRtGetBaseTA_usb(realnum)&0x03FF))
      state |= (statex & 0x2000) >> 2;
    break;
  }
  return state;
}

void FARFN rtlock_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 Dir, U16 SubAddr)
{
  int num;
  unsigned type;
  unsigned sa;
  unsigned dir;

  CLRtmkError;
  num = __tmkNumber_usb;
  sa = SubAddr;
  CHECK_RT_SUBADDR(sa);
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEV(num);
  sa <<= 5;
  dir = Dir;
  CHECK_RT_DIR(dir);
  sa |= dir;
  DrvRtDefSubAddr_usb(num, type, sa);
  DrvRtWMode_usb(num, type, sa | 0x0800);
  return;
}

void FARFN rtunlock_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int num;
  unsigned type;

  CLRtmkError;
  num = __tmkNumber_usb;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEV(num);
  DrvRtWMode_usb(num, type, __rtSubAddr_usb[num]);
  return;
}

U16 FARFN rtgetlock_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber
#endif
        )
{
  int num;
  unsigned mode;

  CLRtmkError;
  num = __tmkNumber;   //; tmkRealNumber2 ???
  mode = __rtMode_usb[num];
  return (mode & 0xFC00) | ((mode >> 5) & 0x001F);
}

U16 FARFN rtgetcmddata_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 BusCommand)
{
  int num;
  unsigned type;
  u16 buf_out[6];
  u16 buf_in[4];
  DEF_VAR(unsigned, cmd);

  CLRtmkError;
  num = __tmkNumber_usb;
  GET_VAR(cmd, BusCommand);
  CHECK_RT_CMD(cmd);
  cmd |= 0x03E0;
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEN(num);
  switch (type)
  {
  case __TA:
// very simple assuming SyncWData commands only
// need to mantain an array of cmddatas for full compatibility
    {
    unsigned data;
    GET_DIS_IRQ_SMP();
////    data = DrvRtPeekTA(num, 0x1F, 0);
//    outpw(TA_ADDR(num), (0x1F << 6) + 0);
//    data = inpw(TA_DATA(num));
    buf_out[0] = WRITE_RG;
    buf_out[1] = TA_ADDR(num);
    buf_out[2] = (0x1F << 6);
    buf_out[3] = READ_RG;
    buf_out[4] = TA_DATA(num);
    buf_out[5] = 0xFFFF;
    Block_out_in(minor_table[num], buf_out, buf_in);
    data = buf_in[2];
    REST_IRQ_SMP();
    return (U16)data;
    }
  }
  return 0;
}

void FARFN rtputcmddata_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 BusCommand, U16 rtData)
{
  int num;
  unsigned type;
  u16 buf_out[9];
  u16 buf_in[4];
  DEF_VAR(unsigned, cmd);

  CLRtmkError;
  num = __tmkNumber_usb;
  GET_VAR(cmd, BusCommand);
  CHECK_RT_CMD(cmd);
  cmd |= 0x03E0;        //;or      ax, 7F0h
  type = __tmkDrvType_usb[num];
  CHECK_TMK_TYPE(type);
  CHECK_TMK_DEVICEV(num);
  switch (type)
  {
  case __TA:
// need to mantain an array of cmddatas for full compatibility
    GET_DIS_IRQ_SMP();
    if (cmd == 0x7F0) //0x410
    {
////      DrvRtPokeTA(num, 0x20, 0, rtData);
//      outpw(TA_ADDR(num), (0x20 << 6) + 0);
//      outpw(TA_DATA(num), rtData);
      buf_out[0] = WRITE_RG;
      buf_out[1] = TA_ADDR(num);
      buf_out[2] = (0x20 << 6);
      buf_out[3] = WRITE_RG;
      buf_out[4] = TA_DATA(num);
      buf_out[5] = rtData;
      buf_out[6] = READ_RG;
      buf_out[7] = TA_DATA(num);
      buf_out[8] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
    }
    else if (cmd == 0x7F3) //0x413
    {
////      DrvRtPokeTA(num, 0x3F, 0, rtData);
//      outpw(TA_ADDR(num), (0x3F << 6) + 0);
//      outpw(TA_DATA(num), rtData);
      buf_out[0] = WRITE_RG;
      buf_out[1] = TA_ADDR(num);
      buf_out[2] = (0x3F << 6);
      buf_out[3] = WRITE_RG;
      buf_out[4] = TA_DATA(num);
      buf_out[5] = rtData;
      buf_out[6] = READ_RG;
      buf_out[7] = TA_DATA(num);
      buf_out[8] = 0xFFFF;
      Block_out_in(minor_table[num], buf_out, buf_in);
    }
    REST_IRQ_SMP();
    break;
  }
  return;
}

void DrvRtWMode_usb(int __tmkNumber_usb, unsigned type, unsigned mode)
{
  int num, realnum;
  unsigned sa;
  u16 buf_out[8];
  u16 buf_in[4];
  IRQ_FLAGS;

  realnum = GET_RealNum;
  if (mode == __rtMode_usb[realnum])
    return;
  num = __tmkNumber_usb;
  __rtMode_usb[num] = mode;
  switch (type)
  {
  case __TA:
    if (__FLAG_MODE_ON_usb[num])
      return;
    sa = (mode >> 5) & 0x3F;
    GET_DIS_IRQ_SMP();
////    DrvRtPokeTA(num, AdrTab, sa, (DrvRtPeekTA(num, AdrTab, sa) & 0x7FFF) | ((mode << 4) & 0x8000));
//    outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//    data2 = inpw(TA_DATA(num));
//    outpw(TA_ADDR(num), (AdrTab << 6) + sa);
//    outpw(TA_DATA(num), (data2 & 0x7FFF) | ((mode << 4) & 0x8000));
    buf_out[0] = MOD_MEM_AND;
    buf_out[1] = 1;
    buf_out[2] = (AdrTab << 6) + sa;
    buf_out[3] = 0x7FFF;
    buf_out[4] = (mode << 4) & 0x8000;
    buf_out[5] = READ_RG;
    buf_out[6] = TA_BASE(realnum);
    buf_out[7] = 0xFFFF;
    Block_out_in(minor_table[realnum], buf_out, buf_in);

//    sa = DrvRtPeekTA(num, AdrTab, sa);
    REST_IRQ_SMP();
    return;
  }
  return;
}

void DrvRtPoke_usb(int __tmkNumber_usb, unsigned type, unsigned pos, unsigned data)
// not for TA boards!
// write to pos in current page, update current subaddr
{
  int register num, realnum;
  unsigned port;
  unsigned save_rama, save_ramiw;
  IRQ_FLAGS;

  num = __tmkNumber_usb;
  __rtSubAddr_usb[num] = pos & 0x07E0;
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  port = __tmkPortsAddr1_usb[realnum];
  switch (type)
  {
//  case __TA:
//  ???
//    break;
//  case __MRTA:
//  ???
//    break;
    default:
    break;
  }
  return;
}

unsigned DrvRtPeek_usb(int __tmkNumber_usb, unsigned type, unsigned pos)
// not for TA boards!
// read from pos in current page, update current subaddr
{
  int register num, realnum;
  unsigned data = 0;
  unsigned port;
  unsigned save_rama, save_ramiw;

  num = __tmkNumber_usb;
  __rtSubAddr_usb[num] = pos & 0x07E0;
  pos |= __hm400Page_usb[num];
  realnum = GET_RealNum;
  save_rama = __tmkRAMAddr_usb[realnum];
  save_ramiw = __tmkRAMInWork_usb[realnum];
  __tmkRAMInWork_usb[realnum] = 1;
  port = __tmkPortsAddr1_usb[realnum];
  switch (type)
  {
//  case __TA:
//  ???
//    break;
//  case __MRTA:
//  ???
//    break;
    default:
    break;
  }
  return data;
}

int ExamTmkRAM_usb(int num, unsigned type)
{
  __mtMaxBase_usb[num] = 0;
  switch (type)
  {
  case __TA:
    __tmkRAMSize_usb[num] = 64;
    __bcMaxBase_usb[num] = (1023 <= DRV_MAX_BASE) ? 1023 : DRV_MAX_BASE;
    __mtMaxBase_usb[num] = (511 <= DRV_MAX_BASE) ? 511 : DRV_MAX_BASE;
    __rtMaxPage_usb[num] = 0;
    break;
  }
  return 0;
}

void DrvInitAll_usb(void)
{
  int i;

  for (i = 0; i < NTMK; ++i)
  {
    __tmkTimeOut_usb[i] = 0;
    __tmkTimerCtrl_usb[i] = 0;
    __rtGap_usb[i] = 0;
    __tmkHWVer_usb[i] = 0;
//      fTmkEventSet[i] = 0;
    __wInDelay_usb[i] = 1;
    __wOutDelay_usb[i] = 1;
    __tmkRAMInWork_usb[i] = 0;
    __tmkRAMAddr_usb[i] = 0;
    __tmkStarted_usb[i] = 0;
    __bcBus_usb[i] = 0;
    __bcMaxBase_usb[i] = 0;
    __mtMaxBase_usb[i] = 0;
  }
  for (i = 0; i < (NTMK + NRT); ++i)
  {
#ifdef STATIC_TMKNUM
    tmkError_usb = 0;
#else
    tmkError_usb[i] = 0;
#endif
    __amrtNumber_usb[i] = 0;
    __tmkDrvType_usb[i] = 0xFFFF;
    __tmkUserType_usb[i] = 0xFFFF;
    __tmkMode_usb[i] = 0xFFFF;
    __rtControls_usb[i] = 0;
    __rtControls1_usb[i] = 0;
    __rtAddress_usb[i] = 0x00FF;
    __rtMaxPage_usb[i] = 0;
    __rtMode_usb[i] = 0;
    __rtSubAddr_usb[i] = 0;
    __hm400Page_usb[i] = 0;
    __rtEnableOnAddr_usb[i] = 1;
    __FLAG_MODE_ON_usb[i] = 0;
  }
#ifdef NMBCID
  for (i = 0; i < NMBCID; ++i)
  {
    __mbcAlloc_usb[i] = 0;
  }
#endif //def NMBCID
}

void DrvInitTmk_usb(int num, unsigned type)
{
  u16 buf_out[48];
  u16 buf_in[64];
  unsigned i;

  switch (type)
  {
  case __TA:
    GET_DIS_IRQ_SMP();
//    outpw(TA_RESET(num), 0);
    buf_out[0] = 0;
    buf_out[1] = TA_RESET(num);
    buf_out[2] = 0;
//    outpw(TA_MODE1(num), TA_FIFO_RESET);
    buf_out[3] = 0;
    buf_out[4] = TA_MODE1(num);
    buf_out[5] = TA_FIFO_RESET;
    buf_out[6] = 0;
    buf_out[7] = TA_MODE1(num);
    buf_out[8] = 0;//TA_FIFO_RESET

    buf_out[9] = 1;
    buf_out[10] = TA_IRQ(num);
    buf_out[11] = 1;
    buf_out[12] = TA_IRQ(num);
    buf_out[13] = 1;
    buf_out[14] = TA_IRQ(num);
    buf_out[15] = 1;
    buf_out[16] = TA_IRQ(num);
    buf_out[17] = 1;
    buf_out[18] = TA_IRQ(num);
    buf_out[19] = 1;
    buf_out[20] = TA_IRQ(num);
    buf_out[21] = 1;
    buf_out[22] = TA_IRQ(num);
    buf_out[23] = 1;
    buf_out[24] = TA_IRQ(num);
    buf_out[25] = 1;
    buf_out[26] = TA_IRQ(num);
    buf_out[27] = 1;
    buf_out[28] = TA_IRQ(num);
    buf_out[29] = 1;
    buf_out[30] = TA_IRQ(num);
    buf_out[31] = 1;
    buf_out[32] = TA_IRQ(num);
    buf_out[33] = 1;
    buf_out[34] = TA_IRQ(num);
    buf_out[35] = 1;
    buf_out[36] = TA_IRQ(num);
    buf_out[37] = 1;
    buf_out[38] = TA_IRQ(num);
    buf_out[39] = 1;
    buf_out[40] = TA_IRQ(num);
    buf_out[41] = 0xFFFF;

    Block_out_in(minor_table[num], buf_out, buf_in);

    for (i = 2; i < 48; i+=3)
    {
//      Register_input_bl(__tmkNumber, port, &data_in);
      __tmkHWVer_usb[num] |= ((buf_in[i] & 0x1000) >> 12) << ((i-2)/3);
    }
//    outpw(TA_MODE1(num), 0);
    REST_IRQ_SMP();
//    port = TA_IRQ(num);  //because of TA1-USB irq polling
//    for (i = 0; i < 16; ++i)
//      __tmkHWVer[num] |= ((inpw(port) & 0x1000) >> 12) << i;
    break;
  }
}

U16 FARFN tmkgethwver_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb
#endif
        )
{
  int realnum;

  CLRtmkError;
  realnum = GET_RealNum;
  return __tmkHWVer_usb[realnum];
}

int FARFN tmkconfig_usb(int hTMK, U16 wType, U16 PortsAddr1, U16 PortsAddr2, U08 Irq1, U08 Irq2)
{
#ifndef STATIC_TMKNUM
  int __tmkNumber; // fake __tmkNumber
#endif
  int num;
  unsigned type;
  int res;

//  spin_lock(&tmkSpinLock);
  if (__tmkFirstTime_usb)
  {
    __tmkFirstTime_usb = 0;
    DrvInitAll_usb();
  }
//  spin_unlock(&tmkSpinLock);

//;  CLRtmkError;
  num = hTMK;
  CHECK_TMK_REAL_NUMBER(num);
  if (__tmkDrvType_usb[num] != 0xFFFF)
  {
    return USER_ERROR_R(TMK_BAD_TYPE);
  }
  __tmkNumber = num;
  CLRtmkError;
  PUT_RealNum(num);
  __tmkMode_usb[num] = UNDEFINED_MODE;//!!!send
  type = wType;
  CHECK_TMK_TYPE_1(type); // special if QNX4VME
  __tmkUserType_usb[num] = type;
  type = __tmkUser2DrvType_usb[type];
  __tmkDrvType_usb[num] = type;
  __RT_DIS_MASK_usb[type] = __RT_DIS_MASK0_usb[__tmkUserType_usb[num]];
  __RT_BRC_MASK_usb[type] = __RT_BRC_MASK0_usb[__tmkUserType_usb[num]];
  __rtDisableMask_usb[num] = __RT_DIS_MASK_usb[type];
  __rtBRCMask_usb[num] = __RT_BRC_MASK_usb[type];
  __rtEnableOnAddr_usb[num] = 1;
  __tmkPortsAddr1_usb[num] = PortsAddr1;
  __tmkPortsAddr2_usb[num] = PortsAddr2;
  DrvInitTmk_usb(num, type);
  res = ExamTmkRAM_usb(num, type);
  if (res)
    return res;
  switch (type)
  {
  case __TA:
    __rtControls1_usb[num] = TA_HBIT_MODE + TA_BRCST_MODE;
    __FLAG_MODE_ON_usb[num] = 0;
//  ???
    break;
  }
  __amrtNumber_usb[num] = num;
//  Irq2;
  return 0;
}

int FARFN tmkdone_usb(int hTMK)
{
  int num;
  unsigned type;
  int ntmk;

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
    type = __tmkDrvType_usb[num];
    if ((unsigned)type == 0xFFFF)
      continue;
//;    CHECK_TMK_TYPE(type);
    __tmkDrvType_usb[num] = 0xFFFF;
//    tmkUserType[num] = 0xFFFF;
    __rtDisableMask_usb[num] = __RT_DIS_MASK_usb[type];
    __rtBRCMask_usb[num] = __RT_BRC_MASK_usb[type];
    __rtEnableOnAddr_usb[num] = 1;
  }
  while (++num < ntmk);
  return 0;
}

/*unsigned DIRQLTmkSave_usb(int hTMK)
{
  u16 buf_out[3];
  u16 buf_in[4];
  if (__tmkDrvType_usb[hTMK] == __TA)
  {
    buf_out[0] = 1;
    buf_out[1] = __tmkPortsAddr1_usb[hTMK] + TMK_AddrPort;
    buf_out[2] = 0xFFFF;
    Block_out_in(minor_table[hTMK], buf_out, buf_in);
    return buf_in[2];
  }
  else
    return 0;
}

void DIRQLTmkRestore_usb(int hTMK, unsigned Saved)
{
  u16 buf_out[4];
  if (__tmkDrvType_usb[hTMK] == __TA)
  {
    buf_out[0] = 0;
    buf_out[1] = __tmkPortsAddr1_usb[hTMK] + TMK_AddrPort;
    buf_out[2] = Saved;
    buf_out[3] = 0xFFFF;
    Block_out_in(minor_table[hTMK], buf_out, NULL);
//    outpw(__tmkPortsAddr1[hTMK] + TMK_AddrPort, Saved);
  }
  return;
}*/

/*void DpcIExcBC(int hTMK, void *pEvData)
{
  int num;
  unsigned type;
  unsigned pos;
  unsigned base;
  unsigned basepc;
  U16 *pevd;
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
}*/

#ifdef NMBCID

U16 FARFN mbcalloc_usb()
{
  unsigned i;
  for (i = 0; i < NMBCID; ++i)
  {
    if (__mbcAlloc_usb[i] == 0)
    {
      __mbcAlloc_usb[i] = 1;
      return (U16)i;
    }
  }
  return MBC_ALLOC_FAIL;
}

int FARFN mbcfree_usb(U16 mbcId)
{
  if (mbcId >= NMBCID || __mbcAlloc_usb[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  __mbcAlloc_usb[mbcId] = 0;
  return 0;
}

int FARFN mbcinit_usb(U16 mbcId)
{
  if (mbcId >= NMBCID || __mbcAlloc_usb[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  __mbci_usb[mbcId] = 0;
  return 0;
}

int FARFN mbcpreparex_usb(
#ifndef STATIC_TMKNUM
        int __tmkNumber_usb,
#endif
        U16 mbcId, U16 bcBase, U16 bcCtrlCode, U16 mbcDelay)
{
  int realnum;
  unsigned type;
  unsigned code;
  unsigned base;
  int i;
  u16 buf_out[12];
  IRQ_FLAGS;

  if (mbcId >= NMBCID || __mbcAlloc_usb[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  CLRtmkError;
  realnum = GET_RealNum;
  CHECK_TMK_DEVICE(realnum);
  CHECK_TMK_MODE(realnum, BC_MODE);
  code = bcCtrlCode;
  CHECK_BC_CTRLX(code);
  type = __tmkDrvType_usb[realnum];
  CHECK_TMK_TYPE(type);
  base = bcBase;
  CHECK_BCMT_BASE_BX(realnum, base);
  for (i = 0; i < __mbci_usb[mbcId]; ++i)
  {
    if (__mbcTmkN_usb[mbcId][i] == realnum)
      break;
  }
  __mbcTmkN_usb[mbcId][i] = realnum;
  __mbcBase_usb[mbcId][i] = base;
  switch (type)
  {
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
      code1 = __bcLinkCCN_usb[realnum][base];
      if (code1 & CX_SIG)
        ContrW |= 0x8000;

      GET_DIS_IRQ_SMP();
//      outpw(TA_ADDR(realnum), (base<<6) | 61);
//      outpw(TA_DATA(realnum), ContrW);
//      outpw(TA_DATA(realnum), mbcDelay);
      buf_out[0] = 2;
      buf_out[1] = 2;
      buf_out[2] = (base<<6) | 61;
      buf_out[3] = ContrW;
      buf_out[4] = mbcDelay;
      buf_out[5] = 0xFFFF;
      REST_IRQ_SMP();
#ifdef DRV_EMULATE_FIRST_CX_SIG
//emulation is through special base 0x3ff
//also it could be a good (or bad) idea to block irq output
//and further poll it until it occurs
      if (code & CX_SIG)
      {
        GET_DIS_IRQ_SMP();
//        outpw(TA_ADDR(realnum), (0x03FF<<6) | 61);
//        port = TA_DATA(realnum);
//        outpw(port, 0xA020);
//        outpw(port, 0);
//        outpw(port, base);
        buf_out[5] = 2;
        buf_out[6] = 3;
        buf_out[7] = (0x03FF<<6) | 61;
        buf_out[8] = 0xA020;
        buf_out[9] = 0;
        buf_out[10] = base;
        buf_out[11] = 0xFFFF;
        REST_IRQ_SMP();
        base = 0x03FF;
      }
#endif //def DRV_EMULATE_FIRST_CX_SIG
      Block_out_in(minor_table[realnum], buf_out, NULL);
      __mbcPort0_usb[mbcId][i] = TA_MSGA(realnum);
      __mbcData0_usb[mbcId][i] = base & 0x03FF;
      __mbcPort_usb[mbcId][i] = TA_MODE2(realnum);
      __mbcData_usb[mbcId][i] = __bcControls1_usb[realnum] | TA_BC_START;
      if (i == __mbci_usb[mbcId])
        ++__mbci_usb[mbcId];
    }
    break;
  }
  return 0;

}

int FARFN mbcstartx_usb(U16 mbcId)
{
  int realnum;
  unsigned type;
  unsigned port;
  register int i, ni;
#ifndef STATIC_TMKNUM
  int __tmkNumber_usb;
#endif
  u16 buf_out[19];
  IRQ_FLAGS;

  if (mbcId >= NMBCID || __mbcAlloc_usb[mbcId] == 0)
    return USER_ERROR_R(TMK_BAD_NUMBER);
  ni = (int)__mbci_usb[mbcId];
  for (i = 0; i < ni; ++i)
  {
    realnum = __mbcTmkN_usb[mbcId][i];
    __tmkNumber_usb = realnum;
    CLRtmkError;
    type = __tmkDrvType_usb[realnum];
//    CHECK_TMK_TYPE(type);
    __bcBaseBus_usb[realnum] = __mbcBase_usb[mbcId][i];//!!!send
    __bcXStart_usb[realnum] = 1;//!!!send
    buf_out[0] = 10;
    buf_out[1] = 2;
    buf_out[2] = __mbcBase_usb[mbcId][i];
    buf_out[3] = 10;
    buf_out[4] = 4;
    buf_out[5] = 1;
    buf_out[6] = 0xFFFF;
    switch (type)
    {
    case __TA:
      if (__tmkStarted_usb[realnum])
      {
        GET_DIS_IRQ_SMP();
//        outpw(TA_RESET(realnum), 0);
//        outpw(TA_TIMCR(realnum), __tmkTimerCtrl_usb[realnum]);
//        outpw(TA_MODE1(realnum), __bcControls_usb[realnum]);
        REST_IRQ_SMP();
//        outpw(TA_MODE2(realnum), __bcControls1_usb[realnum]);
        buf_out[6] = 0;
        buf_out[7] = TA_RESET(realnum);
        buf_out[8] = 0;
        buf_out[9] = 0;
        buf_out[10] = TA_TIMCR(realnum);
        buf_out[11] = __tmkTimerCtrl_usb[realnum];
        buf_out[12] = 0;
        buf_out[13] = TA_MODE1(realnum);
        buf_out[14] = __bcControls_usb[realnum];
        buf_out[15] = 0;
        buf_out[16] = TA_MODE2(realnum);
        buf_out[17] = __bcControls1_usb[realnum];
        buf_out[18] = 0xFFFF;
      }
//      __tmkStarted_usb[realnum] = 1;
      break;
    }
    Block_out_in(minor_table[realnum], buf_out, NULL);
  }
  for (i = 0; i < ni; ++i)
  {
    if ((port = __mbcPort0_usb[mbcId][i]) != 0)
    {
//      outpw(port, __mbcData0_usb[mbcId][i]);
      buf_out[0] = 0;
      buf_out[1] = port;
      buf_out[2] = __mbcData0_usb[mbcId][i];
      buf_out[3] = 0xFFFF;
      Block_out_in(minor_table[__mbcTmkN_usb[mbcId][i]], buf_out, NULL);
    }
  }
  GET_DIS_IRQ();
  for (i = 0; i < ni; ++i)
  {
//    outpw(__mbcPort_usb[mbcId][i], __mbcData_usb[mbcId][i]);
      buf_out[0] = 0;
      buf_out[1] = __mbcPort_usb[mbcId][i];
      buf_out[2] = __mbcData_usb[mbcId][i];
      buf_out[3] = 0xFFFF;
      Block_out_in(minor_table[__mbcTmkN_usb[mbcId][i]], buf_out, NULL);
  }
  REST_IRQ();
  return 0;
}

#endif //def NBCID
