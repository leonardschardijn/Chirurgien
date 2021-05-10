/* png-format.h
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

#include <format-utils.h>

G_BEGIN_DECLS

typedef struct
{
    /* List of IDAT chunk index/length pairs */
    GSList   *chunks;
    /* Size of the ZLIB compressed image (size of all IDAT chunks) */
    gsize     compressed_image_size;

    /* Index of the current IDAT chunk */
    gsize     current_chunk_index;
    /* Length of the current IDAT chunk */
    gsize     current_chunk_length;
    /* How much of the current IDAT chunk has been consumed */
    gsize     chunk_used;

} IdatChunks;

#define IDAT_CHUNKS_INIT { NULL, 0, 0, 0 ,0 }

#define SIGNATURE_COLOR    0
#define CHUNK_TYPE_COLOR   1
#define CHUNK_DATA_COLOR_1 2
#define CHUNK_DATA_COLOR_2 3
#define CHUNK_LENGTH_COLOR 4
#define CHUNK_CRC_COLOR    5
#define ERROR_COLOR_1      6
#define ERROR_COLOR_2      7

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

extern const gchar * const chunk_types[CHUNK_TYPES];

gboolean    process_png_field        (FormatsFile *,
                                      DescriptionTab *,
                                      const gchar *,
                                      const gchar *,
                                      const gchar *,
                                      gint,
                                      gint,
                                      gint,
                                      guint8 *,
                                      const gchar **,
                                      const gchar *,
                                      gpointer);

/* png-ihdr-chunk.c */
gboolean     png_ihdr_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint8 *,
                                      guint8 *);

/* png-plte-chunk.c */
gboolean     png_plte_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint *);

/* png-idat-chunk.c */
gboolean     collect_idat_chunk      (FormatsFile *,
                                      guint32,
                                      gint *,
                                      IdatChunks *);
void         png_idat_chunk          (FormatsFile *,
                                      IdatChunks *,
                                      guint8);

/* png-trns-chunk.c */
gboolean     png_trns_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint8,
                                      guint);

/* png-chrm-chunk.c */
gboolean     png_chrm_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-gama-chunk.c */
gboolean     png_gama_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-iccp-chunk.c */
gboolean     png_iccp_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-sbit-chunk.c */
gboolean     png_sbit_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint8);

/* png-srgb-chunk.c */
gboolean     png_srgb_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-text-chunk.c */
gboolean     png_text_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-ztxt-chunk.c */
gboolean     png_ztxt_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-itxt-chunk.c */
gboolean     png_itxt_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-bkgd-chunk.c */
gboolean     png_bkgd_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint8);

/* png-hist-chunk.c */
gboolean     png_hist_chunk          (FormatsFile *,
                                      guint32,
                                      gint *,
                                      guint);

/* png-phys-chunk.c */
gboolean     png_phys_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-splt-chunk.c */
gboolean     png_splt_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-time-chunk.c */
gboolean     png_time_chunk          (FormatsFile *,
                                      guint32,
                                      gint *);

/* png-zlib-defalte.c */
void         png_zlib_deflate        (FormatsFile *,
                                      DescriptionTab *,
                                      const gchar *,
                                      guint32,
                                      guint *,
                                      gchar **,
                                      guint *,
                                      gboolean *,
                                      gboolean *,
                                      gboolean *,
                                      gboolean *);

G_END_DECLS
