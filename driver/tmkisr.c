
/****************************************************************************/
/*      TMKISR.C v4.06. ELCUS, 1998,2011                                    */
/*      ISR stuff for Windows, Linux, QNX4, QNX6                            */
/****************************************************************************/
//4.03
// - QNX4 nInt==3 event data swap fixed
// - moved MRT irq processing to DPC
// - improved MRT irq processing in DPC, using rtgetblkmrta/rtputblkmrta
// - fixed bug with device type in DPC (nInDpcType added)
//4.05
// - small TMK_INT_OTHER fix (pEvD->awEvData[2] = 0;)
//4.06
// - pEvD->nInt = 0; etc.; added for brc MRT irq when no fWriteSA
// - spin_lock_irqsave/spin_unlock_irqrestore regardless of __SMP__

#ifdef LINUX
#define outpw(port, data) outw(data, port)
#define inpw(port) inw(port)
#define outpb(port, data) outb(data, port)
#define inpb(port) inb(port)
#endif

__inline void IncDpcData(int hTmk)
{
  pTListEvD pEvD;

  if (cEvData[hTmk] < EVENTS_SIZE)
  {
    ++cDpcData[hTmk];
    iEvDataEnd[hTmk] = (iEvDataEnd[hTmk] + 1) & (EVENTS_SIZE-1);
  }
  else
  {
    pEvD = &(aEvData[hTmk][iEvDataEnd[hTmk]]);
    pEvD->nInt = 5;
    pEvD->wMode = UNDEFINED_MODE;
    pEvD->awEvData[0] = TMK_INT_DRV_OVF;
    pEvD->awEvData[1] = 0;
    pEvD->awEvData[2] = 0;
  }
}

//
// This is the ISR
//

