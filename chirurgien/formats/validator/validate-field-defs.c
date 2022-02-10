/* validate-field-defs.c
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


static GError *
attribute_already_set_error (GMarkupParseContext *context,
                             const gchar         *element_name)
{
    gint line, character;

    g_markup_parse_context_get_position (context, &line, &character);
    return g_error_new (G_MARKUP_ERROR,
                        G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                        "Error on line %d char %d: <field-def> attribute already set: %s",
                        line, character, element_name);
}

static void
field_defs_start (GMarkupParseContext *context,
                  const gchar         *element_name,
                  const gchar        **attribute_names,
                  const gchar        **attribute_values,
                  gpointer             user_data,
                  GError             **error)
{
    ParserControl *parser_control = user_data;

    FieldDefinition *field_def;
    FieldDefinitionOption *option;
    FieldDefinitionFlag *flag;

    gchar *attr1, *attr2, *attr3, *attr4,
          *attr5, *attr6, *attr7, *attr8,
          *attr9, *attr10, *attr11;
    gboolean attr12;

    gpointer value_address;

    gsize option_value_size;
    gpointer option_value;

    gint line, character;

    if (parser_control->depth == 2 &&
        !g_strcmp0 (element_name, "field-def"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "id", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "name", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "tag", &attr3,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "color", &attr4,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "tooltip", &attr5,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "mask", &attr6,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "shift", &attr7,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "print", &attr8,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "size", &attr9,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "value", &attr10,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "encoding", &attr11,
            G_MARKUP_COLLECT_INVALID))
        {
            field_def = g_slice_new0 (FieldDefinition);
            field_def->name = attr2;
            field_def->tag = attr3;
            field_def->color = attr4;
            field_def->tooltip = attr5;

            if (attr6)
                field_def->mask = g_ascii_strtoull (attr6, NULL, 16);

            if (attr7)
                field_def->shift = g_ascii_strtoull (attr7, NULL, 10);

            if (attr8)
            {
                if (!g_strcmp0 (attr8, "int"))
                {
                    field_def->print = PRINT_INT;
                }
                else if (!g_strcmp0 (attr8, "uint"))
                {
                    field_def->print = PRINT_UINT;
                }
                else if (!g_strcmp0 (attr8, "text"))
                {
                    field_def->print = PRINT_TEXT;
                }
                else if (!g_strcmp0 (attr8, "option"))
                {
                    field_def->print = PRINT_OPTION;
                }
                else if (!g_strcmp0 (attr8, "flags"))
                {
                    field_def->print = PRINT_FLAGS;
                }
                else
                {
                    field_def->print = PRINT_LITERAL;
                    field_def->print_literal = g_strdup (attr8);
                }
            }

            if (attr9)
            {
                if (!g_strcmp0 (attr9, "available"))
                {
                    field_def->size_type = AVAILABLE_SIZE;
                }
                else
                {
                    field_def->size = g_ascii_strtoull (attr9, NULL, 10);
                }
            }

            if (attr10)
            {
                if (strlen (attr10) != 2)
                {
                    g_markup_parse_context_get_position (context, &line, &character);
                    *error = g_error_new (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_INVALID_CONTENT,
                                          "Error on line %d char %d: Field definition values can only be one byte long",
                                          line, character);
                    field_def_destroy (field_def);
                    return;
                }

                field_def->size_type = VALUE_SIZE;

                value_address = &field_def->value;
                validator_utils_hex_to_binary (attr10,
                                               &value_address,
                                               NULL,
                                               FALSE);
            }

            if (attr11)
            {
                if (!g_strcmp0 (attr11, "UTF-16LE"))
                {
                    field_def->encoding = ENCODING_UTF16LE;
                }
                else if (!g_strcmp0 (attr11, "UTF-32LE"))
                {
                    field_def->encoding = ENCODING_UTF32LE;
                }
                else if (!g_strcmp0 (attr11, "UTF-16BE"))
                {
                    field_def->encoding = ENCODING_UTF16BE;
                }
                else if (!g_strcmp0 (attr11, "UTF-32BE"))
                {
                    field_def->encoding = ENCODING_UTF32BE;
                }
                else if (!g_strcmp0 (attr11, "ISO-8859-1"))
                {
                    field_def->encoding = ENCODING_ISO_8859_1;
                }
                else
                {
                    g_markup_parse_context_get_position (context, &line, &character);
                    *error = g_error_new (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_INVALID_CONTENT,
                                          "Error on line %d char %d: Invalid value for encoding attribute: %s",
                                          line, character, attr11);
                    field_def_destroy (field_def);
                    return;
                }
            }

            g_hash_table_insert (parser_control->definition->fields,
                                 attr1, field_def);
            parser_control->current_field = field_def;
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "options"))
    {
        if (parser_control->current_field->print != PRINT_OPTION)
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_INVALID_CONTENT,
                                  "Error on line %d char %d: <field-def> definitions that do not print options cannot define options",
                                  line, character);
            return;
        }

        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "convert-endianness", &attr12,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->current_field->convert_endianness = attr12;
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }

        parser_control->field_has_options = TRUE;
    }
    else if (parser_control->current_field &&
             parser_control->depth == 4 &&
             parser_control->field_has_options &&
             !g_strcmp0 (element_name, "option"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "name", &attr1,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "char-value", &attr2,
            G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "hex-value", &attr3,
            G_MARKUP_COLLECT_INVALID))
        {
            option = g_slice_new0 (FieldDefinitionOption);

            option->name = attr1;

            if (attr2)
            {
                option_value_size = strlen (attr2);

                if (option_value_size != parser_control->current_field->size)
                {
                    g_markup_parse_context_get_position (context, &line, &character);
                    *error = g_error_new (G_MARKUP_ERROR,
                                          G_MARKUP_ERROR_INVALID_CONTENT,
                                          "Error on line %d char %d: Field <option> values must match the field's size",
                                          line, character);
                    return;
                }

                option->value = attr2;
            }
            else if (attr3)
            {
                if (validator_utils_validate_hex_value (context,
                                                        "hex-value",
                                                        attr3,
                                                        error))
                {
                    option_value_size = strlen (attr3) >> 1;

                    if (option_value_size != parser_control->current_field->size)
                    {
                        g_markup_parse_context_get_position (context, &line, &character);
                        *error = g_error_new (G_MARKUP_ERROR,
                                              G_MARKUP_ERROR_INVALID_CONTENT,
                                              "Error on line %d char %d: Field <option> values must match the field's size",
                                              line, character);
                        return;
                    }

                    validator_utils_hex_to_binary (attr3,
                                                   &option_value,
                                                   NULL,
                                                   TRUE);

                    option->value = option_value;

                    parser_control->current_field->value_collection =
                        g_slist_append (parser_control->current_field->value_collection,
                                        option);
                }
            }
            else
            {
                g_markup_parse_context_get_position (context, &line, &character);
                *error = g_error_new (G_MARKUP_ERROR,
                                      G_MARKUP_ERROR_INVALID_CONTENT,
                                      "Error on line %d char %d: Field <option> doesn't define a value",
                                      line, character);
                return;
            }
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "flags"))
    {
        if (parser_control->current_field->print != PRINT_FLAGS)
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_INVALID_CONTENT,
                                  "Error on line %d char %d: <field-def> definitions that do not print flags cannot define flags",
                                  line, character);
            return;
        }

        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "convert-endianness", &attr12,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->current_field->convert_endianness = attr12;
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }

        parser_control->field_has_options = TRUE;
    }
    else if (parser_control->current_field &&
             parser_control->depth == 4 &&
             parser_control->field_has_options &&
             !g_strcmp0 (element_name, "flag"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_STRDUP, "name", &attr1,
            G_MARKUP_COLLECT_STRING, "mask", &attr2,
            G_MARKUP_COLLECT_STRDUP | G_MARKUP_COLLECT_OPTIONAL, "meaning", &attr3,
            G_MARKUP_COLLECT_INVALID))
        {
            flag = g_slice_new0 (FieldDefinitionFlag);

            flag->name = attr1;
            flag->mask = g_ascii_strtoull (attr2, NULL, 16);
            flag->meaning = attr3;

            parser_control->current_field->value_collection =
                g_slist_append (parser_control->current_field->value_collection,
                                flag);
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "tooltip"))
    {
        if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error,
            G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "auto", &attr12,
            G_MARKUP_COLLECT_INVALID))
        {
            parser_control->current_field->auto_tooltip = attr12;
        }
        else
        {
            validator_utils_prefix_attr_error (context, error);
        }
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "name"))
    {
        if (parser_control->current_field->name)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "tag"))
    {
        if (parser_control->current_field->tag)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "color"))
    {
        if (parser_control->current_field->color)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "mask"))
    {
        if (parser_control->current_field->mask)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "print"))
    {
        if (parser_control->current_field->print)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "size"))
    {
        if (parser_control->current_field->size)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "value"))
    {
        if (parser_control->current_field->value)
            *error = attribute_already_set_error (context, element_name);
    }
    else if (parser_control->current_field &&
             parser_control->depth == 3 &&
             !g_strcmp0 (element_name, "encoding"))
    {
        if (parser_control->current_field->encoding)
            *error = attribute_already_set_error (context, element_name);
    }
    else
    {
        g_markup_parse_context_get_position (context, &line, &character);
        *error = g_error_new (G_MARKUP_ERROR,
                              G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                              "Error on line %d char %d: Unexpected element in <field-defs> section: %s",
                              line, character, element_name);
    }

    parser_control->depth++;
}

static void
field_defs_end (GMarkupParseContext *context,
                const gchar         *element_name,
                gpointer             user_data,
                GError             **error)
{
    ParserControl *parser_control = user_data;

    gint line, character;

    parser_control->depth--;

    if (parser_control->current_field &&
        parser_control->depth == 2 &&
        !g_strcmp0 (element_name, "field-def"))
    {
        /* Final validations */
        if (parser_control->current_field->mask &&
            parser_control->current_field->size_type != FIXED_SIZE)
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                                  "Error on line %d char %d: Masks can only be used on fixed size <field-def> definitions",
                                  line, character);
        }

        parser_control->current_field = NULL;
        parser_control->field_has_options = FALSE;
    }
}

