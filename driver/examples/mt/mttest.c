#define _TMK1553B_LINUX
#ifdef _TMK1553B_WINDOWS
#include <windows.h>
#include <conio.h>
#include "WDMTMKv2.cpp"
#endif
#ifdef _TMK1553B_WINDOWSNT
#include "ntmk.c"
#define _TMK1553B_WINDOWS
#endif
#ifdef _TMK1553B_LINUX
#include "ltmk.c"
#endif

int mt = 0;
#ifdef _TMK1553B_WINDOWS
HANDLE hEvent;
#endif
#ifdef _TMK1553B_LINUX
int events;
#define WAIT_OBJECT_0 1
#define WAIT_TIMEOUT 0
int timeout = 0;
#endif
TTmkEventData tmkEvD;
unsigned short mtbuffer[64];
int i, mtMaxBase, mtLastBase, base, fRun, res;
unsigned sw;


void main(int argc, char *argv[])
{
  if (argc == 1 || sscanf(argv[1], "%d", &mt) != 1)
    mt = 0;
  printf("mt: %d\n", mt);

  if (TmkOpen())
  {
    printf("TmkOpen error\n");
    goto stop;
  }
#ifdef _TMK1553B_WINDOWS
  hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (!hEvent)
  {
    printf("CreateEvent error\n");
    goto stop;
  }
#endif
  if (tmkconfig(mt))
  {
    printf("tmkconfig error\n");
    goto stop;
  }
#ifdef _TMK1553B_WINDOWS
  tmkdefevent(hEvent, TRUE);
#endif
  if (tmkselect(mt))
  {
    printf("tmkselect error\n");
    goto stop;
  }
  if (mtreset())
  {
    printf("mtreset error\n");
    goto stop;
  }
  printf("maxbase=%d\n", mtgetmaxbase());
  mtMaxBase = 63;
  for (i = 0; i < mtMaxBase; ++i)
  {
    mtdefbase(i);
    mtdeflink(i+1, CX_CONT|CX_NOINT|CX_SIG);
  }
  mtdefbase(i);
  mtdeflink(0, CX_CONT|CX_NOINT|CX_SIG);

  tmkselect(0);
  mtLastBase = 0;
  mtstartx(0, CX_CONT|CX_NOINT|CX_NOSIG);

  fRun = 1;
  while (fRun)
  {
#ifdef _TMK1553B_WINDOWS
    res = WaitForSingleObject(hEvent, 100);
#endif
#ifdef _TMK1553B_LINUX
    res = tmkwaitevents(1<<mt, 1000) >> mt;
#endif
    switch (res)
    {
    case WAIT_OBJECT_0:
#ifdef _TMK1553B_WINDOWS
      ResetEvent(hEvent);
#endif
#ifdef _TMK1553B_LINUX
      timeout = 0;
#endif
      tmkselect(0);
      while (tmkgetevd(&tmkEvD), tmkEvD.nInt)
      {
//        if (tmkEvD.nInt == 4)
        {
          printf("int = %d\n", tmkEvD.nInt);
          base = tmkEvD.mt.wBase;
          while (mtLastBase != base)
          {
            mtdefbase(mtLastBase);
            sw = mtgetsw();
            printf("mtsw = %04X", sw);
            mtgetblk(0, mtbuffer, 64);
            for (i = 0; i <= 63; ++i)
            {
              if ((i % 8) == 0)
                printf("\n");
              printf(" %04X", mtbuffer[i]);
            }
            printf("\n");
            ++mtLastBase;
            if (mtLastBase > mtMaxBase)
              mtLastBase = 0;
          }
        }
      }
      break;
    case WAIT_TIMEOUT:
#ifdef _TMK1553B_WINDOWS
      if (kbhit())
      {
#endif
#ifdef _TMK1553B_LINUX
      if (timeout == 0)
      {
            printf("timeout\n");
	    timeout = 1;
#endif

            mtdefbase(mtLastBase);
            sw = mtgetsw();
            printf("mtsw = %04X", sw);
            mtgetblk(0, mtbuffer, 64);
            for (i = 0; i <= 63; ++i)
            {
              if ((i % 8) == 0)
                printf("\n");
              printf(" %04X", mtbuffer[i]);
            }
            printf("\n");
#ifdef _TMK1553B_WINDOWS
        getch();
        fRun = 0;
#endif
      }
      break;
    default:
      fRun = 0;
      break;
    }
  }
  tmkselect(0);
  bcreset();
  stop:
  tmkdone(ALL_TMKS);
  TmkClose();
#ifdef _TMK1553B_WINDOWS
  if (hEvent)
    CloseHandle(hEvent);
#endif
}
