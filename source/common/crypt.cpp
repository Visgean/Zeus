#include <windows.h>
#include <wincrypt.h>

#include "crypt.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html
///////////////////////////////////////////////////////////////////////////////////////////////////
#if(1)
/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
#define MT_RAND_MAX ((long)(0x7FFFFFFF)) /* (1 << 31) - 1 */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti;
static DWORD _last_rand_tickcount;

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
  mt[0]= s & 0xffffffffUL;
  for (mti = 1; mti < N; mti++)
  {
    mt[mti] = (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
  unsigned long y;
  static unsigned long mag01[2] = {0x0UL, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if(mti >= N)/* generate N words at one time */
  {
    int kk;

    for(kk = 0; kk < N - M; kk++)
    {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for(; kk < N - 1; kk++)
    {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

    mti = 0;
  }

  y = mt[mti++];

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return y;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static bool crc32Intalized;

void Crypt::init(void)
{
  _last_rand_tickcount = 0;
  crc32Intalized = false;
}

void Crypt::uninit(void)
{

}

bool Crypt::_md5Hash(LPBYTE output, void *inputData, DWORD dataSize)
{
  bool r = false;
  HCRYPTPROV hashProv;

  if(CWA(advapi32, CryptAcquireContextW)(&hashProv, NULL, NULL, PROV_RSA_FULL /*Именно это значение, не какого иного.*/, CRYPT_VERIFYCONTEXT | CRYPT_SILENT) != FALSE)
  {
    HCRYPTHASH hashHandle;
    if(CWA(advapi32, CryptCreateHash)(hashProv, CALG_MD5, 0, 0, &hashHandle) == TRUE)
    {
      DWORD hashLen = MD5HASH_SIZE;
      if(CWA(advapi32, CryptHashData)(hashHandle, (LPBYTE)inputData, dataSize, 0) == TRUE && CWA(advapi32, CryptGetHashParam)(hashHandle, HP_HASHVAL, output, &hashLen, 0) == TRUE && hashLen == MD5HASH_SIZE)r = true;
      CWA(advapi32, CryptDestroyHash)(hashHandle);
    }
    CWA(advapi32, CryptReleaseContext)(hashProv, 0);
  }

  return r;
}

DWORD Crypt::mtRand(void)
{
  DWORD c = CWA(kernel32, GetTickCount)();
  if(c != _last_rand_tickcount)
  {
    _last_rand_tickcount = c;
    init_genrand(c);
  }

  return genrand_int32();
}

DWORD Crypt::mtRandRange(DWORD minVal, DWORD maxVal)
{
  if(maxVal == minVal)return maxVal;
  register DWORD r = mtRand();
  register DWORD x = maxVal - minVal + 1;
  return minVal + (r - (r / x) * x);
}

static DWORD crc32table[256];

DWORD Crypt::crc32Hash(const void *data, DWORD size)
{
  if(crc32Intalized == false)
  {
    register DWORD crc;
    for(register DWORD i = 0; i < 256; i++)
    {
      crc = i;
      for(register DWORD j = 8; j > 0; j--)
      {
        if(crc & 0x1)crc = (crc >> 1) ^ 0xEDB88320L;
        else crc >>= 1;
      }
      crc32table[i] = crc;
    }

    crc32Intalized = true;
  }

  register DWORD cc = 0xFFFFFFFF;
  for(register DWORD i = 0; i < size; i++)cc = (cc >> 8) ^ crc32table[(((LPBYTE)data)[i] ^ cc) & 0xFF];
  return ~cc;
}

#define swap_byte(a, b) {swapByte = a; a = b; b = swapByte;}

void Crypt::_rc4Init(const void *binKey, WORD binKeySize, RC4KEY *key)
{
  register BYTE swapByte;
  register BYTE index1 = 0, index2 = 0;
  LPBYTE state = &key->state[0];
  register WORD i;

  key->x = 0;
  key->y = 0;

  for(i = 0; i < 256; i++)state[i] = i;
  for(i = 0; i < 256; i++)
  {
    index2 = (((LPBYTE)binKey)[index1] + state[i] + index2) & 0xFF;
    swap_byte(state[i], state[index2]);
    if(++index1 == binKeySize)index1 = 0;
  }
}

void Crypt::_rc4(void *buffer, DWORD size, RC4KEY *key)
{
 
  register BYTE swapByte;
  register BYTE x = key->x;
  register BYTE y = key->y;
  LPBYTE state = &key->state[0];

  for(register DWORD i = 0; i < size; i++)
  {
    x = (x + 1) & 0xFF;
    y = (state[x] + y) & 0xFF;
    swap_byte(state[x], state[y]);
    ((LPBYTE)buffer)[i] ^= state[(state[x] + state[y]) & 0xFF];
  }

  key->x = x;
  key->y = y; 

}

void Crypt::_rc4Full(const void *binKey, WORD binKeySize, void *buffer, DWORD size)
{
  Crypt::RC4KEY key;
  Crypt::_rc4Init(binKey, binKeySize, &key);
  Crypt::_rc4(buffer, size, &key);
}

void Crypt::_visualEncrypt(void *buffer, DWORD size)
{
  for(DWORD i = 1; i < size; i++)((LPBYTE)buffer)[i] ^= ((LPBYTE)buffer)[i - 1];
}

void Crypt::_visualDecrypt(void *buffer, DWORD size)
{
  if(size > 0)for(DWORD i = size - 1; i > 0; i--)((LPBYTE)buffer)[i] ^= ((LPBYTE)buffer)[i - 1];
}

LPSTR Crypt::_base64Encode(LPBYTE source, SIZE_T sourceSize, SIZE_T *destSize)
{
  static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  LPBYTE dest = (LPBYTE)Mem::alloc((sourceSize + 2) / 3 * 4 + 1);
  if(dest != NULL)
  {
    LPBYTE p = dest;
    BYTE cur[3];

    while(sourceSize > 0)
    {
      DWORD len = 0;
      for(DWORD i = 0; i < 3; i++)
      {
        if(sourceSize > 0)
        {
          sourceSize--;
          len++;
          cur[i] = source[i];
        }
        else cur[i] = 0;
      }

      source += 3;

      p[0] = cb64[cur[0] >> 2];
      p[1] = cb64[((cur[0] & 0x03) << 4) | ((cur[1] & 0xF0) >> 4)];
      p[2] = (BYTE)(len > 1 ? cb64[((cur[1] & 0x0F) << 2) | ((cur[2] & 0xC0) >> 6) ] : '=');
      p[3] = (BYTE)(len > 2 ? cb64[cur[2] & 0x3F] : '=');

      p += 4;
    }

    *p = 0;
    if(destSize)*destSize = (SIZE_T)(p - dest);
  }

  return (LPSTR)dest;
}

LPBYTE Crypt::_base64Decode(LPSTR source, SIZE_T sourceSize, SIZE_T *destSize)
{
  static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

  LPBYTE dest = (LPBYTE)Mem::alloc(sourceSize + sizeof(BYTE));
  if(dest != NULL)
  {
    LPBYTE p = (LPBYTE)source;
    LPBYTE e = p + sourceSize;
    LPBYTE r = (LPBYTE)dest;

    BYTE in[4], out[3], v;
    int len, i;

    while(p < e)
    {
      for(len = 0, i = 0; i < 4 && p < e; i++)
      {
        v = 0;
        while(p < e && v == 0)
        {
          v = (BYTE)*(p++);
          v = (BYTE)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
          if(v != 0)v = (BYTE)((v == '$') ? 0 : v - 61);
        }

        if(v != 0)
        {
          len++;
          in[i] = (BYTE)(v - 1);
        }
      }

      if(len)
      {
        out[0] = (BYTE)(in[0] << 2 | in[1] >> 4);
        out[1] = (BYTE)(in[1] << 4 | in[2] >> 2);
        out[2] = (BYTE)(((in[2] << 6) & 0xC0) | in[3]);
        for(i = 0; i < len - 1; i++){*(r++) = out[i]; if(i==0)i=0;/*instrict*/}
      }
    }

    *r = 0;
    if(destSize)*destSize = (SIZE_T)(r - dest);
  }
  return dest;
}

void Crypt::_generateBinaryData(void *buffer, DWORD size, BYTE minValue, BYTE maxValue, bool extMode)
{
  for(DWORD i = 0; i < size; i++)
  {
    if(extMode && i > 0)CWA(kernel32, Sleep)(20);
    ((LPBYTE)buffer)[i] = (BYTE)Crypt::mtRandRange(minValue, maxValue);
  }
}

void Crypt::_generateRc4Key(Crypt::RC4KEY *key)
{
  BYTE keybuf[40];
  _generateBinaryData(keybuf, sizeof(keybuf), 0, 0xFF, true);
  _rc4Init(keybuf, sizeof(keybuf), key);
}
