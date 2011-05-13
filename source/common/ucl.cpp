#include <windows.h>
#include "ucl.h"

#define SWD_N                   (8 * 1024 * 1024ul) /* max. size of ring buffer */
#define SWD_F                   2048                /* upper limit for match length */
#define HEAD2(b, p)             (*(LPWORD)(&(b[p])))
#define NIL2                    MAXDWORD
#define SWD_HMASK               65535
#define SWD_HSIZE               (SWD_HMASK + 1)
#define HEAD3(b, p)             (((0x9F5F * (((((DWORD)b[p] << 5) ^ b[p + 1]) << 5) ^ b[p + 2])) >> 5) & SWD_HMASK)

typedef struct
{
  DWORD dwFlags;
  DWORD dwM2MaxOffset;

  DWORD look;          /* bytes pSource lookahead buffer */

  DWORD m_len;
  DWORD m_off;

  DWORD last_m_off;

  LPBYTE bp;
  LPBYTE ip;
  LPBYTE pSource;
  LPBYTE in_end;

  DWORD bb_b;
  DWORD bb_k;
  LPBYTE bb_p;
  LPBYTE bb_op;
  LPBYTE bb_op_end;

  UCL::PROGRESS_CALLBACK *pCallback;
  DWORD textsize;      /* text size counter */
  DWORD codesize;      /* code size counter */
  DWORD printcount;    /* counter for reporting progress every 1K bytes */
}UCL_COMPRESS_DATA;

typedef struct
{
  /* public - "built-pSource" */
  DWORD n;

  /* public - configuration */
  DWORD max_chain;
  DWORD nice_length;

  /* public - output */
  DWORD m_len;
  DWORD m_off;
  DWORD look;
  int b_char;

  /* semi public */
  UCL_COMPRESS_DATA *c;
  DWORD m_pos;

  /* private */
  LPBYTE dict;
  DWORD dict_len;

  /* private */
  DWORD ip;                /* input pointer (lookahead) */
  DWORD bp;                /* buffer pointer */
  DWORD rp;                /* remove pointer */
  DWORD b_size;

  LPBYTE b_wrap;

  DWORD node_count;
  DWORD first_rp;

  LPBYTE b;
  LPDWORD head3;
  LPDWORD succ3;
  LPDWORD best3;
  LPDWORD llen3;
  LPDWORD head2;
}ucl_swd_t;

typedef struct
{
  BYTE try_lazy;
  BYTE flags;
  WORD good_length;
  WORD max_lazy;
  WORD nice_length;
  WORD max_chain;  
  DWORD max_offset;
}SWD_CONFIG;

static const SWD_CONFIG swd_configs[] =
{
#if(XLIB_UCL_ONLY_MAX_COMPRESSION > 0)
#else
  /* faster compression */
  {0, 0,     0,     0,     8,    4,  48 * 1024L},
  {0, 0,     0,     0,    16,    8,  48 * 1024L},
  {0, 0,     0,     0,    32,   16,  48 * 1024L},
  {1, 0,     4,     4,    16,   16,  48 * 1024L},
  {1, 0,     8,    16,    32,   32,  48 * 1024L},
  {1, 0,     8,    16,   128,  128,  48 * 1024L},
  {2, 0,     8,    32,   128,  256, 128 * 1024L},
  {2, 1,    32,   128, SWD_F, 2048, 128 * 1024L},
  {2, 1,    32,   128, SWD_F, 2048, 256 * 1024L},
#endif
  {2, 1, SWD_F, SWD_F, SWD_F, 4096, SWD_N}
  /* max. compression */
};

static __inline void swd_initdict(ucl_swd_t *s, LPBYTE dict, DWORD dict_len)
{
  s->dict = NULL;
  s->dict_len = 0;

  if(dict && dict_len > 0)
  {
    if(dict_len > s->n)
    {
      dict += dict_len - s->n;
      dict_len = s->n;
    }

    s->dict = dict;
    s->dict_len = dict_len;
    Mem::_copy(s->b, dict, dict_len);
    s->ip = dict_len;
  }
}

