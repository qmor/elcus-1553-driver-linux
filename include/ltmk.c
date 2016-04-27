
/****************************************************************************/
/*      LTMK.C v4.06 for Linux. (c) ELCUS, 2002,2011.                       */
/*      Interface to driver tmk1553b v4.06 for Linux.                       */
/****************************************************************************/

#ifndef _TMK1553B_
#define _TMK1553B_

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include "ltmk.h"

#ifdef USE_TMK_ERROR
int tmkError;
#endif

//int _VTMK4tmkNumber;
//HANDLE _ahVTMK4Event[MAX_TMK_NUMBER+1];
HANDLE _hVTMK4VxD = 0;

/*****************Functions***************************/

int TmkOpen(void)
{
  int _VTMK4Arg;
  int ErrorCode;
//  int iTMK;
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  _hVTMK4VxD = open("/dev/tmk1553b", 0);
  if (_hVTMK4VxD < 0)
  {
    ErrorCode = _hVTMK4VxD; //GetLastError();
/*
    if (ErrorCode == ERROR_NOT_SUPPORTED)
    {
      MessageBox(NULL,"Unable to open VxD,\ndevice does not support DeviceIOCTL",szTitle,MB_OK|MB_ICONSTOP);
    }
    else
    {
      sprintf(szPrintf,"Unable to open VxD, Error code: %lx", ErrorCode);
      MessageBox(NULL,szPrintf,szTitle,MB_OK|MB_ICONSTOP);
    }
*/
    _hVTMK4VxD = 0;
    return ErrorCode;
  }
/*
  for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
    _ahVTMK4Event[iTMK] = 0;
*/
  if ((_VTMK4Arg = ioctl(_hVTMK4VxD, TMK_IOCGetVersion, 0)) < 0 ||
      _VTMK4Arg < TMK_VERSION_MIN)
  {
    close(_hVTMK4VxD);
    _hVTMK4VxD = 0;
    return VTMK_BAD_VERSION;
  }
  return 0;
}

void TmkClose(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  if (_hVTMK4VxD)
  {
    close(_hVTMK4VxD);
    _hVTMK4VxD = 0;
  }
}

int tmkgetmaxn(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetmaxn));
}

int tmkconfig(int tmkNumber)
{
//  _VTMK4tmkNumber = tmkNumber;
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkconfig, tmkNumber));
  
}

int tmkdone(int tmkNumber)
{
/*
  int iTMK;
  if (tmkNumber == ALL_TMKS)
  {
    for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
      _ahVTMK4Event[iTMK] = 0;
  }
  else if (tmkNumber >= 0 && tmkNumber <= MAX_TMK_NUMBER)
    _ahVTMK4Event[tmkNumber] = 0;
*/
  return (
#ifdef USE_TMK_ERROR
    tmkError = 
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkdone, tmkNumber));
}

int tmkselect(int tmkNumber)
{
//  _VTMK4tmkNumber = tmkNumber;
  return (
#ifdef USE_TMK_ERROR
    tmkError  = 
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCtmkselect, tmkNumber));
}

int tmkselected(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (
//  _VTMK4tmkNumber =
    ioctl(_hVTMK4VxD, TMK_IOCtmkselected));
}

TMK_DATA_RET tmkgetmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetmode));
}

void tmksetcwbits(TMK_DATA tmkSetControl)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmksetcwbits, tmkSetControl);
}

void tmkclrcwbits(TMK_DATA tmkClrControl)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmksetcwbits, tmkClrControl);
}

TMK_DATA_RET tmkgetcwbits(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetcwbits));
}

int tmkwaitevents(int maskEvents, int fWait)
{
  int _VTMK4Arg[2];
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  _VTMK4Arg[0] = maskEvents;
  _VTMK4Arg[1] = fWait;
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkwaitevents, &_VTMK4Arg));
}

/*
void tmkdefevent(HANDLE hEvent, BOOL fEventSet)
{
  WORD _awVTMK4InBuf[4], _awVTMK4OutBuf[1];
  int _VTMK4Arg;
  DWORD hVxDEvent;
  int iTMK;
  if (hEvent != 0)
  {
    for (iTMK = 0; iTMK <= MAX_TMK_NUMBER; ++iTMK)
    {
      if (hEvent == _ahVTMK4Event[iTMK])
        break;
    }
    if (iTMK > MAX_TMK_NUMBER)
    {
      hVxDEvent = (DWORD)hEvent;
      _ahVTMK4Event[_VTMK4tmkNumber] = hEvent;
    }
  }
  else
  {
    hVxDEvent = 0;
    _ahVTMK4Event[_VTMK4tmkNumber] = 0;
  }
  _awVTMK4InBuf[0] = LOWORD(hVxDEvent);
  _awVTMK4InBuf[1] = HIWORD(hVxDEvent);
  _awVTMK4InBuf[2] = LOWORD(fEventSet);
  _awVTMK4InBuf[3] = HIWORD(fEventSet);
  ioctl(_hVTMK4VxD, TMK_IOCtmkdefevent, _awVTMK4InBuf, 8, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
*/

