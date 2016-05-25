/*
 * tmk1553b.c -- the tmk1553b v4.06 char module. (c) ELCUS, 2002,2011.
 *
 * Part of this code comes from the book "Linux Device Drivers"
 * by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.
 */

#define TMK_VER_HI 4
#define TMK_VER_LO 06

#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#ifndef TMK1553B_NOCONFIGH
#include <linux/config.h>
#endif 
#ifdef CONFIG_SMP
#define __SMP__
#endif
#ifdef CONFIG_64BIT
#define __64BIT__
#endif
//#if CONFIG_MODVERSIONS==1
//#define MODVERSIONS
//#include <linux/modversions.h>
//#endif
#include <linux/version.h>
#include <linux/module.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */

//#include <asm/system.h>     /* cli(), *_flags */

#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/miscdevice.h>
//#include <asm/msr.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#include <linux/kcomp.h>
#define spin_lock_bh(lock) do {start_bh_atomic(); spin_lock(lock);} while(0)
#define spin_unlock_bh(lock) do {spin_unlock(lock); end_bh_atomic();} while(0)
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26
#endif
#ifndef LINUX26
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQRETURN_T void
#else
#define IRQRETURN_T irqreturn_t
#endif
#ifndef EXPORT_NO_SYMBOLS
#define EXPORT_NO_SYMBOLS
#endif
#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(some_struct) do { } while (0)
#endif

//#define DBG 1
//#define MY_DBG
//#define MY_DBG_DPC

//#ifdef CONFIG_DEVFS_FS
//#define MY_CONFIG_DEVFS_FS
//#endif

#ifdef MY_DBG
#define MY_KERN_DEBUG KERN_EMERG
#define MY_KERN_INFO KERN_EMERG
#define MY_KERN_WARNING KERN_EMERG
#else
#define MY_KERN_DEBUG KERN_DEBUG
#define MY_KERN_INFO KERN_INFO
#define MY_KERN_WARNING KERN_WARNING
#endif // def MY_DBG

typedef void* PVOID;
typedef u32 UINT;
typedef u32* PUINT;
typedef u16 USHORT;
typedef u16* PUSHORT;
typedef u8 UCHAR;
typedef u8* PUCHAR;
typedef int HANDLE;
typedef int NTSTATUS;
typedef int PKEVENT;
typedef int PEPROCESS;

#define IN
#define OUT

#define TRUE 1
#define FALSE 0

#ifdef TMK1553B_THREADS
#define pid group_leader->pid
#endif

#include "tmk1553b.h"          /* local definitions */

#if !defined(IRQF_DISABLED)
#  define IRQF_DISABLED 0x00
#endif

int fTMKInit = 0;

UINT tmkNumber;

#define MAX_TMK_NUMBER 7
#define MAX_RT_NUMBER (30-MAX_TMK_NUMBER-1)
#define MAX_VTMK_NUMBER (MAX_TMK_NUMBER+1+MAX_RT_NUMBER)
#define MAX_LOAD_TYPE 7

int nMaxTmkNumber = MAX_TMK_NUMBER;

#define MRT_READ_BRC_DATA 1
#define MRT_WRITE_BRC_DATA 1
UINT adwTmkOptions[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];

unsigned short awBrcRxState[MAX_TMK_NUMBER+1][32*4];
unsigned short awBrcRxBuf[MAX_TMK_NUMBER+1][32*32];

typedef struct
{
   short nType;
   char szName[10];
   unsigned short wPorts1;
   unsigned short wPorts2;
   unsigned short wIrq1;
   unsigned short wIrq2;
   unsigned short wIODelay;
   unsigned short wHiddenPorts;
   int nTmk;
   int fLoaded;
   int nLoadType;
   int fIrqShared;
   int nDev;
   int nSubDev;
   int fLocalReadInt;
   int fMRT;
   int nPorts;
} TTmkConfig;

TTmkConfig aTmkConfig[MAX_TMK_NUMBER+1];

typedef struct
{
  struct list_head ProcListEntry;
  PEPROCESS hProc;
  int fTMK[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];
  int nSelectedTMK;
  u32 maskTMK[(MAX_VTMK_NUMBER)/32+1];
  u32 maskEvents[(MAX_VTMK_NUMBER)/32+1];
  wait_queue_head_t wq;
  int waitFlag;
} TListProc;

struct list_head hlProc;

//HPROC hTmkVM[MAX_TMK_NUMBER+1];
PEPROCESS hCurProc;
TListProc *hlnCurProc;

typedef struct
{
  PEPROCESS hProc;
  PKEVENT hEvent;
  TListProc *hlnProc;
} TTMK;

TTMK aTMK[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];

typedef struct
{
  int nInt;
  USHORT wMode;
  USHORT awEvData[3];
  PEPROCESS hProc;
} TListEvD, *pTListEvD;

//pTListEvD ahlEvData[MAX_TMK_NUMBER+1];
//pTListEvD hlnEvData;
//VMMLIST ahlEvData[MAX_TMK_NUMBER+1];
//VMMLISTNODE hlnEvData;

spinlock_t tmkIrqSpinLock, tmkSpinLock;
void tmkDpcRoutine(unsigned long);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
DECLARE_TASKLET (tmkDpc, tmkDpcRoutine, 0);
#endif
IRQRETURN_T tmkInterruptServiceRoutine(int irq, void *dev_id
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
                                       , struct pt_regs *regs
#endif
                                      );

// 2^^n only !
#define EVENTS_SIZE 512
TListEvD aEvData[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1][EVENTS_SIZE];
int iEvDataBegin[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];
int iEvDataEnd[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];
int cEvData[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];
int cDpcData[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];

volatile int tmkEvents[(MAX_VTMK_NUMBER)/32+1];

typedef struct
{
  int nIrq;
  int cTmks;
  int hTmk[MAX_TMK_NUMBER+1];
} TIrq;

TIrq ahIrq[MAX_TMK_NUMBER+1];

int cIrqs = 0;

int nmrt, nrtmax;


char *szTmk1553b = "tmk1553b";

int tmk1553b_nr_devs = TMK1553B_NR_DEVS;    /* number of bare tmk1553b devices */

static int major = TMK1553B_MAJOR;
static char *name = 0;
static int misc = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,3)
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number");
module_param(name, charp, 0);
MODULE_PARM_DESC(name, "Device name");
module_param(misc, int, 0);
MODULE_PARM_DESC(major, "Misc device flag");
#else
MODULE_PARM(major,"i");
MODULE_PARM(name,"s");
MODULE_PARM(misc,"i");
//MODULE_PARM(tmk1553b_nr_devs,"i");
#endif

static int a0 = 0xFFFF;
static int b0 = 0xFFFF;
static int i0 = 0xFF;
static int d0 = 0;
static int e0 = 0;
static char *t0 = 0;
static char *l0 = 0;
static int a1 = 0xFFFF;
static int b1 = 0xFFFF;
static int i1 = 0xFF;
static int d1 = 0;
static int e1 = 0;
static char *t1 = 0;
static char *l1 = 0;
static int a2 = 0xFFFF;
static int b2 = 0xFFFF;
static int i2 = 0xFF;
static int d2 = 0;
static int e2 = 0;
static char *t2 = 0;
static char *l2 = 0;
static int a3 = 0xFFFF;
static int b3 = 0xFFFF;
static int i3 = 0xFF;
static int d3 = 0;
static int e3 = 0;
static char *t3 = 0;
static char *l3 = 0;
static int a4 = 0xFFFF;
static int b4 = 0xFFFF;
static int i4 = 0xFF;
static int d4 = 0;
static int e4 = 0;
static char *t4 = 0;
static char *l4 = 0;
static int a5 = 0xFFFF;
static int b5 = 0xFFFF;
static int i5 = 0xFF;
static int d5 = 0;
static int e5 = 0;
static char *t5 = 0;
static char *l5 = 0;
static int a6 = 0xFFFF;
static int b6 = 0xFFFF;
static int i6 = 0xFF;
static int d6 = 0;
static int e6 = 0;
static char *t6 = 0;
static char *l6 = 0;
static int a7 = 0xFFFF;
static int b7 = 0xFFFF;
static int i7 = 0xFF;
static int d7 = 0;
static int e7 = 0;
static char *t7 = 0;
static char *l7 = 0;
static int nrt = 0;
 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,3)
module_param(a0, int, 0);
module_param(b0, int, 0);
module_param(i0, int, 0);
module_param(d0, int, 0);
module_param(e0, int, 0);
module_param(t0, charp, 0);
module_param(l0, charp, 0);
module_param(a1, int, 0);
module_param(b1, int, 0);
module_param(i1, int, 0);
module_param(d1, int, 0);
module_param(e1, int, 0);
module_param(t1, charp, 0);
module_param(l1, charp, 0);
module_param(a2, int, 0);
module_param(b2, int, 0);
module_param(i2, int, 0);
module_param(d2, int, 0);
module_param(e2, int, 0);
module_param(t2, charp, 0);
module_param(l2, charp, 0);
module_param(a3, int, 0);
module_param(b3, int, 0);
module_param(i3, int, 0);
module_param(d3, int, 0);
module_param(e3, int, 0);
module_param(t3, charp, 0);
module_param(l3, charp, 0);
module_param(a4, int, 0);
module_param(b4, int, 0);
module_param(i4, int, 0);
module_param(d4, int, 0);
module_param(e4, int, 0);
module_param(t4, charp, 0);
module_param(l4, charp, 0);
module_param(a5, int, 0);
module_param(b5, int, 0);
module_param(i5, int, 0);
module_param(d5, int, 0);
module_param(e5, int, 0);
module_param(t5, charp, 0);
module_param(l5, charp, 0);
module_param(a6, int, 0);
module_param(b6, int, 0);
module_param(i6, int, 0);
module_param(d6, int, 0);
module_param(e6, int, 0);
module_param(t6, charp, 0);
module_param(l6, charp, 0);
module_param(a7, int, 0);
module_param(b7, int, 0);
module_param(i7, int, 0);
module_param(d7, int, 0);
module_param(e7, int, 0);
module_param(t7, charp, 0);
module_param(l7, charp, 0);
module_param(nrt, int, 0);
MODULE_PARM_DESC(a0,"ISA addr1 device 0");
MODULE_PARM_DESC(b0,"ISA addr2 device 0");
MODULE_PARM_DESC(i0,"ISA interrupt device 0");
MODULE_PARM_DESC(d0,"PCI card number device 0");
MODULE_PARM_DESC(e0,"PCI device on PCI card number device 0");
MODULE_PARM_DESC(t0,"Device type device 0");
MODULE_PARM_DESC(l0,"Device load device 0");
MODULE_PARM_DESC(a1,"ISA addr1 device 1");
MODULE_PARM_DESC(b1,"ISA addr2 device 1");
MODULE_PARM_DESC(i1,"ISA interrupt device 1");
MODULE_PARM_DESC(d1,"PCI card number device 1");
MODULE_PARM_DESC(e1,"PCI device on PCI card number device 1");
MODULE_PARM_DESC(t1,"Device type device 1");
MODULE_PARM_DESC(l1,"Device load device 1");
MODULE_PARM_DESC(a2,"ISA addr1 device 2");
MODULE_PARM_DESC(b2,"ISA addr2 device 2");
MODULE_PARM_DESC(i2,"ISA interrupt device 2");
MODULE_PARM_DESC(d2,"PCI card number device 2");
MODULE_PARM_DESC(e2,"PCI device on PCI card number device 2");
MODULE_PARM_DESC(t2,"Device type device 2");
MODULE_PARM_DESC(l2,"Device load device 2");
MODULE_PARM_DESC(a3,"ISA addr1 device 3");
MODULE_PARM_DESC(b3,"ISA addr2 device 3");
MODULE_PARM_DESC(i3,"ISA interrupt device 3");
MODULE_PARM_DESC(d3,"PCI card number device 3");
MODULE_PARM_DESC(e3,"PCI device on PCI card number device 3");
MODULE_PARM_DESC(t3,"Device type device 3");
MODULE_PARM_DESC(l3,"Device load device 3");
MODULE_PARM_DESC(a4,"ISA addr1 device 4");
MODULE_PARM_DESC(b4,"ISA addr2 device 4");
MODULE_PARM_DESC(i4,"ISA interrupt device 4");
MODULE_PARM_DESC(d4,"PCI card number device 4");
MODULE_PARM_DESC(e4,"PCI device on PCI card number device 4");
MODULE_PARM_DESC(t4,"Device type device 4");
MODULE_PARM_DESC(l4,"Device load device 4");
MODULE_PARM_DESC(a5,"ISA addr1 device 5");
MODULE_PARM_DESC(b5,"ISA addr2 device 5");
MODULE_PARM_DESC(i5,"ISA interrupt device 5");
MODULE_PARM_DESC(d5,"PCI card number device 5");
MODULE_PARM_DESC(e5,"PCI device on PCI card number device 5");
MODULE_PARM_DESC(t5,"Device type device 5");
MODULE_PARM_DESC(l5,"Device load device 5");
MODULE_PARM_DESC(a6,"ISA addr1 device 6");
MODULE_PARM_DESC(b6,"ISA addr2 device 6");
MODULE_PARM_DESC(i6,"ISA interrupt device 6");
MODULE_PARM_DESC(d6,"PCI card number device 6");
MODULE_PARM_DESC(e6,"PCI device on PCI card number device 6");
MODULE_PARM_DESC(t6,"Device type device 6");
MODULE_PARM_DESC(l6,"Device load device 6");
MODULE_PARM_DESC(a7,"ISA addr1 device 7");
MODULE_PARM_DESC(b7,"ISA addr2 device 7");
MODULE_PARM_DESC(i7,"ISA interrupt device 7");
MODULE_PARM_DESC(d7,"PCI card number device 7");
MODULE_PARM_DESC(e7,"PCI device on PCI card number device 7");
MODULE_PARM_DESC(t7,"Device type device 7");
MODULE_PARM_DESC(l7,"Device load device 7");
MODULE_PARM_DESC(nrt,"Maximum number of RTs per MultiRT device");
#else
MODULE_PARM(a0,"i");
MODULE_PARM(b0,"i");
MODULE_PARM(i0,"i");
MODULE_PARM(d0,"i");
MODULE_PARM(e0,"i");
MODULE_PARM(t0,"s");
MODULE_PARM(l0,"s");
MODULE_PARM(a1,"i");
MODULE_PARM(b1,"i");
MODULE_PARM(i1,"i");
MODULE_PARM(d1,"i");
MODULE_PARM(e1,"i");
MODULE_PARM(t1,"s");
MODULE_PARM(l1,"s");
MODULE_PARM(a2,"i");
MODULE_PARM(b2,"i");
MODULE_PARM(i2,"i");
MODULE_PARM(d2,"i");
MODULE_PARM(e2,"i");
MODULE_PARM(t2,"s");
MODULE_PARM(l2,"s");
MODULE_PARM(a3,"i");
MODULE_PARM(b3,"i");
MODULE_PARM(i3,"i");
MODULE_PARM(d3,"i");
MODULE_PARM(e3,"i");
MODULE_PARM(t3,"s");
MODULE_PARM(l3,"s");
MODULE_PARM(a4,"i");
MODULE_PARM(b4,"i");
MODULE_PARM(i4,"i");
MODULE_PARM(d4,"i");
MODULE_PARM(e4,"i");
MODULE_PARM(t4,"s");
MODULE_PARM(l4,"s");
MODULE_PARM(a5,"i");
MODULE_PARM(b5,"i");
MODULE_PARM(i5,"i");
MODULE_PARM(d5,"i");
MODULE_PARM(e5,"i");
MODULE_PARM(t5,"s");
MODULE_PARM(l5,"s");
MODULE_PARM(a6,"i");
MODULE_PARM(b6,"i");
MODULE_PARM(i6,"i");
MODULE_PARM(d6,"i");
MODULE_PARM(e6,"i");
MODULE_PARM(t6,"s");
MODULE_PARM(l6,"s");
MODULE_PARM(a7,"i");
MODULE_PARM(b7,"i");
MODULE_PARM(i7,"i");
MODULE_PARM(d7,"i");
MODULE_PARM(e7,"i");
MODULE_PARM(t7,"s");
MODULE_PARM(l7,"s");
MODULE_PARM(nrt,"i");
#endif

