/*
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

#include "conv.h"

/*
 * UCS-2
 */

/* Here we accept FFFE/FEFF marks as endianness indicators everywhere
   in the stream, not just at the beginning. The default is big-endian. */
/* The state is 0 if big-endian, 1 if little-endian. */
static int
ucs2_mbtowc (conv_t conv, UInt32 *pwc, const unsigned char *s, int n)
{
  UInt32 state = conv->istate;
  UInt32 s0;
  UInt32 s1;
  UInt32 wc;
  int count = 0;
  for (; n >= 2;) {
  	s0 = s[0];
  	s1 = s[1];
    wc = (state ? s0 + (s1 << 8) : (s0 << 8) + s1);
    s += 2; n -= 2; count += 2;
    if (wc == 0xfeff) {
    } else if (wc == 0xfffe) {
      state ^= 1;
    } else if (wc >= 0xd800 && wc < 0xe000) {
      return RET_ILSEQ;
    } else {
      *pwc = wc;
      conv->istate = state;
      return count;
    }
  }
  conv->istate = state;
  return RET_TOOFEW(count);
}

/* But we output UCS-2 in big-endian order, without byte-order mark. */
/* RFC 2152 says:
   "ISO/IEC 10646-1:1993(E) specifies that when characters the UCS-2 form are
    serialized as octets, that the most significant octet appear first." */
static int
ucs2_wctomb (conv_t conv, unsigned char *r, UInt32 wc, int n)
{
  if (wc < 0x10000 && wc != 0xfffe && !(wc >= 0xd800 && wc < 0xe000)) {
    if (n >= 2) {
      r[0] = (unsigned char) (wc >> 8);
      r[1] = (unsigned char) wc;
      return 2;
    } else
      return RET_TOOSMALL;
  } else
    return RET_ILUNI;
}
