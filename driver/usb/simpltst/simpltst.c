/****************************************************************************/
/*      SIMPLTST v3.02 for Linux. ELCUS, 1999, 2006.                        */
/*      Performs simple test of functionality of the driver tmk1553b.       */
/*      Usage:                                                              */
/*        'simpltst'   - performs test of the card 0,                       */
/*        'simpltst 0' - performs test of the card 0,                       */
/*        'simpltst 1' - performs test of the card 1,                       */
/*        etc.                                                              */
/****************************************************************************/
/****************************************************************************/
/*      SIMPLTST v3.02 for Microsoft Windows. ELCUS, 1999, 2006.            */
/*      Performs simple test of functionality of the driver TMK1553B.SYS.   */
/*      Usage:                                                              */
/*        'SIMPLTST.EXE'   - performs test of the card 0,                   */
/*        'SIMPLTST.EXE 0' - performs test of the card 0,                   */
/*        'SIMPLTST.EXE 1' - performs test of the card 1,                   */
/*        etc.                                                              */
/****************************************************************************/

//#define _TMK1553B_WINDOWS
//#define _TMK1553B_WINDOWSNT
//#define _TMK1553B_LINUX

#define _TMK1553B_MRT

#if defined(_TMK1553B_WINDOWS) || defined(_TMK1553B_WINDOWSNT)
#include <windows.h>
#endif

#include <stdio.h>

#ifdef _TMK1553B_WINDOWS
#include "WDMTMKv2.cpp"
#endif
#ifdef _TMK1553B_WINDOWSNT
#include "ntmk.c"
#define _TMK1553B_WINDOWS
#endif
#ifdef _TMK1553B_LINUX
#include "ltmk.c"
#endif

unsigned short wBase, wAddr, wMaxBase, wPage, wMaxPage, wSubAddr;
int fError, cErrors, fEventResult;

#ifdef _TMK1553B_WINDOWS
HANDLE hEvent;
#endif
#ifdef _TMK1553B_LINUX
int events;
#endif

TTmkConfigData tmkCfg;
TTmkEventData tmkEvD;

int hTmk, nMinRT, nNRT, nMaxRT, iRT;
unsigned dwMRT;

unsigned short awBuf[64];

int fTmkCfg = 0;
int fMrtCfg = 0;

int fDebugOut;

