/*
 * $Id: plot2d.h,v 1.18 2015/12/21 20:08:40 sfeam Exp $
 */

/* GNUPLOT - plot2d.h */

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

#ifndef GNUPLOT_PLOT2D_H
#define GNUPLOT_PLOT2D_H

//#include "syscfg.h"

struct curve_points;

//
// EXPERIMENTAL configuration option
//
#define SMOOTH_BINS_OPTION 1
//
// This allows a natural interpretation of providing only a single column in 'using'
//
#define default_smooth_weight(option) oneof3(option, SMOOTH_BINS, SMOOTH_KDENSITY, SMOOTH_FREQUENCY)
//
// Variables of plot2d.c needed by other modules:
//
extern curve_points * first_plot;
extern double boxwidth;
extern bool boxwidth_is_absolute;
//
// prototypes from plot2d.c
//
void plotrequest();
void refresh_bounds(curve_points * first_plot, int nplots);

/* internal and external variables */
void cp_free(curve_points *cp);
void cp_extend(curve_points *cp, int num);

//#include "gp_types.h"
//#include "gadgets.h"
GpTextLabel *store_label(GpTextLabel *, GpCoordinate *, int i, char * string, double colorval);
void parse_plot_title(curve_points *this_plot, char *xtitle, char *ytitle, bool *set_title);

#endif /* GNUPLOT_PLOT2D_H */