static __inline void swd_insertdict(ucl_swd_t *s, DWORD node, DWORD len)
{
  DWORD key;

  s->node_count = s->n - len;
  s->first_rp = node;

  while(len-- > 0)
  {
    key = HEAD3(s->b, node);
    s->succ3[node] = s->head3[key];
    s->head3[key]  = node;
    s->best3[node] = SWD_F + 1;
    s->llen3[key]++;
    
    key = HEAD2(s->b, node);
    s->head2[key] = node;

    node++;
  }
}

static __inline int swd_init(ucl_swd_t *s, LPBYTE dict, DWORD dict_len)
{
  s->b = NULL;
  s->head3 = NULL;
  s->succ3 = NULL;
  s->best3 = NULL;
  s->llen3 = NULL;
  s->head2 = NULL;
  
  s->b     = (LPBYTE) Mem::alloc(s->n + SWD_F + SWD_F);
  s->head3 = (LPDWORD)Mem::alloc(SWD_HSIZE * sizeof(*s->head3));
  s->succ3 = (LPDWORD)Mem::alloc((s->n + SWD_F) * sizeof(*s->succ3));
  s->best3 = (LPDWORD)Mem::alloc((s->n + SWD_F) * sizeof(*s->best3));
  s->llen3 = (LPDWORD)Mem::alloc(SWD_HSIZE * sizeof(*s->llen3));
  s->head2 = (LPDWORD)Mem::alloc(65536 * sizeof(*s->head2));
  if(!s->b || !s->head3  || !s->succ3 || !s->best3 || !s->llen3 || !s->head2)return UCL::E_OUT_OF_MEMORY;

  /* defaults */
  s->max_chain    = 2048;
  s->nice_length  = SWD_F;

  s->b_size = s->n + SWD_F;
  if(s->b_size + SWD_F >= MAXDWORD)return UCL::E_ERROR;
  s->b_wrap = s->b + s->b_size;
  s->node_count = s->n;

  Mem::_zero(s->llen3,      sizeof(s->llen3[0]) * SWD_HSIZE);
  Mem::_set(s->head2, (char)0xFF, sizeof(s->head2[0]) * 65536);

  s->ip = 0;
  swd_initdict(s, dict, dict_len);
  s->bp = s->ip;
  s->first_rp = s->ip;

  s->look = (DWORD)(s->c->in_end - s->c->ip);
  if(s->look > 0)
  {
    if(s->look > SWD_F)s->look = SWD_F;
    Mem::_copy(&s->b[s->ip],s->c->ip,s->look);
    s->c->ip += s->look;
    s->ip += s->look;
  }
  
  if (s->ip == s->b_size)s->ip = 0;
  if (s->look >= 2 && s->dict_len > 0)swd_insertdict(s, 0, s->dict_len);

  s->rp = s->first_rp;
  if(s->rp >= s->node_count)s->rp -= s->node_count;
  else s->rp += s->b_size - s->node_count;

  return UCL::E_SUCCESSED;
}

static void swd_exit(ucl_swd_t *s)
{
  Mem::free(s->head2); s->head2 = NULL;
  Mem::free(s->llen3); s->llen3 = NULL;
  Mem::free(s->best3); s->best3 = NULL;
  Mem::free(s->succ3); s->succ3 = NULL;
  Mem::free(s->head3); s->head3 = NULL;
  Mem::free(s->b); s->b = NULL;
}

static __inline int init_match(UCL_COMPRESS_DATA *c, ucl_swd_t *s, LPBYTE dict, DWORD dict_len, BYTE flags)
{
  s->c = c;
  c->last_m_off = 0;
  c->textsize = c->codesize = c->printcount = 0;
  int r = swd_init(s, dict, dict_len);
  if(r != UCL::E_SUCCESSED)
  {
    swd_exit(s);
    return r;
  }

  return UCL::E_SUCCESSED;
}

static __inline int swd_search2(ucl_swd_t *s)
{
  DWORD key;

  key = s->head2[HEAD2(s->b, s->bp)];
  if(key == NIL2)return 0;

  if(s->m_len < 2)
  {
    s->m_len = 2;
    s->m_pos = key;
  }
  return 1;
}

