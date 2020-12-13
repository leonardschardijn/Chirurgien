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

#include <arpa/inet.h>
#include <glib/gi18n.h>

#include "png-analyzer.h"


gboolean
analyze_hist_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts,
                    guint palette_entries)
{
    guint16 histogram[256];

    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[hIST]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (palette_entries << 1 != chunk_length)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The hIST chunk must have an entry for every palette entry"), NULL);
        return TRUE;
    }

    for (i = 0; i < palette_entries; i++)
    {
        if (!analyzer_utils_read (&histogram[i], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        histogram[i] = ntohs (histogram[i]);

        if (i % 2)
            analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                       _("Palette entry frequency"), NULL);
        else
            analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                       _("Palette entry frequency"), NULL);
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *grid, *label;
        gchar *description_message1;
        gchar *description_message2;

        guint description_lines_count = 0;

        scrolled = gtk_scrolled_window_new (NULL, NULL);

        grid = gtk_grid_new ();
        gtk_widget_set_margin_start (grid, 10);
        gtk_widget_set_margin_end (grid, 10);
        gtk_widget_set_margin_bottom (grid, 10);
        gtk_widget_set_margin_top (grid, 10);
        gtk_grid_set_column_spacing (GTK_GRID (grid), 40);
        gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                     _("<b>Palette entry frequency</b>"), NULL, NULL,
                                     0, 20);
        for (i = 0; i < palette_entries; i++)
        {
            description_message1 = g_strdup_printf (_("Entry %u"), i);
            description_message2 = g_strdup_printf ("%u", histogram[i]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         description_message1, description_message2, NULL,
                                         0, 0);
            g_free (description_message1);
            g_free (description_message2);
        }

        gtk_container_add (GTK_CONTAINER (scrolled), grid);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("hIST");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
