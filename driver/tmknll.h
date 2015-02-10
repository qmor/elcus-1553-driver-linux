
/****************************************************************************/
/*      TMKNLL.H v7.06. ELCUS, 1995,2011                                    */
/*      Interface to the driver TMKNLL v7.06.                               */
/****************************************************************************/

#ifndef _TMKNLLX_
#define _TMKNLLX_

typedef unsigned char U08;
#if defined(DOS) && !defined(DOS32)
typedef unsigned U16;
#else
typedef unsigned short U16;
#endif
typedef unsigned long U32;

#ifndef RETIR
#define RETIR void
#endif

#ifndef TYPIR
#define TYPIR interrupt
#endif

#define TMK_INT_SAVED   0x0001
#define TMK_INT_MORE    0x8000
#define TMK_INT_DRV_OVF 0x0002
#define TMK_INT_OTHER   0x7FF4
//#define TMK_INT_TIMER
//#define TMK_INT_BUSJAM
//#define TMK_INT_FIFO_OVF
//#define TMK_INT_GEN1
//#define TMK_INT_GEN2


#define MIN_TMK_TYPE 2
#define MAX_TMK_TYPE 12

#define RTMK         0
#define RTMK1        1
#define TMK400       2
#define TMKMPC       3
#define RTMK400      4
#define TMKX         5
#define TMKXI        6
#define MRTX         7
#define MRTXI        8
#define TA           9
#define TAI          10
#define MRTA         11
#define MRTAI        12

#define ALL_TMKS 0x00FF

#ifdef NMBCID
#define MBC_ALLOC_FAIL 0xFFFF
#endif

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

#define CC_FMT_1 0x00
#define CC_FMT_2 0x01
#define CC_FMT_3 0x02
#define CC_FMT_4 0x03
#define CC_FMT_5 0x05
#define CC_FMT_6 0x04
#define CC_FMT_7 0x08
#define CC_FMT_8 0x0A
#define CC_FMT_9 0x0B
#define CC_FMT_10 0x0C

#define BUS_A 0
#define BUS_B 1
#define BUS_1 0
#define BUS_2 1

#define ERAO_MASK 0x0001
#define MEO_MASK 0x0002
#define IB_MASK 0x0004
#define TO_MASK 0x0008
#define EM_MASK 0x0010
#define EBC_MASK 0x0020
#define DI_MASK 0x0040
#define ELN_MASK 0x0080

#define G1_MASK 0x1000
#define G2_MASK 0x2000

#define S_ERAO_MASK 0x0001
#define S_MEO_MASK 0x0002
#define S_IB_MASK 0x0004
#define S_TO_MASK 0x0008
#define S_EM_MASK 0x0010
#define S_EBC_MASK 0x0020
#define S_DI_MASK 0x0040
#define S_ELN_MASK 0x0080

#define S_G1_MASK 0x1000
#define S_G2_MASK 0x2000

#define NWORDS_MASK 0x001F
#define CMD_MASK 0x041F
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

#define BC_MODE 0x0000
#define RT_MODE 0x0080
#define MT_MODE 0x0100
#define MRT_MODE 0x0280
#define UNDEFINED_MODE 0xFFFF

#define RT_ENABLE       0x0
#define RT_DISABLE      0x1F
#define RT_GET_ENABLE   0xFFFF

#define RT_TRANSMIT 0x0400
#define RT_RECEIVE 0x0000

#define RT_ERROR_MASK 0x4000

#define RT_FLAG 0x8000
#define RT_FLAG_MASK 0x8000

#define RT_HBIT_MODE 0x0001
#define MT_HBIT_MODE 0x0001
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

#define CW(ADDR,DIR,SUBADDR,NWORDS) (((ADDR)<<11)|(DIR)|((SUBADDR)<<5)|((NWORDS)&0x1F))
#define CWM(ADDR,COMMAND) (((ADDR)<<11)|(CI_MASK)|(COMMAND))

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

