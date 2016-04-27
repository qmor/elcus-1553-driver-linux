/****************************************************************************/
/*      LTMK v4.06 for Linux. (c) ELCUS, 2002,2011.                         */
/*      Interface to driver tmk1553b      v4.06 for Linux.                  */
/*      Interface to driver tmk1553busb   v1.08 for Linux.                  */
/****************************************************************************/

#ifndef _TMK1553B_
#define _TMK1553B_

#include <unistd.h>
#include <sched.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <linux/kd.h>


#include "ltmk.h"

#ifdef USE_TMK_ERROR
int tmkError;
#endif

int tmkUsbCnt = 0;
int tmkCnt = 0;
int tmkCurNumber = -1;
char tmkUsbNumMap[MAX_TMKUSB_NUMBER+1];
HANDLE _ahVTMK4VxDusb[MAX_TMKUSB_NUMBER+1];
HANDLE _hVTMK4VxD = 0;

/*****************Functions***************************/

int TmkOpen(void)
{
  int _VTMK4Arg;
  int ErrorCode;
  int iTMK;
  char devName[32];
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif

  tmkCnt = 0;
  _hVTMK4VxD = open("/dev/tmk1553b", 0);
  if (_hVTMK4VxD < 0)
  {
    ErrorCode = _hVTMK4VxD;
    _hVTMK4VxD = 0;
  }
  else if ((_VTMK4Arg = ioctl(_hVTMK4VxD, TMK_IOCGetVersion, 0)) < 0 ||
      _VTMK4Arg < TMK_VERSION_MIN)
  {
    close(_hVTMK4VxD);
    _hVTMK4VxD = 0;
    return VTMK_BAD_VERSION;
  }
  else
    tmkCnt = ioctl(_hVTMK4VxD, TMK_IOCtmkgetmaxn) + 1;

  tmkUsbCnt = 0;
  for(iTMK = 0; iTMK <= MAX_TMKUSB_NUMBER; ++iTMK)
  {
    _ahVTMK4VxDusb[iTMK] = 0;
    tmkUsbNumMap[iTMK] = 0;
  }
  for(iTMK = 0; iTMK <= MAX_TMKUSB_NUMBER; ++iTMK)
  {
    sprintf(devName, "/dev/tmk1553busb%d", iTMK);
    _ahVTMK4VxDusb[tmkUsbCnt] = open(devName, 0);
    if(_ahVTMK4VxDusb[tmkUsbCnt] < 0)
    {
      _ahVTMK4VxDusb[tmkUsbCnt] = 0;
      continue;
    }
    if ((_VTMK4Arg = ioctl(_ahVTMK4VxDusb[tmkUsbCnt],
                           TMK_IOCGetVersion, 0)) < 0 ||
        _VTMK4Arg < TMKUSB_VERSION_MIN)
    {
      close(_ahVTMK4VxDusb[tmkUsbCnt]);
      _ahVTMK4VxDusb[tmkUsbCnt] = 0;
      return VTMK_BAD_VERSION;
    }
    else
    {
      close(_ahVTMK4VxDusb[tmkUsbCnt]);
      _ahVTMK4VxDusb[tmkUsbCnt] = 0;
    }
    tmkUsbNumMap[tmkUsbCnt] = iTMK;
    tmkUsbCnt++;
  }

  if (!_hVTMK4VxD && !tmkUsbCnt)
    return ErrorCode;

  return 0;
}

void TmkClose(void)
{
  int iTMK;
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  tmkCurNumber = -1;
  tmkUsbCnt = 0;
  tmkCnt = 0;
  for(iTMK = 0; iTMK <= MAX_TMKUSB_NUMBER; ++iTMK)
  {
    if(_ahVTMK4VxDusb[iTMK])
      close(_ahVTMK4VxDusb[iTMK]);
    _ahVTMK4VxDusb[iTMK] = 0;
    tmkUsbNumMap[iTMK] = 0;
  }
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
  if(_hVTMK4VxD != 0)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetmaxn) + tmkUsbCnt);
  else
    return tmkUsbCnt - 1;
}