static void
field_defs_text (GMarkupParseContext *context,
                 const gchar         *text,
                 gsize                text_len,
                 gpointer             user_data,
                 GError             **error)
{
    ParserControl *parser_control = user_data;

    const gchar *element_name;
    gpointer value_address;

    g_autofree gchar *text_string = NULL;

    gint line, character;

    if (parser_control->depth != 4 || !parser_control->current_field || !text_len)
        return;

    element_name = g_markup_parse_context_get_element (context);

    if (!parser_control->current_field->name &&
        !g_strcmp0 (element_name, "name"))
    {
        parser_control->current_field->name = g_strstrip (g_strndup (text, text_len));
    }
    else if (!parser_control->current_field->tag &&
             !g_strcmp0 (element_name, "tag"))
    {
        parser_control->current_field->tag = g_strstrip (g_strndup (text, text_len));
    }
    else if (!parser_control->current_field->color &&
             !g_strcmp0 (element_name, "color"))
    {
        parser_control->current_field->color = g_strstrip (g_strndup (text, text_len));
    }
    else if (!parser_control->current_field->print &&
             !g_strcmp0 (element_name, "print"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        if (!g_strcmp0 (text_string, "int"))
            parser_control->current_field->print = PRINT_INT;
        else if (!g_strcmp0 (text_string, "uint"))
            parser_control->current_field->print = PRINT_UINT;
        else if (!g_strcmp0 (text_string, "text"))
            parser_control->current_field->print = PRINT_TEXT;
        else if (!g_strcmp0 (text_string, "option"))
            parser_control->current_field->print = PRINT_OPTION;
        else if (!g_strcmp0 (text_string, "flags"))
            parser_control->current_field->print = PRINT_FLAGS;
        else
        {
            parser_control->current_field->print = PRINT_LITERAL;
            parser_control->current_field->print_literal = g_steal_pointer (&text_string);
        }
    }
    else if (!parser_control->current_field->tooltip &&
             !g_strcmp0 (element_name, "tooltip"))
    {
        parser_control->current_field->tooltip = g_strstrip (g_strndup (text, text_len));
    }
    else if (!parser_control->current_field->size &&
             !g_strcmp0 (element_name, "mask"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        parser_control->current_field->mask = g_ascii_strtoull (text_string, NULL, 16);
    }
    else if (!parser_control->current_field->size &&
             !g_strcmp0 (element_name, "shift"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        parser_control->current_field->shift = g_ascii_strtoull (text_string, NULL, 10);
    }
    else if (!parser_control->current_field->size &&
             !g_strcmp0 (element_name, "size"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        if (!g_strcmp0 (text_string, "available"))
        {
            parser_control->current_field->size_type = AVAILABLE_SIZE;
        }
        else
        {
            parser_control->current_field->size = g_ascii_strtoull (text_string, NULL, 10);
            parser_control->current_field->size_type = FIXED_SIZE;
        }
    }
    else if (!parser_control->current_field->value &&
             !g_strcmp0 (element_name, "value"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        if (strlen (text_string) == 2)
        {
            parser_control->current_field->size_type = VALUE_SIZE;

            value_address = &parser_control->current_field->value;
            validator_utils_hex_to_binary (text_string,
                                           &value_address,
                                           NULL,
                                           FALSE);
        }
        else
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_INVALID_CONTENT,
                                  "Error on line %d char %d: Field definition values can only be one byte long",
                                  line, character);
        }
    }
    else if (!parser_control->current_field->encoding &&
             !g_strcmp0 (element_name, "encoding"))
    {
        text_string = g_strstrip (g_strndup (text, text_len));

        if (!g_strcmp0 (text_string, "ISO-8859-1"))
        {
            parser_control->current_field->encoding = ENCODING_ISO_8859_1;
        }
        else
        {
            g_markup_parse_context_get_position (context, &line, &character);
            *error = g_error_new (G_MARKUP_ERROR,
                                  G_MARKUP_ERROR_INVALID_CONTENT,
                                  "Error on line %d char %d: Invalid value for encoding attribute: %s",
                                  line, character, text_string);
        }
    }
}

GMarkupParser field_defs_parser =
{
    field_defs_start,
    field_defs_end,
    field_defs_text,
    NULL,
    NULL
};