void tmkgetevd(TTmkEventData *pEvD)
{
  WORD _awVTMK4OutBuf[6];
//  int _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmkgetevd, _awVTMK4OutBuf);
  pEvD->nInt = ((DWORD*)(_awVTMK4OutBuf))[0];
  switch (pEvD->wMode = _awVTMK4OutBuf[2])
  {
  case BC_MODE:
    switch (pEvD->nInt)
    {
    case 1:
      pEvD->bc.wResult = _awVTMK4OutBuf[3];
      break;
    case 2:
      pEvD->bc.wResult = _awVTMK4OutBuf[3];
      pEvD->bc.wAW1 = _awVTMK4OutBuf[4];
      pEvD->bc.wAW2 = _awVTMK4OutBuf[5];
      break;
    case 3:
      pEvD->bcx.wResultX = _awVTMK4OutBuf[3];
      pEvD->bcx.wBase = _awVTMK4OutBuf[4];
      break;
    case 4:
      pEvD->bcx.wBase = _awVTMK4OutBuf[3];
      break;
    }
    break;
  case MT_MODE:
    switch (pEvD->nInt)
    {
    case 3:
      pEvD->mt.wResultX = _awVTMK4OutBuf[3];
      pEvD->mt.wBase = _awVTMK4OutBuf[4];
      break;
    case 4:
      pEvD->mt.wBase = _awVTMK4OutBuf[3];
      break;
    }
    break;
  case RT_MODE:
    switch (pEvD->nInt)
    {
    case 1:
      pEvD->rt.wCmd = _awVTMK4OutBuf[3];
      break;
    case 2:
    case 3:
      pEvD->rt.wStatus = _awVTMK4OutBuf[3];
      break;
    }
    break;
  case MRT_MODE:
    pEvD->mrt.wStatus = _awVTMK4OutBuf[3];
    break;
  case UNDEFINED_MODE:
    pEvD->tmk.wRequest = _awVTMK4OutBuf[3];
    break;
  }
}
void tmkgetinfo(TTmkConfigData *pConfD)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmkgetinfo, pConfD);
}

/*
void bcdefintnorm(void (* UserNormBC)(TMK_DATA, TMK_DATA, TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCbcdefintnorm, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void bcdefintexc(void (* UserExcBC)(TMK_DATA, TMK_DATA, TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCbcdefintexc, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void bcdefintx(void (* UserXBC)(TMK_DATA, TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCbcdefintx, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void bcdefintsig(void (* UserSigBC)(TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCbcdefintsig, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
*/

int bcreset(void)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError = 
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcreset));
}

/*
void bc_def_tldw(TMK_DATA wTLDW)
{
  ioctl(_hVTMK4VxD, TMK_IOCbc_def_tldw, wTLDW);
}
void bc_enable_di(void)
{
  ioctl(_hVTMK4VxD, TMK_IOCbc_enable_di);
}
void bc_disable_di(void)
{
  ioctl(_hVTMK4VxD, TMK_IOCbc_disable_di);
}
*/

int bcdefirqmode(TMK_DATA bcIrqMode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcdefirqmode, bcIrqMode));
}

TMK_DATA_RET bcgetirqmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCbcgetirqmode));
}

TMK_DATA_RET bcgetmaxbase(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCbcgetmaxbase));
}

int bcdefbase(TMK_DATA bcBasePC)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcdefbase, bcBasePC));
}

TMK_DATA_RET bcgetbase(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCbcgetbase));
}

void bcputw(TMK_DATA bcAddr, TMK_DATA bcData)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcputw, bcAddr | (bcData << 16));
}

TMK_DATA_RET bcgetw(TMK_DATA bcAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return ioctl(_hVTMK4VxD, TMK_IOCbcgetw, bcAddr);
}

DWORD bcgetansw(TMK_DATA bcCtrlCode)
{
  DWORD _VTMK4Arg;
  _VTMK4Arg = bcCtrlCode;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcgetansw, &_VTMK4Arg);
  return _VTMK4Arg;
}

