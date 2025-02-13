/*====================================================================*
   -  Copyright (C) 2001 Leptonica.  All rights reserved.
   -
   -  Redistribution and use in source and binary forms, with or without
   -  modification, are permitted provided that the following conditions
   -  are met:
   -  1. Redistributions of source code must retain the above copyright
   -     notice, this list of conditions and the following disclaimer.
   -  2. Redistributions in binary form must reproduce the above
   -     copyright notice, this list of conditions and the following
   -     disclaimer in the documentation and/or other materials
   -     provided with the distribution.
   -
   -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
   -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
   -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*====================================================================*/

/*!
 * \file recogident.c
 * <pre>
 *
 *      Top-level identification
 *         int32             recogaIdentifyMultiple()
 *
 *      Segmentation and noise removal
 *         int32             recogSplitIntoCharacters()
 *         int32             recogCorrelationBestRow()
 *         int32             recogCorrelationBestChar()
 *         static int32      pixCorrelationBestShift()
 *
 *      Low-level identification of single characters
 *         int32             recogaIdentifyPixa()
 *         int32             recogIdentifyPixa()
 *         int32             recogIdentifyPix()
 *         int32             recogSkipIdentify()
 *
 *      Operations for handling identification results
 *         static L_RCHA      *rchaCreate()
 *         int32            *rchaDestroy()
 *         static L_RCH       *rchCreate()
 *         int32            *rchDestroy()
 *         int32             rchaExtract()
 *         int32             rchExtract()
 *         static int32      transferRchToRcha()
 *         static int32      recogaSaveBestRcha()
 *         static int32      recogaTransferRch()
 *         int32             recogTransferRchToDid()
 *
 *      Preprocessing and filtering
 *         int32             recogProcessToIdentify()
 *         PIX                *recogPreSplittingFilter()
 *         PIX                *recogSplittingFilter()
 *
 *      Postprocessing
 *         SARRAY             *recogExtractNumbers()
 *
 *      Modifying recog behavior
 *         int32             recogSetTemplateType()
 *         int32             recogSetScaling()
 *
 *      Static debug helper
 *         static void         l_showIndicatorSplitValues()
 *
 *  See recogbasic.c for examples of training a recognizer, which is
 *  required before it can be used for identification.
 *
 *  The character splitter repeatedly does a greedy correlation with each
 *  averaged unscaled template, at all pixel locations along the text to
 *  be identified.  The vertical alignment is between the template
 *  centroid and the (moving) windowed centroid, including a delta of
 *  1 pixel above and below.  The best match then removes part of the
 *  input image, leaving 1 or 2 pieces, which, after filtering,
 *  are put in a queue.  The process ends when the queue is empty.
 *  The filtering is based on the size and aspect ratio of the
 *  remaining pieces; the intent is to remove anything that is
 *  unlikely to be text, such as small pieces and line graphics.
 *
 *  After splitting, the selected segments are identified using
 *  the input parameters that were initially specified for the
 *  recognizer.  Unlike the splitter, which uses the averaged
 *  templates from the unscaled input, the recognizer can use
 *  either all training examples or averaged templates, and these
 *  can be either scaled or unscaled.  These choices are specified
 *  when the recognizer is constructed.
 * </pre>
 */
#include "allheaders.h"
#pragma hdrstop

/* Padding on pix1: added before correlations and removed from result */
static const int32 LeftRightPadding = 32;

/* Parameters for filtering and sorting connected components in splitter */
static const float MaxAspectRatio = 6.0f;
static const float MinFillFactor = 0.10f;
static const int32 MinOverlap1 = 6;   /* in pass 1 of boxaSort2d() */
static const int32 MinOverlap2 = 6;   /* in pass 2 of boxaSort2d() */
static const int32 MinHeightPass1 = 5;   /* min height to start pass 1 */

static int32 pixCorrelationBestShift(PIX * pix1, PIX * pix2, NUMA * nasum1,
    NUMA * namoment1, int32 area2,
    int32 ycent2, int32 maxyshift,
    int32 * tab8, int32 * pdelx,
    int32 * pdely, float * pscore,
    int32 debugflag);
static L_RCH * rchCreate(int32 index, float score, char * text,
    int32 sample, int32 xloc, int32 yloc,
    int32 width);
static L_RCHA * rchaCreate();
static int32 transferRchToRcha(L_RCH * rch, L_RCHA * rcha);
static void l_showIndicatorSplitValues(NUMA * na1, NUMA * na2, NUMA * na3,
    NUMA * na4, NUMA * na5, NUMA * na6);
static int32 recogaSaveBestRcha(L_RECOGA * recoga, PIXA * pixa);
static int32 recogaTransferRch(L_RECOGA * recoga, L_RECOG * recog,
    int32 index);

/*------------------------------------------------------------------------*
*                             Identification
*------------------------------------------------------------------------*/
/*!
 * \brief   recogaIdentifyMultiple()
 *
 * \param[in]    recoga with training finished
 * \param[in]    pixs containing typically a small number of characters
 * \param[in]    nitems to be identified in pix; use 0 if not known
 * \param[in]    minw remove components with width less than this;
 *                    use -1 for removing all noise components
 * \param[in]    minh remove components with height less than this;
 *                    use -1 for removing all noise components
 * \param[in]    skipsplit 1 to skip the splitting step
 * \param[out]   pboxa [optional] locations of identified components
 * \param[out]   ppixa [optional] images of identified components
 * \param[out]   ppixdb [optional] debug pix: inputs and best fits
 * \param[in]    debugsplit 1 returns pix split debugging images
 * \return  0 if OK; 1 if nothing is found; 2 for other errors.
 *              Get a warning if nitems and the number found are both > 0,
 *              but not equal to each other.
 *
 * <pre>
 * Notes:
 *      (1) This filters the input pixa, looking for %nitems if requested.
 *          Set %nitems == 0 if you don't know how many chars to expect.
 *      (2) This bundles the filtered components into a pixa and calls
 *          recogIdentifyPixa().  If %nitems \> 0, use %minw = -1 and
 *          %minh = -1 to remove all noise components.
 *      (3) Set %minw = 0 and %minh = 0 to get all noise components.
 *          Set %minw \> 0 and/or %minh \> 0 to retain selected noise components.
 *          All noise components are recognized as an empty string with
 *          a score of 0.0.
 *      (4) Splitting is relatively slow, because it tries to match all
 *          character templates to all locations.  This step can be skipped.
 *      (5) An attempt is made to order the (optionally) returned images
 *          and boxes in 2-dimensional sorted order.  These can then
 *          be used to aggregate identified characters into numbers or words.
 *          One typically wants the pixa, which contains a boxa of the
 *          extracted subimages.
 * </pre>
 */
int32 recogaIdentifyMultiple(L_RECOGA  * recoga,
    PIX       * pixs,
    int32 nitems,
    int32 minw,
    int32 minh,
    int32 skipsplit,
    BOXA     ** pboxa,
    PIXA     ** ppixa,
    PIX      ** ppixdb,
    int32 debugsplit)
{
	int32 n, done;
	BOXA     * boxa;
	PIX      * pixb;
	PIXA     * pixa;
	NUMA     * naid;
	L_RECOG  * recog;

	PROCNAME("recogaIdentifyMultiple");

	if(pboxa) *pboxa = NULL;
	if(ppixa) *ppixa = NULL;
	if(ppixdb) *ppixdb = NULL;
	if(!recoga || recoga->n == 0)
		return ERROR_INT("recog not defined or empty", procName, 2);
	recogaTrainingDone(recoga, &done);
	if(!done)
		return ERROR_INT("training not finished", procName, 2);
	if(!pixs)
		return ERROR_INT("pixs not defined", procName, 2);

	/* Binarize if necessary */
	recog = recogaGetRecog(recoga, 0); /* use the first one */
	if(pixGetDepth(pixs) > 1)
		pixb = pixConvertTo1(pixs, recog->threshold);
	else
		pixb = pixClone(pixs);
	/* Noise removal and splitting of touching characters */
	recogSplitIntoCharacters(recog, pixb, minw, minh, skipsplit, &boxa, &pixa, &naid, debugsplit);
	pixDestroy(&pixb);
	if(!pixa || (n = pixaGetCount(pixa)) == 0) {
		pixaDestroy(&pixa);
		boxaDestroy(&boxa);
		numaDestroy(&naid);
		L_WARNING("nothing found\n", procName);
		return 1;
	}
	if(nitems > 0 && n != nitems)
		L_WARNING3("Expected %d items; found %d\n", procName, nitems, n);
	recogaIdentifyPixa(recoga, pixa, naid, ppixdb);
	if(pboxa)
		*pboxa = boxa;
	else
		boxaDestroy(&boxa);
	if(ppixa)
		*ppixa = pixa;
	else
		pixaDestroy(&pixa);

	numaDestroy(&naid);
	return 0;
}

