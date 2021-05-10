/* tiff-rational-tag.c
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

#include "tiff-format.h"
#include "exif-format.h"


static guint
read_rational_values (FormatsFile  *file,
                      guint32       offset,
                      const gchar **tag_value_names,
                      guint32       count,
                      gboolean      is_little_endian,
                      guint32      *values)
{
    gsize save_pointer;
    guint i, j;

    count = count << 1;

    save_pointer = GET_INDEX (file);

    SET_INDEX (file, offset);

    for (i = 0, j = 0; i < count; i += 2, j++)
    {
        if (FILE_HAS_DATA_N (file, 8))
        {
            format_utils_read (file, values + i, 4);
            ADVANCE_INDEX (file, 4);
            format_utils_read (file, values + i + 1, 4);
            ADVANCE_INDEX (file, -4);
        }
        else
        {
            break;
        }

        if (!i)
            format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, 8,
                                         tag_value_names[j], NULL, VALUE_OFFSET_COLOR_2);
        else
            format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 8,
                                    tag_value_names[j], NULL);

        if (!is_little_endian)
        {
            values[i] = g_ntohl (values[i]);
            values[i + 1] = g_ntohl (values[i + 1]);
        }
    }

    SET_INDEX (file, save_pointer);

    return i;
}

static guint32
process_rational_tag_fields (FormatsFile  *file,
                             TiffFieldType field_type,
                             TiffFieldType expected_field_type,
                             guint32       count,
                             guint32       expected_count,
                             guint32       value_offset,
                             gboolean      is_little_endian)
{
    if (field_type == expected_field_type)
    {
        if (field_type == RATIONAL)
            format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, _("Field type: RATIONAL"), NULL);
        else if (field_type == SRATIONAL)
            format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, _("Field type: SRATIONAL"), NULL);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, _("Invalid field type"), NULL);
    }

    if (expected_count && count != expected_count)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, _("Invalid count"), NULL);
    else
        format_utils_add_field (file, COUNT_COLOR, TRUE, 4, _("Count"), NULL);

    format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, _("Tag offset"), NULL);

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    return value_offset;
}

static gboolean
special_rational_tag (const gchar *tag_name,
                      guint32      read_values,
                      guint32      expected_count,
                      guint32     *values,
                      GString     *field_description)
{
    if (!g_strcmp0 (tag_name, "YCbCrCoefficients"))
    {
        if (read_values == expected_count << 1)
            g_string_append_printf (field_description, "%u/%u,%u/%u,%u/%u",
                                    values[0], values[1], values[2],
                                    values[3], values[4], values[5]);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "ReferenceBlackWhite"))
    {
        if (read_values == expected_count << 1)
        {
            if (values[1] && values[3] && values[5] && values[7] && values[9] && values[11])
                g_string_append_printf (field_description, "%u/%u,%u/%u,%u/%u",
                                        values[0] / values[1], values[2] / values[3],
                                        values[4] / values[5], values[6] / values[7],
                                        values[8] / values[9], values[10] / values[11]);
        }
        return TRUE;
    }

    return FALSE;
}

void
process_rational_tag (FormatsFile    *file,
                      DescriptionTab *tab,
                      const gchar    *tag_name_tag,
                      const gchar    *tag_name,
                      const gchar    *tag_tooltip,
                      const gchar   **tag_value_names,
                      TiffFieldType   field_type,
                      TiffFieldType   expected_field_type,
                      guint32         count,
                      guint32         expected_count,
                      guint32         value_offset,
                      gboolean        is_little_endian,
                      const gchar    *print_format)
{
    guint read_values;
    GString *field_description;
    guint32 values[12]; /* The largest RATIONAL tag has a count of 6 (6 RATIONAL values = 12 32-bit values) */

    g_autofree gchar *print_format_multiline = NULL;

    if (!tag_value_names)
        tag_value_names = &tag_name;

    if (!print_format)
        print_format = "%f";

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    value_offset = process_rational_tag_fields (file, field_type, expected_field_type, count,
                                                expected_count, value_offset, is_little_endian);

    read_values = read_rational_values (file, value_offset, tag_value_names, expected_count,
                                        is_little_endian, values);

    field_description = g_string_new (NULL);

    /* Certain RATIONAL tags require special processing */
    if (!special_rational_tag (tag_name, read_values, expected_count, values, field_description))
    {
        /* Standard single RATIONAL value print */
        if (read_values && expected_count == 1)
        {
            if (!values[1])
            {
                if (expected_field_type == RATIONAL)
                    g_string_append_printf (field_description, "%u/0", values[1]);
                else
                    g_string_append_printf (field_description, "%u/0", (gint32) values[1]);
            }
            else
            {
                if (expected_field_type == RATIONAL)
                    g_string_append_printf (field_description, print_format,
                                            (gfloat) values[0] / values[1]);
                else
                    g_string_append_printf (field_description, print_format,
                            (gfloat) (gint32) values[0] / (gint32) values[1]);
            }
        }
        /* Standard multiple RATIONAL value print */
        else
        {
            print_format_multiline = g_strdup_printf (": %s", print_format);

            for (guint i = 0, j = 0; i < read_values; i += 2, j++)
            {
                if (!values[i + 1])
                {
                    field_description = g_string_append (field_description, tag_value_names[j]);
                    g_string_append_printf (field_description, ": %u/0", values[i]);
                }
                else
                {
                        field_description = g_string_append (field_description, tag_value_names[j]);
                        g_string_append_printf (field_description, print_format_multiline,
                                                (gfloat) values[i] / values[i + 1]);
                }

                if (i != read_values - 2)
                    field_description = g_string_append_c (field_description, '\n');
            }
        }
    }

    if (!field_description->len)
        g_string_append (field_description, _("<span foreground=\"red\">INVALID</span>"));

    if (tab)
        format_utils_add_line_tab (tab, tag_name, field_description->str, tag_tooltip);
    else
        format_utils_add_line (file, tag_name, field_description->str, tag_tooltip);

    g_string_free (field_description, TRUE);
}
