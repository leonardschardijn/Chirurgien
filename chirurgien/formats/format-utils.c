/* format-utils.c
 *
 * Copyright (C) 2021 - Daniel Léonard Schardijn
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

#include "format-utils.h"

#include <glib/gi18n.h>


static void
create_section (GtkWidget  **expander,
                GtkWidget  **grid,
                const gchar *section_name)
{
    *grid = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (*grid), 10);
    gtk_widget_set_margin_start (*grid, 10);
    gtk_widget_set_margin_end (*grid, 10);
    gtk_widget_set_margin_top (*grid, 10);
    gtk_widget_set_margin_bottom (*grid, 10);
    gtk_widget_set_halign (*grid, GTK_ALIGN_CENTER);

    *expander = gtk_expander_new (section_name);
    gtk_expander_set_child (GTK_EXPANDER (*expander), *grid);
    gtk_expander_set_expanded (GTK_EXPANDER (*expander), TRUE);
}

static void
create_line_labels (GtkWidget  **left_label,
                    GtkWidget  **right_label,
                    const gchar *field_name,
                    const gchar *field_value,
                    const gchar *field_tooltip,
                    gint         margin_top,
                    gint         margin_bottom)
{
    *left_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (*left_label), field_name);
    gtk_label_set_wrap (GTK_LABEL (*left_label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (*left_label), 1.0);
    gtk_widget_set_halign (*left_label, GTK_ALIGN_END);
    gtk_widget_set_margin_top (*left_label, margin_top);
    gtk_widget_set_margin_bottom (*left_label, margin_bottom);
    gtk_widget_add_css_class (*left_label, "dim-label");

    if (field_tooltip)
        gtk_widget_set_tooltip_markup (GTK_WIDGET (*left_label), field_tooltip);

    *right_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (*right_label), field_value);
    gtk_label_set_wrap (GTK_LABEL (*right_label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (*right_label), 0.0);
    gtk_widget_set_halign (*right_label, GTK_ALIGN_START);
    gtk_widget_set_margin_top (*right_label, margin_top);
    gtk_widget_set_margin_bottom (*right_label, margin_bottom);
}

void
format_utils_set_title (FormatsFile *file,
                        const char  *title)
{
    GtkWidget *label;
    PangoAttrList *attribute_list;
    PangoAttribute *size, *weight;

    attribute_list = pango_attr_list_new ();

    weight = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
    size = pango_attr_scale_new (PANGO_SCALE_LARGE);

    weight->start_index = size->start_index = PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING;
    weight->end_index = size->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;

    pango_attr_list_insert (attribute_list, weight);
    pango_attr_list_insert (attribute_list, size);

    label = gtk_label_new (title);
    gtk_label_set_attributes (GTK_LABEL (label), attribute_list);

    pango_attr_list_unref (attribute_list);

    gtk_box_append (file->overview, label);
}

void
format_utils_start_section (FormatsFile *file,
                            const gchar *section_name)
{
    GtkWidget *expander, *grid;

    create_section (&expander, &grid, section_name);

    gtk_box_append (file->overview, expander);

    file->section = GTK_GRID (grid);
    file->description_lines_count = 0;
}

void
format_utils_start_section_tab (DescriptionTab *tab,
                                const gchar    *section_name)
{
    GtkWidget *expander, *grid;

    create_section (&expander, &grid, section_name);

    gtk_box_append (tab->contents, expander);

    tab->section = GTK_GRID (grid);
    tab->description_lines_count = 0;
}

void
format_utils_init_tab (DescriptionTab *tab,
                       const gchar    *section_name)
{
    tab->contents = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 10));
    gtk_widget_set_can_focus (GTK_WIDGET (tab->contents), FALSE);
    gtk_widget_set_margin_start (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_end (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_top (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_bottom (GTK_WIDGET (tab->contents), 10);

    if (section_name)
        format_utils_start_section_tab (tab, section_name);

    tab->used = FALSE;
}

void
format_utils_insert_tab (FormatsFile    *file,
                         DescriptionTab *tab,
                         const gchar    *tab_name)
{
    GtkWidget *scrolled, *label;

    if (!tab->used)
        return;

    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled),
                                   GTK_WIDGET (tab->contents));

    label = gtk_label_new (tab_name);

    gtk_notebook_insert_page (file->description, scrolled, label, -1);
}

void
format_utils_add_line_full (FormatsFile *file,
                            const gchar *field_name,
                            const gchar *field_value,
                            const gchar *field_tooltip,
                            gint         margin_top,
                            gint         margin_bottom)
{
    GtkWidget *left_label, *right_label;

    create_line_labels (&left_label, &right_label,
                        field_name, field_value, field_tooltip,
                        margin_top, margin_bottom);

    gtk_grid_attach (GTK_GRID (file->section), left_label, 0, file->description_lines_count++, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (file->section), right_label, left_label, GTK_POS_RIGHT, 1, 1);
}

void
format_utils_add_line_full_tab (DescriptionTab *tab,
                                const gchar    *field_name,
                                const gchar    *field_value,
                                const gchar    *field_tooltip,
                                gint            margin_top,
                                gint            margin_bottom)
{
    GtkWidget *left_label, *right_label;

    create_line_labels (&left_label, &right_label,
                        field_name, field_value, field_tooltip,
                        margin_top, margin_bottom);

    gtk_grid_attach (GTK_GRID (tab->section), left_label, 0, tab->description_lines_count++, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (tab->section), right_label, left_label, GTK_POS_RIGHT, 1, 1);

    tab->used = TRUE;
}

void
format_utils_add_text_tab (DescriptionTab *tab,
                           const gchar    *field_name,
                           const gchar    *text,
                           gsize           text_size)
{
    GtkWidget *textview, *expander;
    GtkTextBuffer *buffer;
    GtkTextIter start;

    gsize print_text_size;
    gchar *truncated_message;

    print_text_size = text_size > 4096 ? 4096 : text_size;

    textview = gtk_text_view_new ();
    gtk_widget_set_margin_start (textview, 10);
    gtk_widget_set_margin_end (textview, 10);
    gtk_widget_set_margin_bottom (textview, 10);
    gtk_widget_set_margin_top (textview, 10);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

    if (g_utf8_validate_len (text, print_text_size, NULL))
    {
        gtk_text_buffer_set_text (buffer, text, print_text_size);
    }
    else
    {
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_insert_markup (buffer, &start,
                     _("<span foreground=\"red\">[INVALID ENCODING]</span>"), -1);
    }

    expander = gtk_expander_new (field_name);
    gtk_expander_set_child (GTK_EXPANDER (expander), textview);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);

    gtk_box_append (GTK_BOX (tab->contents), expander);

    if (print_text_size != text_size)
    {
        truncated_message = g_strdup_printf (_("%s was truncated from its original size of %lu"),
                                             field_name, text_size);
        format_utils_add_line_no_section_tab (tab, truncated_message);
        g_free (truncated_message);
    }

    tab->used = TRUE;
}

void
format_utils_add_line_no_section (FormatsFile *file,
                                  const gchar *line)
{
    GtkWidget *label;

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), line);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_add_css_class (label, "dim-label");

    gtk_box_append (file->overview, label);
}

void
format_utils_add_line_no_section_tab (DescriptionTab *tab,
                                      const gchar    *line)
{
    GtkWidget *label;

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), line);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_add_css_class (label, "dim-label");

    gtk_box_append (tab->contents, label);

    tab->used = TRUE;
}

void
format_utils_add_field_full (FormatsFile *file,
                             gint         color_index,
                             gboolean     background,
                             guint        field_length,
                             const gchar *field_name,
                             const gchar *navigation_label,
                             gint         additional_color_index)
{
    FileField *new_field;
    gsize available_data;

    available_data = file->file_size - file->file_contents_index;

    if (field_length > available_data)
        field_length = available_data;

    if (!field_length || file->file_size <= file->file_contents_index)
        return;

    new_field = g_slice_new (FileField);

    new_field->field_name = field_name;
    new_field->field_offset = file->file_contents_index;
    new_field->field_length = field_length;
    new_field->color_index = color_index;
    new_field->background = background;
    new_field->navigation_label = navigation_label;
    new_field->additional_color_index = additional_color_index;

    file->file_fields = g_slist_prepend (file->file_fields, new_field);

    file->file_contents_index += field_length;
}

gboolean
format_utils_read (FormatsFile *file,
                   gpointer     buffer,
                   guint        count)
{
    if ((file->file_contents_index + count) <= file->file_size)
        memmove (buffer, file->file_contents + file->file_contents_index, count);
    else
        return FALSE;

    return TRUE;
}