MODULE_AUTHOR("Yury Savchuk");
MODULE_DESCRIPTION("Driver for ELCUS (http://www.elcus.ru) MIL-STD-1553B boards");

tmk1553b_Dev *tmk1553b_devices; /* allocated in tmk1553b_init_module */

static struct miscdevice tmk1553b_mdev;

#include "tmklllin.h"

UINT TMK_tmkgetmaxn(void);
UINT TMK_tmkconfig(void);
UINT TMK_tmkdone(void);
UINT TMK_tmkselect(void);
UINT TMK_tmkselected(void);
UINT TMK_tmkgetmode(void);
UINT TMK_tmksetcwbits(void);
UINT TMK_tmkclrcwbits(void);
UINT TMK_tmkgetcwbits(void);
UINT TMK_tmkwaitevents(void);
//UINT TMK_tmkdefevent(void);
UINT TMK_tmkgetevd(void);

//UINT TMK_bcdefintnorm(void);
//UINT TMK_bcdefintexc(void);
//UINT TMK_bcdefintx(void);
//UINT TMK_bcdefintsig(void);
UINT TMK_bcreset(void);
UINT TMK_bc_def_tldw(void);
UINT TMK_bc_enable_di(void);
UINT TMK_bc_disable_di(void);
UINT TMK_bcdefirqmode(void);
UINT TMK_bcgetirqmode(void);
UINT TMK_bcgetmaxbase(void);
UINT TMK_bcdefbase(void);
UINT TMK_bcgetbase(void);
UINT TMK_bcputw(void);
UINT TMK_bcgetw(void);
UINT TMK_bcgetansw(void);
UINT TMK_bcputblk(void);
UINT TMK_bcgetblk(void);
UINT TMK_bcdefbus(void);
UINT TMK_bcgetbus(void);
UINT TMK_bcstart(void);
UINT TMK_bcstartx(void);
UINT TMK_bcdeflink(void);
UINT TMK_bcgetlink(void);
UINT TMK_bcstop(void);
UINT TMK_bcgetstate(void);

//UINT TMK_rtdefintcmd(void);
//UINT TMK_rtdefinterr(void);
//UINT TMK_rtdefintdata(void);
UINT TMK_rtreset(void);
UINT TMK_rtdefirqmode(void);
UINT TMK_rtgetirqmode(void);
UINT TMK_rtdefmode(void);
UINT TMK_rtgetmode(void);
UINT TMK_rtgetmaxpage(void);
UINT TMK_rtdefpage(void);
UINT TMK_rtgetpage(void);
UINT TMK_rtdefpagepc(void);
UINT TMK_rtdefpagebus(void);
UINT TMK_rtgetpagepc(void);
UINT TMK_rtgetpagebus(void);
UINT TMK_rtdefaddress(void);
UINT TMK_rtgetaddress(void);
UINT TMK_rtdefsubaddr(void);
UINT TMK_rtgetsubaddr(void);
UINT TMK_rtputw(void);
UINT TMK_rtgetw(void);
UINT TMK_rtputblk(void);
UINT TMK_rtgetblk(void);
UINT TMK_rtsetanswbits(void);
UINT TMK_rtclranswbits(void);
UINT TMK_rtgetanswbits(void);
UINT TMK_rtgetflags(void);
UINT TMK_rtputflags(void);
UINT TMK_rtsetflag(void);
UINT TMK_rtclrflag(void);
UINT TMK_rtgetflag(void);
UINT TMK_rtgetstate(void);
UINT TMK_rtbusy(void);
UINT TMK_rtlock(void);
UINT TMK_rtunlock(void);
UINT TMK_rtgetcmddata(void);
UINT TMK_rtputcmddata(void);

//UINT TMK_mtdefintx(void);
//UINT TMK_mtdefintsig(void);
UINT TMK_mtreset(void);
#define TMK_mtdefirqmode TMK_bcdefirqmode
#define TMK_mtgetirqmode TMK_bcgetirqmode
#define TMK_mtgetmaxbase TMK_bcgetmaxbase
#define TMK_mtdefbase TMK_bcdefbase
#define TMK_mtgetbase TMK_bcgetbase
#define TMK_mtputw TMK_bcputw
#define TMK_mtgetw TMK_bcgetw
UINT TMK_mtgetsw(void);
#define TMK_mtputblk TMK_bcputblk
#define TMK_mtgetblk TMK_bcgetblk
#define TMK_mtstartx TMK_bcstartx
#define TMK_mtdeflink TMK_bcdeflink
#define TMK_mtgetlink TMK_bcgetlink
#define TMK_mtstop TMK_bcstop
#define TMK_mtgetstate TMK_bcgetstate

UINT TMK_tmkgetinfo(void);
UINT TMK_getversion(void);

UINT TMK_rtenable(void);

UINT TMK_mrtgetmaxn(void);
UINT TMK_mrtconfig(void);
UINT TMK_mrtselected(void);
UINT TMK_mrtgetstate(void);
UINT TMK_mrtdefbrcsubaddr0(void);
UINT TMK_mrtreset(void);

UINT TMK_tmktimer(void);
UINT TMK_tmkgettimer(void);
UINT TMK_tmkgettimerl(void);
UINT TMK_bcgetmsgtime(void);
#define TMK_mtgetmsgtime TMK_bcgetmsgtime
UINT TMK_rtgetmsgtime(void);

UINT TMK_tmkgethwver(void);

UINT TMK_tmkgetevtime(void);
UINT TMK_tmkswtimer(void);
UINT TMK_tmkgetswtimer(void);

UINT TMK_tmktimeout(void);

UINT TMK_mrtdefbrcpage(void);
UINT TMK_mrtgetbrcpage(void);

UINT TMK_mbcinit(void);
UINT TMK_mbcpreparex(void);
UINT TMK_mbcstartx(void);
UINT TMK_mbcalloc(void);
UINT TMK_mbcfree(void);

UINT TMK_tmkwaiteventsflag(void);
UINT TMK_tmkwaiteventsm(void);

//UINT TMK_bcputblk64(void);
//UINT TMK_bcgetblk64(void);

//UINT TMK_rtputblk64(void);
//UINT TMK_rtgetblk64(void);

//UINT TMK_rtgetflags64(void);
//UINT TMK_rtputflags64(void);

UINT (*TMK_Procs[])(void) = {
        TMK_tmkconfig,
        TMK_tmkdone,
        TMK_tmkgetmaxn,
        TMK_tmkselect,
        TMK_tmkselected,
        TMK_tmkgetmode,
        TMK_tmksetcwbits,
        TMK_tmkclrcwbits,
        TMK_tmkgetcwbits,
        TMK_tmkwaitevents,
//        TMK_tmkdefevent,
        TMK_tmkgetevd,

//        TMK_bcdefintnorm,
//        TMK_bcdefintexc,
//        TMK_bcdefintx,
//        TMK_bcdefintsig,
        TMK_bcreset,
        TMK_bc_def_tldw,
        TMK_bc_enable_di,
        TMK_bc_disable_di,
        TMK_bcdefirqmode,
        TMK_bcgetirqmode,
        TMK_bcgetmaxbase,
        TMK_bcdefbase,
        TMK_bcgetbase,
        TMK_bcputw,
        TMK_bcgetw,
        TMK_bcgetansw,
        TMK_bcputblk,
        TMK_bcgetblk,
        TMK_bcdefbus,
        TMK_bcgetbus,
        TMK_bcstart,
        TMK_bcstartx,
        TMK_bcdeflink,
        TMK_bcgetlink,
        TMK_bcstop,
        TMK_bcgetstate,

//        TMK_rtdefintcmd,
//        TMK_rtdefinterr,
//        TMK_rtdefintdata,
        TMK_rtreset,
        TMK_rtdefirqmode,
        TMK_rtgetirqmode,
        TMK_rtdefmode,
        TMK_rtgetmode,
        TMK_rtgetmaxpage,
        TMK_rtdefpage,
        TMK_rtgetpage,
        TMK_rtdefpagepc,
        TMK_rtdefpagebus,
        TMK_rtgetpagepc,
        TMK_rtgetpagebus,
        TMK_rtdefaddress,
        TMK_rtgetaddress,
        TMK_rtdefsubaddr,
        TMK_rtgetsubaddr,
        TMK_rtputw,
        TMK_rtgetw,
        TMK_rtputblk,
        TMK_rtgetblk,
        TMK_rtsetanswbits,
        TMK_rtclranswbits,
        TMK_rtgetanswbits,
        TMK_rtgetflags,
        TMK_rtputflags,
        TMK_rtsetflag,
        TMK_rtclrflag,
        TMK_rtgetflag,
        TMK_rtgetstate,
        TMK_rtbusy,
        TMK_rtlock,
        TMK_rtunlock,
        TMK_rtgetcmddata,
        TMK_rtputcmddata,

//        TMK_mtdefintx,
//        TMK_mtdefintsig,
        TMK_mtreset,
        TMK_mtdefirqmode,
        TMK_mtgetirqmode,
        TMK_mtgetmaxbase,
        TMK_mtdefbase,
        TMK_mtgetbase,
        TMK_mtputw,
        TMK_mtgetw,
        TMK_mtgetsw,
        TMK_mtputblk,
        TMK_mtgetblk,
        TMK_mtstartx,
        TMK_mtdeflink,
        TMK_mtgetlink,
        TMK_mtstop,
        TMK_mtgetstate,

        TMK_tmkgetinfo,
        TMK_getversion,

        TMK_rtenable,

        TMK_mrtgetmaxn,
        TMK_mrtconfig,
        TMK_mrtselected,
        TMK_mrtgetstate,
        TMK_mrtdefbrcsubaddr0,
        TMK_mrtreset,

        TMK_tmktimer,
        TMK_tmkgettimer,
        TMK_tmkgettimerl,
        TMK_bcgetmsgtime,
        TMK_mtgetmsgtime,
        TMK_rtgetmsgtime,

        TMK_tmkgethwver,

        TMK_tmkgetevtime,
        TMK_tmkswtimer,
        TMK_tmkgetswtimer,

        TMK_tmktimeout,

        TMK_mrtdefbrcpage,
        TMK_mrtgetbrcpage,

        TMK_mbcinit,
        TMK_mbcpreparex,
        TMK_mbcstartx,
        TMK_mbcalloc,
        TMK_mbcfree,

        TMK_mbcfree,//112
        TMK_mbcfree,//113
        TMK_mbcfree,//114

        TMK_tmkwaiteventsflag,

        TMK_tmkwaiteventsm
        };

#define MAX_TMK_API (sizeof(TMK_Procs)/sizeof(void*)+1)

//int aCmds[MAX_TMK_API+1];

//#include "tmk1553b.h"

//#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
//#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((USHORT)(l))
#define HIWORD(l)           ((USHORT)(((UINT)(l) >> 16) & 0xFFFF))
//#define LOBYTE(w)           ((BYTE)(w))
//#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

//typedef char *PSTR;
//typedef USHORT USHORT;
//typedef USHORT *PUSHORT;

// Pointers to input and output buffers
PUSHORT lpwIn, lpwOut, lpwBuf;
//PUINT lpcbReturned;

#define TMK_BAD_0      -1024
/*
#define TMK_BAD_TYPE   1
#define TMK_BAD_IRQ    2
#define TMK_BAD_NUMBER 3
#define BC_BAD_BUS     4
#define BC_BAD_BASE    5
#define BC_BAD_LEN     6
#define RT_BAD_PAGE    7
#define RT_BAD_LEN     8
#define RT_BAD_ADDRESS 9
#define RT_BAD_FUNC    10
#define BC_BAD_FUNC    11
#define TMK_BAD_FUNC   12

#define TMK_MAX_ERROR  12
*/

#include "tmktest.c"

#define LOADTX1 0
//#define LOADRT1 1
#define LOADTX4 2
#define LOADTX5 3
#define LOADTX6 4
#define LOADMR6 5
//#define LOADTT6 6
//#define LOADTD6 7
//#define LOADRT1
//#define LOADTX4
//#define LOADTX5

#include "load/txv11.c"
#ifdef LOADRT1
#include "load/rt1v03.c"
#endif //def LOADRT1
#ifdef LOADTX6
#define fpgabuf4 fpgabuf0
#endif //def LOADTX6
#ifdef LOADTX4
#define fpgabuf2 fpgabuf0
#endif //def LOADTX4
#ifdef LOADTX5
#define fpgabuf3 fpgabuf0
#endif //def LOADTX5

