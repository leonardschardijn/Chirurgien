/* validator.c
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

#include "chirurgien-validator.h"

#include "validator.h"


FormatDefinition *
format_validate (const gchar *format_definition_text,
                 gsize        format_definition_size,
                 GError     **return_error)
{
    FormatDefinition *format_definition;

    GMarkupParseContext *parse_context;
    ParserControl parser_control = { 0 };

    GError *error = NULL;

    format_definition = format_definition_create ();

    parser_control.definition = format_definition;

    parse_context = g_markup_parse_context_new (&root_parser,
                                                G_MARKUP_IGNORE_QUALIFIED |
                                                G_MARKUP_TREAT_CDATA_AS_TEXT,
                                                &parser_control,
                                                NULL);

    g_markup_parse_context_parse (parse_context,
                                  format_definition_text,
                                  format_definition_size,
                                  &error);

    if (!error)
        g_markup_parse_context_end_parse (parse_context,
                                          &error);

    if (error)
    {
        format_definition_destroy (g_steal_pointer (&format_definition));
        if (return_error)
            *return_error = error;
        else
            g_error_free (error);
    }

    /* Final clean-up */
    g_slist_free_full (parser_control.magic_steps, magic_step_destroy);
    g_slist_free_full (parser_control.run_steps, run_step_destroy);
    g_free (parser_control.current_block_id);

    g_markup_parse_context_free (parse_context);

    return format_definition;
}
