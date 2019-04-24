
/****************************************************************************/
/*      TMKLLLIN v6.02 for Linux. ELCUS, 1995,2006                          */
/*      Linux wrapper for the driver TMKNLL v6.02.                          */
/****************************************************************************/

#include "config.h"
#include "devstruct.h"
#include "tmklllin.h"
#include "tmknllio.h"
#include <linux/slab.h>
//#define RAMwoCLI
//#define MASKTMKS

//#define DRV_MAX_BASE 255
//#define DRV_MAX_BASE 511
#define DRV_MAX_BASE 1023

#define NOT_INCLUDE_DEFS
extern struct tmk1553busb * minor_table[MAX_DEVICES];
#include "tmknllusb.c"
