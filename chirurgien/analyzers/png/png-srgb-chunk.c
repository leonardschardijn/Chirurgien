/* png-srgb-chunk.c
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
analyze_srgb_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    gsize bytes_left = chunk_length;
    guint8 intent;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sRGB]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (!analyzer_utils_read (&intent, file , 1))
        return FALSE;
    bytes_left -= 1;
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                               _("Rendering intent"), NULL);

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
                                     _("<b>Standard RGB color space</b>"), NULL, NULL,
                                     0, 20);

        if (intent == 0)
            description_message = _("Perceptual");
        else if (intent == 1)
            description_message = _("Relative colorimetric");
        else if (intent == 2)
            description_message = _("Saturation");
        else if (intent == 3)
            description_message = _("Absolute colorimetric");
        else
            description_message = _("<span foreground=\"red\">INVALID</span>");

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                                     _("Rendering intent"), description_message,
                                     _("Rendering intents\n"
                                       "<tt>00<sub>16</sub></tt>\tPerceptual\n"
                                       "<tt>01<sub>16</sub></tt>\tRelative colorimetric\n"
                                       "<tt>02<sub>16</sub></tt>\tSaturation\n"
                                       "<tt>03<sub>16</sub></tt>\tAbsolute colorimetric"),
                                     0, 0);

        gtk_container_add (GTK_CONTAINER (scrolled), grid);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("sRGB");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