static __inline void swd_search(ucl_swd_t *s, DWORD node, DWORD cnt)
{
  LPBYTE p1;
  LPBYTE p2;
  LPBYTE px;
  DWORD m_len = s->m_len;
  LPBYTE b  = s->b;
  LPBYTE bp = s->b + s->bp;
  LPBYTE bx = s->b + s->bp + s->look;
  BYTE scan_end1 = bp[m_len - 1];

  for(; cnt-- > 0; node = s->succ3[node])
  {
    p1 = bp;
    p2 = b + node;
    px = bx;

    if(p2[m_len - 1] == scan_end1 && p2[m_len] == p1[m_len] && p2[0] == p1[0] && p2[1] == p1[1])
    {
      DWORD i;

      p1 += 2; p2 += 2;
      while(++p1 < px && *p1 == *++p2);
      i = (DWORD) (p1 - bp);

      if(i > m_len)
      {
        s->m_len = m_len = i;
        s->m_pos = node;
        if(m_len == s->look || m_len >= s->nice_length || m_len > (DWORD) s->best3[node])break;
        scan_end1 = bp[m_len - 1];
      }
    }
  }
}

static void swd_remove_node(ucl_swd_t *s, DWORD node)
{
  if(s->node_count == 0)
  {
    DWORD key = HEAD3(s->b, node);
    --s->llen3[key];
    key = HEAD2(s->b,node);
    if((DWORD) s->head2[key] == node)s->head2[key] = NIL2;
  }
  else --s->node_count;
}

static void swd_findbest(ucl_swd_t *s)
{
  DWORD key;
  DWORD cnt, node;
  DWORD len;

  /* get current head, add bp into HEAD3 */
  key = HEAD3(s->b,s->bp);
  node = s->succ3[s->bp] = s->head3[key];
  cnt = s->llen3[key]++;
  if(cnt > s->max_chain && s->max_chain > 0)cnt = s->max_chain;
  s->head3[key] = s->bp;

  s->b_char = s->b[s->bp];
  len = s->m_len;
  if(s->m_len >= s->look)
  {
    if (s->look == 0)s->b_char = -1;
    s->m_off = 0;
    s->best3[s->bp] = SWD_F + 1;
  }
  else
  {
    if(swd_search2(s) && s->look >= 3)swd_search(s,node,cnt);
    if(s->m_len > len)s->m_off = (s->bp > s->m_pos ? s->bp - s->m_pos : s->b_size - (s->m_pos - s->bp));
    s->best3[s->bp] = s->m_len;
  }

  swd_remove_node(s,s->rp);

  /* add bp into HEAD2 */
  key = HEAD2(s->b,s->bp);
  s->head2[key] = s->bp;
}

static void swd_getbyte(ucl_swd_t *s)
{
  int c;
  if((c = (s->c->ip < s->c->in_end ? *(s->c->ip)++ : (-1))) < 0)
  {
    if (s->look > 0)--s->look;
  }
  else
  {
    s->b[s->ip] = (BYTE)c;
    if(s->ip < SWD_F)s->b_wrap[s->ip] = (BYTE)c;
  }
  if(++s->ip == s->b_size)s->ip = 0;
  if(++s->bp == s->b_size)s->bp = 0;
  if(++s->rp == s->b_size)s->rp = 0;
}

static __inline void swd_accept(ucl_swd_t *s, DWORD n)
{
  if(n > 0)do
  {
    DWORD key;

    swd_remove_node(s, s->rp);

    /* add bp into HEAD3 */
    key = HEAD3(s->b,s->bp);
    
    s->succ3[s->bp] = s->head3[key];
    s->head3[key]   = s->bp;
    s->best3[s->bp] = SWD_F + 1;
    s->llen3[key]++;

    key = HEAD2(s->b, s->bp);
    s->head2[key] = s->bp;

    swd_getbyte(s);
  }
  while(--n > 0);
}

static void find_match(UCL_COMPRESS_DATA *c, ucl_swd_t *s, DWORD this_len, DWORD skip)
{
  if(skip > 0)
  {
    swd_accept(s, this_len - skip);
    c->textsize += this_len - skip + 1;
  }
  else c->textsize += this_len - skip;

  s->m_len = 1;
  swd_findbest(s);
  c->m_len = s->m_len;
  c->m_off = s->m_off;

  swd_getbyte(s);

  if(s->b_char < 0)
  {
    c->look = 0;
    c->m_len = 0;
    swd_exit(s);
  }
  else c->look = s->look + 1;
  c->bp = c->ip - c->look;

  if(c->pCallback && c->textsize > c->printcount)
  {
    (*c->pCallback->callback)(c->textsize, c->codesize, 3, c->pCallback->pData);
    c->printcount += 1024;
  }
}

