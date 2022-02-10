/* validate-colors.c
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

#include <chirurgien-globals.h>


static void
colors_start (GMarkupParseContext *context,
              const gchar         *element_name,
              const gchar        **attribute_names,
              const gchar        **attribute_values,
              gpointer             user_data,
              GError             **error)
{
    ParserControl *parser_control = user_data;
    FormatColor *color;

    gchar *id, *name, *color_index;
    gboolean background;

    gint line, character;

    if (parser_control->field_closure_needed)
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: <color> definitions cannot contain other elements",
                              line, character);
        return;
    }

    if (parser_control->depth == 2 &&
        !g_strcmp0 (element_name, "color"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "id", &id,
            G_MARKUP_COLLECT_STRDUP, "name", &name,
            G_MARKUP_COLLECT_STRING, "index", &color_index,
            G_MARKUP_COLLECT_BOOLEAN, "background", &background,
            G_MARKUP_COLLECT_INVALID))
        {
            color = g_slice_new0 (FormatColor);

            color->color_name = name;

            if (strlen (name) > 50)
            {
                g_markup_parse_context_get_position (context, &line, &character);
                *error = g_error_new (G_MARKUP_ERROR,
                                      G_MARKUP_ERROR_INVALID_CONTENT,
                                      "Error on line %d char %d: Attribute 'name' of element 'color' is limited to 50 characters: %s",
                                      line, character, name);
                format_color_destroy (color);
                return;
            }

            color->color_index = (guint) g_ascii_strtoull (color_index, NULL, 10);
            if (color->color_index >= CHIRURGIEN_TOTAL_COLORS)
            {
                g_markup_parse_context_get_position (context, &line, &character);
                *error = g_error_new (G_MARKUP_ERROR,
                                      G_MARKUP_ERROR_INVALID_CONTENT,
                                      "Error on line %d char %d: Invalid color index: %d",
                                      line, character, color->color_index);
                format_color_destroy (color);
                return;
            }

            color->background = background;

            parser_control->field_closure_needed = TRUE;
            parser_control->closure_depth = parser_control->depth;
            g_hash_table_insert (parser_control->definition->colors,
                                 id, color);
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
                              "Error on line %d char %d: Unexpected element in <colors> section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
colors_end (G_GNUC_UNUSED GMarkupParseContext *context,
            const gchar *element_name,
            gpointer     user_data,
            G_GNUC_UNUSED GError             **error)
{
    ParserControl *parser_control = user_data;

    parser_control->depth--;

    if (parser_control->field_closure_needed &&
        parser_control->closure_depth == parser_control->depth &&
        !g_strcmp0 (element_name, "color"))
    {
        parser_control->field_closure_needed = FALSE;
    }
}

GMarkupParser colors_parser =
{
    colors_start,
    colors_end,
    NULL,
    NULL,
    NULL
};
