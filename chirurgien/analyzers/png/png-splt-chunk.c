/* png-splt-chunk.c
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
analyze_splt_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    g_autofree gchar *splt_chunk = NULL;

    g_autofree gchar *palette_name = NULL;

    gsize i;
    gsize palette_name_length = 0, palette_name_length_utf8,
          palette_entries;

    guint8 sample_depth = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sPLT]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    splt_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (splt_chunk, file, chunk_length))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Chunk length exceeds available data"), NULL);
        return FALSE;
    }

    /* The null character separes the palette name and the sample depth + palette entries */
    /* The palette name must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (splt_chunk[i] == '\0')
        {
            palette_name = splt_chunk;
            palette_name_length = i;

            if (i + 2 <= chunk_length)
                sample_depth = splt_chunk[i + 1];

            break;
        }
    }

    if (palette_name_length == 0 || palette_name_length >= 80)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid palette name length"), NULL);
        palette_name = NULL;

        return TRUE;
    }

    palette_name = g_convert (palette_name, palette_name_length, "UTF-8", "ISO-8859-1", 
                         NULL, &palette_name_length_utf8, NULL);

    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, palette_name_length,
                               _("Palette name"), NULL);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                               _("Null separator"), NULL);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                               _("Sample depth"), NULL);

    chunk_length -= palette_name_length + 2;

    if (sample_depth == 8)
    {
        if (chunk_length % 6)
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                       _("Invalid palette entry length"), NULL);
            return TRUE;
        }
        else
        {
            palette_entries = chunk_length / 6;

            for (i = 0; i < palette_entries; i++)
            {
                if (i % 2)
                {
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                               _("Palette entry red sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                               _("Palette entry green sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                               _("Palette entry blue sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                               _("Palette entry alpha sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry frequency"), NULL);
                }
                else
                {
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                               _("Palette entry red sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                               _("Palette entry green sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                               _("Palette entry blue sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                               _("Palette entry alpha sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry frequency"), NULL);
                }
            }
        }
    }
    else if (sample_depth == 16)
    {
        if (chunk_length % 10)
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                       _("Invalid palette entry length"), NULL);
            return TRUE;
        }
        else
        {
            palette_entries = chunk_length / 10;

            for (i = 0; i < palette_entries; i++)
            {
                if (i % 2)
                {
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry red sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry green sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry blue sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry alpha sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry frequency"), NULL);
                }
                else
                {
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry red sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry green sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry blue sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 2,
                                               _("Palette entry alpha sample"), NULL);
                    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 2,
                                               _("Palette entry frequency"), NULL);
                }
            }
        }
    }
    else
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid sample depth"), NULL);
        return TRUE;
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *grid, *box, *textview, *frame, *label;
        GtkTextBuffer *buffer;
        GtkStyleContext *context;

        gchar *description_message;

        guint description_lines_count = 0;

        scrolled = gtk_scrolled_window_new (NULL, NULL);

        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
        gtk_widget_set_margin_start (box, 10);
        gtk_widget_set_margin_end (box, 10);
        gtk_widget_set_margin_bottom (box, 10);
        gtk_widget_set_margin_top (box, 10);

        grid = gtk_grid_new ();
        gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
        gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

        frame = gtk_frame_new (_("Palette name"));
        gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

        textview = gtk_text_view_new ();
        gtk_widget_set_margin_start (textview, 10);
        gtk_widget_set_margin_end (textview, 10);
        gtk_widget_set_margin_bottom (textview, 10);
        gtk_widget_set_margin_top (textview, 10);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        gtk_text_buffer_set_text (buffer, palette_name, palette_name_length_utf8);

        gtk_container_add (GTK_CONTAINER (frame), textview);

        gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

        if (sample_depth == 8 || sample_depth == 16)
            description_message = g_strdup_printf (_("%u bits"), sample_depth);
        else
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                         _("Sample depth"), description_message,
                         _("Sample depths\n"
                           "<tt>08<sub>16</sub></tt>\t8 bits\n"
                           "<tt>10<sub>16</sub></tt>\t16 bits"),
                         0, 10);
        g_free (description_message);

        gtk_box_pack_start (GTK_BOX (box), grid, FALSE, FALSE, 0);

        label = gtk_label_new (_("NOTE: Palette names are encoded using ISO-8859-1"));
        context = gtk_widget_get_style_context (label);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_label_set_line_wrap (GTK_LABEL (label), GTK_WRAP_WORD);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

        gtk_container_add (GTK_CONTAINER (scrolled), box);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("sPLT");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
