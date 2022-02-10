/* validator-utils.c
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

#include "validator-utils.h"


gboolean
validator_utils_validate_hex_value (GMarkupParseContext *context,
                                    const gchar         *attribute,
                                    const gchar         *hex_string,
                                    GError             **error)
{
    gsize hex_string_length;
    gboolean valid;

    gint line, character;

    hex_string_length = strlen (hex_string);
    valid = TRUE;

    g_markup_parse_context_get_position (context, &line, &character);

    if (!hex_string_length)
    {
       *error = g_error_new (G_MARKUP_ERROR,
                             G_MARKUP_ERROR_INVALID_CONTENT,
                             "Error on line %d char %d: Attribute '%s' empty",
                             line, character, attribute);
        valid = FALSE;
    }
    else if (hex_string_length % 2)
    {
       *error = g_error_new (G_MARKUP_ERROR,
                             G_MARKUP_ERROR_INVALID_CONTENT,
                             "Error on line %d char %d: Attribute '%s' needs to define a pair number of hexadecimal characrters",
                             line, character, attribute);
        valid = FALSE;
    }

    return valid;
}

void
validator_utils_hex_to_binary (const gchar *hex_string,
                               gpointer    *buffer,
                               gsize       *buffer_size,
                               gboolean     allocate_buffer)
{
    gint hex_digit_value1, hex_digit_value2;

    gsize hex_string_length;

    gsize result_buffer_size;
    guchar *result;

    hex_string_length = strlen (hex_string);

    result_buffer_size = hex_string_length >> 1;
    result = g_malloc0 (result_buffer_size);

    if (buffer_size)
        *buffer_size = result_buffer_size;

    for (guint i = 0, j = 0; i < hex_string_length; i++, j++)
    {
        hex_digit_value1 = g_ascii_xdigit_value (hex_string[i++]);
        hex_digit_value2 = g_ascii_xdigit_value (hex_string[i]);

        if (hex_digit_value1 == -1 ||
            hex_digit_value2 == -1)
        {
            break;
        }

        result[j] = hex_digit_value1 << 4 | hex_digit_value2;
    }

    if (allocate_buffer && buffer)
        *buffer = result;
    else if (buffer)
        memcpy (*buffer, result, result_buffer_size);
    else
        g_free (result);
}

void
validator_utils_prefix_attr_error (GMarkupParseContext *context,
                                   GError             **error)
{
    gint line, character;

    g_markup_parse_context_get_position (context, &line, &character);
    g_prefix_error (error, "Error on line %d char %d: ", line, character);
}

static void
run_steps_destroy (gpointer data)
{
    GSList *run_steps;

    run_steps = data;

    g_slist_free_full (run_steps, run_step_destroy);
}

FormatDefinition *
format_definition_create (void)
{
    FormatDefinition *format_definition;

    format_definition = g_slice_new0 (FormatDefinition);
    format_definition->colors = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                       g_free, format_color_destroy);
    format_definition->blocks = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                       g_free, run_steps_destroy);
    format_definition->fields = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                       g_free, field_def_destroy);

    return format_definition;
}

void
format_definition_destroy (FormatDefinition *format_definition)
{
    g_free (format_definition->format_name);
    g_free (format_definition->short_format_name);
    g_free (format_definition->details);
    g_free (format_definition->endianness_var);
    g_free (format_definition->be_value);
    g_free (format_definition->le_value);

    for (GSList *magic = format_definition->magic;
         magic;
         magic = magic->next)
    {
        g_slist_free_full (magic->data, magic_step_destroy);
    }
    g_slist_free (format_definition->magic);
    g_hash_table_destroy (format_definition->colors);
    g_slist_free_full (format_definition->run, run_step_destroy);
    g_hash_table_destroy (format_definition->blocks);
    g_hash_table_destroy (format_definition->fields);

    g_slice_free (FormatDefinition, format_definition);
}

void
format_color_destroy (gpointer data)
{
    FormatColor *format_color;

    format_color = data;

    if (format_color)
    {
        g_free (format_color->color_name);

        g_slice_free (FormatColor, format_color);
    }
}

void
magic_step_destroy (gpointer data)
{
    MagicStep *magic_step;

    magic_step = data;

    if (magic_step)
    {
        if (magic_step->step_type == MATCH_STEP)
        {
            g_free (magic_step->match.value);
            g_free (magic_step->match.offset);
        }
        else if (magic_step->step_type == READ_STEP)
        {
            g_free (magic_step->read.var_id);
            g_free (magic_step->read.offset);
        }

        g_slice_free (MagicStep, magic_step);
    }
}

void
run_step_destroy (gpointer data)
{
    RunStep *run_step;

    run_step = data;

    if (run_step)
    {
        if (run_step->step_type == FIELD_STEP)
        {
            g_free (run_step->field.field_id);
            g_free (run_step->field.store_var);
            g_free (run_step->field.navigation);
            g_free (run_step->field.offset);
            g_free (run_step->field.additional_color);
            g_free (run_step->field.tab);
            g_free (run_step->field.section);
            g_free (run_step->field.limit);
        }
        else if (run_step->step_type == MATCH_START_STEP)
        {
            g_free (run_step->match.var_id);
            g_free (run_step->match.value);
        }
        else if (run_step->step_type == LOOP_START_STEP)
        {
            g_free (run_step->loop.until_set);
            g_free (run_step->loop.var_value);
            g_free (run_step->loop.limit);
        }
        else if (run_step->step_type == PRINT_STEP)
        {
            g_free (run_step->print.line);
            g_free (run_step->print.tooltip);
            g_free (run_step->print.var_id);
            g_free (run_step->print.section);
            g_free (run_step->print.tab);
        }
        else if (run_step->step_type == EXEC_STEP)
        {
            g_free (run_step->exec.var_id);
            g_free (run_step->exec.set);
            g_free (run_step->exec.add);
            g_free (run_step->exec.substract);
            g_free (run_step->exec.multiply);
            g_free (run_step->exec.divide);
        }

        g_slice_free (RunStep, run_step);
    }
}

static void
field_def_option_destroy (gpointer data)
{
    FieldDefinitionOption *option;

    option = data;

    if (option)
    {
        g_free (option->name);
        g_free (option->value);

        g_slice_free (FieldDefinitionOption, option);
    }
}

static void
field_def_flag_destroy (gpointer data)
{
    FieldDefinitionFlag *flag;

    flag = data;

    if (flag)
    {
        g_free (flag->name);
        g_free (flag->meaning);

        g_slice_free (FieldDefinitionFlag, flag);
    }
}

void
field_def_destroy (gpointer data)
{
    FieldDefinition *field_def;

    field_def = data;

    if (field_def)
    {
        g_free (field_def->name);
        g_free (field_def->tag);
        g_free (field_def->tooltip);
        g_free (field_def->color);
        g_free (field_def->print_literal);

        if (field_def->print == PRINT_OPTION)
            g_slist_free_full (field_def->value_collection, field_def_option_destroy);
        else if (field_def->print == PRINT_FLAGS)
            g_slist_free_full (field_def->value_collection, field_def_flag_destroy);

        g_slice_free (FieldDefinition, field_def);
    }
}
