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

#ifndef NO_OLD_ASN1

void * ASN1_dup(i2d_of_void * i2d, d2i_of_void * d2i, void * x)
{
	uchar * b, * p;
	const uchar * p2;
	int i;
	char * ret;

	if(!x)
		return NULL;

	i = i2d(x, 0);
	b = (uchar *)OPENSSL_malloc(i + 10);
	if(!b) {
		ASN1err(ASN1_F_ASN1_DUP, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	p = b;
	i = i2d(x, &p);
	p2 = b;
	ret = (char *)d2i(NULL, &p2, i);
	OPENSSL_free(b);
	return ret;
}

#endif

/*
 * ASN1_ITEM version of dup: this follows the model above except we don't
 * need to allocate the buffer. At some point this could be rewritten to
 * directly dup the underlying structure instead of doing and encode and
 * decode.
 */

void * ASN1_item_dup(const ASN1_ITEM * it, void * x)
{
	uchar * b = NULL;
	const uchar * p;
	long i;
	void * ret;

	if(!x)
		return NULL;

	i = ASN1_item_i2d((ASN1_VALUE*)x, &b, it);
	if(!b) {
		ASN1err(ASN1_F_ASN1_ITEM_DUP, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	p = b;
	ret = ASN1_item_d2i(NULL, &p, i, it);
	OPENSSL_free(b);
	return ret;
}

