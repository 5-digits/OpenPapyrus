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
#include <openssl/idea.h>
#include "idea_lcl.h"
/*
 * The input and output encrypted as though 64bit ofb mode is being used.
 * The extra state information to record how much of the 64bit block we have
 * used is contained in *num;
 */
void IDEA_ofb64_encrypt(const uchar * in, uchar * out, long length, IDEA_KEY_SCHEDULE * schedule, uchar * ivec, int * num)
{
	register ulong v0, v1, t;
	register int n = *num;
	register long l = length;
	uchar d[8];
	register char * dp;
	ulong  ti[2];
	int    save = 0;
	uchar * iv = (uchar *)ivec;
	n2l(iv, v0);
	n2l(iv, v1);
	ti[0] = v0;
	ti[1] = v1;
	dp = (char *)d;
	l2n(v0, dp);
	l2n(v1, dp);
	while(l--) {
		if(n == 0) {
			IDEA_encrypt((ulong*)ti, schedule);
			dp = (char *)d;
			t = ti[0];
			l2n(t, dp);
			t = ti[1];
			l2n(t, dp);
			save++;
		}
		*(out++) = *(in++) ^ d[n];
		n = (n + 1) & 0x07;
	}
	if(save) {
		v0 = ti[0];
		v1 = ti[1];
		iv = (uchar *)ivec;
		l2n(v0, iv);
		l2n(v1, iv);
	}
	t = v0 = v1 = ti[0] = ti[1] = 0;
	*num = n;
}