int tmkconfig(int tmkNumber)
{
  char devName[32];
  int Result;

  if( tmkNumber < 0 || tmkNumber >= tmkCnt + tmkUsbCnt)
    return
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkNumber < tmkCnt)
  {
    if(!_hVTMK4VxD)
      return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
        TMK_BAD_NUMBER;
    tmkCurNumber = tmkNumber;
    return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCtmkconfig, tmkNumber));
  }

  sprintf(devName, "/dev/tmk1553busb%d", tmkUsbNumMap[tmkNumber - tmkCnt]);
  if(_ahVTMK4VxDusb[tmkNumber - tmkCnt] ||
     (_ahVTMK4VxDusb[tmkNumber - tmkCnt] = open(devName, 0)) < 0)
  {
    _ahVTMK4VxDusb[tmkNumber - tmkCnt] = 0;
    return TMK_BAD_NUMBER;
  }

  Result = ioctl(_ahVTMK4VxDusb[tmkNumber - tmkCnt], TMK_IOCtmkconfig, tmkNumber);
  if(!Result)
    tmkCurNumber = tmkNumber;
#ifdef USE_TMK_ERROR
  tmkError = Result;
#endif
  return Result;
}

int tmkdone(int tmkNumber)
{
  int iTMK, bTMK, eTMK;
  int Result;

#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkNumber < tmkCnt && tmkNumber != ALL_TMKS)
  {
    if(!_hVTMK4VxD)
      return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
        TMK_BAD_NUMBER;
    tmkCurNumber = -1;
    return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCtmkdone, tmkNumber));
  }
  if (tmkNumber == ALL_TMKS)
  {
    tmkCurNumber = -1;
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCtmkdone, tmkNumber);
    bTMK = 0;
    eTMK = tmkUsbCnt - 1;
  }
  else if(tmkNumber < 0 || tmkNumber >= tmkCnt + tmkUsbCnt)
  {
    return
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      TMK_BAD_NUMBER;
  }
  else
  {
    if(tmkNumber == tmkCurNumber)
      tmkCurNumber = -1;
    bTMK = eTMK = tmkNumber - tmkCnt;
  }
  for (iTMK = bTMK; iTMK <= eTMK; ++iTMK)
  {
    if(_ahVTMK4VxDusb[iTMK])
    {
      Result = ioctl(_ahVTMK4VxDusb[iTMK], TMK_IOCtmkdone, iTMK);
      close(_ahVTMK4VxDusb[iTMK]);
    }
    _ahVTMK4VxDusb[iTMK] = 0;
  }
#ifdef USE_TMK_ERROR
    tmkError = Result;
#endif
  return Result;
}

int tmkselect(int tmkNumber)
{
  if(tmkNumber < 0 || tmkNumber >= tmkCnt + tmkUsbCnt)
    return
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkNumber < tmkCnt)
  {
    if(!_hVTMK4VxD)
      return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
        TMK_BAD_NUMBER;
    tmkCurNumber = tmkNumber;
    return (
#ifdef USE_TMK_ERROR
    tmkError  =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCtmkselect, tmkNumber));
  }
  if(_ahVTMK4VxDusb[tmkNumber - tmkCnt])
    tmkCurNumber = tmkNumber;
  else
  {
    tmkCurNumber = -1;
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  }
  return 0;
}

int tmkselected(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < tmkCnt)
    return ioctl(_hVTMK4VxD, TMK_IOCtmkselected);
  else
    return tmkCurNumber;
}

TMK_DATA_RET tmkgetmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
    tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetmode));
  return ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgetmode);
}

void tmksetcwbits(TMK_DATA tmkSetControl)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmksetcwbits, tmkSetControl);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmksetcwbits, tmkSetControl);
}

void tmkclrcwbits(TMK_DATA tmkClrControl)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmksetcwbits, tmkClrControl);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmksetcwbits, tmkClrControl);
}

TMK_DATA_RET tmkgetcwbits(void)
{
#ifdef USE_TMK_ERROR
    tmkError = 0;
#endif
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkgetcwbits));
  return ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgetcwbits);
}