/*!
 * \brief   recogSplitIntoCharacters()
 *
 * \param[in]    recog
 * \param[in]    pixs 1 bpp, contains only mostly deskewed text
 * \param[in]    minw remove components with width less than this;
 *                    use -1 for default removing out of band components
 * \param[in]    minh remove components with height less than this;
 *                    use -1 for default removing out of band components
 * \param[in]    skipsplit 1 to skip the splitting step
 * \param[out]   pboxa character bounding boxes
 * \param[out]   ppixa character images
 * \param[out]   pnaid indices of components to identify
 * \param[in]    debug 1 for results written to pixadb_split
 * \return  0 if OK, 1 on error or if no components are returned
 *
 * <pre>
 * Notes:
 *      (1) This can be given an image that has an arbitrary number
 *          of text characters.  It optionally splits connected
 *          components based on greedy correlation matching in
 *          recogCorrelationBestRow().  The returned pixa includes
 *          the boxes from which the (possibly split) components
 *          are extracted.
 *      (2) If either %minw \< 0 or %minh \< 0, noise components are
 *          filtered out, and the returned %naid array is all 1.
 *          Otherwise, some noise components whose dimensions (w,h)
 *          satisfy w \>= %minw and h \>= %minh are allowed through, but
 *          they are identified in the returned %naid, where they are
 *          labelled by 0 to indicate that they are not to be run
 *          through identification.  Retaining the noise components
 *          provides spatial information that can help applications
 *          interpret the results.
 *      (3) In addition to optional filtering of the noise, the
 *          resulting components are put in row-major (2D) order,
 *          and the smaller of overlapping components are removed if
 *          they satisfy conditions of relative size and fractional overlap.
 *      (4) Note that the spliting function uses unscaled templates
 *          and does not bother returning the class results and scores.
 *          Thes are more accurately found later using the scaled templates.
 * </pre>
 */
int32 recogSplitIntoCharacters(L_RECOG  * recog,
    PIX      * pixs,
    int32 minw,
    int32 minh,
    int32 skipsplit,
    BOXA    ** pboxa,
    PIXA    ** ppixa,
    NUMA    ** pnaid,
    int32 debug)
{
	int32 empty, maxw, bw, ncomp, same, savenoise, scaling;
	int32 i, j, n, n3, xoff, yoff;
	BOX     * box, * box3;
	BOXA    * boxa1, * boxa2, * boxa3, * boxa4, * boxat1, * boxat2, * boxad;
	BOXAA   * baa;
	NUMA    * naid;
	PIX     * pix, * pix1, * pix2;

	PROCNAME("recogSplitIntoCharacters");

	if(pboxa) *pboxa = NULL;
	if(ppixa) *ppixa = NULL;
	if(pnaid) *pnaid = NULL;
	if(!pboxa || !ppixa || !pnaid)
		return ERROR_INT("&boxa, &pixa and &naid not defined", procName, 1);
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!recog->train_done)
		return ERROR_INT("training not finished", procName, 1);
	if(!pixs || pixGetDepth(pixs) != 1)
		return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);
	pixZero(pixs, &empty);
	if(empty) return 1;

	/* Small vertical close for consolidation.  Don't do a horizontal
	 * closing, because it might join separate characters. */
	pix1 = pixMorphSequence(pixs, "c1.3", 0);

	/* Carefully filter out noise */
	pix2 = recogPreSplittingFilter(recog, pix1, MaxAspectRatio,
	    MinFillFactor, debug);

	/* Optionally, save a boxa of noise components, filtered
	 * according to input parameters %minw and %minh */
	boxa3 = NULL;
	savenoise = (minw >= 0 && minh >= 0);
	if(savenoise) { /* accept some noise comonents */
		pixXor(pix1, pix1, pix2); /* leave noise components only in pix1 */
		pixZero(pix1, &empty);
		if(!empty) {
			boxat1 = pixConnComp(pix1, NULL, 8);
			boxa3 = boxaSelectBySize(boxat1, minw, minh, L_SELECT_BOTH,
			    L_SELECT_IF_GTE, NULL);
			boxaDestroy(&boxat1);
		}
	}
	pixDestroy(&pix1);

	/* Get the 8-connected non-noise components to be split/identified */
	boxa1 = pixConnComp(pix2, NULL, 8);
	pixDestroy(&pix2);
	ncomp = boxaGetCount(boxa1);
	if(ncomp == 0) {
		boxaDestroy(&boxa1);
		boxaDestroy(&boxa3);
		L_WARNING("all components removed\n", procName);
		return 1;
	}

	/* Save everything and split the large non-noise components */
	boxa2 = boxaCreate(ncomp);
	maxw = recog->maxwidth_u + 5;
	scaling = (recog->scalew > 0 || recog->scaleh > 0) ? TRUE : FALSE;
	for(i = 0; i < ncomp; i++) {
		box = boxaGetBox(boxa1, i, L_CLONE);
		boxGetGeometry(box, &xoff, &yoff, &bw, NULL);
		/* Treat as one character if it is small, if the images
		 * have been scaled, or if splitting is not to be run. */
		if(bw <= maxw || scaling || skipsplit) {
			boxaAddBox(boxa2, box, L_INSERT);
		}
		else {
			pix = pixClipRectangle(pixs, box, NULL);
			recogCorrelationBestRow(recog, pix, &boxat1, NULL, NULL, NULL, debug);
			pixDestroy(&pix);
			boxDestroy(&box);
			if(!boxat1) {
				L_ERROR2("boxat1 not found for component %d\n", procName, i);
			}
			else {
				boxat2 = boxaTransform(boxat1, xoff, yoff, 1.0, 1.0);
				boxaJoin(boxa2, boxat2, 0, -1);
				boxaDestroy(&boxat1);
				boxaDestroy(&boxat2);
			}
		}
	}
	boxaDestroy(&boxa1);

	/* If the noise boxa was retained, add it back in, so we have
	 * a mixture of non-noise and noise components. */
	if(boxa3)
		boxaJoin(boxa2, boxa3, 0, -1);

	/* Do a 2D sort on the bounding boxes, and flatten the result to 1D.
	 * For the 2D sort, to add a box to an existing boxa, we require
	 * specified minimum vertical overlaps for the first two passes
	 * of the 2D sort.  In pass 1, only components with sufficient
	 * height can start a new boxa. */
	baa = boxaSort2d(boxa2, NULL, MinOverlap1, MinOverlap2, MinHeightPass1);
	boxa4 = boxaaFlattenToBoxa(baa, NULL, L_CLONE);
	boxaaDestroy(&baa);
	boxaDestroy(&boxa2);

	/* Remove smaller components of overlapping pairs.
	 * We only remove the small component if the overlap is
	 * at least half its area and if its area is no more
	 * than 30% of the area of the large component.  Because the
	 * components are in a flattened 2D sort, we don't need to
	 * look far ahead in the array to find all overlapping boxes;
	 * 10 boxes is plenty. */
	boxad = boxaHandleOverlaps(boxa4, L_COMBINE, 10, 0.5f, 0.3f, NULL);
	boxaDestroy(&boxa4);

	/* If savenoise == true and there are components in boxa3,
	 * use the full set of noise components in boxa3 to identify
	 * the remaining ones in boxad. */
	n = boxaGetCount(boxad);
	naid = numaMakeConstant(1, n);
	if(savenoise && boxa3) {
		n3 = boxaGetCount(boxa3);
		for(i = 0; i < n; i++) {
			box = boxaGetBox(boxad, i, L_CLONE);
			for(j = 0; j < n3; j++) {
				box3 = boxaGetBox(boxa3, j, L_CLONE);
				boxEqual(box, box3, &same);
				boxDestroy(&box3);
				if(same) {
					numaSetValue(naid, i, 0); /* label noise 0 */
					break;
				}
			}
			boxDestroy(&box);
		}
	}
	boxaDestroy(&boxa3);

	/* Extract and save the image pieces from the input image. */
	*ppixa = pixClipRectangles(pixs, boxad);
	*pboxa = boxad;
	*pnaid = naid;
	return 0;
}

/*!
 * \brief   recogCorrelationBestRow()
 *
 * \param[in]    recog with LUT's pre-computed
 * \param[in]    pixs typically of multiple touching characters, 1 bpp
 * \param[out]   pboxa bounding boxs of best fit character
 * \param[out]   pnascore [optional] correlation scores
 * \param[out]   pnaindex [optional] indices of classes
 * \param[out]   psachar [optional] array of character strings
 * \param[in]    debug 1 for results written to pixadb_split
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Supervises character matching for (in general) a c.c with
 *          multiple touching characters.  Finds the best match greedily.
 *          Rejects small parts that are left over after splitting.
 *      (2) Matching is to the average, and without character scaling.
 * </pre>
 */
