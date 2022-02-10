/* validate-root.c
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
root_start (GMarkupParseContext *context,
            const gchar         *element_name,
            const gchar        **attribute_names,
            const gchar        **attribute_values,
            gpointer             user_data,
            GError             **error)
{
    ParserControl *parser_control = user_data;

    gchar *attr1, *attr2;

    gint line, character;

    if (!parser_control->format_root_found &&
        !parser_control->depth &&
        !g_strcmp0 (element_name, "format"))
    {
        parser_control->format_root_found = TRUE;

        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "name", &attr1,
            G_MARKUP_COLLECT_STRDUP, "short-name", &attr2,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->definition->format_name = attr1;
            parser_control->definition->short_format_name = attr2;

            if (strlen (attr1) > 50)
            {
                g_markup_parse_context_get_position (context, &line, &character);
                *error = g_error_new (G_MARKUP_ERROR,
                                      G_MARKUP_ERROR_INVALID_CONTENT,
                                      "Error on line %d char %d: Attribute 'name' of element 'format' is limited to 50 characters: %s",
                                      line, character, attr1);
            }
            else if (strlen (attr2) > 20)
            {
                g_markup_parse_context_get_position (context, &line, &character);
                *error = g_error_new (G_MARKUP_ERROR,
                                      G_MARKUP_ERROR_INVALID_CONTENT,
                                      "Error on line %d char %d: Attribute 'short-name' of element 'format' is limited to 20 characters: %s",
                                      line, character, attr2);
            }
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->format_root_found &&
             parser_control->endianness_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "endianness"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "var-id", &attr1,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->definition->endianness_var = attr1;
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }

        parser_control->endianness_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &endianness_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->magic_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "magic"))
    {
        parser_control->magic_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &magic_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->colors_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "colors"))
    {
        parser_control->colors_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &colors_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->run_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "run"))
    {
        parser_control->run_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &run_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->block_defs_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "block-defs"))
    {
        parser_control->block_defs_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &run_blocks_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->field_defs_state == PARSER_READY &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "field-defs"))
    {
        parser_control->field_defs_state = PARSER_EXECUTING;
        g_markup_parse_context_push (context,
                                     &field_defs_parser,
                                     parser_control);
    }
    else if (parser_control->format_root_found &&
             parser_control->depth == 1 &&
             !g_strcmp0 (element_name, "details"))
    {
        if (parser_control->definition->details)
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_INVALID_CONTENT,
                                  "Error on line %d char %d: Duplicated element: %s",
                                  line, character, element_name);
        }
    }
    else
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: Unexpected element: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
root_end (GMarkupParseContext *context,
          const gchar         *element_name,
          gpointer             user_data,
          GError             **error)
{
    ParserControl *parser_control = user_data;

    parser_control->depth--;

    if (!g_strcmp0 (element_name, "endianness") &&
        parser_control->endianness_state == PARSER_EXECUTING)
    {
        parser_control->endianness_state = PARSER_DONE;
        g_markup_parse_context_pop (context);
    }
    else if (!g_strcmp0 (element_name, "magic") &&
             parser_control->magic_state == PARSER_EXECUTING)
    {
        parser_control->magic_state = PARSER_DONE;
        g_markup_parse_context_pop (context);
    }
    else if (!g_strcmp0 (element_name, "colors") &&
             parser_control->colors_state == PARSER_EXECUTING)
    {
        parser_control->colors_state = PARSER_DONE;
        g_markup_parse_context_pop (context);
    }
    else if (!g_strcmp0 (element_name, "run") &&
             parser_control->run_state == PARSER_EXECUTING)
    {
        parser_control->run_state = PARSER_DONE;
        g_markup_parse_context_pop (context);

        parser_control->definition->run = parser_control->run_steps;
        parser_control->run_steps = NULL;
    }
    else if (!g_strcmp0 (element_name, "block-defs") &&
             parser_control->block_defs_state == PARSER_EXECUTING)
    {
        parser_control->block_defs_state = PARSER_DONE;
        g_markup_parse_context_pop (context);
    }
    else if (!g_strcmp0 (element_name, "field-defs") &&
             parser_control->field_defs_state == PARSER_EXECUTING)
    {
        parser_control->field_defs_state = PARSER_DONE;
        g_markup_parse_context_pop (context);
    }
    else if (!g_strcmp0 (element_name, "format") &&
             !parser_control->depth)
    {
        if (parser_control->magic_state != PARSER_DONE)
        {
            *error = g_error_new_literal (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                                          "Error: <magic> section undefined");
        }
        else if (parser_control->colors_state != PARSER_DONE)
        {
            *error = g_error_new_literal (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                                          "Error: <colors> section undefined");
        }
        else if (parser_control->run_state != PARSER_DONE)
        {
            *error = g_error_new_literal (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                                          "Error: <run> section undefined");
        }
        else if (parser_control->field_defs_state != PARSER_DONE)
        {
            *error = g_error_new_literal (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                                          "Error: <field-defs> section undefined");
        }
    }
}

static void
root_text (GMarkupParseContext *context,
           const gchar         *text,
           gsize                text_len,
           gpointer             user_data,
           G_GNUC_UNUSED GError **error)
{
    ParserControl *parser_control = user_data;

    const gchar *element_name;
    gchar *details_text, *new_details_text;

    if (parser_control->depth != 2)
        return;

    element_name = g_markup_parse_context_get_element (context);

    if (parser_control->format_root_found &&
        text_len &&
        !g_strcmp0 (element_name, "details"))
    {
        new_details_text = g_strstrip (g_strndup (text, text_len));

        if (!parser_control->definition->details)
        {
            parser_control->definition->details = new_details_text;
        }
        else
        {
            details_text = parser_control->definition->details;
            parser_control->definition->details = g_strconcat (details_text,
                                                               new_details_text,
                                                               NULL);
            g_free (details_text);
            g_free (new_details_text);
        }
    }
}

GMarkupParser root_parser =
{
    root_start,
    root_end,
    root_text,
    NULL,
    NULL
};
