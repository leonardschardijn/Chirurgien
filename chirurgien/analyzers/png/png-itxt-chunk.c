/* png-itxt-chunk.c
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
analyze_itxt_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message1;
    gchar *description_message2 = NULL;

    g_autofree gchar *itxt_chunk = NULL;

    gchar *keyword = NULL;
    gchar *language_tag = NULL;
    gchar *translated_keyword = NULL;
    gchar *text = NULL;

    gsize i;
    gsize keyword_length = 0, language_tag_length = 0,
          translated_keyword_length = 0, text_length = 0;

    gint compression_flag = -1, compression_method = -1;

    g_autofree gchar *inflate_text = NULL;
    gsize deflate_size, inflate_size, expected_deflate_size;

    if (!chunk_length)
        return TRUE;

    chunk_counts[iTXt]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    itxt_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (itxt_chunk, file, chunk_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                                  _("Chunk length exceeds available data"));
        return FALSE;
    }

    /* iTXt chunks have the following structure:
     *   keyword
     *   - null separator -
     *   compression flag
     *   compression method
     *   language tag
     *    - null separator -
     *   translated keyword
     *    - null separator -
     *   text string (possibly compressed)
     * */
    for (i = 0; i < chunk_length; i++)
    {
        if (itxt_chunk[i] == '\0')
        {
            if (keyword == NULL)
            {
                keyword = itxt_chunk;
                keyword_length = i;

                if (i + 2 < chunk_length)
                {
                    compression_flag = itxt_chunk[i + 1];
                    compression_method = itxt_chunk[i + 2];
                    i += 2;
                }
            }
            else if (language_tag == NULL)
            {
                language_tag = itxt_chunk + keyword_length + 3;
                language_tag_length = i - keyword_length - 3;
            }
            else if (translated_keyword == NULL)
            {
                translated_keyword = language_tag + language_tag_length + 1;
                translated_keyword_length = i - keyword_length - 4 - language_tag_length;

                text = translated_keyword + translated_keyword_length + 1;
                text_length = chunk_length - keyword_length - 5 - language_tag_length - translated_keyword_length;

                break;
            }
        }
    }

    if (keyword == NULL)
    {
        keyword = itxt_chunk;
        keyword_length = chunk_length;
    }

    if (keyword_length == 0 || keyword_length >= 80)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid keyword length"));
        return TRUE;
    }

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, keyword_length, _("Keyword"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Compression flag"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Compression method"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, language_tag_length, _("Language tag"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, translated_keyword_length, _("Translated keyword"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));

    analyzer_utils_add_text_tab (&tab, _("Keyword"), keyword, keyword_length);

    if (compression_flag == 0)
    {
        description_message1 = _("Uncompressed text");

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, text_length, _("Uncompressed text string"));
        if (text)
            analyzer_utils_add_text_tab (&tab, _("Text string"), text, text_length);
    }
    else if (compression_flag == 1)
    {
        description_message1 = _("Compressed text");

        if (compression_method == 0) // zlib-format DEFLATE
        {
            description_message2 = _("zlib-format DEFLATE");

            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                _("ZLIB compression method and flags (CMF)\n"
                                "Lower four bits: compression method (CM)\n"
                                "Upper four bits: compression info (CINFO)"));

            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("ZLIB flags (FLG)"));

            /* deflate size = text_length - ZLIB CMF (1) - ZLIB FLG (1) - ZLIB Adler32 chechsum (4) */
            expected_deflate_size = deflate_size = text_length - 6;

            /* Skip ZLIB CMF and FLG */
            text += 2;

            if (!puff (NULL, NULL, NULL, NULL, &inflate_size, (guchar *) text, &deflate_size))
            {
                inflate_text = g_malloc (inflate_size);

                puff (NULL, NULL, NULL, (guchar *) inflate_text, &inflate_size, (guchar *) text, &deflate_size);

                analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, deflate_size, _("ZLIB compressed text string"));
                analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 4, _("ZLIB Adler32 checksum"));

                expected_deflate_size -= deflate_size;
                if (expected_deflate_size)
                    analyzer_utils_tag_error (file, ERROR_COLOR_1, expected_deflate_size, _("Unrecognized data"));

                analyzer_utils_add_text_tab (&tab, _("Text string"), inflate_text, inflate_size);
            }
            else
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, text_length - 2,
                                    _("ZLIB Compressed data (inflate failed)"));
            }
        }
        else
        {
            description_message2 = _("<span foreground=\"red\">INVALID</span>");
            analyzer_utils_tag_error (file, ERROR_COLOR_1, text_length, _("Unrecognized data"));
        }
    }
    else
    {
        description_message1 = _("<span foreground=\"red\">INVALID</span>");

        analyzer_utils_tag_error (file, ERROR_COLOR_1, text_length, _("Unrecognized data"));
    }

    analyzer_utils_add_text_tab (&tab, _("Language tag"), language_tag, language_tag_length);
    analyzer_utils_add_text_tab (&tab, _("Translated keyword"), translated_keyword, translated_keyword_length);

    analyzer_utils_describe_tooltip_tab (&tab, _("Compression flag"), description_message1,
                                         _("Compression flag\n"
                                         "<tt>00<sub>16</sub></tt>\tUncompressed text\n"
                                         "<tt>01<sub>16</sub></tt>\tCompressed text"));
    if (description_message2)
        analyzer_utils_describe_tooltip_tab (&tab, _("Compression method"), description_message2,
                                             _("Text string compression method\n"
                                             "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[iTXt]);

    return TRUE;
}
