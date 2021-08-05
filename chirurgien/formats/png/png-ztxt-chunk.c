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

#include "png-format.h"


gboolean
png_ztxt_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    const gchar *ztxt_chunk;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    g_autofree gchar *inflate_text = NULL;
    guint deflate_size, inflate_size;
    const gchar *compressed_text = NULL;

    guint32 keyword_length = 0;
    gsize utf8_length;

    gint compression_method = -1;
    gchar *compression_method_value;

    gboolean null_found = FALSE;
    gboolean compression_method_found, flags_found,
             decompression_success, checksum_found;

    if (!chunk_length)
        return TRUE;

    chunk_counts[zTXt]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    if (!FILE_HAS_DATA_N (file, chunk_length))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                                "Chunk length exceeds available data", NULL);
        return FALSE;
    }

    format_utils_init_tab (&tab, NULL);

    ztxt_chunk = (const gchar *) GET_CONTENT_POINTER (file);

    /* The null character separes the keyword and the compression method + compressed text string */
    /* The keyword must the 1-79 bytes long */
    for (guint32 i = 0; i < chunk_length; i++)
    {
        if (ztxt_chunk[i] == '\0')
        {
            keyword_length = i;

            null_found = TRUE;

            if (i + 1 < chunk_length)
                compression_method = ztxt_chunk[i + 1];

            if (i + 2 < chunk_length)
                compressed_text = ztxt_chunk + i + 2;

            break;
        }
    }

    /* If 0, no null separator was found: there is no compressed text string */
    if (!keyword_length)
        keyword_length = chunk_length;

    keyword = g_convert (ztxt_chunk, keyword_length, "UTF-8", "ISO-8859-1",
                         NULL, &utf8_length, NULL);
    format_utils_add_text_tab (&tab, "Keyword", keyword, utf8_length);

    if (!keyword_length)
        format_utils_add_line_no_section_tab (&tab, "NOTE: No keyword defined, keywords should be at least 1 byte long");
    else if (keyword_length >= 80)
        format_utils_add_line_no_section_tab (&tab, "NOTE: The keyword exceeds its 79 bytes long limit");

    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, keyword_length,
                            "Keyword", NULL);
    chunk_length -= keyword_length;

    if (null_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                "Null separator", NULL);
        chunk_length--;
    }

    if (compression_method != -1)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                "Compression method", NULL);
        chunk_length--;

        format_utils_start_section_tab (&tab, "Compression");

        if (!compression_method)
            compression_method_value = "zlib-format DEFLATE";
        else
            compression_method_value = "<span foreground=\"red\">INVALID</span>";

        format_utils_add_line_tab (&tab, "Compression method", compression_method_value,
                                   "Text string compression method\n"
                                   "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE");

        if (!compression_method && compressed_text)
        {
            format_utils_start_section_tab (&tab, "ZLIB compression");

            png_zlib_deflate (file,
                              &tab,
                              compressed_text,
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
                format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                        "ZLIB compression method and flags (CMF)\n"
                                        "Lower four bits: compression method (CM)\n"
                                        "Upper four bits: compression info (CINFO)", NULL);
                chunk_length--;

                if (flags_found)
                {
                    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                            "ZLIB flags (FLG)", NULL);
                    chunk_length--;

                    if (decompression_success)
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, deflate_size,
                                                "ZLIB compressed text string", NULL);
                        chunk_length -= deflate_size;

                        if (checksum_found)
                        {
                            format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 4,
                                                    "ZLIB Adler32 checksum", NULL);
                            chunk_length -= 4;
                        }

                        text = g_convert (inflate_text, inflate_size, "UTF-8", "ISO-8859-1",
                                          NULL, &utf8_length, NULL);
                        format_utils_add_text_tab (&tab, "Text string", text, utf8_length);
                    }
                }
            }
        }

        /* If there is data left, tag it as unrecognized */
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "Unrecognized data", NULL);
    }

    format_utils_add_line_no_section_tab (&tab, "NOTE: zTXt chunks are encoded using ISO-8859-1");

    format_utils_insert_tab (file, &tab, chunk_types[zTXt]);

    return TRUE;
}