#ifdef CONFIG_PCI
#define fpgabuf5 fpgabuf0
#ifdef LOADTX6
#define fpgabuf6 fpgabuf0
#endif //def LOADTX6
#endif //def CONFIG_PCI

#ifdef LOADMR6
#include "load/mr6v02.c"
#endif //def LOADMR6
#ifdef LOADTT6
#include "load/tt6v04.c"
#endif //def LOADTT6
#ifdef LOADTD6
#include "load/td6v01.c"
#endif //def LOADTD6

#ifdef CONFIG_PCI
#ifdef LOADMR6
#define fpgabuf10 fpgabuf7
#endif //def LOADMR6
#ifdef LOADTT6
#define fpgabuf11 fpgabuf8
#endif //def LOADTT6
#ifdef LOADTD6
#define fpgabuf12 fpgabuf9
#endif //def LOADTD6
#endif //def CONFIG_PCI


#ifdef CONFIG_PCI
#define PLX_MAX_DEVICE_CNT 2
#define ID_TX1PCI 0x0001E1C5
#define ID_TX6PCI 0x0002E1C5
#define ID_TX1PCI2 0x0003E1C5
#define ID_TX6PCI2 0x0004E1C5
#define ID_TA1PCI 0x0005E1C5
#define ID_TA1PCI4 0x0006E1C5
#define ID_TA1PCI32RT 0x0007E1C5
#define ID_TA1PE2 0x0008E1C5
#define ID_TA1PE4 0x0009E1C5
#define ID_TA1PE32RT 0x000AE1C5
#define CFG_COMMAND      0x04
#define CFG_SUBSYSTEM_ID 0x2C
#define CFG_IRQ          0x3C
#define CFG_ADDR1        0x14
#define CFG_ADDR2        0x18
#define CFG_ADDR3        0x1C
#define CFG_ADDR4        0x20
#define CFG_ADDR5        0x24
#endif //def CONFIG_PCI

//#ifdef CONFIG_PCI
//#define TX_VID 0x10B5
//#define TX_DID 0x9030
//#define TX_SubVID 0xE1C5
//#define TX1_SubDID 0x0001
//#define TX6_SubDID 0x0002
//#define TX1_2_SubDID 0x0003
//#define TX6_2_SubDID 0x0004
//#define ID_TX1PCI 0x0001E1C5
//#define ID_TX6PCI 0x0002E1C5
//#endif //def CONFIG_PCI

#define READ_PORT  (port+0xC)
#define LOAD_PORT  (port+0xC)
#define RESET_PORT (port+6)
#define ADDR_PORT  (port+0xA)
#define DATA_PORT  (port+0xE)
#define MODE_PORT  (port+8)

//#define BUSY_BIT      0x0080
#define BUSY_DEVICE   0x7E7F
#define EMPTY_DEVICE  0x7EFF
#define ERROR_DEVICE  0x7FFF
#define LOADED_DEVICE 0x0000

#define GENER1_BL 0x0004
#define GENER2_BL 0x4000

#define TX_OK         0
#define TX_BAD_PARAMS 1
#define TX_NOT_FOUND  2
#define TX_LOAD_ERROR 4
#define TX_INIT_ERROR 8
#define TX_TEST_ERROR 16

int tmkxload(unsigned port, unsigned char *fpgabuf, int len)
{
  int i;

  if ((inw(READ_PORT) & 0x7FFF) != EMPTY_DEVICE)
    return TX_NOT_FOUND;
  for (i = 0; i < len; ++i)
  {
    if ((inw(READ_PORT) & 0x7FFF) != EMPTY_DEVICE)
      return TX_LOAD_ERROR;
    outw((unsigned short)fpgabuf[i] | 0xFF00, LOAD_PORT);
    while ((inw(READ_PORT) & 0x7FFF) == BUSY_DEVICE);
  }
  for (i = 0; i < 100; ++i) // wait 15 us
    inw(READ_PORT);
  outw(0, RESET_PORT);
  outw(GENER1_BL+GENER2_BL, MODE_PORT);
  if (inw(READ_PORT) != LOADED_DEVICE)
    return TX_INIT_ERROR;
  return TX_OK;
}

#ifdef CONFIG_PCI
int tmkxiload(unsigned portplx, int reset)
{
  unsigned char *fpgabuf;
  int len;
  int res = TX_OK;
  unsigned port;
  int i;
  int fLoad, fWait;
  int cTmk, iTmk;
  TTmkConfig *apTmk[MAX_TMK_NUMBER+1];
  TTmkConfig *pTmk;

  cTmk = 0;
  fLoad = 0;
  for (iTmk = 0; iTmk <= MAX_TMK_NUMBER; ++iTmk)
  {
    pTmk = &aTmkConfig[iTmk];
    if ((unsigned)(pTmk->wHiddenPorts) != portplx ||
        pTmk->fLoaded)
      continue;
    apTmk[cTmk++] = pTmk;
    if (pTmk->nLoadType == LOADTX1)
    {
      port = (unsigned)(pTmk->wPorts1);
      outw(0, RESET_PORT);
      outw(GENER1_BL+GENER2_BL, MODE_PORT);
      if (inw(READ_PORT) != LOADED_DEVICE)
      {
        fLoad = 1;
      }
    }
    else
    {
      fLoad = 1;
    }
  }

  if (fLoad || reset)
  {
    outw(inw(portplx + 0x56) | 0x0010, portplx + 0x56); // M02 = 1
    outw(inw(portplx + 0x52) | 0x4000, portplx + 0x52);  // Reset = 1
    outw(inw(portplx + 0x52) & ~0x4000, portplx + 0x52); // Reset = 0
    res = 0;
    do
    {
      if (++res > 2000)
      {
        outw(inw(portplx + 0x56) & ~0x0010, portplx + 0x56); // M02 = 0
        break;
      }
      fWait = 0;
      for (iTmk = 0; iTmk < cTmk; ++iTmk)
      {
        pTmk = apTmk[iTmk];
        port = (unsigned)(pTmk->wPorts1);
        if ((inw(READ_PORT)&0x7FFF) != EMPTY_DEVICE)
          fWait = 1;
      }
    }
    while (fWait);
    res = 0;
    for (iTmk = 0; iTmk < cTmk; ++iTmk)
    {
      pTmk = apTmk[iTmk];
      pTmk->fLoaded = 1;
      port = (unsigned)(pTmk->wPorts1);
      for (i = 0; i < 100; ++i) // wait 15 us
        inw(READ_PORT);
      switch (pTmk->nLoadType)
      {
      case LOADTX1:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltxi1v11\n");
        #endif
        fpgabuf = fpgabuf5;
        len = sizeof(fpgabuf5);
        break;
#ifdef LOADTX6
      case LOADTX6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltxi6v11\n");
        #endif
        fpgabuf = fpgabuf6;
        len = sizeof(fpgabuf6);
        break;
#endif //def LOADTX6
#ifdef LOADMR6
      case LOADMR6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: lmri6v02\n");
        #endif
        fpgabuf = fpgabuf10;
        len = sizeof(fpgabuf10);
        pTmk->nType = MRTXI;
        break;
#endif //def LOADMR6
#ifdef LOADTT6
      case LOADTT6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltti6v04\n");
        #endif
        fpgabuf = fpgabuf11;
        len = sizeof(fpgabuf11);
        pTmk->nType = TTXI;
        break;
#endif //def LOADTT6
#ifdef LOADTD6
      case LOADTD6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltdi6v01\n");
        #endif
        fpgabuf = fpgabuf12;
        len = sizeof(fpgabuf12);
        pTmk->nType = TDXI;
        break;
#endif //def LOADTD6
      default:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: default disabled\n");
        #endif
        pTmk->nType = -1;
        continue;
      }
      res |= tmkxload(port, fpgabuf, len);
    }
    outw(inw(portplx + 0x56) & ~0x0010, portplx + 0x56); // M02 = 0
  }
  return res;
}
#endif //def CONFIG_PCI

#ifdef NOTDEF
int tmk1553b_trim(tmk1553b_Dev *dev)
{
    tmk1553b_Dev *next, *dptr;
    int qset = dev->qset;   /* "dev" is not-null */
    int i;

    for (dptr = dev; dptr; dptr = next) { /* all the list items */
        if (dptr->data) {
            for (i = 0; i < qset; i++)
                if (dptr->data[i])
                    kfree(dptr->data[i]);
            kfree(dptr->data);
            dptr->data=NULL;
        }
        next=dptr->next;
        if (dptr != dev) kfree(dptr); /* all of them but the first */
    }
    dev->size = 0;
    dev->next = NULL;
    return 0;
}
#endif //def NOTDEF

#ifdef TMK1553B_DEBUG /* use proc only if debugging */
/*
 * The proc filesystem: function to read and entry
 */

int tmk1553b_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
    int i, j, len = 0;
    int limit = count - 80; /* Don't print more than this */

    for (i = 0; i < tmk1553b_nr_devs && len <= limit; i++) {
        tmk1553b_Dev *d = &tmk1553b_devices[i];
        if (down_interruptible(&d->sem))
                return -ERESTARTSYS;
        len += sprintf(buf+len,"\nDevice %i: sz %li\n",
                       i, d->size);
        for (; d && len <= limit; d = d->next) { /* scan the list */
            len += sprintf(buf+len, "  item at %p, qset at %p\n", d, d->data);
            if (d->data && !d->next) /* dump only the last item - save space */
                for (j = 0; j < d->qset; j++) {
                    if (d->data[j])
                        len += sprintf(buf+len,"    % 4i: %8p\n",j,d->data[j]);
                }
        }
        up(&tmk1553b_devices[i].sem);
    }
    *eof = 1;
    return len;
}

#ifdef USE_PROC_REGISTER

static int tmk1553b_get_info(char *buf, char **start, off_t offset,
                int len, int unused)
{
    int eof = 0;
    return tmk1553b_read_procmem (buf, start, offset, len, &eof, NULL);
}

struct proc_dir_entry tmk1553b_proc_entry = {
        namelen:    8,
        name:       "tmk1553bmem",
        mode:       S_IFREG | S_IRUGO,
        nlink:      1,
        get_info:   tmk1553b_get_info,
};

static void tmk1553b_create_proc()
{
    proc_register_dynamic(&proc_root, &tmk1553b_proc_entry);
}

static void tmk1553b_remove_proc()
{
    proc_unregister(&proc_root, tmk1553b_proc_entry.low_ino);
}

#else  /* no USE_PROC_REGISTER - modern world */

static void tmk1553b_create_proc()
{
    create_proc_read_entry("tmk1553bmem", 0 /* default mode */,
                           NULL /* parent dir */, tmk1553b_read_procmem,
                           NULL /* client data */);
}

static void tmk1553b_remove_proc()
{
    /* no problem if it was not registered */
    remove_proc_entry("tmk1553bmem", NULL /* parent dir */);
}

#endif /* def USE_PROC_REGISTER */

#endif /* def TMK1553B_DEBUG */


/*
 * Open and close
 */

int tmk1553b_open(struct inode *inode, struct file *filp)
{
//    tmk1553b_Dev *dev; /* device information */
    int num = MINOR(inode->i_rdev);

    TListProc *hlnProc;
    int iTMK;
  
    #ifdef MY_DBG
    printk(MY_KERN_DEBUG "Tmk1553b: Opened!!\n");
    #endif

    /*
     * the num value is only valid if we are not using devfs.
     * However, since we use them to retrieve the device pointer, we
     * don't need them with devfs as filp->private_data is already
     * initialized
     */

    /*
     * If private data is not valid, we are not using devfs
     * so use the num (from minor nr.) to select
     */

    if ((!misc && (num > 0)) || (misc && (num != tmk1553b_mdev.minor))) 
      return -ENODEV;
  
//    if (!filp->private_data || !(dev = (tmk1553b_Dev *)filp->private_data))
//    {
//      if (num >= tmk1553b_nr_devs) return -ENODEV;
//      dev = &tmk1553b_devices[num];
//      filp->private_data = dev; /* for other methods */
//    }

#ifdef LINUX26
    if (!try_module_get(THIS_MODULE))
        return -ENODEV;
#else
    MOD_INC_USE_COUNT;
#endif

    hlnProc = kmalloc(sizeof(TListProc), GFP_KERNEL);
  
    spin_lock_bh(&tmkSpinLock);
  
    for (iTMK = 0; iTMK <= nMaxTmkNumber; ++iTMK)
      hlnProc->fTMK[iTMK] = 0;
    hlnProc->hProc = current->pid;
    hlnProc->nSelectedTMK = -1;
    for (iTMK = 0; iTMK < MAX_VTMK_NUMBER/32+1; ++iTMK)
      hlnProc->maskTMK[iTMK] = 0;
    init_waitqueue_head(&hlnProc->wq);
    hlnProc->waitFlag = 0;
    list_add_tail(&hlnProc->ProcListEntry, &hlProc);
    hCurProc = hlnProc->hProc;
    hlnCurProc = hlnProc;;
    tmkNumber = -1;
  
    spin_unlock_bh(&tmkSpinLock);

    return 0;          /* success */
}

int tmk1553b_release(struct inode *inode, struct file *filp)
{
    TListProc *hlnProc;
    int iTMK;
    PEPROCESS hProc;
  
    #ifdef MY_DBG
    printk(MY_KERN_DEBUG "Tmk1553b: Closed!!\n");
    #endif
  
    hProc = current->pid;
  
    spin_lock_bh(&tmkSpinLock);
  
    for (hlnProc = (TListProc*)(hlProc.next);
         hlnProc != (TListProc*)(&hlProc);
         hlnProc = (TListProc*)(hlnProc->ProcListEntry.next)
        )
    {
      if (hlnProc->hProc != hProc)
        continue;
      for (iTMK = 0; iTMK <= nMaxTmkNumber; ++iTMK)
      {
        if (hlnProc->fTMK[iTMK] == 0)
          continue;
        tmkselect(iTMK);
        bcreset();
        if (aTMK[iTMK].hEvent)
        {
        //tmkdefevent(0,0);
        //VWIN32_CloseVxDHandle(aTMK[iTMK].hEvent);
//          ObDereferenceObject(aTMK[iTMK].hEvent);
          aTMK[iTMK].hEvent = 0;
        }
        tmkEvents[iTMK>>5] &= ~(1<<(iTMK&0x1F));
        iEvDataBegin[iTMK] = iEvDataEnd[iTMK] = cEvData[iTMK] = cDpcData[iTMK] = 0;
        hlnProc->fTMK[iTMK] = 0;
        hlnProc->maskTMK[iTMK>>5] &= ~(1<<(iTMK&0x1F));
        aTMK[iTMK].hProc = 0;
        aTMK[iTMK].hlnProc = 0;
      }
      list_del((struct list_head *)hlnProc);
      kfree(hlnProc);
      break;
    }
    hCurProc = 0;
    hlnCurProc = NULL;
    tmkNumber = -1;
  
    spin_unlock_bh(&tmkSpinLock);

#ifdef LINUX26
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif
    return 0;
}

