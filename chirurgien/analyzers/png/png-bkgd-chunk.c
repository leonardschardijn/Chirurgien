/* png-bkgd-chunk.c
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
analyze_bkgd_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts,
                    guint8 colortype)
{
    gsize bytes_left = chunk_length;
    guint16 color[3];
    guint8 palette_index;

    if (!chunk_length)
        return TRUE;

    chunk_counts[bKGD]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (colortype == 0 || colortype == 4)
    {
        if (!analyzer_utils_read (&color[0], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        color[0] = ntohs (color[0]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Grayscale background"), NULL);
        bytes_left -= 2;
    }
    else if (colortype == 2 || colortype == 6)
    {
        if (!analyzer_utils_read (&color[0], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        color[0] = ntohs (color[0]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Background red sample"), NULL);
        bytes_left -= 2;
        
        if (!analyzer_utils_read (&color[1], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        color[1] = ntohs (color[1]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                   _("Background green sample"), NULL);
        bytes_left -= 2;
        
        if (!analyzer_utils_read (&color[2], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        color[2] = ntohs (color[2]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Background blue sample"), NULL);
        bytes_left -= 2;
    }
    else if (colortype == 3)
    {
        if (!analyzer_utils_read (&palette_index, file , 1))
            return FALSE;
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("Background palette index"), NULL);
        bytes_left -= 1;
    }
    else
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                           _("Invalid color type"), NULL);
        /* Advance pointer */
        file->file_contents_index += bytes_left;
        return TRUE;
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
                                     _("<b>Background color</b>"), NULL, NULL,
                                     0, 20);

        if (colortype == 0 || colortype == 4)
        {
            description_message = g_strdup_printf ("%u", color[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Grayscale background"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }
        else if (colortype == 2 || colortype == 6)
        {
            description_message = g_strdup_printf ("%u", color[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Background red sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%u", color[1]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Background green sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%u", color[2]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Background blue sample"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }
        else
        {
            description_message = g_strdup_printf ("%u", palette_index);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Background palette index"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }

        gtk_container_add (GTK_CONTAINER (scrolled), grid);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("bKGD");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
