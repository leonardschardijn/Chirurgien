/* png-ztxt-chunk.c
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


gboolean
analyze_ztxt_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *ztxt_chunk;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    g_autofree gchar *inflate_text = NULL;
    gchar *compressed_text = NULL;

    gsize i;
    gsize keyword_length = 0, keyword_length_utf8, text_length_utf8;

    gsize deflate_size, inflate_size, expected_deflate_size;

    gint compression_method = -1;

    if (!chunk_length)
        return TRUE;

    chunk_counts[zTXt]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (!FILE_HAS_DATA_N (file, chunk_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                                  _("Chunk length exceeds available data"));
        return FALSE;
    }

    analyzer_utils_init_tab (&tab);

    ztxt_chunk = (gchar *) file->file_contents + GET_POINTER (file);
    ADVANCE_POINTER (file, chunk_length);

    /* The null character separes the keyword and the text string */
    /* The keyword must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (ztxt_chunk[i] == '\0')
        {
            keyword = ztxt_chunk;
            keyword_length = i;

            /* null separator (1) + compression method (1) + ZLIB CMF (1) +
             * ZLIB FLG (1) + at least a single byte of compressed data (1) */
            if (i + 5 < chunk_length)
            {
                compression_method = ztxt_chunk[i + 1];

                compressed_text = ztxt_chunk + i + 4;
            }
            break;
        }
    }

    if (keyword == NULL)
    {
        keyword = ztxt_chunk;
        keyword_length = chunk_length;
    }

    if (keyword_length == 0 || keyword_length >= 80)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid keyword length"));
        keyword = NULL;
        text = NULL;

        return TRUE;
    }

    keyword = g_convert (keyword, keyword_length, "UTF-8", "ISO-8859-1", 
                         NULL, &keyword_length_utf8, NULL);

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, keyword_length, _("Keyword"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));

    analyzer_utils_add_text_tab (&tab, _("Keyword"), keyword, keyword_length_utf8);

    if (compression_method == 0) // zlib-format DEFLATE
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Compression method"));

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                            _("ZLIB compression method and flags (CMF)\n"
                              "Lower four bits: compression method (CM)\n"
                              "Upper four bits: compression info (CINFO)"));

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("ZLIB flags (FLG)"));

        /* deflate size = chunk_length - keyword - null separator (1) -
         * compression method (1) - ZLIB CMF (1) - ZLIB FLG (1) - ZLIB Adler32 chechsum (4) */
        expected_deflate_size = deflate_size = chunk_length - keyword_length - 8;

        if (!puff (NULL, NULL, NULL, NULL, &inflate_size, (guchar *) compressed_text, &deflate_size))
        {
            inflate_text = g_malloc (inflate_size);
            puff (NULL, NULL, NULL, (guchar *) inflate_text, &inflate_size, (guchar *) compressed_text, &deflate_size);

            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, deflate_size,
                                _("ZLIB compressed text string"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4,
                                _("ZLIB Adler32 checksum"));

            expected_deflate_size -= deflate_size;
            if (expected_deflate_size)
                analyzer_utils_tag_error (file, ERROR_COLOR_1, expected_deflate_size, _("Unrecognized data"));

            text = g_convert (inflate_text, inflate_size, "UTF-8", "ISO-8859-1",
                              NULL, &text_length_utf8, NULL);

            analyzer_utils_add_text_tab (&tab, _("Text string"), text, text_length_utf8);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length - keyword_length - 4,
                                       _("ZLIB Compressed data (inflate failed)"));
        }

        analyzer_utils_describe_tooltip_tab (&tab, _("Compression method"), _("zlib-format DEFLATE"),
                                             _("Text string compression method\n"
                                               "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_2, 1, _("Compression method"));
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length - keyword_length - 2,
                            _("Unrecognized data"));

        analyzer_utils_describe_tooltip_tab (&tab, _("Compression method"), _("<span foreground=\"red\">INVALID</span>"),
                                             _("Text string compression method\n"
                                               "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: zTXt chunks are encoded using ISO-8859-1"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[zTXt]);

    return TRUE;
}
