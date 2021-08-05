/* tiff-ascii-tag.c
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

#include "tiff-format.h"


void
process_ascii_tag (FormatsFile    *file,
                   DescriptionTab *tab,
                   const gchar    *tag_name_tag,
                   const gchar    *tag_name,
                   TiffFieldType   field_type,
                   guint32         count,
                   guint32         expected_count,
                   guint32         value_offset,
                   gboolean        is_little_endian)
{
    gsize save_index;
    const gchar *ascii_text = NULL;

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    if (field_type == ASCII)
        format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, "Field type: ASCII", NULL);
    else
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, "Invalid field type", NULL);

    if (expected_count && count != expected_count)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, "Invalid count", NULL);
    else
        format_utils_add_field (file, COUNT_COLOR, TRUE, 4, "Count", NULL);

    if (count <= 4)
    {
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag value", NULL);
        ascii_text = (const gchar *) &value_offset;
    }
    else
    {
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag offset", NULL);

        if (!is_little_endian)
            value_offset = g_ntohl (value_offset);

        save_index = GET_INDEX (file);

        SET_INDEX (file, value_offset);

        if (FILE_HAS_DATA_N (file, count))
        {
            ascii_text = (const gchar *) GET_CONTENT_POINTER (file);

            format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, count,
                                         tag_name, NULL, VALUE_OFFSET_COLOR_2);
        }

        SET_INDEX (file, save_index);
    }

    if (ascii_text && count)
    {
        if (ascii_text[count - 1] == '\0')
            count--;

        format_utils_add_text_tab (tab, tag_name, ascii_text, count);
    }
}
