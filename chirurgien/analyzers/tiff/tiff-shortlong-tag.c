/* tiff-shortlong-tag.c
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


gboolean
common_shortlong_tag_structure (AnalyzerFile *file,
                                guint16 field_type,
                                guint32 count,
                                guint32 expected_count,
                                guint32 *value_offset,
                                gboolean is_little_endian)
{
    guint16 short_value1, short_value2;

    guint tag_data_length = 0;
    gboolean invalid = FALSE;

    if (field_type == 3) // SHORT
    {
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: SHORT"));

        tag_data_length = (count << 1);

        if (!is_little_endian && count == 1)
        {
            *value_offset = g_ntohs (*value_offset);
        }
        else if (!is_little_endian && count == 2)
        {
            short_value1 = g_ntohs (*value_offset);
            short_value2 = g_ntohs (*value_offset >> 16);
            *value_offset = (short_value2 << 16) & short_value1;
        }
    }
    else if (field_type == 4) // LONG
    {
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: LONG"));

        tag_data_length = (count << 2);

        if (!is_little_endian)
            *value_offset = g_ntohl (*value_offset);
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
        invalid = TRUE;
    }

    if (expected_count)
    {
        if (count != expected_count)
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
        else
            analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));
    }
    else
    {
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));
    }

    if (tag_data_length > 4)
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));
    else
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));

    return invalid;
}

/*
 * Standard TIFF SHORT or LONG tags
 */

void
analyze_imagewidth_tag (AnalyzerFile *file,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;
    gboolean invalid;;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ImageWidth"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 1,
                                              &value_offset, is_little_endian);

    if (!invalid)
        description_message = g_strdup_printf ("%u", value_offset);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip (file, "ImageWidth", description_message,
                                     _("Number of columns in the image"));
    g_free (description_message);
}

void
analyze_imagelength_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;
    gboolean invalid = FALSE;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ImageLength"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 1,
                                              &value_offset, is_little_endian);

    if (!invalid)
        description_message = g_strdup_printf ("%u", value_offset);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip (file, "ImageLength", description_message,
                                     _("Number of rows in the image"));
    g_free (description_message);
}

void
analyze_rowsperstrip_tag (AnalyzerFile *file,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian)
{
    gchar *description_message;
    gboolean invalid = FALSE;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: RowsPerStrip"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 1,
                                              &value_offset, is_little_endian);

    if (!invalid)
        description_message = g_strdup_printf ("%u", value_offset);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip (file, "RowsPerStrip", description_message,
                                     _("Number of rows per strip"));
    g_free (description_message);
}

guint
analyze_stripoffsets_tag (AnalyzerFile *file,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian,
                          guint32 **strip_offsets,
                          GSList **tagged_bytes)
{
    gsize save_pointer;
    guint tag_data_length = 0;
    guint offset_length = 0;
    gboolean invalid = FALSE;

    guint32 strip_offset;
    guint strip_count;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: StripOffsets"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 0,
                                              &value_offset, is_little_endian);

    if (field_type == 3) // SHORT
    {
        tag_data_length = (count << 1);
        offset_length = 2;
    }
    else if (field_type == 4) // LONG
    {
        tag_data_length = (count << 2);
        offset_length = 4;
    }

    if (!invalid)
    {
        *strip_offsets = g_malloc (sizeof (guint32) * count);

        if (tag_data_length <= 4)
        {
            (*strip_offsets)[0] = value_offset;
            strip_count = count;
        }
        else
        {
            save_pointer = GET_POINTER (file);

            SET_POINTER (file, value_offset);
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            for (strip_count = 0; strip_count < count; strip_count++)
            {
                if (!analyzer_utils_read (&strip_offset, file, offset_length))
                    break;

                if (offset_length == 2 && !is_little_endian)
                    strip_offset = g_ntohs (strip_offset);
                if (offset_length == 4 && !is_little_endian)
                    strip_offset = g_ntohl (strip_offset);

                (*strip_offsets)[strip_count] = strip_offset;

                analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, offset_length, _("Strip offset"));
            }

            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            SET_POINTER (file, value_offset);
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Strip offset"));

            SET_POINTER (file, save_pointer);
        }
    }
    else
    {
        strip_count = 0;
    }

    return strip_count;
}

guint
analyze_stripbytecounts_tag (AnalyzerFile *file,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian,
                             guint32 **strip_byte_counts,
                             GSList **tagged_bytes)
{
    gsize save_pointer;
    guint tag_data_length = 0;
    guint offset_length = 0;
    gboolean invalid = FALSE;

    guint32 strip_byte_count;
    guint strip_count;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: StripByteCounts"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 0,
                                              &value_offset, is_little_endian);

    if (field_type == 3) // SHORT
    {
        tag_data_length = (count << 1);
        offset_length = 2;
    }
    else if (field_type == 4) // LONG
    {
        tag_data_length = (count << 2);
        offset_length = 4;
    }

    if (!invalid)
    {
        *strip_byte_counts = g_malloc (sizeof (guint32) * count);

        if (tag_data_length <= 4)
        {
            (*strip_byte_counts)[0] = value_offset;
            strip_count = count;
        }
        else
        {
            save_pointer = GET_POINTER (file);

            SET_POINTER (file, value_offset);
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            for (strip_count = 0; strip_count < count; strip_count++)
            {
                if (!analyzer_utils_read (&strip_byte_count, file, offset_length))
                    break;

                if (offset_length == 2 && !is_little_endian)
                    strip_byte_count = g_ntohs (strip_byte_count);
                if (offset_length == 4 && !is_little_endian)
                    strip_byte_count = g_ntohl (strip_byte_count);

                (*strip_byte_counts)[strip_count] = strip_byte_count;

                analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, offset_length, _("Strip byte count"));
            }

            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            SET_POINTER (file, value_offset);
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Strip byte count"));

            SET_POINTER (file, save_pointer);
        }
    }
    else
    {
        strip_count = 0;
    }

    return strip_count;
}

/*
 * Exif SHORT or LONG tags
 */

void
analyze_pixelxdimension_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;
    gboolean invalid = FALSE;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PixelXDimension"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 1,
                                              &value_offset, is_little_endian);

    if (!invalid)
        description_message = g_strdup_printf ("%u", value_offset);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip_tab (exif_tab, "PixelXDimension", description_message,
                                         _("Valid width of the meaningful image"));
    g_free (description_message);
}

void
analyze_pixelydimension_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;
    gboolean invalid = FALSE;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PixelYDimension"));

    invalid = common_shortlong_tag_structure (file, field_type, count, 1,
                                              &value_offset, is_little_endian);

    if (!invalid)
        description_message = g_strdup_printf ("%u", value_offset);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip_tab (exif_tab, "PixelYDimension", description_message,
                                         _("Valid height of the meaningful image"));
    g_free (description_message);
}
