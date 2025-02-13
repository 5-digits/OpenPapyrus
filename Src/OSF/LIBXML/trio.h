/*************************************************************************
 *
 * $Id$
 *
 * Copyright (C) 1998 Bjorn Reese and Daniel Stenberg.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 *************************************************************************
 *
 * http://ctrio.sourceforge.net/
 *
 ************************************************************************/

#ifndef TRIO_TRIO_H
#define TRIO_TRIO_H

#if !defined(WITHOUT_TRIO)
/*
 * Use autoconf defines if present. Packages using trio must define
 * HAVE_CONFIG_H as a compiler option themselves.
 */
#if defined(TRIO_HAVE_CONFIG_H)
	#include "config.h"
#endif
#include "triodef.h"
//#include <stdio.h>
//#include <stdlib.h>
#if defined(TRIO_COMPILER_ANCIENT)
	#include <varargs.h>
#else
	#include <stdarg.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
// 
// Error codes.
// Remember to add a textual description to trio_strerror.
// 
enum {
	TRIO_EOF      = 1,
	TRIO_EINVAL   = 2,
	TRIO_ETOOMANY = 3,
	TRIO_EDBLREF  = 4,
	TRIO_EGAP     = 5,
	TRIO_ENOMEM   = 6,
	TRIO_ERANGE   = 7,
	TRIO_ERRNO    = 8,
	TRIO_ECUSTOM  = 9
};

// Error macros 
#define TRIO_ERROR_CODE(x) ((-(x)) & 0x00FF)
#define TRIO_ERROR_POSITION(x) ((-(x)) >> 8)
#define TRIO_ERROR_NAME(x) trio_strerror(x)

typedef int (*trio_outstream_t)(trio_pointer_t, int);
typedef int (*trio_instream_t)(trio_pointer_t);

const char * trio_strerror(int);
// 
// Print Functions
// 
int trio_printf(const char * format, ...);
int trio_vprintf(const char * format, va_list args);
int trio_printfv(const char * format, void ** args);
int trio_fprintf(FILE *file, const char * format, ...);
int trio_vfprintf(FILE *file, const char * format, va_list args);
int trio_fprintfv(FILE *file, const char * format, void ** args);
int trio_dprintf(int fd, const char * format, ...);
int trio_vdprintf(int fd, const char * format, va_list args);
int trio_dprintfv(int fd, const char * format, void ** args);
int trio_cprintf(trio_outstream_t stream, trio_pointer_t closure, const char * format, ...);
int trio_vcprintf(trio_outstream_t stream, trio_pointer_t closure, const char * format, va_list args);
int trio_cprintfv(trio_outstream_t stream, trio_pointer_t closure, const char * format, void ** args);
int trio_sprintf(char * buffer, const char * format, ...);
int trio_vsprintf(char * buffer, const char * format, va_list args);
int trio_sprintfv(char * buffer, const char * format, void ** args);
int trio_snprintf(char * buffer, size_t max, const char * format, ...);
int trio_vsnprintf(char * buffer, size_t bufferSize, const char * format, va_list args);
int trio_snprintfv(char * buffer, size_t bufferSize, const char * format, void ** args);
int trio_snprintfcat(char * buffer, size_t max, const char * format, ...);
int trio_vsnprintfcat(char * buffer, size_t bufferSize, const char * format, va_list args);
char * trio_aprintf(const char * format, ...);
char * trio_vaprintf(const char * format, va_list args);
int trio_asprintf(char ** ret, const char * format, ...);
int trio_vasprintf(char ** ret, const char * format, va_list args);
//
// Scan Functions
//
int trio_scanf(const char * format, ...);
int trio_vscanf(const char * format, va_list args);
int trio_scanfv(const char * format, void ** args);
int trio_fscanf(FILE *file, const char * format, ...);
int trio_vfscanf(FILE *file, const char * format, va_list args);
int trio_fscanfv(FILE *file, const char * format, void ** args);
int trio_dscanf(int fd, const char * format, ...);
int trio_vdscanf(int fd, const char * format, va_list args);
int trio_dscanfv(int fd, const char * format, void ** args);
int trio_cscanf(trio_instream_t stream, trio_pointer_t closure, const char * format, ...);
int trio_vcscanf(trio_instream_t stream, trio_pointer_t closure, const char * format, va_list args);
int trio_cscanfv(trio_instream_t stream, trio_pointer_t closure, const char * format, void ** args);
int trio_sscanf(const char * buffer, const char * format, ...);
int trio_vsscanf(const char * buffer, const char * format, va_list args);
int trio_sscanfv(const char * buffer, const char * format, void ** args);
//
// Locale Functions
//
void trio_locale_set_decimal_point(char* decimalPoint);
void trio_locale_set_thousand_separator(char* thousandSeparator);
void trio_locale_set_grouping(char* grouping);
//
// Renaming
//
#ifdef TRIO_REPLACE_STDIO
/* Replace the <stdio.h> functions */
#ifndef HAVE_PRINTF
	#undef printf
	#define printf trio_printf
#endif
#ifndef HAVE_VPRINTF
	#undef vprintf
	#define vprintf trio_vprintf
#endif
#ifndef HAVE_FPRINTF
	#undef fprintf
	#define fprintf trio_fprintf
#endif
#ifndef HAVE_VFPRINTF
	#undef vfprintf
	#define vfprintf trio_vfprintf
#endif
#ifndef HAVE_SPRINTF
	#undef sprintf
	#define sprintf trio_sprintf
#endif
#ifndef HAVE_VSPRINTF
	#undef vsprintf
	#define vsprintf trio_vsprintf
#endif
#ifndef HAVE_SNPRINTF
	#undef snprintf
	#define snprintf trio_snprintf
#endif
#ifndef HAVE_VSNPRINTF
	#undef vsnprintf
	#define vsnprintf trio_vsnprintf
#endif
#ifndef HAVE_SCANF
	#undef scanf
	#define scanf trio_scanf
#endif
#ifndef HAVE_VSCANF
	#undef vscanf
	#define vscanf trio_vscanf
#endif
#ifndef HAVE_FSCANF
	#undef fscanf
	#define fscanf trio_fscanf
#endif
#ifndef HAVE_VFSCANF
	#undef vfscanf
	#define vfscanf trio_vfscanf
#endif
#ifndef HAVE_SSCANF
	#undef sscanf
	#define sscanf trio_sscanf
#endif
#ifndef HAVE_VSSCANF
	#undef vsscanf
	#define vsscanf trio_vsscanf
#endif
// These aren't stdio functions, but we make them look similar 
#define dprintf trio_dprintf
#define vdprintf trio_vdprintf
#define aprintf trio_aprintf
#define vaprintf trio_vaprintf
#define asprintf trio_asprintf
#define vasprintf trio_vasprintf
#define dscanf trio_dscanf
#define vdscanf trio_vdscanf
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WITHOUT_TRIO */

#endif /* TRIO_TRIO_H */
