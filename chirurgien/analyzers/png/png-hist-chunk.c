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

#include "png-analyzer.h"


gboolean
analyze_hist_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    guint palette_entries)
{
    AnalyzerTab tab;

    GdkRGBA *color_toggle;
    gchar *description_message;

    if (!chunk_length)
        return TRUE;

    chunk_counts[hIST]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (palette_entries << 1 != chunk_length)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The hIST chunk must have an entry for every palette entry"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Palette entry frequency</b>"));

    for (guint i = 0; i < palette_entries; i++)
    {
        if (i % 2)
            color_toggle = CHUNK_DATA_COLOR_2;
        else
            color_toggle = CHUNK_DATA_COLOR_1;

        description_message = g_strdup_printf (_("Entry %u"), i);

        if (!process_png_field (file, &tab, description_message, _("Entry frequency"),
                           NULL, color_toggle, 2, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        g_free (description_message);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[hIST]);

    return TRUE;

    END_ERROR:
    g_free (description_message);
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
