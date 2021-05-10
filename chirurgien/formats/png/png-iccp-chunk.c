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

#include "png-format.h"


gboolean
png_iccp_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    const gchar *iccp_chunk;

    g_autofree gchar *profile_name = NULL;

    guint deflate_size;
    const gchar *compressed_profile = NULL;

    guint32 profile_name_length = 0;
    gsize utf8_length;

    gint compression_method = -1;
    const gchar *compression_method_value;

    gboolean null_found = FALSE;
    gboolean compression_method_found, flags_found,
             decompression_success, checksum_found;

    if (!chunk_length)
        return TRUE;

    chunk_counts[iCCP]++;

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

    iccp_chunk = (const gchar *) GET_CONTENT_POINTER (file);

    /* The null character separes the profile name and the compression method + compressed profile */
    /* The profile name must the 1-79 bytes long */
    for (guint32 i = 0; i < chunk_length; i++)
    {
        if (iccp_chunk[i] == '\0')
        {
            profile_name_length = i;

            null_found = TRUE;

            if (i + 1 < chunk_length)
                compression_method = iccp_chunk[i + 1];

            if (i + 2 < chunk_length)
                compressed_profile = iccp_chunk + i + 2;

            break;
        }
    }

    /* If 0, no null separator was found: there is no compressed profile */
    if (!profile_name_length)
        profile_name_length = chunk_length;

    profile_name = g_convert (iccp_chunk, profile_name_length, "UTF-8", "ISO-8859-1",
                              NULL, &utf8_length, NULL);
    format_utils_add_text_tab (&tab, _("ICC profile name"), profile_name, utf8_length);

    if (!profile_name_length)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: No profile name defined, profile name should be at least 1 byte long"));
    else if (profile_name_length >= 80)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: The profile name exceeds its 79 bytes long limit"));

    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, profile_name_length,
                          _("Profile name"), NULL);
    chunk_length -= profile_name_length;

    if (null_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);
        chunk_length--;
    }

    if (compression_method != -1)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                              _("Compression method"), NULL);
        chunk_length--;

        format_utils_start_section_tab (&tab, _("Compression"));

        if (!compression_method)
            compression_method_value = _("zlib-format DEFLATE");
        else
            compression_method_value = _("<span foreground=\"red\">INVALID</span>");

        format_utils_add_line_tab (&tab, _("Compression method"), compression_method_value,
                                 _("ICC profile compression method\n"
                                   "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));

        if (!compression_method && compressed_profile)
        {
            format_utils_start_section_tab (&tab, _("ZLIB compression"));

            png_zlib_deflate (file,
                              &tab,
                              compressed_profile,
                              chunk_length,
                              &deflate_size,
                              NULL,
                              NULL,
                              &compression_method_found,
                              &flags_found,
                              &decompression_success,
                              &checksum_found);

            if (compression_method_found)
            {
                format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                      _("ZLIB compression method and flags (CMF)\n"
                                        "Lower four bits: compression method (CM)\n"
                                        "Upper four bits: compression info (CINFO)"), NULL);
                chunk_length--;

                if (flags_found)
                {
                    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                          _("ZLIB flags (FLG)"), NULL);
                    chunk_length--;

                    if (decompression_success)
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, deflate_size,
                                              _("ZLIB compressed ICC profile"), NULL);
                        chunk_length -= deflate_size;

                        if (checksum_found)
                        {
                            format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 4,
                                                  _("ZLIB Adler32 checksum"), NULL);
                            chunk_length -= 4;
                        }
                    }
                }
            }
        }

        /* If there is data left, tag it as unrecognized */
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("Unrecognized data"), NULL);
    }

    format_utils_add_line_no_section_tab (&tab, _("NOTE: ICC profile names are encoded using ISO-8859-1"));

    format_utils_insert_tab (file, &tab, chunk_types[iCCP]);

    return TRUE;
}