int32 recogCorrelationBestRow(L_RECOG  * recog,
    PIX      * pixs,
    BOXA    ** pboxa,
    NUMA    ** pnascore,
    NUMA    ** pnaindex,
    SARRAY  ** psachar,
    int32 debug)
{
	char      * charstr;
	int32 index, remove, w, h, bx, bw, bxc, bwc, w1, w2, w3;
	float score;
	BOX       * box, * boxc, * boxtrans, * boxl, * boxr, * boxlt, * boxrt;
	BOXA      * boxat;
	NUMA      * nascoret, * naindext, * nasort;
	PIX       * pixb, * pixc, * pixl, * pixr, * pixdb, * pixd;
	PIXA      * pixar, * pixadb;
	SARRAY    * sachart;

	int32 iter;

	PROCNAME("recogCorrelationBestRow");

	if(pnascore) *pnascore = NULL;
	if(pnaindex) *pnaindex = NULL;
	if(psachar) *psachar = NULL;
	if(!pboxa)
		return ERROR_INT("&boxa not defined", procName, 1);
	*pboxa = NULL;
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!pixs || pixGetDepth(pixs) != 1)
		return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);
	if(pixGetWidth(pixs) < recog->minwidth_u - 4)
		return ERROR_INT("pixs too narrow", procName, 1);
	if(!recog->train_done)
		return ERROR_INT("training not finished", procName, 1);

	/* Binarize and crop to foreground if necessary */
	pixb = recogProcessToIdentify(recog, pixs, 0);

	/* Initialize the arrays */
	boxat = boxaCreate(4);
	nascoret = numaCreate(4);
	naindext = numaCreate(4);
	sachart = sarrayCreate(4);
	pixadb = (debug) ? pixaCreate(4) : NULL;

	/* Initialize the images remaining to be processed with the input.
	 * These are stored in pixar, which is used here as a queue,
	 * on which we only put image fragments that are large enough to
	 * contain at least one character.  */
	pixar = pixaCreate(1);
	pixGetDimensions(pixb, &w, &h, NULL);
	box = boxCreate(0, 0, w, h);
	pixaAddPix(pixar, pixb, L_INSERT);
	pixaAddBox(pixar, box, L_INSERT);

	/* Successively split on the best match until nothing is left.
	 * To be safe, we limit the search to 10 characters. */
	for(iter = 0; iter < 11; iter++) {
		if(pixaGetCount(pixar) == 0)
			break;
		if(iter == 10) {
			L_WARNING("more than 10 chars; ending search\n", procName);
			break;
		}

		/* Pop one from the queue */
		pixaRemovePixAndSave(pixar, 0, &pixc, &boxc);
		boxGetGeometry(boxc, &bxc, NULL, &bwc, NULL);

		/* This is a single component; if noise, remove it */
		recogSplittingFilter(recog, pixc, MaxAspectRatio, MinFillFactor,
		    &remove, debug);
		if(debug)
			fprintf(stderr, "iter = %d, removed = %d\n", iter, remove);
		if(remove) {
			pixDestroy(&pixc);
			boxDestroy(&boxc);
			continue;
		}

		/* Find the best character match */
		if(debug) {
			recogCorrelationBestChar(recog, pixc, &box, &score, &index, &charstr, &pixdb);
			pixaAddPix(pixadb, pixdb, L_INSERT);
		}
		else {
			recogCorrelationBestChar(recog, pixc, &box, &score, &index, &charstr, NULL);
		}

		/* Find the box in original coordinates, and append
		 * the results to the arrays. */
		boxtrans = boxTransform(box, bxc, 0, 1.0, 1.0);
		boxaAddBox(boxat, boxtrans, L_INSERT);
		numaAddNumber(nascoret, score);
		numaAddNumber(naindext, index);
		sarrayAddString(sachart, charstr, L_INSERT);

		/* Split the current pixc into three regions and save
		 * each region if it is large enough. */
		boxGetGeometry(box, &bx, NULL, &bw, NULL);
		w1 = bx;
		w2 = bw;
		w3 = bwc - bx - bw;
		if(debug)
			fprintf(stderr, " w1 = %d, w2 = %d, w3 = %d\n", w1, w2, w3);
		if(w1 < recog->minwidth_u - 4) {
			if(debug) 
				L_INFO2("discarding width %d on left\n", procName, w1);
		}
		else { /* extract and save left region */
			boxl = boxCreate(0, 0, bx + 1, h);
			pixl = pixClipRectangle(pixc, boxl, NULL);
			boxlt = boxTransform(boxl, bxc, 0, 1.0, 1.0);
			pixaAddPix(pixar, pixl, L_INSERT);
			pixaAddBox(pixar, boxlt, L_INSERT);
			boxDestroy(&boxl);
		}
		if(w3 < recog->minwidth_u - 4) {
			if(debug) 
				L_INFO2("discarding width %d on right\n", procName, w3);
		}
		else { /* extract and save left region */
			boxr = boxCreate(bx + bw - 1, 0, w3 + 1, h);
			pixr = pixClipRectangle(pixc, boxr, NULL);
			boxrt = boxTransform(boxr, bxc, 0, 1.0, 1.0);
			pixaAddPix(pixar, pixr, L_INSERT);
			pixaAddBox(pixar, boxrt, L_INSERT);
			boxDestroy(&boxr);
		}
		pixDestroy(&pixc);
		boxDestroy(&box);
		boxDestroy(&boxc);
	}
	pixaDestroy(&pixar);

	/* Sort the output results by left-to-right in the boxa */
	*pboxa = boxaSort(boxat, L_SORT_BY_X, L_SORT_INCREASING, &nasort);
	if(pnascore)
		*pnascore = numaSortByIndex(nascoret, nasort);
	if(pnaindex)
		*pnaindex = numaSortByIndex(naindext, nasort);
	if(psachar)
		*psachar = sarraySortByIndex(sachart, nasort);
	numaDestroy(&nasort);
	boxaDestroy(&boxat);
	numaDestroy(&nascoret);
	numaDestroy(&naindext);
	sarrayDestroy(&sachart);

	/* Final debug output */
	if(debug) {
		pixd = pixaDisplayTiledInRows(pixadb, 32, 2000, 1.0, 0, 15, 2);
		pixDisplay(pixd, 400, 400);
		pixaAddPix(recog->pixadb_split, pixd, L_INSERT);
		pixaDestroy(&pixadb);
	}
	return 0;
}

/*!
 * \brief   recogCorrelationBestChar()
 *
 * \param[in]    recog with LUT's pre-computed
 * \param[in]    pixs can be of multiple touching characters, 1 bpp
 * \param[out]   pbox bounding box of best fit character
 * \param[out]   pscore correlation score
 * \param[out]   pindex [optional] index of class
 * \param[out]   pcharstr [optional] character string of class
 * \param[out]   ppixdb [optional] debug pix showing input and best fit
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Basic matching character splitter.  Finds the best match among
 *          all templates to some region of the image.  This can result
 *          in splitting the image into two parts.  This is "image decoding"
 *          without dynamic programming, because we don't use a setwidth
 *          and compute the best matching score for the entire image.
 *      (2) Matching is to the average templates, without character scaling.
 * </pre>
 */
int32 recogCorrelationBestChar(L_RECOG    * recog,
    PIX        * pixs,
    BOX       ** pbox,
    float  * pscore,
    int32    * pindex,
    char      ** pcharstr,
    PIX       ** ppixdb)
{
	int32 i, n, w1, h1, w2, area2, ycent2, delx, dely;
	int32 bestdelx, bestdely, bestindex;
	float score, bestscore;
	BOX       * box;
	BOXA      * boxa;
	NUMA      * nasum, * namoment;
	PIX       * pix1, * pix2;

	PROCNAME("recogCorrelationBestChar");

	if(pindex) *pindex = 0;
	if(pcharstr) *pcharstr = NULL;
	if(ppixdb) *ppixdb = NULL;
	if(pbox) *pbox = NULL;
	if(pscore) *pscore = 0.0;
	if(!pbox || !pscore)
		return ERROR_INT("&box and &score not both defined", procName, 1);
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!pixs || pixGetDepth(pixs) != 1)
		return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);
	if(!recog->train_done)
		return ERROR_INT("training not finished", procName, 1);

	/* Binarize and crop to foreground if necessary.  Add padding
	 * to both the left and right side; this is compensated for
	 * when reporting the bounding box of the best matched character. */
	pix1 = recogProcessToIdentify(recog, pixs, LeftRightPadding);
	pixGetDimensions(pix1, &w1, &h1, NULL);

	/* Compute vertical sum and moment arrays */
	nasum = pixCountPixelsByColumn(pix1);
	namoment = pixGetMomentByColumn(pix1, 1);

	/* Do shifted correlation against all averaged templates. */
	n = recog->setsize;
	boxa = boxaCreate(n); /* location of best fits for each character */
	bestscore = 0.0;
	for(i = 0; i < n; i++) {
		pix2 = pixaGetPix(recog->pixa_u, i, L_CLONE);
		w2 = pixGetWidth(pix2);
		/* Note that the slightly expended w1 is typically larger
		 * than w2 (the template). */
		if(w1 >= w2) {
			numaGetIValue(recog->nasum_u, i, &area2);
			ptaGetIPt(recog->pta_u, i, NULL, &ycent2);
			pixCorrelationBestShift(pix1, pix2, nasum, namoment, area2, ycent2,
			    recog->maxyshift, recog->sumtab, &delx,
			    &dely, &score, 1);
			if(ppixdb) {
				fprintf(stderr,
				    "Best match template %d: (x,y) = (%d,%d), score = %5.3f\n",
				    i, delx, dely, score);
			}
			/* Compensate for padding */
			box = boxCreate(delx - LeftRightPadding, 0, w2, h1);
			if(score > bestscore) {
				bestscore = score;
				bestdelx = delx - LeftRightPadding;
				bestdely = dely;
				bestindex = i;
			}
		}
		else {
			box = boxCreate(0, 0, 1, 1); /* placeholder */
			if(ppixdb)
				fprintf(stderr, "Component too thin: w1 = %d, w2 = %d\n", w1, w2);
		}
		boxaAddBox(boxa, box, L_INSERT);
		pixDestroy(&pix2);
	}
	*pscore = bestscore;
	*pbox = boxaGetBox(boxa, bestindex, L_COPY);
	if(pindex) *pindex = bestindex;
	if(pcharstr)
		recogGetClassString(recog, bestindex, pcharstr);
	if(ppixdb) {
		L_INFO4("Best match: class %d; shifts (%d, %d)\n", procName, bestindex, bestdelx, bestdely);
		pix2 = pixaGetPix(recog->pixa_u, bestindex, L_CLONE);
		*ppixdb = recogShowMatch(recog, pix1, pix2, NULL, -1, 0.0);
		pixDestroy(&pix2);
	}
	pixDestroy(&pix1);
	boxaDestroy(&boxa);
	numaDestroy(&nasum);
	numaDestroy(&namoment);
	return 0;
}

