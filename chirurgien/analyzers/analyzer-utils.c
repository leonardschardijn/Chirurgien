/* analyzer-utils.c
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

#include "analyzer-utils.h"


/*
 * Change the tooltip on the hexadecimal/text view
 */
static gboolean
handle_tag_events (__attribute__((unused)) GtkTextTag  *tag,
                   GObject     *object,
                   GdkEvent    *event,
                   __attribute__((unused)) GtkTextIter *iter,
                   gpointer     user_data)
{
    gchar *tooltip = user_data;

    if (event->type == GDK_MOTION_NOTIFY)
        gtk_widget_set_tooltip_text (GTK_WIDGET (object), tooltip);

    return TRUE;
}

/*
 * Create a color tag on the hexadecimal/text view using the current index
 */
void
analyzer_utils_create_tag (AnalyzerFile *file,
                           GdkRGBA *color,
                           gboolean set_background,
                           glong count,
                           gchar *tooltip,
                           gchar *navigation_label)
{
    GtkTextIter start, end;
    GtkTextTag *hex_tag, *text_tag;
    GtkTextMark *mark;

    gsize hex_buffer_count;

    if (count == -1)
        count = file->file_size - file->file_contents_index;

    if (set_background)
    {
        hex_tag = gtk_text_buffer_create_tag (file->hex_buffer, NULL, "background-rgba", color, NULL);
        text_tag = gtk_text_buffer_create_tag (file->text_buffer, NULL, "background-rgba", color, NULL);
    }
    else
    {
        hex_tag = gtk_text_buffer_create_tag (file->hex_buffer, NULL, "foreground-rgba", color, NULL);
        text_tag = gtk_text_buffer_create_tag (file->text_buffer, NULL, "foreground-rgba", color, NULL);
    }

    g_signal_connect (hex_tag, "event", G_CALLBACK (handle_tag_events), tooltip);
    g_signal_connect (text_tag, "event", G_CALLBACK (handle_tag_events), tooltip);

    hex_buffer_count = count * 3;

    gtk_text_buffer_get_iter_at_offset (file->hex_buffer, &start, file->hex_buffer_index);
    gtk_text_buffer_get_iter_at_offset (file->hex_buffer, &end, file->hex_buffer_index + hex_buffer_count - 1);
    gtk_text_buffer_apply_tag (file->hex_buffer, hex_tag, &start, &end);

    if (navigation_label != NULL)
    {
        mark = gtk_text_buffer_create_mark (file->hex_buffer, NULL, &start, TRUE);
        file->hex_navigation_marks = g_slist_append (file->hex_navigation_marks, mark);
        file->hex_navigation_marks = g_slist_append (file->hex_navigation_marks, navigation_label);
    }

    gtk_text_buffer_get_iter_at_offset (file->text_buffer, &start, file->hex_buffer_index);
    gtk_text_buffer_get_iter_at_offset (file->text_buffer, &end, file->hex_buffer_index + hex_buffer_count - 1);
    gtk_text_buffer_apply_tag (file->text_buffer, text_tag, &start, &end);

    if (navigation_label != NULL)
    {
        mark = gtk_text_buffer_create_mark (file->text_buffer, NULL, &start, TRUE);
        file->text_navigation_marks = g_slist_append (file->text_navigation_marks, mark);
        file->text_navigation_marks = g_slist_append (file->text_navigation_marks, navigation_label);
    }

    file->hex_buffer_index += hex_buffer_count;
}

/*
 * Create a color tag on the hexadecimal/text view using a supplied index
 */
void
analyzer_utils_create_tag_index (AnalyzerFile *file,
                                 GdkRGBA *color,
                                 gboolean set_background,
                                 glong count,
                                 gsize index,
                                 gchar *tooltip)
{
    GtkTextIter start, end;
    GtkTextTag *hex_tag, *text_tag;

    gsize hex_buffer_index, hex_buffer_count;

    if (set_background)
    {
        hex_tag = gtk_text_buffer_create_tag (file->hex_buffer, NULL, "background-rgba", color, NULL);
        text_tag = gtk_text_buffer_create_tag (file->text_buffer, NULL, "background-rgba", color, NULL);
    }
    else
    {
        hex_tag = gtk_text_buffer_create_tag (file->hex_buffer, NULL, "foreground-rgba", color, NULL);
        text_tag = gtk_text_buffer_create_tag (file->text_buffer, NULL, "foreground-rgba", color, NULL);
    }
    
    g_signal_connect (hex_tag, "event", G_CALLBACK (handle_tag_events), tooltip);
    g_signal_connect (text_tag, "event", G_CALLBACK (handle_tag_events), tooltip);

    hex_buffer_index = index * 3;
    hex_buffer_count = count * 3;

    gtk_text_buffer_get_iter_at_offset (file->hex_buffer, &start, hex_buffer_index);
    gtk_text_buffer_get_iter_at_offset (file->hex_buffer, &end, hex_buffer_index + hex_buffer_count  - 1);
    gtk_text_buffer_apply_tag (file->hex_buffer, hex_tag, &start, &end);

    gtk_text_buffer_get_iter_at_offset (file->text_buffer, &start, hex_buffer_index);
    gtk_text_buffer_get_iter_at_offset (file->text_buffer, &end, hex_buffer_index + hex_buffer_count  - 1);
    gtk_text_buffer_apply_tag (file->text_buffer, text_tag, &start, &end);
}