int waitTh(void * arg)
{
  int _VTMK4Arg[2];
  int tmkMask = 0;
  int DrvMask = 0;
  int iTMK;
  TTmkWaitData * WData = (TTmkWaitData *)arg;

  _VTMK4Arg[0] = 0;
  _VTMK4Arg[1] = WData->fWait;
  WData->Events = 0;
  for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
  {
    if((WData->maskEvents & (1<<(iTMK + tmkCnt))) && _ahVTMK4VxDusb[iTMK])
      _VTMK4Arg[0] |= 1<<tmkUsbNumMap[iTMK];
  }
  if(_VTMK4Arg[0])
  {
    for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
    {
      if(_ahVTMK4VxDusb[iTMK])
      {
        DrvMask = ioctl(_ahVTMK4VxDusb[iTMK], TMK_IOCtmkwaitevents, &_VTMK4Arg);
        break;
      }
    }
    if(DrvMask > 0)
    for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
    {
      if(DrvMask & (1<<tmkUsbNumMap[iTMK]))
        tmkMask |= 1<<(iTMK + tmkCnt);
    }
  }

  _VTMK4Arg[0] = WData->mainpid;
  _VTMK4Arg[1] = 1;
  ioctl(_hVTMK4VxD, TMK_IOCtmkwaiteventsflag, &_VTMK4Arg);// << tmkUsbCnt;

  WData->Events = tmkMask;
  _exit(0);
}

int tmkwaitevents(int maskEvents, int fWait)
{
  int _VTMK4Arg[2];
  int tmkMask = 0;
  int DrvMask = 0;
  int iTMK;
  TTmkWaitData WData;
  pid_t tpid = 0;
  int status = 0;
  char stack[2000];
  int DevMask;
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif

  if(tmkCnt > 31)
    DevMask = 0xFFFFFFFF;
  else
    DevMask = (1 << tmkCnt) - 1;
  if((maskEvents & DevMask) == 0)//usb only
  {
    _VTMK4Arg[0] = 0;
    _VTMK4Arg[1] = fWait;
    for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
    {
      if((maskEvents & (1<<(iTMK + tmkCnt))) && _ahVTMK4VxDusb[iTMK])
        _VTMK4Arg[0] |= 1<<tmkUsbNumMap[iTMK];
    }
    if(_VTMK4Arg[0])
    {
      for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
      {
        if(_ahVTMK4VxDusb[iTMK])
        {
          DrvMask = ioctl(_ahVTMK4VxDusb[iTMK], TMK_IOCtmkwaitevents, &_VTMK4Arg);
          break;
        }
      }
      if(DrvMask > 0)
        for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
        {
          if(DrvMask & (1<<tmkUsbNumMap[iTMK]))
            tmkMask |= 1<<(iTMK + tmkCnt);
        }
      else
        return DrvMask;
    }
    return tmkMask;
  }
  else if((maskEvents >> tmkCnt) == 0)//tmk only
  {
    _VTMK4Arg[0] = maskEvents;
    _VTMK4Arg[1] = fWait;
    if(_hVTMK4VxD && _VTMK4Arg[0])
      tmkMask = ioctl(_hVTMK4VxD, TMK_IOCtmkwaitevents, &_VTMK4Arg);
    return tmkMask;
  }
  else
  {
    _VTMK4Arg[0] = WData.maskEvents = maskEvents;
    _VTMK4Arg[1] = WData.fWait = fWait;
    WData.mainpid = getpid();

    tpid = clone(waitTh, stack + 1999, CLONE_VM|CLONE_FILES, &WData);
    if(tpid < 0)
      return -1;
    tmkMask = ioctl(_hVTMK4VxD, TMK_IOCtmkwaitevents, &_VTMK4Arg);
    if(tmkMask < 0)
      tmkMask = 0;
    for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
    {
      if(_ahVTMK4VxDusb[iTMK])
      {
        _VTMK4Arg[0] = tpid;
        _VTMK4Arg[1] = 1;
        ioctl(_ahVTMK4VxDusb[iTMK], TMK_IOCtmkwaiteventsflag, &_VTMK4Arg);
        break;
      }
    }

    waitpid(tpid, &status, __WCLONE);

    _VTMK4Arg[0] = WData.mainpid;
    _VTMK4Arg[1] = 0;
    ioctl(_hVTMK4VxD, TMK_IOCtmkwaiteventsflag, &_VTMK4Arg);
    for(iTMK = 0; iTMK < tmkUsbCnt; ++iTMK)
    {
      if(_ahVTMK4VxDusb[iTMK])
      {
        _VTMK4Arg[0] = 0;
        ioctl(_ahVTMK4VxDusb[iTMK], TMK_IOCtmkwaiteventsflag, &_VTMK4Arg);
        break;
      }
    }
    tmkMask |= WData.Events;
  }
  return tmkMask;
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

  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  else if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkgetevd, _awVTMK4OutBuf);
  }
  else
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgetevd, _awVTMK4OutBuf);
  }

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
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkgetinfo, pConfD);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgetinfo, pConfD);
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
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcreset));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcreset));
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
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcdefirqmode, bcIrqMode));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcdefirqmode, bcIrqMode));
}