#ifdef QNX6
const struct sigevent* tmkInterruptServiceRoutine(void* arg, int id)
{
//  LARGE_INTEGER ddwTick, ddwTick1;
//  unsigned short wTimer;
//  int fReadTime = 1;
#endif //QNX6

#ifdef QNX4
pid_t far tmkInterruptServiceRoutine()
{
#endif //QNX4

#ifdef LINUX
IRQRETURN_T tmkInterruptServiceRoutine(int irq, void *dev_id
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
                                       , struct pt_regs *regs
#endif
                                      )
{
  int hIrq;
#endif //LINUX

#ifdef WIN
BOOLEAN
tmkInterruptServiceRoutine(
    IN PKINTERRUPT Interrupt,
    IN OUT PVOID Context
    )
{
//  PDEVICE_OBJECT DeviceObject;
//  PTMK_DEVICE_EXTENSION extension;
  LARGE_INTEGER ddwTick, ddwTick1;
  unsigned short wTimer;
  int fReadTime = 1;
#endif //WIN

  int ihTmk, hTmk;
//  int fTaskletSchedule = 0;
  int fMyISR = FALSE;
  pTListEvD pEvD;
  TIrq *pIrq;
  TTmkConfig *pTmkCfg;
  unsigned intr;
  unsigned saved;
#ifdef LINUX
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
  static struct tq_struct tmkDpc_task = {NULL, 0, tmkDpcRoutine, 0};
#endif
  unsigned long flags;
#endif //LINUX

#ifdef QNX6
  InterruptLock(&tmkIrqSpinLock);
#endif //QNX6

#ifdef QNX4
  pIrq = &tmkIrq;
#endif //QNX4  

#ifdef LINUX
  hIrq = (int)((ptrdiff_t)dev_id - (ptrdiff_t)tmkInterruptServiceRoutine);
  pIrq = &ahIrq[hIrq];

//  #ifdef MY_DBG_DPC
//  printk(MY_KERN_DEBUG "Tmk1553b: irq %d\n", hIrq);
//  #endif

  spin_lock_irqsave(&tmkIrqSpinLock, flags);
#endif //LINUX

#ifdef WIN
//  DeviceObject = Context;
//  extension = DeviceObject->DeviceExtension;

  pIrq = (TIrq*)Context;
#endif //WIN

#ifdef QNX6
  // !!!
  for (hTmk = 0; hTmk <= MAX_TMK_NUMBER; ++hTmk) 
  { 
    if((id != irq_id[hTmk]) || (aTmkConfig[hTmk].nType == -1))
      continue;
#else
  for (ihTmk = 0; ihTmk < pIrq->cTmks; ++ihTmk)
  {
    hTmk = pIrq->hTmk[ihTmk];
#endif //def QNX6
    pTmkCfg = &aTmkConfig[hTmk];
    if (pTmkCfg->nType == TAI || pTmkCfg->nType == MRTAI)
    {
      if ((inpw(pTmkCfg->wPorts1 + 0x6) & 0xE000) == 0)
        continue;
    }
    else if (pTmkCfg->nType == TMKXI || pTmkCfg->nType == MRTXI)
    {
      if (pTmkCfg->fLocalReadInt)
      {
        if ((inpw(pTmkCfg->wPorts1 + 0xA) & 0x8000) == 0)
          continue;
      }
      else
      {
        if ((inpw(pTmkCfg->wHiddenPorts + 0x4C) & 0x0004) == 0)
          continue;
      }
    }
    fMyISR = TRUE;
//    fTaskletSchedule = 1;

#ifdef WIN
    if (fReadTime)
    {
      if (fUseEvTime)
      {
//        do
//        {
//          KeQuerySystemTime((PLARGE_INTEGER)&ddwTime1);
//          dwTimer = get_timer();
//          KeQuerySystemTime((PLARGE_INTEGER)&ddwTime);
//        }
//        while (ddwTime.LowPart != ddwTime1.LowPart);// || (wTimer-wTimer1) > 20);
//    //    wTimer |= wTimer1<<16;
//        ddwTime.LowPart = (ULONG)(ddwTime.QuadPart / 10);
        do
        {
          KeQueryTickCount(&ddwTick1);
          wTimer = get_timer();
          KeQueryTickCount(&ddwTick);
        }
        while (ddwTick.LowPart != ddwTick1.LowPart);
      }
      else
      {
//        ddwTime.LowPart = 0;
//        dwTimer = 0;
        ddwTick.LowPart = 0;
        wTimer = 0;
      }
      fReadTime = 0;
    }
#endif //WIN

    saved = DIRQLTmkSave(hTmk);
    do
    {
      pEvD = &(aEvData[hTmk][iEvDataEnd[hTmk]]);

      intr = DIRQLTmksInt1(hTmk, pEvD);

      if (intr & TMK_INT_SAVED)
      {
#ifdef WIN
        aTickCount[hTmk][iEvDataEnd[hTmk]] = ddwTick.LowPart;
        aTimerCount[hTmk][iEvDataEnd[hTmk]] = wTimer;
#endif //WIN
        IncDpcData(hTmk);
////        fTaskletSchedule = 1;
      }//if (intr & TMK_INT_SAVED)
      if (intr & TMK_INT_OTHER) //TMK_INT_TIMER | TMK_INT_BUSJAM | TMK_INT_FIFO_OVF | TMK_INT_GEN1 | TMK_INT_GEN2...
      {
        pEvD = &(aEvData[hTmk][iEvDataEnd[hTmk]]);
        pEvD->nInt = 5;
        pEvD->wMode = UNDEFINED_MODE;
        pEvD->awEvData[0] = (unsigned short) (intr & TMK_INT_OTHER);
// this requires SMP support in tmkgettimer or move this to DPC
//        *((ULONG*)(&(pEvD->awEvData[1]))) = tmkgettimer(m_Unit+hTmk);
        pEvD->awEvData[1] = 0;
        pEvD->awEvData[2] = 0;
#ifdef WIN
        aTickCount[hTmk][iEvDataEnd[hTmk]] = ddwTick.LowPart;
        aTimerCount[hTmk][iEvDataEnd[hTmk]] = wTimer;
#endif //WIN
        IncDpcData(hTmk);
////        fTaskletSchedule = 1;
      }
    }
    while (intr & TMK_INT_MORE);
    DIRQLTmkRestore(hTmk, saved);
  }

#ifdef QNX6
  InterruptUnlock(&tmkIrqSpinLock);
  if (fMyISR) 
    return(&intr_event); 
  else
    return(NULL);
#endif //QNX6

#ifdef QNX4
  return (fMyISR) ? tmk_proxy : 0;
#endif //QNX4

#ifdef LINUX
  spin_unlock_irqrestore(&tmkIrqSpinLock, flags);

//  if (!fTaskletSchedule)
  if (!fMyISR)
    return IRQ_NONE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
  tasklet_schedule(&tmkDpc);
#else
  queue_task(&tmkDpc_task, &tq_immediate);
  mark_bh(IMMEDIATE_BH);
#endif
  return IRQ_HANDLED;
#endif //LINUX

#ifdef WIN
//  if (fTaskletSchedule)
  if (fMyISR)
  {
    KeInsertQueueDpc(
        &tmkDpc,
        0,
        0);
  }
  return fMyISR;
#endif //WIN

}

//
// This is the deferred procedure call that gets data queued by the ISR to
// finish any interrupt relate processing (AKA tasklet)
// DpcIExcBC called in tmkgetevd() for LINUX and WIN 
// DpcIExcBC called in tmkDpcRoutine() for QNX4  
//

#ifdef QNX6
void
tmkDpcRoutine()
{
#endif //QNX6

#ifdef QNX4
void
tmkDpcRoutine()
{
#endif //QNX4

#ifdef LINUX
void
tmkDpcRoutine(
    unsigned long SystemArgument
    )
{
  TListProc *hlnProcTmk;
  TListProc *hlnProcWake[MAX_TMK_NUMBER+1+MAX_RT_NUMBER+1];
  int cToWake = 0;
#endif //LINUX

#ifdef WIN
VOID
tmkDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
//  PDEVICE_OBJECT DeviceObject;
//  PTMK_DEVICE_EXTENSION extension;
#endif //WIN

  int hTmk, cDpcDataTmk, hTmkI;
  pTListEvD pEvD, pEvD0;
#ifndef DYNAMIC_TMKNUM
  int hTmkSave;
#endif
  unsigned short wSaveSA, wSaveLock;
  int fWriteSA;
  int iEvD, cEvD, iEvDStop;
  unsigned short wSA, nWords;
  char afMRT[MAX_TMK_NUMBER+1];
  int nInt, hTmk0, hTmk1, hTmkT;
  unsigned short wMask;
  int nInDpcType;
#ifdef QNX4
  unsigned short tmpw;
#endif //QNX4

#ifndef DYNAMIC_TMKNUM
#define HTMK
#define HTMK__
#else
#define HTMK hTmk
#define HTMK__ hTmk, 
#endif

#ifdef QNX6
  pthread_spin_lock(&tmkSpinLock);
  #ifdef MY_DBG_DPC
  printf("DPC\n");
  #endif
#endif //QNX6

#ifdef LINUX
  spin_lock(&tmkSpinLock);

//  #ifdef MY_DBG_DPC
//  printk(MY_KERN_DEBUG "Tmk1553b: DPC ");
//  #endif
#endif //LINUX

#ifdef WIN
//  DeviceObject = DeferredContext;
//  extension = DeviceObject->DeviceExtension;

  KeAcquireSpinLockAtDpcLevel(&tmkSpinLock);

  #ifdef MY_DBG_DPC
  KdPrint(("DPC\n"));
  #endif
#endif //WIN

#ifndef DYNAMIC_TMKNUM
  hTmkSave = tmkselected();
#endif
  for (hTmk = 0; hTmk <= nMaxTmkNumber; ++hTmk)
  {
    if (hTmk <= MAX_TMK_NUMBER)
    {
      if (aTmkConfig[hTmk].nType == -1)
        continue;
      afMRT[hTmk] = 0;
      hTmkI = hTmk;
      nInDpcType = aTmkConfig[hTmk].nType;
    }
    else
    {
      hTmkI = rt2mrt(hTmk);
      if (afMRT[hTmkI] == 0)
        continue;
      nInDpcType = aTmkConfig[hTmkI].nType; 
    }

#ifdef QNX6
    InterruptLock(&tmkIrqSpinLock);
    cDpcDataTmk = cDpcData[hTmk];  // dwSyncArg
    cDpcData[hTmk] = 0;            
    InterruptUnlock(&tmkIrqSpinLock);
#endif //QNX6

#ifdef QNX4
    _disable();
    cDpcDataTmk = cDpcData[hTmk];
    cDpcData[hTmk] = 0;
    _enable();
#endif //QNX4

#ifdef LINUX
    spin_lock_irq(&tmkIrqSpinLock);
    cDpcDataTmk = cDpcData[hTmk];  // dwSyncArg
    cDpcData[hTmk] = 0;            
    spin_unlock_irq(&tmkIrqSpinLock);
#endif //LINUX

#ifdef WIN
    cDpcDataTmk = hTmk;
    KeSynchronizeExecution(ahIrq[aTmkConfig[hTmkI].hIrq].InterruptObject1,
                           tmkGetClrIrqs,
                           &cDpcDataTmk);
#endif //WIN

    if (cDpcDataTmk == 0)
      continue;

#ifdef QNX6
    #ifdef MY_DBG_DPC
    printf("Dpc %d (%d) cDpcDataTmk %d\n", hTmk, hTmkI, cDpcDataTmk);
    fflush(stdout);
    #endif
#endif //QNX6

#ifdef LINUX
//    #ifdef MY_DBG_DPC
//    printk(MY_KERN_DEBUG "Tmk1553b: dpc %d, %d\n", hTmk, cDpcDataTmk);
//    #endif
#endif //LINUX

#ifdef WIN
    #ifdef MY_DBG_DPC
    KdPrint(("Dpc %d (%d)\n", hTmk, hTmkI));
    #endif
#endif //WIN

// Drop last
//    if (cDpcDataTmk + cEvData[hTmk] > EVENTS_SIZE)
//      cDpcDataTmk = EVENTS_SIZE - cEvData[hTmk];
// Drop last

    cEvData[hTmk] += cDpcDataTmk;

// Drop first
    if (cEvData[hTmk] > EVENTS_SIZE)
    {
      iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + cEvData[hTmk] - EVENTS_SIZE) & (EVENTS_SIZE-1);
      cEvData[hTmk] = EVENTS_SIZE;
    }
// Drop first

    if (hTmk <= MAX_TMK_NUMBER &&
        (nInDpcType == MRTX || nInDpcType == MRTXI ||
         nInDpcType == MRTA || nInDpcType == MRTAI))
    {
      afMRT[hTmk] = 1;

        iEvD = (iEvDataBegin[hTmk] + cEvData[hTmk] - cDpcDataTmk) & (EVENTS_SIZE-1);
        iEvDStop = (iEvD + cDpcDataTmk) & (EVENTS_SIZE-1);
        // process all brc commands and datas
        // for MRTX would be better to find last brc data only
        // (because of single brc subaddr on MRTX)
        // !!! move cmds copy for MRTA from irq !!!
        do
        {
          pEvD = &(aEvData[hTmk][iEvD]);

#ifdef QNX6
          #ifdef MY_DBG_DPC
          printf("MRT nInt %Xh mode %d EvData %Xh %Xh %Xh\n", pEvD->nInt, pEvD->wMode, pEvD->awEvData[0], pEvD->awEvData[1], pEvD->awEvData[2]);
          #endif
#endif //QNX6
#ifdef WIN
          #ifdef MY_DBG_DPC
          KdPrint(("MRT nInt %d\n", pEvD->nInt));
          #endif
#endif //WIN

          if (pEvD->nInt == 4)
          {
            hTmk0 = (int)(pEvD->awEvData[1]);
            hTmkT = hTmk0 >> 8; // transmitting RT (if exist)
            hTmk0 &= 0x00FF;    // min receiving RT
            hTmk1 = hTmk0 + (int)(pEvD->awEvData[2]) - 1; // max receiving RT
            wMask = 0xFFFF;

            if (pEvD->awEvData[0] & 0x4000)
            {
              nInt = 2;
              wMask = 0x47FF;
            }
            else if (((pEvD->awEvData[0] & 0x03E0) == 0x0000) ||
                     ((pEvD->awEvData[0] & 0x03E0) == 0x03E0))
            {
              nInt = 1;
              wMask = 0x041F;
            }
            else
            {
              nInt = 3;
              wMask = 0x07FF;
            }
            pEvD->awEvData[1] = (unsigned short)nInt;

            if (nInt == 3 && (adwTmkOptions[hTmk] & MRT_READ_BRC_DATA))
            {
              unsigned short wSavePage = 0xFFFF;
  #ifndef DYNAMIC_TMKNUM
              tmkselect(hTmk);
  #endif
              wSA = pEvD->awEvData[0] & 0x03E0;
              nWords = pEvD->awEvData[0] & 0x1F;
              if (nWords == 0)
                nWords = 32;
              if (nInDpcType == MRTX || nInDpcType == MRTXI)
              {
                mrtdefbrcsubaddr0(HTMK); // save ???
                rtgetblk(HTMK__ 0, &(awBrcRxBuf[hTmk][wSA]), nWords);
                if (rtgetmode(HTMK) & RT_FLAG_MODE)
                {
                  rtdefsubaddr(HTMK__ RT_RECEIVE, wSA>>5);
                  rtclrflag(HTMK); // clrflag in mrt aka rt0
                    // the flag may be cleared later if rt0 is used
                    // (subj to optimize)
                    // but if rt0 isn't used we have to have the flag cleared!
                    // also the cleared flag is checked below before writing
                    // saved awBrcRxBuf data into rt0 (so if we do not clear 
                    // it here we need a workaround for rt0
                }
              }
              else //(nInDpcType == MRTA || nInDpcType == MRTAI)
              {
                wSavePage = rtgetpage(HTMK);
                mrtdefbrcpage(HTMK__ 0);
                rtdefsubaddr(HTMK__ RT_RECEIVE, wSA>>5);
                __rtgetblkmrta(HTMK__ 0, &(awBrcRxBuf[hTmk][wSA]), nWords);
                __rtgetblkmrta(HTMK__ 58, &(awBrcRxState[hTmk][wSA>>3]), 3);
                // rtgetblk does not accept position 58
//                if (rtgetmode(HTMK) & RT_FLAG_MODE)
//                {
//                  rtclrflag(HTMK);
                    // We may need clrflag in the mrt brc page to be able to 
                    // receive new brc data if the brc page uses flags. 
                    // Actually it could be better (but more complex) to 
                    // clear the flag when saved awBrcRxBuf data are written 
                    // into all RT pages.
                    // But now we do not use flags in the brc page at all so 
                    // it always can receive data.
//                }
                rtdefpage(HTMK__ wSavePage);
                wSavePage = 0xFFFF;
              }
              if (wSavePage != 0xFFFF)
                rtdefpage(HTMK__ wSavePage);
  #ifndef DYNAMIC_TMKNUM
              tmkselect(hTmkSave);
  #endif
            }

            while (hTmk0 <= hTmk1)
            {
              if (!__rtDisableMask[hTmk0] && (hTmk0 != hTmkT))
              {
                pEvD0 = &(aEvData[hTmk0][iEvDataEnd[hTmk0]]);
                pEvD0->nInt = (nInt == 2) ? 2 : 4;
                pEvD0->wMode = RT_MODE;
                pEvD0->awEvData[0] = pEvD->awEvData[0] & wMask;
                pEvD0->awEvData[1] = (unsigned short)nInt;
#ifdef WIN
                aTickCount[hTmk0][iEvDataEnd[hTmk0]] = ddwTick.LowPart;
                aTimerCount[hTmk0][iEvDataEnd[hTmk0]] = wTimer;
#endif //WIN
                IncDpcData(hTmk0);
////                fTaskletSchedule = 1;
              }
              ++hTmk0;
            }
          } //if (pEvD->nInt == 4)
          else
          {
            hTmk0 = (int)(pEvD->awEvData[1]);
            pEvD0 = &(aEvData[hTmk0][iEvDataEnd[hTmk0]]);
            pEvD0->nInt = pEvD->nInt;
            pEvD0->wMode = RT_MODE;
            pEvD0->awEvData[0] = pEvD->awEvData[0];
#ifdef WIN
            aTickCount[hTmk0][iEvDataEnd[hTmk0]] = ddwTick.LowPart;
            aTimerCount[hTmk0][iEvDataEnd[hTmk0]] = wTimer;
#endif //WIN
            IncDpcData(hTmk0);
////            fTaskletSchedule = 1;
          }

          iEvD = (iEvD + 1) & (EVENTS_SIZE-1);
        }
        while (iEvD != iEvDStop);

    }

    if (hTmk > MAX_TMK_NUMBER)
//        && ((adwTmkOptions[hTmk] & MRT_WRITE_BRC_DATA) ||
//         (adwTmkOptions[hTmk] & RT_NO_BRC_IRQS)))
    {
      iEvD = (iEvDataBegin[hTmk] + cEvData[hTmk] - cDpcDataTmk) & (EVENTS_SIZE-1);
      cEvD = cDpcDataTmk;
      do
      {
        pEvD = &(aEvData[hTmk][iEvD]);
#ifdef QNX6
        #ifdef MY_DBG_DPC
        printf("MRT1 nInt %d", pEvD->nInt);
        #endif
#endif //QNX6
#ifdef WIN
        #ifdef MY_DBG_DPC
        KdPrint(("MRT1 nInt %d", pEvD->nInt));
        #endif
#endif //WIN
        if (pEvD->nInt == 4)
        {
//          if (adwTmkOptions[hTmk] & RT_NO_BRC_IRQS)
          pEvD->nInt = (int)(pEvD->awEvData[1]);
#ifdef QNX6
          #ifdef MY_DBG_DPC
          printf(" -> %d", pEvD->nInt);
          #endif
#endif //QNX6
#ifdef WIN
          #ifdef MY_DBG_DPC
          KdPrint((" -> %d", pEvD->nInt));
          #endif
#endif //WIN
          if ((int)(pEvD->awEvData[1]) == 3 &&
              (adwTmkOptions[hTmk] & MRT_WRITE_BRC_DATA))
          {
            unsigned short wSavePage = 0xFFFF;
#ifndef DYNAMIC_TMKNUM
            tmkselect(hTmk);
#endif
            if (rtgetirqmode(HTMK) & RT_DATA_BL)
            {
              pEvD->nInt = 0;
              if (iEvD == iEvDataBegin[hTmk])
              {
                iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + 1) & (EVENTS_SIZE-1);
                --cEvData[hTmk];
                --cDpcDataTmk;
              }
            }

            if (nInDpcType == MRTA || nInDpcType == MRTAI)
            {
              wSavePage = rtgetpage(HTMK);
              rtdefpage(HTMK__ 0);
            }
            wSaveSA = rtgetsubaddr(HTMK);  // how about saves of mrt data ???
            wSaveLock = rtgetlock(HTMK);
            wSA = (pEvD->awEvData[0] >> 5) & 0x1F;
            if (rtgetmode(HTMK) & RT_FLAG_MODE)
            {
              fWriteSA = !(rtgetflag(HTMK__ RT_RECEIVE, wSA) & RT_FLAG_MASK);
            }
            else
            {
              fWriteSA = !(wSaveLock == (wSA | 0x0800));
            }

            if (fWriteSA)
            {
              rtdefsubaddr(HTMK__ RT_RECEIVE, wSA);
              nWords = pEvD->awEvData[0] & 0x1F;
              if (nWords == 0)
                nWords = 32;
              if (nInDpcType == MRTX || nInDpcType == MRTXI)
                rtputblk(HTMK__ 0, &(awBrcRxBuf[hTmkI][wSA<<5]), nWords);
              else // (nInDpcType == MRTA || nInDpcType == MRTAI)
              {
                __rtputblkmrta(HTMK__ 0, &(awBrcRxBuf[hTmkI][wSA<<5]), nWords);
                __rtputblkmrta(HTMK__ 58, &(awBrcRxState[hTmkI][wSA<<2]), 3);
                // rtputblk does not accept position 58
              }
            }
            else
            {
              pEvD->nInt = 0;
              if (iEvD == iEvDataBegin[hTmk])
              {
                iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + 1) & (EVENTS_SIZE-1);
                --cEvData[hTmk];
                --cDpcDataTmk;
              }
            }

            if (rtgetmode(HTMK) & RT_FLAG_MODE)
            {
              if (fWriteSA)
              {
                rtputflag(HTMK__ RT_RECEIVE, wSA, pEvD->awEvData[0] | RT_FLAG_MASK); // rtsetflag(HTMK);
              }
              rtdefsubaddr(HTMK__ wSaveSA, wSaveSA);
            }
            else
            {
              if (fWriteSA)
              {
                if (wSaveLock & 0x0800)
                  rtlock(HTMK__ wSaveLock, wSaveLock);
                else
                  rtdefsubaddr(HTMK__ wSaveSA, wSaveSA);
              }
            }
            if (wSavePage != 0xFFFF)
              rtdefpage(HTMK__ wSavePage);
#ifndef DYNAMIC_TMKNUM
            tmkselect(hTmkSave);
#endif
          }
        }
#ifdef QNX6
        #ifdef MY_DBG_DPC
        printf(("\n"));
        #endif
#endif //QNX6
#ifdef WIN
        #ifdef MY_DBG_DPC
        KdPrint(("\n"));
        #endif
#endif //WIN
        iEvD = (iEvD + 1) & (EVENTS_SIZE-1);
      }
      while (--cEvD);
    }

#ifdef QNX6
    if (cDpcDataTmk > 0)
    {
      hlnProcTmk = aTMK[hTmk].hlnProc;
      if (hlnProcTmk)
      {
        tmkEvents[hTmk>>5] |= (1<<hTmk);
//    #ifdef MY_DBG_DPC
//    printk(MY_KERN_DEBUG " %X ", tmkEvents[hTmk>>5]);
//    #endif    
        if (hlnProcTmk->maskEvents[hTmk>>5] & (1<<hTmk))
        {
          hlnProcWake[cToWake++] = hlnProcTmk;
//          hlnProcTmk->maskEvents = 0;
          memset(hlnProcTmk->maskEvents, 0, sizeof(hlnProcTmk->maskEvents));
        }
        else
        {
          if (cEvData[hTmk] == EVENTS_SIZE)
          {
            iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + EVENTS_SIZE/2) & (EVENTS_SIZE-1);
            cEvData[hTmk] = EVENTS_SIZE/2;
          }
        }
      }
      else
      {
        iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + cEvData[hTmk]) & (EVENTS_SIZE-1);
        cEvData[hTmk] = 0;
      }
    }
#endif //QNX6

#ifdef QNX4
    if (cDpcDataTmk > 0)
    {
      iEvD = (iEvDataBegin[hTmk] + cEvData[hTmk] - cDpcDataTmk) & (EVENTS_SIZE-1);
      cEvD = cDpcDataTmk;
      do
      {
        pEvD = &(aEvData[hTmk][iEvD]);
        pEvD->nInt |= (pEvD->nInt << 16);
        switch (pEvD->wMode)
        {
        case BC_MODE:
        case MT_MODE:
// check for tmkselect/tmkselected !!!        
          if (pEvD->nInt)
          {
            if (pEvD->nInt == 0x30003)
            {
              tmpw = pEvD->awEvData[1];
              pEvD->awEvData[1] = pEvD->awEvData[0];
              pEvD->awEvData[0] = tmpw;
            }
            else if (pEvD->awEvData[2])
              DpcIExcBC(hTmk, pEvD);
            if (pTmkStartOCB->tmkProxy)
            {
              if (pTmkStartOCB->wProxyMode)
                pTmkStartOCB->aiPxD[(pTmkStartOCB->iiPxD + pTmkStartOCB->ciPxD++) & 511] = iEvD;
              Trigger(pTmkStartOCB->tmkProxy);
            }
            iPxD[hTmk] = iEvD;
          }
          break;
        case RT_MODE:
        case MRT_MODE:
          if (pEvD->nInt)
          {
            if (pEvD->nInt == 0x10001)
              pEvD->awEvData[1] = pEvD->awEvData[0];
            for (pOCB = FirstOCB[hTmk].pNext; pOCB != NULL; pOCB = pOCB->pNext)
            {
              if (pOCB->tmkProxy)
              {
                if (pOCB->wModeDeep && pOCB->wProxyMode)
                  pOCB->aiPxD[(pOCB->iiPxD + pOCB->ciPxD++) & 511] = iEvD;
                Trigger(pOCB->tmkProxy);
              }
            }
            iPxD[hTmk] = iEvD;
          }
          break;
        }
        iEvD = (iEvD + 1) & (EVENTS_SIZE-1);
      }
      while (--cEvD);
      iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + cEvData[hTmk]) & (EVENTS_SIZE-1);
      cEvData[hTmk] = 0;
    }
#endif //QNX4

#ifdef LINUX
    if (cDpcDataTmk > 0)
    {
      hlnProcTmk = aTMK[hTmk].hlnProc;
      if (hlnProcTmk)
      {
        tmkEvents[hTmk>>5] |= (1<<hTmk);
//    #ifdef MY_DBG_DPC
//    printk(MY_KERN_DEBUG " %X ", tmkEvents[hTmk>>5]);
//    #endif    
        if (hlnProcTmk->maskEvents[hTmk>>5] & (1<<hTmk))
        {
          hlnProcWake[cToWake++] = hlnProcTmk;
//          hlnProcTmk->maskEvents = 0;
          memset(hlnProcTmk->maskEvents, 0, sizeof(hlnProcTmk->maskEvents));
        }
        else
        {
          if (cEvData[hTmk] == EVENTS_SIZE)
          {
            iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + EVENTS_SIZE/2) & (EVENTS_SIZE-1);
            cEvData[hTmk] = EVENTS_SIZE/2;
          }
        }
      }
      else
      {
        iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + cEvData[hTmk]) & (EVENTS_SIZE-1);
        cEvData[hTmk] = 0;
      }
    }