int main(int argc, char *argv[])
{
  if (argc == 1 || sscanf(argv[1], "%d", &hTmk) != 1)
    hTmk = 0;

  fDebugOut = 0;
  if (argc > 1 && argv[argc-1][0] == 'd')
    fDebugOut = 1;

#ifdef _TMK1553B_WINDOWS
  hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (!hEvent)
  {
    printf("CreateEvent() failed!\n");
    goto SimpleTest_Exit;
  }
#endif

  if (TmkOpen())
  {
    printf("TmkOpen() failed!\n");
    goto SimpleTest_Exit;
  }
  printf("TmkOpen() successful!\n");

  //
  //  Tell to the driver the number of the card we want test
  //

  printf("tmkgetmaxn() = %d, mrtgetmaxn() = %d\n", tmkgetmaxn(), mrtgetmaxn());

  if (tmkconfig(hTmk) != 0)
  {
    printf("tmkconfig() failed!\n");
    printf("tmkconfig(hTmk) = %d\n",tmkconfig(hTmk));
    goto SimpleTest_MRT;
  }
  printf("tmkconfig() successful!\n");
  if (fDebugOut)
    printf("tmkgetmaxn() = %d, mrtgetmaxn() = %d\n", tmkgetmaxn(), mrtgetmaxn());
  fTmkCfg = 1;

  tmkselect(hTmk);

  if (fDebugOut)
    printf("tmkselected() = %d, mrtselected() = %d\n", tmkselected(), mrtselected());

  //
  //  Get and display some information about the selected card
  //

  tmkgetinfo(&tmkCfg);
  printf(
      "Device Number:     %d\n"
      "Device Type:       %d\n"
      "Device Name:       %s\n"
      "Device I/O Ports:  %X",
      hTmk,
      tmkCfg.nType,
      tmkCfg.szName,
      tmkCfg.wPorts1
      );
  if (tmkCfg.wPorts2 == 0xFFFF)
  {
    printf("\n");
  }
  else
  {
    printf(
        ", %X\n",
        tmkCfg.wPorts2
        );
  }
  if (tmkCfg.wIrq2 == 0xFF)
  {
    printf(
        "Device Interrupt:  %d\n",
        tmkCfg.wIrq1
        );
  }
  else
  {
    printf(
        "Device Interrupts: %d, %d\n",
        tmkCfg.wIrq1,
        tmkCfg.wIrq2
        );
  }
  printf(
      "Device I/O Delay:  %d\n",
      tmkCfg.wIODelay
      );

  printf("Device HW Version: %d\n\n",
      tmkgethwver()
      );
       
  cErrors = 0;

  //
  //  Now we'll test onboard RAM in Bus Controller mode
  //

  if (bcreset() != 0)
  {
    printf("bcreset() failed!\n\n");
    goto SkipBCTest;
  }

  wMaxBase = bcgetmaxbase();
  printf("bcMaxBase = %u\n", wMaxBase);

  fError = 0;

  for (wBase = 0; wBase <= wMaxBase; ++wBase)
  {
    bcdefbase(wBase);
    for (wAddr = 0; wAddr <= 63; ++wAddr)
    {
      bcputw(wAddr, wAddr|(wBase<<6));
    }
  }
  for (wBase = 0; wBase <= wMaxBase; ++wBase)
  {
    bcdefbase(wBase);
    for (wAddr = 0; wAddr <= 63; ++wAddr)
    {
      if (bcgetw(wAddr) != (unsigned short)(wAddr|(wBase<<6)))
      {
        ++cErrors;
        fError = 1;
      }
    }
  }
  if (!fError)
    printf("bcputw()/bcgetw() test Ok!\n");
  else
    printf("bcputw()/bcgetw() test failed!\n");

  fError = 0;

  for (wBase = 0; wBase <= wMaxBase; ++wBase)
  {
    bcdefbase(wBase);
    for (wAddr = 0; wAddr <= 63; ++wAddr)
    {
      awBuf[63-wAddr] = wBase+(wAddr<<8);
    }
    bcputblk(0, awBuf, 64);
  }
  for (wBase = 0; wBase <= wMaxBase; ++wBase)
  {
    bcdefbase(wBase);
    bcgetblk(0, awBuf, 64);
    for (wAddr = 0; wAddr <= 63; ++wAddr)
    {
      if (awBuf[63-wAddr] != (wBase+(wAddr<<8)))
      {
        ++cErrors;
        fError = 1;
      }
    }
  }
  if (!fError)
    printf("bcputblk()/bcgetblk() test Ok!\n");
  else
    printf("bcputblk()/bcgetblk() test failed!\n");
  
  SkipBCTest:

  //
  //  Now we'll test onboard RAM in Remote Terminal mode
  //

  if (rtreset() != 0)
  {
    printf("rtreset() failed!\n\n");
    goto SkipRTTest;
  }

  wMaxPage = rtgetmaxpage();
  printf("rtMaxPage = %u\n", wMaxPage);

  fError = 0;

  for (wPage = 0; wPage <= wMaxPage; ++wPage)
  {
    rtdefpage(wPage);
    for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
    {
      rtdefsubaddr(RT_RECEIVE, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        rtputw(wAddr, wAddr|(wSubAddr<<8)|(wPage<<13));
      }
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        rtputw(wAddr, (wAddr+32)|(wSubAddr<<8)|(wPage<<13));
      }
    }
  }
  for (wPage = 0; wPage <= wMaxPage; ++wPage)
  {
    rtdefpage(wPage);
    for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
    {
      rtdefsubaddr(RT_RECEIVE, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        if (rtgetw(wAddr) != (wAddr|(wSubAddr<<8)|(wPage<<13)))
        {
          ++cErrors;
          fError = 1;
        }
      }
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        if (rtgetw(wAddr) != ((wAddr+32)|(wSubAddr<<8)|(wPage<<13)))
        {
          ++cErrors;
          fError = 1;
        }
      }
    }
  }
  if (!fError)
    printf("rtputw()/rtgetw() test Ok!\n");
  else
    printf("rtputw()/rtgetw() test failed!\n");

  fError = 0;

  for (wPage = 0; wPage <= wMaxPage; ++wPage)
  {
    rtdefpage(wPage);
    for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
    {
      rtdefsubaddr(RT_RECEIVE, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        awBuf[31-wAddr] = wSubAddr|(wAddr<<8)|(wPage<<13);
      }
      rtputblk(0, awBuf, 32);
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        awBuf[31-wAddr] = (wSubAddr+32)|(wAddr<<8)|(wPage<<13);
      }
      rtputblk(0, awBuf, 32);
    }
  }
  for (wPage = 0; wPage <= wMaxPage; ++wPage)
  {
    rtdefpage(wPage);
    for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
    {
      rtdefsubaddr(RT_RECEIVE, wSubAddr);
      rtgetblk(0, awBuf, 32);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        if (awBuf[31-wAddr] != (wSubAddr|(wAddr<<8)|(wPage<<13)))
        {
          ++cErrors;
          fError = 1;
        }
      }
      rtdefsubaddr(RT_TRANSMIT, wSubAddr);
      rtgetblk(0, awBuf, 32);
      for (wAddr = 0; wAddr <= 31; ++wAddr)
      {
        if (awBuf[31-wAddr] != ((wSubAddr+32)|(wAddr<<8)|(wPage<<13)))
        {
          ++cErrors;
          fError = 1;
        }
      }
    }
  }
  if (!fError)
    printf("rtputblk()/rtgetblk() test Ok!\n");
  else
    printf("rtputblk()/rtgetblk() test failed!\n");

  SkipRTTest:

  printf("Test errors = %d\n\n", cErrors);

  //
  //  Now we'll test interrupt in Bus Controller mode
  //

  if (bcreset() != 0)
    goto SkipEventTest;