TMK_DATA_RET bcgetirqmode(void)
{
#ifdef USE_TMK_ERROR
    tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCbcgetirqmode));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetirqmode));
}

TMK_DATA_RET bcgetmaxbase(void)
{
#ifdef USE_TMK_ERROR
    tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCbcgetmaxbase));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetmaxbase));
}

int bcdefbase(TMK_DATA bcBasePC)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcdefbase, bcBasePC));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcdefbase, bcBasePC));
}

TMK_DATA_RET bcgetbase(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCbcgetbase));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetbase));
}

void bcputw(TMK_DATA bcAddr, TMK_DATA bcData)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcputw, bcAddr | (bcData << 16));
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcputw, bcAddr | (bcData << 16));
}

TMK_DATA_RET bcgetw(TMK_DATA bcAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return ioctl(_hVTMK4VxD, TMK_IOCbcgetw, bcAddr);
  return ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetw, bcAddr);
}

DWORD bcgetansw(TMK_DATA bcCtrlCode)
{
  DWORD _VTMK4Arg;
  _VTMK4Arg = bcCtrlCode;

  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcgetansw, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetansw, &_VTMK4Arg);
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

  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcputblk, &_VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcputblk, &_VTMK4Arg);
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

  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcgetblk, _VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetblk, _VTMK4Arg);
}

int bcdefbus(TMK_DATA bcBus)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcdefbus, bcBus));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcdefbus, bcBus));
}

TMK_DATA_RET bcgetbus(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCbcgetbus));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetbus));
}

int bcstart(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcstart, bcBase | (bcCtrlCode << 16)));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcstart, bcBase | (bcCtrlCode << 16)));
}

int bcstartx(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcstartx, bcBase | (bcCtrlCode << 16)));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcstartx, bcBase | (bcCtrlCode << 16)));
}

int bcdeflink(TMK_DATA bcBase, TMK_DATA bcCtrlCode)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCbcdeflink, bcBase | (bcCtrlCode << 16)));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcdeflink, bcBase | (bcCtrlCode << 16)));
}

DWORD bcgetlink(void)  // ???????????????????????????????????????
{
  DWORD _VTMK4Arg;

  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcgetlink, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetlink, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET bcstop(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCbcstop));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcstop));
}

DWORD bcgetstate(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcgetstate, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetstate, &_VTMK4Arg);
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
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtreset));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtreset));
}

int rtdefirqmode(TMK_DATA rtIrqMode)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefirqmode, rtIrqMode));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefirqmode, rtIrqMode));
}

TMK_DATA_RET rtgetirqmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetirqmode));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetirqmode));
}

int rtdefmode(TMK_DATA rtMode)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefmode, rtMode));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefmode, rtMode));
}

TMK_DATA_RET rtgetmode(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetmode));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetmode));
}

TMK_DATA_RET rtgetmaxpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetmaxpage));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetmaxpage));
}

int rtdefpage(TMK_DATA rtPage)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefpage, rtPage));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefpage, rtPage));
}

TMK_DATA_RET rtgetpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpage));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetpage));
}

int rtdefpagepc(TMK_DATA rtPagePC)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefpagepc, rtPagePC));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefpagepc, rtPagePC));
}

int rtdefpagebus(TMK_DATA rtPageBus)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefpagebus, rtPageBus));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefpagebus, rtPageBus));
}

TMK_DATA_RET rtgetpagepc(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpagepc));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetpagepc));
}

TMK_DATA_RET rtgetpagebus(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetpagebus));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetpagebus));
}

