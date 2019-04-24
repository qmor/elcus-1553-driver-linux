#ifndef __TMK1553BUSB_STRUCT__
#define __TMK1553BUSB_STRUCT__
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#include <linux/usb.h>

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

struct tmk1553busb
{
  struct usb_device *    udev; /* save off the usb device pointer */
  struct usb_interface * interface; /* the interface for this device */
#ifdef CONFIG_DEVFS_FS
  devfs_handle_t         devfs; /* devfs device node */
#endif
  unsigned char          minor; /* the starting minor number for this device */
  unsigned char          device_type; /* type of the device */
/* TA1-USB-01 stuff */
  __u8                   ep2_address; /* the address of the bulk out endpoint */
  __u8                   ep4_address; /* the address of the bulk in endpoint */
  __u8                   ep6_address; /* the address of the bulk in endpoint */
  __u8                   ep8_address; /* the address of the bulk in endpoint */
  __u16                  ep2_maxsize; /* endpoint 2 max packet size */
  __u16                  ep4_maxsize; /* endpoint 2 max packet size */
  __u16                  ep6_maxsize; /* endpoint 2 max packet size */
  __u16                  ep8_maxsize; /* endpoint 2 max packet size */

  int                    curproc; /* current process for this device */
  int                    open_count; /* number of times this port has been opened */
  struct semaphore       startstopth_sem;
  u8                     fwver[2];
  TTmkConfigData         tmkConfigData_usb;
  char                   intLoopMode;
  char                   ResetEP6mt;
  char                   ResetEP6int;
  volatile char          event;
  int                    SerialNumber;
};
#endif
