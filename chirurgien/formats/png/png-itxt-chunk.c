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

#include "png-format.h"


gboolean
png_itxt_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    const gchar *itxt_chunk;

    const gchar *language_tag = NULL;
    const gchar *translated_keyword = NULL;
    const gchar *text = NULL;

    gchar *compression_flag_value;
    gchar *compression_method_value = NULL;

    guint32 keyword_length = 0, language_tag_length = 0,
            translated_keyword_length = 0;

    gint nulls_found = 0;
    gint compression_flag = -1, compression_method = -1;

    g_autofree gchar *inflate_text = NULL;
    guint deflate_size, inflate_size;
    gboolean compression_method_found, flags_found,
             decompression_success, checksum_found;

    if (!chunk_length)
        return TRUE;

    chunk_counts[iTXt]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (!FILE_HAS_DATA_N (file, chunk_length))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Chunk length exceeds available data"), NULL);
        return FALSE;
    }

    format_utils_init_tab (&tab, NULL);

    itxt_chunk = (const gchar *) GET_CONTENT_POINTER (file);

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
    for (guint32 i = 0; i < chunk_length; i++)
    {
        if (itxt_chunk[i] == '\0')
        {
            nulls_found++;

            if (nulls_found == 1)
            {
                keyword_length = i;

                if (i + 2 < chunk_length)
                {
                    compression_flag = itxt_chunk[++i];
                    compression_method = itxt_chunk[++i];
                }

                language_tag = itxt_chunk + keyword_length + 3;
            }
            else if (nulls_found == 2)
            {
                language_tag_length = i - keyword_length - 3;

                translated_keyword = language_tag + language_tag_length + 1;
            }
            else if (nulls_found == 3)
            {
                translated_keyword_length = i - keyword_length - language_tag_length - 4;

                text = translated_keyword + translated_keyword_length + 1;

                break;
            }
        }
    }

    /* If 0, no null separator was found: only the keyword is defined */
    if (!keyword_length)
        keyword_length = chunk_length;

    /* Keyword */
    format_utils_add_text_tab (&tab, _("Keyword"), itxt_chunk, keyword_length);

    if (!keyword_length)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: No keyword defined, keywords should be at least 1 byte long"));
    else if (keyword_length >= 80)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: The keyword exceeds its 79 bytes long limit"));

    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, keyword_length,
                          _("Keyword"), NULL);
    chunk_length -= keyword_length;

    /* Null separator */
    if (nulls_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);
        chunk_length--;
        nulls_found--;
    }
    else
    {
        goto END;
    }

    /* Compression flag */
    if (compression_flag != -1)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                              _("Compression flag"), NULL);
        chunk_length--;
    }
    else
    {
        goto END;
    }

    /* Compression method */
    if (compression_method != -1)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Compression method"), NULL);
        chunk_length--;
    }
    else
    {
        goto END;
    }

    /* Language tag */
    if (language_tag_length)
    {
        format_utils_add_text_tab (&tab, _("Language tag"), language_tag, language_tag_length);
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, language_tag_length,
                              _("Language tag"), NULL);
        chunk_length -= language_tag_length;
    }

    /* Null separator */
    if (nulls_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);
        chunk_length--;
        nulls_found--;
    }
    else
    {
        format_utils_add_text_tab (&tab, _("Language tag"), language_tag, chunk_length);
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, chunk_length,
                              _("Language tag"), NULL);

        goto END;
    }

    /* Translated keyword */
    if (translated_keyword_length)
    {
        format_utils_add_text_tab (&tab, _("Translated keyword"), translated_keyword, translated_keyword_length);
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, translated_keyword_length,
                              _("Translated keyword"), NULL);
        chunk_length -= translated_keyword_length;
    }

    /* Null separator */
    if (nulls_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);
        chunk_length--;
        nulls_found--;
    }
    else
    {
        format_utils_add_text_tab (&tab, _("Translated keyword"), translated_keyword, chunk_length);
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, chunk_length,
                              _("Translated keyword"), NULL);
        goto END;
    }

    format_utils_start_section_tab (&tab, _("Compression"));

    if (!compression_flag)
    {
       compression_flag_value = _("Uncompressed text");
    }
    else if (compression_flag == 1)
    {
        compression_flag_value = _("Compressed text");

        if (!compression_method)
            compression_method_value = _("zlib-format DEFLATE");
        else
            compression_method_value = _("<span foreground=\"red\">INVALID</span>");
    }
    else
    {
        compression_flag_value = _("<span foreground=\"red\">INVALID</span>");
    }

    format_utils_add_line_tab (&tab, _("Compression flag"), compression_flag_value,
                             _("Compression flag\n"
                               "<tt>00<sub>16</sub></tt>\tUncompressed text\n"
                               "<tt>01<sub>16</sub></tt>\tCompressed text"));
    if (compression_method_value)
        format_utils_add_line_tab (&tab, _("Compression method"), compression_method_value,
                                 _("Text string compression method\n"
                                   "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));

    if (chunk_length)
    {
        if (!compression_flag)
        {
            format_utils_add_text_tab (&tab, _("Text string"), text, chunk_length);
            format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, chunk_length,
                                  _("Uncompressed text string"), NULL);
            chunk_length = 0;
        }
        else if (compression_flag == 1 && !compression_method)
        {
            format_utils_start_section_tab (&tab, _("ZLIB compression"));

            png_zlib_deflate (file,
                              &tab,
                              text,
                              chunk_length,
                              &deflate_size,
                              &inflate_text,
                              &inflate_size,
                              &compression_method_found,
                              &flags_found,
                              &decompression_success,
                              &checksum_found);

            if (compression_method_found)
            {
                format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                      _("ZLIB compression method and flags (CMF)\n"
                                        "Lower four bits: compression method (CM)\n"
                                        "Upper four bits: compression info (CINFO)"), NULL);
                chunk_length--;

                if (flags_found)
                {
                    format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                          _("ZLIB flags (FLG)"), NULL);
                    chunk_length--;

                    if (decompression_success)
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, deflate_size,
                                              _("ZLIB compressed text string"), NULL);
                        chunk_length -= deflate_size;

                        if (checksum_found)
                        {
                            format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 4,
                                                  _("ZLIB Adler32 checksum"), NULL);
                            chunk_length -= 4;
                        }

                        format_utils_add_text_tab (&tab, _("Text string"), inflate_text, inflate_size);
                    }
                }
            }
        }

        /* If there is data left, tag it as unrecognized */
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("Unrecognized data"), NULL);
    }

    END:
    format_utils_insert_tab (file, &tab, chunk_types[iTXt]);

    return TRUE;
}
