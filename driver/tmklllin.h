
/****************************************************************************/
/*      TMKLLLIN.H v7.06 for Linux. ELCUS, 1995,2011                        */
/*      Interface to the driver TMKNLL v7.06 for Linux.                     */
/****************************************************************************/

#ifndef _TMKLLX_
#define _TMKLLX_

#define FARFN
#define FARIR
#define FARDT

#define NTMK 8
#define NRT 32
#define NMBCID 8

#define LINUX

//#define TMK_ERROR_0 0xE0040000

#define STATIC_TMKNUM
//#define DYNAMIC_TMKNUM


#define __EXTERN extern

#ifdef __cplusplus
extern "C" {
#endif

#include "tmknll.h"

#ifdef __cplusplus
}
#endif

#endif