#ifndef TMK_ERROR_0
#define TMK_ERROR_0 0
#endif

#define TMK_BAD_TYPE     (1 + TMK_ERROR_0)
#define TMK_BAD_IRQ      (2 + TMK_ERROR_0)
#define TMK_BAD_NUMBER   (3 + TMK_ERROR_0)
#define BC_BAD_BUS       (4 + TMK_ERROR_0)
#define BC_BAD_BASE      (5 + TMK_ERROR_0)
#define BC_BAD_LEN       (6 + TMK_ERROR_0)
#define RT_BAD_PAGE      (7 + TMK_ERROR_0)
#define RT_BAD_LEN       (8 + TMK_ERROR_0)
#define RT_BAD_ADDRESS   (9 + TMK_ERROR_0)
#define RT_BAD_FUNC      (10 + TMK_ERROR_0)
#define BC_BAD_FUNC      (11 + TMK_ERROR_0)
#define TMK_BAD_FUNC     (12 + TMK_ERROR_0)
#define TMK_PCI_ERROR    (13 + TMK_ERROR_0)

#define TMK_MAX_ERROR    (13 + TMK_ERROR_0)

#ifdef DOS32
#define TMK_DPMI_ERROR (100 + TMK_ERROR_0) 
#endif //DOS32

#ifndef __EXTERN
#ifdef __cplusplus
#define __EXTERN extern
#else
#define __EXTERN
#endif
#endif

#ifdef __cplusplus
//__EXTERN "C" {
#endif

#if !(defined(DOS))
__EXTERN U16 __rtDisableMask[NTMK+NRT];
#endif

#ifdef QNX4
__EXTERN U16 __tmkIrqPort[NTMK];
__EXTERN U16 __tmkIrqBit[NTMK];
#endif

#ifndef DYNAMIC_TMKNUM
__EXTERN int tmkError;
#define int_TMKNUM void
#define int_TMKNUM__ 
#else
__EXTERN int tmkError[NTMK+NRT];
#define int_TMKNUM int tmkNumber
#define int_TMKNUM__ int tmkNumber,
#endif

#ifdef DOS32
__EXTERN void FARFN tmkOpenDPMI(void);
#endif

__EXTERN U16 FARFN tmkiodelay(int_TMKNUM__ U16 IODelay);
__EXTERN int FARFN tmkgetmaxn(void);
#ifdef QNX4VME
__EXTERN int FARFN tmkconfig(int tmkNumber, U16 tmkType, U16 tmkPorts1, U16 tmkPorts2, U08 tmkIrq1, U08 tmkIrq2, char *pTmkName);
#else
__EXTERN int FARFN tmkconfig(int tmkNumber, U16 tmkType, U16 tmkPorts1, U16 tmkPorts2, U08 tmkIrq1, U08 tmkIrq2);
#endif //def QNX4VME
__EXTERN int FARFN tmkdone(int tmkNumber);
#ifdef DOS
#if NRT > 0
__EXTERN U32 FARFN mrtconfig(int tmkNumber, U16 tmkType, U16 tmkPorts1, U16 tmkPorts2, U08 tmkIrq1, U08 tmkIrq2);
#endif //NRT
__EXTERN void FARFN tmkdefirq(int_TMKNUM__ U16 pcIrq);
__EXTERN void FARFN tmkundefirq(int_TMKNUM__ U16 pcIrq);
__EXTERN void FARFN tmkdeferrors(int_TMKNUM__ void (FARIR* UserErrors)());
#endif
__EXTERN int FARFN tmkselect(int tmkNumber);
#ifndef DYNAMIC_TMKNUM
__EXTERN int FARFN tmkselected(void);
#endif
__EXTERN U16 FARFN tmkgetmode(int_TMKNUM);
__EXTERN void FARFN tmksave(int_TMKNUM);
__EXTERN void FARFN tmkrestore(int_TMKNUM);
__EXTERN void FARFN tmksetcwbits(int_TMKNUM__ U16 tmkSetControl);
__EXTERN void FARFN tmkclrcwbits(int_TMKNUM__ U16 tmkClrControl);
__EXTERN U16 FARFN tmkgetcwbits(int_TMKNUM);

