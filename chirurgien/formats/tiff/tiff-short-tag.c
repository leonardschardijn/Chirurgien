/* tiff-short-tag.c
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
read_short_values (FormatsFile *file,
                   guint32      offset,
                   const gchar *tag_name,
                   guint32      count,
                   gboolean     is_little_endian,
                   guint32     *values)
{
    guint16 short_value;
    gsize save_index;
    guint i;

    save_index = GET_INDEX (file);

    SET_INDEX (file, offset);

    for (i = 0; i < count; i++)
    {
        if (!format_utils_read (file, &short_value, 2))
            break;

        if (!i)
            format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, 2,
                                         tag_name, NULL, VALUE_OFFSET_COLOR_2);
        else
            format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 2,
                                    tag_name, NULL);

        if (!is_little_endian)
            *(values + i) = g_ntohs (short_value);
        else
            *(values + i) = short_value;
    }

    SET_INDEX (file, save_index);

    return i;
}

static gboolean
process_short_tag_fields (FormatsFile  *file,
                          TiffFieldType field_type,
                          TiffFieldType expected_field_type,
                          guint32       count,
                          guint32       expected_count,
                          guint32      *value_offset,
                          gboolean      is_little_endian)
{
    guint16 short_value1, short_value2;
    gboolean valid_tag = TRUE;

    if (field_type == expected_field_type)
    {
        format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, _("Field type: SHORT"), NULL);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, _("Invalid field type"), NULL);
        valid_tag = FALSE;
    }

    if (count != expected_count)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, _("Invalid count"), NULL);
    else
        format_utils_add_field (file, COUNT_COLOR, TRUE, 4, _("Count"), NULL);

    if (!is_little_endian && expected_count == 1)
    {
        *value_offset = g_ntohs (*value_offset);
    }
    else if (!is_little_endian && expected_count == 2)
    {
        short_value1 = g_ntohs (*value_offset);
        short_value2 = g_ntohs (*value_offset >> 16);
        *value_offset = (short_value2 << 16) & short_value1;
    }

    if (expected_count > 2)
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, _("Tag offset"), NULL);
    else
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, _("Tag value"), NULL);

    return valid_tag;
}

static gboolean
special_short_tag (const gchar  *tag_name,
                   guint32       value_offset,
                   const gchar **value)
{
    GString *string_value;

    if (!g_strcmp0 (tag_name, "PageNumber"))
    {
        *value = g_strdup_printf ("%u/%u", value_offset & 0x0000FFFF, value_offset >> 16);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "YCbCrSubSampling"))
    {
        string_value = g_string_new ("YCbCrSubsampleHoriz: ");
        switch (value_offset & 0x0000FFFF)
        {
            case 1:
                string_value = g_string_append (string_value,
                                                       _("Chroma image ImageWidth = Luma image ImageWidth"));
            break;
            case 2:
                string_value = g_string_append (string_value,
                                                       _("Chroma image ImageWidth = 1/2 Luma image ImageWidth"));
            break;
            case 4:
                string_value = g_string_append (string_value,
                                                       _("Chroma image ImageWidth = 1/4 Luma image ImageWidth"));
            break;
            default:
                string_value = g_string_append (string_value,
                                                       _("<span foreground=\"red\">INVALID</span>"));
        }

        g_string_append (string_value, "\nYCbCrSubsampleVert: ");
        switch (value_offset >> 16)
        {
            case 1:
                string_value = g_string_append (string_value,
                                                      _("Chroma image ImageLength = Luma image ImageLength"));
            break;
            case 2:
                string_value = g_string_append (string_value,
                                                      _("Chroma image ImageLength = 1/2 Luma image ImageLength"));
            break;
            case 4:
                string_value = g_string_append (string_value,
                                                      _("Chroma image ImageLength = 1/4 Luma image ImageLength"));
            break;
            default:
                string_value = g_string_append (string_value,
                                                      _("<span foreground=\"red\">INVALID</span>"));
        }

        *value = g_string_free (string_value, FALSE);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "Flash"))
    {
        string_value = g_string_new (NULL);

        if (!(value_offset & 0x1))
            g_string_append_printf (string_value, "%s\n", _("Flash did not fire"));
        else
            g_string_append_printf (string_value, "%s\n", _("Flash fired"));

        switch ((value_offset >> 1) & 0x3)
        {
            case 0:
                g_string_append_printf (string_value, "%s\n", _("No strobe return detection function"));
            break;
            case 1:
                g_string_append_printf (string_value, "%s\n", _("<span foreground=\"red\">INVALID</span>"));
            break;
            case 2:
                g_string_append_printf (string_value, "%s\n", _("Strobe return light not detected"));
            break;
            case 3:
                g_string_append_printf (string_value, "%s\n", _("Strobe return light detected"));
            break;
        }

        switch ((value_offset >> 3) & 0x3)
        {
            case 0:
                g_string_append_printf (string_value, "%s\n", _("Unknown"));
            break;
            case 1:
                g_string_append_printf (string_value, "%s\n", _("Compulsory flash firing"));
            break;
            case 2:
                g_string_append_printf (string_value, "%s\n", _("Compulsory flash suppression"));
            break;
            case 3:
                g_string_append_printf (string_value, "%s\n", _("Auto mode"));
            break;
        }

        if (!((value_offset >> 5) & 0x1))
            g_string_append_printf (string_value, "%s\n", _("Flash function present"));
        else
            g_string_append_printf (string_value, "%s\n", _("No flash function"));

        if (!((value_offset >> 6) & 0x1))
            g_string_append_printf (string_value, "%s", _("No red-eye reduction mode or unknown"));
        else
            g_string_append_printf (string_value, "%s", _("Red-eye reduction supported"));

        *value = g_string_free (string_value, FALSE);
        return TRUE;
    }

    return FALSE;
}

guint32
process_short_tag (FormatsFile    *file,
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
                   guint16        *field_values,
                   const gchar   **value_descriptions,
                   guint32       **values_out)
{
    const gchar *value = NULL;
    gboolean free_value = FALSE;

    GString *string_value;

    guint i;
    guint32 *numeric_values;

    if (!expected_count)
        expected_count = count;

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    if (!process_short_tag_fields (file, field_type, expected_field_type, count, expected_count,
                                   &value_offset, is_little_endian))
    {
        value = _("<span foreground=\"red\">INVALID</span>");
    }
    else
    {
        if (possible_values)
        {
            for (i = 0; i < possible_values; i++)
            {
                if (value_offset == field_values[i])
                {
                    value = value_descriptions[i];
                    break;
                }
            }

            if (!value)
                value = value_descriptions[possible_values];
        }
        else
        {
            /* Certain SHORT tags require special processing */
            if (special_short_tag(tag_name, value_offset, &value))
            {
                free_value = TRUE;
            }
            /* These two tags in particular need to return the array of read values and the number of read values */
            else if ((!g_strcmp0 (tag_name, "StripOffsets")) || (!g_strcmp0 (tag_name, "StripByteCounts")))
            {
                *values_out = g_malloc (sizeof (guint32) * count);

                if (count == 1)
                {
                    *values_out[0] = value_offset;
                }
                else if (count == 2)
                {
                    *values_out[0] = value_offset & 0xFFFF;
                    *values_out[1] = value_offset >> 16;
                }
                else
                {
                    count = read_short_values (file, value_offset, tag_name, expected_count,
                                               is_little_endian, *values_out);
                }

                return count;
            }
            /* Standard single SHORT value print */
            else if (expected_count <= 1)
            {
                value = g_strdup_printf ("%u", value_offset);
                free_value = TRUE;
            }
            /* Multiple SHORT value print */
            else
            {
                numeric_values = g_malloc (sizeof (guint32) * expected_count);
                count = read_short_values (file, value_offset, tag_name, expected_count,
                                                 is_little_endian, numeric_values);

                string_value = g_string_new (NULL);

                for (i = 0; i < count; i++)
                    g_string_append_printf (string_value, "%u,", numeric_values[i]);

                g_string_truncate (string_value, string_value->len - 1);

                value = g_string_free (string_value, FALSE);
                free_value = TRUE;

                g_free (numeric_values);
            }
        }
    }

    if (value)
    {
        if (tab)
            format_utils_add_line_tab (tab, tag_name, value, tag_tooltip);
        else
            format_utils_add_line (file, tag_name, value, tag_tooltip);

        if (free_value)
            g_free ((gpointer) value);
    }

    return 0;
}