void bcputblk(TMK_DATA bcAddr, void *pcBuffer, TMK_DATA cwLength)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = bcAddr;
  _awVTMK4InBuf[1] = cwLength;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)bcAddr | ((DWORD)cwLength << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcputblk, &_VTMK4Arg);
}

void bcgetblk(TMK_DATA bcAddr, void *pcBuffer, TMK_DATA cwLength)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = bcAddr;
  _awVTMK4InBuf[1] = cwLength;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)bcAddr | ((DWORD)cwLength << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcgetblk, _VTMK4Arg);
}

int bcdefbus(TMK_DATA bcBus)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcdefbus, bcBus));
}

TMK_DATA_RET bcgetbus(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCbcgetbus));
}

int bcstart(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcstart, bcBase | (bcCtrlCode << 16)));
}

int bcstartx(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcstartx, bcBase | (bcCtrlCode << 16)));
}

int bcdeflink(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCbcdeflink, bcBase | (bcCtrlCode << 16)));
}

DWORD bcgetlink(void)  // ???????????????????????????????????????
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcgetlink, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET bcstop(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCbcstop));
}

DWORD bcgetstate(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcgetstate, &_VTMK4Arg);
  return _VTMK4Arg;
}

/*
void rtdefintcmd(void (* UserCmdRT)(TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCrtdefintcmd, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void rtdefinterr(void (* UserErrRT)(TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCrtdefinterr, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void rtdefintdata(void (* UserDataRT)(TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCrtdefintdata, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
*/

int rtreset(void)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError = 
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtreset));
}

int rtdefirqmode(TMK_DATA rtIrqMode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError = 
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefirqmode, rtIrqMode));
}

TMK_DATA_RET rtgetirqmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetirqmode));
}

int rtdefmode(TMK_DATA rtMode)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefmode, rtMode));
}

TMK_DATA_RET rtgetmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetmode));
}

TMK_DATA_RET rtgetmaxpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetmaxpage));
}

int rtdefpage(TMK_DATA rtPage)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefpage, rtPage));
}

TMK_DATA_RET rtgetpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpage));
}

int rtdefpagepc(TMK_DATA rtPagePC)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefpagepc, rtPagePC));
}

int rtdefpagebus(TMK_DATA rtPageBus)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefpagebus, rtPageBus));
}

TMK_DATA_RET rtgetpagepc(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpagepc));
}

TMK_DATA_RET rtgetpagebus(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpagebus));
}

int rtdefaddress(TMK_DATA rtAddress)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCrtdefaddress, rtAddress));
}

TMK_DATA_RET rtgetaddress(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetaddress));
}

void rtdefsubaddr(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtdefsubaddr, rtDir | (rtSubAddr << 16));
}

TMK_DATA_RET rtgetsubaddr(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetsubaddr));
}

void rtputw(TMK_DATA rtAddr, TMK_DATA rtData)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtputw, rtAddr | (rtData << 16));
}

TMK_DATA_RET rtgetw(TMK_DATA rtAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetw, rtAddr));
}

void rtputblk(TMK_DATA rtAddr, void *pcBuffer, TMK_DATA cwLength)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = rtAddr;
  _awVTMK4InBuf[1] = cwLength;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)rtAddr | ((DWORD)cwLength << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtputblk, &_VTMK4Arg);
}

void rtgetblk(TMK_DATA rtAddr, void *pcBuffer, TMK_DATA cwLength)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = rtAddr;
  _awVTMK4InBuf[1] = cwLength;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)rtAddr | ((DWORD)cwLength << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtgetblk, &_VTMK4Arg);
}

void rtsetanswbits(TMK_DATA rtSetControl)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtsetanswbits, rtSetControl);
}

void rtclranswbits(TMK_DATA rtClrControl)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtclranswbits, rtClrControl);
}

TMK_DATA_RET rtgetanswbits(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetanswbits));
}

void rtgetflags(void *pcBuffer, TMK_DATA rtDir, TMK_DATA rtFlagMin, TMK_DATA rtFlagMax)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = rtDir | rtFlagMin;
  _awVTMK4InBuf[1] = rtDir | rtFlagMax;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)(rtDir | rtFlagMin) | ((DWORD)(rtDir | rtFlagMax) << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtgetflags, &_VTMK4Arg);
}