/*!
 * \brief   pixCorrelationBestShift()
 *
 * \param[in]    pix1   1 bpp, the unknown image; typically larger
 * \param[in]    pix2   1 bpp, the matching template image)
 * \param[in]    nasum1 vertical column pixel sums for pix1
 * \param[in]    namoment1  vertical column first moment of pixels for pix1
 * \param[in]    area2  number of on pixels in pix2
 * \param[in]    ycent2  y component of centroid of pix2
 * \param[in]    maxyshift  max y shift of pix2 around the location where
 *                          the centroids of pix2 and a windowed part of pix1
 *                          are vertically aligned
 * \param[in]    tab8 [optional] sum tab for ON pixels in byte; can be NULL
 * \param[out]   pdelx [optional] best x shift of pix2 relative to pix1
 *           [out]   pdely ([optional] best y shift of pix2 relative to pix1
 *           [out]   pscore ([optional] maximum score found; can be NULL
 * \param[in]    debugflag <= 0 to skip; positive to generate output.
 *                         The integer is used to label the debug image.
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This maximizes the correlation score between two 1 bpp images,
 *          one of which is typically wider.  In a typical example,
 *          pix1 is a bitmap of 2 or more touching characters and pix2 is
 *          a single character template.  This finds the location of pix2
 *          that gives the largest correlation.
 *      (2) The windowed area of fg pixels and windowed first moment
 *          in the y direction are computed from the input sum and moment
 *          column arrays, %nasum1 and %namoment1
 *      (3) This is a brute force operation.  We compute the correlation
 *          at every x shift for which pix2 fits entirely within pix1,
 *          and where the centroid of pix2 is aligned, within +-maxyshift,
 *          with the centroid of a window of pix1 of the same width.
 *          The correlation is taken over the full height of pix1.
 *          This can be made more efficient.
 * </pre>
 */
static int32 pixCorrelationBestShift(PIX        * pix1,
    PIX        * pix2,
    NUMA       * nasum1,
    NUMA       * namoment1,
    int32 area2,
    int32 ycent2,
    int32 maxyshift,
    int32    * tab8,
    int32    * pdelx,
    int32    * pdely,
    float  * pscore,
    int32 debugflag)
{
	int32 w1, w2, h1, h2, i, j, nx, shifty, delx, dely;
	int32 sum, moment, count;
	int32    * tab, * area1, * arraysum, * arraymoment;
	float maxscore, score;
	float  * ycent1;
	FPIX       * fpix;
	PIX        * pixt, * pixt1, * pixt2;

	PROCNAME("pixCorrelationBestShift");

	if(pdelx) *pdelx = 0;
	if(pdely) *pdely = 0;
	if(pscore) *pscore = 0.0;
	if(!pix1 || pixGetDepth(pix1) != 1)
		return ERROR_INT("pix1 not defined or not 1 bpp", procName, 1);
	if(!pix2 || pixGetDepth(pix2) != 1)
		return ERROR_INT("pix2 not defined or not 1 bpp", procName, 1);
	if(!nasum1 || !namoment1)
		return ERROR_INT("nasum1 and namoment1 not both defined", procName, 1);
	if(area2 <= 0 || ycent2 <= 0)
		return ERROR_INT("area2 and ycent2 must be > 0", procName, 1);

	/* If pix1 (the unknown image) is narrower than pix2,
	 * don't bother to try the match.  pix1 is already padded with
	 * 2 pixels on each side. */
	pixGetDimensions(pix1, &w1, &h1, NULL);
	pixGetDimensions(pix2, &w2, &h2, NULL);
	if(w1 < w2) {
		if(debugflag > 0) {
			L_INFO3("skipping match with w1 = %d and w2 = %d\n", procName, w1, w2);
		}
		return 0;
	}
	nx = w1 - w2 + 1;

	if(debugflag > 0)
		fpix = fpixCreate(nx, 2 * maxyshift + 1);
	if(!tab8)
		tab = makePixelSumTab8();
	else
		tab = tab8;

	/* Set up the arrays for area1 and ycent1.  We have to do this
	 * for each template (pix2) because the window width is w2. */
	area1 = (int32*)LEPT_CALLOC(nx, sizeof(int32));
	ycent1 = (float*)LEPT_CALLOC(nx, sizeof(int32));
	arraysum = numaGetIArray(nasum1);
	arraymoment = numaGetIArray(namoment1);
	for(i = 0, sum = 0, moment = 0; i < w2; i++) {
		sum += arraysum[i];
		moment += arraymoment[i];
	}
	for(i = 0; i < nx - 1; i++) {
		area1[i] = sum;
		ycent1[i] = (sum == 0) ? ycent2 : (float)moment / (float)sum;
		sum += arraysum[w2 + i] - arraysum[i];
		moment += arraymoment[w2 + i] - arraymoment[i];
	}
	area1[nx - 1] = sum;
	ycent1[nx - 1] = (sum == 0) ? ycent2 : (float)moment / (float)sum;

	/* Find the best match location for pix2.  At each location,
	 * to insure that pixels are ON only within the intersection of
	 * pix and the shifted pix2:
	 *  (1) Start with pixt cleared and equal in size to pix1.
	 *  (2) Blit the shifted pix2 onto pixt.  Then all ON pixels
	 *      are within the intersection of pix1 and the shifted pix2.
	 *  (3) AND pix1 with pixt. */
	pixt = pixCreate(w2, h1, 1);
	maxscore = 0;
	delx = 0;
	dely = 0; /* amount to shift pix2 relative to pix1 to get alignment */
	for(i = 0; i < nx; i++) {
		shifty = (int32)(ycent1[i] - ycent2 + 0.5);
		for(j = -maxyshift; j <= maxyshift; j++) {
			pixClearAll(pixt);
			pixRasterop(pixt, 0, shifty + j, w2, h2, PIX_SRC, pix2, 0, 0);
			pixRasterop(pixt, 0, 0, w2, h1, PIX_SRC & PIX_DST, pix1, i, 0);
			pixCountPixels(pixt, &count, tab);
			score = (float)count * (float)count / ((float)area1[i] * (float)area2);
			if(score > maxscore) {
				maxscore = score;
				delx = i;
				dely = shifty + j;
			}
			if(debugflag > 0)
				fpixSetPixel(fpix, i, maxyshift + j, 1000.0f * score);
		}
	}

	if(debugflag > 0) {
		lept_mkdir("lept/recog");
		char buf[128];
		pixt1 = fpixDisplayMaxDynamicRange(fpix);
		pixt2 = pixExpandReplicate(pixt1, 5);
		_snprintf(buf, sizeof(buf), "/tmp/lept/recog/junkbs_%d.png", debugflag);
		pixWrite(buf, pixt2, IFF_PNG);
		pixDestroy(&pixt1);
		pixDestroy(&pixt2);
		fpixDestroy(&fpix);
	}

	if(pdelx) *pdelx = delx;
	if(pdely) *pdely = dely;
	if(pscore) *pscore = maxscore;
	if(!tab8) LEPT_FREE(tab);
	LEPT_FREE(area1);
	LEPT_FREE(ycent1);
	LEPT_FREE(arraysum);
	LEPT_FREE(arraymoment);
	pixDestroy(&pixt);
	return 0;
}

/*------------------------------------------------------------------------*
*                          Low-level identification                      *
*------------------------------------------------------------------------*/
/*!
 * \brief   recogaIdentifyPixa()
 *
 * \param[in]    recoga
 * \param[in]    pixa of 1 bpp images to match
 * \param[in]    naid [optional] indices of components to identify; can be null
 * \param[out]   ppixdb [optional] pix showing inputs and best fits
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) See recogIdentifyPixa().  This does the same operation
 *          for each recog, returning the arrays of results (scores,
 *          class index and character string) for the best correlation match.
 * </pre>
 */
int32 recogaIdentifyPixa(L_RECOGA  * recoga,
    PIXA      * pixa,
    NUMA      * naid,
    PIX      ** ppixdb)
{
	int32 done, i, n, nrec;
	PIX       * pix1;
	PIXA      * pixadb;
	L_RECOG   * recog;

	PROCNAME("recogaIdentifyPixa");

	if(ppixdb) *ppixdb = NULL;
	if(!recoga)
		return ERROR_INT("recoga not defined", procName, 2);
	if((nrec = recogaGetCount(recoga)) == 0)
		return ERROR_INT("recoga empty", procName, 2);
	recogaTrainingDone(recoga, &done);
	if(!done)
		return ERROR_INT("training not finished", procName, 1);
	if(!pixa || (pixaGetCount(pixa) == 0))
		return ERROR_INT("pixa not defined", procName, 1);
	if((n = pixaGetCount(pixa)) == 0)
		return ERROR_INT("pixa is empty", procName, 1);

	/* Run each recognizer on the set of images.  This writes
	 * the text string into each pix of the pixa_id copy. */
	rchaDestroy(&recoga->rcha);
	recoga->rcha = rchaCreate();
	pixadb = (ppixdb) ? pixaCreate(n) : NULL;
	for(i = 0; i < nrec; i++) {
		recog = recogaGetRecog(recoga, i);
		if(!ppixdb) {
			recogIdentifyPixa(recog, pixa, naid, NULL);
		}
		else {
			recogIdentifyPixa(recog, pixa, naid, &pix1);
			pixaAddPix(pixadb, pix1, L_INSERT);
		}
	}

	/* Accumulate the best results in the cha of the recoga.  This
	 * also writes the text string into each pix of the input pixa. */
	recogaSaveBestRcha(recoga, pixa);

	/* Package the images for debug */
	if(pixadb)
		*ppixdb = pixaDisplayLinearly(pixadb, L_VERT, 1.0, 0, 20, 2, NULL);
	pixaDestroy(&pixadb);
	return 0;
}

