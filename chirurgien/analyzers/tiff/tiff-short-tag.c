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

#include "tiff-analyzer.h"
#include "exif-analyzer.h"


static guint
read_short_values (AnalyzerFile *file,
                   guint32 offset,
                   gchar *tag_name,
                   guint32 count,
                   gboolean is_little_endian,
                   GSList **tagged_bytes,
                   guint32 *values)
{
    guint16 short_value;
    gsize save_pointer;
    guint i;

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (i = 0; i < count; i++)
    {
        if (!analyzer_utils_read (&short_value, file, 2))
            break;

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 2, tag_name);

        if (!is_little_endian)
            *(values + i) = g_ntohs (short_value);
        else
            *(values + i) = short_value;
    }
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, tag_name);

    SET_POINTER (file, save_pointer);

    return i;
}

static gboolean
process_short_tag_fields (AnalyzerFile *file,
                          guint16 field_type,
                          guint16 expected_field_type,
                          guint32 count,
                          guint32 expected_count,
                          guint32 *value_offset,
                          gboolean is_little_endian)
{
    guint16 short_value1, short_value2;
    gboolean valid_tag = TRUE;

    if (field_type == expected_field_type)
    {
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: SHORT"));
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
        valid_tag = FALSE;
    }

    if (count != expected_count)
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

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
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));
    else
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));

    return valid_tag;
}


static gboolean
special_short_tag (gchar *tag_name,
                   guint32 value_offset,
                   gchar **description_message)
{
    GString *description_string;

    if (!g_strcmp0 (tag_name, "PageNumber"))
    {
        *description_message = g_strdup_printf ("%u/%u", value_offset & 0x0000FFFF, value_offset >> 16);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "YCbCrSubSampling"))
    {
        description_string = g_string_new ("YCbCrSubsampleHoriz: ");
        switch (value_offset & 0x0000FFFF)
        {
            case 1:
                description_string = g_string_append (description_string,
                                                       _("Chroma image ImageWidth = Luma image ImageWidth"));
            break;
            case 2:
                description_string = g_string_append (description_string,
                                                       _("Chroma image ImageWidth = 1/2 Luma image ImageWidth"));
            break;
            case 4:
                description_string = g_string_append (description_string,
                                                       _("Chroma image ImageWidth = 1/4 Luma image ImageWidth"));
            break;
            default:
                description_string = g_string_append (description_string,
                                                       _("<span foreground=\"red\">INVALID</span>"));
        }

        g_string_append (description_string, "\nYCbCrSubsampleVert: ");
        switch (value_offset >> 16)
        {
            case 1:
                description_string = g_string_append (description_string,
                                                      _("Chroma image ImageLength = Luma image ImageLength"));
            break;
            case 2:
                description_string = g_string_append (description_string,
                                                      _("Chroma image ImageLength = 1/2 Luma image ImageLength"));
            break;
            case 4:
                description_string = g_string_append (description_string,
                                                      _("Chroma image ImageLength = 1/4 Luma image ImageLength"));
            break;
            default:
                description_string = g_string_append (description_string,
                                                      _("<span foreground=\"red\">INVALID</span>"));
        }

        *description_message = g_string_free (description_string, FALSE);
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "Flash"))
    {
        description_string = g_string_new (NULL);

        if (!(value_offset & 0x1))
            g_string_append_printf (description_string, "%s\n", _("Flash did not fire"));
        else
            g_string_append_printf (description_string, "%s\n", _("Flash fired"));

        switch ((value_offset >> 1) & 0x3)
        {
            case 0:
                g_string_append_printf (description_string, "%s\n", _("No strobe return detection function"));
            break;
            case 1:
                g_string_append_printf (description_string, "%s\n", _("<span foreground=\"red\">INVALID</span>"));
            break;
            case 2:
                g_string_append_printf (description_string, "%s\n", _("Strobe return light not detected"));
            break;
            case 3:
                g_string_append_printf (description_string, "%s\n", _("Strobe return light detected"));
            break;
        }

        switch ((value_offset >> 3) & 0x3)
        {
            case 0:
                g_string_append_printf (description_string, "%s\n", _("Unknown"));
            break;
            case 1:
                g_string_append_printf (description_string, "%s\n", _("Compulsory flash firing"));
            break;
            case 2:
                g_string_append_printf (description_string, "%s\n", _("Compulsory flash suppression"));
            break;
            case 3:
                g_string_append_printf (description_string, "%s\n", _("Auto mode"));
            break;
        }

        if (!((value_offset >> 5) & 0x1))
            g_string_append_printf (description_string, "%s\n", _("Flash function present"));
        else
            g_string_append_printf (description_string, "%s\n", _("No flash function"));

        if (!((value_offset >> 6) & 0x1))
            g_string_append_printf (description_string, "%s", _("No red-eye reduction mode or unknown"));
        else
            g_string_append_printf (description_string, "%s", _("Red-eye reduction supported"));

        *description_message = g_string_free (description_string, FALSE);
        return TRUE;
    }

    return FALSE;
}

guint32
process_short_tag (AnalyzerFile *file,
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
                   guint16 *field_values,
                   gchar **value_descriptions,
                   GSList **tagged_bytes,
                   guint32 **output_values)
{
    gchar *description_message = NULL;
    gboolean free_string = FALSE;

    GString *description_string;

    guint i;
    guint32 *values;

    if (!expected_count)
        expected_count = count;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    if (!process_short_tag_fields (file, field_type, expected_field_type, count, expected_count,
                                   &value_offset, is_little_endian))
    {
        description_message = _("<span foreground=\"red\">INVALID</span>");
    }
    else
    {
        if (possible_values)
        {
            for (i = 0; i < possible_values; i++)
            {
                if (value_offset == field_values[i])
                {
                    description_message = value_descriptions[i];
                    break;
                }
            }

            if (!description_message)
                description_message = value_descriptions[possible_values];
        }
        else
        {
            /* Certain SHORT tags require special processing */
            if (special_short_tag(tag_name, value_offset, &description_message))
            {
                free_string = TRUE;
            }
            /* These two tags in particular need to return the array of read values and the number of read values */
            else if ((!g_strcmp0 (tag_name, "StripOffsets")) || (!g_strcmp0 (tag_name, "StripByteCounts")))
            {
                values = g_malloc (sizeof (guint32) * count);

                if (count == 1)
                {
                    *values = value_offset;
                }
                else if (count == 2)
                {
                    *values = value_offset & 0xFFFF;
                    *(values + 1) = value_offset >> 16;
                }
                else
                {
                    count = read_short_values (file, value_offset, tag_name, expected_count,
                                               is_little_endian, tagged_bytes, values);
                }

                *output_values = values;
                return count;
            }
            else if (expected_count <= 1) // Standard single SHORT value print
            {
                description_message = g_strdup_printf ("%u", value_offset);
                free_string = TRUE;
            }
            else // Multiple SHORT value print
            {
                values = g_malloc (sizeof (guint32) * expected_count);
                count = read_short_values (file, value_offset, tag_name, expected_count,
                                                 is_little_endian, tagged_bytes, values);

                description_string = g_string_new (NULL);

                for (i = 0; i < count; i++)
                    g_string_append_printf (description_string, "%u,", values[i]);

                g_string_truncate (description_string, description_string->len - 1);

                description_message = g_string_free (description_string, FALSE);
                free_string = TRUE;

                g_free (values);
            }
        }
    }

    if (description_message)
    {
        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, tag_name, description_message, tag_tooltip);
        else
            analyzer_utils_describe_tooltip (file, tag_name, description_message, tag_tooltip);

        if (free_string)
            g_free (description_message);
    }

    return 0;
}
