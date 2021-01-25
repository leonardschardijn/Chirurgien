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

#include <glib/gi18n.h>

#include <chirurgien-actions-analyzer.h>


/* Colors available to analyzers */
GdkRGBA colors[] =
{
    /* #2567C1 */
    { 0.145, 0.403, 0.756, 1.0 },
    /* #BB40CE */
    { 0.733, 0.250, 0.807, 0.5 },
    /* #1CAF08 */
    { 0.109, 0.686, 0.031, 0.7 },
    /* #1C6E11 */
    { 0.109, 0.431, 0.066, 0.7 },
    /* #DDE70D */
    { 0.866, 0.905, 0.050, 0.5 },
    /* #0DE7BA */
    { 0.050, 0.905, 0.729, 0.5 },
    /* #FF0000 */
    { 1.0, 0.0, 0.0, 1.0 },
    /* #E84FDB */
    { 0.909, 0.309, 0.858, 1.0 },
    /* #0C67B6 */
    { 0.047, 0.403, 0.713, 1.0 }
};

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

    hex_buffer_count = file->hex_buffer_index / 3;

    if (hex_buffer_count >= file->file_size)
        return;

    if (count == -1)
        count = file->file_size - hex_buffer_count;

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

    if (!field2)
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
 * Insert a notice at the start of the main Overview tab
 */
void
analyzer_utils_insert_notice (AnalyzerFile *file,
                              gchar *notice,
                              guint margin_top,
                              guint margin_bottom)
{
    GtkWidget *label;

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), notice);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (label, margin_top);
    gtk_widget_set_margin_bottom (label, margin_bottom);
    gtk_widget_show (label);

    gtk_grid_insert_row (GTK_GRID (file->file_description), 0);
    file->description_lines_count++;

    gtk_grid_attach (GTK_GRID (file->file_description), label, 0, 0, 2, 1);
}

/*
 * Add a line to the supplied tab
 * Useful when an analyzer wants to add additional description tabs
 */
void
analyzer_utils_add_description_tab (AnalyzerTab *description_tab,
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
        gtk_grid_attach(description_tab->description, label1, 0,
                        description_tab->description_lines_count++, 2, 1);
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

        gtk_grid_attach (description_tab->description, label1, 0,
                         description_tab->description_lines_count++, 1, 1);
        gtk_grid_attach_next_to (description_tab->description, label2, label1, GTK_POS_RIGHT, 1, 1);
    }
}

/*
 * Add a text field to the description tab
 */
void
analyzer_utils_add_text_tab (AnalyzerTab *tab,
                             gchar *name,
                             gchar *text,
                             gsize text_size)
{
    GtkWidget *textview, *widget;
    GtkTextBuffer *buffer;

    widget = gtk_frame_new (name);
    gtk_frame_set_label_align (GTK_FRAME (widget), 0.5, 0.5);

    textview = gtk_text_view_new ();
    gtk_widget_set_margin_start (textview, 10);
    gtk_widget_set_margin_end (textview, 10);
    gtk_widget_set_margin_bottom (textview, 10);
    gtk_widget_set_margin_top (textview, 10);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

    gtk_text_buffer_set_text (buffer, text, text_size);

    gtk_container_add (GTK_CONTAINER (widget), textview);

    if (tab->description_lines_count)
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), GTK_WIDGET (tab->description), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);

        widget = gtk_grid_new ();
        gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
        gtk_grid_set_column_spacing (GTK_GRID (widget), 10);

        tab->description = GTK_GRID (widget);
        tab->description_lines_count = 0;
    }
    else
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);
    }
}

/*
 * Add a footer to the description tab
 */
