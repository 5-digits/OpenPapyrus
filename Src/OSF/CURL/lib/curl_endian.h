#ifndef HEADER_CURL_ENDIAN_H
#define HEADER_CURL_ENDIAN_H
/***************************************************************************
 *                                _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                           / __| | | | |_) | |
 *                          | (__| |_| |  _ <| |___
 *                           \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

/* Converts a 16-bit integer from little endian */
ushort Curl_read16_le(const uchar *buf);

/* Converts a 32-bit integer from little endian */
uint Curl_read32_le(const uchar *buf);

#if (CURL_SIZEOF_CURL_OFF_T > 4)
/* Converts a 64-bit integer from little endian */
#if defined(HAVE_LONGLONG)
uint64 Curl_read64_le(const uchar *buf);
#else
uint64 Curl_read64_le(const uchar *buf);
#endif
#endif

/* Converts a 16-bit integer from big endian */
ushort Curl_read16_be(const uchar *buf);

/* Converts a 32-bit integer from big endian */
uint Curl_read32_be(const uchar *buf);

#if (CURL_SIZEOF_CURL_OFF_T > 4)
/* Converts a 64-bit integer from big endian */
#if defined(HAVE_LONGLONG)
uint64 Curl_read64_be(const uchar *buf);
#else
uint64 Curl_read64_be(const uchar *buf);
#endif
#endif

/* Converts a 16-bit integer to little endian */
void Curl_write16_le(const short value, uchar *buffer);

/* Converts a 32-bit integer to little endian */
void Curl_write32_le(const int value, uchar *buffer);

#if (CURL_SIZEOF_CURL_OFF_T > 4)
/* Converts a 64-bit integer to little endian */
#if defined(HAVE_LONGLONG)
void Curl_write64_le(const long long value, uchar *buffer);
#else
void Curl_write64_le(const __int64 value, uchar *buffer);
#endif
#endif

#endif /* HEADER_CURL_ENDIAN_H */
