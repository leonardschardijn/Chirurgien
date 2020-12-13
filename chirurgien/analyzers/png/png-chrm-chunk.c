/* png-chrm-chunk.c
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


enum {
    WHITE_X,
    WHITE_Y,
    RED_X,
    RED_Y,
    GREEN_X,
    GREEN_Y,
    BLUE_X,
    BLUE_Y,
    CHROMATICITIES
};

gboolean
analyze_chrm_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    gsize bytes_left = chunk_length;
    guint32 chromaticities[CHROMATICITIES];

    if (!chunk_length)
        return TRUE;

    chunk_counts[cHRM]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (!analyzer_utils_read (&chromaticities[WHITE_X], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[WHITE_X] = ntohl (chromaticities[WHITE_X]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 4,
                               _("White point x chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[WHITE_Y], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[WHITE_Y] = ntohl (chromaticities[WHITE_Y]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 4,
                               _("White point y chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[RED_X], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[RED_X] = ntohl (chromaticities[RED_X]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 4,
                               _("Red x chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[RED_Y], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[RED_Y] = ntohl (chromaticities[RED_Y]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 4,
                               _("Red y chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[GREEN_X], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[GREEN_X] = ntohl (chromaticities[GREEN_X]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 4,
                               _("Green x chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[GREEN_Y], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[GREEN_Y] = ntohl (chromaticities[GREEN_Y]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 4,
                               _("Green y chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[BLUE_X], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[BLUE_X] = ntohl (chromaticities[BLUE_X]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 4,
                               _("Blue x chromaticity"), NULL);
    bytes_left -= 4;

    if (!analyzer_utils_read (&chromaticities[BLUE_Y], file , 4))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Unrecognized data"), NULL);
        return FALSE;
    }
    chromaticities[BLUE_Y] = ntohl (chromaticities[BLUE_Y]);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 4,
                               _("Blue y chromaticity"), NULL);
    bytes_left -= 4;

    if (bytes_left > 0)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, bytes_left,
                                   _("Unrecognized data"), NULL);
        /* Advance pointer */
        file->file_contents_index += bytes_left;
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *grid, *box, *label;
        GtkStyleContext *context;
        gchar *description_message;

        guint description_lines_count = 0;

        scrolled = gtk_scrolled_window_new (NULL, NULL);
        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
        gtk_widget_set_margin_start (box, 10);
        gtk_widget_set_margin_end (box, 10);
        gtk_widget_set_margin_bottom (box, 10);
        gtk_widget_set_margin_top (box, 10);

        grid = gtk_grid_new ();
        gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
        gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("<b>Primary chromaticities and white point</b>"), NULL, NULL,
                             0, 20);

        description_message = g_strdup_printf ("%u", chromaticities[WHITE_X]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("White point x"), description_message, NULL,
                             0, 0);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[WHITE_Y]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("White point y"), description_message, NULL,
                             0, 10);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[RED_X]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Red x"), description_message, NULL,
                             0, 0);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[RED_Y]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Red y"), description_message, NULL,
                             0, 10);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[GREEN_X]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Green x"), description_message, NULL,
                             0, 0);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[GREEN_Y]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Green y"), description_message, NULL,
                             0, 10);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[BLUE_X]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Blue x"), description_message, NULL,
                             0, 0);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", chromaticities[BLUE_Y]);
        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                             _("Blue y"), description_message, NULL,
                             0, 0);
        g_free (description_message);

        gtk_box_pack_start (GTK_BOX (box), grid, FALSE, FALSE, 0);

        label = gtk_label_new (_("NOTE: Values represent the 1931 CIE x,y chromaticities times 100000"));
        context = gtk_widget_get_style_context (label);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_label_set_line_wrap (GTK_LABEL (label), GTK_WRAP_WORD);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_widget_set_halign (label, GTK_ALIGN_START);

        gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

        gtk_container_add (GTK_CONTAINER (scrolled), box);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("cHRM");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