/*
 * The ioctl() implementation
 *
 * This is done twice, once the 2.2 way, followed by the 2.0 way.  One
 * would not normally do things in this manner, but we wanted to illustrate
 * both ways...
 */

#ifndef LINUX_20

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
long tmk1553b_uioctl(struct file *filp,
                 unsigned int cmd, unsigned long arg)
#else
int tmk1553b_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
#endif
{

    int err = 0;
    UINT dwService;
    UINT dwRetVal = 0;
    PEPROCESS hProc;
    TListProc *hlnProc;
    USHORT awBuf[64];
    USHORT awIn[5];
    USHORT awOut[6];
    USHORT *ptr=NULL;
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != TMK_IOC_MAGIC) return -ENOTTY;
    dwService = _IOC_NR(cmd);
    if (dwService > MAX_TMK_API /*||
        aCmds[dwService] != cmd*/) return -ENOTTY;
    {
      /*
       * the direction is a bitmask, and VERIFY_WRITE catches R/W
       * transfers. `Type' is user-oriented, while
       * access_ok is kernel-oriented, so the concept of "read" and
       * "write" is reversed
       */
  
      if (_IOC_DIR(cmd) & _IOC_READ)
          err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
      else if (_IOC_DIR(cmd) & _IOC_WRITE)
          err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
      if (err) return -EFAULT;
  
      if (_IOC_DIR(cmd) & _IOC_WRITE)
      {
        if (_IOC_SIZE(cmd) == sizeof(u32[2]))
        {
          __get_user(*((u32*)awIn), (u32*)arg);
          __get_user(*((u32*)awIn+1), (u32*)arg+1);
          // use 32bit ptr on 32bit system or convert it to 64bit ptr on 
          // 64bit system for backward compatibility with pre4.02 programs
          // can fail on 64bit system with addresses above 4G!!!
          ptr = (PUSHORT)((unsigned long)(*((u32*)awIn+1))); // not needed for tmkwaitevents
        }
#ifdef __64BIT__
        else if (_IOC_SIZE(cmd) == sizeof(u64[2]))
        {
          __get_user(*((u32*)awIn), (u32*)arg);
          *((u32*)awIn+1) = 0; // not needed for put/get
          __get_user(ptr, (PUSHORT*)arg+1);
        }
#endif    
        else
          return -EFAULT;
      }
      else // if ((IOC_DIR(cmd) & IOC_READ) == 0)
        *((UINT*)awIn) = (UINT)arg;

      hProc = current->pid;

      switch (dwService)
      {
#ifdef TMK1553B_DEBUG
        case tmk1553b_IOCHARDRESET:
           /*
            * reset the counter to 1, to allow unloading in case
            * of problems. Use 1, not 0, because the invoking
            * process has the device open.
            */
//           while (MOD_IN_USE)
//               MOD_DEC_USE_COUNT;
//           MOD_INC_USE_COUNT;
           /* don't break: fall through and reset things */
           break;
#endif /* TMK1553B_DEBUG */
      case VTMK_bcputblk:
      case VTMK_mtputblk:
      case VTMK_rtputblk:
        if (awIn[1] > 64 || awIn[1] == 0)
          break;
        if (__copy_from_user(
              awBuf,
              ptr,
              awIn[1]<<1
              ))
          return -EFAULT;
        break;
      case VTMK_rtputflags:
        awIn[4] = awIn[0] & RT_DIR_MASK;
        *((PUINT)(awIn)) &= ~(RT_DIR_MASK | (RT_DIR_MASK<<16));
        if (awIn[1] < awIn[0] || awIn[1] > 31)
          break;
        if (__copy_from_user(
              awBuf,
              ptr,
              (awIn[1]-awIn[0]+1)<<1
              ))
          return -EFAULT;
        break;
      case VTMK_rtgetflags:
        awIn[4] = awIn[0] & RT_DIR_MASK;
        *((PUINT)(awIn)) &= ~(RT_DIR_MASK | (RT_DIR_MASK<<16));
        break;
      case VTMK_tmkwaiteventsflag:
        hProc = *((PUINT)awIn);
      }

#ifdef LINUX26
//      #ifdef MY_DBG
//{
//      printk(MY_KERN_DEBUG "Tmk1553b: pid=%d parentpid=%d rparentpid=%d leaderpid=%d srv=%d.\n",
//        current->pid, current->parent->pid, current->real_parent->pid, current->group_leader->pid, dwService);
//      unsigned long jjj;
/*
      jjj = jiffies + HZ;
      while (jiffies < jjj)
        schedule();
*/
//}
//      #endif
#endif      
  
      spin_lock_bh(&tmkSpinLock);
  
      lpwIn = (PUSHORT)&awIn;
      lpwOut = (PUSHORT)&awOut;
      lpwBuf = (PUSHORT)&awBuf;
  
      if (hCurProc != hProc)
      {
        for (hlnProc = (TListProc*)(hlProc.next);
             hlnProc->hProc != hProc;
             hlnProc = (TListProc*)(hlnProc->ProcListEntry.next)
            );
        hCurProc = hProc;
        hlnCurProc = hlnProc;
        tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
      }
  
      tmkError = 0;
//      err = 0;
      dwRetVal = (TMK_Procs[dwService-2])();
      err = tmkError;
   
      spin_unlock_bh(&tmkSpinLock);

    if (err == 0)
    {
      switch (dwService)
      {
      case VTMK_bcgetblk:
      case VTMK_mtgetblk:
      case VTMK_rtgetblk:
        if (awIn[1] > 64 || awIn[1] == 0)
          break;
        if (__copy_to_user(
              ptr,
              awBuf,
              awIn[1]<<1
              ))
          return -EFAULT;
        break;
      case VTMK_rtgetflags:
        if (awIn[1] < awIn[0] || awIn[1] > 31)
          break;
        if (__copy_to_user(
              ptr,
              awBuf,
              (awIn[1]-awIn[0]+1)<<1
              ))
          return -EFAULT;
        break;
      case VTMK_tmkgetinfo:
        if (__copy_to_user(
              (TTmkConfigData*)arg,
              awBuf,
              sizeof(TTmkConfigData)
              ))
          return -EFAULT;
        break;
      case VTMK_tmkgetevd:
        if (__copy_to_user(
              (PUSHORT)arg,
              awOut,
              12
              ))
          return -EFAULT;
        break;
      default:
        if (_IOC_DIR(cmd) & _IOC_READ)
        {
          __put_user(*((u32*)awOut), (u32*)arg);
/*
          if (dwService == VTMK_tmkgetevd) // _IOC_SIZE(cmd) == 12
          {
            __put_user(*((u32*)awOut+1), (u32*)arg+1);
            __put_user(*((u32*)awOut+2), (u32*)arg+2);
          }
*/
        }
//        else
//          dwRetVal = (UINT)awOut[0];
        break;
      }
    }
  
      if (err != 0)
        err = TMK_BAD_0 - err;
      else
        err = dwRetVal; // has to be > 0 for Ok and < 0 for error !!!
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
    return (long)err;
#else
    return err;
#endif
}

#else  /* LINUX_20 */

change !acces_ok -> verify_area
change __get_user -> get_user
change __put_user -> put_user
change copy_to_from_user -> memcpy_tofs_fromfs

#endif /* LINUX_20 */

/*
 * The following wrappers are meant to make things work with 2.0 kernels
 */
#ifdef LINUX_20
void tmk1553b_release_20(struct inode *ino, struct file *f)
{
    tmk1553b_release(ino, f);
}

/* Redefine "real" names to the 2.0 ones */
#define tmk1553b_release tmk1553b_release_20
#endif  /* LINUX_20 */

struct file_operations tmk1553b_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    owner:      THIS_MODULE,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
    unlocked_ioctl:      tmk1553b_uioctl,
#else
    ioctl:      tmk1553b_ioctl,
#endif
    open:       tmk1553b_open,
    release:    tmk1553b_release,
};

/*
 * Finally, the module stuff
 */

#ifdef MY_CONFIG_DEVFS_FS
devfs_handle_t tmk1553b_devfs_dir;
static char devname[4];
#endif

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void tmk1553b_cleanup_module(void)
{
//    int i;
  int iTMK, jTMK;
  int fReleaseHiddenPorts;
  TIrq *pIrq;
  int hIrq;

  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: Unloading!!\n");
  #endif

  //
  // Disconnect the interrupt and region
  //

/*
  if (fTMKInit)
  {
    tmkdone(ALL_TMKS);
    List_Destroy(hlProc);
  }
*/

  fTMKInit = 0;
  tmkdone(ALL_TMKS);
//what if irq?

  for (hIrq = 0; hIrq < cIrqs; ++hIrq)
  {
    pIrq = ahIrq + hIrq;
    free_irq(pIrq->nIrq, tmkInterruptServiceRoutine + hIrq);
    pIrq->cTmks = 0;
  }
  cIrqs = 0;
  
  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
  {
    if (aTmkConfig[iTMK].nType == -1)
      continue;

    release_region(aTmkConfig[iTMK].wPorts1, aTmkConfig[iTMK].nPorts);

    if (aTmkConfig[iTMK].wPorts2 != 0xFFFF &&
        aTmkConfig[iTMK].wPorts2 != aTmkConfig[iTMK].wPorts1)
      release_region(aTmkConfig[iTMK].wPorts2, aTmkConfig[iTMK].nPorts);

    if (aTmkConfig[iTMK].nType == TMKXI || aTmkConfig[iTMK].nType == TAI || 
        aTmkConfig[iTMK].nType == MRTXI || aTmkConfig[iTMK].nType == MRTAI )
    {
      fReleaseHiddenPorts = 1;
      for (jTMK = 0; jTMK < iTMK; ++jTMK)
      {
        if (aTmkConfig[jTMK].nType != TMKXI && aTmkConfig[jTMK].nType != TAI && 
            aTmkConfig[jTMK].nType != MRTXI && aTmkConfig[jTMK].nType != MRTAI)
          continue;
        if (aTmkConfig[iTMK].wHiddenPorts == aTmkConfig[jTMK].wHiddenPorts)
        {
          fReleaseHiddenPorts = 0;
          break;
        }
      }
      if (fReleaseHiddenPorts)
        release_region(aTmkConfig[iTMK].wHiddenPorts, 128);
    }

    aTmkConfig[iTMK].nType = -1;
  }

  for (iTMK = MAX_TMK_NUMBER+1; iTMK <= nMaxTmkNumber; ++iTMK)
  {
    aTmkConfig[iTMK].nType = -1;
  }

#ifndef MY_CONFIG_DEVFS_FS
    /* cleanup_module is never called if registering failed */
    if (!misc)
      unregister_chrdev(major, name);
    else
      misc_deregister(&tmk1553b_mdev);
#endif

#ifdef TMK1553B_DEBUG /* use proc only if debugging */
    tmk1553b_remove_proc();
#endif

#ifdef MY_CONFIG_DEVFS_FS
    if (tmk1553b_devices) {
        for (i=0; i<tmk1553b_nr_devs; i++) {
// here was ...trim
            // the following line is only used for devfs
            devfs_unregister(tmk1553b_devices[i].handle);
        }
        kfree(tmk1553b_devices);
    }
    // once again, only for devfs
    devfs_unregister(tmk1553b_devfs_dir);
#endif

}

char *apszTmkTypeName[MAX_TMK_TYPE+1] =
                                       {"",
                                        "",
                                        "TMK400",
                                        "TMKMPC",
                                        "RTMK400",
                                        "TMKX",
                                        "TMKXI",
                                        "",
                                        "",
                                        "TA",
                                        "TAI",
                                        "",
                                        "MRTAI"};

char *apszTmkLoadName[MAX_LOAD_TYPE+1] = {"TX1", 
#ifdef LOADRT1
                            "RT1"
#else
""
#endif //def LOADRT1
,
#ifdef LOADTX4
                            "TX4"
#else
""
#endif //def LOADTX4
,
#ifdef LOADTX5
                            "TX5"
#else
""
#endif //def LOADTX5
,
#ifdef LOADTX6
                            "TX6"
#else
""
#endif //def LOADTX6
,
#ifdef LOADMR6
                            "MR6"
#else
""
#endif //def LOADMR6
,
#ifdef LOADTT6
                            "TT6"
#else
""
#endif //def LOADTT6
,
#ifdef LOADTD6
                            "TD6"
#else
""
#endif //def LOADTD6
                            };

