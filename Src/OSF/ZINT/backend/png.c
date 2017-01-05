/* png.c - Handles output to PNG file */

/*
    libzint - the open source barcode library
    Copyright (C) 2009-2016 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */

#include "common.h"
#ifdef _MSC_VER
	#include <fcntl.h>
	#include <io.h>
#endif
#ifdef _MSC_VER
	#include <malloc.h>
#endif /* _MSC_VER */
#ifndef NO_PNG
	#include <libpng/png.h>
	#include <zlib.h>
	#include <setjmp.h>
#endif /* NO_PNG */
#include "maxipng.h"    /* Maxicode shapes */
#include "font.h"       /* Font for human readable text */

#define SSET    "0123456789ABCDEF"

#define PNG_DATA        100
#define BMP_DATA        200

#ifndef NO_PNG

struct mainprog_info_type {
	long width;
	long height;
	FILE * outfile;
	jmp_buf jmpbuf;
};

static void writepng_error_handler(png_structp png_ptr, png_const_charp msg) 
{
	fprintf(stderr, "writepng libpng error: %s\n", msg);
	fflush(stderr);
	struct mainprog_info_type * graphic = (struct mainprog_info_type*)png_get_error_ptr(png_ptr);
	if(graphic == NULL) {
		/* we are completely hosed now */
		fprintf(stderr, "writepng severe error:  jmpbuf not recoverable; terminating.\n");
		fflush(stderr);
		return;
	}
	longjmp(graphic->jmpbuf, 1);
}