#if defined(DOS) || defined(QNX4)
__EXTERN void FARFN bcdefintnorm(int_TMKNUM__ RETIR (FARIR* UserNormBC)(U16, U16, U16));
__EXTERN void FARFN bcdefintexc(int_TMKNUM__ RETIR (FARIR* UserExcBC)(U16, U16, U16));
__EXTERN void FARFN bcdefintx(int_TMKNUM__ RETIR (FARIR* UserXBC)(U16, U16));
__EXTERN void FARFN bcdefintsig(int_TMKNUM__ RETIR (FARIR* UserSigBC)(U16));
#endif
__EXTERN int FARFN bcreset(int_TMKNUM);
__EXTERN void FARFN bcrestore(int_TMKNUM);
__EXTERN void FARFN bc_def_tldw(U16 wTLDW);
__EXTERN void FARFN bc_enable_di(void);
__EXTERN void FARFN bc_disable_di(void);
__EXTERN int FARFN bcdefirqmode(int_TMKNUM__ U16 bcIrqMode);
__EXTERN U16 FARFN bcgetirqmode(int_TMKNUM);
__EXTERN U16 FARFN bcgetmaxbase(int_TMKNUM);
__EXTERN int FARFN bcdefbase(int_TMKNUM__ U16 bcBasePC);
__EXTERN U16 FARFN bcgetbase(int_TMKNUM);
__EXTERN void FARFN bcputw(int_TMKNUM__ U16 bcAddr, U16 bcData);
__EXTERN U16 FARFN bcgetw(int_TMKNUM__ U16 bcAddr);
__EXTERN U32 FARFN bcgetansw(int_TMKNUM__ U16 bcCtrlCode);
__EXTERN void FARFN bcputblk(int_TMKNUM__ U16 bcAddr, void FARDT *pcBuffer, U16 cwLength);
__EXTERN void FARFN bcgetblk(int_TMKNUM__ U16 bcAddr, void FARDT *pcBuffer, U16 cwLength);
__EXTERN int FARFN bcdefbus(int_TMKNUM__ U16 bcBus);
__EXTERN U16 FARFN bcgetbus(int_TMKNUM);
__EXTERN int FARFN bcstart(int_TMKNUM__ U16 bcBase, U16 bcCtrlCode);
__EXTERN int FARFN bcstartx(int_TMKNUM__ U16 bcBase, U16 bcCtrlCode);
__EXTERN int FARFN bcdeflink(int_TMKNUM__ U16 bcBase, U16 bcCtrlCode);
__EXTERN U32 FARFN bcgetlink(int_TMKNUM);
__EXTERN int FARFN bcstop(int_TMKNUM);
__EXTERN U32 FARFN bcgetstate(int_TMKNUM);

