/* tiff-byte-undefined-tag.c
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

#include "exif-format.h"


static guint32
read_byte_undefined_values (FormatsFile *file,
                            guint32      offset,
                            const gchar *tag_name,
                            guint32      count,
                            gboolean     is_little_endian)
{
    gsize save_index;
    guint32 read_bytes = 0;

    save_index = GET_INDEX (file);

    if (!is_little_endian)
        offset = g_ntohl (offset);

    SET_INDEX (file, offset);

    if (FILE_HAS_DATA_N (file, count))
    {
        format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, count,
                                     tag_name, NULL, VALUE_OFFSET_COLOR_2);

        read_bytes = count;
    }

    SET_INDEX (file, save_index);

    return read_bytes;
}

static void
process_byte_undefined_tag_fields (FormatsFile  *file,
                                   TiffFieldType field_type,
                                   TiffFieldType expected_field_type,
                                   guint32       count,
                                   guint32       expected_count)
{
    if (field_type == expected_field_type)
    {
        if (field_type == BYTE)
            format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, "Field type: BYTE", NULL);
        else if (field_type == UNDEFINED)
            format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, "Field type: UNDEFINED", NULL);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, "Invalid field type", NULL);
    }

    if (count != expected_count)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, "Invalid count", NULL);
    else
        format_utils_add_field (file, COUNT_COLOR, TRUE, 4, "Count", NULL);

    if (expected_count > 4)
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag offset", NULL);
    else
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag value", NULL);
}

/*
 * This function handles both BYTE and UNDEFINED tags
 */
void
process_byte_undefined_tag (FormatsFile    *file,
                            DescriptionTab *tab,
                            const gchar    *tag_name_tag,
                            const gchar    *tag_name,
                            const gchar    *tag_tooltip,
                            TiffFieldType   field_type,
                            TiffFieldType   expected_field_type,
                            guint32         count,
                            guint32         expected_count,
                            guint32         value_offset,
                            gboolean        is_little_endian,
                            guint           possible_values,
                            guint8         *field_values,
                            const gchar   **value_descriptions)
{
    const gchar *field_description = NULL;
    guint read_values = 0;

    if (!expected_count)
        expected_count = count;

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    process_byte_undefined_tag_fields (file, field_type, expected_field_type, count, expected_count);

    if (possible_values)
    {
        for (guint i = 0; i < possible_values; i++)
        {
            if (!memcmp (&value_offset, &field_values[i * expected_count], expected_count))
            {
                field_description = value_descriptions[i];
                break;
            }
        }
    }
    /* Treat tag as ASCII */
    else
    {
        if (expected_count > 4)
        {
            read_values = read_byte_undefined_values (file, value_offset, tag_name, expected_count, is_little_endian);

            if (read_values == expected_count)
                field_description = (const gchar *) GET_CONTENT_POINTER_AT (file, value_offset);
        }
        else
        {
            field_description = (const gchar *) &value_offset;
        }

        if (field_description)
        {
            if (!g_strcmp0 (tag_name, "UserComment"))
            {
                field_description = field_description + 8;
            }

            if (!g_strcmp0 (tag_name, "MakerNote") ||
                !g_strcmp0 (tag_name, "UserComment") ||
                !g_strcmp0 (tag_name, "GPSProcessingMethod") ||
                !g_strcmp0 (tag_name, "GPSAreaInformation"))
            {
                if (field_description[count - 1] == '\0')
                    count--;

                if (g_utf8_validate (field_description, count, NULL))
                    format_utils_add_text_tab (tab, tag_name, field_description, count);
                else
                    format_utils_add_text_tab (tab, tag_name, "", 0);
            }
        }

        field_description = NULL;
    }

    if (!field_description && possible_values)
        field_description = value_descriptions[possible_values];

    if (field_description)
    {
        if (tab)
            format_utils_add_line_tab (tab, tag_name, field_description, tag_tooltip);
        else
            format_utils_add_line (file, tag_name, field_description, tag_tooltip);
    }
}