int RegInit(int hTMK, UINT tmkPorts1, UINT tmkPorts2, UINT tmkIrq1, int tmkDev, int tmkDevExt, char *pszType, char *pszLoad)
{
  int tmkType, tmkLoad = 0;
  int nPciDeviceID;
  struct pci_dev *pciDev;
  u32 pciSubSystemID, tmkSystemID = 0;
  u16 aPciDeviceID[PLX_MAX_DEVICE_CNT] = {0x9030, 0x9056};
  UINT tmkHiddenPorts = 0;
  UINT tmkLocalReadInt = 0;
  int fDev, nFoundDev;
  int found;
  int subdev1;

  if (pszType == 0) 
    return 0;
  for (tmkType = MIN_TMK_TYPE; tmkType <= MAX_TMK_TYPE; ++tmkType)
  {
    if (strcmp(pszType, apszTmkTypeName[tmkType]) == 0)
      break;
  }
  if (tmkType > MAX_TMK_TYPE)
  {
    printk(MY_KERN_WARNING "tmk1553b: device type '%s' is incorrect!\n", pszType);
    return -1;
  }
  switch (tmkType)
  {
  case TMK400:
    if (tmkPorts2 == 0xFFFF)
    {
      printk(MY_KERN_WARNING "tmk1553b: port address #2 isn't specified!\n");
      return -1;
    }
  case RTMK400:
  case TMKMPC:
  case TMKX:
  case TA:
    if (tmkPorts1 == 0xFFFF || tmkIrq1 == 0xFF)
    {
      printk(MY_KERN_WARNING "tmk1553b: port address or irq line aren't specified!\n");
      return -1;
    }
    break;
#ifdef CONFIG_PCI
  case TMKXI:
  case TAI:
  case MRTAI:
    if (tmkDev == 0)
      tmkDev = 1;
    if (tmkDevExt == 0)
      tmkDevExt = 1;
    if (tmkDevExt > 4)
    {
      printk(MY_KERN_WARNING "tmk1553b: wrong subdevice number!\n");
      return -1;
    }
#ifndef LINUX26
    if (!pcibios_present())
    {
      printk(MY_KERN_WARNING "tmk1553b: Can not find PCI BIOS!\n");
      return -1;
    }
#endif
    fDev = 1;
    pciDev = NULL;
    nFoundDev = 0;
    for(nPciDeviceID = 0; nPciDeviceID < PLX_MAX_DEVICE_CNT; nPciDeviceID++)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
      while ((pciDev = pci_get_device(0x10B5, aPciDeviceID[nPciDeviceID], pciDev)) != NULL)
#else
      while ((pciDev = pci_find_device(0x10B5, aPciDeviceID[nPciDeviceID], pciDev)) != NULL)
#endif
      {
        pci_read_config_dword(pciDev, CFG_SUBSYSTEM_ID, &pciSubSystemID);
        if (pciSubSystemID != ID_TX1PCI &&
            pciSubSystemID != ID_TX1PCI2
#ifdef LOADTX6
            && pciSubSystemID != ID_TX6PCI
            && pciSubSystemID != ID_TX6PCI2
#endif //def LOADTX6
            && pciSubSystemID != ID_TA1PCI
            && pciSubSystemID != ID_TA1PCI4
            && pciSubSystemID != ID_TA1PCI32RT
            && pciSubSystemID != ID_TA1PE2
            && pciSubSystemID != ID_TA1PE4
            && pciSubSystemID != ID_TA1PE32RT
           )
          continue;
        tmkSystemID = pciSubSystemID;
        ++nFoundDev;
//        nFoundIrq = pciReadConfigByte(wBusDevFun, CFG_IRQ);
        if ((fDev && tmkDev == nFoundDev)) // || (fIrq && nIrq == nFoundIrq))
        {
          if (pci_enable_device(pciDev))
          {
            printk(MY_KERN_WARNING "tmk1553b: Can not enable specified PCI device!\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
            pci_dev_put(pciDev);
#endif
            return -1;
          }
          pci_read_config_dword(pciDev, CFG_ADDR1, &tmkHiddenPorts);
          tmkHiddenPorts &= 0x0000FFFE;
          subdev1 = tmkDevExt;
          if(pciSubSystemID == ID_TA1PE4)
            subdev1 = ((tmkDevExt - 1) >> 1) + 1;
          pci_read_config_dword(pciDev, CFG_ADDR1+(subdev1<<2), &tmkPorts1);
          tmkPorts1 &= 0x0000FFFE;
          
          if(pciSubSystemID == ID_TA1PE4 && tmkPorts1 != 0)
            tmkPorts1 += 32 * ((tmkDevExt - 1) & 0x1);
          if(pciSubSystemID == ID_TA1PE4 && tmkDevExt == 2 && tmkPorts1 != 0)
            if((inw(tmkHiddenPorts) & 0xFF) != 0xC1)
              tmkPorts1 = 0;

          if (tmkPorts1 == 0)
          {
            printk(MY_KERN_WARNING "tmk1553b: Can not find specified PCI subdevice!\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
            pci_dev_put(pciDev);
#endif
            return -1;
          }
//          pci_read_config_byte(pciDev, CFG_IRQ, &tmkIrq1);
//          tmkIrq1 &= 0x000000FF;
          tmkIrq1 = pciDev->irq;
//          pciWriteConfigWord(wBusDevFun, CFG_ADDR2, wPort | 1);
          if (tmkSystemID == ID_TX1PCI2 || tmkSystemID == ID_TX6PCI2)
            tmkLocalReadInt = 1;
          else
            tmkLocalReadInt = 0;
          if (tmkSystemID == ID_TA1PE2 || tmkSystemID == ID_TA1PE4 || tmkSystemID == ID_TA1PE32RT)
	  {
            outw(inw(tmkHiddenPorts + 0x68) | 0x0800, tmkHiddenPorts + 0x68); //Enable INTi#
            outw(inw(tmkHiddenPorts + 0x6A) & 0xFFFE, tmkHiddenPorts + 0x6A); //Disable INTo#
	  }
	  nPciDeviceID = PLX_MAX_DEVICE_CNT;
          break;
        }
      }
    }
    if (pciDev == NULL)
    {
      printk(MY_KERN_WARNING "tmk1553b: Can not find specified PCI device!\n");
      return -1;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
    pci_dev_put(pciDev);
#endif
    break;
#endif //def CONFIG_PCI
  default:
    printk(MY_KERN_WARNING "tmk1553b: device type %d isn't supported!\n", tmkType);
    return -1;
  }
  if (tmkType == TMKX || tmkType == TMKXI)
  {
    found = 0;
    if (pszLoad != 0)
    {
      for (tmkLoad = 0; tmkLoad <= MAX_LOAD_TYPE; ++tmkLoad)
      {
        if (apszTmkLoadName[tmkLoad] == 0)
          continue;
        if (strcmp(pszLoad, apszTmkLoadName[tmkLoad]) == 0)
        {
          found = 1;
          break;
        }
      }
    }
    else
    {
      found = 1;
      tmkLoad = 0;
#ifdef LOADTX6
      if (tmkSystemID == ID_TX6PCI || tmkSystemID == ID_TX6PCI2)
        tmkLoad = 4;
#endif //def LOADTX6
    }
    if (!found)
    {
      printk(MY_KERN_WARNING "tmk1553b: load type '%s' is incorrect!\n", pszLoad);
      return -1;
    }
  }
  aTmkConfig[hTMK].nType = (unsigned short)tmkType;
  strcpy(aTmkConfig[hTMK].szName, apszTmkTypeName[tmkType]);
/*  pchS = apszTmkTypeName[tmkType];
  pchD = aTmkConfig[hTMK].szName;
  while (*pchS != '\0')
    *pchD++ = *pchS++;
*/
  aTmkConfig[hTMK].wPorts1 = (unsigned short)tmkPorts1;
  aTmkConfig[hTMK].wPorts2 = (unsigned short)tmkPorts2;
  aTmkConfig[hTMK].wIrq1 = (unsigned short)tmkIrq1;
  aTmkConfig[hTMK].wIrq2 = 0xFF;
  aTmkConfig[hTMK].nLoadType = tmkLoad;
  aTmkConfig[hTMK].wHiddenPorts = (unsigned short)tmkHiddenPorts;
  if (tmkType == TMKXI || tmkType == TAI || tmkType == MRTAI)
    aTmkConfig[hTMK].fIrqShared = 1;
  aTmkConfig[hTMK].fLocalReadInt = tmkLocalReadInt;
  aTmkConfig[hTMK].fMRT = 0;
  aTmkConfig[hTMK].fLoaded = 0;
  if (tmkType == TA || tmkType == TAI || tmkType == MRTAI)
    aTmkConfig[hTMK].nPorts = TA_NR_PORTS;
  else
    aTmkConfig[hTMK].nPorts = TMK_NR_PORTS;
  return 1;
}

int tmk1553b_init_module(void)
{
    int result; //, i;
    int tmk1553b_irq;
    int share;
//    int probe;
    int iTMK, jTMK;
    int fRequestHiddenPorts;
    TIrq *pIrq=NULL;
    int hIrq;
    int iRt, iRt1;
  
    #ifdef MY_DBG
    #ifdef __SMP__
    printk(MY_KERN_DEBUG "Tmk1553b: Entered the Tmk1553b SMP driver!\n");
    #else
    printk(MY_KERN_DEBUG "Tmk1553b: Entered the Tmk1553b UP driver!\n");
    #endif
    #endif

    if (name == 0)
      name = szTmk1553b;

    if (misc && major)
    {
      printk(MY_KERN_WARNING "tmk1553b: 'misc' and 'major' parameters cannot be set both\n");
      return -EINVAL;
    }

    fTMKInit = 0;
/*
    if (RegTmkInit(RegistryPath))
    {
      #ifdef MY_DBG
      printk(MY_KERN_DEBUG "Tmk1553b: Configuration Loading from Registry Failed\n");
      #endif
      return STATUS_UNSUCCESSFUL;
    }
*/
  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
  {
    aTmkConfig[iTMK].nTmk = iTMK;
    aTmkConfig[iTMK].nType = -1;
    aTmkConfig[iTMK].szName[0] = '\0';
    aTmkConfig[iTMK].wPorts1 = aTmkConfig[iTMK].wPorts2 = 0;
    aTmkConfig[iTMK].wIrq1 = aTmkConfig[iTMK].wIrq2 = 0;
    aTmkConfig[iTMK].wHiddenPorts = 0;
    aTmkConfig[iTMK].fIrqShared = 0;
    aTmkConfig[iTMK].fMRT = 0;
    aTmkConfig[iTMK].fLoaded = 0;
    aTmkConfig[iTMK].nPorts = 0;
  }

  if (nrt < 0)
    nrt = 0;

  if (RegInit(0, a0, b0, i0, d0, e0, t0, l0) < 0 ||
      RegInit(1, a1, b1, i1, d1, e1, t1, l1) < 0 ||
      RegInit(2, a2, b2, i2, d2, e2, t2, l2) < 0 ||
      RegInit(3, a3, b3, i3, d3, e3, t3, l3) < 0 ||
      RegInit(4, a4, b4, i4, d4, e4, t4, l4) < 0 ||
      RegInit(5, a5, b5, i5, d5, e5, t5, l5) < 0 ||
      RegInit(6, a6, b6, i6, d6, e6, t6, l6) < 0 ||
      RegInit(7, a7, b7, i7, d7, e7, t7, l7) < 0)
  {
    printk(MY_KERN_WARNING "tmk1553b: RegInit error!\n");
    return -1;
  }

    fTMKInit = 1;

    for (iTMK = 0; iTMK <= (MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1) - 1; ++iTMK)
    {
      adwTmkOptions[iTMK] = MRT_READ_BRC_DATA | MRT_WRITE_BRC_DATA;
    }

  
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    SET_MODULE_OWNER(&tmk1553b_fops);
#endif
#ifdef MY_CONFIG_DEVFS_FS
    /* If we have devfs, create /dev/tmk1553b to put files in there */
    tmk1553b_devfs_dir = devfs_mk_dir(NULL, name, NULL);
    if (!tmk1553b_devfs_dir) return -EBUSY; /* problem */

#else /* no devfs, do it the "classic" way  */    
    if (!misc)
    {
      /*
       * Register your major, and accept a dynamic number. This is the
       * first thing to do, in order to avoid releasing other module's
       * fops in tmk1553b_cleanup_module()
       */
      result = register_chrdev(major, name, &tmk1553b_fops);
      if (result < 0) {
          printk(MY_KERN_WARNING "tmk1553b: can't get major %d for device %s\n", major, name);
          return result;
      }
      if (major == 0) major = result; /* dynamic */

      printk(MY_KERN_INFO "tmk1553b: Registered device %s, major=%d\n", name, major);
    }
    else
    {
      tmk1553b_mdev.minor = MISC_DYNAMIC_MINOR;
      tmk1553b_mdev.name = name;
      tmk1553b_mdev.fops = &tmk1553b_fops;
      result = misc_register(&tmk1553b_mdev);
      if (result < 0) {
          printk(MY_KERN_WARNING "tmk1553b: can't register dynamic misc device %s\n", name);
          return result;
      }

      printk(MY_KERN_INFO "tmk1553b: Registered dynamic misc device %s, minor=%d\n", name, tmk1553b_mdev.minor);
    }
#endif /* ndef MY_CONFIG_DEVFS_FS */

    /* 
     * allocate the devices -- we can't have them static, as the number
     * can be specified at load time
     */
/*
    tmk1553b_devices = kmalloc(tmk1553b_nr_devs * sizeof(tmk1553b_Dev), GFP_KERNEL);
    if (!tmk1553b_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(tmk1553b_devices, 0, tmk1553b_nr_devs * sizeof(tmk1553b_Dev));
    for (i=0; i < tmk1553b_nr_devs; i++) {
        tmk1553b_devices[i].quantum = tmk1553b_quantum;
        tmk1553b_devices[i].qset = tmk1553b_qset;
        sema_init(&tmk1553b_devices[i].sem, 1);

#ifdef MY_CONFIG_DEVFS_FS
        sprintf(devname, "%i", i);
        devfs_register(tmk1553b_devfs_dir, devname,
                       DEVFS_FL_AUTO_DEVNUM,
                       0, 0, S_IFCHR | S_IRUGO | S_IWUGO,
                       &tmk1553b_fops,
                       tmk1553b_devices+i);
#endif  
    }
*/

    /* At this point call the init function for any friend device */
//    if ( (result = tmk1553b_access_init()) )
//        goto fail;
    /* ... */

  spin_lock_init(&tmkSpinLock);
  spin_lock_init(&tmkIrqSpinLock);

  //
  // connect the device driver to the IRQs
  //

  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
  {
    if (aTmkConfig[iTMK].nType == -1)
      continue;

    //
    // check if resources (ports and interrupt) are available
    //

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
    result = check_region(aTmkConfig[iTMK].wPorts1, aTmkConfig[iTMK].nPorts);
#else
    if (!request_region(aTmkConfig[iTMK].wPorts1, aTmkConfig[iTMK].nPorts, name))
      result = -EINVAL;
    else
      result = 0;
#endif
    if (result) {
        printk(MY_KERN_WARNING "tmk1553b: can't get I/O port 1 address 0x%x for TMK%d\n",
           aTmkConfig[iTMK].wPorts1, iTMK);
        goto fail;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
    request_region(aTmkConfig[iTMK].wPorts1, aTmkConfig[iTMK].nPorts, name);
#endif

    if (aTmkConfig[iTMK].wPorts2 != 0xFFFF &&
        aTmkConfig[iTMK].wPorts2 != aTmkConfig[iTMK].wPorts1)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
      result = check_region(aTmkConfig[iTMK].wPorts2, aTmkConfig[iTMK].nPorts);
#else
      if (!request_region(aTmkConfig[iTMK].wPorts2, aTmkConfig[iTMK].nPorts, name))
        result = -EINVAL;
      else
        result = 0;
#endif
      if (result) {
          printk(MY_KERN_WARNING "tmk1553b: can't get I/O port 2 address 0x%x for TMK%d\n",
             aTmkConfig[iTMK].wPorts2, iTMK);
          goto fail;
      }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
      request_region(aTmkConfig[iTMK].wPorts2, aTmkConfig[iTMK].nPorts, name);
#endif
    }

    if (aTmkConfig[iTMK].nType == TMKXI || aTmkConfig[iTMK].nType == TAI ||
        aTmkConfig[iTMK].nType == MRTAI)
    {
      fRequestHiddenPorts = 1;
      for (jTMK = 0; jTMK < iTMK; ++jTMK)
      {
        if (aTmkConfig[jTMK].nType != TMKXI && aTmkConfig[jTMK].nType != TAI &&
            aTmkConfig[jTMK].nType != MRTAI)
          continue;
        if (aTmkConfig[iTMK].wHiddenPorts == aTmkConfig[jTMK].wHiddenPorts)
        {
          fRequestHiddenPorts = 0;
          break;
        }
      }
      if (fRequestHiddenPorts)
      {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
        result = check_region(aTmkConfig[iTMK].wHiddenPorts, 128);
#else
        if (!request_region(aTmkConfig[iTMK].wHiddenPorts, 128, name))
          result = -EINVAL;
        else
          result = 0;
#endif
        if (result) {
            printk(MY_KERN_WARNING "tmk1553b: can't get hidden I/O port address 0x%x for TMK%d\n",
               aTmkConfig[iTMK].wHiddenPorts, iTMK);
            goto fail;
        }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
        request_region(aTmkConfig[iTMK].wHiddenPorts, 128, name);
#endif
      }
    }
    /*
     * Now we deal with the interrupt: either kernel-based
     * autodetection, DIY detection or default number
     */

       tmk1553b_irq = aTmkConfig[iTMK].wIrq1;
       share = aTmkConfig[iTMK].fIrqShared;
//       probe = 0;

/*
    if (tmk1553b_irq < 0 && probe == 1)
        tmk1553b_kernelprobe();

    if (tmk1553b_irq < 0 && probe == 2)
        tmk1553b_selfprobe();
*/

    /*
     * If shared has been specified, installed the shared handler
     * instead of the normal one. Do it first, before a -EBUSY will
     * force tmk1553b_irq to -1.
     */

    for (hIrq = 0; hIrq < cIrqs; ++hIrq)
    {
      pIrq = ahIrq + hIrq;
      if (pIrq->nIrq == tmk1553b_irq)
        break;
    }
    if (hIrq == cIrqs)
    {
      if (share) 
      {
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: request_irq shared irq%d, hIrq=%d: %X, %X\n", tmk1553b_irq, hIrq, (int)tmkInterruptServiceRoutine, (int)tmkInterruptServiceRoutine + hIrq);
        #endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24) 
        result = request_irq(tmk1553b_irq, tmkInterruptServiceRoutine,
                             SA_SHIRQ | SA_INTERRUPT, name,
                             tmkInterruptServiceRoutine + (ptrdiff_t)hIrq);
#else
        result = request_irq(tmk1553b_irq, tmkInterruptServiceRoutine,
                             IRQF_DISABLED | IRQF_SHARED, name,
                             tmkInterruptServiceRoutine + (ptrdiff_t)hIrq);
#endif
        if (result) {
            printk(MY_KERN_WARNING "tmk1553b: can't get assigned shared irq %i for TMK%d\n", tmk1553b_irq, iTMK);
            goto fail;
        }
      }
      else
      {
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553B: request_irq irq%d, hIrq=%d: %X, %X\n", tmk1553b_irq, hIrq, (int)tmkInterruptServiceRoutine, (int)tmkInterruptServiceRoutine + hIrq);
        #endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24) 
        result = request_irq(tmk1553b_irq, tmkInterruptServiceRoutine,
                             SA_INTERRUPT, name, 
                             tmkInterruptServiceRoutine + (ptrdiff_t)hIrq);
#else
        result = request_irq(tmk1553b_irq, tmkInterruptServiceRoutine,
                             IRQF_DISABLED, name, 
                             tmkInterruptServiceRoutine + (ptrdiff_t)hIrq);
#endif
        if (result) {
            printk(MY_KERN_WARNING "tmk1553b: can't get assigned irq %i for TMK%d\n", tmk1553b_irq, iTMK);
            goto fail;
        }
      }
      pIrq = ahIrq + hIrq;
      pIrq->nIrq = tmk1553b_irq;
      pIrq->cTmks = 0;
      ++cIrqs;
    }
    else
    {
      if (!share)
      {
        printk(MY_KERN_WARNING "tmk1553b: can't share assigned irq %i for TMK%d\n", tmk1553b_irq, iTMK);
        goto fail;
      }
    }
    pIrq->hTmk[pIrq->cTmks++] = iTMK;

  } //end for

  //
  // Initialize the device
  //

//!!! here find max possible nrt from MAX_RT_NUMBER!
  nmrt = 0;
  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
  {
//    if (aTmkConfig[iTMK].nType == -1)
//      continue;
    if (((aTmkConfig[iTMK].nType == TMKX || aTmkConfig[iTMK].nType == TMKXI) && aTmkConfig[iTMK].nLoadType == LOADMR6) ||
         (aTmkConfig[iTMK].nType == MRTA || aTmkConfig[iTMK].nType == MRTAI))
    {
      ++nmrt;
    }
  }
  if (nmrt == 0)
    nmrt = 1;
  nrtmax = (MAX_RT_NUMBER + 1) / nmrt;
  if (nrt == 0 || nrt > nrtmax)
    nrt = nrtmax;

  mrtdefmaxnrt(nrt);

  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
  {
    iEvDataBegin[iTMK] = iEvDataEnd[iTMK] = cEvData[iTMK] = cDpcData[iTMK] = 0;

    if (aTmkConfig[iTMK].nType == -1)
      continue;

    if (aTmkConfig[iTMK].nType == TMKX)
    {
      switch (aTmkConfig[iTMK].nLoadType)
      {
      case LOADTX1:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltx1v11\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf0, sizeof(fpgabuf0));
        break;
#ifdef LOADRT1
      case LOADRT1:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: lrt1v03\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf1, sizeof(fpgabuf1));
        break;
#endif //def LOADRT1
#ifdef LOADTX4  
      case LOADTX4:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltx4v11\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf2, sizeof(fpgabuf2));
        break;
#endif //def LOADTX4
#ifdef LOADTX5  
      case LOADTX5:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltx5v11\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf3, sizeof(fpgabuf3));
        break;
#endif //def LOADTX5
#ifdef LOADTX6  
      case LOADTX6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltx6v11\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf4, sizeof(fpgabuf4));
        break;
#endif //def LOADTX6    
#ifdef LOADMR6
      case LOADMR6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: lmr6v02\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf7, sizeof(fpgabuf7));
        aTmkConfig[iTMK].nType = MRTX;
        break;