/*!
 * \brief   recogIdentifyPixa()
 *
 * \param[in]    recog
 * \param[in]    pixa of 1 bpp images to match
 * \param[in]    naid [optional] indices of components to identify; can be null
 * \param[out]   ppixdb [optional] pix showing inputs and best fits
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) See recogIdentifyPix().  This does the same operation
 *          for each pix in a pixa, and optionally returns the arrays
 *          of results (scores, class index and character string)
 *          for the best correlation match.
 * </pre>
 */
int32 recogIdentifyPixa(L_RECOG  * recog,
    PIXA     * pixa,
    NUMA     * naid,
    PIX     ** ppixdb)
{
	char      * text;
	int32 i, n, doit, fail, index, depth;
	float score;
	NUMA      * naidt;
	PIX       * pix1, * pix2, * pix3;
	PIXA      * pixa1;
	L_RCH     * rch;

	PROCNAME("recogIdentifyPixa");

	if(ppixdb) *ppixdb = NULL;
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!pixa)
		return ERROR_INT("pixa not defined", procName, 1);

	/* Set up the components to run through the recognizer */
	n = pixaGetCount(pixa);
	if(naid)
		naidt = numaClone(naid);
	else
		naidt = numaMakeConstant(1, n);

	/* Run the recognizer on the set of images.  This writes
	 * the text string into each pix in pixa.  If this is called
	 * multiple times for different recognizers, the text string
	 * will be overwritten, but it will be finalized with the correct
	 * string from the cha in the recoga, using recogaSaveBestCha(). */
	rchaDestroy(&recog->rcha);
	recog->rcha = rchaCreate();
	pixa1 = (ppixdb) ? pixaCreate(n) : NULL;
	depth = 1;
	for(i = 0; i < n; i++) {
		pix1 = pixaGetPix(pixa, i, L_CLONE);
		pix2 = NULL;
		fail = FALSE;
		numaGetIValue(naidt, i, &doit);
		if(!doit)
			recogSkipIdentify(recog);
		else if(!ppixdb)
			fail = recogIdentifyPix(recog, pix1, NULL);
		else
			fail = recogIdentifyPix(recog, pix1, &pix2);
		if(fail)
			recogSkipIdentify(recog);
		if((rch = recog->rch) == NULL) {
			L_ERROR2("rch not found for char %d\n", procName, i);
			pixDestroy(&pix1);
			pixDestroy(&pix2);
			continue;
		}
		rchExtract(rch, NULL, NULL, &text, NULL, NULL, NULL, NULL);
		pixSetText(pix1, text);
		LEPT_FREE(text);
		if(ppixdb && doit) {
			rchExtract(rch, &index, &score, NULL, NULL, NULL, NULL, NULL);
			pix3 = recogShowMatch(recog, pix2, NULL, NULL, index, score);
			if(i == 0) depth = pixGetDepth(pix3);
			pixaAddPix(pixa1, pix3, L_INSERT);
			pixDestroy(&pix2);
		}
		transferRchToRcha(rch, recog->rcha);
		pixDestroy(&pix1);
	}
	numaDestroy(&naidt);

	/* Package the images for debug */
	if(ppixdb) {
		*ppixdb = pixaDisplayTiledInRows(pixa1, depth, 2500, 1.0, 0, 20, 1);
		pixaDestroy(&pixa1);
	}

	return 0;
}

/*!
 * \brief   recogIdentifyPix()
 *
 * \param[in]    recog with LUT's pre-computed
 * \param[in]    pixs of a single character, 1 bpp
 * \param[out]   ppixdb [optional] debug pix showing input and best fit
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Basic recognition function for a single character.
 *      (2) If L_USE_ALL, matching is attempted to every bitmap in the recog,
 *          and the identify of the best match is returned.  However,
 *          if L_USE_AVERAGE, the matching is only to the averaged bitmaps,
 *          and the index of the bestsample is meaningless (0 is returned
 *          if requested).
 *      (3) The score is related to the confidence (probability of correct
 *          identification), in that a higher score is correlated with
 *          a higher probability.  However, the actual relation between
 *          the correlation (score) and the probability is not known;
 *          we call this a "score" because "confidence" can be misinterpreted
 *          as an actual probability.
 * </pre>
 */
int32 recogIdentifyPix(L_RECOG  * recog,
    PIX      * pixs,
    PIX     ** ppixdb)
{
	char      * text;
	int32 i, j, n, bestindex, bestsample, area1, area2;
	int32 shiftx, shifty, bestdelx, bestdely, bestwidth, maxyshift;
	float x1, y1, x2, y2, delx, dely, score, maxscore;
	NUMA      * numa;
	PIX       * pix0, * pix1, * pix2;
	PIXA      * pixa;
	PTA       * pta;

	PROCNAME("recogIdentifyPix");

	if(ppixdb) *ppixdb = NULL;
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!pixs || pixGetDepth(pixs) != 1)
		return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

	/* Do the averaging if not yet done.  This will also
	 * call recogFinishTraining(), if necessary. */
	if(!recog->ave_done)
		recogAverageSamples(recog, 0);

	/* Binarize and crop to foreground if necessary */
	if((pix0 = recogProcessToIdentify(recog, pixs, 0)) == NULL)
		return ERROR_INT("no fg pixels in pix0", procName, 1);

	/* Do correlation at all positions within +-maxyshift of
	 * the nominal centroid alignment. */
	pix1 = recogScaleCharacter(recog, pix0);
	pixCountPixels(pix1, &area1, recog->sumtab);
	pixCentroid(pix1, recog->centtab, recog->sumtab, &x1, &y1);
	bestindex = bestsample = bestdelx = bestdely = bestwidth = 0;
	maxscore = 0.0;
	maxyshift = recog->maxyshift;
	if(recog->templ_type == L_USE_AVERAGE) {
		for(i = 0; i < recog->setsize; i++) {
			numaGetIValue(recog->nasum, i, &area2);
			if(area2 == 0) continue;  /* no template available */
			pix2 = pixaGetPix(recog->pixa, i, L_CLONE);
			ptaGetPt(recog->pta, i, &x2, &y2);
			delx = x1 - x2;
			dely = y1 - y2;
			for(shifty = -maxyshift; shifty <= maxyshift; shifty++) {
				for(shiftx = -maxyshift; shiftx <= maxyshift; shiftx++) {
					pixCorrelationScoreSimple(pix1, pix2, area1, area2,
					    delx + shiftx, dely + shifty,
					    5, 5, recog->sumtab, &score);
					if(score > maxscore) {
						bestindex = i;
						bestdelx = delx + shiftx;
						bestdely = dely + shifty;
						maxscore = score;
					}
				}
			}
			pixDestroy(&pix2);
		}
	}
	else { /* use all the samples */
		for(i = 0; i < recog->setsize; i++) {
			pixa = pixaaGetPixa(recog->pixaa, i, L_CLONE);
			n = pixaGetCount(pixa);
			if(n == 0) {
				pixaDestroy(&pixa);
				continue;
			}
			numa = numaaGetNuma(recog->naasum, i, L_CLONE);
			pta = ptaaGetPta(recog->ptaa, i, L_CLONE);
			for(j = 0; j < n; j++) {
				pix2 = pixaGetPix(pixa, j, L_CLONE);
				numaGetIValue(numa, j, &area2);
				ptaGetPt(pta, j, &x2, &y2);
				delx = x1 - x2;
				dely = y1 - y2;
				for(shifty = -maxyshift; shifty <= maxyshift; shifty++) {
					for(shiftx = -maxyshift; shiftx <= maxyshift; shiftx++) {
						pixCorrelationScoreSimple(pix1, pix2, area1, area2,
						    delx + shiftx, dely + shifty,
						    5, 5, recog->sumtab, &score);
						if(score > maxscore) {
							bestindex = i;
							bestsample = j;
							bestdelx = delx + shiftx;
							bestdely = dely + shifty;
							maxscore = score;
							bestwidth = pixGetWidth(pix2);
						}
					}
				}
				pixDestroy(&pix2);
			}
			pixaDestroy(&pixa);
			numaDestroy(&numa);
			ptaDestroy(&pta);
		}
	}

	/* Package up the results */
	recogGetClassString(recog, bestindex, &text);
	rchDestroy(&recog->rch);
	recog->rch = rchCreate(bestindex, maxscore, text, bestsample, bestdelx, bestdely, bestwidth);

	if(ppixdb) {
		if(recog->templ_type == L_USE_AVERAGE) {
			L_INFO6("Best match: str %s; class %d; sh (%d, %d); score %5.3f\n", procName, text, bestindex, bestdelx, bestdely, maxscore);
			pix2 = pixaGetPix(recog->pixa, bestindex, L_CLONE);
		}
		else { /* L_USE_ALL */
			L_INFO5("Best match: str %s; sample %d in class %d; score %5.3f\n", procName, text, bestsample, bestindex, maxscore);
			if(maxyshift > 0) {
				L_INFO3("  Best shift: (%d, %d)\n", procName, bestdelx, bestdely);
			}
			pix2 = pixaaGetPix(recog->pixaa, bestindex, bestsample, L_CLONE);
		}
		*ppixdb = recogShowMatch(recog, pix1, pix2, NULL, -1, 0.0);
		pixDestroy(&pix2);
	}

	pixDestroy(&pix0);
	pixDestroy(&pix1);
	return 0;
}

/*!
 * \brief   recogSkipIdentify()
 *
 * \param[in]    recog
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This just writes a "dummy" result with 0 score and empty
 *          string id into the rch.
 * </pre>
 */
int32 recogSkipIdentify(L_RECOG  * recog)
{
	PROCNAME("recogSkipIdentify");

	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);

	/* Package up placeholder results */
	rchDestroy(&recog->rch);
	recog->rch = rchCreate(0, 0.0, stringNew(""), 0, 0, 0, 0);
	return 0;
}