static int len_of_coded_match(UCL_COMPRESS_DATA *c, DWORD m_len, DWORD m_off)
{
  int b;
  if(m_len < 2 || (m_len == 2 && (m_off > c->dwM2MaxOffset)))return -1;

  m_len = m_len - 2 - (m_off > c->dwM2MaxOffset);

  if(m_off == c->last_m_off)b = 1 + 2;
  else
  {
#if(XLIB_UCL_ENABLE_NRV2B > 0)
  #if(XLIB_UCL_ENABLE_NRV2D > 0 || XLIB_UCL_ENABLE_NRV2E > 0)
    if(c->dwFlags & UCL::CF_NRV2B)
  #endif
    {
      b = 1 + 10;
      m_off = (m_off - 1) >> 8;
      while(m_off > 0)
      {
        b += 2;
        m_off >>= 1;
      }
    }
#endif
#if(XLIB_UCL_ENABLE_NRV2D > 0 || XLIB_UCL_ENABLE_NRV2E > 0)
  #if(XLIB_UCL_ENABLE_NRV2B > 0)
    else
  #endif
    {
      b = 1 + 9;
      m_off = (m_off - 1) >> 7;
      while(m_off > 0)
      {
        b += 3;
        m_off >>= 2;
      }
    }
#endif
  }
#if(XLIB_UCL_ENABLE_NRV2B > 0 || XLIB_UCL_ENABLE_NRV2D > 0)
  #if(XLIB_UCL_ENABLE_NRV2E > 0)
  if(c->dwFlags & (UCL::CF_NRV2B | UCL::CF_NRV2D))
  #endif
  {
    b += 2;
    if(m_len < 3)return b;
    m_len -= 3;
  }
#endif
#if(XLIB_UCL_ENABLE_NRV2E > 0)
  #if(XLIB_UCL_ENABLE_NRV2B > 0 || XLIB_UCL_ENABLE_NRV2D > 0)
  else
  #endif
  {
    b += 2;
    if(m_len < 2)return b;
    if(m_len < 4)return b + 1;
    m_len -= 4;
  }
#endif

  do
  {
    b += 2;
    m_len >>= 1;
  }
  while(m_len > 0);

  return b;
}

static void bbWriteBits(UCL_COMPRESS_DATA *c)
{
  LPBYTE p = c->bb_p;
  DWORD b = c->bb_b;

  p[0] = (BYTE)b;
  p[1] = (BYTE)(b >>  8);
  p[2] = (BYTE)(b >> 16);
  p[3] = (BYTE)(b >> 24);
}

static void bbPutBit(UCL_COMPRESS_DATA *c, unsigned bit)
{
  if(c->bb_k < 32)
  {
    if(c->bb_k == 0)
    {
      c->bb_p = c->bb_op;
      c->bb_op += 4;
    }

    c->bb_b = (c->bb_b << 1) + bit;
    c->bb_k++;
  }
  else
  {
    bbWriteBits(c);
    
    c->bb_p = c->bb_op;
    c->bb_op += 4;
    c->bb_b = bit;
    c->bb_k = 1;
  }
}

static bool bbPutByte(UCL_COMPRESS_DATA *c, unsigned b)
{
  if(c->bb_op < c->bb_op_end)
  {
    *c->bb_op++ = (BYTE)b;
    return true;
  }
  return false;
}

static bool code_run(UCL_COMPRESS_DATA *c, LPBYTE ii, DWORD lit)
{
  if(lit != 0)do
  {
    bbPutBit(c, 1);
    if(!bbPutByte(c, *ii++))return false;
  }
  while(--lit > 0);
  return true;
}

static void code_prefix_ss11(UCL_COMPRESS_DATA *c, DWORD i)
{
  if(i >= 2)
  {
    DWORD t = 4;
    i += 2;
    do
    {
      t <<= 1;
    }
    while (i >= t);
    
    t >>= 1;
    
    do
    {
      t >>= 1;
      bbPutBit(c, (i & t) ? 1 : 0);
      bbPutBit(c, 0);
    }
    while(t > 2);
  }
  bbPutBit(c, i & 1);
  bbPutBit(c, 1);
}

