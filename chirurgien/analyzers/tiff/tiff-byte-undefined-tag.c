/* tiff-byte-tag.c
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

#include <config.h>

#include <glib/gi18n.h>

#include "exif-gpsinfo-analyzer.h"


static guint32
read_byte_undefined_values (AnalyzerFile *file,
                            guint32 offset,
                            gchar *tag_name,
                            guint32 count,
                            gboolean is_little_endian,
                            GSList **tagged_bytes)
{
    gsize save_pointer;
    guint32 read_bytes = 0;

    save_pointer = GET_POINTER (file);

    if (!is_little_endian)
        offset = g_ntohl (offset);

    SET_POINTER (file, offset);

    if (FILE_HAS_DATA_N (file, count))
    {
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, tag_name);
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file) + count));

        SET_POINTER (file, offset);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, tag_name);

        read_bytes = count;
    }

    SET_POINTER (file, save_pointer);

    return read_bytes;
}

static void
process_byte_undefined_tag_fields (AnalyzerFile *file,
                                   guint16 field_type,
                                   guint16 expected_field_type,
                                   guint32 count,
                                   guint32 expected_count)
{
    if (field_type == expected_field_type)
    {
        if (field_type == BYTE)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: BYTE"));
        else if (field_type == UNDEFINED)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: UNDEFINED"));
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
    }

    if (count != expected_count)
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    if (expected_count > 4)
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));
    else
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));
}

/*
 * This function handles both BYTE and UNDEFINED tags
 */
void
process_byte_undefined_tag (AnalyzerFile *file,
                            AnalyzerTab *tab,
                            gchar *tag_name_tag,
                            gchar *tag_name,
                            gchar *tag_tooltip,
                            guint16 field_type,
                            guint16 expected_field_type,
                            guint32 count,
                            guint32 expected_count,
                            guint32 value_offset,
                            gboolean is_little_endian,
                            guint possible_values,
                            guint8 *field_values,
                            gchar **value_descriptions,
                            GSList **tagged_bytes)
{
    gchar *description_message = NULL;
    guint read_values = 0;

    if (!expected_count)
        expected_count = count;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    process_byte_undefined_tag_fields (file, field_type, expected_field_type, count, expected_count);

    if (possible_values)
    {
        for (guint i = 0; i < possible_values; i++)
        {
            if (!memcmp (&value_offset, &field_values[i * expected_count], expected_count))
            {
                description_message = value_descriptions[i];
                break;
            }
        }
    }
    else // Treat tag as ASCII
    {
        if (expected_count > 4)
        {
            read_values = read_byte_undefined_values (file, value_offset, tag_name, expected_count, is_little_endian, tagged_bytes);

            if (read_values == expected_count)
                description_message = (gchar *) file->file_contents + value_offset;
        }
        else
        {
            description_message = (gchar *) &value_offset;
        }

        if (description_message)
        {
            if (!g_strcmp0 (tag_name, "UserComment"))
            {
                description_message = description_message + 8;
            }

            if (!g_strcmp0 (tag_name, "MakerNote") ||
                !g_strcmp0 (tag_name, "UserComment") ||
                !g_strcmp0 (tag_name, "GPSProcessingMethod") ||
                !g_strcmp0 (tag_name, "GPSAreaInformation"))
            {
                if (description_message[count - 1] == '\0')
                    count--;

                if (g_utf8_validate (description_message, count, NULL))
                    analyzer_utils_add_text_tab (tab, tag_name, description_message, count);
                else
                    analyzer_utils_add_text_tab (tab, tag_name, "", 0);
            }
        }

        description_message = NULL;
    }

    if (!description_message && possible_values)
        description_message = value_descriptions[possible_values];

    if (description_message)
    {
        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, tag_name, description_message, tag_tooltip);
        else
            analyzer_utils_describe_tooltip (file, tag_name, description_message, tag_tooltip);
    }
}