#endif //def LOADMR6
#ifdef LOADTT6
      case LOADTT6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltt6v04\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf8, sizeof(fpgabuf8));
        aTmkConfig[iTMK].nType = TTX;
        break;
#endif //def LOADTT6
#ifdef LOADTD6
      case LOADTD6:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: ltd6v01\n");
        #endif
        tmkxload(aTmkConfig[iTMK].wPorts1, fpgabuf9, sizeof(fpgabuf9));
        aTmkConfig[iTMK].nType = TDX;
        break;
#endif //def LOADTD6
      default:
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: default disabled\n");
        #endif
        aTmkConfig[iTMK].nType = -1;
        continue;
//        printk(MY_KERN_WARNING "tmk1553b: unsupported load type %d requested\n", aTmkConfig[iTMK].nLoadType);
//        goto fail;
//        break;
      }
    }
#ifdef CONFIG_PCI
    if (aTmkConfig[iTMK].nType == TMKXI)
      tmkxiload(aTmkConfig[iTMK].wHiddenPorts, 0);
#endif //def CONFIG_PCI

    switch (aTmkConfig[iTMK].nType)
    {
    case TMK400:
    case TMKMPC:
    case RTMK400:
    case TMKX:
    case TMKXI:
    case MRTX:
    case MRTXI:
    case TA:
    case TAI:
    case MRTA:
    case MRTAI:
      tmkconfig(
        iTMK,
        aTmkConfig[iTMK].nType,
        aTmkConfig[iTMK].wPorts1,
        aTmkConfig[iTMK].wPorts2,
        (unsigned char)aTmkConfig[iTMK].wIrq1,
        (unsigned char)aTmkConfig[iTMK].wIrq2
        );
        #ifdef MY_DBG
        printk(MY_KERN_DEBUG "Tmk1553b: tmkconfig(%d, %d, 0x%X, 0x%X, %d, %d)\n",
          iTMK,
          aTmkConfig[iTMK].nType,
          aTmkConfig[iTMK].wPorts1,
          aTmkConfig[iTMK].wPorts2,
          (unsigned char)aTmkConfig[iTMK].wIrq1,
          (unsigned char)aTmkConfig[iTMK].wIrq2
        );
        #endif
      break;
#ifdef LOADTT6
    case TTX:
    case TTXI:
#endif
#ifdef LOADTD6    
    case TDX:
    case TDXI:
#endif    
      break;
    }

    switch (aTmkConfig[iTMK].nType)
    {
    case TMK400:
    case TMKMPC:
    case RTMK400:
      tmkiodelay(30);
    case TMKX:
    case TMKXI:
    case TA:
    case TAI:
      bcreset();
//      #ifdef MY_DBG
      if (TMK_TuneIODelay(0))
        printk(MY_KERN_WARNING "tmk1553b: Unable to tune IO Delay for TMK%d\n", iTMK);
//      #else
//      TMK_TuneIODelay(0);
//      #endif
      break;
#ifdef LOADMR6
    case MRTX:
    case MRTXI:
    case MRTA:
    case MRTAI:
      iRt = mrtgetrt0();
      aTmkConfig[iTMK].fMRT = 0;
      iRt1 = iRt + mrtgetnrt();
      #ifdef MY_DBG
      printk(MY_KERN_DEBUG "Tmk1553b: MRT:");
      #endif
      while (iRt < iRt1)
      {
        tmkError = 0;
        tmkselect(iRt);
        rtreset();
        #ifdef MY_DBG
        if (tmkError)
          printk(MY_KERN_DEBUG "-");
        else
          printk(MY_KERN_DEBUG "+");
        #endif
        bcreset();
        tmkError = 0;
        ++iRt;
      }
      #ifdef MY_DBG
      printk(MY_KERN_DEBUG "\n");
      #endif
      break;
#endif //def LOADMR6
#ifdef LOADTT6
    case TTX:
    case TTXI:
#endif //def LOADTT6
#ifdef LOADTT6
    case TDX:
    case TDXI:
#endif //def LOADTD6
      break;
    }
  }

  nMaxTmkNumber = tmkgetmaxn();

  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: just about ready!\n");
  #endif

  for (iTMK = 0; iTMK < (MAX_VTMK_NUMBER/32 + 1); ++iTMK)
    tmkEvents[iTMK] = 0;
  hCurProc = 0;
  INIT_LIST_HEAD(&hlProc);
  for (iTMK = 0; iTMK <= nMaxTmkNumber; ++iTMK)
  {
    aTMK[iTMK].hProc = 0;
    aTMK[iTMK].hlnProc = 0;
    aTMK[iTMK].hEvent = 0;
  //if (tmkselect(iTMK))
  //  continue;
  //tmkdefevent(0,0);
  }
  tmkNumber = -1;

  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: All initialized!\n");
  #endif

#ifndef TMK1553B_DEBUG
    EXPORT_NO_SYMBOLS; /* otherwise, leave global symbols visible */
#endif

#ifdef TMK1553B_DEBUG /* only when debugging */
    tmk1553b_create_proc();
#endif

    return 0; /* succeed */

  fail:
    tmk1553b_cleanup_module();
    return result;
}

module_init(tmk1553b_init_module);
module_exit(tmk1553b_cleanup_module);