int png_pixel_plot(ZintSymbol * symbol, int image_height, int image_width, char * pixelbuf, int rotate_angle) 
{
	struct mainprog_info_type wpng_info;
	struct mainprog_info_type * graphic;
	png_structp png_ptr;
	png_infop info_ptr;
	uchar * image_data;
	int i, row, column, errno;
	int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
#ifndef _MSC_VER
	uchar outdata[image_width * 3];
#else
	uchar* outdata = (uchar*)_alloca(image_width * 3);
#endif
	graphic = &wpng_info;
	switch(rotate_angle) {
		case 0:
		case 180:
		    graphic->width = image_width;
		    graphic->height = image_height;
		    break;
		case 90:
		case 270:
		    graphic->width = image_height;
		    graphic->height = image_width;
		    break;
	}

	/* sort out colour options */
	to_upper((uchar*)symbol->fgcolour);
	to_upper((uchar*)symbol->bgcolour);

	if(strlen(symbol->fgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	if(strlen(symbol->bgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (uchar*)symbol->fgcolour, strlen(symbol->fgcolour));
	if(errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (uchar*)symbol->bgcolour, strlen(symbol->bgcolour));
	if(errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}

	fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
	fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
	fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
	bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
	bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
	bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

	/* Open output file in binary mode */
	if((symbol->output_options & BARCODE_STDOUT) != 0) {
#ifdef _MSC_VER
		if(-1 == _setmode(_fileno(stdout), _O_BINARY)) {
			strcpy(symbol->errtxt, "Can't open output file");
			return ZINT_ERROR_FILE_ACCESS;
		}
#endif
		graphic->outfile = stdout;
	}
	else {
		if(!(graphic->outfile = fopen(symbol->outfile, "wb"))) {
			strcpy(symbol->errtxt, "Can't open output file");
			return ZINT_ERROR_FILE_ACCESS;
		}
	}
	/* Set up error handling routine as proc() above */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, graphic, writepng_error_handler, NULL);
	if(!png_ptr) {
		strcpy(symbol->errtxt, "Out of memory");
		return ZINT_ERROR_MEMORY;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_write_struct(&png_ptr, NULL);
		strcpy(symbol->errtxt, "Out of memory");
		return ZINT_ERROR_MEMORY;
	}
	/* catch jumping here */
	if(setjmp(graphic->jmpbuf)) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		strcpy(symbol->errtxt, "libpng error occurred");
		return ZINT_ERROR_MEMORY;
	}
	/* open output file with libpng */
	png_init_io(png_ptr, graphic->outfile);
	/* set compression */
	// @sobolev png_set_compression_level(png_ptr, 9);
	/* set Header block */
	png_set_IHDR(png_ptr, info_ptr, graphic->width, graphic->height,
	    8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	/* write all chunks up to (but not including) first IDAT */
	png_write_info(png_ptr, info_ptr);
	/* set up the transformations:  for now, just pack low-bit-depth pixels
	   into bytes (one, two or four pixels per byte) */
	png_set_packing(png_ptr);
	/* Pixel Plotting */
	switch(rotate_angle) {
		case 0: /* Plot the right way up */
		    for(row = 0; row < image_height; row++) {
			    for(column = 0; column < image_width; column++) {
				    i = column * 3;
				    switch(*(pixelbuf + (image_width * row) + column)) {
					    case '1':
						outdata[i] = fgred;
						outdata[i + 1] = fggrn;
						outdata[i + 2] = fgblu;
						break;
					    default:
						outdata[i] = bgred;
						outdata[i + 1] = bggrn;
						outdata[i + 2] = bgblu;
						break;
				    }
			    }
			    /* write row contents to file */
			    image_data = outdata;
			    png_write_row(png_ptr, image_data);
		    }
		    break;
		case 90: /* Plot 90 degrees clockwise */
		    for(row = 0; row < image_width; row++) {
			    for(column = 0; column < image_height; column++) {
				    i = column * 3;
				    switch(*(pixelbuf + (image_width * (image_height - column - 1)) + row)) {
					    case '1':
						outdata[i] = fgred;
						outdata[i + 1] = fggrn;
						outdata[i + 2] = fgblu;
						break;
					    default:
						outdata[i] = bgred;
						outdata[i + 1] = bggrn;
						outdata[i + 2] = bgblu;
						break;
				    }
			    }

			    /* write row contents to file */
			    image_data = outdata;
			    png_write_row(png_ptr, image_data);
		    }
		    break;
		case 180: /* Plot upside down */
		    for(row = 0; row < image_height; row++) {
			    for(column = 0; column < image_width; column++) {
				    i = column * 3;
				    switch(*(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1))) {
					    case '1':
						outdata[i] = fgred;
						outdata[i + 1] = fggrn;
						outdata[i + 2] = fgblu;
						break;
					    default:
						outdata[i] = bgred;
						outdata[i + 1] = bggrn;
						outdata[i + 2] = bgblu;
						break;
				    }
			    }

			    /* write row contents to file */
			    image_data = outdata;
			    png_write_row(png_ptr, image_data);
		    }
		    break;
		case 270: /* Plot 90 degrees anti-clockwise */
		    for(row = 0; row < image_width; row++) {
			    for(column = 0; column < image_height; column++) {
				    i = column * 3;
				    switch(*(pixelbuf + (image_width * column) + (image_width - row - 1))) {
					    case '1':
						outdata[i] = fgred;
						outdata[i + 1] = fggrn;
						outdata[i + 2] = fgblu;
						break;
					    default:
						outdata[i] = bgred;
						outdata[i + 1] = bggrn;
						outdata[i + 2] = bgblu;
						break;
				    }
			    }
			    /* write row contents to file */
			    image_data = outdata;
			    png_write_row(png_ptr, image_data);
		    }
		    break;
	}
	/* End the file */
	png_write_end(png_ptr, NULL);
	/* make sure we have disengaged */
	if(png_ptr && info_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
	if(symbol->output_options & BARCODE_STDOUT) {
		fflush(wpng_info.outfile);
	}
	else {
		fclose(wpng_info.outfile);
	}
	return 0;
}

#endif /* NO_PNG */

int bmp_pixel_plot(ZintSymbol * symbol, int image_height, int image_width, char * pixelbuf, int rotate_angle) 
{
	int i, row, column, errno;
	int fgred, fggrn, fgblu, bgred, bggrn, bgblu;
	switch(rotate_angle) {
		case 0:
		case 180:
		    symbol->bitmap_width = image_width;
		    symbol->bitmap_height = image_height;
		    break;
		case 90:
		case 270:
		    symbol->bitmap_width = image_height;
		    symbol->bitmap_height = image_width;
		    break;
	}
	free(symbol->P_Bitmap);
	symbol->P_Bitmap = (char*)malloc(image_width * image_height * 3);
	/* sort out colour options */
	to_upper((uchar*)symbol->fgcolour);
	to_upper((uchar*)symbol->bgcolour);
	if(strlen(symbol->fgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	if(strlen(symbol->bgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (uchar*)symbol->fgcolour, strlen(symbol->fgcolour));
	if(errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (uchar*)symbol->bgcolour, strlen(symbol->fgcolour));
	if(errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}

	fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
	fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
	fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
	bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
	bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
	bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

	/* Pixel Plotting */
	i = 0;
	switch(rotate_angle) {
		case 0: /* Plot the right way up */
		    for(row = 0; row < image_height; row++) {
			    for(column = 0; column < image_width; column++) {
				    switch(*(pixelbuf + (image_width * row) + column)) {
					    case '1':
						symbol->P_Bitmap[i++] = fgred;
						symbol->P_Bitmap[i++] = fggrn;
						symbol->P_Bitmap[i++] = fgblu;
						break;
					    default:
						symbol->P_Bitmap[i++] = bgred;
						symbol->P_Bitmap[i++] = bggrn;
						symbol->P_Bitmap[i++] = bgblu;
						break;
				    }
			    }
		    }
		    break;
		case 90: /* Plot 90 degrees clockwise */
		    for(row = 0; row < image_width; row++) {
			    for(column = 0; column < image_height; column++) {
				    switch(*(pixelbuf + (image_width * (image_height - column - 1)) + row)) {
					    case '1':
						symbol->P_Bitmap[i++] = fgred;
						symbol->P_Bitmap[i++] = fggrn;
						symbol->P_Bitmap[i++] = fgblu;
						break;
					    default:
						symbol->P_Bitmap[i++] = bgred;
						symbol->P_Bitmap[i++] = bggrn;
						symbol->P_Bitmap[i++] = bgblu;
						break;
				    }
			    }
		    }
		    break;
		case 180: /* Plot upside down */
		    for(row = 0; row < image_height; row++) {
			    for(column = 0; column < image_width; column++) {
				    switch(*(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1))) {
					    case '1':
						symbol->P_Bitmap[i++] = fgred;
						symbol->P_Bitmap[i++] = fggrn;
						symbol->P_Bitmap[i++] = fgblu;
						break;
					    default:
						symbol->P_Bitmap[i++] = bgred;
						symbol->P_Bitmap[i++] = bggrn;
						symbol->P_Bitmap[i++] = bgblu;
						break;
				    }
			    }
		    }
		    break;
		case 270: /* Plot 90 degrees anti-clockwise */
		    for(row = 0; row < image_width; row++) {
			    for(column = 0; column < image_height; column++) {
				    switch(*(pixelbuf + (image_width * column) + (image_width - row - 1))) {
					    case '1':
						symbol->P_Bitmap[i++] = fgred;
						symbol->P_Bitmap[i++] = fggrn;
						symbol->P_Bitmap[i++] = fgblu;
						break;
					    default:
						symbol->P_Bitmap[i++] = bgred;
						symbol->P_Bitmap[i++] = bggrn;
						symbol->P_Bitmap[i++] = bgblu;
						break;
				    }
			    }
		    }
		    break;
	}

	return 0;
}

int png_to_file(ZintSymbol * symbol, int image_height, int image_width, char * pixelbuf, int rotate_angle, int image_type) 
{
	int error_number;
	float scaler = NZOR(symbol->scale, 0.5f);
	char * scaled_pixelbuf;
	int horiz, vert, i;
	int scale_width = (int)(image_width * scaler);
	int scale_height = (int)(image_height * scaler);
	/* Apply scale options by creating another pixel buffer */
	if(!(scaled_pixelbuf = (char*)malloc(scale_width * scale_height))) {
		printf("Insufficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	}
	else {
		for(i = 0; i < (scale_width * scale_height); i++) {
			*(scaled_pixelbuf + i) = '0';
		}
	}
	for(vert = 0; vert < scale_height; vert++) {
		for(horiz = 0; horiz < scale_width; horiz++) {
			*(scaled_pixelbuf +
			    (vert * scale_width) + horiz) = *(pixelbuf + ((int)(vert / scaler) * image_width) + (int)(horiz / scaler));
		}
	}

	if(image_type == PNG_DATA) {
#ifndef NO_PNG
		error_number = png_pixel_plot(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle);
#else
		return ZINT_ERROR_INVALID_OPTION;
#endif
	}
	else {
		error_number = bmp_pixel_plot(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle);
	}

	free(scaled_pixelbuf);

	return error_number;
}

void draw_bar(char * pixelbuf, int xpos, int xlen, int ypos, int ylen, int image_width, int image_height) {
	/* Draw a rectangle */
	int i, j, png_ypos;

	png_ypos = image_height - ypos - ylen;
	/* This fudge is needed because EPS measures height from the bottom up but
	   PNG measures y position from the top down */

	for(i = (xpos); i < (xpos + xlen); i++) {
		for(j = (png_ypos); j < (png_ypos + ylen); j++) {
			*(pixelbuf + (image_width * j) + i) = '1';
		}
	}
}

int bullseye_pixel(int row, int col) {
	int block_val, block_pos, return_val;

	block_val = bullseye_compressed[(row * 12) + (col / 8)];
	return_val = 0;
	block_pos = col % 8;

	if(block_val & (0x80 >> block_pos)) {
		return_val = 1;
	}

	return return_val;
}

void draw_bullseye(char * pixelbuf, int image_width, int xoffset, int yoffset) {
	/* Central bullseye in Maxicode symbols */
	int i, j;

	for(j = 103; j < 196; j++) {
		for(i = 0; i < 93; i++) {
			if(bullseye_pixel(j - 103, i)) {
				/* if(bullseye[(((j - 103) * 93) + i)] == 1) { */
				*(pixelbuf + (image_width * j) + (image_width * yoffset) + i + 99 + xoffset) = '1';
			}
		}
	}
}

void draw_hexagon(char * pixelbuf, int image_width, int xposn, int yposn) {
	/* Put a hexagon into the pixel buffer */
	int i, j;

	for(i = 0; i < 12; i++) {
		for(j = 0; j < 10; j++) {
			if(hexagon[(i * 10) + j] == 1) {
				*(pixelbuf + (image_width * i) + (image_width * yposn) + xposn + j) = '1';
			}
		}
	}
}

void draw_letter(char * pixelbuf, uchar letter, int xposn, int yposn, int smalltext, int image_width, int image_height) {
	/* Put a letter into a position */
	int skip, x, y, glyph_no;

	skip = 0;

	if(letter < 33) {
		skip = 1;
	}
	if((letter > 127) && (letter < 161)) {
		skip = 1;
	}

	if(skip == 0) {
		if(letter > 128) {
			glyph_no = letter - 66;
		}
		else {
			glyph_no = letter - 33;
		}

		if(smalltext) {
			for(y = 0; y < 9; y++) {
				for(x = 0; x < 5; x++) {
					if(small_font[(glyph_no * 8) + y] & (0x10 >> x)) {
						*(pixelbuf + (y * image_width) + (yposn * image_width) + xposn + x) = '1';
					}
				}
			}
		}
		else {
			for(y = 0; y < 14; y++) {
				for(x = 0; x < 7; x++) {
					if(ascii_font[(glyph_no * 14) + y] & (0x40 >> x)) {
						*(pixelbuf + (y * image_width) + (yposn * image_width) + xposn + x) = '1';
					}
				}
			}
		}
	}
}

/* Plot a string into the pixel buffer */
void draw_string(char * pixbuf, char input_string[], int xposn, int yposn, int smalltext, int image_width, int image_height) 
{
	const int string_length = strlen(input_string);
	int string_left_hand = xposn - ((7 * string_length) / 2);
	for(int i = 0; i < string_length; i++) {
		draw_letter(pixbuf, input_string[i], string_left_hand + (i * 7), yposn, smalltext, image_width, image_height);
	}
}

int maxi_png_plot(ZintSymbol * symbol, int rotate_angle, int data_type) 
{
	int i, row, column, xposn, yposn;
	char * pixelbuf;
	int error_number;
	int xoffset = symbol->border_width + symbol->whitespace_width;
	int yoffset = symbol->border_width;
	int image_width = 300 + (2 * xoffset * 2);
	int image_height = 300 + (2 * yoffset * 2);
	if(!(pixelbuf = (char*)malloc(image_width * image_height))) {
		printf("Insifficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	}
	else {
		for(i = 0; i < (image_width * image_height); i++)
			*(pixelbuf + i) = '0';
	}
	draw_bullseye(pixelbuf, image_width, (2 * xoffset), (2 * yoffset));
	for(row = 0; row < symbol->rows; row++) {
		yposn = row * 9;
		for(column = 0; column < symbol->width; column++) {
			xposn = column * 10;
			if(module_is_set(symbol, row, column)) {
				if(row & 1) {
					xposn += 5; // Odd (reduced) row
					draw_hexagon(pixelbuf, image_width, xposn + (2 * xoffset), yposn + (2 * yoffset));
				}
				else {
					draw_hexagon(pixelbuf, image_width, xposn + (2 * xoffset), yposn + (2 * yoffset)); // Even (full) row
				}
			}
		}
	}
	if((symbol->output_options & BARCODE_BOX) || (symbol->output_options & BARCODE_BIND)) {
		/* boundary bars */
		draw_bar(pixelbuf, 0, image_width, 0, symbol->border_width * 2, image_width, image_height);
		draw_bar(pixelbuf, 0, image_width, 300 + (symbol->border_width * 2), symbol->border_width * 2, image_width, image_height);
	}
	if((symbol->output_options & BARCODE_BOX) != 0) {
		/* side bars */
		draw_bar(pixelbuf, 0, symbol->border_width * 2, 0, image_height, image_width, image_height);
		draw_bar(pixelbuf,
		    300 + ((symbol->border_width + symbol->whitespace_width + symbol->whitespace_width) * 2),
		    symbol->border_width * 2, 0, image_height, image_width, image_height);
	}
	error_number = png_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
	free(pixelbuf);
	return error_number;
}

/* Convert UTF-8 to Latin1 Codepage for the interpretation line */
void to_latin1(uchar source[], uchar preprocessed[]) 
{
	int input_length = sstrlen(source);
	int j = 0;
	int i = 0;
	while(i < input_length) {
		switch(source[i]) {
			case 0xC2:
			    /* UTF-8 C2xxh */
			    /* Character range: C280h (latin: 80h) to C2BFh (latin: BFh) */
			    i++;
			    preprocessed[j] = source[i];
			    j++;
			    break;
			case 0xC3:
			    /* UTF-8 C3xx */
			    /* Character range: C380h (latin: C0h) to C3BFh (latin: FFh) */
			    i++;
			    preprocessed[j] = source[i] + 64;
			    j++;
			    break;
			default:
			    /* Process ASCII (< 80h), all other unicode points are ignored */
			    if(source[i] < 128) {
				    preprocessed[j] = source[i];
				    j++;
			    }
			    break;
		}
		i++;
	}
	preprocessed[j] = '\0';
	return;
}

int png_plot(ZintSymbol * symbol, int rotate_angle, int data_type) 
{
	int textdone, main_width, comp_offset, large_bar_count;
	char textpart[10], addon[6];
	float addon_text_posn, preset_height, large_bar_height;
	int i, r, textoffset, yoffset, xoffset, latch, image_width, image_height;
	char * pixelbuf;
	int addon_latch = 0, smalltext = 0;
	int this_row, block_width, plot_height, plot_yposn, textpos;
	float row_height, row_posn;
	int error_number;
	int default_text_posn;
	int next_yposn;
#ifndef _MSC_VER
	uchar local_text[sstrlen(symbol->text) + 1];
#else
	uchar * local_text = (uchar*)_alloca(sstrlen(symbol->text) + 1);
#endif
	if(symbol->show_hrt != 0) {
		to_latin1(symbol->text, local_text);
	}
	else {
		local_text[0] = '\0';
	}
	textdone = 0;
	main_width = symbol->width;
	strcpy(addon, "");
	comp_offset = 0;
	addon_text_posn = 0.0;
	row_height = 0;
	if(symbol->output_options & SMALL_TEXT)
		smalltext = 1;
	SETIFZ(symbol->height, 50);
	large_bar_count = 0;
	preset_height = 0.0;
	for(i = 0; i < symbol->rows; i++) {
		preset_height += symbol->row_height[i];
		if(symbol->row_height[i] == 0) {
			large_bar_count++;
		}
	}
	if(large_bar_count == 0) {
		symbol->height = (int)preset_height;
		large_bar_height = 10;
	}
	else {
		large_bar_height = (symbol->height - preset_height) / large_bar_count;
	}
	while(!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
		comp_offset++;
	}
	/* Certain symbols need whitespace otherwise characters get chopped off the sides */
	if((((symbol->Std == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->Std == BARCODE_EANX_CC))
	    || (symbol->Std == BARCODE_ISBNX)) {
		switch(sstrlen(local_text)) {
			case 13: /* EAN 13 */
			case 16:
			case 19:
				SETIFZ(symbol->whitespace_width, 10);
			    main_width = 96 + comp_offset;
			    break;
			default:
			    main_width = 68 + comp_offset;
		}
	}
	if(((symbol->Std == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->Std == BARCODE_UPCA_CC)) {
		if(symbol->whitespace_width == 0) {
			symbol->whitespace_width = 10;
			main_width = 96 + comp_offset;
		}
	}
	if(((symbol->Std == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->Std == BARCODE_UPCE_CC)) {
		if(symbol->whitespace_width == 0) {
			symbol->whitespace_width = 10;
			main_width = 51 + comp_offset;
		}
	}
	latch = 0;
	r = 0;
	/* Isolate add-on text */
	if(is_extendable(symbol->Std)) {
		const int _len = (int)sstrlen(local_text);
		for(i = 0; i < _len; i++) {
			if(latch == 1) {
				addon[r] = local_text[i];
				r++;
			}
			if(symbol->text[i] == '+') {
				latch = 1;
			}
		}
	}
	addon[r] = '\0';
	textoffset = (sstrlen(local_text) != 0) ? 9 : 0;
	xoffset = symbol->border_width + symbol->whitespace_width;
	yoffset = symbol->border_width;
	image_width = 2 * (symbol->width + xoffset + xoffset);
	image_height = 2 * (symbol->height + textoffset + yoffset + yoffset);
	if(!(pixelbuf = (char*)malloc(image_width * image_height))) {
		printf("Insufficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	}
	else {
		for(i = 0; i < (image_width * image_height); i++) {
			*(pixelbuf + i) = '0';
		}
	}
	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		default_text_posn = image_height - 17;
	}
	else {
		default_text_posn = image_height - 17 - symbol->border_width - symbol->border_width;
	}
	row_posn = (float)(textoffset + yoffset);
	next_yposn = textoffset + yoffset;
	row_height = 0;
	/* Plot the body of the symbol to the pixel buffer */
	for(r = 0; r < symbol->rows; r++) {
		this_row = symbol->rows - r - 1; /* invert r otherwise plots upside down */
		row_posn += row_height;
		plot_yposn = next_yposn;
		row_height = (symbol->row_height[this_row] == 0) ? large_bar_height : (float)symbol->row_height[this_row];
		next_yposn = (int)(row_posn + row_height);
		plot_height = next_yposn - plot_yposn;
		i = 0;
		if(module_is_set(symbol, this_row, 0)) {
			latch = 1;
		}
		else {
			latch = 0;
		}
		do {
			block_width = 0;
			do {
				block_width++;
			} while(module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));
			if((addon_latch == 0) && (r == 0) && (i > main_width)) {
				plot_height = (int)(row_height - 5.0f);
				plot_yposn = (int)(row_posn - 5.0f);
				addon_text_posn = (float)(row_posn + row_height - 8.0f);
				addon_latch = 1;
			}
			if(latch == 1) {
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset) * 2, block_width * 2, plot_yposn * 2, plot_height * 2,
				    image_width, image_height);
				latch = 0;
			}
			else {
				/* a space */
				latch = 1;
			}
			i += block_width;
		} while(i < symbol->width);
	}
	xoffset += comp_offset;
	if((((symbol->Std == BARCODE_EANX) &&
		(symbol->rows == 1)) || (symbol->Std == BARCODE_EANX_CC)) || (symbol->Std == BARCODE_ISBNX)) {
		/* guard bar extensions and text formatting for EAN8 and EAN13 */
		switch(sstrlen(local_text)) {
			case 8: /* EAN-8 */
			case 11:
			case 14:
			    draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (32 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (34 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (64 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (66 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    for(i = 0; i < 4; i++) {
				    textpart[i] = symbol->text[i];
			    }
			    textpart[4] = '\0';
			    textpos = 2 * (17 + xoffset);

			    draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
			    for(i = 0; i < 4; i++) {
				    textpart[i] = symbol->text[i + 4];
			    }
			    textpart[4] = '\0';
			    textpos = 2 * (50 + xoffset);
			    draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
			    textdone = 1;
			    switch(strlen(addon)) {
				    case 2:
						textpos = 2 * (xoffset + 86);
						draw_string(pixelbuf, addon, textpos,
							(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
						break;
				    case 5:
						textpos = 2 * (xoffset + 100);
						draw_string(pixelbuf, addon, textpos,
							(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
					break;
			    }
			    break;
			case 13: /* EAN 13 */
			case 16:
			case 19:
			    draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (92 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
			    draw_bar(pixelbuf, (94 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);

			    textpart[0] = symbol->text[0];
			    textpart[1] = '\0';
			    textpos = 2 * (-7 + xoffset);
			    draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
			    for(i = 0; i < 6; i++) {
				    textpart[i] = symbol->text[i + 1];
			    }
			    textpart[6] = '\0';
			    textpos = 2 * (24 + xoffset);
			    draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
			    for(i = 0; i < 6; i++) {
				    textpart[i] = symbol->text[i + 7];
			    }
			    textpart[6] = '\0';
			    textpos = 2 * (71 + xoffset);
			    draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
			    textdone = 1;
			    switch(strlen(addon)) {
				    case 2:
					textpos = 2 * (xoffset + 114);
					draw_string(pixelbuf, addon, textpos,
						(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
					break;
				    case 5:
					textpos = 2 * (xoffset + 128);
					draw_string(pixelbuf, addon, textpos,
						(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
					break;
			    }
			    break;
		}
	}
	if(((symbol->Std == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->Std == BARCODE_UPCA_CC)) {
		/* guard bar extensions and text formatting for UPCA */
		latch = 1;
		i = 0 + comp_offset;
		do {
			block_width = 0;
			do {
				block_width++;
			} while(module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
			if(latch == 1) {
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2,
				    block_width * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				latch = 0;
			}
			else {
				/* a space */
				latch = 1;
			}
			i += block_width;
		} while(i < 11 + comp_offset);
		draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		latch = 1;
		i = 85 + comp_offset;
		do {
			block_width = 0;
			do {
				block_width++;
			} while(module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
			if(latch == 1) {
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2,
				    block_width * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				latch = 0;
			}
			else {
				/* a space */
				latch = 1;
			}
			i += block_width;
		} while(i < 96 + comp_offset);
		textpart[0] = symbol->text[0];
		textpart[1] = '\0';
		textpos = 2 * (-5 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 5; i++) {
			textpart[i] = symbol->text[i+1];
		}
		textpart[5] = '\0';
		textpos = 2 * (27 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 5; i++) {
			textpart[i] = symbol->text[i+6];
		}
		textpart[6] = '\0';
		textpos = 2 * (68 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textpart[0] = symbol->text[11];
		textpart[1] = '\0';
		textpos = 2 * (100 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textdone = 1;
		switch(strlen(addon)) {
			case 2:
			    textpos = 2 * (xoffset + 116);
			    draw_string(pixelbuf, addon, textpos,
					(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
			    break;
			case 5:
			    textpos = 2 * (xoffset + 130);
			    draw_string(pixelbuf, addon, textpos,
					(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
			    break;
		}
	}

	if(((symbol->Std == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->Std == BARCODE_UPCE_CC)) {
		/* guard bar extensions and text formatting for UPCE */
		draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (50 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);

		textpart[0] = symbol->text[0];
		textpart[1] = '\0';
		textpos = 2 * (-5 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 6; i++) {
			textpart[i] = symbol->text[i + 1];
		}
		textpart[6] = '\0';
		textpos = 2 * (24 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textpart[0] = symbol->text[7];
		textpart[1] = '\0';
		textpos = 2 * (55 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textdone = 1;
		switch(strlen(addon)) {
			case 2:
			    textpos = 2 * (xoffset + 70);
			    draw_string(pixelbuf, addon, textpos, 
					(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width, image_height);
			    break;
			case 5:
			    textpos = 2 * (xoffset + 84);
			    draw_string(pixelbuf, addon, textpos, 
					(int)(image_height - (addon_text_posn * 2.0f) - 13.0f), smalltext, image_width,	image_height);
			    break;
		}
	}

	xoffset -= comp_offset;

	/* Put boundary bars or box around symbol */
	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		/* boundary bars */
		draw_bar(pixelbuf, 0, (symbol->width + xoffset + xoffset) * 2, textoffset * 2,
		    symbol->border_width * 2, image_width, image_height);
		draw_bar(pixelbuf, 0, (symbol->width + xoffset + xoffset) * 2,
		    (textoffset + symbol->height + symbol->border_width) * 2,
		    symbol->border_width * 2, image_width, image_height);
		if((symbol->output_options & BARCODE_BIND) != 0) {
			if((symbol->rows > 1) && (is_stackable(symbol->Std) == 1)) {
				/* row binding */
				for(r = 1; r < symbol->rows; r++) {
					draw_bar(pixelbuf, xoffset * 2, symbol->width * 2,
					    (int)(((r * row_height) + textoffset + yoffset - 1) * 2), 2 * 2, image_width, image_height);
				}
			}
		}
	}

	if((symbol->output_options & BARCODE_BOX) != 0) {
		/* side bars */
		draw_bar(pixelbuf, 0, symbol->border_width * 2,
		    textoffset * 2, (symbol->height + (2 * symbol->border_width)) * 2,
		    image_width, image_height);
		draw_bar(pixelbuf, (symbol->width + xoffset + xoffset - symbol->border_width) * 2, symbol->border_width * 2, textoffset * 2,
		    (symbol->height + (2 * symbol->border_width)) * 2, image_width, image_height);
	}
	/* Put the human readable text at the bottom */
	if((textdone == 0) && (sstrlen(local_text) != 0)) {
		textpos = (image_width / 2);
		draw_string(pixelbuf, (char*)local_text, textpos, default_text_posn, smalltext, image_width, image_height);
	}
	error_number = png_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
	free(pixelbuf);
	return error_number;
}

#ifndef NO_PNG

int png_handle(ZintSymbol * symbol, int rotate_angle) 
{
	int error;
	if(symbol->Std == BARCODE_MAXICODE) {
		error = maxi_png_plot(symbol, rotate_angle, PNG_DATA);
	}
	else {
		error = png_plot(symbol, rotate_angle, PNG_DATA);
	}
	return error;
}

#endif /* NO_PNG */

int bmp_handle(ZintSymbol * symbol, int rotate_angle) 
{
	int error;
	if(symbol->Std == BARCODE_MAXICODE) {
		error = maxi_png_plot(symbol, rotate_angle, BMP_DATA);
	}
	else {
		error = png_plot(symbol, rotate_angle, BMP_DATA);
	}
	return error;
}

