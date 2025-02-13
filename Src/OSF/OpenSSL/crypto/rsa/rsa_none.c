/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
//#include <openssl/rsa.h>

int RSA_padding_add_none(uchar * to, int tlen, const uchar * from, int flen)
{
	if(flen > tlen) {
		RSAerr(RSA_F_RSA_PADDING_ADD_NONE, RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
		return 0;
	}
	if(flen < tlen) {
		RSAerr(RSA_F_RSA_PADDING_ADD_NONE, RSA_R_DATA_TOO_SMALL_FOR_KEY_SIZE);
		return 0;
	}
	memcpy(to, from, (uint)flen);
	return 1;
}

int RSA_padding_check_none(uchar * to, int tlen, const uchar * from, int flen, int num)
{
	if(flen > tlen) {
		RSAerr(RSA_F_RSA_PADDING_CHECK_NONE, RSA_R_DATA_TOO_LARGE);
		return -1;
	}
	memzero(to, tlen - flen);
	memcpy(to + tlen - flen, from, flen);
	return (tlen);
}

