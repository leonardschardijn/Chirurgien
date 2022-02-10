/* processor-utils.c
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

#include "processor-utils.h"


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
    gtk_label_set_xalign (GTK_LABEL (*left_label), 1.0f);
    gtk_widget_set_halign (*left_label, GTK_ALIGN_END);
    gtk_widget_set_margin_top (*left_label, margin_top);
    gtk_widget_set_margin_bottom (*left_label, margin_bottom);
    gtk_widget_add_css_class (*left_label, "dim-label");

    if (field_tooltip)
        gtk_widget_set_tooltip_markup (GTK_WIDGET (*left_label), field_tooltip);

    *right_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (*right_label), field_value);
    gtk_label_set_wrap (GTK_LABEL (*right_label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (*right_label), 0.0f);
    gtk_widget_set_halign (*right_label, GTK_ALIGN_START);
    gtk_widget_set_margin_top (*right_label, margin_top);
    gtk_widget_set_margin_bottom (*right_label, margin_bottom);
}

void
processor_utils_set_title (ProcessorFile *file,
                           const char    *title)
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
processor_utils_start_section (ProcessorFile *file,
                               const gchar   *section_name)
{
    GtkWidget *expander, *grid;

    create_section (&expander, &grid, section_name);

    gtk_box_append (file->overview, expander);

    file->section = GTK_GRID (grid);
    file->description_lines_count = 0;
}

void
processor_utils_add_line (ProcessorFile *file,
                          const gchar   *field_name,
                          const gchar   *field_value,
                          const gchar   *field_tooltip,
                          gint           margin_top,
                          gint           margin_bottom)
{
    GtkWidget *left_label, *right_label;

    if (!field_name)
        return;

    if (!file->section)
        processor_utils_start_section (file, "[UNNAMED SECTION]");

    create_line_labels (&left_label, &right_label,
                        field_name, field_value, field_tooltip,
                        margin_top, margin_bottom);

    gtk_grid_attach (GTK_GRID (file->section), left_label, 0, file->description_lines_count++, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (file->section), right_label, left_label, GTK_POS_RIGHT, 1, 1);
}

DescriptionTab *
processor_utils_new_tab (const gchar *section_name)
{
    DescriptionTab *tab;

    tab = g_slice_new0 (DescriptionTab);

    tab->contents = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 10));
    gtk_widget_set_can_focus (GTK_WIDGET (tab->contents), FALSE);
    gtk_widget_set_margin_start (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_end (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_top (GTK_WIDGET (tab->contents), 10);
    gtk_widget_set_margin_bottom (GTK_WIDGET (tab->contents), 10);

    if (section_name)
        processor_utils_start_section_tab (tab, section_name);

    tab->used = FALSE;

    return tab;
}

void
processor_utils_start_section_tab (DescriptionTab *tab,
                                   const gchar    *section_name)
{
    GtkWidget *expander, *grid;

    create_section (&expander, &grid, section_name);

    gtk_box_append (tab->contents, expander);

    tab->section = GTK_GRID (grid);
    tab->description_lines_count = 0;
}

void
processor_utils_add_line_tab (DescriptionTab *tab,
                              const gchar    *field_name,
                              const gchar    *field_value,
                              const gchar    *field_tooltip,
                              gint            margin_top,
                              gint            margin_bottom)
{
    GtkWidget *left_label, *right_label;

    if (!field_name)
        return;

    if (!tab->section)
        processor_utils_start_section_tab (tab, "[UNNAMED SECTION]");

    create_line_labels (&left_label, &right_label,
                        field_name, field_value, field_tooltip,
                        margin_top, margin_bottom);

    gtk_grid_attach (GTK_GRID (tab->section), left_label, 0, tab->description_lines_count++, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (tab->section), right_label, left_label, GTK_POS_RIGHT, 1, 1);

    tab->used = TRUE;
}

void
processor_utils_add_note_tab (DescriptionTab *tab,
                              const gchar    *line)
{
    GtkWidget *label;

    if (!line)
        return;

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), line);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_add_css_class (label, "dim-label");

    gtk_box_append (tab->contents, label);

    tab->used = TRUE;
}

static void
validate_insert_text (GtkTextBuffer *buffer,
                      const gchar   *text,
                      gsize          text_size)
{
    GtkTextIter start;

    text_size = strnlen (text, text_size);

    if (g_utf8_validate_len (text, text_size, NULL))
    {
        gtk_text_buffer_set_text (buffer, text, text_size);
    }
    else
    {
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_insert_markup (buffer, &start,
                     "<span foreground=\"red\">[INVALID ENCODING]</span>", -1);
    }
}

void
processor_utils_add_text_tab (DescriptionTab *tab,
                              const gchar    *field_name,
                              const gchar    *text,
                              gsize           text_size,
                              TextEncoding    encoding)
{
    GtkWidget *textview, *expander;
    GtkTextBuffer *buffer;

    gsize print_text_size;
    gchar *converted_text, *truncated_message;

    gsize utf8_size;

    if (!tab || !text_size)
        return;

    textview = gtk_text_view_new ();
    gtk_widget_set_margin_start (textview, 10);
    gtk_widget_set_margin_end (textview, 10);
    gtk_widget_set_margin_bottom (textview, 10);
    gtk_widget_set_margin_top (textview, 10);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

    switch (encoding)
    {
        case ENCODING_UTF8:
        utf8_size = text_size;
        converted_text = NULL;

        break;
        case ENCODING_UTF16LE:
        converted_text = g_convert (text, text_size, "UTF-8", "UTF-16LE",
                                    NULL, &utf8_size, NULL);

        break;
        case ENCODING_UTF32LE:
        converted_text = g_convert (text, text_size, "UTF-8", "UTF-32LE",
                                    NULL, &utf8_size, NULL);

        break;
        case ENCODING_UTF16BE:
        converted_text = g_convert (text, text_size, "UTF-8", "UTF-16BE",
                                    NULL, &utf8_size, NULL);

        break;
        case ENCODING_UTF32BE:
        converted_text = g_convert (text, text_size, "UTF-8", "UTF-32BE",
                                    NULL, &utf8_size, NULL);

        break;
        case ENCODING_ISO_8859_1:
        converted_text = g_convert (text, text_size, "UTF-8", "ISO-8859-1",
                                    NULL, &utf8_size, NULL);

        break;
    }

    print_text_size = utf8_size > 4096 ? 4096 : utf8_size;
    validate_insert_text (buffer,
                          converted_text ? converted_text : text,
                          print_text_size);
    g_free (converted_text);

    expander = gtk_expander_new (field_name);
    gtk_expander_set_child (GTK_EXPANDER (expander), textview);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);

    gtk_box_append (GTK_BOX (tab->contents), expander);

    if (utf8_size > 4096)
    {
        truncated_message = g_strdup_printf ("%s was truncated from its original size of %lu",
                                             field_name, text_size);
        processor_utils_add_note_tab (tab, truncated_message);
        g_free (truncated_message);
    }

    tab->used = TRUE;
}

void
processor_utils_insert_tab (ProcessorFile  *file,
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
processor_utils_add_field (ProcessorFile *file,
                           guint          color_index,
                           gboolean       background,
                           gsize          field_size,
                           const gchar   *field_name,
                           const gchar   *navigation_label,
                           guint          additional_color_index)
{
    FileField *new_field;
    gsize available_data;

    available_data = file->file_size - file->file_contents_index;

    if (!field_size ||
        !field_name ||
        field_size > available_data ||
        file->file_size <= file->file_contents_index)
    {
        return;
    }

    new_field = g_slice_new (FileField);

    new_field->field_name = g_strdup (field_name);
    new_field->field_offset = file->file_contents_index;
    new_field->field_size = field_size;
    new_field->color_index = color_index;
    new_field->background = background;
    new_field->navigation_label = g_strdup (navigation_label);
    new_field->additional_color_index = additional_color_index;

    file->file_fields = g_slist_prepend (file->file_fields, new_field);

    file->file_contents_index += field_size;
}

static gboolean
get_format_endianness (const FormatDefinition *format_definition,
                       const ProcessorState   *state)
{
    ProcessorVariable *endianness;

    if (format_definition->endianness == BIG_ENDIAN_FORMAT)
    {
        return FALSE;
    }
    else if (format_definition->endianness == LITTLE_ENDIAN_FORMAT)
    {
        return TRUE;
    }
    else if (format_definition->endianness == VARIABLE_ENDIANNESS &&
             format_definition->endianness_var)
    {
        endianness = g_hash_table_lookup (state->variables,
                                          format_definition->endianness_var);
        if (endianness)
        {
            if (!memcmp (endianness->value,
                         format_definition->be_value,
                         format_definition->be_value_size))
            {
                return FALSE;
            }
            else if (!memcmp (endianness->value,
                              format_definition->le_value,
                              format_definition->le_value_size))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

gboolean
processor_utils_read (const FormatDefinition *format_definition,
                      const ProcessorState   *state,
                      const ProcessorFile    *file,
                      const FieldDefinition  *field_def,
                      gboolean                convert_endianness,
                      gpointer                buffer)
{
    gboolean little_endian;

    if (!field_def->size)
        return TRUE;

    if (FILE_HAS_DATA_N (file, field_def->size))
        memcpy (buffer, GET_CONTENT_POINTER (file), field_def->size);
    else
        return FALSE;

    if (convert_endianness ||
        field_def->convert_endianness ||
        field_def->print == PRINT_INT ||
        field_def->print == PRINT_UINT)
    {
        little_endian = get_format_endianness (format_definition, state);

        switch (field_def->size)
        {
            case 2:
            if (little_endian)
                *(guint16 *) buffer = GUINT16_FROM_LE (*(guint16 *) buffer);
            else
                *(guint16 *) buffer = GUINT16_FROM_BE (*(guint16 *) buffer);

            break;
            case 3:
            case 4:
            if (little_endian)
                *(guint32 *) buffer = GUINT32_FROM_LE (*(guint32 *) buffer);
            else
                *(guint32 *) buffer = GUINT32_FROM_BE (*(guint32 *) buffer);

            break;
            case 5:
            case 6:
            case 7:
            case 8:
            if (little_endian)
                *(guint64 *) buffer = GUINT64_FROM_LE (*(guint64 *) buffer);
            else
                *(guint64 *) buffer = GUINT64_FROM_BE (*(guint64 *) buffer);

            break;
        }
    }

    if (field_def->shift || field_def->mask)
    {
        switch (field_def->size)
        {
            case 1:
            if (field_def->shift)
                *(guint8 *) buffer = (*(guint8 *) buffer) >> field_def->shift;
            if (field_def->mask)
                *(guint8 *) buffer = (*(guint8 *) buffer) & field_def->mask;

            break;
            case 2:
            if (field_def->shift)
                *(guint16 *) buffer = (*(guint16 *) buffer) >> field_def->shift;
            if (field_def->mask)
                *(guint16 *) buffer = (*(guint16 *) buffer) & field_def->mask;

            break;
            case 3:
            case 4:
            if (field_def->shift)
                *(guint32 *) buffer = (*(guint32 *) buffer) >> field_def->shift;
            if (field_def->mask)
                *(guint32 *) buffer = (*(guint32 *) buffer) & field_def->mask;

            break;
            case 5:
            case 6:
            case 7:
            case 8:
            if (field_def->shift)
                *(guint64 *) buffer = (*(guint64 *) buffer) >> field_def->shift;
            if (field_def->mask)
                *(guint64 *) buffer = (*(guint64 *) buffer) & field_def->mask;

            break;
        }
    }

    return TRUE;
}

void
processor_utils_format_byte_order (const FormatDefinition *format_definition,
                                   const ProcessorState   *state,
                                   gpointer                value,
                                   gsize                   value_size)
{
    gboolean little_endian;

    little_endian = get_format_endianness (format_definition, state);

    switch (value_size)
    {
        case 2:
        if (little_endian)
            *(guint16 *) value = GUINT16_SWAP_LE_BE (*(guint16 *) value);

        break;
        case 3:
        case 4:
        if (little_endian)
            *(guint32 *) value = GUINT32_SWAP_LE_BE (*(guint32 *) value);

        break;
        case 5:
        case 6:
        case 7:
        case 8:
        if (little_endian)
            *(guint64 *) value = GUINT64_SWAP_LE_BE (*(guint64 *) value);

        break;
    }
}

void
processor_utils_read_value (const ProcessorState *state,
                            const gchar          *value,
                            ReadValueType         read_type,
                            ProcessorVariable   **return_var,
                            gpointer              return_value,
                            gboolean              signed_value)
{
    ProcessorVariable *processor_var;

    if (return_var)
        *return_var = NULL;
    if (return_value)
        *(guint64 *) return_value = 0;

    if (value)
    {
        /* Look for the processor variable */
        if (read_type & READ_VARIABLE)
        {
            processor_var = g_hash_table_lookup (state->variables,
                                                 value);
            if (processor_var)
            {
                switch (processor_var->size)
                {
                    case 1:
                    if (signed_value)
                    {
                        if (processor_var->rational_value)
                            *(gint64 *) return_value = (gint8) processor_var->rational;
                        else
                            *(gint64 *) return_value = processor_var->sone;
                    }
                    else
                    {
                        if (processor_var->rational_value)
                            *(guint64 *) return_value = (guint8) processor_var->rational;
                        else
                            *(guint64 *) return_value = processor_var->one;
                    }

                    break;
                    case 2:
                    if (signed_value)
                    {
                        if (processor_var->rational_value)
                            *(gint64 *) return_value = (gint16) processor_var->rational;
                        else
                            *(gint64 *) return_value = processor_var->stwo;
                    }
                    else
                    {
                        if (processor_var->rational_value)
                            *(guint64 *) return_value = (guint16) processor_var->rational;
                        else
                            *(guint64 *) return_value = processor_var->two;
                    }

                    break;
                    case 3:
                    case 4:
                    if (signed_value)
                    {
                        if (processor_var->rational_value)
                            *(gint64 *) return_value = (gint32) processor_var->rational;
                        else
                            *(gint64 *) return_value = processor_var->sfour;
                    }
                    else
                    {
                        if (processor_var->rational_value)
                            *(guint64 *) return_value = (guint32) processor_var->rational;
                        else
                            *(guint64 *) return_value = processor_var->four;
                    }

                    break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    if (signed_value)
                    {
                        if (processor_var->rational_value)
                            *(gint64 *) return_value = (gint64) processor_var->rational;
                        else
                            *(gint64 *) return_value = processor_var->seight;
                    }
                    else
                    {
                        if (processor_var->rational_value)
                            *(guint64 *) return_value = (guint64) processor_var->rational;
                        else
                            *(guint64 *) return_value = processor_var->eight;
                    }

                    break;
                }

                if (return_var)
                    *return_var = processor_var;

                return;
            }
        }

        /* Read value as a decimal number */
        if ((read_type & READ_NUMERIC) && return_value)
        {
             *(guint64 *) return_value = g_ascii_strtoull (value, NULL, 10);
        }
    }
}

