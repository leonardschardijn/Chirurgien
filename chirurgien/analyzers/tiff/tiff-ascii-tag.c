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

#include <config.h>

#include <glib/gi18n.h>

#include "tiff-analyzer.h"


void
process_ascii_tag (AnalyzerFile *file,
                   AnalyzerTab *tab,
                   gchar *tag_name_tag,
                   gchar *tag_name,
                   guint16 field_type,
                   guint32 count,
                   guint32 expected_count,
                   guint32 value_offset,
                   gboolean is_little_endian,
                   GSList **tagged_bytes)
{
    gsize save_pointer;
    gchar *ascii_text = NULL;
    gboolean valid_ascii_tag = FALSE;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    if (field_type == ASCII)
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: ASCII"));
    else
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));

    if (expected_count && count != expected_count)
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    if (count <= 4)
    {
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));
        ascii_text = (gchar *) &value_offset;
    }
    else
    {
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));

        if (!is_little_endian)
            value_offset = g_ntohl (value_offset);

        save_pointer = GET_POINTER (file);

        SET_POINTER (file, value_offset);

        if (FILE_HAS_DATA_N (file, count))
        {
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            ascii_text = (gchar *) file->file_contents + value_offset;

            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, tag_name);

            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file) + count));

            SET_POINTER (file, value_offset);
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, tag_name);
        }

        SET_POINTER (file, save_pointer);
    }

    if (ascii_text && count)
    {
        if (ascii_text[count - 1] == '\0')
            count--;

        if (g_utf8_validate (ascii_text, count, NULL))
        {
            analyzer_utils_add_text_tab (tab, tag_name, ascii_text, count);
            valid_ascii_tag = TRUE;
        }
    }

    if (!valid_ascii_tag)
        analyzer_utils_add_text_tab (tab, tag_name, "", 0);
}