#ifdef _TMK1553B_WINDOWS
  tmkdefevent(hEvent, TRUE);
#endif  

  //
  //  Get Event Data with the nInt filed == 0 meaning we haven't
  //  unserved interrupts now
  //

  tmkgetevd(&tmkEvD);
  printf("Int: %d\n", tmkEvD.nInt);

  //
  //  Put in the base 0 the message 'SYNCHRONIZE' in broadcast mode
  //

  bcdefbase(0);
  bcputw(0, 0xFFE1);

  //
  //  Start the message from the base 0 with Control Code CTRL_C_BRCST
  //

  bcstart(0, CTRL_C_BRCST);

  //
  //  Waiting for interrupt (one second for example)
  //

#ifdef _TMK1553B_WINDOWS
  switch (WaitForSingleObject(hEvent, 1000))
  {
  case WAIT_OBJECT_0:
    ResetEvent(hEvent);
    printf("We got interrupt!\n");
    break;
  case WAIT_TIMEOUT:
    printf("We didn't get interrupt!\n");
    break;
  default:
    printf("Somebody abandon our interrupt waiting!\n");
    break;
  }
#endif
#ifdef _TMK1553B_LINUX
  events = tmkwaitevents(1<<hTmk, 1000);
  if (events < 0)
    printf("Error occured during interrupt waiting!\n");
  else if (events == 0)
    printf("We didn't get interrupt!\n");
  else if (events == (1<<hTmk))
    printf("We got interrupt!\n");
  else
    printf("We got very strange interrupt!\n");
#endif

  //
  //  Get Event Data with the nInt filed == 1 meaning we got
  //  normal interrupt from the Bus Controller
  //

  tmkgetevd(&tmkEvD);
  printf("Int: %d\n", tmkEvD.nInt);

#ifdef _TMK1553B_WINDOWS
  tmkdefevent(0,TRUE);
