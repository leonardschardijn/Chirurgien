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

#include "tiff-analyzer.h"
#include "exif-analyzer.h"


static guint
read_rational_values (AnalyzerFile *file,
                      guint32 offset,
                      gchar **tag_value_names,
                      guint32 count,
                      gboolean is_little_endian,
                      GSList **tagged_bytes,
                      guint32 *values)
{
    gsize save_pointer;
    guint i, j;

    count = count << 1;

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (i = 0, j = 0; i < count; i += 2, j++)
    {
        if (FILE_HAS_DATA_N (file, 8))
        {
            analyzer_utils_read (values + i, file, 4);
            analyzer_utils_read (values + i + 1, file, 4);
        }
        else
        {
            break;
        }

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, tag_value_names[j]);

        if (!is_little_endian)
        {
            values[i] = g_ntohl (values[i]);
            values[i + 1] = g_ntohl (values[i + 1]);
        }
    }
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, tag_value_names[0]);

    SET_POINTER (file, save_pointer);

    return i;
}

static guint32
process_rational_tag_fields (AnalyzerFile *file,
                             guint16 field_type,
                             guint16 expected_field_type,
                             guint32 count,
                             guint32 expected_count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    if (field_type == expected_field_type)
    {
        if (field_type == RATIONAL)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: RATIONAL"));
        else if (field_type == SRATIONAL)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: SRATIONAL"));
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
    }

    if (expected_count && count != expected_count)
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    return value_offset;
}

static gboolean
special_rational_tag (gchar *tag_name,
                      guint32 read_values,
                      guint32 expected_count,
                      guint32 *values,
                      GString *description_message)
{
    if (!g_strcmp0 (tag_name, "YCbCrCoefficients"))
    {
        if (read_values == expected_count << 1)
            g_string_append_printf (description_message, "%u/%u,%u/%u,%u/%u",
                                    values[0], values[1], values[2],
                                    values[3], values[4], values[5]);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "ReferenceBlackWhite"))
    {
        if (read_values == expected_count << 1)
        {
            if (values[1] && values[3] && values[5] && values[7] && values[9] && values[11])
                g_string_append_printf (description_message, "%u/%u,%u/%u,%u/%u",
                                        values[0] / values[1], values[2] / values[3],
                                        values[4] / values[5], values[6] / values[7],
                                        values[8] / values[9], values[10] / values[11]);
        }
        return TRUE;
    }

    return FALSE;
}

void
process_rational_tag (AnalyzerFile *file,
                      AnalyzerTab *tab,
                      gchar *tag_name_tag,
                      gchar *tag_name,
                      gchar *tag_tooltip,
                      gchar **tag_value_names,
                      guint16 field_type,
                      guint16 expected_field_type,
                      guint32 count,
                      guint32 expected_count,
                      guint32 value_offset,
                      gboolean is_little_endian,
                      gchar *print_format,
                      GSList **tagged_bytes)
{
    guint read_values;
    GString *description_message;
    guint32 values[12]; // The largest RATIONAL tag has a count of 6 (6 RATIONAL values = 12 32-bit values)

    g_autofree gchar *print_format_multiline = NULL;

    if (!tag_value_names)
        tag_value_names = &tag_name;

    if (!print_format)
        print_format = "%f";

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    value_offset = process_rational_tag_fields (file, field_type, expected_field_type, count,
                                                expected_count, value_offset, is_little_endian);

    read_values = read_rational_values (file, value_offset, tag_value_names, expected_count,
                                        is_little_endian, tagged_bytes, values);

    description_message = g_string_new (NULL);

    /* Certain RATIONAL tags require special processing */
    if (!special_rational_tag (tag_name, read_values, expected_count, values, description_message))
    {
        if (read_values && expected_count == 1) // Standard single RATIONAL value print
        {
            if (!values[1])
            {
                if (expected_field_type == RATIONAL)
                    g_string_append_printf (description_message, "%u/0", values[1]);
                else
                    g_string_append_printf (description_message, "%u/0", (gint32) values[1]);
            }
            else
            {
                if (expected_field_type == RATIONAL)
                    g_string_append_printf (description_message, print_format,
                                            (gfloat) values[0] / values[1]);
                else
                    g_string_append_printf (description_message, print_format,
                            (gfloat) (gint32) values[0] / (gint32) values[1]);
            }
        }
        else // Standard multiple RATIONAL value print
        {
            print_format_multiline = g_strdup_printf (": %s", print_format);

            for (guint i = 0, j = 0; i < read_values; i += 2, j++)
            {
                if (!values[i + 1])
                {
                    description_message = g_string_append (description_message, tag_value_names[j]);
                    g_string_append_printf (description_message, ": %u/0", values[i]);
                }
                else
                {
                        description_message = g_string_append (description_message, tag_value_names[j]);
                        g_string_append_printf (description_message, print_format_multiline,
                                                (gfloat) values[i] / values[i + 1]);
                }

                if (i != read_values - 2)
                    description_message = g_string_append_c (description_message, '\n');
            }
        }
    }

    if (!description_message->len)
        g_string_append (description_message, _("<span foreground=\"red\">INVALID</span>"));

    if (tab)
        analyzer_utils_describe_tooltip_tab (tab, tag_name, description_message->str, tag_tooltip);
    else
        analyzer_utils_describe_tooltip (file, tag_name, description_message->str, tag_tooltip);

    g_string_free (description_message, TRUE);
}