void
analyzer_utils_add_footer_tab (AnalyzerTab *tab,
                               gchar *footer)
{
    GtkWidget *widget;
    GtkStyleContext *context;

    widget = gtk_label_new (footer);
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
    gtk_label_set_line_wrap (GTK_LABEL (widget), GTK_WRAP_WORD);
    gtk_label_set_xalign (GTK_LABEL (widget), 0.0);
    gtk_widget_set_halign (widget, GTK_ALIGN_START);
    gtk_widget_set_margin_top (widget, 10);

    if (tab->description_lines_count)
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), GTK_WIDGET (tab->description), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);

        widget = gtk_grid_new ();
        gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
        gtk_grid_set_column_spacing (GTK_GRID (widget), 10);

        tab->description = GTK_GRID (widget);
        tab->description_lines_count = 0;
    }
    else
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);
    }
}

/*
 * Add a button to analyze an embedded file to the description tab
 */
void
analyzer_utils_embedded_file (AnalyzerFile *file,
                              AnalyzerTab *tab,
                              gsize file_size)
{
    GtkWidget *widget;

    file->embedded_files = g_slist_append (file->embedded_files, GSIZE_TO_POINTER (file->file_contents_index));
    file->embedded_files = g_slist_append (file->embedded_files, GSIZE_TO_POINTER (file_size));
    file->file_contents_index += file_size;

    widget = gtk_button_new_with_label (_("Analyze!"));
    gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (widget, 5);

    g_signal_connect (widget, "clicked", G_CALLBACK (chirurgien_actions_embedded_file),
                      GUINT_TO_POINTER (file->embedded_files_count++));

    if (tab->description_lines_count)
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), GTK_WIDGET (tab->description), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);

        widget = gtk_grid_new ();
        gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
        gtk_grid_set_column_spacing (GTK_GRID (widget), 10);

        tab->description = GTK_GRID (widget);
        tab->description_lines_count = 0;
    }
    else
    {
        gtk_box_pack_start (GTK_BOX (tab->contents), widget, FALSE, FALSE, 0);
    }
}

/*
 * Initialize the new description tab
 */
void
analyzer_utils_init_tab (AnalyzerTab *tab)
{
    GtkWidget *box;
    GtkGrid *grid;

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start (box, 10);
    gtk_widget_set_margin_end (box, 10);
    gtk_widget_set_margin_bottom (box, 10);
    gtk_widget_set_margin_top (box, 10);

    grid = GTK_GRID (gtk_grid_new ());
    gtk_widget_set_halign (GTK_WIDGET (grid), GTK_ALIGN_CENTER);
    gtk_grid_set_column_spacing (grid, 10);

    tab->contents = box;
    tab->description = grid;
    tab->description_lines_count = 0;
}

/*
 * Add the description tab
 */
void
analyzer_utils_insert_tab (AnalyzerFile *file,
                           AnalyzerTab *tab,
                           gchar *tab_name)
{
    GtkWidget *scrolled, *label;

    scrolled = gtk_scrolled_window_new (NULL, NULL);

    if (tab->description_lines_count)
        gtk_box_pack_start (GTK_BOX (tab->contents), GTK_WIDGET (tab->description), FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (tab->contents));
    gtk_widget_show_all (scrolled);

    label = gtk_label_new (tab_name);
    gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
}


/*
 * Advance pointer until the supplied byte is found
 */
gsize
analyzer_utils_advance_to (AnalyzerFile *file,
                           guchar byte)
{
    gsize prev_index = file->file_contents_index;

    while (file->file_contents_index <= file->file_size)
    {
        if (G_UNLIKELY (*(file->file_contents + file->file_contents_index) == byte))
            return file->file_contents_index - prev_index;
        file->file_contents_index++;
    }

    return 0;
}

/*
 * Read 'count' bytes from the file
 */
gboolean
analyzer_utils_read (void *buffer,
                     AnalyzerFile *file,
                     gsize count)
{
    gboolean success = TRUE;

    if ((file->file_contents_index + count) <= file->file_size)
        memmove (buffer, file->file_contents + file->file_contents_index, count);
    else
        success = FALSE;

    file->file_contents_index += count;

    return success;
}

/*
 * Order tagged area indices
 */
gint
tagged_bytes_compare (gconstpointer a,
                      gconstpointer b)
{
    if (a < b)
        return -1;
    else if (a > b)
        return 1;
    else
        return 0;
}
