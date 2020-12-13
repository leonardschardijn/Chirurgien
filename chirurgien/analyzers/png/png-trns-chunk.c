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

#include <config.h>

#include <arpa/inet.h>
#include <glib/gi18n.h>

#include "png-analyzer.h"


gboolean
analyze_trns_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts,
                    guint8 colortype, guint palette_entries)
{
    gsize bytes_left = chunk_length;
    guint16 alpha[3];

    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tRNS]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (colortype == 0)
    {
        if (!analyzer_utils_read (&alpha[0], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        alpha[0] = ntohs (alpha[0]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Grayscale alpha channel"), NULL);
        bytes_left -= 2;
    }
    else if (colortype == 2)
    {
        if (!analyzer_utils_read (&alpha[0], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        alpha[0] = ntohs (alpha[0]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Red sample alpha channel"), NULL);
        bytes_left -= 2;
        
        if (!analyzer_utils_read (&alpha[1], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        alpha[1] = ntohs (alpha[1]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Green alpha channel"), NULL);
        bytes_left -= 2;
        
        if (!analyzer_utils_read (&alpha[2], file , 2))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data"), NULL);
            return FALSE;
        }
        alpha[2] = ntohs (alpha[2]);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                   _("Blue alpha channel"), NULL);
        bytes_left -= 2;
    }
    else if (colortype == 3)
    {
        /* Advance pointer */
        file->file_contents_index += chunk_length;

        if (palette_entries < chunk_length)
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                       _("The tRNS chunk has more alpha values than palette entries"), NULL);
            return TRUE;
        }

        for (i = 0; i < chunk_length; i++)
        {
            if (i % 2)
                analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                           _("Palette entry alpha"), NULL);
            else
                analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                           _("Palette entry alpha"), NULL);
        }
        bytes_left -= chunk_length;
    }
    else
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                           _("tRNS chunks are only valid in grayscale, RGB and indexed-color images"), NULL);
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
                                     _("<b>Transparency</b>"), NULL, NULL,
                                     0, 20);

        if (colortype == 0)
        {
            description_message = g_strdup_printf ("%u", alpha[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Grayscale alpha"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }
        else if (colortype == 2)
        {
            description_message = g_strdup_printf ("%u", alpha[0]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Red channel alpha"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%u", alpha[1]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Green channel alpha"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%u", alpha[2]);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Blue channel alpha"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }
        else
        {
            description_message = g_strdup_printf ("%u", palette_entries);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Palette entries"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%lu", chunk_length);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Alpha entries"), description_message, NULL,
                                         0, 0);
            g_free (description_message);

            description_message = g_strdup_printf ("%lu", chunk_length - palette_entries);
            analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                         _("Palette entries without alpha"), description_message, NULL,
                                         0, 0);
            g_free (description_message);
        }

        gtk_container_add (GTK_CONTAINER (scrolled), grid);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("tRNS");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
