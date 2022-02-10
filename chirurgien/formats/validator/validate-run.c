/* validate-run.c
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

#include "validator.h"


static void
run_start (GMarkupParseContext *context,
           const gchar         *element_name,
           const gchar        **attribute_names,
           const gchar        **attribute_values,
           gpointer             user_data,
           GError             **error)
{
    ParserControl *parser_control = user_data;

    RunStep *step;

    gchar *attr1, *attr2, *attr3, *attr4,
          *attr5, *attr6, *attr7, *attr8,
          *attr9, *attr10, *attr11, *attr12;
    gboolean attr13, attr14, attr15, attr16;

    gint line, character;

    if (parser_control->field_closure_needed)
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: <field> steps cannot contain other elements",
                              line, character);
        return;
    }
    if (parser_control->print_closure_needed)
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: <print> steps cannot contain other elements",
                              line, character);
        return;
    }
    if (parser_control->exec_closure_needed)
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: <exec> steps cannot contain other elements",
                              line, character);
        return;
    }

    if (!g_strcmp0 (element_name, "field"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "id", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "store-var", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "navigation", &attr3,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "offset", &attr4,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "additional-color", &attr5,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "tab", &attr6,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "section", &attr7,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "limit", &attr8,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "ascii-base", &attr9,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "navigation-limit", &attr10,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "margin-top", &attr11,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "margin-bottom", &attr12,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "convert-endianness", &attr13,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "limit-failed", &attr14,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "insert-tab", &attr15,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "suppress-print", &attr16,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = FIELD_STEP;

            step->field.field_id = attr1;
            step->field.store_var = attr2;
            step->field.navigation = attr3;
            step->field.offset = attr4;
            step->field.additional_color = attr5;
            step->field.tab = attr6;
            step->field.section = attr7;
            step->field.limit = attr8;

            if (attr9)
            {
                step->field.ascii_base = g_ascii_strtoull (attr9, NULL, 10);
                if (step->field.ascii_base < 2 || step->field.ascii_base > 36)
                {
                    g_markup_parse_context_get_position (context, &line, &character);
                    *error = g_error_new (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_INVALID_CONTENT,
                                          "Error on line %d char %d: Invalid value for ascii-base attribute: %s",
                                          line, character, attr9);
                    run_step_destroy (step);
                    return;
                }
            }

            if (attr10)
                step->field.navigation_limit = g_ascii_strtoull (attr10, NULL, 10);
            if (attr11)
                step->field.margin_top = g_ascii_strtoull (attr11, NULL, 10);
            if (attr12)
                step->field.margin_bottom = g_ascii_strtoull (attr12, NULL, 10);

            step->field.convert_endianness = attr13;
            step->field.limit_failed = attr14;
            step->field.insert_tab = attr15;
            step->field.suppress_print = attr16;

            parser_control->field_closure_needed = TRUE;
            parser_control->closure_depth = parser_control->depth;
            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (!g_strcmp0 (element_name, "match"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "var-id", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "char-value", &attr2,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "hex-value", &attr3,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "num-value", &attr4,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "op", &attr5,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "convert-endianness", &attr13,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = MATCH_START_STEP;

            step->match.var_id = attr1;

            if (attr2)
            {
                step->match.value = attr2;
                step->match.value_size = strlen (attr2);
            }
            else if (attr3)
            {
                if (validator_utils_validate_hex_value (context,
                                                        "hex-value",
                                                        attr3,
                                                        error))
                {
                    validator_utils_hex_to_binary (attr3,
                                                   &step->match.value,
                                                   &step->match.value_size,
                                                   TRUE);
                }
                else
                {
                    run_step_destroy (step);
                    return;
                }
            }

            if (attr4)
                step->match.num_value = g_ascii_strtoull (attr4, NULL, 10);

            if (attr5)
            {
                if (!g_strcmp0 (attr5, "def"))
                {
                    step->match.op = OP_DEFINED;
                }
                else if (!g_strcmp0 (attr5, "eq"))
                {
                    step->match.op = OP_EQUAL;
                }
                else if (!g_strcmp0 (attr5, "gt"))
                {
                    step->match.op = OP_GREATER;
                }
                else if (!g_strcmp0 (attr5, "bit"))
                {
                    step->match.op = OP_BIT;
                }
                else
                {
                    g_markup_parse_context_get_position (context, &line, &character);
                    *error = g_error_new (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_INVALID_CONTENT,
                                          "Error on line %d char %d: Invalid value for op attribute: %s",
                                          line, character, attr2);
                    run_step_destroy (step);
                    return;
                }
            }

            step->match.convert_endianness = attr13;

            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (!g_strcmp0 (element_name, "loop"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "until-set", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "limit", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "var-value", &attr3,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "num-value", &attr4,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = LOOP_START_STEP;

            step->loop.until_set = attr1;
            step->loop.limit = attr2;
            step->loop.var_value = attr3;

            if (attr4)
            {
                step->loop.num_value = g_ascii_strtoull (attr4, NULL, 10);
                step->loop.num_value_used = TRUE;
            }

            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (!g_strcmp0 (element_name, "selection"))
    {
        step = g_slice_new0 (RunStep);
        step->step_type = SELECTION_START_STEP;

        parser_control->run_steps =
            g_slist_append (parser_control->run_steps, step);
    }
    else if (!g_strcmp0 (element_name, "print"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "line", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "var-id", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "tooltip", &attr3,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "margin-top", &attr4,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "margin-bottom", &attr5,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "section", &attr6,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "tab", &attr7,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "signed", &attr13,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "insert-tab", &attr14,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "omit-undefined", &attr15,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "no-section", &attr16,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = PRINT_STEP;

            step->print.line = attr1;
            step->print.var_id = attr2;
            step->print.tooltip = attr3;

            if (attr4)
                step->print.margin_top = g_ascii_strtoull (attr4, NULL, 10);
            if (attr5)
                step->print.margin_bottom = g_ascii_strtoull (attr5, NULL, 10);

            step->print.section = attr6;
            step->print.tab = attr7;
            step->print.signed_val = attr13;
            step->print.insert_tab = attr14;
            step->print.omit_undefined = attr15;
            step->print.no_section = attr16;

            parser_control->print_closure_needed = TRUE;
            parser_control->closure_depth = parser_control->depth;
            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (!g_strcmp0 (element_name, "exec"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "var-id", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "set", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "modulo", &attr3,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "add", &attr4,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "subtract", &attr5,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "multiply", &attr6,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "divide", &attr7,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "signed", &attr13,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = EXEC_STEP;

            step->exec.var_id = attr1;
            step->exec.set = attr2;
            step->exec.modulo = attr3;
            step->exec.add = attr4;
            step->exec.substract = attr5;
            step->exec.multiply = attr6;
            step->exec.divide = attr7;
            step->exec.signed_op = attr13;

            parser_control->exec_closure_needed = TRUE;
            parser_control->closure_depth = parser_control->depth;
            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (!g_strcmp0 (element_name, "block"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "id", &attr1,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (RunStep);
            step->step_type = BLOCK_STEP;

            step->block.block_id = attr1;

            parser_control->block_closure_needed = TRUE;
            parser_control->closure_depth = parser_control->depth;
            parser_control->run_steps =
                g_slist_append (parser_control->run_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: Unexpected element in run section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
run_end (G_GNUC_UNUSED GMarkupParseContext *context,
         const gchar *element_name,
         gpointer     user_data,
         G_GNUC_UNUSED GError             **error)
{
    ParserControl *parser_control = user_data;

    RunStep *step;

    parser_control->depth--;

    if (parser_control->field_closure_needed &&
        parser_control->closure_depth == parser_control->depth &&
        !g_strcmp0 (element_name, "field"))
    {
        parser_control->field_closure_needed = FALSE;
    }
    else if (!g_strcmp0 (element_name, "match"))
    {
        step = g_slice_new0 (RunStep);
        step->step_type = MATCH_END_STEP;

        parser_control->run_steps =
            g_slist_append (parser_control->run_steps, step);
    }
    else if (!g_strcmp0 (element_name, "loop"))
    {
        step = g_slice_new0 (RunStep);
        step->step_type = LOOP_END_STEP;

        parser_control->run_steps =
            g_slist_append (parser_control->run_steps, step);
    }
    else if (!g_strcmp0 (element_name, "selection"))
    {
        step = g_slice_new0 (RunStep);
        step->step_type = SELECTION_END_STEP;

        parser_control->run_steps =
            g_slist_append (parser_control->run_steps, step);
    }
    else if (parser_control->print_closure_needed &&
             parser_control->closure_depth == parser_control->depth &&
             !g_strcmp0 (element_name, "print"))
    {
        parser_control->print_closure_needed = FALSE;
    }
    else if (parser_control->exec_closure_needed &&
             parser_control->closure_depth == parser_control->depth &&
             !g_strcmp0 (element_name, "exec"))
    {
        parser_control->exec_closure_needed = FALSE;
    }
    else if (parser_control->block_closure_needed &&
             parser_control->closure_depth == parser_control->depth &&
             !g_strcmp0 (element_name, "block"))
    {
        parser_control->block_closure_needed = FALSE;
    }
}

GMarkupParser run_parser =
{
    run_start,
    run_end,
    NULL,
    NULL,
    NULL
};
