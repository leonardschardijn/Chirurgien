/* png-idat-chunk.c
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

#include <config.h>

#include <glib/gi18n.h>

#include "puff.h"
#include "png-analyzer.h"


typedef struct
{
    /* List of IDAT chunk (index, length) pairs */
    GSList *idat_chunks;
    /* Index of the current IDAT chunk */
    gsize current_chunk_index;
    /* Length of the current IDAT chunk */
    gsize current_chunk_length;
    /* How much of the current IDAT chunk has been consumed */
    gsize chunk_used;
} IdatChunkTagger;

static void
idat_chunk_create_tag (IdatChunkTagger *tagger,
                       AnalyzerFile *file,
                       GdkRGBA *color,
                       gboolean set_background,
                       gsize count,
                       gchar *tooltip)
{
    gsize chunk_available;

    chunk_available = tagger->current_chunk_length - tagger->chunk_used;
    if (count <= chunk_available)
    {
        analyzer_utils_create_tag_index (file, color, set_background, count, 
                                         tagger->current_chunk_index + tagger->chunk_used, tooltip);
        tagger->chunk_used += count;
    }
    else
    {
        analyzer_utils_create_tag_index (file, color, set_background, chunk_available,
                                         tagger->current_chunk_index + tagger->chunk_used, tooltip);
        if (tagger->idat_chunks)
        {
            /* Assign pair and advance list */
            tagger->current_chunk_index = GPOINTER_TO_SIZE (tagger->idat_chunks->data);
            tagger->current_chunk_length = GPOINTER_TO_SIZE (tagger->idat_chunks->next->data);
            tagger->idat_chunks = tagger->idat_chunks->next->next;
            tagger->chunk_used = 0;
        }
        idat_chunk_create_tag (tagger, file, color, set_background, count - chunk_available, tooltip);
    }
}

gboolean
collect_idat_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    ZlibData *zlib_data)
{
    if (!chunk_length)
        return TRUE;

    chunk_counts[IDAT]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (FILE_HAS_DATA_N (file, chunk_length))
    {
        zlib_data->idat_chunks = g_slist_append (zlib_data->idat_chunks, GSIZE_TO_POINTER (file->file_contents_index));
        zlib_data->idat_chunks = g_slist_append (zlib_data->idat_chunks, GSIZE_TO_POINTER (chunk_length));
        zlib_data->compressed_image_size += chunk_length;

        SKIP_DATA (file, chunk_length);
        return TRUE;
    }
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                              _("Chunk length exceeds available data"));
    return FALSE;
}