void rtputflags(void *pcBuffer, TMK_DATA rtDir, TMK_DATA rtFlagMin, TMK_DATA rtFlagMax)
{
  ULONG _VTMK4Arg[2];
/*
  _awVTMK4InBuf[0] = rtDir | rtFlagMin;
  _awVTMK4InBuf[1] = rtDir | rtFlagMax;
  _awVTMK4InBuf[2] = (WORD)(LOWORD(pcBuffer));
  _awVTMK4InBuf[3] = (WORD)(HIWORD(pcBuffer));
*/
  *((DWORD*)_VTMK4Arg) = (DWORD)(rtDir | rtFlagMin) | ((DWORD)(rtDir | rtFlagMax) << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtputflags, &_VTMK4Arg);
}

void rtsetflag(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtsetflag);
}

void rtclrflag(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtclrflag);
}

TMK_DATA_RET rtgetflag(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetflag, rtDir | (rtSubAddr << 16)));
}

TMK_DATA_RET rtgetstate(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetstate));
}

TMK_DATA_RET rtbusy(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtbusy));
}

void rtlock(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtlock, rtDir | (rtSubAddr << 16));
}

void rtunlock(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtunlock);
}

TMK_DATA_RET rtgetcmddata(TMK_DATA rtBusCommand)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCrtgetcmddata, rtBusCommand));
}

void rtputcmddata(TMK_DATA rtBusCommand, TMK_DATA rtData)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtputcmddata, rtBusCommand | (rtData << 16));
}

/*
void mtdefintx(void (* UserIntXMT)(TMK_DATA, TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCmtdefintx, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
void mtdefintsig(void (* UserSigMT)(TMK_DATA))
{
  ;
//  ioctl(_hVTMK4VxD, TMK_IOCmtdefintsig, _awVTMK4InBuf, 4, _awVTMK4OutBuf, 0, &_VTMK4Arg, NULL);
}
*/

int mtreset(void)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCmtreset));
}

TMK_DATA_RET mtgetsw(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif  
  return (ioctl(_hVTMK4VxD, TMK_IOCmtgetsw));
}

TMK_DATA_RET rtenable(TMK_DATA rtEnable)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCrtenable, rtEnable));
}

#ifdef _TMK1553B_MRT
int mrtgetmaxn(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetmaxn));
}

DWORD mrtconfig(int mrtNumber)
{
  DWORD dwres;
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  dwres = ioctl(_hVTMK4VxD, TMK_IOCmrtconfig, mrtNumber);
//  if (dwres)
//    _VTMK4tmkNumber = dwres & 0xFFFF;
  if ((int)dwres < 0)
    dwres = 0;
#ifdef USE_TMK_ERROR
  if (!dwres)
    tmkError = TMK_BAD_NUMBER;
#endif
  return (dwres);
}

int mrtselected(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCmrtselected));
}

TMK_DATA_RET mrtgetstate(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetstate));
}

void mrtdefbrcsubaddr0(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 
#endif
  ioctl(_hVTMK4VxD, TMK_IOCmrtdefbrcsubaddr0);
}

int mrtreset(void)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif  
    ioctl(_hVTMK4VxD, TMK_IOCmrtreset));
}
#endif //def _TMK1553B_MRT

TMK_DATA_RET tmktimer(TMK_DATA tmkTimerCtrl)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCtmktimer, tmkTimerCtrl));
}

DWORD tmkgettimer(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmkgettimer, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmkgettimerl(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkgettimerl));
}

DWORD bcgetmsgtime(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCbcgetmsgtime, &_VTMK4Arg);
  return _VTMK4Arg;
}

DWORD rtgetmsgtime(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCrtgetmsgtime, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmkgethwver(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkgethwver));
}

DWORD tmkgetevtime(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmkgetevtime, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmkswtimer(TMK_DATA tmkSwTimerCtrl)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCtmkswtimer, tmkSwTimerCtrl));
}

DWORD tmkgetswtimer(void)
{
  DWORD _VTMK4Arg;
#ifdef USE_TMK_ERROR
  tmkError = 
#endif  
  ioctl(_hVTMK4VxD, TMK_IOCtmkgetswtimer, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmktimeout(TMK_DATA tmkTimeOut)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCtmktimeout, tmkTimeOut));
}

#ifdef _TMK1553B_MRT
int mrtdefbrcpage(TMK_DATA rtBrcPage)
{
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCmrtdefbrcpage, rtBrcPage));
}

TMK_DATA_RET mrtgetbrcpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetbrcpage));
}
#endif //def _TMK1553B_MRT

#endif
