
/****************************************************************************/
/*      TMKTEST v4.02                                                       */
/*      ELCUS, 1998,2008.                                                   */
/****************************************************************************/

#define TMK_RAM_TEST_BLK
#define TMK_DEFAULT_MAX_IO_DELAY 50

#define BC_RAM_ERROR 40
#define RT_RAM_ERROR 41

#ifndef RT_GOOD
#define RT_GOOD 0
#endif
#ifndef RT_BAD_RAM
#define RT_BAD_RAM 1
#endif

unsigned short _wRand;
int _fRand;

void srand(unsigned short wRand0)
{
  _wRand = wRand0;
  _fRand = 1;
}

unsigned short rand(void)
{
  ++_wRand;
  _fRand = !_fRand;
  if (_fRand)
    return (_wRand&0xFF)+(_wRand<<8);
  else
    return (~_wRand&0xFF)+(~_wRand<<8);
}


#ifdef TMK_RAM_TEST_BLK

unsigned short TMK_RAMTestBuf[64];

int BC_RAMTest(void)
{
  unsigned short wMaxBase;
  int nResult;
  register unsigned short wBase;
  register unsigned short wAddr;

  wMaxBase = bcgetmaxbase();
  srand( 1 );
  for ( wBase = 0; wBase <= wMaxBase; ++wBase )
  {
    for ( wAddr = 0; wAddr <= 63; ++wAddr )
      TMK_RAMTestBuf[wAddr] = rand();
    bcdefbase( wBase );
    bcputblk( 0, TMK_RAMTestBuf, 64 );
  }
  nResult = 0;
  srand( 1 );
  for ( wBase = 0; !nResult && wBase <= wMaxBase; ++wBase )
  {
    bcdefbase( wBase );
    bcgetblk( 0, TMK_RAMTestBuf, 64 );
    for ( wAddr = 0; wAddr <= 63; ++wAddr )
      if ( TMK_RAMTestBuf[wAddr] != rand() )
      {
        nResult = BC_RAM_ERROR;
        break;
      }
  }
  return nResult;
}

#else

int BC_RAMTest()
{
  unsigned short wMaxBase;
  int nResult;
  register unsigned short wBase;
  register unsigned short wAddr;

  wMaxBase = bcgetmaxbase();
  srand( 1 );
  for ( wBase = 0; wBase <= wMaxBase; ++wBase )
  {
    bcdefbase( wBase );
    for ( wAddr = 0; wAddr <= 63; ++wAddr )
      bcputw( wAddr, rand() );
  }
  nResult = 0;
  srand( 1 );
  for ( wBase = 0; !nResult && wBase <= wMaxBase; ++wBase )
  {
    bcdefbase( wBase );
    for ( wAddr = 0; wAddr <= 63; ++wAddr )
      if ( bcgetw( wAddr ) != rand() )
      {
        nResult = BC_RAM_ERROR;
        break;
      }
  }
  return nResult;
}

#endif /*def TMK_RAM_TEST_BLOCK*/

int RT_RAMTest(void)
{
  unsigned short wMaxPage, wPage;
  int nResult;
  register unsigned short wSubAddr;
  register unsigned short wAddr;

  wMaxPage = rtgetmaxpage();
  srand( 1 );
  for ( wPage = 0; wPage <= wMaxPage; ++wPage )
  {
    rtdefpage( wPage );
    for ( wSubAddr = 0; wSubAddr <= 31; ++wSubAddr )
    {
      rtdefsubaddr( RT_RECEIVE, wSubAddr );
      for ( wAddr = 0; wAddr <= 31; ++wAddr )
        rtputw( wAddr, rand() );
      rtdefsubaddr( RT_TRANSMIT, wSubAddr );
      for ( wAddr = 0; wAddr <= 31; ++wAddr )
        rtputw( wAddr, rand() );
    }
  }
  nResult = 0;
  srand( 1 );
  for ( wPage = 0; !nResult && wPage <= wMaxPage; ++wPage )
  {
    rtdefpage( wPage );
    for ( wSubAddr = 0; wSubAddr <= 31; ++wSubAddr )
    {
      rtdefsubaddr( RT_RECEIVE, wSubAddr );
      for ( wAddr = 0; wAddr <= 31; ++wAddr )
        if ( rtgetw( wAddr ) != rand() )
        {
          nResult = RT_RAM_ERROR;
          break;
        }
      rtdefsubaddr( RT_TRANSMIT, wSubAddr );
      for ( wAddr = 0; wAddr <= 31; ++wAddr )
        if ( rtgetw( wAddr ) != rand() )
        {
          nResult = RT_RAM_ERROR;
          break;
        }
    }
  }
  return nResult;
}

int RT_Test(void)
{
  int nResult;

  rtsetanswbits( BUSY );
  if ( ( nResult = RT_RAMTest() ) != 0 )
  {
    rtputcmddata( 0x413, RT_BAD_RAM );
    rtsetanswbits( RTFL );
  }
  else
  {
    rtputcmddata( 0x413, RT_GOOD );
    rtclranswbits( RTFL );
  }
  rtclranswbits( BUSY );
  return nResult;
}

int TMK_TuneIODelay( unsigned short nMaxIODelay )
{
  int fLastError, fErrors;
  unsigned short nIODelay, nSaveIODelay;

  if ( nMaxIODelay == 0 )
    nMaxIODelay = TMK_DEFAULT_MAX_IO_DELAY;
  nIODelay = nSaveIODelay = tmkiodelay(GET_IO_DELAY);
  fErrors = 0;
  fLastError = 0;
  do
  {
    tmkiodelay(nIODelay);
    bcreset();
    if ( ( fLastError = BC_RAMTest() ) != 0 )
    {
      fErrors = 1;
      ++nIODelay;
    }
    else
    {
      if (!fErrors)
        --nIODelay;
      else
        break;
    }
  }
  while ( nIODelay > 0 && nIODelay <= nMaxIODelay );
  if (fLastError)
    tmkiodelay(nSaveIODelay);
  else
    tmkiodelay((unsigned short)(nIODelay+1));
  return fLastError;
}