static void code_prefix_ss12(UCL_COMPRESS_DATA *c, DWORD i)
{
  if(i >= 2)
  {
    DWORD t = 2;
    do
    {
      i -= t;
      t <<= 2;
    }
    while(i >= t);
    
    do
    {
      t >>= 1;
      bbPutBit(c, (i & t) ? 1 : 0);
      bbPutBit(c, 0);
      t >>= 1;
      bbPutBit(c, (i & t) ? 1 : 0);
    }
    while(t > 2);
  }
  bbPutBit(c, i & 1);
  bbPutBit(c, 1);
}

static __inline bool code_match(UCL_COMPRESS_DATA *c, DWORD m_len, DWORD m_off)
{
  DWORD m_low = 0;

  bbPutBit(c, 0);

#if(XLIB_UCL_ENABLE_NRV2B > 0)
  #if(XLIB_UCL_ENABLE_NRV2D > 0 || XLIB_UCL_ENABLE_NRV2E > 0) 
  if(c->dwFlags & UCL::CF_NRV2B)
  #endif
  {
    if(m_off == c->last_m_off)
    {
      bbPutBit(c, 0);
      bbPutBit(c, 1);
    }
    else
    {
      code_prefix_ss11(c, 1 + ((m_off - 1) >> 8));
      if(!bbPutByte(c, m_off - 1))return false;
    }
    m_len = m_len - 1 - (m_off > c->dwM2MaxOffset);
    if(m_len >= 4)
    {
      bbPutBit(c,0);
      bbPutBit(c,0);
      code_prefix_ss11(c, m_len - 4);
    }
    else
    {
      bbPutBit(c, m_len > 1);
      bbPutBit(c, m_len & 1);
    }
  }
#endif
#if(XLIB_UCL_ENABLE_NRV2D > 0)  
  #if(XLIB_UCL_ENABLE_NRV2B > 0) 
  else 
  #endif
  #if(XLIB_UCL_ENABLE_NRV2B > 0 || XLIB_UCL_ENABLE_NRV2E > 0) 
  if(c->dwFlags & UCL::CF_NRV2D)
  #endif
  {
    m_len = m_len - 1 - (m_off > c->dwM2MaxOffset);
    m_low = (m_len >= 4) ? 0 : m_len;
    if(m_off == c->last_m_off)
    {
      bbPutBit(c, 0);
      bbPutBit(c, 1);
      bbPutBit(c, m_low > 1);
      bbPutBit(c, m_low & 1);
    }
    else
    {   
      code_prefix_ss12(c, 1 + ((m_off - 1) >> 7));
      if(!bbPutByte(c, (((m_off - 1) & 0x7f) << 1) | ((m_low > 1) ? 0 : 1)))return false;
      bbPutBit(c, m_low & 1);
    }
    if (m_len >= 4)code_prefix_ss11(c, m_len - 4);
  }
#endif
#if(XLIB_UCL_ENABLE_NRV2E > 0)
  #if(XLIB_UCL_ENABLE_NRV2B > 0 || XLIB_UCL_ENABLE_NRV2D > 0) 
  else /*if(c->dwFlags & UCL::CF_NRV2E)*/
  #endif
  {
    m_len = m_len - 1 - (m_off > c->dwM2MaxOffset);
    m_low = (m_len <= 2);
    if(m_off == c->last_m_off)
    {
      bbPutBit(c, 0);
      bbPutBit(c, 1);
      bbPutBit(c, m_low);
    }
    else
    {
      code_prefix_ss12(c, 1 + ((m_off - 1) >> 7));
      if(!bbPutByte(c, (((m_off - 1) & 0x7f) << 1) | (m_low ^ 1)))return false;
    }
    if(m_low)bbPutBit(c, m_len - 1);
    else if (m_len <= 4)
    {
      bbPutBit(c, 1);
      bbPutBit(c, m_len - 3);
    }
    else
    {
      bbPutBit(c, 0);
      code_prefix_ss11(c, m_len - 5);
    }
  }
#endif
  c->last_m_off = m_off;
  return true;
}

static __inline void bbFlushBits(UCL_COMPRESS_DATA *c, DWORD filler_bit)
{
  if (c->bb_k > 0)
  {
    while(c->bb_k != 32)bbPutBit(c, filler_bit);
    bbWriteBits(c);
    c->bb_k = 0;
  }
  c->bb_p = NULL;
}

