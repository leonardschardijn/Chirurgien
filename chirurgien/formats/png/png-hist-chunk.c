/* png-hist-chunk.c
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
png_hist_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint        palette_entries)
{
    DescriptionTab tab;

    gint color_toggle;
    gchar *entry_number;

    if (!chunk_length)
        return TRUE;

    chunk_counts[hIST]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (palette_entries << 1 != chunk_length)
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The hIST chunk must have an entry for every palette entry"), NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, _("Palette entry frequency"));

    for (guint i = 0; i < palette_entries; i++)
    {
        if (i % 2)
            color_toggle = CHUNK_DATA_COLOR_2;
        else
            color_toggle = CHUNK_DATA_COLOR_1;

        entry_number = g_strdup_printf (_("Entry %u"), i);

        if (!process_png_field (file, &tab, entry_number, _("Entry frequency"),
                           NULL, color_toggle, 2, 0, NULL, NULL, "%u", NULL))
        {
            g_free (entry_number);
            return FALSE;
        }

        g_free (entry_number);
    }

    format_utils_insert_tab (file, &tab, chunk_types[hIST]);

    return TRUE;
}