/*------------------------------------------------------------------------*
*             Operations for handling identification results             *
*------------------------------------------------------------------------*/
/*!
 * \brief   rchaCreate()
 *
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Be sure to destroy any existing rcha before assigning this.
 */
static L_RCHA * rchaCreate()
{
	L_RCHA  * rcha;

	rcha = (L_RCHA*)LEPT_CALLOC(1, sizeof(L_RCHA));
	rcha->naindex = numaCreate(0);
	rcha->nascore = numaCreate(0);
	rcha->satext = sarrayCreate(0);
	rcha->nasample = numaCreate(0);
	rcha->naxloc = numaCreate(0);
	rcha->nayloc = numaCreate(0);
	rcha->nawidth = numaCreate(0);
	return rcha;
}

/*!
 * \brief   rchaDestroy()
 *
 * \param[in,out]  prcha to be nulled
 */
void rchaDestroy(L_RCHA  ** prcha)
{
	L_RCHA  * rcha;

	PROCNAME("rchaDestroy");

	if(prcha == NULL) {
		L_WARNING("&rcha is null!\n", procName);
		return;
	}
	if((rcha = *prcha) == NULL)
		return;

	numaDestroy(&rcha->naindex);
	numaDestroy(&rcha->nascore);
	sarrayDestroy(&rcha->satext);
	numaDestroy(&rcha->nasample);
	numaDestroy(&rcha->naxloc);
	numaDestroy(&rcha->nayloc);
	numaDestroy(&rcha->nawidth);
	LEPT_FREE(rcha);
	*prcha = NULL;
	return;
}

/*!
 * \brief   rchCreate()
 *
 * \param[in]    index index of best template
 * \param[in]    score correlation score of best template
 * \param[in]    text character string of best template
 * \param[in]    sample index of best sample; -1 if averages are used
 * \param[in]    xloc x-location of template: delx + shiftx
 * \param[in]    yloc y-location of template: dely + shifty
 * \param[in]    width width of best template
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Be sure to destroy any existing rch before assigning this.
 *      (2) This stores the text string, not a copy of it, so the
 *          caller must not destroy the string.
 * </pre>
 */
static L_RCH * rchCreate(int32 index,
    float score,
    char      * text,
    int32 sample,
    int32 xloc,
    int32 yloc,
    int32 width)
{
	L_RCH  * rch;

	rch = (L_RCH*)LEPT_CALLOC(1, sizeof(L_RCH));
	rch->index = index;
	rch->score = score;
	rch->text = text;
	rch->sample = sample;
	rch->xloc = xloc;
	rch->yloc = yloc;
	rch->width = width;
	return rch;
}

/*!
 * \brief   rchDestroy()
 *
 * \param[in,out] prch to be nulled
 */
void rchDestroy(L_RCH  ** prch)
{
	L_RCH  * rch;

	PROCNAME("rchDestroy");

	if(prch == NULL) {
		L_WARNING("&rch is null!\n", procName);
		return;
	}
	if((rch = *prch) == NULL)
		return;
	LEPT_FREE(rch->text);
	LEPT_FREE(rch);
	*prch = NULL;
	return;
}

/*!
 * \brief   rchaExtract()
 *
 * \param[in]    rcha
 * \param[out]   pnaindex [optional] indices of best templates
 * \param[out]   pnascore [optional] correl scores of best templates
 * \param[out]   psatext [optional] character strings of best templates
 * \param[out]   pnasample [optional] indices of best samples
 * \param[out]   pnaxloc [optional] x-locations of templates
 * \param[out]   pnayloc [optional] y-locations of templates
 * \param[out]   pnawidth [optional] widths of best templates
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This returns clones of the number and string arrays.  They must
 *          be destroyed by the caller.
 * </pre>
 */
int32 rchaExtract(L_RCHA   * rcha,
    NUMA    ** pnaindex,
    NUMA    ** pnascore,
    SARRAY  ** psatext,
    NUMA    ** pnasample,
    NUMA    ** pnaxloc,
    NUMA    ** pnayloc,
    NUMA    ** pnawidth)
{
	PROCNAME("rchaExtract");

	if(pnaindex) *pnaindex = NULL;
	if(pnascore) *pnascore = NULL;
	if(psatext) *psatext = NULL;
	if(pnasample) *pnasample = NULL;
	if(pnaxloc) *pnaxloc = NULL;
	if(pnayloc) *pnayloc = NULL;
	if(pnawidth) *pnawidth = NULL;
	if(!rcha)
		return ERROR_INT("rcha not defined", procName, 1);

	if(pnaindex) *pnaindex = numaClone(rcha->naindex);
	if(pnascore) *pnascore = numaClone(rcha->nascore);
	if(psatext) *psatext = sarrayClone(rcha->satext);
	if(pnasample) *pnasample = numaClone(rcha->nasample);
	if(pnaxloc) *pnaxloc = numaClone(rcha->naxloc);
	if(pnayloc) *pnayloc = numaClone(rcha->nayloc);
	if(pnawidth) *pnawidth = numaClone(rcha->nawidth);
	return 0;
}

/*!
 * \brief   rchExtract()
 *
 * \param[in]    rch
 * \param[out]   pindex [optional] index of best template
 * \param[out]   pscore [optional] correlation score of best template
 * \param[out]   ptext [optional] character string of best template
 * \param[out]   psample [optional] index of best sample
 * \param[out]   pxloc [optional] x-location of template
 * \param[out]   pyloc [optional] y-location of template
 * \param[out]   pwidth [optional] width of best template
 * \return  0 if OK, 1 on error
 */
int32 rchExtract(L_RCH      * rch,
    int32    * pindex,
    float  * pscore,
    char      ** ptext,
    int32    * psample,
    int32    * pxloc,
    int32    * pyloc,
    int32    * pwidth)
{
	PROCNAME("rchExtract");

	if(pindex) *pindex = 0;
	if(pscore) *pscore = 0.0;
	if(ptext) *ptext = NULL;
	if(psample) *psample = 0;
	if(pxloc) *pxloc = 0;
	if(pyloc) *pyloc = 0;
	if(pwidth) *pwidth = 0;
	if(!rch)
		return ERROR_INT("rch not defined", procName, 1);

	if(pindex) *pindex = rch->index;
	if(pscore) *pscore = rch->score;
	if(ptext) *ptext = stringNew(rch->text);  /* new string: owned by caller */
	if(psample) *psample = rch->sample;
	if(pxloc) *pxloc = rch->xloc;
	if(pyloc) *pyloc = rch->yloc;
	if(pwidth) *pwidth = rch->width;
	return 0;
}

/*!
 * \brief   transferRchToRcha()
 *
 * \param[in]    rch source of data
 * \param[in]    rcha append to arrays in this destination
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This is used to transfer the results of a single character
 *          identification to an rcha array for the array of characters.
 * </pre>
 */
static int32 transferRchToRcha(L_RCH   * rch,
    L_RCHA  * rcha)
{
	PROCNAME("transferRchToRcha");

	if(!rch)
		return ERROR_INT("rch not defined", procName, 1);
	if(!rcha)
		return ERROR_INT("rcha not defined", procName, 1);

	numaAddNumber(rcha->naindex, rch->index);
	numaAddNumber(rcha->nascore, rch->score);
	sarrayAddString(rcha->satext, rch->text, L_COPY);
	numaAddNumber(rcha->nasample, rch->sample);
	numaAddNumber(rcha->naxloc, rch->xloc);
	numaAddNumber(rcha->nayloc, rch->yloc);
	numaAddNumber(rcha->nawidth, rch->width);
	return 0;
}

/*!
 * \brief   recogaSaveBestRcha()
 *
 * \param[in]    recoga
 * \param[in]    pixa with all components having been identified
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Finds the best score among the recognizers for each character,
 *          and puts the rch data into a rcha in the recoga.  This is
 *          run after all recognizers have been applied to the pixa.
 *      (2) This also writes the best text id for each pix into its text field.
 * </pre>
 */
static int32 recogaSaveBestRcha(L_RECOGA  * recoga,
    PIXA      * pixa)
{
	char      * text;
	int32 i, j, npix, nrec, jmax;
	float score, maxscore;
	L_RECOG   * recog;
	PIX       * pix;
	L_RCHA    * rcha;
	SARRAY    * satext;

	PROCNAME("recogaSaveBestRcha");

	if(!recoga)
		return ERROR_INT("recoga not defined", procName, 1);
	if(!pixa)
		return ERROR_INT("pixa not defined", procName, 1);

	/* Make a clean rcha to accept the results */
	rchaDestroy(&recoga->rcha);
	recoga->rcha = rchaCreate();

	npix = pixaGetCount(pixa);
	nrec = recogaGetCount(recoga);
	for(i = 0; i < npix; i++) {
		/* Find the recog in the recoga with the best score */
		maxscore = 0.0;
		jmax = 0;
		for(j = 0; j < nrec; j++) {
			if((recog = recogaGetRecog(recoga, j)) == NULL) {
				L_ERROR2("recog %d not found\n", procName, j);
				continue;
			}
			if((rcha = recog->rcha) == NULL) {
				L_ERROR2("rcha not found for recog %d\n", procName, j);
				continue;
			}
			numaGetFValue(rcha->nascore, i, &score);
			if(score > maxscore) {
				maxscore = score;
				jmax = j;
			}
		}
		recog = recogaGetRecog(recoga, jmax);

		/* Transfer the data for this char to the recoga */
		recogaTransferRch(recoga, recog, i);
	}

	/* Write the best text string for each pix into the pixa */
	if((rcha = recoga->rcha) == NULL)
		return ERROR_INT("rcha not found!", procName, 1);
	rchaExtract(rcha, NULL, NULL, &satext, NULL, NULL, NULL, NULL);
	for(i = 0; i < npix; i++) {
		pix = pixaGetPix(pixa, i, L_CLONE);
		text = sarrayGetString(satext, i, L_NOCOPY);
		pixSetText(pix, text);
		pixDestroy(&pix);
	}
	sarrayDestroy(&satext); /* it's a clone */

	return 0;
}

