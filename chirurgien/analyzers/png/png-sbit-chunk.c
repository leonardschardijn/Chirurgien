/* png-sbit-chunk.c
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
analyze_sbit_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    gboolean is_rgb = FALSE;
    gsize bytes_left = chunk_length;
    guint8 significant_bits[4] = {0, 0, 0, 0};

    if (!chunk_length)
        return TRUE;

    chunk_counts[sBIT]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (!analyzer_utils_read (&significant_bits[0], file, 1))
        return FALSE;

    if (chunk_length <= 2)
    {
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("Grayscale sample significant bits"), NULL);

        bytes_left -= 1;
    }
    else if (chunk_length <= 4)
    {
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("Red sample significant bits"), NULL);

        if (!analyzer_utils_read (&significant_bits[1], file, 1))
            return FALSE;
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                   _("Green sample significant bits"), NULL);

        if (!analyzer_utils_read (&significant_bits[2], file, 1))
            return FALSE;
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("Blue sample significant bits"), NULL);

        is_rgb = TRUE;
        bytes_left -= 3;
    }

    if (chunk_length == 2 || chunk_length == 4)
    {
        if (!analyzer_utils_read (&significant_bits[3], file, 1))
            return FALSE;
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                   _("Alpha sample significant bits"), NULL);

        bytes_left -= 1;
    }

    if (bytes_left > 0)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, bytes_left,
                                   _("Unrecognized data"), NULL);
        /* Advance pointer */
        file->file_contents_index += bytes_left;
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *grid, *label;
        gchar *description_message;

        guint description_lines_count = 0;

        scrolled = gtk_scrolled_window_new (NULL, NULL);

        grid = gtk_grid_new ();
        gtk_widget_set_margin_start (grid, 10);
        gtk_widget_set_margin_end (grid, 10);
        gtk_widget_set_margin_bottom (grid, 10);
        gtk_widget_set_margin_top (grid, 10);
        gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
        gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                     _("<b>Original number of significant bits</b>"), NULL, NULL,
                                     0, 20);

        if (is_rgb)
        {
            description_message = g_strdup_printf (_("%u bits"), significant_bits[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Red sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf (_("%u bits"), significant_bits[1]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Green sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf (_("%u bits"), significant_bits[2]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Blue sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }
        else
        {
            description_message = g_strdup_printf (_("%u bits"), significant_bits[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Grayscale sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }

        if (significant_bits[3])
        {
            description_message = g_strdup_printf (_("%u bits"), significant_bits[3]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Alpha sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }

        gtk_container_add (GTK_CONTAINER (scrolled), grid);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("sBIT");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
