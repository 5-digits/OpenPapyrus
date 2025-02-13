/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Stolen from tjh's ssl/ssl_trc.c stuff.
 */
#include "internal/cryptlib.h"
#pragma hdrstop
//#include "bio_lcl.h"

#define TRUNCATE
#define DUMP_WIDTH      16
#define DUMP_WIDTH_LESS_INDENT(i) (DUMP_WIDTH-((i-(i>6 ? 6 : i)+3)/4))

int BIO_dump_cb(int (* cb)(const void * data, size_t len, void * u), void * u, const char * s, int len)
{
	return BIO_dump_indent_cb(cb, u, s, len, 0);
}

int BIO_dump_indent_cb(int (* cb)(const void * data, size_t len, void * u),
    void * u, const char * s, int len, int indent)
{
	int ret = 0;
	char buf[288 + 1], tmp[20], str[128 + 1];
	int i, j, rows, trc;
	uchar ch;
	int dump_width;

	trc = 0;

#ifdef TRUNCATE
	for(; (len > 0) && ((s[len-1] == ' ') || (s[len-1] == '\0')); len--)
		trc++;
#endif

	if(indent < 0)
		indent = 0;
	if(indent) {
		if(indent > 128)
			indent = 128;
		memset(str, ' ', indent);
	}
	str[indent] = '\0';

	dump_width = DUMP_WIDTH_LESS_INDENT(indent);
	rows = (len / dump_width);
	if((rows * dump_width) < len)
		rows++;
	for(i = 0; i < rows; i++) {
		OPENSSL_strlcpy(buf, str, sizeof buf);
		BIO_snprintf(tmp, sizeof(tmp), "%04x - ", i * dump_width);
		OPENSSL_strlcat(buf, tmp, sizeof buf);
		for(j = 0; j < dump_width; j++) {
			if(((i * dump_width) + j) >= len) {
				OPENSSL_strlcat(buf, "   ", sizeof buf);
			}
			else {
				ch = ((uchar)*(s + i * dump_width + j)) & 0xff;
				BIO_snprintf(tmp, sizeof(tmp), "%02x%c", ch, j == 7 ? '-' : ' ');
				OPENSSL_strlcat(buf, tmp, sizeof buf);
			}
		}
		OPENSSL_strlcat(buf, "  ", sizeof buf);
		for(j = 0; j < dump_width; j++) {
			if(((i * dump_width) + j) >= len)
				break;
			ch = ((uchar)*(s + i * dump_width + j)) & 0xff;
#ifndef CHARSET_EBCDIC
			BIO_snprintf(tmp, sizeof(tmp), "%c",
			    ((ch >= ' ') && (ch <= '~')) ? ch : '.');
#else
			BIO_snprintf(tmp, sizeof(tmp), "%c",
			    ((ch >= os_toascii[' ']) && (ch <= os_toascii['~']))
			    ? os_toebcdic[ch]
			    : '.');
#endif
			OPENSSL_strlcat(buf, tmp, sizeof buf);
		}
		OPENSSL_strlcat(buf, "\n", sizeof buf);
		/*
		 * if this is the last call then update the ddt_dump thing so that we
		 * will move the selection point in the debug window
		 */
		ret += cb((void *)buf, strlen(buf), u);
	}
#ifdef TRUNCATE
	if(trc > 0) {
		BIO_snprintf(buf, sizeof buf, "%s%04x - <SPACES/NULS>\n", str, len + trc);
		ret += cb((void *)buf, strlen(buf), u);
	}
#endif
	return ret;
}

#ifndef OPENSSL_NO_STDIO
static int write_fp(const void * data, size_t len, void * fp)
{
	return UP_fwrite(data, len, 1, fp);
}

int BIO_dump_fp(FILE * fp, const char * s, int len)
{
	return BIO_dump_cb(write_fp, fp, s, len);
}

int BIO_dump_indent_fp(FILE * fp, const char * s, int len, int indent)
{
	return BIO_dump_indent_cb(write_fp, fp, s, len, indent);
}

#endif

static int write_bio(const void * data, size_t len, void * bp)
{
	return BIO_write((BIO*)bp, (const char *)data, len);
}

int BIO_dump(BIO * bp, const char * s, int len)
{
	return BIO_dump_cb(write_bio, bp, s, len);
}

int BIO_dump_indent(BIO * bp, const char * s, int len, int indent)
{
	return BIO_dump_indent_cb(write_bio, bp, s, len, indent);
}

int BIO_hex_string(BIO * out, int indent, int width, uchar * data, int datalen)
{
	int i, j = 0;
	if(datalen > 0) {
		for(i = 0; i < datalen - 1; i++) {
			if(i && !j)
				BIO_printf(out, "%*s", indent, "");
			BIO_printf(out, "%02X:", data[i]);
			j = (j + 1) % width;
			if(!j)
				BIO_printf(out, "\n");
		}
		if(i && !j)
			BIO_printf(out, "%*s", indent, "");
		BIO_printf(out, "%02X", data[datalen - 1]);
	}
	return 1;
}

