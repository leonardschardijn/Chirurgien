/* process-field-step.c
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

#include "processor.h"

#include <chirurgien-globals.h>


void
process_field_step (const FormatDefinition *format_definition,
                    ProcessorFile          *file,
                    RunStep                *run_step,
                    ProcessorState         *state)
{
    FieldDefinition *field_def;
    FieldDefinitionOption *option;
    FieldDefinitionFlag *flag;
    FormatColor *color, *additional_color;

    ProcessorVariable *processor_var;
    guint64 op_value;

    DescriptionTab *tab;

    gsize available_data, field_size;
    const guchar *file_contents;

    union
    {
        guint8  one;
        guint16 two;
        guint32 four;
        guint64 eight;
        guchar  value[8];
    } raw_field_value;
    gchar *field_value;

    GSList *option_i;
    GString *auto_tooltip, *string_obj;
    gchar *field_tag;

    gboolean index_saved, store_var;
    gsize save_index;

    /* Implicit limit on all fields: EOF */
    if (state->file_end_reached && !run_step->field.limit_failed)
        return;

    field_def = g_hash_table_lookup (format_definition->fields,
                                     run_step->field.field_id);
    /* The field doesn't exist, skip */
    if (!field_def)
        return;

    /* Clear 'available' size fields */
    if (field_def->size_type == AVAILABLE_SIZE || field_def->size_type == VALUE_SIZE)
        field_def->size = 0;

    /* The field has an explicit offset */
    if (run_step->field.offset)
    {
        processor_utils_read_value (state,
                                    run_step->field.offset,
                                    READ_VARIABLE | READ_NUMERIC,
                                    &processor_var,
                                    &op_value,
                                    FALSE);
        index_saved = TRUE;
        save_index = file->file_contents_index;

        file->file_contents_index = op_value;
    }
    else
    {
        index_saved = FALSE;
    }

    tab = NULL;
    available_data = FILE_AVAILABLE_DATA (file);

    if (!index_saved && (field_def->size > available_data))
    {
        state->file_end_reached = TRUE;
        return;
    }

    /* The field's size is determinted by a byte value */
    if (field_def->size_type == VALUE_SIZE)
    {
        for (file_contents = GET_CONTENT_POINTER (file), field_size = 0;
             field_size < available_data;
             file_contents++, field_size++)
        {
            if (*file_contents == field_def->value)
                break;
        }
        field_def->size = field_size;
    }

    /* The field has a limit */
    if (run_step->field.limit)
    {
        processor_utils_read_value (state,
                                    run_step->field.limit,
                                    READ_VARIABLE | READ_NUMERIC,
                                    &processor_var,
                                    &op_value,
                                    FALSE);
        if (processor_var)
        {
            if (processor_var->failed && !run_step->field.limit_failed)
                return;

            if (field_def->size_type == AVAILABLE_SIZE)
            {
                field_def->size = MIN (op_value, available_data);
            }
            else if (field_def->size_type == VALUE_SIZE)
            {
                field_def->size = MIN (op_value, field_def->size);
                field_def->size = MIN (available_data, field_def->size);
            }
            else if (op_value < field_def->size)
            {
                processor_var->failed = TRUE;
                return;
            }

            if (!field_def->mask && !field_def->shift)
            {
                switch (processor_var->size)
                {
                    case 1:
                    processor_var->one -= field_def->size;

                    break;
                    case 2:
                    processor_var->two -= field_def->size;

                    break;
                    case 3:
                    case 4:
                    processor_var->four -= field_def->size;

                    break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    processor_var->eight -= field_def->size;

                    break;
                }
            }
        }
        else
        {
            if (field_def->size_type == AVAILABLE_SIZE)
            {
                field_def->size = MIN (op_value, available_data);
            }
            else if (field_def->size_type == VALUE_SIZE)
            {
                field_def->size = MIN (op_value, field_def->size);
                field_def->size = MIN (available_data, field_def->size);
            }
        }
    }
    /* No limit, but the field has dynamic size */
    else if (field_def->size_type == AVAILABLE_SIZE)
    {
        field_def->size = available_data;
    }

    /* The field's value should be stored */
    if (run_step->field.store_var)
    {
        if (field_def->size)
        {
            processor_var = g_slice_new0 (ProcessorVariable);

            store_var = FALSE;

            /* The field is an ASCII-encoded number */
            if (run_step->field.ascii_base)
            {
                if (field_def->size)
                {
                    processor_var->size = 8;

                    field_value = g_strndup (GET_CONTENT_POINTER (file), field_def->size);
                    processor_var->eight = g_ascii_strtoull (field_value, NULL, run_step->field.ascii_base);
                    g_free (field_value);

                    store_var = TRUE;
                }
            }
            /* The field is a binary number or raw data */
            else if (field_def->size <= 8)
            {
                processor_var->size = field_def->size;

                if (processor_utils_read (format_definition,
                                          state,
                                          file,
                                          field_def,
                                          run_step->field.convert_endianness,
                                          processor_var->value))
                {
                    store_var = TRUE;
                }
            }

            if (store_var)
            {
                g_hash_table_insert (state->variables,
                                     run_step->field.store_var,
                                     processor_var);
            }
            else
            {
                g_slice_free (ProcessorVariable, processor_var);
            }
        }
    }

    /* The field uses a tab */
    if (run_step->field.tab)
    {
        tab = g_hash_table_lookup (state->tabs,
                                   run_step->field.tab);
        if (!tab)
        {
            tab = processor_utils_new_tab (run_step->field.section);
            g_hash_table_insert (state->tabs,
                                 run_step->field.tab,
                                 tab);
        }
        else if (run_step->field.section)
        {
            processor_utils_start_section_tab (tab, run_step->field.section);
        }
    }
    /* The field starts a description section */
    else if (run_step->field.section)
    {
        processor_utils_start_section (file, run_step->field.section);
    }

    /* The field should print a description line */
    if (field_def->print && !run_step->field.suppress_print)
    {
        /* Field value: string literal */
        if (field_def->print == PRINT_LITERAL)
        {
            if (tab)
                processor_utils_add_line_tab (tab,
                                              field_def->name,
                                              field_def->print_literal,
                                              field_def->tooltip,
                                              run_step->field.margin_top,
                                              run_step->field.margin_bottom);
            else
                processor_utils_add_line (file,
                                          field_def->name,
                                          field_def->print_literal,
                                          field_def->tooltip,
                                          run_step->field.margin_top,
                                          run_step->field.margin_bottom);
        }
        /* Field value: text */
        else if (field_def->print == PRINT_TEXT)
        {
            processor_utils_add_text_tab (tab,
                                          field_def->name,
                                          GET_CONTENT_POINTER (file),
                                          field_def->size,
                                          field_def->encoding);
        }
        /* Other value types are limited to 8 bytes */
        else if (field_def->size && field_def->size <= 8 &&
                 processor_utils_read (format_definition,
                                       state,
                                       file,
                                       field_def,
                                       run_step->field.convert_endianness,
                                       raw_field_value.value))
        {
            /* Field value: int/uint */
            if (field_def->print == PRINT_INT || field_def->print == PRINT_UINT)
            {
                field_value = NULL;

                switch (field_def->size)
                {
                    case 1:
                    if (field_def->print == PRINT_INT)
                        field_value = g_strdup_printf ("%hhd", raw_field_value.one);
                    else
                        field_value = g_strdup_printf ("%hhu", raw_field_value.one);

                    break;
                    case 2:
                    if (field_def->print == PRINT_INT)
                        field_value = g_strdup_printf ("%hd", raw_field_value.two);
                    else
                        field_value = g_strdup_printf ("%hu", raw_field_value.two);

                    break;
                    case 3:
                    case 4:
                    if (field_def->print == PRINT_INT)
                        field_value = g_strdup_printf ("%d", raw_field_value.four);
                    else
                        field_value = g_strdup_printf ("%u", raw_field_value.four);

                    break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    if (field_def->print == PRINT_INT)
                        field_value = g_strdup_printf ("%ld", raw_field_value.eight);
                    else
                        field_value = g_strdup_printf ("%lu", raw_field_value.eight);

                    break;
                }

                if (field_value)
                {
                    if (tab)
                        processor_utils_add_line_tab (tab,
                                                      field_def->name,
                                                      field_value,
                                                      field_def->tooltip,
                                                      run_step->field.margin_top,
                                                      run_step->field.margin_bottom);
                    else
                        processor_utils_add_line (file,
                                                  field_def->name,
                                                  field_value,
                                                  field_def->tooltip,
                                                  run_step->field.margin_top,
                                                  run_step->field.margin_bottom);
                    g_free (field_value);
                }
            }
            /* Field value: one of a set of options */
            else if (field_def->print == PRINT_OPTION)
            {
                /* The field has an automatically generated tooltip */
                if (field_def->auto_tooltip)
                {
                    auto_tooltip = g_string_new (field_def->tooltip ?
                                                 field_def->tooltip :
                                                 field_def->name);
                    auto_tooltip = g_string_append_c (auto_tooltip, '\n');

                    for (option_i = field_def->value_collection;
                         option_i;
                         option_i = option_i->next)
                    {
                        option = option_i->data;
                        field_value = option->value;

                        g_string_append (auto_tooltip, "<tt>");
                        for (gsize i = 0; i < field_def->size; i++)
                        {
                            g_string_append_c (auto_tooltip,
                                               hex_chars[((guchar) field_value[i]) >> 4]);
                            g_string_append_c (auto_tooltip,
                                               hex_chars[((guchar) field_value[i]) & 0x0F]);
                            g_string_append_c (auto_tooltip, ' ');
                        }
                        g_string_truncate (auto_tooltip, auto_tooltip->len - 1);
                        g_string_append (auto_tooltip, "<sub>16</sub></tt>\t");
                        g_string_append (auto_tooltip, option->name);
                        g_string_append_c (auto_tooltip, '\n');
                    }
                    g_string_truncate (auto_tooltip, auto_tooltip->len - 1);
                }
                else
                {
                    auto_tooltip = NULL;
                }

                /* Field option values are always big-endian
                 * Convert variable endianness fields to big-endian */
                if (field_def->convert_endianness)
                {
                    switch (field_def->size)
                    {
                        case 2:
                        raw_field_value.two = GUINT16_TO_BE (raw_field_value.two);

                        break;
                        case 3:
                        case 4:
                        raw_field_value.four = GUINT32_TO_BE (raw_field_value.four);

                        break;
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        raw_field_value.eight = GUINT64_TO_BE (raw_field_value.eight);

                        break;
                    }
                }

                /* Match one of the possible values */
                field_value = NULL;
                for (option_i = field_def->value_collection;
                     option_i;
                     option_i = option_i->next)
                {
                    option = option_i->data;

                    if (!memcmp (raw_field_value.value,
                                 option->value,
                                 field_def->size))
                    {
                        field_value = option->name;
                        break;
                    }
                }

                if (!field_value)
                    field_value = "<span foreground=\"red\">INVALID</span>";

                if (tab)
                    processor_utils_add_line_tab (tab,
                                                  field_def->name,
                                                  field_value,
                                                  auto_tooltip ? auto_tooltip->str : field_def->tooltip,
                                                  run_step->field.margin_top,
                                                  run_step->field.margin_bottom);
                else
                    processor_utils_add_line (file,
                                              field_def->name,
                                              field_value,
                                              auto_tooltip ? auto_tooltip->str : field_def->tooltip,
                                              run_step->field.margin_top,
                                              run_step->field.margin_bottom);
                if (auto_tooltip)
                    g_string_free (auto_tooltip, TRUE);
            }
            /* Field value: a set of flags */
            else if (field_def->print == PRINT_FLAGS)
            {
                /* The field has an automatically generated tooltip */
                if (field_def->auto_tooltip)
                {
                    auto_tooltip = g_string_new (field_def->tooltip ?
                                                 field_def->tooltip :
                                                 field_def->name);
                    auto_tooltip = g_string_append_c (auto_tooltip, '\n');

                    for (option_i = field_def->value_collection;
                         option_i;
                         option_i = option_i->next)
                    {
                        flag = option_i->data;

                        g_string_append_printf (auto_tooltip,
                                                "%s (<tt>%lX<sub>16</sub></tt>): %s\n",
                                                flag->name,
                                                flag->mask,
                                                flag->meaning);
                    }
                    g_string_truncate (auto_tooltip, auto_tooltip->len - 1);
                }
                else
                {
                    auto_tooltip = NULL;
                }

                switch (field_def->size)
                {
                    case 1:
                    op_value = raw_field_value.one;

                    break;
                    case 2:
                    op_value = raw_field_value.two;

                    break;
                    case 3:
                    case 4:
                    op_value = raw_field_value.four;

                    break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    op_value = raw_field_value.eight;

                    break;
                }

                /* Look for the set flags */
                string_obj = g_string_new (NULL);
                for (option_i = field_def->value_collection;
                     option_i;
                     option_i = option_i->next)
                {
                    flag = option_i->data;

                    if (op_value & flag->mask)
                        g_string_append_printf (string_obj, "%s\n", flag->name);
                }
                if (string_obj->len)
                    g_string_truncate (string_obj, string_obj->len - 1);

                if (tab)
                    processor_utils_add_line_tab (tab,
                                                  field_def->name,
                                                  string_obj->str,
                                                  auto_tooltip ? auto_tooltip->str : field_def->tooltip,
                                                  run_step->field.margin_top,
                                                  run_step->field.margin_bottom);
                else
                    processor_utils_add_line (file,
                                              field_def->name,
                                              string_obj->str,
                                              auto_tooltip ? auto_tooltip->str : field_def->tooltip,
                                              run_step->field.margin_top,
                                              run_step->field.margin_bottom);

                g_string_free (string_obj, TRUE);

                if (auto_tooltip)
                    g_string_free (auto_tooltip, TRUE);
            }
        }
    }

    /* Get the field's color */
    color = field_def->color ? g_hash_table_lookup (format_definition->colors,
                                                    field_def->color) :
                               NULL;
    /* Get the field's additional color */
    additional_color = run_step->field.additional_color ?
                       g_hash_table_lookup (format_definition->colors,
                                            run_step->field.additional_color) :
                       NULL;

    string_obj = g_string_new (run_step->field.navigation);
    /* Get the navigation tag */
    if (run_step->field.navigation)
    {
        processor_utils_read_value (state,
                                    run_step->field.navigation,
                                    READ_VARIABLE,
                                    &processor_var,
                                    &op_value,
                                    FALSE);
        if (processor_var && file->file_size > op_value)
        {
            if (strnlen (file->file_contents + op_value, file->file_size - op_value) > 15)
            {
                g_string_append_len (g_string_truncate (string_obj, 0),
                                     file->file_contents + op_value, 10);
                g_string_append (string_obj, " [...]");
            }
            else
            {
                g_string_append (g_string_truncate (string_obj, 0),
                                 file->file_contents + op_value);
            }

            if (run_step->field.navigation_limit && string_obj->len > run_step->field.navigation_limit)
                g_string_truncate (string_obj, run_step->field.navigation_limit);
            for (guint i = 0; i < string_obj->len; i++)
            {
                if (!g_ascii_isprint (string_obj->str[i]))
                {
                    g_string_truncate (string_obj, i);
                    break;
                }
            }

            if (!string_obj->len && field_def->name)
                g_string_append (string_obj, field_def->name);
        }
    }

    /* Masked or shifted fields do not emit file fields */
    if (!field_def->mask && !field_def->shift && color && field_def->size)
    {
        /* Get the field tag */
        if (field_def->tag && !g_strcmp0 (field_def->tag, "navigation"))
            field_tag = string_obj->len ? string_obj->str : field_def->name;
        else if (field_def->tag)
            field_tag = field_def->tag;
        else
            field_tag = field_def->name;

        /* Add field */
        processor_utils_add_field (file,
                                   color ? color->color_index : G_MAXUINT,
                                   color ? color->background : TRUE,
                                   field_def->size,
                                   field_tag,
                                   string_obj->len ? string_obj->str : NULL,
                                   additional_color ? additional_color->color_index : G_MAXUINT);
    }
    g_string_free (string_obj, TRUE);

    /* Insert tab */
    if (tab && run_step->field.insert_tab)
    {
        processor_utils_insert_tab (file,
                                    tab,
                                    run_step->field.tab);
        g_hash_table_remove (state->tabs,
                             run_step->field.tab);
    }

    if (index_saved)
        file->file_contents_index = save_index;
}
