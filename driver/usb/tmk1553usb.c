/*
 * tmk1553busb.c -- the tmk1553busb v1.9g usb kernel module.
 * (c) ELCUS, 2008.
 */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif
#include "config.h"
#define TMK1553B_NOCONFIGH
#ifndef TMK1553B_NOCONFIGH
#include <linux/config.h>// for very old kernels
#endif 
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/smp.h>
#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif
#include <linux/usb.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#error Unsupported Kernel Version!
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
#define _LINUX_3_0_
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define _LINUX_2_6_
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define _LINUX_2_4_
#endif
#if defined(_LINUX_3_0_) || defined(_LINUX_2_6_)
MODULE_LICENSE("GPL");
#endif
#ifdef CONFIG_64BIT
#define __64BIT__
#endif

#ifdef _LINUX_3_0_
#include<linux/kthread.h>
#endif

#include "tmk1553busb.h"
/* Module paramaters */

#define MAX_TMK_NUMBER (NTMK - 1)

#define TMK_BAD_0      -1024

/* table of devices that work with this driver */
static struct usb_device_id tmk1553busb_table [] =
{
  { USB_DEVICE(TA1_USB_01_VENDOR_ID, TA1_USB_01_PRODUCT_ID) },
  { }     /* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, tmk1553busb_table);

#ifdef CONFIG_DEVFS_FS
/* the global usb devfs handle */
extern devfs_handle_t usb_devfs_handle;
#endif

#include "tmklllin.h"

u32 TMK_tmkgetmaxn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkconfig(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkdone(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkselect(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkselected(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgetmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmksetcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkclrcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgetcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkwaitevents(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
//u32 TMK_tmkdefevent(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgetevd(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_bcdefintnorm();
//u32 TMK_bcdefintexc();
//u32 TMK_bcdefintx();
//u32 TMK_bcdefintsig();
u32 TMK_bcreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bc_def_tldw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bc_enable_di(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bc_disable_di(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcdefirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetmaxbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcdefbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcputw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetansw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcputblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcdefbus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetbus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcstart(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcstartx(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcdeflink(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetlink(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcstop(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_rtdefintcmd();
//u32 TMK_rtdefinterr();
//u32 TMK_rtdefintdata();
u32 TMK_rtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetmaxpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefpagepc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefpagebus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetpagepc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetpagebus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefaddress(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetaddress(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtdefsubaddr(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetsubaddr(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtputw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtputblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtsetanswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtclranswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetanswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetflags(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtputflags(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtsetflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtclrflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtbusy(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtlock(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtunlock(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtgetcmddata(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_rtputcmddata(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_mtdefintx();
//u32 TMK_mtdefintsig();
u32 TMK_mtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
#define TMK_mtdefirqmode TMK_bcdefirqmode
#define TMK_mtgetirqmode TMK_bcgetirqmode
#define TMK_mtgetmaxbase TMK_bcgetmaxbase
#define TMK_mtdefbase TMK_bcdefbase
#define TMK_mtgetbase TMK_bcgetbase
#define TMK_mtputw TMK_bcputw
#define TMK_mtgetw TMK_bcgetw
u32 TMK_mtgetsw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
#define TMK_mtputblk TMK_bcputblk
#define TMK_mtgetblk TMK_bcgetblk
#define TMK_mtstartx TMK_bcstartx
#define TMK_mtdeflink TMK_bcdeflink
#define TMK_mtgetlink TMK_bcgetlink
#define TMK_mtstop TMK_bcstop
#define TMK_mtgetstate TMK_bcgetstate

u32 TMK_tmkgetinfo(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_getversion(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_rtenable(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_mrtgetmaxn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtconfig(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtselected(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtdefbrcsubaddr0(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmktimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgettimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgettimerl(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_bcgetmsgtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
#define TMK_mtgetmsgtime TMK_bcgetmsgtime
u32 TMK_rtgetmsgtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmkgethwver(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmkgetevtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkswtimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkgetswtimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmktimeout(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_MT_Start(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_MT_GetMessage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_MT_Stop(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_mrtdefbrcpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mrtgetbrcpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_mbcinit(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mbcpreparex(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mbcstartx(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mbcalloc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_mbcfree(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmkwaiteventsflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
u32 TMK_tmkwaiteventsm(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_bcputblk64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
//u32 TMK_bcgetblk64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_rtputblk64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
//u32 TMK_rtgetblk64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

//u32 TMK_rtgetflags64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);
//u32 TMK_rtputflags64(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 TMK_tmkreadsn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf);

u32 (*TMK_Procs[])(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf) = {
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

        TMK_MT_Start,
        TMK_MT_GetMessage,
        TMK_MT_Stop,
        TMK_tmkwaiteventsflag, //115

        TMK_tmkwaiteventsm,

        TMK_tmkwaiteventsflag, //117
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkwaiteventsflag,
        TMK_tmkreadsn //127

//        TMK_bcputblk64,
//        TMK_bcgetblk64,

//        TMK_rtputblk64,
//        TMK_rtgetblk64,

//        TMK_rtgetflags64,
//        TMK_rtputflags64
        };

#define MAX_TMK_API (sizeof(TMK_Procs)/sizeof(void*)+1)

#define LOWORD(l)           ((u16)(l))
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))

/* local function prototypes */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
static long tmk1553busb_uioctl (struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int tmk1553busb_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
static int tmk1553busb_open (struct inode *inode, struct file *file);
static int tmk1553busb_release (struct inode *inode, struct file *file);

#ifdef _LINUX_2_4_
static void * tmk1553busb_probe (struct usb_device *dev, unsigned int ifnum, const struct usb_device_id *id);
static void tmk1553busb_disconnect (struct usb_device *dev, void *ptr);
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
static int tmk1553busb_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void tmk1553busb_disconnect(struct usb_interface *interface);
#endif

void tmk1553busb_delete (struct tmk1553busb * dev);
void kthread_launcher(void *data);
#ifdef _LINUX_3_0_
int int_thread(void * dev);
#else
void int_thread(struct tmk1553busb * dev);
#endif

/* array of pointers to our devices that are currently connected */
struct tmk1553busb * minor_table[MAX_DEVICES];
char intth_run[MAX_DEVICES];
struct list_head hlProc;

/* semaphores to protect the minor_table structure */
#ifdef SPIN_LOCK_BLOCKING
spinlock_t dev_lock[MAX_DEVICES];
spinlock_t list_lock;
#define LOCK_DEVICE(lock) spin_lock_irq(lock)
#define UNLOCK_DEVICE(lock) spin_unlock_irq(lock)
#define LOCK_LIST() spin_lock_irq(&list_lock)
#define UNLOCK_LIST() spin_unlock_irq(&list_lock)
#else
struct semaphore dev_lock[MAX_DEVICES];
struct semaphore list_lock;
#define LOCK_DEVICE(lock) down(lock)
#define UNLOCK_DEVICE(lock) up(lock)
#define LOCK_LIST() down(&list_lock)
#define UNLOCK_LIST() up(&list_lock)
#endif

#ifdef SPIN_LOCK_IRQ_BLOCKING
spinlock_t intLock[MAX_DEVICES];
#define LOCK_IRQ(lock) spin_lock_irq(lock)
#define UNLOCK_IRQ(lock) spin_unlock_irq(lock)
#else
struct semaphore intLock[MAX_DEVICES];
#define LOCK_IRQ(lock) down(lock)
#define UNLOCK_IRQ(lock) up(lock)
#endif

//volatile int tmkEvents = 0;
wait_queue_head_t wq;
#ifdef _LINUX_2_4_
pid_t keventd_pid = -1;
#endif

/* int FIFO stuff */
#define EVENTS_SIZE 1024
TListEvD aEvData_usb[MAX_DEVICES][EVENTS_SIZE];
int iEvDataBegin_usb[MAX_DEVICES];
int iEvDataEnd_usb[MAX_DEVICES];
int cEvData_usb[MAX_DEVICES];
#ifdef ASYNCHRONOUS_IO
char cAsync[MAX_DEVICES];
#endif

/* mt FIFO stuff*/
u8 * MonitorHwBuf[MAX_DEVICES];
u8 MonitorHwBufOF[MAX_DEVICES];
u32 MonitorHwBufRead[MAX_DEVICES];
u32 MonitorHwBufWrite[MAX_DEVICES];
u32 MonitorHwBufSize[MAX_DEVICES];
u32 MonitorHwBufCount[MAX_DEVICES];
u8 MonitorHwTimer[MAX_DEVICES];

static int usbminor = TMK1553BUSB_MINOR_BASE;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,3)
module_param(usbminor, int, 0);
MODULE_PARM_DESC(usbminor, "Minor base number");
#else
MODULE_PARM(usbminor,"i");
#endif

/*
 * File operations needed when we register this driver.
 * This assumes that this driver NEEDS file operations,
 * of course, which means that the driver is expected
 * to have a node in the /dev directory. If the USB
 * device were for a network interface then the driver
 * would use "struct net_driver" instead, and a serial
 * device would use "struct tty_driver".
 */
static struct file_operations tmk1553busb_fops =
{
/*
 * The owner field is part of the module-locking
 * mechanism. The idea is that the kernel knows
 * which module to increment the use-counter of
 * BEFORE it calls the device's open() function.
 * This also means that the kernel can decrement
 * the use-counter again before calling release()
 * or should the open() function fail.
 *
 * Not all device structures have an "owner" field
 * yet. "struct file_operations" and "struct net_device"
 * do, while "struct tty_driver" does not. If the struct
 * has an "owner" field, then initialize it to the value
 * THIS_MODULE and the kernel will handle all module
 * locking for you automatically. Otherwise, you must
 * increment the use-counter in the open() function
 * and decrement it again in the release() function
 * yourself.
 */
#ifdef MY_OWNER
  owner:    THIS_MODULE,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
  unlocked_ioctl:      tmk1553busb_uioctl,
#else
  ioctl:    tmk1553busb_ioctl,
#endif
  open:     tmk1553busb_open,
  release:  tmk1553busb_release,
};

#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
static struct usb_class_driver tmk1553busb_class = {
  name:         "tmk1553busb%d",
  fops:	        &tmk1553busb_fops,
  minor_base:   TMK1553BUSB_MINOR_BASE,
};
#endif

/* usb specific object needed to register this driver with the usb subsystem */
static struct usb_driver tmk1553busb_driver =
{
  name:        "tmk1553busb",
  probe:       tmk1553busb_probe,
  disconnect:  tmk1553busb_disconnect,
#ifdef _LINUX_2_4_
  fops:        &tmk1553busb_fops,
  minor:       TMK1553BUSB_MINOR_BASE,
#endif
  id_table:    tmk1553busb_table,
};

/*
 * tmk1553busb_delete
 */
inline void tmk1553busb_delete(struct tmk1553busb * dev)
{
  if(MonitorHwBuf[dev->minor])
    kfree(MonitorHwBuf[dev->minor]);
  minor_table[dev->minor] = NULL;
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  usb_deregister_dev(dev->interface, &tmk1553busb_class);
#endif
  kfree (dev);
}

/*
 * tmk1553busb_open
 */
static int tmk1553busb_open (struct inode *inode, struct file *file)
{
  struct tmk1553busb * dev = NULL;
  int subminor;
  TListProc *hlnProc;
  TListProc *hlnProc1;

  if(MINOR (inode->i_rdev) >= usbminor)
    subminor = MINOR (inode->i_rdev) - usbminor;
  else
    subminor = MINOR (inode->i_rdev);
  
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: Enter to tmk1553busb_open minor %d procID: %d\n",
         subminor,
         current->pid);
#endif

  if ((subminor < 0) || (subminor >= MAX_DEVICES))
  {
    return -ENODEV;
  }

 /* Increment our usage count for the module.
  * This is redundant here, because "struct file_operations"
  * has an "owner" field. This line is included here soley as
  * a reference for drivers using lesser structures.
  */
#ifndef MY_OWNER
  MOD_INC_USE_COUNT;
#endif

  hlnProc = kmalloc(sizeof(TListProc), GFP_KERNEL);
  if(hlnProc == NULL)
    return -ENODEV;

  /* lock our device and get our local data for this minor */
  LOCK_DEVICE(&dev_lock[subminor]);

  dev = minor_table[subminor];
  if (dev == NULL) //device was removed
  {
    UNLOCK_DEVICE(&dev_lock[subminor]);
#ifndef MY_OWNER
    MOD_DEC_USE_COUNT;
#endif
    kfree(hlnProc);
    return -ENODEV;
  }

  /* increment our usage count for the driver */
  ++dev->open_count;

  /* save our object in the file's private structure */
  file->private_data = dev;

  /* unlock this device */
  UNLOCK_DEVICE(&dev_lock[subminor]);

  LOCK_LIST();
  for(hlnProc1 = (TListProc*)(hlProc.next);
      hlnProc1 != (TListProc*)(&hlProc);
      hlnProc1 = (TListProc*)(hlnProc1->ProcListEntry.next)
      )
  {
    if (hlnProc1->hProc == current->pid)
    {
      hlnProc1->openCnt++;
      kfree(hlnProc);
      goto exit_open;
    }
  }

  hlnProc->hProc = current->pid;
  hlnProc->openCnt = 1;
  hlnProc->waitFlag = 0;
  list_add_tail(&hlnProc->ProcListEntry, &hlProc);

exit_open:
  UNLOCK_LIST();
  return 0;
}

/*
 * tmk1553busb_release
 */
static int tmk1553busb_release (struct inode *inode, struct file *file)
{
  struct tmk1553busb * dev = NULL;
  int subminor;
  TListProc *hlnProc;
  pid_t hProc;

  if(MINOR (inode->i_rdev) >= usbminor)
    subminor = MINOR (inode->i_rdev) - usbminor;
  else
    subminor = MINOR (inode->i_rdev);
  
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: Enter to tmk1553busb_release minor %d\n",
         subminor);
#endif

  if ((subminor < 0) || (subminor >= MAX_DEVICES))
  {
    return -ENODEV;
  }

  hProc = current->pid;
  LOCK_LIST();
  for (hlnProc = (TListProc*)(hlProc.next);
       hlnProc != (TListProc*)(&hlProc);
       hlnProc = (TListProc*)(hlnProc->ProcListEntry.next)
      )
  {
    if (hlnProc->hProc != hProc)
      continue;
    hlnProc->openCnt--;
    if(hlnProc->openCnt <= 0)
    {
      list_del((struct list_head *)hlnProc);
      kfree(hlnProc);
    }
    break;
  }
  UNLOCK_LIST();

  LOCK_DEVICE(&dev_lock[subminor]);
//  dev = (struct tmk1553busb *)file->private_data;
  dev = minor_table[subminor];
  if (dev == NULL)
  {
    UNLOCK_DEVICE(&dev_lock[subminor]);
    return -ENODEV;
  }

  if (dev->udev == NULL)
  {
    /* the device was unplugged before the file was released */
    --dev->open_count;
    if(dev->open_count <= 0)
    {
      tmk1553busb_delete (dev);
      goto exit2;
    }
    goto exit1;
  }

  /* decrement our usage count for the device */
  --dev->open_count;
  if (dev->open_count <= 0)
  {
    dev->open_count = 0;
  }
exit1:
  if(dev->curproc == current->pid)
  {
    tmkdone_usb(dev->minor);
    LOCK_IRQ(&intLock[dev->minor]);
//    tmkEvents &= ~(1<<(dev->minor));
    dev->event = 0;
    iEvDataBegin_usb[dev->minor] = iEvDataEnd_usb[dev->minor] = cEvData_usb[dev->minor] = 0;
    UNLOCK_IRQ(&intLock[dev->minor]);

    dev->curproc = 0;
  }
exit2:
  UNLOCK_DEVICE(&dev_lock[subminor]);
#ifndef MY_OWNER
  MOD_DEC_USE_COUNT;
#endif
  return 0;
}

/*
 *  tmk1553busb_ioctl
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
static long tmk1553busb_uioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int tmk1553busb_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
  struct tmk1553busb * dev;
  int err = 0;
  int subminor;
  u32 dwService;
  u32 dwRetVal = 0;
  u16 awBuf[64];
  u16 awIn[8];
  u16 awOut[6];
  u16 * ptr = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
  subminor = ((struct tmk1553busb *)filp->private_data)->minor;
#else
  if(MINOR (inode->i_rdev) >= usbminor)
    subminor = MINOR (inode->i_rdev) - usbminor;
  else
    subminor = MINOR (inode->i_rdev);
#endif

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: Enter to tmk1553busb_ioctl minor %d cmd %d\n",
         subminor, _IOC_NR(cmd));
#endif
  if ((subminor < 0) || (subminor >= MAX_DEVICES))
  {
    return -ENODEV;
  }
 /*
  * extract the type and number bitfields, and don't decode
  * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
  */
  if (_IOC_TYPE(cmd) != TMK_IOC_MAGIC) return -ENOTTY;
  dwService = _IOC_NR(cmd);
  if (dwService > MAX_TMK_API) return -ENOTTY;
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
      ptr = (u16*)((unsigned long)(*((u32*)awIn+1))); // not needed for tmkwaitevents
    }
#ifdef __64BIT__
    else if (_IOC_SIZE(cmd) == sizeof(u64[2]))
    {
      __get_user(*((u32*)awIn), (u32*)arg);
      *((u32*)awIn+1) = 0; // not needed for put/get
      __get_user(ptr, (u16**)arg+1);
    }
#endif    
    else
      return -EFAULT;
  }
  else
    *((u32 *)awIn) = (u32)arg;

  switch (dwService)
  {
    case VTMK_MT_GetMessage://update later!!!!
      __get_user(*((u32*)awIn), (unsigned long*)arg);
      __get_user(*((u32*)awIn+1), (unsigned long*)arg+1);
      __get_user(*((unsigned long*)(awIn+4)), (unsigned long*)arg+2);
      break;
    case VTMK_bcputblk:
    case VTMK_mtputblk:
    case VTMK_rtputblk:
      if (awIn[1] > 64 || awIn[1] == 0)
        break;
      if (__copy_from_user(
              awBuf,
//              (u16 *)(*((u32 *)(awIn+2))),
              ptr,
              awIn[1]<<1
              ))
        return -EFAULT;
    break;
    case VTMK_rtputflags:
      awIn[4] = awIn[0] & RT_DIR_MASK;
      *((u32 *)(awIn)) &= ~(RT_DIR_MASK | (RT_DIR_MASK<<16));
      if (awIn[1] < awIn[0] || awIn[1] > 31)
        break;
      if (__copy_from_user(
              awBuf,
//              (u16 *)(*((u32 *)(awIn))),
              ptr,
              (awIn[1]-awIn[0]+1)<<1
              ))
        return -EFAULT;
    break;
    case VTMK_rtgetflags:
      awIn[4] = awIn[0] & RT_DIR_MASK;
      *((u32 *)(awIn)) &= ~(RT_DIR_MASK | (RT_DIR_MASK<<16));
    break;
  }

  LOCK_DEVICE(&dev_lock[subminor]);
  dev = minor_table[subminor];
//  dev = (struct tmk1553busb *)file->private_data;
  if (dev == NULL)
  {
    UNLOCK_DEVICE(&dev_lock[subminor]);
    return -ENODEV;
  }
  /* verify that the device wasn't unplugged */
  if (dev->udev == NULL)
  {
    UNLOCK_DEVICE(&dev_lock[subminor]);
    return -ENODEV;
  }

  tmkError_usb[dev->minor] = 0;
  dwRetVal = (TMK_Procs[dwService-2])(dev, awIn, awOut, awBuf);
  err = tmkError_usb[dev->minor];

  /* unlock the device */
  UNLOCK_DEVICE(&dev_lock[subminor]);
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: ioctl: error %d\n",err);
#endif

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
//              (u16 *)(*((u32 *)(awIn+2))),
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
//              (u16 *)(*((u32 *)(awIn))),
              ptr,
              awBuf,
              (awIn[3]-awIn[2]+1)<<1
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
              (u16 *)arg,//check it!!!
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
//          dwRetVal = (u32)awOut[0];
        break;
    }
  }

  if (err != 0)
    err = TMK_BAD_0 - err;
  else
    err = dwRetVal; // has to be > 0 for Ok and < 0 for error !!!

  return err;
}

/*
 * tmk1553busb_probe
 *
 * Called by the usb core when a new device is connected that it thinks
 * this driver might be interested in.
 */
#ifdef _LINUX_2_4_
static void * tmk1553busb_probe(struct usb_device *udev, unsigned int ifnum, const struct usb_device_id *id)
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
static int tmk1553busb_probe(struct usb_interface *interface, const struct usb_device_id *id)
#endif
{
  struct tmk1553busb * dev = NULL;
#ifdef _LINUX_2_4_
  struct usb_interface * interface;
  struct usb_interface_descriptor * iface_desc;
  struct tq_struct tq;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
struct usb_device * udev;
struct usb_host_interface * iface_desc;
#ifdef TMK1553BUSB_DEBUG
struct usb_endpoint_descriptor * endpoint;
#endif
#endif
  int minor;
  char * chBuf;
  int i;
  unsigned char devtype;
  int ret;
  //char speed[2];
  __u8 length;
  //u8 aSerialNumber[8];
#ifdef CONFIG_DEVFS_FS
  char name[20];
#endif

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: Enter to tmk1553busb_probe %d\n", current->pid);
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  udev = usb_get_dev(interface_to_usbdev(interface));
#endif
  /* See if the device offered us matches what we can accept */
  if ((udev->descriptor.idVendor != TA1_USB_01_VENDOR_ID) ||
      (udev->descriptor.idProduct != TA1_USB_01_PRODUCT_ID))
  {
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }

  chBuf = kmalloc (sizeof(struct usb_string_descriptor), GFP_KERNEL);
  if (chBuf == NULL)
  {
  /* get the string descriptor */
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Out of memory!\n");
#endif
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -ENOMEM;
#endif
  }
  ret = usb_control_msg(udev,
                        usb_rcvctrlpipe(udev, 0),
                        USB_REQ_GET_DESCRIPTOR,
                        USB_DIR_IN,
                        (USB_DT_STRING << 8) + 2,
                        27,
                        chBuf,
                        sizeof(chBuf),
                        5*HZ);

  if(ret < 0)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Error get string descriptor!\n");
#endif
    kfree(chBuf);
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }

  length = ((struct usb_string_descriptor *)chBuf)->bLength;
  kfree(chBuf);
  if(length == 0)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Error string descriptor length!\n");
#endif
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: probe: ctrlrequest ret: %d string dscr len: %d\n",
                    ret, length);
#endif
  /* get real string descriptor */
  chBuf = kmalloc (sizeof(__u8) * length, GFP_KERNEL);
  if (chBuf == NULL)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Out of memory!\n");
#endif
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -ENOMEM;
#endif
  }
  ret = usb_control_msg(udev,
                        usb_rcvctrlpipe(udev, 0),
                        USB_REQ_GET_DESCRIPTOR,
                        USB_DIR_IN,
                        (USB_DT_STRING << 8) + 2,
                        27,
                        chBuf,
                        length,
                        5*HZ);

  if(ret < 0)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Error get string descriptor!\n");
#endif
    kfree(chBuf);
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }

  length = ((struct usb_string_descriptor *)chBuf)->bLength;
  if(length == 0)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: Error string descriptor length!\n");
#endif
    kfree(chBuf);
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: probe: ctrlrequest 2 ret: %d string dscr len: %d\n",
                    ret, length);
#endif

  for(devtype = 0; devtype < TMKUSB_DEVICES_NUM;)
  {
    if((strlen(tmkusb_device_name[devtype])) == (length / 2) - 1)
    {
      for(i = 0; i < (length / 2) - 1; i++)
      {
        if((char)((struct usb_string_descriptor *)chBuf)->wData[i] !=
           tmkusb_device_name[devtype][i])
          goto nextdev;
      }
#ifdef TMK1553BUSB_DEBUG
      printk(KERN_INFO "tmk1553busb: probe: driver supports the device %s\n",
                        tmkusb_device_name[devtype]);
#endif
      break;
    }
nextdev:
    devtype++;
  }

  kfree(chBuf);

  if(devtype >= TMKUSB_DEVICES_NUM)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: probe: driver do not support the device\n");
#endif
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -EPERM;
#endif
  }
  else
    devtype++;

  /* allocate memory for our device state and intialize it */
  dev = kmalloc (sizeof(*dev), GFP_KERNEL);
  if (dev == NULL)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: Out of memory!\n");
#endif
//    goto exit;
#ifdef _LINUX_2_4_
    return NULL;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    return -ENOMEM;
#endif
  }
  memset (dev, 0x00, sizeof (*dev));

  /* select a "subminor" number (part of a minor number) */
#ifdef _LINUX_2_4_
  for (minor = 0; minor < MAX_DEVICES; ++minor)
  {
    LOCK_DEVICE(&dev_lock[minor]);
    if (minor_table[minor] == NULL)
      break;//found free minor
    else
      UNLOCK_DEVICE(&dev_lock[minor]);
  }

  if (minor >= MAX_DEVICES)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: Too many devices plugged in\n");
#endif
    kfree(dev);
    return NULL;
  }
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  usb_set_intfdata(interface, dev);
  tmk1553busb_class.minor_base = usbminor;
  if (usb_register_dev(interface, &tmk1553busb_class)) 
  {
    /* something prevented us from registering this driver */
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: Not able to get a minor for this device\n");
#endif
    usb_set_intfdata(interface, NULL);
    kfree(dev);
    return -EPERM;
  }
  if((interface->minor) >= usbminor)
    minor = interface->minor - usbminor;
  else
    minor = interface->minor;
  if((minor >= MAX_DEVICES) || (minor < 0))
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: Too many devices plugged in\n");
#endif
    usb_set_intfdata(interface, NULL);
    usb_deregister_dev(interface, &tmk1553busb_class);
    goto error;
  }
#endif

  minor_table[minor] = dev;

#ifdef _LINUX_2_4_
  interface = &udev->actconfig->interface[ifnum];
#endif

  dev->udev = udev;
  dev->interface = interface;
  dev->minor = minor;
  dev->device_type = devtype;
  dev->curproc = 0;
#ifdef ASYNCHRONOUS_IO
  cAsync[minor] = 1;
#endif
  dev->intLoopMode = INT_LOOP_MODE;
  dev->ResetEP6mt = 0;
  dev->ResetEP6int = 0;
//  dev->event = 0;
  intth_run[minor] = 1;

  LOCK_IRQ(&intLock[minor]);
  dev->event = 0;
  iEvDataBegin_usb[minor] = iEvDataEnd_usb[minor] = cEvData_usb[minor] = 0;
  UNLOCK_IRQ(&intLock[dev->minor]);

  /* set up the endpoint information */
  /* check out the endpoints */
#ifdef _LINUX_2_4_
  iface_desc = &interface->altsetting[0];
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  iface_desc = interface->cur_altsetting;
#endif

  switch(dev->device_type)
  {
    case TA1_USB_01_ID:
#ifdef TMK1553BUSB_DEBUG
#ifdef _LINUX_2_4_
      printk(KERN_INFO "tmk1553busb: Config info: Device #%d NumEP %d\n",
                       minor,
                       iface_desc->bNumEndpoints);
      for (i = 0; i < iface_desc->bNumEndpoints; ++i)
      {
        printk(KERN_INFO "tmk1553busb: Config info: EP%d Adr: 0x%X Type: %d Size: %d\n",
                         iface_desc->endpoint[i].bEndpointAddress & 0x0F,
                         iface_desc->endpoint[i].bEndpointAddress,
                         iface_desc->endpoint[i].bmAttributes & 3,
                         iface_desc->endpoint[i].wMaxPacketSize);
      }
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
      printk(KERN_INFO "tmk1553busb: Config info: Device #%d NumEP %d\n",
                       minor,
                       iface_desc->desc.bNumEndpoints);
      for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i)
      {
        endpoint = &iface_desc->endpoint[i].desc;
        printk(KERN_INFO "tmk1553busb: Config info: EP%d Adr: 0x%X Type: %d Size: %d\n",
                         endpoint->bEndpointAddress & 0x0F,
                         endpoint->bEndpointAddress,
                         endpoint->bmAttributes & 3,
                         endpoint->wMaxPacketSize);
      }
#endif
#endif
#ifdef _LINUX_2_4_
      if(iface_desc->bNumEndpoints != 4)
        goto error;
      dev->ep2_address = iface_desc->endpoint[0].bEndpointAddress;
      dev->ep4_address = iface_desc->endpoint[1].bEndpointAddress;
      dev->ep6_address = iface_desc->endpoint[2].bEndpointAddress;
      dev->ep8_address = iface_desc->endpoint[3].bEndpointAddress;

      dev->ep2_maxsize = iface_desc->endpoint[0].wMaxPacketSize;
      dev->ep4_maxsize = iface_desc->endpoint[1].wMaxPacketSize;
      dev->ep6_maxsize = iface_desc->endpoint[2].wMaxPacketSize;
      dev->ep8_maxsize = iface_desc->endpoint[3].wMaxPacketSize;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
      if(iface_desc->desc.bNumEndpoints != 4)
      {
        usb_set_intfdata(interface, NULL);
        usb_deregister_dev(interface, &tmk1553busb_class);
        goto error;
      }
      dev->ep2_address = iface_desc->endpoint[0].desc.bEndpointAddress;
      dev->ep4_address = iface_desc->endpoint[1].desc.bEndpointAddress;
      dev->ep6_address = iface_desc->endpoint[2].desc.bEndpointAddress;
      dev->ep8_address = iface_desc->endpoint[3].desc.bEndpointAddress;

      dev->ep2_maxsize = iface_desc->endpoint[0].desc.wMaxPacketSize;
      dev->ep4_maxsize = iface_desc->endpoint[1].desc.wMaxPacketSize;
      dev->ep6_maxsize = iface_desc->endpoint[2].desc.wMaxPacketSize;
      dev->ep8_maxsize = iface_desc->endpoint[3].desc.wMaxPacketSize;
#endif

      /* device info */
      dev->tmkConfigData_usb.nType = TA;
      strcpy(dev->tmkConfigData_usb.szName, "TA1USB");
      dev->tmkConfigData_usb.wPorts1 = 0;
      dev->tmkConfigData_usb.wPorts2 = 0;
      dev->tmkConfigData_usb.wIrq1 = 0;
      dev->tmkConfigData_usb.wIrq2 = 0;
      dev->tmkConfigData_usb.wIODelay = 0;
    break;
  }
  /* check the firmware version */
  ret = usb_control_msg(dev->udev,
                        usb_rcvctrlpipe(dev->udev, 0),
                        0xb9,
                        USB_DIR_IN,
                        0,
                        0,
                        dev->fwver,
                        2,
                        5*HZ);
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: firmware version: %04X\n",
                     ((u16)(dev->fwver[0]) << 8) + (u16)(dev->fwver[1]));
#endif
  if(ret < 0 || ((u16)(dev->fwver[0]) << 8) + (u16)(dev->fwver[1]) < TMKUSB_FWVER_MIN)
  {
    printk(KERN_INFO "tmk1553busb: unsupported firmware version!\n");
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    usb_set_intfdata(interface, NULL);
    usb_deregister_dev(interface, &tmk1553busb_class);
#endif
    goto error;
  }

  /* read serial number */
  char *aSerialNumber = kmalloc(8, GFP_DMA);
  ret = usb_control_msg(dev->udev,
                        usb_rcvctrlpipe(dev->udev, 0),
                        0xb4,
                        USB_DIR_IN,
                        0,
                        0,
                        aSerialNumber,
                        8,
                        5*HZ);
  if(ret < 0)
  {
    printk(KERN_INFO "tmk1553busb: serial number read error!\n");
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
    usb_set_intfdata(interface, NULL);
    usb_deregister_dev(interface, &tmk1553busb_class);
#endif
    kfree(aSerialNumber);
    goto error;
  }

  dev->SerialNumber = aSerialNumber[7] - '0';
  dev->SerialNumber += (aSerialNumber[6] - '0') * 10;
  dev->SerialNumber += (aSerialNumber[5] - '0') * 100;
  dev->SerialNumber += (aSerialNumber[4] - '0') * 1000;
  dev->SerialNumber += (aSerialNumber[3] - '0') * 10000;
  dev->SerialNumber += (aSerialNumber[2] - '0') * 100000;
  dev->SerialNumber += (aSerialNumber[1] - '0') * 1000000;
  dev->SerialNumber += (aSerialNumber[0] - '0') * 10000000;
  kfree(aSerialNumber);
  char *speed = kmalloc(2, GFP_DMA);
  /* set up the device speed */
  ret = usb_control_msg(dev->udev,
                        usb_rcvctrlpipe(dev->udev, 0),
                        0xb8,
                        USB_DIR_IN,
                        0,
                        0,
                        speed,
                        2,
                        5*HZ);
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: USB speed %d\n", speed[1]);
#endif
kfree(speed);
//start thread
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
  sema_init(&dev->startstopth_sem, 0);
#else
  init_MUTEX_LOCKED(&dev->startstopth_sem);
#endif

#ifdef _LINUX_2_4_
  tq.sync = 0;
  INIT_LIST_HEAD(&tq.list);
  tq.routine =  kthread_launcher;
  tq.data = dev;

  /* schedule it for execution */
  ret = schedule_task(&tq);
  if(!ret)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: int_thread start error!\n");
#endif
    goto error;
  }
#endif
#ifdef _LINUX_2_6_
  kernel_thread((int (*)(void *))kthread_launcher, (void *)dev,
                CLONE_FILES | CLONE_FS |
                CLONE_SIGHAND);
#endif
#ifdef _LINUX_3_0_
  kthread_run(&int_thread,(void *)dev,"tmk1553busb_int_thread");
#endif
  down(&dev->startstopth_sem);

#ifdef CONFIG_DEVFS_FS
  /* initialize the devfs node for this device and register it */
  sprintf(name, "tmk1553busb%d", dev->minor);
  dev->devfs = devfs_register(usb_devfs_handle,
                              name,
                              DEVFS_FL_DEFAULT,
                              USB_MAJOR,
                              usbminor + dev->minor,
                              S_IFCHR | S_IRUSR | S_IWUSR |
                              S_IRGRP | S_IWGRP | S_IROTH,
                              &tmk1553busb_fops,
                              NULL);

  /* let the user know what node this device is now attached to */
  if(dev->devfs != NULL)
    printk(KERN_INFO "tmk1553busb: device attached to tmk1553busb%d \
                     major %d minor %d\n",
                     dev->minor,
                     USB_MAJOR,
                     usbminor + dev->minor);
  else
    printk(KERN_INFO "tmk1553busb: devfs_register error. major %d minor %d\n",
                     USB_MAJOR,
                     usbminor + dev->minor);//check in devfs
#else
    printk(KERN_INFO "tmk1553busb: device #%d connected: major %d minor %d\n",
                     dev->minor,
                     USB_MAJOR,
                     usbminor + dev->minor);
#endif  //devfs
  goto exit;

error:
//  tmk1553busb_delete (dev);
  kfree(dev);
  minor_table[minor] = NULL;
  dev = NULL;

exit:
  UNLOCK_DEVICE(&dev_lock[minor]);
#ifdef _LINUX_2_4_
  return dev;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  if(dev == NULL)
    return -EPERM;
  else
    return 0;
#endif
}

/*
 * kthread_launcher
 */

#ifndef _LINUX_3_0_
void kthread_launcher(void *data)
{
  struct tmk1553busb * arg = data;
#ifdef _LINUX_2_6_
  daemonize("thread_launcher", 0);
#endif
  kernel_thread((int (*)(void *))int_thread, (void *)arg,
                CLONE_FILES | CLONE_FS |
                CLONE_SIGHAND);
}
#endif

#ifdef _LINUX_3_0_
int int_thread(void * arg)
#else
void int_thread(struct tmk1553busb * dev)
#endif
{
#ifdef _LINUX_3_0_
  struct tmk1553busb * dev = arg;
#endif
  int Result;
  int count;
  unsigned char num = dev->minor;
  unsigned i, j;
  unsigned char * buffer;
  pTListEvD pEvD;
  u32 dwNCopy;
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: int thread start: minor:%d pid:%d par:%d\n",
                   dev->minor,
                   current->pid,
#ifdef _LINUX_2_4_
                   current->p_pptr->pid
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
                   0
#endif
                   );
#endif
#ifdef _LINUX_2_4_
  if(keventd_pid < 0)
    keventd_pid = current->p_pptr->pid;
#endif

  siginitsetinv(&current->blocked,
                sigmask(SIGKILL) | sigmask(SIGINT) | sigmask(SIGTERM));

  up(&dev->startstopth_sem);

  buffer = kmalloc (sizeof(char) * dev->ep6_maxsize, GFP_KERNEL);
  if(buffer == NULL)
  {
    printk(KERN_INFO "tmk1553busb: int_thread out of memory!");
    goto exit;
  }
  // start polling
loopmk:
  for(;;)
  {
    switch(dev->intLoopMode)
    {
      case INT_LOOP_MODE:
#ifdef TMK1553BUSB_MT_DEBUG
        printk(KERN_INFO "Tmk1553b: INT_LOOP_MODE\n");
#endif
        if(dev->ResetEP6int)
        {
#ifdef TMK1553BUSB_MT_DEBUG
          printk(KERN_INFO "Tmk1553b: INT_LOOP reset EP6\n");
#endif
          Result = usb_control_msg(dev->udev,
                                   usb_sndctrlpipe(dev->udev, 0),
                                   0xbc,
                                   USB_DIR_OUT,
                                   0,
                                   0,
                                   NULL,
                                   0,
                                   5*HZ);
          dev->ResetEP6int = 0;
        }

        Result = usb_bulk_msg(dev->udev,
                              usb_rcvbulkpipe (dev->udev,
                                               dev->ep6_address),
                              buffer,
                              sizeof(char) * dev->ep6_maxsize,
                              &count, 30000);
                              //(u32)MAX_SCHEDULE_TIMEOUT);
#ifdef TMK1553BUSB_DEBUG
        printk(KERN_INFO "Tmk1553b: Int th minor %d Res %d\n", dev->minor, Result);
#endif
        if(Result != 0 && intth_run[num] == 1)
        {
          set_current_state(TASK_INTERRUPTIBLE);
          schedule_timeout(HZ/100000);
          goto loopmk;
        }

        if(Result != 0 && Result != -ETIMEDOUT)
          goto error;

        if(count != 0)
        {
          LOCK_IRQ(&intLock[num]);

          i = 1;
          j = buffer[0] * 12;
          for(;(i <= j ) && (i < dev->ep6_maxsize);)
          {
            pEvD = &(aEvData_usb[num][iEvDataEnd_usb[num]]);
            pEvD->nInt = (buffer[i] << 8) | buffer[i + 1];
            pEvD->wMode = (buffer[i + 4] << 8) | buffer[i + 5];
            pEvD->awEvData[0] = (buffer[i + 6] << 8) | buffer[i + 7];
            pEvD->awEvData[1] = (buffer[i + 8] << 8) | buffer[i + 9];
            pEvD->awEvData[2] = (buffer[i + 10] << 8) | buffer[i + 11];
            i += 12;

//            iEvDataEnd_usb[num] = (iEvDataBegin_usb[num] + 1) & (EVENTS_SIZE-1);
//            cEvData_usb[num] = (cEvData_usb[num] + 1) & (EVENTS_SIZE-1);
            iEvDataEnd_usb[num] = (iEvDataEnd_usb[num] + 1) & (EVENTS_SIZE-1);
            cEvData_usb[num] = (cEvData_usb[num] + 1) & (EVENTS_SIZE-1);
            if(iEvDataEnd_usb[num] == iEvDataBegin_usb[num])
              iEvDataBegin_usb[num] = (iEvDataBegin_usb[num] + 1) & (EVENTS_SIZE-1);
          }
//          tmkEvents |= (1<<num);
          dev->event = 1;
          wake_up_interruptible(&wq);

          UNLOCK_IRQ(&intLock[num]);
        }
      break;
      case MT_LOOP_MODE:
#ifdef TMK1553BUSB_MT_DEBUG
        printk(KERN_INFO "Tmk1553b: MT_LOOP_MODE\n");
#endif
        if(dev->ResetEP6mt)
        {
#ifdef TMK1553BUSB_MT_DEBUG
          printk(KERN_INFO "Tmk1553b: MT_LOOP reset EP6\n");
#endif
          Result = usb_control_msg(dev->udev,
                                   usb_sndctrlpipe(dev->udev, 0),
                                   0xbc,
                                   USB_DIR_OUT,
                                   0,
                                   0,
                                   NULL,
                                   0,
                                   5*HZ);
          dev->ResetEP6mt = 0;
        }

        Result = usb_bulk_msg(dev->udev,
                              usb_rcvbulkpipe (dev->udev,
                                               dev->ep6_address),
                              buffer,
                              sizeof(char) * dev->ep6_maxsize,
                              &count, 30000);
                              //(u32)MAX_SCHEDULE_TIMEOUT);

        if(Result != 0 && intth_run[num] == 1)
        {
          set_current_state(TASK_INTERRUPTIBLE);
          schedule_timeout(HZ / 100000);
          goto loopmk;
        }

        if(Result != 0 && Result != -ETIMEDOUT)
          goto error;

#ifdef TMK1553BUSB_MT_DEBUG
          printk(KERN_INFO "Tmk1553b: MT_LOOP Interrupt!!!\n");
#endif
        LOCK_IRQ(&intLock[num]);
        if(count != 0 || dev->ep6_maxsize >= 512 || MonitorHwBuf[num] != NULL)
        {
          if((MonitorHwBufSize[num] - MonitorHwBufCount[num]) >= count) //check free space
          {
            buffer[0] |= (0x02 * MonitorHwBufOF[num]); //OF flag
            MonitorHwBufOF[num] = FALSE;

            dwNCopy = MonitorHwBufSize[num] - MonitorHwBufWrite[num];
            if(dwNCopy >= count) //memcpy 1
            {
              memcpy(MonitorHwBuf[num] + MonitorHwBufWrite[num], buffer, count);
              MonitorHwBufWrite[num] += count;
              MonitorHwBufCount[num] += count;
              if(MonitorHwBufWrite[num] >= MonitorHwBufSize[num])
                MonitorHwBufWrite[num] = 0;
            }
            else//memcpy 2
            {
              memcpy(MonitorHwBuf[num] + MonitorHwBufWrite[num], buffer, dwNCopy);
              memcpy(MonitorHwBuf[num], buffer + dwNCopy, count - dwNCopy);

              MonitorHwBufCount[num] += count;
              MonitorHwBufWrite[num] = count - dwNCopy;
            }
          }
          else
          {
            MonitorHwBufOF[num] = TRUE;
          }
          dev->event = 1;
          wake_up_interruptible(&wq);
        }
        UNLOCK_IRQ(&intLock[num]);
      break;
      case SLEEP_LOOP_MODE:
#ifdef TMK1553BUSB_MT_DEBUG
        printk(KERN_INFO "Tmk1553b: SLEEP_LOOP_MODE\n");
#endif
        if(dev->udev == NULL)
          goto error;
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ / 1000);
      break;
      default:
        goto error;
    }
    //spin unlock
  }
error:
  kfree(buffer);
exit:
  up(&dev->startstopth_sem);
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: int thread stop. minor: %d\n", dev->minor);
#endif
#ifdef _LINUX_3_0_
  return 0;
#endif
}

/*
 * tmk1553busb_disconnect
 *
 * Called by the usb core when the device is removed from the system.
 */
#ifdef _LINUX_2_4_
static void tmk1553busb_disconnect(struct usb_device *udev, void *ptr)
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
static void tmk1553busb_disconnect(struct usb_interface *interface)
#endif
{
  struct tmk1553busb * dev;
  int minor;

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: Enter to tmk1553busb_disconnect\n");
#endif

#ifdef _LINUX_2_4_
  dev = (struct tmk1553busb *)ptr;
  if(dev == NULL)
    return;
  minor = dev->minor;
#endif
#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  if((interface->minor) >= usbminor)
    minor = interface->minor - usbminor;
  else
    minor = interface->minor;
#endif
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: tmk1553busb_disconnect minor %d\n", minor);
#endif
  intth_run[minor] = 0;

  LOCK_DEVICE(&dev_lock[minor]);

#if defined(_LINUX_2_6_) || defined(_LINUX_3_0_)
  dev = minor_table[minor];
  if(dev == NULL)
    return;
  usb_set_intfdata(interface, NULL);
#endif
#ifdef CONFIG_DEVFS_FS
  /* remove our devfs node */
  devfs_unregister(dev->devfs);
#endif

  //wait the int_thread
  down(&dev->startstopth_sem);

  if (dev->open_count > 0)
    dev->udev = NULL;
#ifdef _LINUX_2_4_
  if(keventd_pid < 0)
    keventd_pid = _KEVENTD_PID;

  kill_proc(keventd_pid, SIGCHLD, 1);
#endif

  /* if the device is not opened, then we clean up right now */
  if (dev->open_count <= 0)
  {
    tmkdone_usb(minor);
    tmk1553busb_delete (dev);
  }
  UNLOCK_DEVICE(&dev_lock[minor]);

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: USB device #%d now disconnected\n", minor);
#endif
}

/*
 * tmk1553busb_init
 */
static int __init tmk1553busb_init(void)
{
  int result;
  int tmkNumber;

  /* initialize semaphores */
  for(result = 0; result < MAX_DEVICES; ++result)
  {
#ifdef SPIN_LOCK_BLOCKING
    spin_lock_init(&dev_lock[result]);
#else
    sema_init(&dev_lock[result], 1);
#endif
  }

  for(tmkNumber = 0; tmkNumber < MAX_DEVICES; ++tmkNumber)
  {
    iEvDataBegin_usb[tmkNumber] = iEvDataEnd_usb[tmkNumber] = cEvData_usb[tmkNumber] = 0;
#ifdef SPIN_LOCK_IRQ_BLOCKING
    spin_lock_init(&intLock[tmkNumber]);
#else
    sema_init(&intLock[tmkNumber], 1);
#endif
  }

#ifdef SPIN_LOCK_IRQ_BLOCKING
  spin_lock_init(&list_lock);
#else
  sema_init(&list_lock, 1);
#endif

  /* register this driver with the USB subsystem */
#ifdef _LINUX_2_4_
  tmk1553busb_driver.minor = usbminor;
#endif
  result = usb_register(&tmk1553busb_driver);
  if (result < 0)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: usb_register failed! Error# %d\n", result);
#endif
    return -1;
  }

  init_waitqueue_head(&wq);
  INIT_LIST_HEAD(&hlProc);
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: usb_register success\n");
#endif

//  info(DRIVER_DESC " " DRIVER_VERSION);
  return 0;
}

/*
 * tmk1553busb_exit
 */
static void __exit tmk1553busb_exit(void)
{
  /* deregister this driver with the USB subsystem */
  usb_deregister(&tmk1553busb_driver);
}

module_init (tmk1553busb_init);
module_exit (tmk1553busb_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

/* Functions */

u32 TMK_tmkgetmaxn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
//  lpwOut[0] = (USHORT)tmkgetmaxn();
//  return tmkError;
  return tmkgetmaxn_usb();
}

u32 TMK_tmkconfig(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
  unsigned short tmkType;
#ifdef TMK1553BUSB_DEBUG
  char chBuf[8];
  int ret;
#endif

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: tmkconfig \n");
#endif

  if (dev->curproc)
  {
    tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  else
  {
    switch(dev->device_type)
    {
      case TA1_USB_01_ID:
        tmkType = TA;
      break;
      default:
        return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
     break;
    }
    tmkconfig_usb(tmkNumber, tmkType, 0, 0, 0, 0);
    dev->curproc = current->pid;
    TMK_MT_Stop(dev, awIn, awOut, awBuf);
    tmkError_usb[tmkNumber] = 0;
    bcreset_usb(tmkNumber);
#ifdef TMK1553BUSB_DEBUG
    ret = usb_control_msg(dev->udev,
                          usb_rcvctrlpipe(dev->udev, 0),
                          0xb4,
                          USB_DIR_IN,
                          0,
                          0,
                          chBuf,
                          8,
                          5*HZ);
    printk(KERN_INFO "tmk1553busb: serial number: %c%c%c%c%c%c%c%c\n",
                      chBuf[0], chBuf[1], chBuf[2], chBuf[3],
                      chBuf[4], chBuf[5], chBuf[6], chBuf[7]);
#endif
    LOCK_IRQ(&intLock[tmkNumber]);
//    tmkEvents &= ~(1<<tmkNumber);
    dev->event = 0;
    iEvDataBegin_usb[tmkNumber] = iEvDataEnd_usb[tmkNumber] = cEvData_usb[tmkNumber] = 0;
    UNLOCK_IRQ(&intLock[tmkNumber]);
  }
  return tmkError_usb[tmkNumber];
}
u32 TMK_tmkdone(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "tmk1553busb: tmkdone\n");
#endif

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  bcreset_usb(tmkNumber);

  dev->curproc = 0;
  return tmkError_usb[tmkNumber] = tmkdone_usb(tmkNumber);
}
u32 TMK_tmkselect(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkselect\n");
//  #endif
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_tmkselected(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
//  #ifdef MY_DBG
//  printk(MY_KERN_DEBUG "Tmk1553b: tmkselected\n");
//  #endif

  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_tmkgetmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  return tmkgetmode_usb(tmkNumber);
}
u32 TMK_tmksetcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_tmkclrcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_tmkgetcwbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_tmkwaitevents(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int hUserEvents;
  int fWait;
  int tmkMyEvents = 0;
  long timeout;
  wait_queue_t __wait;
  int fSignal = 0;
  int tmkNumber = (int)dev->minor;
  int i;
  TListProc *hlnProc;
  pid_t hProc;

  hUserEvents = (int)(*((u32 *)(awIn)));
  for(i = 0; i < MAX_DEVICES; ++i)
  {
    if(!(hUserEvents & (1 << i)))
      continue;
    if(i == tmkNumber)
    {
//      if(dev->curproc != current->pid)
//        hUserEvents = hUserEvents & ~(1 << i);
//      else
        tmkMyEvents |= (dev->event) << i;
      continue;
    }//block & search
    LOCK_DEVICE(&dev_lock[i]);
    if(!minor_table[i] /*|| minor_table[i]->curproc != current->pid*/)
      hUserEvents = hUserEvents & ~(1 << i);
    else
      tmkMyEvents |= (minor_table[i]->event) << i;
    UNLOCK_DEVICE(&dev_lock[i]);
  }

  fWait = *((int*)(awIn+2));

  if (hUserEvents == 0)
    return (tmkError_usb[tmkNumber] = TMK_BAD_NUMBER);
//  for(i = 0; i < MAX_DEVICES; ++i)
//    if(hUserEvents & (1 << i))
//      tmkMyEvents = hUserEvents & ((dev->event) << i);
  if (fWait == 0 || tmkMyEvents)
  {
    return tmkMyEvents;
  }
  if (fWait > 0)
  {
    timeout = fWait * HZ / 1000;
    if (timeout == 0)
      timeout = 1;
  }
  else
    timeout = 0;

  hProc = current->pid;

  init_waitqueue_entry(&__wait, current);
  set_current_state(TASK_INTERRUPTIBLE);
  add_wait_queue(&(wq), &__wait);
  for (;;)
  {
    for(i = 0; i < MAX_DEVICES; ++i)//get event info
    {
      if(!(hUserEvents & (1 << i)))
        continue;
      if(i == tmkNumber)
      {
//        if(dev->curproc != current->pid)
//          hUserEvents = hUserEvents & ~(1 << i);
//        else
          tmkMyEvents |= (dev->event) << i;
        continue;
      }//block & search
      LOCK_DEVICE(&dev_lock[i]);
      if(!minor_table[i] /*|| minor_table[i]->curproc != current->pid*/)
        hUserEvents = hUserEvents & ~(1 << i);
      else
        tmkMyEvents |= (minor_table[i]->event) << i;
      UNLOCK_DEVICE(&dev_lock[i]);
    }
//    for(i = 0; i < MAX_DEVICES; ++i)
//      if(hUserEvents & (1 << i))
//        tmkMyEvents = hUserEvents & ((dev->event) << i);
    if (tmkMyEvents)
      break;

    LOCK_LIST();
    for (hlnProc = (TListProc*)(hlProc.next);
         hlnProc != (TListProc*)(&hlProc);
         hlnProc = (TListProc*)(hlnProc->ProcListEntry.next)
        )
    {
      if (hlnProc->waitFlag != hProc)
        continue;
      hlnProc->waitFlag = 0;
      UNLOCK_LIST();
      goto break_wait;
    }
    UNLOCK_LIST();
    if (fWait > 0 && timeout == 0)
      break;
    set_current_state(TASK_INTERRUPTIBLE);
    UNLOCK_DEVICE(&dev_lock[tmkNumber]);
    if (fWait > 0)
      timeout = schedule_timeout(timeout);
    else
      schedule();
    LOCK_DEVICE(&dev_lock[tmkNumber]);
    if ((fSignal = signal_pending(current)) != 0)
    {
#ifdef TMK1553BUSB_DEBUG
//      printk(KERN_INFO "tmk1553busb: break on signal %X, events=%X, myevents=%X\n", signal_pending(current), tmkEvents, (hUserEvents & tmkEvents));
#endif
      break;
    }
  }
break_wait:
  set_current_state(TASK_RUNNING);
  remove_wait_queue(&(wq), &__wait);

  tmkError_usb[tmkNumber] = 0;

//  for(i = 0; i < MAX_DEVICES; ++i)
//    if(hUserEvents & (1 << i))
//      tmkMyEvents = hUserEvents & ((dev->event) << i);
  if (fSignal)
    return -ERESTARTSYS;
  return tmkMyEvents;
}
u32 TMK_tmkwaiteventsm(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  return -EINVAL;
}
u32 TMK_tmkgetevd(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  pTListEvD pEvD;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  LOCK_IRQ(&intLock[tmkNumber]);

  while (cEvData_usb[tmkNumber])
  {
    pEvD = &(aEvData_usb[tmkNumber][iEvDataBegin_usb[tmkNumber]]);

    if (pEvD->nInt)
    {
//      if (pEvD->awEvData[2])
//        DpcIExcBC(tmkNumber, pEvD);
      memcpy(awOut, pEvD, 12);
      break;
    }

    iEvDataBegin_usb[tmkNumber] = (iEvDataBegin_usb[tmkNumber] + 1) & (EVENTS_SIZE-1);
    --cEvData_usb[tmkNumber];
  }

  if (cEvData_usb[tmkNumber])
  {
    iEvDataBegin_usb[tmkNumber] = (iEvDataBegin_usb[tmkNumber] + 1) & (EVENTS_SIZE-1);
    --cEvData_usb[tmkNumber];
  }
  else
  {
    memset(
        awOut,
        0,
        12
        );
  }

  if (cEvData_usb[tmkNumber] == 0)
  {
//    tmkEvents &= ~(1<<tmkNumber);
    dev->event = 0;
  }

  UNLOCK_IRQ(&intLock[tmkNumber]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_tmkgetinfo(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  memcpy(awBuf, &(dev->tmkConfigData_usb), sizeof(TTmkConfigData));
  return tmkError_usb[tmkNumber];
}
u32 TMK_getversion(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "Tmk1553b: TMK_GetVersion\n");
#endif

  return ((TMKUSB_VER_HI<<8)|TMKUSB_VER_LO);
}
u32 TMK_bcreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  if (tmkgetmode_usb(tmkNumber) == MRT_MODE)
    return (tmkError_usb[tmkNumber] = TMK_BAD_FUNC);
  bcreset_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bc_def_tldw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber];
}
u32 TMK_bc_enable_di(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber];
}
u32 TMK_bc_disable_di(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcdefirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  bcdefirqmode_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcgetirqmode_usb(tmkNumber);
}
u32 TMK_bcgetmaxbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
#ifdef TMK1553BUSB_DEBUG
  printk(KERN_INFO "Tmk1553b: TMK_bcgetmaxbase\n");
#endif

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcgetmaxbase_usb(tmkNumber);
}
u32 TMK_bcdefbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcdefbase_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetbase(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcgetbase_usb(tmkNumber);
}
u32 TMK_bcputw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcputw_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcgetw_usb(tmkNumber, awIn[0]);
}
u32 TMK_bcgetansw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = bcgetansw_usb(tmkNumber, awIn[0]);
  awOut[0] = (u16)(LOWORD(res));
  awOut[1] = (u16)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcputblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcputblk_usb(tmkNumber, awIn[0], awBuf, awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcgetblk_usb(tmkNumber, awIn[0], awBuf, awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcdefbus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcdefbus_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetbus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcgetbus_usb(tmkNumber);
}
u32 TMK_bcstart(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
#ifdef ASYNCHRONOUS_IO
  cAsync[tmkNumber] = 0;
#endif
  bcstart_usb(tmkNumber, awIn[0], awIn[1]);
#ifdef ASYNCHRONOUS_IO
  cAsync[tmkNumber] = 1;
#endif
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcstartx(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
#ifdef ASYNCHRONOUS_IO
  cAsync[tmkNumber] = 0;
#endif
  bcstartx_usb(tmkNumber, awIn[0], awIn[1]);
#ifdef ASYNCHRONOUS_IO
  cAsync[tmkNumber] = 1;
#endif
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcdeflink(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  bcdeflink_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcgetlink(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = bcgetlink_usb(tmkNumber);
  awOut[0] = (u16)(LOWORD(res));
  awOut[1] = (u16)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}
u32 TMK_bcstop(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return bcstop_usb(tmkNumber);
}
u32 TMK_bcgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = bcgetstate_usb(tmkNumber);
  awOut[0] = (u32)(LOWORD(res));
  awOut[1] = (u32)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtreset_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtdefirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefirqmode_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetirqmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetirqmode_usb(tmkNumber);
}
u32 TMK_rtdefmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefmode_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetmode(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetmode_usb(tmkNumber);
}
u32 TMK_rtgetmaxpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetmaxpage_usb(tmkNumber);
}
u32 TMK_rtdefpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefpage_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetpage_usb(tmkNumber);
}
u32 TMK_rtdefpagepc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefpagepc_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtdefpagebus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefpagebus_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetpagepc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetpagepc_usb(tmkNumber);
}
u32 TMK_rtgetpagebus(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetpagebus_usb(tmkNumber);
}
u32 TMK_rtdefaddress(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefaddress_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetaddress(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetaddress_usb(tmkNumber);
}
u32 TMK_rtdefsubaddr(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtdefsubaddr_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetsubaddr(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetsubaddr_usb(tmkNumber);
}
u32 TMK_rtputw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtputw_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetw_usb(tmkNumber, awIn[0]);
}
u32 TMK_rtputblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtputblk_usb(tmkNumber, awIn[0], awBuf, awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetblk(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtgetblk_usb(tmkNumber, awIn[0], awBuf, awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtsetanswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtsetanswbits_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtclranswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtclranswbits_usb(tmkNumber, awIn[0]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetanswbits(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetanswbits_usb(tmkNumber);
}
u32 TMK_rtgetflags(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtgetflags_usb(tmkNumber, awBuf, awIn[0], awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtputflags(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtputflags_usb(tmkNumber, awBuf, awIn[0], awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtsetflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtsetflag_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtclrflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtclrflag_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetflag_usb(tmkNumber, awIn[0], awIn[1]);
}
u32 TMK_rtgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetstate_usb(tmkNumber);
}
u32 TMK_rtbusy(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtbusy_usb(tmkNumber);
}
u32 TMK_rtlock(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtlock_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtunlock(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtunlock_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_rtgetcmddata(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtgetcmddata_usb(tmkNumber, awIn[0]);
}
u32 TMK_rtputcmddata(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  rtputcmddata_usb(tmkNumber, awIn[0], awIn[1]);
  return tmkError_usb[tmkNumber];
}
u32 TMK_mtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  mtreset_usb(tmkNumber);
  return tmkError_usb[tmkNumber];
}
u32 TMK_mtgetsw(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return mtgetsw_usb(tmkNumber);
}

u32 TMK_rtenable(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return rtenable_usb(tmkNumber, awIn[0]);
}

u32 TMK_mrtgetmaxn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_mrtconfig(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_mrtselected(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_mrtgetstate(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_mrtdefbrcsubaddr0(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}
u32 TMK_mrtreset(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_tmktimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmktimer_usb(tmkNumber, awIn[0]);
}

u32 TMK_tmkgettimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = tmkgettimer_usb(tmkNumber);
  awOut[0] = (u16)(LOWORD(res));
  awOut[1] = (u16)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}

u32 TMK_tmkgettimerl(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkgettimerl_usb(tmkNumber);
}

u32 TMK_bcgetmsgtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = bcgetmsgtime_usb(tmkNumber);
  awOut[0] = (u16)(LOWORD(res));
  awOut[1] = (u16)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}

u32 TMK_rtgetmsgtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  u32 res;
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  res = rtgetmsgtime_usb(tmkNumber);
  awOut[0] = (u16)(LOWORD(res));
  awOut[1] = (u16)(HIWORD(res));
  return tmkError_usb[tmkNumber];
}

u32 TMK_tmkgethwver(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkgethwver_usb(tmkNumber);
}

u32 TMK_tmkgetevtime(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_tmkswtimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_tmkgetswtimer(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_tmktimeout(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return tmktimeout_usb(tmkNumber, awIn[0]);
}

u32 TMK_MT_Start(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
  int Result;
  int MaxBase;
  int iBase;
  u32 dwBufSize = (u32)(*((u32 *)(awIn)));
  u8 buffer[1];
  u16 tmkTimerCtrlT;

#ifdef TMK1553BUSB_MT_DEBUG
  printk(KERN_INFO "Tmk1553b: TMK_MT_Start %d\n", dwBufSize);
//  return 0;
#endif

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  if(dev->intLoopMode == MT_LOOP_MODE ||
     dev->ep2_maxsize < 512 || dwBufSize < 256)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
  }

  tmkTimerCtrlT = tmktimer_usb(tmkNumber, GET_TIMER_CTRL);

  Result = mtreset_usb(tmkNumber);
  if(Result)
    return tmkError_usb[tmkNumber] = Result;

  tmktimer_usb(tmkNumber, tmkTimerCtrlT);

  MaxBase = mtgetmaxbase_usb(tmkNumber);

  for (iBase = 0; iBase <= MaxBase; ++iBase)
  {
    mtdefbase_usb(tmkNumber, iBase);

    if (iBase < MaxBase)
      mtdeflink_usb(tmkNumber, (iBase+1)&MaxBase, CX_CONT|CX_NOINT|CX_SIG);
    else
      mtdeflink_usb(tmkNumber, 0/*MaxBase*/, CX_CONT|CX_NOINT|CX_SIG);//CX_STOP|CX_NOINT|CX_NOSIG);
  }

  dev->intLoopMode = SLEEP_LOOP_MODE;

  Result = usb_control_msg(dev->udev,
                           usb_rcvctrlpipe(dev->udev, 0),
                           0xb6,
                           USB_DIR_IN,
                           0,
                           0,
                           NULL,
                           0,
                           5*HZ);
  dev->ResetEP6mt = 1;

  if(MonitorHwBuf[tmkNumber])
    kfree(MonitorHwBuf[tmkNumber]);
  MonitorHwBuf[tmkNumber] = (u8 *)kmalloc(dwBufSize * sizeof(u8), GFP_KERNEL); //alloc buffer space
  if(MonitorHwBuf[tmkNumber] == NULL)
  {
    dev->intLoopMode = MT_LOOP_MODE;
    return TMK_BAD_FUNC;//error buf malloc
  }

  LOCK_IRQ(&intLock[tmkNumber]);
  MonitorHwBufOF[tmkNumber] = FALSE;
  MonitorHwBufRead[tmkNumber] = 0;
  MonitorHwBufWrite[tmkNumber] = 0;
  MonitorHwBufSize[tmkNumber] = dwBufSize;
  MonitorHwBufCount[tmkNumber] = 0;
  dev->event = 0;
  if((tmktimer_usb(tmkNumber, GET_TIMER_CTRL) >> 10) & 0x01)
    MonitorHwTimer[tmkNumber] = 1;
  else
    MonitorHwTimer[tmkNumber] = 0;
  UNLOCK_IRQ(&intLock[tmkNumber]);

  buffer[0] = MonitorHwTimer[tmkNumber];

  Result = usb_control_msg(dev->udev,
                           usb_sndctrlpipe(dev->udev, 0),
                           0xba,
                           USB_DIR_OUT,
                           0,
                           0,
                           buffer,
                           1,
                           5*HZ);

  if(Result == FALSE)
  {
    kfree(MonitorHwBuf[tmkNumber]);
    dev->intLoopMode = MT_LOOP_MODE;//mb int_loop_mode
    return TMK_BAD_FUNC;
  }

  dev->intLoopMode = MT_LOOP_MODE;

  Result = mtstartx_usb(tmkNumber, 0, CX_CONT|CX_NOINT|CX_NOSIG);
  if(Result)
  {
    kfree(MonitorHwBuf[tmkNumber]);
    return tmkError_usb[tmkNumber] = Result;
  }

  return 0;
}

u32 TMK_MT_GetMessage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
  u32 dwMsCount = 0;
  char Running = TRUE;
  u16 OFFlag = 0;
  u32 TMonitorHwBufRead;
  u32 DataPtr = 0;
  u32 TMonitorHwBufSize;
  u16 TWord;
  u32 ReadCount;
  u32 dwBufSize = (u32)(*((u32 *)(awIn)));
  u8 FillFlag = (u8)(*((u32 *)(awIn + 2)));
  u16 zero[2] = {0, 0};

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  if(MonitorHwBuf[tmkNumber] == NULL)
  {
    (*((u32 *)(awOut))) = dwMsCount;//(u32)!!!
    return 0;
  }
  TMonitorHwBufSize = MonitorHwBufSize[tmkNumber] - 1;

  do
  {
//    dwWaitResult = WaitForSingleObject(MonitorHwMutex[i_Dev], 10000L);
    LOCK_IRQ(&intLock[tmkNumber]);
//    switch (dwWaitResult)
//    {
//      case WAIT_OBJECT_0:
//        printf("s cnt - %u ", MonitorHwBufCount[i_Dev]); //debug
    if(MonitorHwBufCount[tmkNumber] != 0)//Buf not empty
    {
      TMonitorHwBufRead = MonitorHwBufRead[tmkNumber];
      if(MonitorHwBuf[tmkNumber][TMonitorHwBufRead] > 0x03)//if Error
      {
        OFFlag = 1;
        MonitorHwBufCount[tmkNumber] = 0;
        MonitorHwBufRead[tmkNumber] = 0;
        MonitorHwBufWrite[tmkNumber] = 0;
        MonitorHwBufOF[tmkNumber] = FALSE;
        Running = FALSE;
//        if(MonitorHwEvents[tmkNumber])
//          ResetEvent(MonitorHwEvents[tmkNumber]);
        dev->event = 0;
      }
      else if((OFFlag = MonitorHwBuf[tmkNumber][TMonitorHwBufRead]) == 0)//reading
      {
        if(TMonitorHwBufRead == TMonitorHwBufSize)
          TMonitorHwBufRead = 0;
        else
          TMonitorHwBufRead++;

        TWord = MonitorHwBuf[tmkNumber][TMonitorHwBufRead];//read SW
        ReadCount = TWord >> 2;

        TWord = TWord << 8;

        if(TMonitorHwBufRead == TMonitorHwBufSize)
          TMonitorHwBufRead = 0;
        else
          TMonitorHwBufRead++;

        TWord |= MonitorHwBuf[tmkNumber][TMonitorHwBufRead] & 0xFF;

        if(TMonitorHwBufRead == TMonitorHwBufSize)
          TMonitorHwBufRead = 0;
        else
          TMonitorHwBufRead++;

        if((dwBufSize - DataPtr) > (ReadCount + 1))
        {
          MonitorHwBufCount[tmkNumber] -= 3;
//          Data[DataPtr] = TWord;
          __copy_to_user((u16 *)(*((unsigned long*)(awIn + 4))) + DataPtr,
                         &TWord,
                         2);
          DataPtr++;

          if(MonitorHwTimer[tmkNumber])
            ReadCount += 2;
          else
          {
//            Data[DataPtr] = 0;
            __copy_to_user((u16 *)(*((unsigned long *)(awIn + 4))) + DataPtr,
                           &zero,
                           4);
            DataPtr++;
//            Data[DataPtr] = 0;
            DataPtr++;
          }

          for(; ReadCount > 0; ReadCount--)
          {
            TWord = MonitorHwBuf[tmkNumber][TMonitorHwBufRead] << 8;
            if(TMonitorHwBufRead == TMonitorHwBufSize)
              TMonitorHwBufRead = 0;
            else
              TMonitorHwBufRead++;
            TWord |= MonitorHwBuf[tmkNumber][TMonitorHwBufRead] & 0xFF;
            if(TMonitorHwBufRead == TMonitorHwBufSize)
              TMonitorHwBufRead = 0;
            else
              TMonitorHwBufRead++;
//            awBuf[DataPtr] = TWord;
            __copy_to_user((u16 *)(*((unsigned long *)(awIn + 4))) + DataPtr,
                           &TWord,
                           2);
            DataPtr++;
            MonitorHwBufCount[tmkNumber] -= 2;
          }
          dwMsCount++;

          if(FillFlag)
            Running = FALSE;

          MonitorHwBufRead[tmkNumber] = TMonitorHwBufRead;
        }
        else
        {
          Running = FALSE;
        }//user buf full
      }//OF
      else
      {
        OFFlag = MonitorHwBuf[tmkNumber][TMonitorHwBufRead];
        MonitorHwBuf[tmkNumber][TMonitorHwBufRead] = 0;
        Running = FALSE;
      }
    }//buf not empty
    else
    {
//      if(MonitorHwEvents[i_Dev])
//        ResetEvent(MonitorHwEvents[i_Dev]);
      dev->event = 0;
      Running = FALSE;
    }

    UNLOCK_IRQ(&intLock[tmkNumber]);
//        ReleaseMutex(MonitorHwMutex[i_Dev]);
//      break;
//      default:
//        return TMK_BAD_FUNC;
//      break;
//    }

  }while(Running);

//  *dwMsWritten = dwMsCount;
  (*((u32 *)(awOut))) = dwMsCount;//(u32)!!!
  return OFFlag;
}

u32 TMK_MT_Stop(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;
  int ret;
  u8 Result = 0;

#ifdef TMK1553BUSB_MT_DEBUG
  printk(KERN_INFO "Tmk1553b: TMK_MT_Stop\n");
#endif

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }

  if(dev->intLoopMode == INT_LOOP_MODE ||
     dev->ep2_maxsize < 512)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
  }

  Result = bcreset_usb(tmkNumber);
  if(Result)
    return tmkError_usb[tmkNumber] = Result;

  dev->intLoopMode = SLEEP_LOOP_MODE;

  ret = usb_control_msg(dev->udev,
                        usb_rcvctrlpipe(dev->udev, 0),
                        0xbb,
                        USB_DIR_IN,
                        0,
                        0,
                        &Result,
                        1,
                        5*HZ);
  dev->ResetEP6int = 1;

  LOCK_IRQ(&intLock[tmkNumber]);
  if(MonitorHwBuf[tmkNumber])
  {
    kfree(MonitorHwBuf[tmkNumber]);
    MonitorHwBuf[tmkNumber] = NULL;
  }
  dev->event = 0;
  iEvDataBegin_usb[tmkNumber] = iEvDataEnd_usb[tmkNumber] = cEvData_usb[tmkNumber] = 0;
  UNLOCK_IRQ(&intLock[tmkNumber]);

  dev->intLoopMode = INT_LOOP_MODE;

  return Result;
}

u32 TMK_mrtdefbrcpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
//  mrtdefbrcpage_usb(awIn[0]);
//  return tmkError_usb[tmkNumber];
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_mrtgetbrcpage(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
//  lpwOut[0] = mrtgetbrcpage();
//  return tmkError;
//  return mrtgetbrcpage_usb();
  return tmkError_usb[tmkNumber] = TMK_BAD_FUNC;
}

u32 TMK_mbcinit(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  mbcinit_usb(awIn[0]);
  return tmkError_usb[tmkNumber];
}

u32 TMK_mbcpreparex(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  mbcpreparex_usb(tmkNumber, awIn[0], awIn[1], awIn[2], awIn[3]);
  return tmkError_usb[tmkNumber];
}

u32 TMK_mbcstartx(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  mbcstartx_usb(awIn[0]);
  return tmkError_usb[tmkNumber];
}

u32 TMK_mbcalloc(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
//  awOut[0] = mbcalloc();
//  return tmkError;
  return mbcalloc_usb();
}

u32 TMK_mbcfree(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  mbcfree_usb(awIn[0]);
  return tmkError_usb[tmkNumber];
}

u32 TMK_tmkwaiteventsflag(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int flag = *((int *)awIn + 1);
  TListProc *hlnProc;
  pid_t hProc = current->pid;
  pid_t hMainProc = *((int *)awIn);

  LOCK_LIST();
  for (hlnProc = (TListProc*)(hlProc.next);
       hlnProc != (TListProc*)(&hlProc);
       hlnProc = (TListProc*)(hlnProc->ProcListEntry.next)
      )
  {
    if (hlnProc->hProc != hProc)
      continue;
    if(flag)
    {
      hlnProc->waitFlag = hMainProc;
      wake_up_interruptible(&wq);
    }
    else
      hlnProc->waitFlag = 0;
    break;
  }
  UNLOCK_LIST();
  return 0;
}
u32 TMK_tmkreadsn(struct tmk1553busb * dev, u16 * awIn, u16 * awOut, u16 * awBuf)
{
  int tmkNumber = (int)dev->minor;

  if (dev->curproc != current->pid)
  {
    return tmkError_usb[tmkNumber] = TMK_BAD_NUMBER;
  }
  return dev->SerialNumber;
}
