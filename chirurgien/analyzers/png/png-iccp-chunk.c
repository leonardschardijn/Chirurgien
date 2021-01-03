/* png-iccp-chunk.c
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
analyze_iccp_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    g_autofree guchar *iccp_chunk = NULL;

    g_autofree gchar *profile_name = NULL;
    guchar *compressed_profile;

    gsize i;
    gsize profile_name_length = 0, profile_name_length_utf8;

    gsize deflate_size, inflate_size, expected_deflate_size;

    gint compression_method = -1;

    if (!chunk_length)
        return TRUE;

    chunk_counts[iCCP]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    iccp_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (iccp_chunk, file, chunk_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                                  _("Chunk length exceeds available data"));
        return FALSE;
    }

    /* The null character separes the profile name and the compression method + compressed profile */
    /* The profile name must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (iccp_chunk[i] == '\0')
        {
            profile_name = (gchar *) iccp_chunk;
            profile_name_length = i;

            /* null separator (1) + compression method (1) + ZLIB CMF (1) +
             * ZLIB FLG (1) + at least a single byte of compressed data (1) */
            if (i + 5 <= chunk_length)
            {
                compression_method = iccp_chunk[i + 1];

                compressed_profile = iccp_chunk + i + 4;
            }
            break;
        }
    }

    if (profile_name_length == 0 || profile_name_length >= 80)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("Invalid ICC profile name length"));
        profile_name = NULL;

        return TRUE;
    }

    profile_name = g_convert (profile_name, profile_name_length, "UTF-8", "ISO-8859-1",
                              NULL, &profile_name_length_utf8, NULL);

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, profile_name_length, _("Profile name"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));

    analyzer_utils_add_text_tab (&tab, _("ICC profile name"), profile_name, profile_name_length);

    if (compression_method == 0) // zlib-format DEFLATE
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Compression method"));

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                            _("ZLIB compression method and flags (CMF)\n"
                            "Lower four bits: compression method (CM)\n"
                            "Upper four bits: compression info (CINFO)"));

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("ZLIB flags (FLG)"));

        analyzer_utils_describe_tooltip_tab (&tab, _("Compression method"), _("zlib-format DEFLATE"),
                                             _("ICC profile compression method\n"
                                             "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));

        /* deflate profile size = chunk_length - profile_name - null separator (1) -
         * compression method (1) - ZLIB CMF (1) - ZLIB FLG (1) - ZLIB Adler32 chechsum (4) */
        expected_deflate_size = deflate_size = chunk_length - profile_name_length - 8;

        if (!puff (NULL, NULL, NULL, NULL, &inflate_size, compressed_profile, &deflate_size))
        {
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, deflate_size,
                                _("ZLIB compressed ICC profile"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4,
                                _("ZLIB Adler32 checksum"));

            expected_deflate_size -= deflate_size;
            if (expected_deflate_size)
                analyzer_utils_tag_error (file, ERROR_COLOR_1, expected_deflate_size, _("Unrecognized data"));
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length - profile_name_length - 4,
                                      _("ZLIB Compressed data (inflate failed)"));
        }
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_2, 1, _("Compression method"));
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length - profile_name_length - 2,
                                  _("Unrecognized data"));

        analyzer_utils_describe_tooltip_tab (&tab, _("Compression method"),
                                             _("<span foreground=\"red\">INVALID</span>"),
                                             _("ICC profile compression method\n"
                                             "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: ICC profile names are encoded using ISO-8859-1"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[iCCP]);

    return TRUE;
}
