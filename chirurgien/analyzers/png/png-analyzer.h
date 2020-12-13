/* png-analyzer.h
 *
 * Copyright (C) 2020 - Daniel Léonard Schardijn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtk/gtk.h>

#include <chirurgien-analyzer-file.h>
#include <analyzer-utils.h>

G_BEGIN_DECLS

typedef struct
{
    /* List of IDAT chunk (index, length) pairs */
    GSList *idat_chunks;
    /* Size of the ZLIB compressed image (size of all IDAT chunks) */
    gsize compressed_image_size;
} ZlibData;


enum {
    SIGNATURE_COLOR,
    CHUNK_LENGTH_COLOR,
    CHUNK_TYPE_COLOR,
    CHUNK_DATA_COLOR_1,
    CHUNK_DATA_COLOR_2,
    CHUNK_CRC_COLOR,
    ERROR_COLOR_1,
    ERROR_COLOR_2
};

extern GdkRGBA png_colors[8];

enum {
    IHDR,
    PLTE,
    IDAT,
    IEND,
    tRNS,
    cHRM,
    gAMA,
    iCCP,
    sBIT,
    sRGB,
    tEXt,
    zTXt,
    iTXt,
    bKGD,
    hIST,
    pHYs,
    sPLT,
    tIME,
    CHUNK_TYPES
};


/* png-ihdr-chunk.c */
gboolean     analyze_ihdr_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      guint8 *);

/* png-plte-chunk.c */
gboolean     analyze_plte_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      guint *);

/* png-idat-chunk.c */
gboolean     collect_idat_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      ZlibData *);
void         analyze_idat_chunk      (AnalyzerFile *,
                                      ZlibData *);

/* png-trns-chunk.c */
gboolean     analyze_trns_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      guint8,
                                      guint);

/* png-chrm-chunk.c */
gboolean     analyze_chrm_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-gama-chunk.c */
gboolean     analyze_gama_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-iccp-chunk.c */
gboolean     analyze_iccp_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-sbit-chunk.c */
gboolean     analyze_sbit_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-srgb-chunk.c */
gboolean     analyze_srgb_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-text-chunk.c */
gboolean     analyze_text_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-ztxt-chunk.c */
gboolean     analyze_ztxt_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-itxt-chunk.c */
gboolean     analyze_itxt_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-bkgd-chunk.c */
gboolean     analyze_bkgd_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      guint8);

/* png-hist-chunk.c */
gboolean     analyze_hist_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *,
                                      guint);

/* png-phys-chunk.c */
gboolean     analyze_phys_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-splt-chunk.c */
gboolean     analyze_splt_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

/* png-time-chunk.c */
gboolean     analyze_time_chunk      (AnalyzerFile *,
                                      gsize,
                                      guint *);

G_END_DECLS