void UCL::Init(void)
{

}

void UCL::Uninit(void)
{

}

int UCL::_Compress(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pBuffer, LPDWORD pdwBufferSize, PROGRESS_CALLBACK *pCallback, DWORD dwFlags)
{
  if(*pdwBufferSize == 0)return E_OUT_OF_BUFFER;

  DWORD m_len, m_off;
  UCL_COMPRESS_DATA c_buffer;
  UCL_COMPRESS_DATA * c = &c_buffer;
  ucl_swd_t the_swd;
  int iRetVal;
  SWD_CONFIG *sc;
  
  //Выбираем метод сжатия.
#if(XLIB_UCL_ONLY_MAX_COMPRESSION > 0)
  sc = (SWD_CONFIG *)&swd_configs[0];
#else
  {
    register BYTE i;
    if(dwFlags & CF_LEVEL_1)      i = 0;
    else if(dwFlags & CF_LEVEL_2) i = 1;
    else if(dwFlags & CF_LEVEL_3) i = 2;
    else if(dwFlags & CF_LEVEL_4) i = 3;
    else if(dwFlags & CF_LEVEL_5) i = 4;
    else if(dwFlags & CF_LEVEL_6) i = 5;
    else if(dwFlags & CF_LEVEL_7) i = 6;
    else if(dwFlags & CF_LEVEL_8) i = 7;
    else if(dwFlags & CF_LEVEL_9) i = 8;
    else if(dwFlags & CF_LEVEL_10)i = 9;
    else return UCL::E_INVALID_ARGUMENT;
    sc = (SWD_CONFIG *)&swd_configs[i];
  }
#endif
  
  Mem::_zero(c, sizeof(*c));

  c->dwM2MaxOffset = dwFlags & UCL::CF_NRV2B ? 0xD00 : 0x500;
  c->dwFlags       = dwFlags;
  c->ip            = c->pSource = pSource;
  c->in_end        = pSource + dwSourceSize;
  c->bb_op         = pBuffer;
  c->bb_op_end     = pBuffer + (*pdwBufferSize);
  c->pCallback     = pCallback;

  LPBYTE ii = c->ip;             /* point to start of literal run */
  DWORD lit = 0;

  the_swd.n = sc->max_offset;

  if(dwSourceSize < the_swd.n)the_swd.n = max(dwSourceSize, 256);
  if((iRetVal = init_match(c, &the_swd, NULL, 0, sc->flags)) != E_SUCCESSED)return iRetVal;
  /*if(sc->max_chain > 0)*/the_swd.max_chain = sc->max_chain;
  /*if(sc->nice_length > 0)*/the_swd.nice_length = sc->nice_length;
  if(c->pCallback)(*c->pCallback->callback)(0, 0, -1, c->pCallback->pData);

  c->last_m_off = 1;
  find_match(c, &the_swd, 0, 0);
  
  while(c->look > 0)
  {
    DWORD ahead;
    DWORD max_ahead;
    int l1, l2;

    c->codesize = (DWORD)(c->bb_op - pBuffer);

    m_len = c->m_len;
    m_off = c->m_off;

    if(lit == 0)ii = c->bp;
    if(m_len < 2 || (m_len == 2 && (m_off > c->dwM2MaxOffset)))
    {
      /* a literal */
      lit++;
      the_swd.max_chain = sc->max_chain;
      find_match(c, &the_swd, 1, 0);
      continue;
    }

    /* shall we try a lazy match ? */
    ahead = 0;
    if(sc->try_lazy <= 0 || m_len >= sc->max_lazy || m_off == c->last_m_off)
    {
      /* no */
      l1 = 0;
      max_ahead = 0;
    }
    else
    {
      /* yes, try a lazy match */
      l1 = len_of_coded_match(c,m_len,m_off);
      max_ahead = min(sc->try_lazy, m_len - 1);
    }

    while (ahead < max_ahead && c->look > m_len)
    {
      the_swd.max_chain = m_len >= sc->good_length ? sc->max_chain >> 2 : sc->max_chain;
      
      find_match(c,&the_swd,1,0);
      ahead++;

      if(c->m_len < 2)continue;
      l2 = len_of_coded_match(c, c->m_len, c->m_off);
      if(l2 < 0)continue;
      if(l1 + (int)(ahead + c->m_len - m_len) * 5 > l2 + (int)(ahead) * 9)
      {
        lit += ahead;
        goto LAZY_MATCH_DONE;
      }
    }

    /* 1 - code run */
    if(!code_run(c, ii, lit))
    {
      iRetVal = E_OUT_OF_BUFFER;
      goto END;
    }

    lit = 0;

    /* 2 - code match */
    if(!code_match(c, m_len, m_off))
    {
      iRetVal = E_OUT_OF_BUFFER;
      goto END;
    }

    the_swd.max_chain = sc->max_chain;
    find_match(c, &the_swd, m_len, 1 + ahead);

LAZY_MATCH_DONE:;
  }

  /* store final run */
  if(!code_run(c, ii, lit))
  {
    iRetVal = E_OUT_OF_BUFFER;
    goto END;
  }

  /* EOF */
  bbPutBit(c, 0);
  if(dwFlags & UCL::CF_NRV2B)code_prefix_ss11(c, 0x1000000);
  else code_prefix_ss12(c, 0x1000000);
  
  if(!bbPutByte(c, 0xFF))
  {
    iRetVal = E_OUT_OF_BUFFER;
    goto END;
  }
  bbFlushBits(c, 0);

  c->codesize = (DWORD)(c->bb_op - pBuffer);
  *pdwBufferSize = c->codesize;
  if(c->pCallback)(*c->pCallback->callback)(c->textsize, c->codesize, 4, c->pCallback->pData);

END:
  swd_exit(&the_swd);
  return iRetVal;
}

