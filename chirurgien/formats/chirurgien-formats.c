/* chirurgien-formats.c
 *
 * Copyright (C) 2020 - Daniel Léonard Schardijn
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

#include <chirurgien-globals.h>
#include "chirurgien-formats.h"

#include "chirurgien-types.h"

#include "validator/chirurgien-validator.h"
#include "processor/chirurgien-processor.h"


void
chirurgien_formats_analyze (ProcessorFile *file)
{
    const FormatDefinition *format_definition;

    gboolean format_found;

    format_found = FALSE;

    for (GSList *format_iter = chirurgien_system_format_definitions;
         format_iter && !format_found;
         format_iter = format_iter->next)
    {
        format_definition = format_iter->data;

        if (!format_definition->disabled)
            format_found = format_identify (format_definition, file);
    }

    if (!format_found)
    {
        for (GList *format_iter = chirurgien_user_format_definitions;
             format_iter && !format_found;
             format_iter = format_iter->next)
        {
            format_definition = format_iter->data;

            format_found = format_identify (format_definition, file);
        }
    }

    if (!format_found)
        format_definition = NULL;

    format_process (format_definition, file);
}

void
chirurgien_formats_initialize (const gchar *format_definition_path)
{
    FormatDefinition *format_definition;

    GBytes *format_definition_bytes;
    const gchar *format_definition_text;
    gsize format_definition_size;

    format_definition_bytes = g_resources_lookup_data (format_definition_path,
                                                       G_RESOURCE_LOOKUP_FLAGS_NONE,
                                                       NULL);

    format_definition_text = g_bytes_get_data (format_definition_bytes,
                                               &format_definition_size);

    format_definition = format_validate (format_definition_text,
                                         format_definition_size,
                                         NULL);
    chirurgien_system_format_definitions = g_slist_append (chirurgien_system_format_definitions,
                                                           format_definition);

    g_bytes_unref (format_definition_bytes);
}

gchar *
chirurgien_formats_load (GFile *file)
{
    FormatDefinition *format_definition;

    g_autoptr (GFileInputStream) file_input;
    g_autoptr (GFileInfo) file_info;

    g_autofree gchar *format_definition_text;
    gsize format_definition_size;

    GError *error = NULL;
    gchar *error_message = NULL;

    file_input = g_file_read (file, NULL, NULL);
    file_info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                   G_FILE_QUERY_INFO_NONE, NULL, NULL);

    format_definition_size = g_file_info_get_size (file_info);
    format_definition_text = g_malloc (format_definition_size);

    g_input_stream_read_all (G_INPUT_STREAM (file_input),
                             format_definition_text,
                             format_definition_size,
                             NULL, NULL, NULL);

    format_definition = format_validate (format_definition_text,
                                         format_definition_size,
                                         &error);

    if (error)
    {
        error_message = g_strdup (error->message);
        g_error_free (error);
    }
    else
    {
        chirurgien_user_format_definitions = g_list_append (chirurgien_user_format_definitions,
                                                            format_definition);
    }

    return error_message;
}
