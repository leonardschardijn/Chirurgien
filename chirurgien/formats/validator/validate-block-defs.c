/* validate-block-defs.c
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
run_blocks_start (GMarkupParseContext *context,
                  const gchar         *element_name,
                  const gchar        **attribute_names,
                  const gchar        **attribute_values,
                  gpointer             user_data,
                  GError             **error)
{
    ParserControl *parser_control = user_data;

    gchar *id;

    gint line, character;

    if (parser_control->depth == 2 &&
        !g_strcmp0 (element_name, "block-def"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "id", &id,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->current_block_id = id;

            g_markup_parse_context_push (context,
                                         &run_parser,
                                         parser_control);
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
                              "Error on line %d char %d: Unexpected element in <block-defs> section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
run_blocks_end (GMarkupParseContext *context,
                const gchar         *element_name,
                gpointer             user_data,
                G_GNUC_UNUSED GError **error)
{
    ParserControl *parser_control = user_data;

    parser_control->depth--;

    if (!g_strcmp0 (element_name, "block-def") &&
        parser_control->current_block_id)
    {
        g_hash_table_insert (parser_control->definition->blocks,
                             parser_control->current_block_id,
                             parser_control->run_steps);
        g_markup_parse_context_pop (context);

        parser_control->run_steps = NULL;
        parser_control->current_block_id = NULL;
    }
}

GMarkupParser run_blocks_parser =
{
    run_blocks_start,
    run_blocks_end,
    NULL,
    NULL,
    NULL
};
