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

#include "tiff-format.h"
#include "chirurgien-exif.h"


static guint
read_long_values (FormatsFile *file,
                  guint32      offset,
                  const gchar *tag_name,
                  guint32      count,
                  gboolean     is_little_endian,
                  guint32     *values)
{
    gsize save_index;
    guint i;

    save_index = GET_INDEX (file);

    SET_INDEX (file, offset);

    for (i = 0; i < count; i++)
    {
        if (!format_utils_read (file, values + i, 4))
            break;

        if (!i)
            format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, 4,
                                         tag_name, NULL, VALUE_OFFSET_COLOR_2);
        else
            format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4,
                                    tag_name, NULL);

        if (!is_little_endian)
            *(values + i) = g_ntohl (*(values + i));
    }

    SET_INDEX (file, save_index);

    return i;
}

static gboolean
process_long_tag_fields (FormatsFile  *file,
                         TiffFieldType field_type,
                         TiffFieldType expected_field_type,
                         guint32       count,
                         guint32       expected_count,
                         guint32      *value_offset,
                         gboolean      is_little_endian,
                         gboolean     *is_offset)
{
    gboolean valid = TRUE;

    if (field_type == expected_field_type)
    {
        format_utils_add_field (file, FIELD_TYPE_COLOR, TRUE, 2, "Field type: LONG", NULL);

        if (!is_little_endian)
            *value_offset = g_ntohl (*value_offset);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, "Invalid field type", NULL);
        valid = FALSE;
    }

    if (count != expected_count)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, "Invalid count", NULL);
    else
        format_utils_add_field (file, COUNT_COLOR, TRUE, 4, "Count", NULL);

    if (expected_count > 1)
    {
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag offset", NULL);
        if (is_offset)
            *is_offset = TRUE;
    }
    else
    {
        format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 4, "Tag value", NULL);
        if (is_offset)
            *is_offset = FALSE;
    }

    return valid;
}

static gboolean
special_long_tag (FormatsFile *file,
                  const gchar *tag_name,
                  guint32      value_offset)
{
    const gchar *value;

    if (!g_strcmp0 (tag_name, "NewSubfileType"))
    {
        if (value_offset & 0x1)
            value = "Yes";
        else
            value = "No";
        format_utils_add_line (file, "Reducen resolution version of another image", value,
                               "Bit 0 of the NewSubfileType tag value");

        if (value_offset & 0x2)
            value = "Yes";
        else
            value = "No";
        format_utils_add_line (file, "Single page of multi-page image", value,
                               "Bit 1 of the NewSubfileType tag value");

        if (value_offset & 0x3)
            value = "Yes";
        else
            value = "No";
        format_utils_add_line (file, "Defines a transparency mask for another image", value,
                               "Bit 2 of the NewSubfileType tag value");

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
process_long_tag (FormatsFile    *file,
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
                  guint32       **output_values)
{
    const gchar *value = NULL;
    gboolean free_value = FALSE;

    gboolean is_offset;

    if (!expected_count)
        expected_count = count;

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    if (!process_long_tag_fields (file, field_type, expected_field_type, count, expected_count,
                             &value_offset, is_little_endian, &is_offset))
    {
        value = "<span foreground=\"red\">INVALID</span>";
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
                    *output_values[0] = value_offset;
                    return 1;
                }
                else
                {
                    return read_long_values (file, value_offset, tag_name, expected_count,
                                             is_little_endian, *output_values);
                }
            }
            /* Standard single LONG value print */
            else if (expected_count <= 1)
            {
                value = g_strdup_printf ("%u", value_offset);
                free_value = TRUE;
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

    return value_offset;
}

/*
 * Exif and Exif GPSInfo IFDs
 */

gboolean
process_exififd_tag (FormatsFile   *file,
                     const gchar   *tag_name_tag,
                     TiffFieldType  field_type,
                     guint32        count,
                     guint32        value_offset,
                     gboolean       is_little_endian,
                     DescriptionTab *tab,
                     DescriptionTab *ascii_tab,
                     ExifIFDSelector exififd)
{
    gsize save_index;
    gboolean success = TRUE;

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, tag_name_tag, NULL);

    if (process_long_tag_fields (file, field_type, LONG, count, 1, &value_offset, is_little_endian, NULL))
    {
        save_index = GET_INDEX (file);
        SET_INDEX (file, value_offset);

        if (exififd == EXIF_IFD)
            success = chirurgien_exif (file, is_little_endian, tab, ascii_tab);
        else if (exififd == GPSINFO_IFD)
            success = chirurgien_gpsinfo (file, is_little_endian, tab, ascii_tab);

        SET_INDEX (file, save_index);
    }

    return success;
}
