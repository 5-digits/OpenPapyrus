#ifndef HEADER_CURL_NTLM_CORE_H
#define HEADER_CURL_NTLM_CORE_H
/***************************************************************************
 *                                _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                           / __| | | | |_) | |
 *                          | (__| |_| |  _ <| |___
 *                           \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
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

#include "curl_setup.h"

#if defined(USE_NTLM)

#if !defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO)

#ifdef USE_OPENSSL
	#if !defined(OPENSSL_VERSION_NUMBER) && !defined(HEADER_SSL_H) && !defined(HEADER_MD5_H)
		#error "curl_ntlm_core.h shall not be included before OpenSSL headers."
	#endif
#endif
/* Define USE_NTRESPONSES in order to make the type-3 message include the NT response message. */
#if !defined(USE_OPENSSL) || !defined(OPENSSL_NO_MD4)
	#define USE_NTRESPONSES
#endif
/* Define USE_NTLM2SESSION in order to make the type-3 message include the
   NTLM2Session response message, requires USE_NTRESPONSES defined to 1 and a
   Crypto engine that we have curl_ssl_md5sum() for. */
#if defined(USE_NTRESPONSES) && !defined(USE_WIN32_CRYPTO)
	#define USE_NTLM2SESSION
#endif

/* Define USE_NTLM_V2 in order to allow the type-3 message to include the
   LMv2 and NTLMv2 response messages, requires USE_NTRESPONSES defined to 1
   and support for 64-bit integers. */
#if defined(USE_NTRESPONSES) && (CURL_SIZEOF_CURL_OFF_T > 4)
#define USE_NTLM_V2
#endif

void Curl_ntlm_core_lm_resp(const uchar *keys,
                            const uchar *plaintext,
                            uchar *results);

CURLcode Curl_ntlm_core_mk_lm_hash(struct Curl_easy *data,
                                   const char *password,
                                   uchar *lmbuffer /* 21 bytes */);

#ifdef USE_NTRESPONSES
CURLcode Curl_ntlm_core_mk_nt_hash(struct Curl_easy *data,
                                   const char *password,
                                   uchar *ntbuffer /* 21 bytes */);

#if defined(USE_NTLM_V2) && !defined(USE_WINDOWS_SSPI)

CURLcode Curl_hmac_md5(const uchar *key, uint keylen,
                       const uchar *data, uint datalen,
                       uchar *output);

CURLcode Curl_ntlm_core_mk_ntlmv2_hash(const char *user, size_t userlen,
                                       const char *domain, size_t domlen,
                                       uchar *ntlmhash,
                                       uchar *ntlmv2hash);

CURLcode  Curl_ntlm_core_mk_ntlmv2_resp(uchar *ntlmv2hash,
                                        uchar *challenge_client,
                                        struct ntlmdata *ntlm,
                                        uchar **ntresp,
                                        uint *ntresp_len);

CURLcode  Curl_ntlm_core_mk_lmv2_resp(uchar *ntlmv2hash,
                                      uchar *challenge_client,
                                      uchar *challenge_server,
                                      uchar *lmresp);

#endif /* USE_NTLM_V2 && !USE_WINDOWS_SSPI */

#endif /* USE_NTRESPONSES */

#endif /* !USE_WINDOWS_SSPI || USE_WIN32_CRYPTO */

#endif /* USE_NTLM */

#endif /* HEADER_CURL_NTLM_CORE_H */
