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

    gchar *description_message1, *description_message2;

    guint16 frequency;

    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[hIST]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    if (palette_entries << 1 != chunk_length)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The hIST chunk must have an entry for every palette entry"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Palette entry frequency</b>"));

    for (i = 0; i < palette_entries; i++)
    {
        if (!analyzer_utils_read (&frequency, file , 2))
            goto END_ERROR;

        if (i % 2)
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2, _("Palette entry frequency"));
        else
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2, _("Palette entry frequency"));

        frequency = g_ntohs (frequency);
        description_message1 = g_strdup_printf (_("Entry %u"), i);
        description_message2 = g_strdup_printf ("%u", frequency);
        analyzer_utils_describe_tab (&tab, description_message1, description_message2);
        g_free (description_message1);
        g_free (description_message2);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[hIST]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