#endif //LINUX

#ifdef WIN
    if (aTMK[hTmk].hEvent)
    {
//      for (; cDpcDataTmk != 0; --cDpcDataTmk)
      #ifdef MY_DBG_DPC
      KdPrint(("DpcEvent %d for Tmk %d\n", cDpcDataTmk, hTmk));
      #endif
      if (cDpcDataTmk != 0)
      {
        KeSetEvent(aTMK[hTmk].hEvent, (KPRIORITY)0, FALSE);
      }
    }
    else
    {
      iEvDataBegin[hTmk] = (iEvDataBegin[hTmk] + cEvData[hTmk]) & (EVENTS_SIZE-1);
      cEvData[hTmk] = 0;
    }
#endif //WIN

  }

#ifdef QNX6
  pthread_spin_unlock(&tmkSpinLock);
  while (cToWake > 0)
  {
//    wake_up_interruptible(&hlnProcWake[--cToWake]->wq);//!!!!!
    #ifdef MY_DBG
    printf("Sending pulse!\n");
    #endif
    MsgSendPulse(hlnProcWake[--cToWake]->IntConnect, PRIORITY, TMK_PULSE_EVENT, 0);
  }
//  pthread_spin_unlock(&tmkSpinLock);// OR HERE!!
#endif //QNX6

#ifdef LINUX
  spin_unlock(&tmkSpinLock);

  while (cToWake > 0)
  {
//    #ifdef MY_DBG_DPC
//    printk(MY_KERN_DEBUG "w");
//    #endif    
    wake_up_interruptible(&hlnProcWake[--cToWake]->wq);
  }
//    #ifdef MY_DBG_DPC
//    printk(MY_KERN_DEBUG "\n");
//    #endif
#endif //LINUX

#ifdef WIN
  KeReleaseSpinLockFromDpcLevel(&tmkSpinLock);
#endif //WIN

}

#ifdef WIN
BOOLEAN
tmkGetClrIrqs(
    IN OUT PVOID Context
    )
{
  int hTmk = *(PULONG)Context;
  *(PULONG)Context = cDpcData[hTmk];
  cDpcData[hTmk] = 0;

  return TRUE;
}
#endif //WIN
