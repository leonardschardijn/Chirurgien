/* validate-endianness.c
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
endianness_start (GMarkupParseContext *context,
                  const gchar         *element_name,
                  const gchar        **attribute_names,
                  const gchar        **attribute_values,
                  gpointer             user_data,
                  GError             **error)
{
    ParserControl *parser_control = user_data;

    gchar *hex_value;

    gint line, character;

    if (parser_control->depth == 2 &&
        !g_strcmp0 (element_name, "big-endian"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "hex-value", &hex_value,
            G_MARKUP_COLLECT_INVALID))
        {
            if (hex_value)
            {
                parser_control->definition->endianness = VARIABLE_ENDIANNESS;
                if (validator_utils_validate_hex_value (context,
                                                        "hex-value",
                                                        hex_value,
                                                        error))
                {
                    validator_utils_hex_to_binary (hex_value,
                                                   &parser_control->definition->be_value,
                                                   &parser_control->definition->be_value_size,
                                                   TRUE);
                }
            }
            else
            {
                parser_control->definition->endianness = BIG_ENDIAN_FORMAT;
            }
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->depth == 2 &&
             !g_strcmp0 (element_name, "little-endian"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "hex-value", &hex_value,
            G_MARKUP_COLLECT_INVALID))
        {
            if (hex_value)
            {
                parser_control->definition->endianness = VARIABLE_ENDIANNESS;
                if (validator_utils_validate_hex_value (context,
                                                        "hex-value",
                                                        hex_value,
                                                        error))
                {
                    validator_utils_hex_to_binary (hex_value,
                                                   &parser_control->definition->le_value,
                                                   &parser_control->definition->le_value_size,
                                                   TRUE);
                }
            }
            else
            {
                parser_control->definition->endianness = LITTLE_ENDIAN_FORMAT;
            }
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
                              "Error on line %d char %d: Unexpected element in <endianness> section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
endianness_end (G_GNUC_UNUSED GMarkupParseContext *context,
                G_GNUC_UNUSED const gchar         *element_name,
                gpointer user_data,
                G_GNUC_UNUSED GError             **error)
{
    ParserControl *parser_control = user_data;

    parser_control->depth--;
}

GMarkupParser endianness_parser =
{
    endianness_start,
    endianness_end,
    NULL,
    NULL,
    NULL
};
