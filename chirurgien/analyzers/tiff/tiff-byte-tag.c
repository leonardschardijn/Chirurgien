/* tiff-byte-tag.c
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

#include "exif-gpsinfo-analyzer.h"


void
common_byte_tag_structure (AnalyzerFile *file,
                           guint16 field_type,
                           guint32 count,
                           guint32 expected_count)
{
    if (field_type == 1) // BYTE
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: BYTE"));
    else
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));

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

    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));
}

/*
 * Exif BYTE tags
 */

void
analyze_gpsversionid_tag (AnalyzerFile *file,
                          AnalyzerTab *exif_tab,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: GPSVersionID"));

    common_byte_tag_structure (file, field_type, count, 4);

    if (value_offset == 0x00000302)
        description_message = "v2.3";
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (exif_tab, "GPSVersionID", description_message,
                                         "GPSVersionID\n"
                                         "<tt>00 00 03 02<sub>16</sub></tt>\tv2.3");
}