int rtdefaddress(TMK_DATA rtAddress)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCrtdefaddress, rtAddress));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefaddress, rtAddress));
}

TMK_DATA_RET rtgetaddress(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetaddress));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetaddress));
}

void rtdefsubaddr(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtdefsubaddr, rtDir | (rtSubAddr << 16));
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtdefsubaddr, rtDir | (rtSubAddr << 16));
}

TMK_DATA_RET rtgetsubaddr(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetsubaddr));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetsubaddr));
}

void rtputw(TMK_DATA rtAddr, TMK_DATA rtData)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtputw, rtAddr | (rtData << 16));
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtputw, rtAddr | (rtData << 16));
}

TMK_DATA_RET rtgetw(TMK_DATA rtAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetw, rtAddr));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetw, rtAddr));
}

void rtputblk(TMK_DATA rtAddr, void *pcBuffer, TMK_DATA cwLength)
{
  ULONG _VTMK4Arg[2];
  *((DWORD*)_VTMK4Arg) = (DWORD)rtAddr | ((DWORD)cwLength << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtputblk, &_VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtputblk, &_VTMK4Arg);
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
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtgetblk, &_VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetblk, &_VTMK4Arg);
}

void rtsetanswbits(TMK_DATA rtSetControl)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtsetanswbits, rtSetControl);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtsetanswbits, rtSetControl);
}

void rtclranswbits(TMK_DATA rtClrControl)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtclranswbits, rtClrControl);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtclranswbits, rtClrControl);
}

TMK_DATA_RET rtgetanswbits(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetanswbits));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetanswbits));
}

void rtgetflags(void *pcBuffer, TMK_DATA rtDir, TMK_DATA rtFlagMin, TMK_DATA rtFlagMax)
{
  ULONG _VTMK4Arg[2];
  *((DWORD*)_VTMK4Arg) = (DWORD)(rtDir | rtFlagMin) | ((DWORD)(rtDir | rtFlagMax) << 16);
  _VTMK4Arg[1] = (ULONG)pcBuffer;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtgetflags, &_VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetflags, &_VTMK4Arg);
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
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtputflags, &_VTMK4Arg);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtputflags, &_VTMK4Arg);
}

void rtsetflag(void)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtsetflag);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtsetflag);
}

void rtclrflag(void)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtclrflag);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtclrflag);
}

TMK_DATA_RET rtgetflag(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetflag, rtDir | (rtSubAddr << 16)));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetflag, rtDir | (rtSubAddr << 16)));
}

TMK_DATA_RET rtgetstate(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetstate));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetstate));
}

TMK_DATA_RET rtbusy(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtbusy));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtbusy));
}

void rtlock(TMK_DATA rtDir, TMK_DATA rtSubAddr)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtlock, rtDir | (rtSubAddr << 16));
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtlock, rtDir | (rtSubAddr << 16));
}

void rtunlock(void)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtunlock);
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtunlock);
}

TMK_DATA_RET rtgetcmddata(TMK_DATA rtBusCommand)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtgetcmddata, rtBusCommand));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetcmddata, rtBusCommand));
}

void rtputcmddata(TMK_DATA rtBusCommand, TMK_DATA rtData)
{
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtputcmddata, rtBusCommand | (rtData << 16));
    return;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtputcmddata, rtBusCommand | (rtData << 16));
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
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCmtreset));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCmtreset));
}

TMK_DATA_RET mtgetsw(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCmtgetsw));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCmtgetsw));
}

TMK_DATA_RET rtenable(TMK_DATA rtEnable)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCrtenable, rtEnable));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtenable, rtEnable));
}

#ifdef _TMK1553B_MRT
int mrtgetmaxn(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(_hVTMK4VxD != 0)
    return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetmaxn) + tmkUsbCnt);
  else
    return tmkUsbCnt - 1;
}

DWORD mrtconfig(int mrtNumber)
{
  DWORD dwres = 0;
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(_hVTMK4VxD != 0 && mrtNumber < tmkCnt && mrtNumber >= 0)
    dwres = ioctl(_hVTMK4VxD, TMK_IOCmrtconfig, mrtNumber);
  if ((int)dwres < 0)
    dwres = 0;
  if (dwres)
    tmkCurNumber = mrtNumber;
#ifdef USE_TMK_ERROR
  else
    tmkError = TMK_BAD_NUMBER;
#endif
  return (dwres);
}

