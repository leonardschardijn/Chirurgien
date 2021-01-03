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
#include "exif-analyzer.h"
#include "chirurgien-analyze-exif.h"


guint32
common_long_tag_structure (AnalyzerFile *file,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian)
{
    if (field_type == 4) // LONG
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: LONG"));
    else
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));

    if (count != 1)
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
    else
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    return value_offset;
}

/*
 * Standard TIFF LONG tags
 */

void
analyze_newsubfiletype_tag (AnalyzerFile *file,
                            guint16 field_type,
                            guint32 count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    gchar *description_message;
    gboolean reduced_resolution, single_page, transparency_mask;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: NewSubfileType"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    reduced_resolution = value_offset & 0x1; // Bit 0
    single_page = value_offset & 0x2; // Bit 1
    transparency_mask = value_offset & 0x4; // Bit 2

    if (reduced_resolution)
        description_message = _("Yes");
    else
        description_message = _("No");
    analyzer_utils_describe_tooltip (file, _("Reducen resolution version of another image"), description_message,
                                     _("Bit 0 of the NewSubfileType tag value"));

    if (single_page)
        description_message = _("Yes");
    else
        description_message = _("No");
    analyzer_utils_describe_tooltip (file, _("Single page of multi-page image"), description_message,
                                     _("Bit 1 of the NewSubfileType tag value"));

    if (transparency_mask)
        description_message = _("Yes");
    else
        description_message = _("No");
    analyzer_utils_describe_tooltip (file, _("Defines a transparency mask for another image"), description_message,
                                     _("Bit 2 of the NewSubfileType tag value"));
}

guint32
analyze_jpeginterchangeformat_tag (AnalyzerFile *file,
                                   guint16 field_type,
                                   guint32 count,
                                   guint32 value_offset,
                                   gboolean is_little_endian)
{
    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: JPEGInterchangeFormat"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    return value_offset;
}

void
analyze_jpeginterchangeformatlength_tag (AnalyzerFile *file,
                                         guint16 field_type,
                                         guint32 count,
                                         guint32 value_offset,
                                         gboolean is_little_endian,
                                         GSList **tagged_bytes,
                                         guint32 jpeg_offset,
                                         AnalyzerTab *jpeg_tab)
{
    gsize save_pointer;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: JPEGInterchangeFormatLength"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    if (jpeg_offset)
    {
        save_pointer = GET_POINTER (file);
        SET_POINTER (file, jpeg_offset);
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, value_offset, _("Embedded JPEG file"));

        analyzer_utils_add_description_tab (jpeg_tab, _("There is an embedded JPEG file, analyze it in another tab"),
                                            NULL, NULL, 10, 0);
        analyzer_utils_embedded_file (file, jpeg_tab, value_offset);

        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        SET_POINTER (file, jpeg_offset);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Embedded JPEG file"));

        SET_POINTER (file, save_pointer);
    }
}

void
analyze_exififd_tag (AnalyzerFile *file,
                     guint16 field_type,
                     guint32 count,
                     guint32 value_offset,
                     gboolean is_little_endian,
                     GSList **tagged_bytes,
                     AnalyzerTab *exif_tab,
                     AnalyzerTab *exif_ascii_tab)
{
    gsize save_pointer;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExifIFD"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);
    SET_POINTER (file, value_offset);

    chirurgien_analyze_exif (file, is_little_endian, tagged_bytes, exif_tab, exif_ascii_tab);

    SET_POINTER (file, save_pointer);
}

void
analyze_gpsinfoifd_tag (AnalyzerFile *file,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian,
                        GSList **tagged_bytes,
                        AnalyzerTab *gpsinfo_tab,
                        AnalyzerTab *gpsinfo_ascii_tab)
{
    gsize save_pointer;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: GPSInfo"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);
    SET_POINTER (file, value_offset);

    chirurgien_analyze_gpsinfo (file, is_little_endian, tagged_bytes, gpsinfo_tab, gpsinfo_ascii_tab);

    SET_POINTER (file, save_pointer);
}

/*
 * Exif LONG tags
 */

void
analyze_standardoutputsensitivity_tag (AnalyzerFile *file,
                                       AnalyzerTab *exif_tab,
                                       guint16 field_type,
                                       guint32 count,
                                       guint32 value_offset,
                                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: StandardOutputSensitivity"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip_tab (exif_tab, "StandardOutputSensitivity", description_message,
        _("Standard output sensitivity value of the camera or input device defined in ISO 12232"));
    g_free (description_message);
}

void
analyze_recommendedexposureindex_tag (AnalyzerFile *file,
                                      AnalyzerTab *exif_tab,
                                      guint16 field_type,
                                      guint32 count,
                                      guint32 value_offset,
                                      gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: RecommendedExposureIndex"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip_tab (exif_tab, "RecommendedExposureIndex", description_message,
        _("Recommended exposure index value of the camera or input device defined in ISO 12232"));
    g_free (description_message);
}

void
analyze_isospeed_tag (AnalyzerFile *file,
                      AnalyzerTab *exif_tab,
                      guint16 field_type,
                      guint32 count,
                      guint32 value_offset,
                      gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ISOSpeed"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip_tab (exif_tab, "ISOSpeed", description_message,
        _("ISO speed value of the camera or input device defined in ISO 12232"));
    g_free (description_message);
}

void
analyze_isospeedlatitudeyyy_tag (AnalyzerFile *file,
                                 AnalyzerTab *exif_tab,
                                 guint16 field_type,
                                 guint32 count,
                                 guint32 value_offset,
                                 gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ISOSpeedLatitudeyyy"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip_tab (exif_tab, "ISOSpeedLatitudeyyy", description_message,
        _("ISO speed latitude yyy value of the camera or input device defined in ISO 12232"));
    g_free (description_message);
}

void
analyze_isospeedlatitudezzz_tag (AnalyzerFile *file,
                                 AnalyzerTab *exif_tab,
                                 guint16 field_type,
                                 guint32 count,
                                 guint32 value_offset,
                                 gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ISOSpeedLatitudezzz"));

    value_offset = common_long_tag_structure (file, field_type, count,
                                              value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip_tab (exif_tab, "ISOSpeedLatitudezzz", description_message,
        _("ISO speed latitude zzz value of the camera or input device defined in ISO 12232"));
    g_free (description_message);
}
