/*
 * $Id: gp_hist.h,v 1.13 2013/12/17 00:49:52 sfeam Exp $
 */

/* GNUPLOT - gp_hist.h */

/*[
 * Copyright 1999, 2004   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the complete modified source code.  Modifications are to
 * be distributed as patches to the released version.  Permission to
 * distribute binaries produced by compiling modified sources is granted,
 * provided you
 *   1. distribute the corresponding source modifications from the
 *    released version in the form of a patch file along with the binaries,
 *   2. add special version identification to distinguish your version
 *    in addition to the base release version number,
 *   3. provide your name and address as the primary contact for the
 *    support of your modified version, and
 *   4. retain our contact information in regard to use of the base
 *    software.
 * Permission to distribute the released version of the source code along
 * with corresponding source modifications in the form of a patch file is
 * granted with same provisions 2 through 4 for binary distributions.
 *
 * This software is provided "as is" without express or implied warranty
 * to the extent permitted by applicable law.
]*/

#ifndef GNUPLOT_HISTORY_H
# define GNUPLOT_HISTORY_H

//#include "syscfg.h"

/* Type and struct definitions */

struct GpHist {
    char *line;
    GpHist * prev;
    GpHist * next;
};

#define HISTORY_SIZE 500

/* Variables of history.c needed by other modules: */

extern struct GpHist *history;
extern struct GpHist *cur_entry;
extern int gnuplot_history_size;
extern bool history_quiet;
extern bool history_full;

/* Prototypes of functions exported by history.c */

/* GNU readline
 * Only required by two files directly,
 * so I don't put this into a header file. -lh
 */
#ifdef HAVE_LIBREADLINE
	#include <readline/history.h>
#endif
// NetBSD editline
// (almost) compatible readline replacement
#if defined(HAVE_LIBEDITLINE)
	#include <editline/readline.h>
#endif
#if defined(READLINE) && !defined(HAVE_LIBREADLINE) && !defined(HAVE_LIBEDITLINE)
	void add_history(char *line);
	void write_history_n(const int, const char *, const char *);
	void write_history(char *);
	void read_history(char *);
	const char *history_find(char *);
	const char *history_find_by_number(int);
	int history_find_all(char *);
#elif defined(HAVE_LIBREADLINE) || defined(HAVE_LIBEDITLINE)
	void write_history_n(const int, const char *, const char *);
	const char *history_find(char *);
	const char *history_find_by_number(int);
	int history_find_all(char *);
#endif /* READLINE && !HAVE_LIBREADLINE && !HAVE_LIBEDITLINE */

#endif /* GNUPLOT_HISTORY_H */