/*!
 * \brief   recogaTransferRch()
 *
 * \param[in]    recoga destination, with rcha defined
 * \param[in]    recog source, with best scoring char in its rcha
 * \param[in]    index index of component in the original pixa
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This is called by recogaGetBestRcha() to transfer the results
 *          of a single character identification in a selected recog to the
 *          rcha array in the recoga, which holds the best scoring characters.
 * </pre>
 */
static int32 recogaTransferRch(L_RECOGA  * recoga,
    L_RECOG   * recog,
    int32 index)
{
	char      * str;
	int32 ival;
	float fval;
	L_RCHA    * rchas, * rchad;

	PROCNAME("recogaTransferRch");

	if(!recoga)
		return ERROR_INT("recoga not defined", procName, 1);
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);

	rchas = recog->rcha;
	rchad = recoga->rcha;
	numaGetIValue(rchas->naindex, index, &ival);
	numaAddNumber(rchad->naindex, ival);
	numaGetFValue(rchas->nascore, index, &fval);
	numaAddNumber(rchad->nascore, fval);
	str = sarrayGetString(rchas->satext, index, L_COPY);
	sarrayAddString(rchad->satext, str, L_INSERT);
	numaGetIValue(rchas->nasample, index, &ival);
	numaAddNumber(rchad->nasample, ival);
	numaGetIValue(rchas->naxloc, index, &ival);
	numaAddNumber(rchad->naxloc, ival);
	numaGetIValue(rchas->nayloc, index, &ival);
	numaAddNumber(rchad->nayloc, ival);
	numaGetIValue(rchas->nawidth, index, &ival);
	numaAddNumber(rchad->nawidth, ival);
	return 0;
}

/*------------------------------------------------------------------------*
*                        Preprocessing and filtering                     *
*------------------------------------------------------------------------*/
/*!
 * \brief   recogProcessToIdentify()
 *
 * \param[in]    recog with LUT's pre-computed
 * \param[in]    pixs typ. single character, possibly d > 1 and uncropped
 * \param[in]    pad extra pixels added to left and right sides
 * \return  pixd 1 bpp, clipped to foreground, or NULL if there
 *                    are no fg pixels or on error.
 *
 * <pre>
 * Notes:
 *      (1) This is a lightweight operation to insure that the input
 *          image is 1 bpp, properly cropped, and padded on each side.
 *          If bpp \> 1, the image is thresholded.
 * </pre>
 */
PIX * recogProcessToIdentify(L_RECOG  * recog,
    PIX      * pixs,
    int32 pad)
{
	int32 canclip;
	PIX     * pix1, * pix2, * pixd;

	PROCNAME("recogProcessToIdentify");

	if(!recog)
		return (PIX*)ERROR_PTR("recog not defined", procName, NULL);
	if(!pixs)
		return (PIX*)ERROR_PTR("pixs not defined", procName, NULL);

	if(pixGetDepth(pixs) != 1)
		pix1 = pixThresholdToBinary(pixs, recog->threshold);
	else
		pix1 = pixClone(pixs);
	pixTestClipToForeground(pix1, &canclip);
	if(canclip)
		pixClipToForeground(pix1, &pix2, NULL);
	else
		pix2 = pixClone(pix1);
	pixDestroy(&pix1);
	if(!pix2)
		return (PIX*)ERROR_PTR("no foreground pixels", procName, NULL);

	pixd = pixAddBorderGeneral(pix2, pad, pad, 0, 0, 0);
	pixDestroy(&pix2);
	return pixd;
}

/*!
 * \brief   recogPreSplittingFilter()
 *
 * \param[in]    recog
 * \param[in]    pixs 1 bpp, single connected component
 * \param[in]    maxasp maximum asperity ratio (width/height) to be retained
 * \param[in]    minaf minimum area fraction (|fg|/(w*h)) to be retained
 * \param[in]    debug 1 to output indicator arrays
 * \return  pixd with filtered components removed or NULL on error
 */
PIX * recogPreSplittingFilter(L_RECOG   * recog,
    PIX       * pixs,
    float maxasp,
    float minaf,
    int32 debug)
{
	int32 scaling, minsplitw, minsplith, maxsplith;
	BOXA    * boxas;
	NUMA    * naw, * nah, * na1, * na1c, * na2, * na3, * na4, * na5, * na6, * na7;
	PIX     * pixd;
	PIXA    * pixas;

	PROCNAME("recogPreSplittingFilter");

	if(!recog)
		return (PIX*)ERROR_PTR("recog not defined", procName, NULL);
	if(!pixs)
		return (PIX*)ERROR_PTR("pixs not defined", procName, NULL);

	/* If there is scaling, do not remove components based on the
	 * values of min_splitw, min_splith and max_splith. */
	scaling = (recog->scalew > 0 || recog->scaleh > 0) ? TRUE : FALSE;
	minsplitw = (scaling) ? 1 : recog->min_splitw - 3;
	minsplith = (scaling) ? 1 : recog->min_splith - 3;
	maxsplith = (scaling) ? 200 : recog->max_splith;

	/* Generate an indicator array of connected components to remove:
	 *    small stuff
	 *    tall stuff
	 *    components with large width/height ratio
	 *    components with small area fill fraction  */
	boxas = pixConnComp(pixs, &pixas, 8);
	pixaFindDimensions(pixas, &naw, &nah);
	na1 = numaMakeThresholdIndicator(naw, minsplitw, L_SELECT_IF_LT);
	na1c = numaCopy(na1);
	na2 = numaMakeThresholdIndicator(nah, minsplith, L_SELECT_IF_LT);
	na3 = numaMakeThresholdIndicator(nah, maxsplith, L_SELECT_IF_GT);
	na4 = pixaFindWidthHeightRatio(pixas);
	na5 = numaMakeThresholdIndicator(na4, maxasp, L_SELECT_IF_GT);
	na6 = pixaFindAreaFraction(pixas);
	na7 = numaMakeThresholdIndicator(na6, minaf, L_SELECT_IF_LT);
	numaLogicalOp(na1, na1, na2, L_UNION);
	numaLogicalOp(na1, na1, na3, L_UNION);
	numaLogicalOp(na1, na1, na5, L_UNION);
	numaLogicalOp(na1, na1, na7, L_UNION);
	pixd = pixCopy(NULL, pixs);
	pixRemoveWithIndicator(pixd, pixas, na1);
	if(debug)
		l_showIndicatorSplitValues(na1c, na2, na3, na5, na7, na1);
	numaDestroy(&naw);
	numaDestroy(&nah);
	numaDestroy(&na1);
	numaDestroy(&na1c);
	numaDestroy(&na2);
	numaDestroy(&na3);
	numaDestroy(&na4);
	numaDestroy(&na5);
	numaDestroy(&na6);
	numaDestroy(&na7);
	boxaDestroy(&boxas);
	pixaDestroy(&pixas);
	return pixd;
}

/*!
 * \brief   recogSplittingFilter()
 *
 * \param[in]    recog
 * \param[in]    pixs 1 bpp, single connected component
 * \param[in]    maxasp maximum asperity ratio (width/height) to be retained
 * \param[in]    minaf minimum area fraction (|fg|/(w*h)) to be retained
 * \param[out]   premove 0 to save, 1 to remove
 * \param[in]    debug 1 to output indicator arrays
 * \return  0 if OK, 1 on error
 */
int32 recogSplittingFilter(L_RECOG   * recog,
    PIX       * pixs,
    float maxasp,
    float minaf,
    int32   * premove,
    int32 debug)
{
	int32 w, h;
	float aspratio, fract;

	PROCNAME("recogSplittingFilter");

	if(!premove)
		return ERROR_INT("&remove not defined", procName, 1);
	*premove = 0;
	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(!pixs)
		return ERROR_INT("pixs not defined", procName, 1);

	/* Remove from further consideration:
	 *    small stuff
	 *    components with large width/height ratio
	 *    components with small area fill fraction */
	pixGetDimensions(pixs, &w, &h, NULL);
	if(w < recog->min_splitw) {
		if(debug) 
			L_INFO3("w = %d < %d\n", procName, w, recog->min_splitw);
		*premove = 1;
		return 0;
	}
	if(h < recog->min_splith) {
		if(debug) 
			L_INFO3("h = %d < %d\n", procName, h, recog->min_splith);
		*premove = 1;
		return 0;
	}
	aspratio = (float)w / (float)h;
	if(aspratio > maxasp) {
		if(debug) 
			L_INFO2("w/h = %5.3f too large\n", procName, aspratio);
		*premove = 1;
		return 0;
	}
	pixFindAreaFraction(pixs, recog->sumtab, &fract);
	if(fract < minaf) {
		if(debug) 
			L_INFO3("area fill fract %5.3f < %5.3f\n", procName, fract, minaf);
		*premove = 1;
		return 0;
	}
	return 0;
}