UINT TMK_tmkgetmaxn()
{
//  lpwOut[0] = (USHORT)tmkgetmaxn();
//  return tmkError;
  return tmkgetmaxn();
}
UINT TMK_tmkconfig()
{
  int nRt0;
  USHORT aw0[30];
  
  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: tmkconfig\n");
  #endif

  if (tmkselect(tmkNumber = (int)lpwIn[0]) || tmkgetmode() == MRT_MODE || aTMK[tmkNumber].hProc)
  {
    tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
    tmkError = TMK_BAD_NUMBER;
  }
  else
  {
    if (bcreset() && tmkNumber > MAX_TMK_NUMBER)
    {
      nRt0 = mrtgetrt0();
      if (aTMK[nRt0].hProc == 0)
      {
        tmkselect(nRt0);
        memset(aw0, 0, sizeof(aw0)); // RtlZeroMemory(aw0, sizeof(aw0));
        rtputflags(aw0, RT_RECEIVE, 1, 30);
        tmkselect(tmkNumber);
      }
      tmkError = 0;
    }
    hlnCurProc->fTMK[tmkNumber] = 1;
    hlnCurProc->maskTMK[tmkNumber>>5] |= (1<<(tmkNumber&0x1F));
    hlnCurProc->nSelectedTMK = tmkNumber;
    aTMK[tmkNumber].hProc = hCurProc;
    aTMK[tmkNumber].hlnProc = hlnCurProc;
    tmkEvents[tmkNumber>>5] &= ~(1<<(tmkNumber&0x1F));
    iEvDataBegin[tmkNumber] = iEvDataEnd[tmkNumber] = cEvData[tmkNumber] = 0;
  }
  return tmkError;
}
UINT TMK_tmkdone()
{
  int nMinTMK, nMaxTMK;
  int nPrevTMK;
  int iTMK;
  int iRt, nMaxRt;
  USHORT aw0[30];

  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: tmkdone\n");
  #endif

  nPrevTMK = tmkNumber;
  iTMK = (int)lpwIn[0];
  if (iTMK == ALL_TMKS)
  {
    nMinTMK = 0;
    nMaxTMK = nMaxTmkNumber;
  }
  else if (iTMK >= 0 && iTMK <= nMaxTmkNumber && aTMK[iTMK].hProc == hCurProc)
  {
    nMinTMK = nMaxTMK = iTMK;
  }
  else
  {
    tmkError = TMK_BAD_NUMBER;
    return tmkError;
  }
  for (iTMK = nMinTMK; iTMK <= nMaxTMK; ++iTMK)
  {
    if (aTMK[iTMK].hProc != hCurProc)
      continue;
    tmkselect(iTMK);
    if (iTMK <= MAX_TMK_NUMBER && aTmkConfig[iTMK].fMRT)
    {
      iRt = mrtgetrt0();
      nMaxRt = mrtgetnrt() + iRt - 1;
      for (; iRt <= nMaxRt; ++iRt)
      {
        if (aTMK[iRt].hProc != hCurProc)
          continue;
        if (aTMK[iTMK].hEvent)
        {
        //tmkdefevent(0,0);
        //VWIN32_CloseVxDHandle(aTMK[iTMK].hEvent);
//          ObDereferenceObject(aTMK[iTMK].hEvent);
          aTMK[iTMK].hEvent = 0;
        }
        tmkEvents[iRt>>5] &= ~(1<<(iRt&0x1F));
        iEvDataBegin[iRt] = iEvDataEnd[iRt] = cEvData[iRt] = 0;
        aTMK[iRt].hProc = 0;
        aTMK[iRt].hlnProc = 0;
        hlnCurProc->fTMK[iRt] = 0;
        hlnCurProc->maskTMK[iRt>>5] &= ~(1<<(iRt&0x1F));
      }
      memset(aw0, 0, sizeof(aw0)); // RtlZeroMemory(aw0, sizeof(aw0));
      rtputflags(aw0, RT_RECEIVE, 1, 30);
      aTmkConfig[iTMK].fMRT = 0;
    }

    if (iTMK > MAX_TMK_NUMBER && iTMK == mrtgetrt0())
    {
      memset(aw0, 0, sizeof(aw0)); // RtlZeroMemory(aw0, sizeof(aw0));
      rtputflags(aw0, RT_RECEIVE, 1, 30);
    }

    bcreset();
    if (aTMK[iTMK].hEvent)
    {
    //tmkdefevent(0,0);
    //VWIN32_CloseVxDHandle(aTMK[iTMK].hEvent);
//      ObDereferenceObject(aTMK[iTMK].hEvent);
      aTMK[iTMK].hEvent = 0;
    }
    tmkEvents[iTMK>>5] &= ~(1<<(iTMK&0x1F));
    iEvDataBegin[iTMK] = iEvDataEnd[iTMK] = cEvData[iTMK] = 0;
    aTMK[iTMK].hProc = 0;
    aTMK[iTMK].hlnProc = 0;
    hlnCurProc->fTMK[iTMK] = 0;
    hlnCurProc->maskTMK[iTMK>>5] &= ~(1<<(iTMK&0x1F));
  }
  tmkNumber = -1;
  for (iTMK = 0; iTMK <= nMaxTmkNumber; ++iTMK)
  {
    if (hlnCurProc->fTMK[iTMK])
    {
      tmkselect(tmkNumber = iTMK);
      break;
    }
  }
  hlnCurProc->nSelectedTMK = tmkNumber;
  tmkError = 0;
  return tmkError;
}
UINT TMK_tmkselect()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkselect\n");
//  #endif

  if (tmkselect(tmkNumber = (int)lpwIn[0]) || !hlnCurProc->fTMK[tmkNumber])
  {
    tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
    tmkError = TMK_BAD_NUMBER;
  }
  else
  {
    hlnCurProc->nSelectedTMK = tmkNumber;
  }
  return tmkError;
}
UINT TMK_tmkselected()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkselected\n");
//  #endif

////  lpwOut[0] = (USHORT)tmkselected();
//  lpwOut[0] = (USHORT)hlnCurProc->nSelectedTMK;
//  return tmkError;
  return hlnCurProc->nSelectedTMK;
}
UINT TMK_tmkgetmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmkgetmode();
//  return tmkError;
  return tmkgetmode();
}
UINT TMK_tmksetcwbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_tmkclrcwbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_tmkgetcwbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_tmkwaitevents()
{
  HANDLE hUserEvents;
  int fWait;
  int tmkMyEvents;
  TListProc *hlnSaveCurProc;
  PEPROCESS hSaveCurProc;
//  int tmkSaveNumber;
  long timeout;
  wait_queue_t __wait;
  int fSignal = 0;

//  if (tmkNumber == -1)
//    return (tmkError = TMK_BAD_NUMBER);
  hUserEvents = (HANDLE)(*((PUINT)(lpwIn))) & hlnCurProc->maskTMK[0] & 0x7FFFFFFF;
  fWait = *((int*)(lpwIn+2));
  if (hUserEvents == 0)
    return (tmkError = TMK_BAD_NUMBER);
  tmkMyEvents = hUserEvents & tmkEvents[0];
//  tmkEvents[0] &= ~tmkMyEvents;
  if (fWait == 0 || tmkMyEvents)
  {
    return tmkMyEvents;
  }
  if (fWait > 0)
  {
//    return (tmkError = TMK_BAD_FUNC);
    timeout = fWait * HZ / 1000;
    if (timeout == 0)
      timeout = 1;
  }
  else
    timeout = 0;
  hlnCurProc->maskEvents[0] = hUserEvents;
  hlnSaveCurProc = hlnCurProc;
  hSaveCurProc = hCurProc;
//  tmkSaveNumber = tmkNumber;

//  printk(MY_KERN_DEBUG "Tmk1553b: waitev1 ");

  init_waitqueue_entry(&__wait, current);
  add_wait_queue(&(hlnSaveCurProc->wq), &__wait);
  for (;;)
  {
    if (hlnSaveCurProc->waitFlag)
    {
      hlnSaveCurProc->waitFlag = 0;
      break;
    }
    if (hUserEvents & tmkEvents[0])
      break;
    set_current_state(TASK_INTERRUPTIBLE);
    spin_unlock_bh(&tmkSpinLock);
    if (fWait > 0)
      timeout = schedule_timeout(timeout);
    else
      schedule();
//  printk(MY_KERN_DEBUG "Tmk1553b: waitev w\n");
    spin_lock_bh(&tmkSpinLock);
    if ((fSignal = signal_pending(current)) != 0)
    {
      #ifdef MY_DBG
      printk(MY_KERN_DEBUG "Tmk1553b: break on signal %X, events=%X, myevents=%X\n", signal_pending(current), tmkEvents[0], (hUserEvents&tmkEvents[0]));
      #endif
      break;
    }
    if (hUserEvents & tmkEvents[0])
      break;
    if (fWait > 0 && timeout == 0)
    {
      break;
    }
  }
  set_current_state(TASK_RUNNING);
  remove_wait_queue(&(hlnSaveCurProc->wq), &__wait);
  
  tmkError = 0;

//  printk(MY_KERN_DEBUG "Tmk1553b: waitev2 %X\n", tmkEvents[0]);

//  wait_event_interruptible(hlnSaveCurProc->wq,  hUserEvents & tmkEvents[0]);
//  interruptible_sleep_on_timeout(&hlnCurProc->wq, HZ*2);
  hlnCurProc = hlnSaveCurProc;
  hCurProc = hSaveCurProc;
  tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
  hlnCurProc->maskEvents[0] = 0;
  tmkMyEvents = hUserEvents & tmkEvents[0];
//  tmkEvents[0] &= ~tmkMyEvents;
  if (fSignal)
    return -ERESTARTSYS;
  return tmkMyEvents;
}

UINT TMK_tmkwaiteventsm()
{
  return -EINVAL;
}
/*
{
  HANDLE hUserEvents;
  int cUserEvents;
  int fWait;
  int tmkMyEvents;
  TListProc *hlnSaveCurProc;
  PEPROCESS hSaveCurProc;
//  int tmkSaveNumber;
  long timeout;
  wait_queue_t __wait;
  int fSignal = 0;

//  if (tmkNumber == -1)
//    return (tmkError = TMK_BAD_NUMBER);
  cUserEvents = (*((PUINT)(lpwIn)));
  fWait = *((int*)(lpwIn+2));
  ...
  if (hUserEvents == 0)
    return (tmkError = TMK_BAD_NUMBER);
  tmkMyEvents = hUserEvents & tmkEvents[0];
//  tmkEvents[0] &= ~tmkMyEvents;
  if (fWait == 0 || tmkMyEvents)
  {
    return tmkMyEvents;
  }
  if (fWait > 0)
  {
//    return (tmkError = TMK_BAD_FUNC);
    timeout = fWait * HZ / 1000;
    if (timeout == 0)
      timeout = 1;
  }
  else
    timeout = 0;
  hlnCurProc->maskEvents[0] = hUserEvents;
  hlnSaveCurProc = hlnCurProc;
  hSaveCurProc = hCurProc;
//  tmkSaveNumber = tmkNumber;

//  printk(MY_KERN_DEBUG "Tmk1553b: waitev1 ");

  init_waitqueue_entry(&__wait, current);
  add_wait_queue(&(hlnSaveCurProc->wq), &__wait);
  for (;;)
  {
    if (hUserEvents & tmkEvents[0])
      break;
    set_current_state(TASK_INTERRUPTIBLE);
    spin_unlock_bh(&tmkSpinLock);
    if (fWait > 0)
      timeout = schedule_timeout(timeout);
    else
      schedule();
//  printk(MY_KERN_DEBUG "Tmk1553b: waitev w\n");
    spin_lock_bh(&tmkSpinLock);
    if ((fSignal = signal_pending(current)) != 0)
    {
      #ifdef MY_DBG
      printk(MY_KERN_DEBUG "Tmk1553b: break on signal %X, events=%X, myevents=%X\n", signal_pending(current), tmkEvents[0], (hUserEvents&tmkEvents[0]));
      #endif
      break;
    }
    if (hUserEvents & tmkEvents[0])
      break;
    if (fWait > 0 && timeout == 0)
    {
      break;
    }
  }
  set_current_state(TASK_RUNNING);
  remove_wait_queue(&(hlnSaveCurProc->wq), &__wait);
  
  tmkError = 0;

//  printk(MY_KERN_DEBUG "Tmk1553b: waitev2 %X\n", tmkEvents[0]);

//  wait_event_interruptible(hlnSaveCurProc->wq,  hUserEvents & tmkEvents[0]);
//  interruptible_sleep_on_timeout(&hlnCurProc->wq, HZ*2);
  hlnCurProc = hlnSaveCurProc;
  hCurProc = hSaveCurProc;
  tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
  hlnCurProc->maskEvents[0] = 0;
  tmkMyEvents = hUserEvents & tmkEvents[0];
//  tmkEvents[0] &= ~tmkMyEvents;
  if (fSignal)
    return -ERESTARTSYS;
  return tmkMyEvents;
}
*/
//UINT TMK_tmkdefevent()
//{
//  HANDLE hUserEvent;
////  PKEVENT hKeEvent;
//  int fEventSet;
////  HANDLE *phEvent;
////  int iTMK;
//
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkdefevent\n");
//  #endif
//
//  if (tmkNumber == -1)
//    return (tmkError = TMK_BAD_NUMBER);
//  hUserEvent = (HANDLE)(*((PUINT)(lpwIn))); //(((UINT)lpwIn[0])|((UINT)lpwIn[1]<<16));
//  fEventSet = (*((PUINT)(lpwIn+2))); //((UINT)lpwIn[2])|((UINT)lpwIn[3]<<16);
/*
  if (hUserEvent && !NT_SUCCESS(tmkError = ObReferenceObjectByHandle(
                                               hUserEvent,
                                               EVENT_ALL_ACCESS,
                                               *ExEventObjectType,
                                               KernelMode,
                                               &hKeEvent,
                                               NULL
                                               )))
  {
    return tmkError;
  }
  else
  {
    tmkError = 0;
  }
  if (!hUserEvent)
    hKeEvent = NULL;
  phEvent = &(aTMK[tmkNumber].hEvent);
  if (*phEvent && *phEvent != hKeEvent)
  {
*/
/*
    for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
    {
      if (iTMK == tmkNumber)
        continue;
      if (*phEvent == aTMK[iTMK].hEvent)
        break;
    }
    if (iTMK > MAX_TMK_NUMBER)
*/
/*
    {
//      ObDereferenceObject(*phEvent);
    //VWIN32_CloseVxDHandle(*hEvent);
    }
  }
//tmkdefevent(*phEvent=hUserEvent, fEventSet);
  *phEvent = hKeEvent;
*/
//  return tmkError;
//}
UINT TMK_tmkgetevd()
{
//  void *lpdOut;
//  void *lpdEvD;
  pTListEvD pEvD;

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpdOut = lpwOut;
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkgetevd(%d).\n", tmkNumber);
//  #endif
  while (cEvData[tmkNumber])
  {
    pEvD = &(aEvData[tmkNumber][iEvDataBegin[tmkNumber]]);

    if (pEvD->nInt)
    {
      if (pEvD->awEvData[2])
        DpcIExcBC(tmkNumber, pEvD);
  //    lpdEvD = &(aEvData[tmkNumber][iEvDataBegin[tmkNumber]]);
  //    *((UINT*)lpdOut) = *((UINT*)lpdEvD);
  //    *(++(UINT*)lpdOut) = *(++(UINT*)lpdEvD);
  //    *(++(UINT*)lpdOut) = *(++(UINT*)lpdEvD);
      memcpy(
          lpwOut,
          pEvD,
          12
          );
      break;
    }

    iEvDataBegin[tmkNumber] = (iEvDataBegin[tmkNumber] + 1) & (EVENTS_SIZE-1);
    --cEvData[tmkNumber];
  }

  if (cEvData[tmkNumber])
  {
    iEvDataBegin[tmkNumber] = (iEvDataBegin[tmkNumber] + 1) & (EVENTS_SIZE-1);
    --cEvData[tmkNumber];
  }
  else
  {
//    *((UINT*)lpdOut) = 0;
//    *(++(UINT*)lpdOut) = 0;
//    *(++(UINT*)lpdOut) = 0;
    memset(
        lpwOut,
        0,
        12
        );
  }

//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkgetevd");
//  #endif
  if (cEvData[tmkNumber] == 0)
  {
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG " %X ", tmkEvents[tmkNumber>>5]);
//  #endif
    tmkEvents[tmkNumber>>5] &= ~(1<<(tmkNumber&0x1F));
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG " %X ", tmkEvents[tmkNumber>>5]);
//  #endif
  }
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "\n");
//  #endif

  return tmkError;
}
UINT TMK_tmkgetinfo()
{
//  TTmkConfigData *pUserTmkConfig;
//  char *pchS, *pchD;

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  pUserTmkConfig = (TTmkConfigData*)(*((PUINT)(lpwIn))); //((UINT)(lpwIn[0])+(((UINT)(lpwIn[1]))<<16));
  memcpy(
      lpwBuf,
      &aTmkConfig[mrtselected()],
      sizeof(TTmkConfigData)
      );
/*
  pTmkConfig->nType = aTmkConfig[tmkNumber].nType;
  pchD = pTmkConfig->szName;
  pchS = aTmkConfig[tmkNumber].szName;
  while (*pchD++ = *pchS++);
  pTmkConfig->wPorts1 = aTmkConfig[tmkNumber].wPorts1;
  pTmkConfig->wPorts2 = aTmkConfig[tmkNumber].wPorts2;
  pTmkConfig->wIrq1 = aTmkConfig[tmkNumber].wIrq1;
  pTmkConfig->wIrq2 = aTmkConfig[tmkNumber].wIrq2;
  pTmkConfig->wIODelay = tmkiodelay(GET_IO_DELAY);
*/
  return tmkError;
}
UINT TMK_getversion()
{
  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: TMK_GetVersion\n");
  #endif

//  lpwOut[0] = (USHORT)((TMK_VER_HI<<8)|TMK_VER_LO);
//  lpwOut[1] = 0;
//  return tmkError;
  return ((TMK_VER_HI<<8)|TMK_VER_LO);
}