#if defined(DOS) || defined(QNX4)
__EXTERN void FARFN rtdefintcmd(int_TMKNUM__ RETIR (FARIR* UserCmdRT)(U16));
__EXTERN void FARFN rtdefinterr(int_TMKNUM__ RETIR (FARIR* UserErrRT)(U16));
__EXTERN void FARFN rtdefintdata(int_TMKNUM__ RETIR (FARIR* UserDataRT)(U16));
#endif
__EXTERN int FARFN rtreset(int_TMKNUM);
__EXTERN void FARFN rtrestore(int_TMKNUM);
__EXTERN int FARFN rtdefirqmode(int_TMKNUM__ U16 rtIrqMode);
__EXTERN U16 FARFN rtgetirqmode(int_TMKNUM);
__EXTERN int FARFN rtdefmode(int_TMKNUM__ U16 rtMode);
__EXTERN U16 FARFN rtgetmode(int_TMKNUM);
__EXTERN U16 FARFN rtgetmaxpage(int_TMKNUM);
__EXTERN int FARFN rtdefpage(int_TMKNUM__ U16 rtPage);
__EXTERN U16 FARFN rtgetpage(int_TMKNUM);
__EXTERN int FARFN rtdefpagepc(int_TMKNUM__ U16 rtPagePC);
__EXTERN int FARFN rtdefpagebus(int_TMKNUM__ U16 rtPageBus);
__EXTERN U16 FARFN rtgetpagepc(int_TMKNUM);
__EXTERN U16 FARFN rtgetpagebus(int_TMKNUM);
__EXTERN int FARFN rtdefaddress(int_TMKNUM__ U16 rtAddress);
__EXTERN U16 FARFN rtgetaddress(int_TMKNUM);
__EXTERN void FARFN rtdefsubaddr(int_TMKNUM__ U16 rtDir, U16 rtSubAddr);
__EXTERN U16 FARFN rtgetsubaddr(int_TMKNUM);
__EXTERN void FARFN rtputw(int_TMKNUM__ U16 rtAddr, U16 rtData);
__EXTERN U16 FARFN rtgetw(int_TMKNUM__ U16 rtAddr);
__EXTERN void FARFN rtputblk(int_TMKNUM__ U16 rtAddr, void FARDT *pcBuffer, U16 cwLength);
__EXTERN void FARFN rtgetblk(int_TMKNUM__ U16 rtAddr, void FARDT *pcBuffer, U16 cwLength);
__EXTERN void FARFN rtsetanswbits(int_TMKNUM__ U16 rtSetControl);
__EXTERN void FARFN rtclranswbits(int_TMKNUM__ U16 rtClrControl);
__EXTERN U16 FARFN rtgetanswbits(int_TMKNUM);
__EXTERN void FARFN rtgetflags(int_TMKNUM__ void FARDT *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
__EXTERN void FARFN rtputflags(int_TMKNUM__ void FARDT *pcBuffer, U16 rtDir, U16 rtFlagMin, U16 rtFlagMax);
__EXTERN void FARFN rtsetflag(int_TMKNUM);
__EXTERN void FARFN rtclrflag(int_TMKNUM);
__EXTERN U16 FARFN rtgetflag(int_TMKNUM__ U16 rtDir, U16 rtSubAddr);
__EXTERN U16 FARFN rtgetstate(int_TMKNUM);
__EXTERN int FARFN rtbusy(int_TMKNUM);
__EXTERN void FARFN rtlock(int_TMKNUM__ U16 rtDir, U16 rtSubAddr);
__EXTERN void FARFN rtunlock(int_TMKNUM);
__EXTERN U16 FARFN rtgetcmddata(int_TMKNUM__ U16 rtBusCommand);
__EXTERN void FARFN rtputcmddata(int_TMKNUM__ U16 rtBusCommand, U16 rtData);

#if defined(DOS) || defined(QNX4)
__EXTERN void FARFN mtdefintx(int_TMKNUM__ RETIR (FARIR* UserIntXMT)(U16, U16));
__EXTERN void FARFN mtdefintsig(int_TMKNUM__ RETIR (FARIR* UserSigMT)(U16));
#endif
__EXTERN int FARFN mtreset(int_TMKNUM);
#define mtrestore bcrestore
#define mtdefirqmode bcdefirqmode
#define mtgetirqmode bcgetirqmode
#define mtgetmaxbase bcgetmaxbase
#define mtdefbase bcdefbase
#define mtgetbase bcgetbase
#define mtputw bcputw
#define mtgetw bcgetw
__EXTERN U16 FARFN mtgetsw(int_TMKNUM);
#define mtputblk bcputblk
#define mtgetblk bcgetblk
#define mtstartx bcstartx
#define mtdeflink bcdeflink
#define mtgetlink bcgetlink
#define mtstop bcstop
#define mtgetstate bcgetstate

__EXTERN U16 FARFN rtenable(int_TMKNUM__ U16 rtfEnable);

#if NRT > 0
__EXTERN int FARFN mrtgetmaxn(void);
#ifndef DYNAMIC_TMKNUM
__EXTERN int FARFN mrtselected(void);
#endif
__EXTERN int FARFN mrtgetnrt(int_TMKNUM);
__EXTERN int FARFN mrtgetrt0(int_TMKNUM);
__EXTERN U16 FARFN mrtgetstate(int_TMKNUM);
__EXTERN void FARFN mrtdefbrcsubaddr0(int_TMKNUM);
__EXTERN int FARFN mrtdefbrcpage(int_TMKNUM__ U16 mrtBrcPage);
__EXTERN U16 FARFN mrtgetbrcpage(int_TMKNUM);
//__EXTERN int _cdecl mrtreset(void);
#define mrtreset bcreset
__EXTERN int FARFN rt2mrt(int rtNumber);
__EXTERN void FARFN mrtdefmaxnrt(int mrtMaxNrt);
#endif //NRT

__EXTERN U16 FARFN rtgetlock(int_TMKNUM);
__EXTERN void FARFN rtputflag(int_TMKNUM__ U16 rtDir, U16 rtSubAddr, U16 rtFlag);

__EXTERN U16 FARFN tmkgethwver(int_TMKNUM);

__EXTERN U16 FARFN tmktimer(int_TMKNUM__ U16 tmkTimerCtrl);
__EXTERN U16 FARFN tmkgettimerl(int_TMKNUM);
__EXTERN U32 FARFN tmkgettimer(int_TMKNUM);
__EXTERN U32 FARFN bcgetmsgtime(int_TMKNUM);
#define mtgetmsgtime bcgetmsgtime
__EXTERN U32 FARFN rtgetmsgtime(int_TMKNUM);

#ifdef DOS
__EXTERN U16 FARFN tmkswtimer(int_TMKNUM__  U16 tmkSwTimerCtrl);
__EXTERN U32 FARFN tmkgetswtimer(int_TMKNUM);
__EXTERN U32 FARFN tmkgetevtime(int_TMKNUM);
#endif //def DOS

__EXTERN U16 FARFN tmktimeout(int_TMKNUM__ U16 tmkTimeOut);

__EXTERN int FARFN mtdefmode(int_TMKNUM__ U16 mtMode);
__EXTERN U16 FARFN mtgetmode(int_TMKNUM);

__EXTERN int FARFN tmkdefdac(int_TMKNUM__ U16 tmkDacValue);
__EXTERN int FARFN tmkgetdac(int_TMKNUM__ U16 *tmkDacValue, U16 *tmkDacMode);

#ifdef NMBCID
__EXTERN int FARFN mbcinit(U16 mbcId);
__EXTERN int FARFN mbcpreparex(int_TMKNUM__ U16 mbcId, U16 bcBase, U16 bcCtrlCode, U16 mbcDelay);
__EXTERN int FARFN mbcstartx(U16 mbcId);
__EXTERN U16 FARFN mbcalloc(void);
__EXTERN int FARFN mbcfree(U16 mbcId);
#endif

#if NRT > 0 && defined(MRTA)
__EXTERN void FARFN __rtputblkmrta(int_TMKNUM__ U16 rtAddr, void FARDT *pcBuffer, U16 cwLength);
__EXTERN void FARFN __rtgetblkmrta(int_TMKNUM__ U16 rtAddr, void FARDT *pcBuffer, U16 cwLength);
unsigned long DIRQLTmkSave(int hTMK);
void DIRQLTmkRestore(int hTMK, unsigned long Saved);
#else
unsigned DIRQLTmkSave(int hTMK);
void DIRQLTmkRestore(int hTMK, unsigned Saved);
#endif //NRT MRTA
unsigned DIRQLTmksInt1(int hTMK, void *pEvData);
void DpcIExcBC(int hTMK, void *pEvData);

#ifdef __cplusplus
//}
#endif

#endif
