/* tiff-long-tag.c
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
#include "chirurgien-analyze-exif.h"


static guint
read_long_values (AnalyzerFile *file,
                  guint32 offset,
                  gchar *tag_name,
                  guint32 count,
                  gboolean is_little_endian,
                  GSList **tagged_bytes,
                  guint32 *values)
{
    gsize save_pointer;
    guint i;

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (i = 0; i < count; i++)
    {
        if (!analyzer_utils_read (values + i, file, 4))
            break;

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, tag_name);

        if (!is_little_endian)
            *(values + i) = g_ntohl (*(values + i));
    }
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, tag_name);

    SET_POINTER (file, save_pointer);

    return i;
}

static gboolean
process_long_tag_fields (AnalyzerFile *file,
                         guint16 field_type,
                         guint16 expected_field_type,
                         guint32 count,
                         guint32 expected_count,
                         guint32 *value_offset,
                         gboolean is_little_endian,
                         gboolean *is_offset)
{
    gboolean valid = TRUE;

    if (field_type == expected_field_type)
    {
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: LONG"));

        if (!is_little_endian)
            *value_offset = g_ntohl (*value_offset);
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
        valid = FALSE;
    }

    if (count != expected_count)
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    if (expected_count > 1)
    {
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));
        if (is_offset)
            *is_offset = TRUE;
    }
    else
    {
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));
        if (is_offset)
            *is_offset = FALSE;
    }

    return valid;
}

static gboolean
special_long_tag (AnalyzerFile *file,
                  gchar *tag_name,
                  guint32 value_offset)
{
    gchar *description_message;

    if (!g_strcmp0 (tag_name, "NewSubfileType"))
    {
        if (value_offset & 0x1)
            description_message = _("Yes");
        else
            description_message = _("No");
        analyzer_utils_describe_tooltip (file, _("Reducen resolution version of another image"), description_message,
                                         _("Bit 0 of the NewSubfileType tag value"));

        if (value_offset & 0x2)
            description_message = _("Yes");
        else
            description_message = _("No");
        analyzer_utils_describe_tooltip (file, _("Single page of multi-page image"), description_message,
                                         _("Bit 1 of the NewSubfileType tag value"));

        if (value_offset & 0x3)
            description_message = _("Yes");
        else
            description_message = _("No");
        analyzer_utils_describe_tooltip (file, _("Defines a transparency mask for another image"), description_message,
                                         _("Bit 2 of the NewSubfileType tag value"));

        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "JPEGInterchangeFormat"))
    {
        return TRUE;
    }
    else if (!g_strcmp0 (tag_name, "JPEGInterchangeFormatLength"))
    {
        return TRUE;
    }

    return FALSE;
}

guint32
process_long_tag (AnalyzerFile *file,
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
                  GSList **tagged_bytes,
                  guint32 **output_values)
{
    gchar *description_message = NULL;
    gboolean is_offset;

    if (!expected_count)
        expected_count = count;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    if (!process_long_tag_fields (file, field_type, expected_field_type, count, expected_count,
                             &value_offset, is_little_endian, &is_offset))
    {
        description_message = _("<span foreground=\"red\">INVALID</span>");
    }
    else
    {
        /* Certain LONG tags require special processing */
        if (!special_long_tag (file, tag_name, value_offset))
        {
            /* These two tags in particular need to return the array of read values and the number of read values */
            if ((!g_strcmp0 (tag_name, "StripOffsets")) || (!g_strcmp0 (tag_name, "StripByteCounts")))
            {
                *output_values = g_malloc (sizeof (guint32) * count);

                if (count == 1)
                {
                    *(*output_values) = value_offset;
                    return 1;
                }
                else
                {
                    return read_long_values (file, value_offset, tag_name, expected_count,
                                             is_little_endian, tagged_bytes, *output_values);
                }
            }
            else if (expected_count <= 1) // Standard single LONG value print
            {
                description_message = g_strdup_printf ("%u", value_offset);
            }
        }
    }

    if (description_message)
    {
        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, tag_name, description_message, tag_tooltip);
        else
            analyzer_utils_describe_tooltip (file, tag_name, description_message, tag_tooltip);
        g_free (description_message);
    }

    return value_offset;
}

/*
 * Exif and Exif GPSInfo IFDs
 */

void
process_exififd_tag (AnalyzerFile *file,
                     gchar *tag_name_tag,
                     guint16 field_type,
                     guint32 count,
                     guint32 value_offset,
                     gboolean is_little_endian,
                     GSList **tagged_bytes,
                     AnalyzerTab *tab,
                     AnalyzerTab *ascii_tab,
                     gboolean exififd)
{
    gsize save_pointer;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, tag_name_tag);

    if (process_long_tag_fields (file, field_type, LONG, count, 1, &value_offset, is_little_endian, NULL))
    {
        save_pointer = GET_POINTER (file);
        SET_POINTER (file, value_offset);

        if (exififd)
            chirurgien_analyze_exif (file, is_little_endian, tagged_bytes, tab, ascii_tab);
        else
            chirurgien_analyze_gpsinfo (file, is_little_endian, tagged_bytes, tab, ascii_tab);

        SET_POINTER (file, save_pointer);
    }
}
