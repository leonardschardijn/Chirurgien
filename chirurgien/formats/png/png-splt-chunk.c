/* png-splt-chunk.c
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
png_splt_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    const gchar *splt_chunk;

    g_autofree gchar *palette_name = NULL;
    gchar *value;

    guint32 i, palette_entries = 0, palette_name_length = 0;
    gsize utf8_length;

    gint sample_depth = -1;

    gboolean null_found = FALSE;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sPLT]++;

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

    splt_chunk = (const gchar *) GET_CONTENT_POINTER (file);

    /* The null character separes the palette name and the sample depth + palette entries */
    /* The palette name must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (splt_chunk[i] == '\0')
        {
            palette_name_length = i;

            null_found = TRUE;

            if (i + 1 < chunk_length)
                sample_depth = splt_chunk[i + 1];

            break;
        }
    }

    /* If 0, no null separator was found: there is no palette */
    if (!palette_name_length)
        palette_name_length = chunk_length;

    palette_name = g_convert (splt_chunk, palette_name_length, "UTF-8", "ISO-8859-1",
                         NULL, &utf8_length, NULL);
    format_utils_add_text_tab (&tab, _("Palette name"), palette_name, utf8_length);

    if (!palette_name_length)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: No palette name defined, palette names should be at least 1 byte long"));
    else if (palette_name_length >= 80)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: The palette name exceeds its 79 bytes long limit"));

    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, palette_name_length,
                          _("Palette name"), NULL);
    chunk_length -= palette_name_length;

    if (null_found)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);
        chunk_length--;
    }

    if (sample_depth != -1)
    {
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                              _("Sample depth"), NULL);
        chunk_length--;

        format_utils_start_section_tab (&tab, _("Palette details"));

        if (sample_depth == 8 || sample_depth == 16)
            value = g_strdup_printf (_("%u bits"), sample_depth);
        else
            value = g_strdup (_("<span foreground=\"red\">INVALID</span>"));

        format_utils_add_line_tab (&tab, _("Sample depth"), value,
                                 _("Sample depth\n"
                                   "<tt>08<sub>16</sub></tt>\t8 bits\n"
                                   "<tt>10<sub>16</sub></tt>\t16 bits"));
        g_free (value);

        if (sample_depth == 8)
        {
            if (chunk_length % 6)
            {
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                      _("Invalid palette entry length"), NULL);
                return TRUE;
            }
            else
            {
                palette_entries = chunk_length / 6;

                for (i = 0; i < palette_entries; i++)
                {
                    if (i % 2)
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 3,
                                              _("Palette entry"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                              _("Palette entry aplha"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 2,
                                              _("Palette entry frequency"), NULL);
                    }
                    else
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 3,
                                              _("Palette entry"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                              _("Palette entry aplha"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 2,
                                              _("Palette entry frequency"), NULL);
                    }
                }
            }
        }
        else if (sample_depth == 16)
        {
            if (chunk_length % 10)
            {
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                      _("Invalid palette entry length"), NULL);
                return TRUE;
            }
            else
            {
                palette_entries = chunk_length / 10;

                for (i = 0; i < palette_entries; i++)
                {
                    if (i % 2)
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 6,
                                              _("Palette entry"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 2,
                                              _("Palette entry aplha"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 2,
                                              _("Palette entry frequency"), NULL);
                    }
                    else
                    {
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 6,
                                              _("Palette entry"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 2,
                                              _("Palette entry aplha"), NULL);
                        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 2,
                                              _("Palette entry frequency"), NULL);

                    }
                }
            }
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                  _("Invalid sample depth"), NULL);
            return TRUE;
        }
    }

    if (palette_entries)
    {
        value = g_strdup_printf ("%u", palette_entries);
        format_utils_add_line_tab (&tab, _("Palette entries"), value, NULL);
        g_free (value);
    }

    format_utils_add_line_no_section_tab (&tab, _("NOTE: Palette names are encoded using ISO-8859-1"));

    format_utils_insert_tab (file, &tab, chunk_types[sPLT]);

    return TRUE;
}