#define getbit(bb)  (bc > 0 ? ((bb >> --bc) & 1) : (bc = 31, bb = (*(LPDWORD)((pSource) + ilen)), ilen += 4,(bb >> 31) & 1))

bool UCL::_DecompressNRV2BSafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1, oend = *pdwDestSize;

  for(;;)
  {
    DWORD m_off, m_len;

    while(getbit(bb))
    {
      if(ilen >= dwSourceSize || olen >= oend)return false;
      pDest[olen++] = pSource[ilen++];
    }
    m_off = 1;
    do
    {
      m_off = m_off * 2 + getbit(bb);
      if(ilen >= dwSourceSize || m_off > 0xFFFFFF + 3)return false;
    }
    while(!getbit(bb));
    if(m_off == 2)m_off = last_m_off;
    else
    {
      if(ilen >= dwSourceSize)return false;
      m_off = (m_off-3)*256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      last_m_off = ++m_off;
    }
    m_len = getbit(bb);
    m_len = m_len * 2 + getbit(bb);
    if(m_len == 0)
    {
      m_len++;
      do
      {
        m_len = m_len * 2 + getbit(bb);
        if(ilen >= dwSourceSize || m_len >= oend)return false;
      }
      while(!getbit(bb));
      m_len += 2;
    }
    m_len += (m_off > 0xd00);
    if(olen + m_len > oend || m_off > olen)return false;
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}

