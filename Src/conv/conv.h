#ifndef _SMSTW_CONV
#define _SMSTW_CONV

/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ      -1
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)  (-2-(n))

typedef struct {
  unsigned short indx; /* index into big table */
  unsigned short used; /* bitmask of used entries */
} Summary16;

/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI      -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -2

struct conv_struct;
typedef struct conv_struct * conv_t;
typedef void* iconv_t;

struct loop_funcs {
  UInt32 (*loop_convert) (iconv_t icd,
                          const char* * inbuf, UInt32 *inbytesleft,
                          char* * outbuf, UInt32 *outbytesleft);
  UInt32 (*loop_reset) (iconv_t icd,
                        char* * outbuf, UInt32 *outbytesleft);
};

struct mbtowc_funcs {
  int (*xxx_mbtowc) (conv_t conv, UInt32 *pwc, unsigned char const *s, int n);
  /*
   * int xxx_mbtowc (conv_t conv, ucs4_t *pwc, unsigned char const *s, int n)
   * converts the byte sequence starting at s to a wide character. Up to n bytes
   * are available at s. n is >= 1.
   * Result is number of bytes consumed (if a wide character was read),
   * or -1 if invalid, or -2 if n too small, or -2-(number of bytes consumed)
   * if only a shift sequence was read.
   */
  int (*xxx_flushwc) (conv_t conv, UInt32 *pwc);
  /*
   * int xxx_flushwc (conv_t conv, ucs4_t *pwc)
   * returns to the initial state and stores the pending wide character, if any.
   * Result is 1 (if a wide character was read) or 0 if none was pending.
   */
};

struct wctomb_funcs {
  int (*xxx_wctomb) (conv_t conv, unsigned char *r, UInt32 wc, int n);
  /*
   * int xxx_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n)
   * converts the wide character wc to the character set xxx, and stores the
   * result beginning at r. Up to n bytes may be written at r. n is >= 1.
   * Result is number of bytes written, or -1 if invalid, or -2 in too small.
   */
  int (*xxx_reset) (conv_t conv, unsigned char *r, int n);
  /*
   * int xxx_reset (conv_t conv, unsigned char *r, int n)
   * stores a shift sequences returning to the initial state beginning at r.
   * Up to n bytes may be written at r. n is >= 0.
   * Result is number of bytes written, or -2 if n too small.
   */
};

struct conv_struct {
  struct loop_funcs lfuncs;
  /* Input (conversion multibyte -> unicode) */
  int iindex;
  struct mbtowc_funcs ifuncs;
  UInt32 istate;
  /* Output (conversion unicode -> multibyte) */
  int oindex;
  struct wctomb_funcs ofuncs;
  int oflags;
  UInt32 ostate;
  /* Operation flags */
  int transliterate;
  int discard_ilseq;
};

typedef UInt32 ucs4_t;
typedef UInt32 state_t;

struct gb2312_table {
	unsigned short* gb2312_2uni_page21;
	unsigned short* gb2312_2uni_page30;
	unsigned short* gb2312_2charset;
	Summary16* gb2312_uni2indx_page00;
	Summary16* gb2312_uni2indx_page20;
	Summary16* gb2312_uni2indx_page30;
	Summary16* gb2312_uni2indx_page4e;
	Summary16* gb2312_uni2indx_page9e;
	Summary16* gb2312_uni2indx_pageff;
	MemHandle memHandle[9];
};

UInt32 ucs2_to_gb2312(gb2312_table& table, const unsigned char* ucs2_buf, unsigned char* gb_buf, UInt32 len);
UInt32 gb2312_to_ucs2(gb2312_table& table, const unsigned char* gb_buf, unsigned char* ucs2_buf, UInt32 len);
UInt32 buffer_to_gb2312(gb2312_table& table, const unsigned char* buf, unsigned char* gb_buf, UInt32 len, Int8 coding);
DmOpenRef init_gb2312_table(gb2312_table& table);
void fini_gb2312_table(DmOpenRef db, gb2312_table& table);

#endif