/*------------------------------------------------------------------------*
*                              Postprocessing                            *
*------------------------------------------------------------------------*/
/*!
 * \brief   recogaExtractNumbers()
 *
 * \param[in]    recoga
 * \param[in]    boxas location of components
 * \param[in]    scorethresh min score for which we accept a component
 * \param[in]    spacethresh max horizontal distance allowed between digits,
 *                           use -1 for default
 * \param[out]   pbaa [optional] bounding boxes of identified numbers
 * \param[out]   pnaa [optional] scores of identified digits
 * \return  sa of identified numbers, or NULL on error
 *
 * <pre>
 * Notes:
 *      (1) This extracts digit data after recogaIdentifyMultiple() or
 *          lower-level identification has taken place.
 *      (2) Each string in the returned sa contains a sequence of ascii
 *          digits in a number.
 *      (3) The horizontal distance between boxes (limited by %spacethresh)
 *          is the negative of the horizontal overlap.
 *      (4) Components with a score less than %scorethresh, which may
 *          be hyphens or other small characters, will signal the
 *          end of the current sequence of digits in the number.  A typical
 *          value for %scorethresh is 0.60.
 *      (5) We allow two digits to be combined if these conditions apply:
 *            (a) the first is to the left of the second
 *            (b) the second has a horizontal separation less than %spacethresh
 *            (c) the vertical overlap \>= 0 (vertical separation \< 0)
 *            (d) both have a score that exceeds %scorethresh
 *      (6) Each numa in the optionally returned naa contains the digit
 *          scores of a number.  Each boxa in the optionally returned baa
 *          contains the bounding boxes of the digits in the number.
 * </pre>
 */
SARRAY * recogaExtractNumbers(L_RECOGA  * recoga,
    BOXA      * boxas,
    float scorethresh,
    int32 spacethresh,
    BOXAA    ** pbaa,
    NUMAA    ** pnaa)
{
	char      * str, * text;
	int32 i, n, x1, x2, h_sep, v_sep;
	float score;
	BOX       * box, * prebox;
	BOXA      * ba;
	BOXAA     * baa;
	NUMA      * nascore, * na;
	NUMAA     * naa;
	L_RECOG   * recog;
	SARRAY    * satext, * sa, * saout;

	PROCNAME("recogaExtractNumbers");

	if(pbaa) *pbaa = NULL;
	if(pnaa) *pnaa = NULL;
	if(!recoga || !recoga->rcha)
		return (SARRAY*)ERROR_PTR("recoga and rcha not both defined",
		    procName, NULL);
	if(!boxas)
		return (SARRAY*)ERROR_PTR("boxas not defined", procName, NULL);

	if(spacethresh < 0) {
		if((recog = recogaGetRecog(recoga, 0)) == NULL)
			return (SARRAY*)ERROR_PTR("recog not found", procName, NULL);
		spacethresh = MAX(recog->maxheight_u, 20);
	}
	rchaExtract(recoga->rcha, NULL, &nascore, &satext, NULL, NULL, NULL, NULL);
	if(!nascore || !satext) {
		numaDestroy(&nascore);
		sarrayDestroy(&satext);
		return (SARRAY*)ERROR_PTR("nascore and satext not both returned",
		    procName, NULL);
	}

	saout = sarrayCreate(0);
	naa = numaaCreate(0);
	baa = boxaaCreate(0);
	prebox = NULL;
	n = numaGetCount(nascore);
	for(i = 0; i < n; i++) {
		numaGetFValue(nascore, i, &score);
		text = sarrayGetString(satext, i, L_NOCOPY);
		if(prebox == NULL) { /* no current run */
			if(score < scorethresh) {
				continue;
			}
			else { /* start a number run */
				sa = sarrayCreate(0);
				ba = boxaCreate(0);
				na = numaCreate(0);
				sarrayAddString(sa, text, L_COPY);
				prebox = boxaGetBox(boxas, i, L_CLONE);
				boxaAddBox(ba, prebox, L_COPY);
				numaAddNumber(na, score);
			}
		}
		else { /* in a current number run */
			box = boxaGetBox(boxas, i, L_CLONE);
			boxGetGeometry(prebox, &x1, NULL, NULL, NULL);
			boxGetGeometry(box, &x2, NULL, NULL, NULL);
			boxSeparationDistance(box, prebox, &h_sep, &v_sep);
			boxDestroy(&prebox);
			if(x1 < x2 && h_sep <= spacethresh &&
			    v_sep < 0 && score >= scorethresh) { /* add to number */
				sarrayAddString(sa, text, L_COPY);
				boxaAddBox(ba, box, L_COPY);
				numaAddNumber(na, score);
				prebox = box;
			}
			else { /* save the completed number */
				str = sarrayToString(sa, 0);
				sarrayAddString(saout, str, L_INSERT);
				sarrayDestroy(&sa);
				boxaaAddBoxa(baa, ba, L_INSERT);
				numaaAddNuma(naa, na, L_INSERT);
				boxDestroy(&box);
				if(score >= scorethresh) { /* start a new number */
					i--;
					continue;
				}
			}
		}
	}

	if(prebox) { /* save the last number */
		str = sarrayToString(sa, 0);
		sarrayAddString(saout, str, L_INSERT);
		boxaaAddBoxa(baa, ba, L_INSERT);
		numaaAddNuma(naa, na, L_INSERT);
		sarrayDestroy(&sa);
		boxDestroy(&prebox);
	}

	numaDestroy(&nascore);
	sarrayDestroy(&satext);
	if(sarrayGetCount(saout) == 0) {
		sarrayDestroy(&saout);
		boxaaDestroy(&baa);
		numaaDestroy(&naa);
		return (SARRAY*)ERROR_PTR("saout has no strings", procName, NULL);
	}

	if(pbaa)
		*pbaa = baa;
	else
		boxaaDestroy(&baa);
	if(pnaa)
		*pnaa = naa;
	else
		numaaDestroy(&naa);
	return saout;
}

/*------------------------------------------------------------------------*
*                         Modifying recog behavior                       *
*------------------------------------------------------------------------*/
/*!
 * \brief   recogSetTemplateType()
 *
 * \param[in]    recog
 * \param[in]    templ_type L_USE_AVERAGE or L_USE_ALL
 * \return  0 if OK, 1 on error
 */
int32 recogSetTemplateType(L_RECOG  * recog,
    int32 templ_type)
{
	PROCNAME("recogSetTemplateType");

	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(templ_type != L_USE_AVERAGE && templ_type != L_USE_ALL)
		return ERROR_INT("invalid templ_type", procName, 1);

	recog->templ_type = templ_type;
	return 0;
}

/*!
 * \brief   recogSetScaling()
 *
 * \param[in]    recog
 * \param[in]    scalew  scale all widths to this; use 0 for no scaling
 * \param[in]    scaleh  scale all heights to this; use 0 for no scaling
 * \param[in]    templ_type L_USE_AVERAGE or L_USE_ALL
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This is entirely equivalent to extracting the unscaled
 *          pixa from the recog, and creating a new recog using it:
 *            pixa = recogExtractPixa(recog1);
 *            recog2 = recogCreateFromPixa(pixa, 0, 0, L_USE_ALL, 128, 1);
 *          This is useful when making a book adapted recognizer (BAR)
 *          from the training results with a boot recognizer (BR).
 *          The BR is used to train the BAR on unlabelled images that
 *          are scaled to a fixed dimension.  Once trained, the BAR
 *          works best if converted to identify unscaled images.
 * </pre>
 */
int32 recogSetScaling(L_RECOG  * recog,
    int32 scalew,
    int32 scaleh,
    int32 templ_type)
{
	PROCNAME("recogSetScaling");

	if(!recog)
		return ERROR_INT("recog not defined", procName, 1);
	if(scalew < 0 || scaleh < 0)
		return ERROR_INT("invalid scalew or scaleh", procName, 1);
	if(scalew == recog->scalew && scaleh == recog->scaleh) {
		L_INFO("scaling factors not changed\n", procName);
		return 0;
	}
	if(templ_type != L_USE_AVERAGE && templ_type != L_USE_ALL)
		return ERROR_INT("invalid templ_type", procName, 1);

	recog->scalew = scalew;
	recog->scaleh = scaleh;
	recog->templ_type = templ_type;
	recog->train_done = FALSE;

	/* Restock the scaled character images and recompute all averages */
	recogTrainingFinished(recog, 0);
	return 0;
}

/*------------------------------------------------------------------------*
*                        Static debug helper                             *
*------------------------------------------------------------------------*/
/*!
 * \brief   l_showIndicatorSplitValues()
 *
 * \param[in]  na1, na2, na3, na4, na5, na6  6 indicator array
 *
 * <pre>
 * Notes:
 *      (1) The values indicate that specific criteria has been met
 *          for component removal by pre-splitting filter..
 *          The 'result' line shows which components have been removed.
 * </pre>
 */
static void l_showIndicatorSplitValues(NUMA  * na1,
    NUMA  * na2,
    NUMA  * na3,
    NUMA  * na4,
    NUMA  * na5,
    NUMA  * na6)
{
	int32 i, n;

	n = numaGetCount(na1);
	fprintf(stderr, "================================================\n");
	fprintf(stderr, "lt minw:    ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na1->array[i]);
	fprintf(stderr, "\nlt minh:    ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na2->array[i]);
	fprintf(stderr, "\ngt maxh:    ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na3->array[i]);
	fprintf(stderr, "\ngt maxasp:  ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na4->array[i]);
	fprintf(stderr, "\nlt minaf:   ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na5->array[i]);
	fprintf(stderr, "\n------------------------------------------------");
	fprintf(stderr, "\nresult:     ");
	for(i = 0; i < n; i++)
		fprintf(stderr, "%4d ", (int32)na6->array[i]);
	fprintf(stderr, "\n================================================\n");
}