GSList *
processor_utils_skip_steps (GSList     *run_iter,
                            RunStepType skip_to_nesting,
                            RunStepType skip_to)
{
    const RunStep *run_step;
    guint step_nesting;

    for (step_nesting = 1; run_iter; run_iter = run_iter->next)
    {
        run_step = run_iter->data;

        if (run_step->step_type == skip_to_nesting)
            step_nesting++;
        else if (run_step->step_type == skip_to)
            step_nesting--;

        if (!step_nesting)
            break;
    }

    return run_iter;
}

static gint
sort_file_fields (gconstpointer a,
                  gconstpointer b)
{
    const FileField *field_a, *field_b;

    field_a = a;
    field_b = b;

    return field_a->field_offset - field_b->field_offset;
}

void
processor_utils_sort_find_unused (const FormatDefinition *format_definition,
                                  ProcessorFile          *file)
{
    const FieldDefinition *field_def;
    const FormatColor *format_color;

    FileField *file_field, *unused_data;
    GSList *new_fields;
    gsize tagged_up_to, field_end;

    new_fields = NULL;
    tagged_up_to = 0;

    file->file_fields = g_slist_sort (file->file_fields, sort_file_fields);

    field_def = g_hash_table_lookup (format_definition->fields,
                                     "unused-data");
    if (!field_def || (!field_def->tag  && !field_def->name) || !field_def->color)
        return;

    format_color = g_hash_table_lookup (format_definition->colors,
                                        field_def->color);
    if (!format_color)
        return;

    for (GSList *i = file->file_fields; i; i = i->next)
    {
        file_field = i->data;
        if (tagged_up_to < file_field->field_offset)
        {
            unused_data = g_slice_new (FileField);

            unused_data->field_name = g_strdup (field_def->tag ? field_def->tag : field_def->name);
            unused_data->field_offset = tagged_up_to;
            unused_data->field_size = file_field->field_offset - tagged_up_to;
            unused_data->color_index = format_color->color_index;
            unused_data->background = format_color->background;
            unused_data->navigation_label = NULL;
            unused_data->additional_color_index = -1;

            new_fields = g_slist_prepend (new_fields, unused_data);
        }

        field_end = file_field->field_offset + file_field->field_size;

        if (tagged_up_to < field_end)
            tagged_up_to = field_end;
    }

    if (tagged_up_to < file->file_size)
    {
        unused_data = g_slice_new (FileField);

        unused_data->field_name = g_strdup (field_def->tag ? field_def->tag : field_def->name);
        unused_data->field_offset = tagged_up_to;
        unused_data->field_size = file->file_size - tagged_up_to;
        unused_data->color_index = format_color->color_index;
        unused_data->background = format_color->background;
        unused_data->navigation_label = NULL;
        unused_data->additional_color_index = -1;

        new_fields = g_slist_prepend (new_fields, unused_data);
    }

    if (new_fields)
        file->file_fields = g_slist_sort (g_slist_concat (file->file_fields, new_fields), sort_file_fields);
}

void
selection_scope_destroy (gpointer data)
{
    g_slice_free (SelectionScope, data);
}

void
processor_variable_destroy (gpointer data)
{
    g_slice_free (ProcessorVariable, data);
}

void
description_tab_destroy (gpointer data)
{
    g_slice_free (DescriptionTab, data);
}
