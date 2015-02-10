/*
 * tmk1553b.h -- definitions for the tmk1553b char module. (c) ELCUS, 2002,2009.
 *
 * Part of this code comes from the book "Linux Device Drivers" 
 * by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.
 */

#ifndef _TMK1553B_H_
#define _TMK1553B_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/* version dependencies have been confined to a separate file */
//#include "sysdep.h"

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef TMK1553B_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "TMK1553B: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef TMK1553B_MAJOR
#define TMK1553B_MAJOR 0   /* dynamic major by default */
#endif

#ifndef TMK1553B_NR_DEVS
#define TMK1553B_NR_DEVS 8    /* S1553B0 through S1553B7 */
#endif

#define TMK_NR_PORTS 16
#define TA_NR_PORTS 32

//#define CONFIG_DEVFS_FS

#ifdef CONFIG_DEVFS_FS /* only if enabled, to avoid errors in 2.0 */
#include <linux/devfs_fs_kernel.h>
//#else
//  typedef void * devfs_handle_t;  /* avoid #ifdef inside the structure */
#endif

/*
 * This is somehow a hack: avoid ifdefs in the cleanup code by declaring
 * an empty procedure as a placeholder for devfs_unregister. This is
 * only done *unless* <linux/devfs_fs_kernel.h> was included, as that
 * header already implements placeholder for all the devfs functions
 */
/*............................................... degin-tag devfs-ifdef */
//#ifndef DEVFS_FL_DEFAULT
//extern inline void devfs_unregister(devfs_handle_t de) {}
//#endif

//extern devfs_handle_t tmk1553b_devfs_dir;

typedef struct tmk1553b_Dev {
   void **data;
   struct tmk1553b_Dev *next;   /* next listitem */
//   devfs_handle_t handle;    /* only used if devfs is there */
   struct semaphore sem;     /* mutual exclusion semaphore     */
} tmk1553b_Dev;

extern struct file_operations tmk1553b_fops;

/*
 * The different configurable parameters
 */
//extern int tmk1553b_major;     /* main.c */
//extern int tmk1553b_nr_devs;

/*
 * Prototypes for shared functions
 */

//int     tmk1553b_access_init(void);
//void    tmk1553b_access_cleanup(void);

//int     tmk1553b_trim(tmk1553b_Dev *dev);

//int     tmk1553b_ioctl (struct inode *inode, struct file *filp,
//                     unsigned int cmd, unsigned long arg);

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

#define VTMK_MT_Start 112
#define VTMK_MT_GetMessage 113
#define VTMK_MT_Stop 114
#define VTMK_tmkwaiteventsflag 115

#define TMK_IOC_MAXNR 115

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

#define TMK_IOCmrtdefbrcpage _IO(TMK_IOC_MAGIC, VTMK_mrtdefbrcpage+TMK_IOC0)
#define TMK_IOCmrtgetbrcpage _IO(TMK_IOC_MAGIC, VTMK_mrtgetbrcpage+TMK_IOC0)

#define TMK_IOCMT_Start _IO(TMK_IOC_MAGIC, VTMK_MT_Start+TMK_IOC0)
#define TMK_IOCMT_GetMessage _IOR(TMK_IOC_MAGIC, VTMK_MT_GetMessage+TMK_IOC0, ULONG[3])
#define TMK_IOCMT_Stop _IO(TMK_IOC_MAGIC, VTMK_MT_Stop+TMK_IOC0)

#define TMK_IOCtmkwaiteventsflag _IOW(TMK_IOC_MAGIC, VTMK_tmkwaiteventsflag+TMK_IOC0, __u64)

#endif /* _TMK1553B_H_ */
