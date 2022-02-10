/* validate-magic.c
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
magic_start (GMarkupParseContext *context,
             const gchar         *element_name,
             const gchar        **attribute_names,
             const gchar        **attribute_values,
             gpointer             user_data,
             GError             **error)
{
    ParserControl *parser_control = user_data;

    MagicStep *step;

    gchar *attr1, *attr2, *attr3;

    gint line, character;

    if (parser_control->field_closure_needed)
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: Steps in a <signature> cannot contain other elements",
                              line, character);
        return;
    }

    if (parser_control->depth == 3 &&
        !g_strcmp0 (element_name, "match"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "char-value", &attr1,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "hex-value", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "offset", &attr3,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (MagicStep);
            step->step_type = MATCH_STEP;

            if (attr1)
            {
                step->match.value = attr1;
                step->match.value_size = strlen (attr1);
            }
            else if (attr2)
            {
                if (validator_utils_validate_hex_value (context,
                                                        "hex-value",
                                                        attr2,
                                                        error))
                {
                    validator_utils_hex_to_binary (attr2,
                                                   &step->match.value,
                                                   &step->match.value_size,
                                                   TRUE);
                }
                else
                {
                    magic_step_destroy (step);
                    return;
                }
            }

            step->match.offset = attr3;

            parser_control->field_closure_needed = TRUE;
            parser_control->magic_steps =
                g_slist_append (parser_control->magic_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "read"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "var-id", &attr1,
            G_MARKUP_COLLECT_STRING, "size", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "offset", &attr3,
            G_MARKUP_COLLECT_INVALID))
        {
            step = g_slice_new0 (MagicStep);
            step->step_type = READ_STEP;

            step->read.var_id = attr1;
            step->read.size = g_ascii_strtoull (attr2, NULL, 10);
            step->read.offset = attr3;

            parser_control->field_closure_needed = TRUE;
            parser_control->magic_steps =
                g_slist_append (parser_control->magic_steps, step);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->depth != 2 || g_strcmp0 (element_name, "signature"))
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: Unexpected element in <magic> section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
magic_end (G_GNUC_UNUSED GMarkupParseContext *context,
           const gchar *element_name,
           gpointer     user_data,
           G_GNUC_UNUSED GError             **error)
{
    ParserControl *parser_control = user_data;

    parser_control->depth--;

    if (!g_strcmp0 (element_name, "signature"))
    {
        parser_control->definition->magic =
            g_slist_append (parser_control->definition->magic,
                            parser_control->magic_steps);
        parser_control->magic_steps = NULL;
    }
    else if (parser_control->field_closure_needed &&
             !g_strcmp0 (element_name, "match"))
    {
        parser_control->field_closure_needed = FALSE;
    }
    else if (parser_control->field_closure_needed &&
             !g_strcmp0 (element_name, "read"))
    {
        parser_control->field_closure_needed = FALSE;
    }
}

GMarkupParser magic_parser =
{
    magic_start,
    magic_end,
    NULL,
    NULL,
    NULL
};