int mrtselected(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCmrtselected));
  else
    return tmkCurNumber;
}

TMK_DATA_RET mrtgetstate(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetstate));
  else
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return 0;
}

void mrtdefbrcsubaddr0(void)
{
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCmrtdefbrcsubaddr0);
    return;
  }
  else
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return;
}

int mrtreset(void)
{
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCmrtreset));
  return
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    TMK_BAD_NUMBER;
}
#endif //def _TMK1553B_MRT

TMK_DATA_RET tmktimer(TMK_DATA tmkTimerCtrl)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmktimer, tmkTimerCtrl));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmktimer, tmkTimerCtrl));
}

DWORD tmkgettimer(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkgettimer, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgettimer, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmkgettimerl(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkgettimerl));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgettimerl));
}

DWORD bcgetmsgtime(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCbcgetmsgtime, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCbcgetmsgtime, &_VTMK4Arg);
  return _VTMK4Arg;
}

DWORD rtgetmsgtime(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCrtgetmsgtime, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError =
#endif
  ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCrtgetmsgtime, &_VTMK4Arg);
  return _VTMK4Arg;
}

TMK_DATA_RET tmkgethwver(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkgethwver));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmkgethwver));
}

DWORD tmkgetevtime(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkgetevtime, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return 0;
}

TMK_DATA_RET tmkswtimer(TMK_DATA tmkSwTimerCtrl)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmkswtimer, tmkSwTimerCtrl));
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return 0;
}

DWORD tmkgetswtimer(void)
{
  DWORD _VTMK4Arg;
  if(tmkCurNumber < tmkCnt)
  {
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    ioctl(_hVTMK4VxD, TMK_IOCtmkgetswtimer, &_VTMK4Arg);
    return _VTMK4Arg;
  }
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return 0;
}

TMK_DATA_RET tmktimeout(TMK_DATA tmkTimeOut)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCtmktimeout, tmkTimeOut));
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCtmktimeout, tmkTimeOut));
}

#ifdef _TMK1553B_MRT
int mrtdefbrcpage(TMK_DATA rtBrcPage)
{
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return (
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      ioctl(_hVTMK4VxD, TMK_IOCmrtdefbrcpage, rtBrcPage));
  return (
#ifdef USE_TMK_ERROR
    tmkError =
#endif
    TMK_BAD_NUMBER);
}

TMK_DATA_RET mrtgetbrcpage(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
  {
#ifdef USE_TMK_ERROR
    tmkError = TMK_BAD_NUMBER;
#endif
    return 0;
  }
  if(tmkCurNumber < tmkCnt)
    return (ioctl(_hVTMK4VxD, TMK_IOCmrtgetbrcpage));
#ifdef USE_TMK_ERROR
  tmkError = TMK_BAD_NUMBER;
#endif
  return 0;
}
#endif //def _TMK1553B_MRT

int MT_Start(DWORD dwBufSize)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_FUNC;
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCMT_Start, dwBufSize));
}

int MT_GetMessage(WORD * Data, DWORD dwBufSize, BOOL FillFlag, DWORD * dwMsWritten)
{
  ULONG _VTMK4Arg[3];
  int Result;

#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_FUNC;
  _VTMK4Arg[0] = dwBufSize;
  _VTMK4Arg[1] = FillFlag;
  _VTMK4Arg[2] = (ULONG)Data;
  Result = ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCMT_GetMessage, &_VTMK4Arg);
  *dwMsWritten = _VTMK4Arg[0];
  return Result;
}

int MT_Stop(void)
{
#ifdef USE_TMK_ERROR
  tmkError = 0;
#endif
  if(tmkCurNumber < 0)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_NUMBER;
  if(tmkCurNumber < tmkCnt)
    return
#ifdef USE_TMK_ERROR
      tmkError =
#endif
      TMK_BAD_FUNC;
  return (ioctl(_ahVTMK4VxDusb[tmkCurNumber - tmkCnt], TMK_IOCMT_Stop));
}

#endif
