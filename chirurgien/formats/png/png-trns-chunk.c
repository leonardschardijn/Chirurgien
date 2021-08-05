/* png-trns-chunk.c
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
png_trns_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint8       colortype,
                guint        palette_entries)
{
    DescriptionTab tab;

    gchar *value;

    guint32 chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tRNS]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, "Transparency");

    if (colortype == 0)
    {
        if (!process_png_field (file, &tab, "Grayscale alpha channel", NULL,
                                NULL,
                                CHUNK_DATA_COLOR_1, 2,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        chunk_used += 2;
    }
    else if (colortype == 2)
    {
        if (!process_png_field (file, &tab, "Red alpha channel", NULL,
                                NULL,
                                CHUNK_DATA_COLOR_1, 2,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, "Green alpha channel", NULL,
                                NULL,
                                CHUNK_DATA_COLOR_2, 2,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, "Blue alpha channel", NULL,
                                NULL,
                                CHUNK_DATA_COLOR_1, 2,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        chunk_used += 6;
    }
    else if (colortype == 3)
    {
        if (palette_entries < chunk_length)
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                    "The tRNS chunk has more alpha values than palette entries", NULL);
            return TRUE;
        }

        for (guint32 i = 0; i < chunk_length; i++)
        {
            if (i % 2)
                format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                        "Palette entry alpha", NULL);
            else
                format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                        "Palette entry alpha", NULL);
        }

        value = g_strdup_printf ("%u", palette_entries);
        format_utils_add_line_tab (&tab, "Palette entries", value, NULL);
        g_free (value);

        value = g_strdup_printf ("%u", chunk_length);
        format_utils_add_line_tab (&tab, "Alpha entries", value, NULL);
        g_free (value);

        value = g_strdup_printf ("%u", chunk_length - palette_entries);
        format_utils_add_line_tab (&tab, "Palette entries without alpha", value, NULL);
        g_free (value);

        chunk_used += chunk_length;
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "tRNS chunks are only valid in grayscale, RGB and indexed-color images", NULL);
        return TRUE;
    }

    if (chunk_used < chunk_length)
    {
        chunk_length -= chunk_used;
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "Unrecognized data", NULL);
    }

    format_utils_insert_tab (file, &tab, chunk_types[tRNS]);

    return TRUE;
}