/*
 * Add a line to the main description tab
 */
void
analyzer_utils_add_description (AnalyzerFile *file,
                                gchar *field1,
                                gchar *field2,
                                gchar *tooltip,
                                guint margin_top,
                                guint margin_bottom)
{
    GtkWidget *label1, *label2;
    GtkStyleContext *context;

    if (file->file_description == NULL)
        return;

    label1 = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label1), field1);
    gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label1), 1.0);
    gtk_widget_set_halign (label1, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (label1, margin_top);
    gtk_widget_set_margin_bottom (label1, margin_bottom);
    gtk_widget_show (label1);

    if (tooltip)
        gtk_widget_set_tooltip_markup (GTK_WIDGET (label1), tooltip);

    if (field2 == NULL)
    {
        gtk_grid_attach(GTK_GRID (file->file_description), label1, 0, file->description_lines_count++, 2, 1);
    }
    else
    {
        label2 = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (label2), field2);
        gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
        gtk_label_set_xalign (GTK_LABEL (label2), 0.0);
        gtk_widget_set_halign (label2, GTK_ALIGN_START);
        gtk_widget_set_margin_top (label2, margin_top);
        gtk_widget_set_margin_bottom (label2, margin_bottom);
        gtk_widget_show (label2);

        context = gtk_widget_get_style_context (label1);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_widget_set_halign (label1, GTK_ALIGN_END);

        gtk_grid_attach (GTK_GRID (file->file_description), label1, 0, file->description_lines_count++, 1, 1);
        gtk_grid_attach_next_to (GTK_GRID (file->file_description), label2, label1, GTK_POS_RIGHT, 1, 1);
    }
}

/*
 * Add a line to the specified grid using the supplied position
 * Useful when an analyzer wants to add additional description tabs
 */
void
analyzer_utils_add_description_here (GtkGrid *description_grid,
                                     guint *grid_position,
                                     gchar *field1,
                                     gchar *field2,
                                     gchar *tooltip,
                                     guint margin_top,
                                     guint margin_bottom)
{
    GtkWidget *label1, *label2;
    GtkStyleContext *context;

    label1 = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label1), field1);
    gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label1), 0.0);
    gtk_widget_set_halign (label1, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (label1, margin_top);
    gtk_widget_set_margin_bottom (label1, margin_bottom);

    if (tooltip)
        gtk_widget_set_tooltip_markup (GTK_WIDGET (label1), tooltip);

    if (field2 == NULL)
    {
        gtk_grid_attach(description_grid, label1, 0, (*grid_position)++, 2, 1);
    }
    else
    {
        label2 = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (label2), field2);
        gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
        gtk_label_set_xalign (GTK_LABEL (label2), 0.0);
        gtk_widget_set_halign (label2, GTK_ALIGN_START);
        gtk_widget_set_margin_top (label2, margin_top);
        gtk_widget_set_margin_bottom (label2, margin_bottom);

        context = gtk_widget_get_style_context (label1);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_widget_set_halign (label1, GTK_ALIGN_END);

        gtk_grid_attach (description_grid, label1, 0, (*grid_position)++, 1, 1);
        gtk_grid_attach_next_to (description_grid, label2, label1, GTK_POS_RIGHT, 1, 1);
    }
}

/*
 * Read 'count' bytes from the file
 */
gboolean
analyzer_utils_read (void *buffer,
                     AnalyzerFile *file,
                     gsize count)
{
    if ((file->file_contents_index + count) <= file->file_size)
        memmove (buffer, file->file_contents + file->file_contents_index, count);
    else
        return FALSE;

    file->file_contents_index += count;

    return TRUE;
}