#endif  

  SkipEventTest:

  tmkdone(hTmk);

  SimpleTest_MRT:

  if (fTmkCfg == 1 && tmkCfg.nType != MRTX && tmkCfg.nType != MRTXI)
    goto SimpleTest_Exit;
  if (hTmk > mrtgetmaxn())
    goto SimpleTest_Exit;

  //
  //  Tell to the driver the number of the card we want test
  //

  if ((dwMRT = mrtconfig(hTmk)) == 0)
  {
    printf("mrtconfig() failed!\n");
    goto SimpleTest_Exit;
  }
  printf("mrtconfig() successful!\n");
  if (fDebugOut)
    printf("tmkgetmaxn() = %d, mrtgetmaxn() = %d\n", tmkgetmaxn(), mrtgetmaxn());
  fMrtCfg = 1;

  nMinRT = dwMRT & 0xFFFF;
  nNRT = dwMRT >> 16;
  nMaxRT = nMinRT + nNRT - 1;

  tmkselect(hTmk);

  if (fDebugOut)
    printf("tmkselected() = %d, mrtselected() = %d\n", tmkselected(), mrtselected());

  //
  //  Get and display some information about the selected card
  //

  tmkgetinfo(&tmkCfg);
  printf(
      "Card Number:     %d\n"
      "Card Type:       %d\n"
      "Card Name:       %s\n"
      "Card I/O Ports:  %X-%X",
      hTmk,
      tmkCfg.nType,
      tmkCfg.szName,
      tmkCfg.wPorts1, tmkCfg.wPorts1 + 0xF
      );
  if (tmkCfg.wPorts2 == 0xFFFF)
  {
    printf("\n");
  }
  else
  {
    printf(
        ", %X-%X\n",
        tmkCfg.wPorts2, tmkCfg.wPorts2 + 0xF
        );
  }
  if (tmkCfg.wIrq2 == 0xFF)
  {
    printf(
        "Card Interrupt:  %d\n",
        tmkCfg.wIrq1
        );
  }
  else
  {
    printf(
        "Card Interrupts: %d, %d\n",
        tmkCfg.wIrq1,
        tmkCfg.wIrq2
        );
  }
  printf(
      "Card I/O Delay:  %d\n\n",
      tmkCfg.wIODelay
      );

  printf("First RT number: %d\nNumber of RTs: %d\n\n", nMinRT, nNRT);

  cErrors = 0;

  if (mrtreset() != 0)
  {
    printf("mrtreset() failed!\n\n");
    goto SkipMRTTest;
  }

  //
  //  Now we'll test onboard RAM in Remote Terminal mode
  //

  for (iRT = nMinRT; iRT <= nMaxRT; ++iRT)
  {
    if (tmkselect(iRT))
    {
      printf("tmkselect(%d) failed!\n", iRT);
      continue;
    }
    printf("tmkselect(%d) successful!\n", iRT);
    if (fDebugOut)
      printf("tmkselected() = %d, mrtselected() = %d\n", tmkselected(), mrtselected());

    rtreset();

    wMaxPage = rtgetmaxpage();
    printf("rtMaxPage = %u\n", wMaxPage);

    fError = 0;

    for (wPage = 0; wPage <= wMaxPage; ++wPage)
    {
      rtdefpage(wPage);
      for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
      {
        rtdefsubaddr(RT_RECEIVE, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          rtputw(wAddr, wAddr|(wSubAddr<<8)|(wPage<<13));
        }
        rtdefsubaddr(RT_TRANSMIT, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          rtputw(wAddr, (wAddr+32)|(wSubAddr<<8)|(wPage<<13));
        }
      }
    }
    for (wPage = 0; wPage <= wMaxPage; ++wPage)
    {
      rtdefpage(wPage);
      for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
      {
        rtdefsubaddr(RT_RECEIVE, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          if (rtgetw(wAddr) != (wAddr|(wSubAddr<<8)|(wPage<<13)))
          {
            ++cErrors;
            fError = 1;
          }
        }
        rtdefsubaddr(RT_TRANSMIT, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          if (rtgetw(wAddr) != ((wAddr+32)|(wSubAddr<<8)|(wPage<<13)))
          {
            ++cErrors;
            fError = 1;
          }
        }
      }
    }
    if (!fError)
      printf("rtputw()/rtgetw() test Ok!\n");
    else
      printf("rtputw()/rtgetw() test failed!\n");

    fError = 0;

    for (wPage = 0; wPage <= wMaxPage; ++wPage)
    {
      rtdefpage(wPage);
      for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
      {
        rtdefsubaddr(RT_RECEIVE, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          awBuf[31-wAddr] = wSubAddr|(wAddr<<8)|(wPage<<13);
        }
        rtputblk(0, awBuf, 32);
        rtdefsubaddr(RT_TRANSMIT, wSubAddr);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          awBuf[31-wAddr] = (wSubAddr+32)|(wAddr<<8)|(wPage<<13);
        }
        rtputblk(0, awBuf, 32);
      }
    }
    for (wPage = 0; wPage <= wMaxPage; ++wPage)
    {
      rtdefpage(wPage);
      for (wSubAddr = 0; wSubAddr <= 0x1F; ++wSubAddr)
      {
        rtdefsubaddr(RT_RECEIVE, wSubAddr);
        rtgetblk(0, awBuf, 32);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          if (awBuf[31-wAddr] != (wSubAddr|(wAddr<<8)|(wPage<<13)))
          {
            ++cErrors;
            fError = 1;
          }
        }
        rtdefsubaddr(RT_TRANSMIT, wSubAddr);
        rtgetblk(0, awBuf, 32);
        for (wAddr = 0; wAddr <= 31; ++wAddr)
        {
          if (awBuf[31-wAddr] != ((wSubAddr+32)|(wAddr<<8)|(wPage<<13)))
          {
            ++cErrors;
            fError = 1;
          }
        }
      }
    }
    if (!fError)
      printf("rtputblk()/rtgetblk() test Ok!\n");
    else
      printf("rtputblk()/rtgetblk() test failed!\n");
  }

  SkipMRTTest:

  printf("Test errors = %d\n\n", cErrors);

  tmkdone(ALL_TMKS);

  SimpleTest_Exit:

  if (!fTmkCfg && !fMrtCfg)
    printf("Somebody works with the card %d or the card doesn't exist in Registry\n", hTmk);

  TmkClose();

#ifdef _TMK1553B_WINDOWS
  if (hEvent)
    CloseHandle(hEvent);
#endif

  return 0;
}