void
analyze_idat_chunk (AnalyzerFile *file,
                    ZlibData *zlib_data)
{
    g_autofree guchar *compressed_data = NULL;

    guchar *compressed_data_pointer;

    GSList *index;
    gsize chunk_index, chunk_length;
    IdatChunkTagger tagger;

    guint8 compression_method, info_flags;
    guint slidingwindow_size, no_compression_blocks, fixed_blocks, dynamic_blocks;
    gsize deflate_size, inflate_size, expected_deflate_size;

    gchar *description_message;

    if (zlib_data->compressed_image_size)
    {
        compressed_data = g_malloc (zlib_data->compressed_image_size);

        compressed_data_pointer = compressed_data;
        for (index = zlib_data->idat_chunks; index != NULL; index = index->next)
        {
            chunk_index = GPOINTER_TO_SIZE (index->data);
            index = index->next;
            chunk_length = GPOINTER_TO_SIZE (index->data);

            memmove (compressed_data_pointer, file->file_contents + chunk_index, chunk_length);
            compressed_data_pointer += chunk_length;
        }

        /* Assign first pair and advance list */
        tagger.current_chunk_index = GPOINTER_TO_SIZE (zlib_data->idat_chunks->data);
        tagger.current_chunk_length = GPOINTER_TO_SIZE (zlib_data->idat_chunks->next->data);
        tagger.idat_chunks = zlib_data->idat_chunks->next->next;
        tagger.chunk_used = 0;

        if (zlib_data->compressed_image_size > 7) /* Minimum zlib format size? */
        {
            analyzer_utils_set_subtitle (file, _("<b>ZLIB compressed image</b>"), NULL);

            compressed_data_pointer = compressed_data;

            /*ZLIB Compression method and flags */
            compression_method = *compressed_data_pointer & 0x0F;
            info_flags = (*compressed_data_pointer & 0xF0) >> 4;
            if (compression_method == 8)
            {
                analyzer_utils_describe_tooltip (file, _("Compression method"), "DEFLATE",
                                                 _("Compression method (CM): Lower four bits of CMF\n"
                                                 "Compression method\n"
                                                 "<tt>8<sub>16</sub></tt>\tDEFLATE"));
                if (info_flags > 7)
                {
                    description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));
                }
                else
                {
                    slidingwindow_size = 1 << (info_flags + 8);
                    description_message = g_strdup_printf ("%u", slidingwindow_size);
                }
                analyzer_utils_describe_tooltip (file, _("LZ77 sliding window size"), description_message,
                                                 _("Compression info (CINFO): Upper four bits of CMF, only valid when CM = 8\n"
                                                 "Used to calculate the sliding window size as:\n"
                                                 "2<sup>CINFO + 8</sup> if CINFO &lt; 8"));
                g_free (description_message);
            }
            else
            {
                analyzer_utils_describe_tooltip (file, _("Compression method"),
                                                 _("<span foreground=\"red\">INVALID</span>"),
                                                 _("Compression method (CM): Lower four bits of CMF\n"
                                                 "Compression method\n"
                                                 "<tt>8<sub>16</sub></tt>\tDEFLATE"));
            }
            idat_chunk_create_tag (&tagger, file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                   _("ZLIB compression method and flags (CMF)\n"
                                   "Lower four bits: compression method (CM)\n"
                                   "Upper four bits: compression info (CINFO)"));

            compressed_data_pointer++;

            /* ZLIB Flags */
            info_flags = (*compressed_data_pointer & 0xC0) >> 6;
            switch (info_flags)
            {
                case 0:
                    description_message = _("Compressor used fastest algorithm");
                    break;
                case 1:
                    description_message = _("Compressor used fast algorithm");
                    break;
                case 2:
                    description_message = _("Compressor used default algorithm");
                    break;
                case 3:
                    description_message = _("Compressor used maximum compression, slowest algorithm");
                    break;
            }
            analyzer_utils_add_description (file, _("Compression level"), description_message,
                                    _("Compression level: upper two bits of FLG\n"
                                    "Compression levels\n"
                                    "<tt>0</tt>\tCompressor used fastest algorithm\n"
                                    "<tt>1</tt>\tCompressor used fast algorithm\n"
                                    "<tt>2</tt>\tCompressor used default algorithm\n"
                                    "<tt>3</tt>\tCompressor used maximum compression, slowest algorithm"),
                                    10, 10);
            idat_chunk_create_tag (&tagger, file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                   _("ZLIB flags (FLG)"));

            compressed_data_pointer++;

            /* ZLIB Compressed data */
            expected_deflate_size = deflate_size = zlib_data->compressed_image_size - 6;
            if (!puff (&no_compression_blocks, &fixed_blocks, &dynamic_blocks,
                       NULL, &inflate_size, compressed_data_pointer, &deflate_size))
            {
                idat_chunk_create_tag (&tagger, file, CHUNK_DATA_COLOR_1, TRUE, deflate_size,
                                       _("ZLIB compressed image"));
                idat_chunk_create_tag (&tagger, file, CHUNK_DATA_COLOR_2, TRUE, 4,
                                       _("ZLIB Adler32 checksum"));

                description_message = g_strdup_printf ("%lu", inflate_size);
                analyzer_utils_describe_tooltip (file, _("Inflate size"), description_message,
                                                 _("Size of the raw data"));
                g_free (description_message);

                description_message = g_strdup_printf ("%lu", deflate_size);
                analyzer_utils_describe_tooltip (file, _("Deflate size"), description_message,
                                                 _("Size of the compressed data"));
                g_free (description_message);

                description_message = g_strdup_printf ("%.2f", (gfloat) inflate_size / deflate_size);
                analyzer_utils_describe (file, _("Compression ratio"), description_message);
                g_free (description_message);

                analyzer_utils_set_subtitle (file, _("<b>DEFLATE block count</b>"), NULL);

                description_message = g_strdup_printf ("%u", no_compression_blocks);
                analyzer_utils_describe_tooltip (file, _("No compression blocks"), description_message,
                                                 _("Number of blocks without compression"));
                g_free (description_message);

                description_message = g_strdup_printf ("%u", fixed_blocks);
                analyzer_utils_describe_tooltip (file, _("Fixed Huffman code blocks"), description_message,
                                                 _("Number of blocks using the implicit fixed alphabets"));
                g_free (description_message);

                description_message = g_strdup_printf ("%u", dynamic_blocks);
                analyzer_utils_describe_tooltip (file, _("Dynamic Huffman code blocks"), description_message,
                                                 _("Number of blocks using dynamically created alphabets"));
                g_free (description_message);

                expected_deflate_size -= deflate_size;
                if (expected_deflate_size)
                    idat_chunk_create_tag (&tagger, file, ERROR_COLOR_1, FALSE, expected_deflate_size,
                                           _("Unrecognized data"));
            }
            else
            {
                idat_chunk_create_tag (&tagger, file, ERROR_COLOR_1, FALSE, zlib_data->compressed_image_size - 2,
                                       _("ZLIB Compressed data (inflate failed)"));
            }
            g_slist_free (zlib_data->idat_chunks);
        }
    }
}