/*
UINT TMK_bcdefintnorm()
{
  return tmkError;
}
UINT TMK_bcdefintexc()
{
  return tmkError;
}
UINT TMK_bcdefintx()
{
  return tmkError;
}
UINT TMK_bcdefintsig()
{
  return tmkError;
}
*/
UINT TMK_bcreset()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: bcreset\n");
//  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  if (tmkgetmode() == MRT_MODE)
    return (tmkError = TMK_BAD_FUNC);
  bcreset();
  return tmkError;
}
UINT TMK_bc_def_tldw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_bc_enable_di()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_bc_disable_di()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  return tmkError;
}
UINT TMK_bcdefirqmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcdefirqmode(lpwIn[0]);
  return tmkError;
}
UINT TMK_bcgetirqmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcgetirqmode();
//  return tmkError;
  return bcgetirqmode();
}
UINT TMK_bcgetmaxbase()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcgetmaxbase();
//  return tmkError;
  return bcgetmaxbase();
}
UINT TMK_bcdefbase()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: bcdefbase\n");
//  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcdefbase(lpwIn[0]);
  return tmkError;
}
UINT TMK_bcgetbase()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcgetbase();
//  return tmkError;
  return bcgetbase();
}
UINT TMK_bcputw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcputw(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_bcgetw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcgetw(lpwIn[0]);
//  return tmkError;
  return bcgetw(lpwIn[0]);
}
UINT TMK_bcgetansw()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = bcgetansw(lpwIn[0]);
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}
UINT TMK_bcputblk()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcputblk(lpwIn[0],lpwBuf,lpwIn[1]);
  return tmkError;
}
UINT TMK_bcgetblk()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcgetblk(lpwIn[0],lpwBuf,lpwIn[1]);
  return tmkError;
}
UINT TMK_bcdefbus()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcdefbus(lpwIn[0]);
  return tmkError;
}
UINT TMK_bcgetbus()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcgetbus();
//  return tmkError;
  return bcgetbus();
}
UINT TMK_bcstart()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: bcstart\n");
//  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcstart(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_bcstartx()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: bcstartx\n");
//  #endif
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcstartx(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_bcdeflink()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  bcdeflink(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_bcgetlink()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = bcgetlink();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}
UINT TMK_bcstop()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = bcstop();
//  return tmkError;
  return bcstop();
}
UINT TMK_bcgetstate()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = bcgetstate();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

/*
UINT TMK_rtdefintcmd()
{
  return tmkError;
}
UINT TMK_rtdefinterr()
{
  return tmkError;
}
UINT TMK_rtdefintdata()
{
  return tmkError;
}
*/
UINT TMK_rtreset()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: rtreset\n");
//  #endif
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtreset();
  return tmkError;
}
UINT TMK_rtdefirqmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefirqmode(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetirqmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetirqmode();
//  return tmkError;
  return rtgetirqmode();
}
UINT TMK_rtdefmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefmode(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetmode()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetmode();
//  return tmkError;
  return rtgetmode();
}
UINT TMK_rtgetmaxpage()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetmaxpage();
//  return tmkError;
  return rtgetmaxpage();
}
UINT TMK_rtdefpage()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefpage(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetpage()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetpage();
//  return tmkError;
  return rtgetpage();
}
UINT TMK_rtdefpagepc()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefpagepc(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtdefpagebus()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefpagebus(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetpagepc()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetpagepc();
//  return tmkError;
  return rtgetpagepc();
}
UINT TMK_rtgetpagebus()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetpagebus();
//  return tmkError;
  return rtgetpagebus();
}
UINT TMK_rtdefaddress()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefaddress(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetaddress()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetaddress();
//  return tmkError;
  return rtgetaddress();
}
UINT TMK_rtdefsubaddr()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtdefsubaddr(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_rtgetsubaddr()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetsubaddr();
//  return tmkError;
  return rtgetsubaddr();
}
UINT TMK_rtputw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtputw(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_rtgetw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetw(lpwIn[0]);
//  return tmkError;
  return rtgetw(lpwIn[0]);
}
UINT TMK_rtputblk()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtputblk(lpwIn[0],lpwBuf,lpwIn[1]);
  return tmkError;
}
UINT TMK_rtgetblk()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtgetblk(lpwIn[0],lpwBuf,lpwIn[1]);
  return tmkError;
}
UINT TMK_rtsetanswbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtsetanswbits(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtclranswbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtclranswbits(lpwIn[0]);
  return tmkError;
}
UINT TMK_rtgetanswbits()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetanswbits();
//  return tmkError;
  return rtgetanswbits();
}
UINT TMK_rtgetflags()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtgetflags(lpwBuf,lpwIn[0],lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_rtputflags()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtputflags(lpwBuf,lpwIn[0],lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_rtsetflag()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtsetflag();
  return tmkError;
}
UINT TMK_rtclrflag()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtclrflag();
  return tmkError;
}
UINT TMK_rtgetflag()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetflag(lpwIn[0],lpwIn[1]);
//  return tmkError;
  return rtgetflag(lpwIn[0],lpwIn[1]);
}
UINT TMK_rtgetstate()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetstate();
//  return tmkError;
  return rtgetstate();
}
UINT TMK_rtbusy()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtbusy();
//  return tmkError;
  return rtbusy();
}
UINT TMK_rtlock()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtlock(lpwIn[0],lpwIn[1]);
  return tmkError;
}
UINT TMK_rtunlock()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtunlock();
  return tmkError;
}
UINT TMK_rtgetcmddata()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtgetcmddata(lpwIn[0]);
//  return tmkError;
  return rtgetcmddata(lpwIn[0]);
}
UINT TMK_rtputcmddata()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  rtputcmddata(lpwIn[0],lpwIn[1]);
  return tmkError;
}
/*
UINT TMK_mtdefintx()
{
  return tmkError;
}
UINT TMK_mtdefintsig()
{
  return tmkError;
}
*/
UINT TMK_mtreset()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: mtreset\n");
//  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mtreset();
  return tmkError;
}
UINT TMK_mtgetsw()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = mtgetsw();
//  return tmkError;
  return mtgetsw();
}

UINT TMK_rtenable()
{
  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: rtenable\n");
  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = rtenable(lpwIn[0]);
//  return tmkError;
  return rtenable(lpwIn[0]);
}
UINT TMK_mrtgetmaxn()
{
//  lpwOut[0] = (USHORT)mrtgetmaxn();
//  return tmkError;
  return mrtgetmaxn();
}
UINT TMK_mrtconfig()
{
  int iRt, iRt0, nRt;
  USHORT aw0[30];

  #ifdef MY_DBG
  printk(MY_KERN_DEBUG "Tmk1553b: mrtconfig\n");
  #endif

  if (tmkselect(tmkNumber = (int)lpwIn[0]) || tmkgetmode() != MRT_MODE || aTMK[tmkNumber].hProc)
  {
    tmkselect(tmkNumber = hlnCurProc->nSelectedTMK);
    tmkError = TMK_BAD_NUMBER;
//    lpwOut[0] = lpwOut[1] = 0;
    nRt = iRt0 = 0;
  }
  else
  {
    bcreset();
    memset(aw0, 0, sizeof(aw0)); // RtlZeroMemory(aw0, sizeof(aw0));
    rtputflags(aw0, RT_RECEIVE, 1, 30);
    hlnCurProc->fTMK[tmkNumber] = 1;
    hlnCurProc->maskTMK[tmkNumber>>5] |= (1<<(tmkNumber&0x1F));
    hlnCurProc->nSelectedTMK = tmkNumber;
    aTMK[tmkNumber].hProc = hCurProc;
    aTMK[tmkNumber].hlnProc = hlnCurProc;
    tmkEvents[tmkNumber>>5] &= ~(1<<(tmkNumber&0x1F));
    iEvDataBegin[tmkNumber] = iEvDataEnd[tmkNumber] = cEvData[tmkNumber] = 0;
    aTmkConfig[tmkNumber].fMRT = 1;
    iRt0 = mrtgetrt0();
    nRt = mrtgetnrt();
    for (iRt = iRt0+nRt-1; iRt >= iRt0; --iRt)
    {
      if (aTMK[iRt].hProc == 0)
      {
        hlnCurProc->fTMK[iRt] = 1;
        hlnCurProc->maskTMK[iRt>>5] |= (1<<(iRt&0x1F));
      
        aTMK[iRt].hProc = hCurProc;
        aTMK[iRt].hlnProc = hlnCurProc;

        tmkEvents[iRt>>5] &= ~(1<<(iRt&0x1F));
        iEvDataBegin[iRt] = iEvDataEnd[iRt] = cEvData[iRt] = 0;
      }
    }
//    lpwOut[0] = (USHORT)iRt0;
//    lpwOut[1] = (USHORT)nRt;
  }
  return ((nRt<<16) | iRt0);
}
UINT TMK_mrtselected()
{
//  lpwOut[0] = (USHORT)((tmkNumber != -1) ? mrtselected() : -1);
//  return tmkError;
  return ((tmkNumber != -1) ? mrtselected() : -1);
}
UINT TMK_mrtgetstate()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = (USHORT)mrtgetstate();
//  return tmkError;
  return mrtgetstate();
}
UINT TMK_mrtdefbrcsubaddr0()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mrtdefbrcsubaddr0();
  return tmkError;
}
UINT TMK_mrtreset()
{
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: mrtreset\n");
//  #endif

  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  if (tmkgetmode() != MRT_MODE)
    return (tmkError = TMK_BAD_FUNC);
  bcreset();
  return tmkError;
}

UINT TMK_tmktimer()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmktimer(lpwIn[0]);
//  return tmkError;
  return tmktimer(lpwIn[0]);
}

UINT TMK_tmkgettimer()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = tmkgettimer();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

UINT TMK_tmkgettimerl()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmkgettimerl();
//  return tmkError;
  return tmkgettimerl();
}

UINT TMK_bcgetmsgtime()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = bcgetmsgtime();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

UINT TMK_rtgetmsgtime()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = rtgetmsgtime();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

UINT TMK_tmkgethwver()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmkgethwver();
//  return tmkError;
  return tmkgethwver();
}

UINT TMK_tmkgetevtime()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = 0;//tmkgetevtime();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

UINT TMK_tmkswtimer()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmkswtimer(lpwIn[0]);
//  return tmkError;
  return 0;//tmkswtimer(lpwIn[0]);
}

UINT TMK_tmkgetswtimer()
{
  UINT res;
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  res = 0;//tmkgetswtimer();
  lpwOut[0] = (USHORT)(LOWORD(res));
  lpwOut[1] = (USHORT)(HIWORD(res));
  return tmkError;
}

UINT TMK_tmktimeout()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = tmktimeout(lpwIn[0]);
//  return tmkError;
  return tmktimeout(lpwIn[0]);
}

UINT TMK_mrtdefbrcpage()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mrtdefbrcpage(lpwIn[0]);
  return tmkError;
}

UINT TMK_mrtgetbrcpage()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = mrtgetbrcpage();
//  return tmkError;
  return mrtgetbrcpage();
}

UINT TMK_mbcinit()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mbcinit(lpwIn[0]);
  return tmkError;
}

UINT TMK_mbcpreparex()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mbcpreparex(lpwIn[0],lpwIn[1],lpwIn[2],lpwIn[3]);
  return tmkError;
}

UINT TMK_mbcstartx()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mbcstartx(lpwIn[0]);
  return tmkError;
}

UINT TMK_mbcalloc()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
//  lpwOut[0] = mbcalloc();
//  return tmkError;
  return mbcalloc();
}

UINT TMK_mbcfree()
{
  if (tmkNumber == -1)
    return (tmkError = TMK_BAD_NUMBER);
  mbcfree(lpwIn[0]);
  return tmkError;
}

UINT TMK_tmkwaiteventsflag()
{
  int flag = *((PUINT)lpwIn + 1);

  hlnCurProc->waitFlag = flag;
  if(flag)
    wake_up_interruptible(&(hlnCurProc->wq));
  return 0;
}

#include "tmkisr.c"
