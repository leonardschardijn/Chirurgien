/* png-text-chunk.c
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
analyze_text_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    g_autofree gchar *text_chunk = NULL;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    gsize i;
    gsize keyword_length = 0, keyword_length_utf8;
    gsize text_length = 0, text_length_utf8;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tEXt]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    text_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (text_chunk, file, chunk_length))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Chunk length exceeds available data"), NULL);
        return FALSE;
    }

    /* The null character separes the keyword and the text string */
    /* The keyword must the 1-79 bytes long */
    if (*text_chunk == '\0')
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid keyword length"), NULL);
        return TRUE;
    }

    for (i = 0; i < chunk_length; i++)
    {
        if (text_chunk[i] == '\0')
        {
            keyword = text_chunk;
            keyword_length = i;

            if (i != chunk_length - 1)
            {
                text = text_chunk + i + 1;
                text_length = chunk_length - i - 1;
            }
            break;
        }
    }

    if (keyword == NULL)
    {
        keyword = text_chunk;
        keyword_length = chunk_length;
    }

    if (keyword_length >= 80)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid keyword length"), NULL);
        keyword = NULL;
        text = NULL;

        return TRUE;
    }

    keyword = g_convert (keyword, keyword_length, "UTF-8", "ISO-8859-1", 
                         NULL, &keyword_length_utf8, NULL);

    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, keyword_length,
                               _("Keyword"), NULL);

    if (text != NULL)
    {
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                           _("Null separator"), NULL);

        text = g_convert (text, text_length, "UTF-8", "ISO-8859-1", 
                          NULL, &text_length_utf8, NULL);

        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, text_length,
                                   _("Text string"), NULL);
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *box, *textview, *frame, *label;
        GtkTextBuffer *buffer;
        GtkStyleContext *context;

        scrolled = gtk_scrolled_window_new (NULL, NULL);

        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
        gtk_widget_set_margin_start (box, 10);
        gtk_widget_set_margin_end (box, 10);
        gtk_widget_set_margin_bottom (box, 10);
        gtk_widget_set_margin_top (box, 10);

        frame = gtk_frame_new (_("Keyword"));
        gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

        textview = gtk_text_view_new ();
        gtk_widget_set_margin_start (textview, 10);
        gtk_widget_set_margin_end (textview, 10);
        gtk_widget_set_margin_bottom (textview, 10);
        gtk_widget_set_margin_top (textview, 10);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        gtk_text_buffer_set_text (buffer, keyword, keyword_length_utf8);

        gtk_container_add (GTK_CONTAINER (frame), textview);

        gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

        if (text != NULL)
        {
            frame = gtk_frame_new (_("Text string"));
            gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

            textview = gtk_text_view_new ();
            gtk_widget_set_margin_start (textview, 10);
            gtk_widget_set_margin_end (textview, 10);
            gtk_widget_set_margin_bottom (textview, 10);
            gtk_widget_set_margin_top (textview, 10);
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
            gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
            gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
            buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
            gtk_text_buffer_set_text (buffer, text, text_length_utf8);

            gtk_container_add (GTK_CONTAINER (frame), textview);

            gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);
        }

        label = gtk_label_new (_("NOTE: tEXt chunks are encoded using ISO-8859-1"));
        context = gtk_widget_get_style_context (label);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_label_set_line_wrap (GTK_LABEL (label), GTK_WRAP_WORD);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

        gtk_container_add (GTK_CONTAINER (scrolled), box);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("tEXt");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