bool UCL::_DecompressNRV2B(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1;

  for(;;)
  {
    DWORD m_off, m_len;

    while(getbit(bb))pDest[olen++] = pSource[ilen++];
    m_off = 1;
    do {m_off = m_off * 2 + getbit(bb);}while(!getbit(bb));
    if(m_off == 2)m_off = last_m_off;
    else
    {
      m_off = (m_off-3)*256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      last_m_off = ++m_off;
    }
    m_len = getbit(bb);
    m_len = m_len * 2 + getbit(bb);
    if(m_len == 0)
    {
      m_len++;
      do {m_len = m_len * 2 + getbit(bb);}while(!getbit(bb));
      m_len += 2;
    }
    m_len += (m_off > 0xD00);
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while(--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}

bool UCL::_DecompressNRV2DSafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1, oend = *pdwDestSize;
  
  for(;;)
  {
    DWORD m_off, m_len;

    while(getbit(bb))
    {
      if(ilen >= dwSourceSize || olen >= oend)return false;
      pDest[olen++] = pSource[ilen++];
    }
    m_off = 1;
    for(;;)
    {
      m_off = m_off * 2 + getbit(bb);
      if(ilen >= dwSourceSize || m_off > 0xFFFFFF + 3)return false;
      if(getbit(bb)) break;
      m_off = (m_off - 1) * 2 + getbit(bb);
    }
    if(m_off == 2)
    {
      m_off = last_m_off;
      m_len = getbit(bb);
    }
    else
    {
      if(ilen >= dwSourceSize)return false;
      m_off = (m_off - 3) * 256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      m_len = (m_off ^ 0xFFFFFFFF) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    m_len = m_len * 2 + getbit(bb);
    if(m_len == 0)
    {
      m_len++;
      do
      {
        m_len = m_len*2 + getbit(bb);
        if(ilen >= dwSourceSize || m_len >= oend)return false;
      }while(!getbit(bb));
      m_len += 2;
    }
    m_len += (m_off > 0x500);
    if(olen + m_len > oend || m_off > olen)return false;
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}

bool UCL::_DecompressNRV2D(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1;

  for(;;)
  {
    DWORD m_off, m_len;

    while(getbit(bb)){pDest[olen++] = pSource[ilen++];}
    m_off = 1;
    for(;;)
    {
      m_off = m_off * 2 + getbit(bb);
      if(getbit(bb))break;
      m_off = (m_off - 1) * 2 + getbit(bb);
    }
    if(m_off == 2)
    {
      m_off = last_m_off;
      m_len = getbit(bb);
    }
    else
    {
      m_off = (m_off - 3) * 256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      m_len = (m_off ^ 0xFFFFFFFF) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    m_len = m_len * 2 + getbit(bb);
    if(m_len == 0)
    {
      m_len++;
      do
      {
        m_len = m_len*2 + getbit(bb);
      }while(!getbit(bb));
      m_len += 2;
    }
    m_len += (m_off > 0x500);
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}

bool UCL::_DecompressNRV2ESafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1, oend = *pdwDestSize;

  for(;;)
  {
    DWORD m_off, m_len;

    while (getbit(bb))
    {
      if(ilen >= dwSourceSize || olen >= oend)return false;
      pDest[olen++] = pSource[ilen++];
    }
    m_off = 1;
    for(;;)
    {
      m_off = m_off*2 + getbit(bb);
      if(ilen >= dwSourceSize || m_off > 0xFFFFFF + 3)return false;
      if(getbit(bb)) break;
      m_off = (m_off - 1) * 2 + getbit(bb);
    }
    if(m_off == 2)
    {
      m_off = last_m_off;
      m_len = getbit(bb);
    }
    else
    {
      if(ilen >= dwSourceSize)return false;
      m_off = (m_off-3)*256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      m_len = (m_off ^ 0xFFFFFFFF) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    if (m_len)m_len = 1 + getbit(bb);
    else if (getbit(bb))m_len = 3 + getbit(bb);
    else
    {
      m_len++;
      do
      {
        m_len = m_len*2 + getbit(bb);
        if(ilen >= dwSourceSize || m_len >= oend)return false;
      }
      while(!getbit(bb));
      m_len += 3;
    }
    m_len += (m_off > 0x500);
    if(olen + m_len > oend || m_off > olen)return false;
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}

bool UCL::_DecompressNRV2E(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize)
{
  DWORD bb = 0, bc = 0, ilen = 0, olen = 0, last_m_off = 1;

  for(;;)
  {
    DWORD m_off, m_len;

    while(getbit(bb))pDest[olen++] = pSource[ilen++];
    m_off = 1;
    for(;;)
    {
      m_off = m_off*2 + getbit(bb);
      if(getbit(bb)) break;
      m_off = (m_off - 1) * 2 + getbit(bb);
    }
    if(m_off == 2)
    {
      m_off = last_m_off;
      m_len = getbit(bb);
    }
    else
    {
      m_off = (m_off-3)*256 + pSource[ilen++];
      if(m_off == 0xFFFFFFFF)break;
      m_len = (m_off ^ 0xFFFFFFFF) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    if (m_len)m_len = 1 + getbit(bb);
    else if (getbit(bb))m_len = 3 + getbit(bb);
    else
    {
      m_len++;
      do {m_len = m_len*2 + getbit(bb);}while(!getbit(bb));
      m_len += 3;
    }
    m_len += (m_off > 0x500);
    {
      LPBYTE m_pos;
      m_pos = pDest + olen - m_off;
      pDest[olen++] = *m_pos++;
      do pDest[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *pdwDestSize = olen;
  return ilen == dwSourceSize ? true : false;
}
