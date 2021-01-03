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


guint32
common_rational_tag_structure (AnalyzerFile *file,
                               guint16 field_type,
                               guint16 expected_field_type,
                               guint32 count,
                               guint32 expected_count,
                               guint32 value_offset,
                               gboolean is_little_endian)
{
    if (field_type == expected_field_type) // 5 = RATIONAL, 10 = SRATIONAL
    {
        if (field_type == 5)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: RATIONAL"));
        else if (field_type == 10)
            analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: SRATIONAL"));
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));
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

    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    return value_offset;
}

/*
 * Standard TIFF RATIONAL tags
 */

void
analyze_xresolution_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian,
                         GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 xresolution[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: XResolution"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&xresolution[0], file, 4);
    read_success = analyzer_utils_read (&xresolution[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "XResolution");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "XResolution");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            xresolution[0] = g_ntohl (xresolution[0]);
            xresolution[1] = g_ntohl (xresolution[1]);
        }

        description_message = g_strdup_printf ("%.1f", (gfloat) xresolution[0] / xresolution[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip (file, "XResolution", description_message,
                                     _("Number of pixels per ResolutionUnit in the ImageWidth direction"));
    g_free (description_message);
}

void
analyze_yresolution_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian,
                         GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 yresolution[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: YResolution"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&yresolution[0], file, 4);
    read_success = analyzer_utils_read (&yresolution[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "YResolution");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "YResolution");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            yresolution[0] = g_ntohl (yresolution[0]);
            yresolution[1] = g_ntohl (yresolution[1]);
        }

        description_message = g_strdup_printf ("%.1f", (gfloat) yresolution[0] / yresolution[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip (file, "YResolution", description_message,
                                     _("Number of pixels per ResolutionUnit in the ImageLength direction"));
    g_free (description_message);
}

void
analyze_xposition_tag (AnalyzerFile *file,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian,
                       GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 xposition[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: XPosition"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&xposition[0], file, 4);
    read_success = analyzer_utils_read (&xposition[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "XPosition");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "XPosition");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            xposition[0] = g_ntohl (xposition[0]);
            xposition[1] = g_ntohl (xposition[1]);
        }

        description_message = g_strdup_printf ("%.1f", (gfloat) xposition[0] / xposition[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip (file, "XPosition", description_message,
                                     _("The X offset in ResolutionUnits of the left side of the image"));
    g_free (description_message);
}

void
analyze_yposition_tag (AnalyzerFile *file,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian,
                       GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 yposition[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: YPosition"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&yposition[0], file, 4);
    read_success = analyzer_utils_read (&yposition[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "YPosition");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "YPosition");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            yposition[0] = g_ntohl (yposition[0]);
            yposition[1] = g_ntohl (yposition[1]);
        }

        description_message = g_strdup_printf ("%.1f", (gfloat) yposition[0] / yposition[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip (file, "YPosition", description_message,
                            _("The Y offset in ResolutionUnits of the top of the image"));
    g_free (description_message);
}

void
analyze_whitepoint_tag (AnalyzerFile *file,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian,
                        GSList **tagged_bytes)
{
    gboolean read_success;
    gsize save_pointer;
    gchar *description_message;
    guint32 whitepoint[4];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: WhitePoint"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 2,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&whitepoint[0], file, 4);
    analyzer_utils_read (&whitepoint[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, _("White point x"));

    analyzer_utils_read (&whitepoint[2], file, 4);
    read_success = analyzer_utils_read (&whitepoint[3], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, _("White point y"));

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("White point x"));

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            whitepoint[0] = g_ntohl (whitepoint[0]);
            whitepoint[1] = g_ntohl (whitepoint[1]);
            whitepoint[2] = g_ntohl (whitepoint[2]);
            whitepoint[3] = g_ntohl (whitepoint[3]);
        }

        description_message = g_strdup_printf (_("White point x: %f\nWhite point y: %f"),
                                               (gfloat) whitepoint[0] / whitepoint[1],
                                               (gfloat) whitepoint[2] / whitepoint[3]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip (file, "WhitePoint", description_message,
                                     _("The values are described using the 1931 CIE xy chromaticity diagram"));
    g_free (description_message);
}

void
analyze_primarychromaticities_tag (AnalyzerFile *file,
                                   guint16 field_type,
                                   guint32 count,
                                   guint32 value_offset,
                                   gboolean is_little_endian,
                                   GSList **tagged_bytes)
{
    gchar *chromaticities[] = {
        _("Red x"),
        _("Red y"),
        _("Green x"),
        _("Green y"),
        _("Blue x"),
        _("Blue y")
    };

    gboolean read_success;
    gsize save_pointer;
    GString *description_message;
    guint32 chromaticity[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PrimaryChromaticities"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 6,
                                                  value_offset, is_little_endian);

    description_message = g_string_new (NULL);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < 6; i++)
    {
        analyzer_utils_read (&chromaticity[0], file, 4);
        read_success = analyzer_utils_read (&chromaticity[1], file, 4);

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, chromaticities[i]);

        description_message = g_string_append (description_message, chromaticities[i]);

        if (!read_success)
        {
            g_string_append_printf (description_message, ": %s",
                                    _("<span foreground=\"red\">INVALID OFFSET</span>"));
            break;
        }

        if (!is_little_endian)
        {
            chromaticity[0] = g_ntohl (chromaticity[0]);
            chromaticity[1] = g_ntohl (chromaticity[1]);
        }

        g_string_append_printf (description_message, ": %f",
                                (gfloat) chromaticity[0] / chromaticity[1]);
        if (i != 5)
            description_message = g_string_append_c (description_message, '\n');
    }
    analyzer_utils_describe_tooltip (file, "PrimaryChromaticities", description_message->str,
                                     _("The values are described using the 1931 CIE xy chromaticity diagram"));
    g_string_free (description_message, TRUE);

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, chromaticities[0]);

    SET_POINTER (file, save_pointer);
}

void
analyze_ycbcrcoefficients_tag (AnalyzerFile *file,
                               guint16 field_type,
                               guint32 count,
                               guint32 value_offset,
                               gboolean is_little_endian,
                               GSList **tagged_bytes)
{
    gboolean read_success;
    gsize save_pointer;
    gchar *description_message;
    guint32 coefficient[6];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: YCbCrCoefficients"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 3,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < 6; i+= 2)
    {
        analyzer_utils_read (&coefficient[i], file, 4);
        read_success = analyzer_utils_read (&coefficient[i + 1], file, 4);
        if (i % 4)
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 8, _("YCbCr coefficient"));
        else
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, _("YCbCr coefficient"));

        if (!read_success)
        {
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
            break;
        }

        if (!is_little_endian)
        {
            coefficient[i] = g_ntohl (coefficient[i]);
            coefficient[i + 1] = g_ntohl (coefficient[i + 1]);
        }
    }

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("YCbCr coefficient"));

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        description_message = g_strdup_printf ("%u/%u,%u/%u,%u/%u",
                    coefficient[0], coefficient[1], coefficient[2], coefficient[3],
                    coefficient[4], coefficient[5]);
    }
    analyzer_utils_describe_tooltip (file, "YCbCrCoefficients", description_message,
                                     _("Coefficients used to compute luminance Y"));
    g_free (description_message);
}

void
analyze_referenceblackwhite_tag (AnalyzerFile *file,
                                 guint16 field_type,
                                 guint32 count,
                                 guint32 value_offset,
                                 gboolean is_little_endian,
                                 GSList **tagged_bytes)
{
    gboolean read_success;
    gsize save_pointer;
    gchar *description_message;
    guint32 referenceblackwhite[12];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ReferenceBlackWhite"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 6,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < 12; i+= 2)
    {
        analyzer_utils_read (&referenceblackwhite[i], file, 4);
        read_success = analyzer_utils_read (&referenceblackwhite[i + 1], file, 4);
        if (i % 4)
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, _("ReferenceBlackWhite footroom"));
        else
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, _("ReferenceBlackWhite headroom"));

        if (!read_success)
        {
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
            break;
        }

        if (!is_little_endian)
        {
            referenceblackwhite[i] = g_ntohl (referenceblackwhite[i]);
            referenceblackwhite[i + 1] = g_ntohl (referenceblackwhite[i + 1]);
        }
    }

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("ReferenceBlackWhite footroom"));

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        description_message = g_strdup_printf ("%u/%u,%u/%u,%u/%u",
                referenceblackwhite[0] / referenceblackwhite[1], referenceblackwhite[2] / referenceblackwhite[3],
                referenceblackwhite[4] / referenceblackwhite[5], referenceblackwhite[6] / referenceblackwhite[7],
                referenceblackwhite[8] / referenceblackwhite[9], referenceblackwhite[10] / referenceblackwhite[11]);
    }
    analyzer_utils_describe_tooltip (file, "ReferenceBlackWhite", description_message,
                                     _("Headroom and footroom pairs for each component"));
    g_free (description_message);
}

/*
 * Exif RATIONAL tags
 */

void
analyze_exposuretime_tag (AnalyzerFile *file,
                          AnalyzerTab *exif_tab,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian,
                          GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 exposuretime[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExposureTime"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&exposuretime[0], file, 4);
    read_success = analyzer_utils_read (&exposuretime[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "ExposureTime");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "ExposureTime");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            exposuretime[0] = g_ntohl (exposuretime[0]);
            exposuretime[1] = g_ntohl (exposuretime[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) exposuretime[0] / exposuretime[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "ExposureTime", description_message,
                                         _("Exposure time, given in seconds"));
    g_free (description_message);
}

void
analyze_fnumber_tag (AnalyzerFile *file,
                     AnalyzerTab *exif_tab,
                     guint16 field_type,
                     guint32 count,
                     guint32 value_offset,
                     gboolean is_little_endian,
                     GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 fnumber[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FNumber"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&fnumber[0], file, 4);
    read_success = analyzer_utils_read (&fnumber[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "FNumber");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "FNumber");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            fnumber[0] = g_ntohl (fnumber[0]);
            fnumber[1] = g_ntohl (fnumber[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) fnumber[0] / fnumber[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "FNumber", description_message,
                                         _("The focal ratio"));
    g_free (description_message);
}

void
analyze_compressedbitsperpixel_tag (AnalyzerFile *file,
                                    AnalyzerTab *exif_tab,
                                    guint16 field_type,
                                    guint32 count,
                                    guint32 value_offset,
                                    gboolean is_little_endian,
                                    GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 compressedbitsperpixel[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: CompressedBitsPerPixel"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&compressedbitsperpixel[0], file, 4);
    read_success = analyzer_utils_read (&compressedbitsperpixel[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "CompressedBitsPerPixel");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "CompressedBitsPerPixel");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            compressedbitsperpixel[0] = g_ntohl (compressedbitsperpixel[0]);
            compressedbitsperpixel[1] = g_ntohl (compressedbitsperpixel[1]);
        }

        description_message = g_strdup_printf ("%f",
                              (gfloat) compressedbitsperpixel[0] / compressedbitsperpixel[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "CompressedBitsPerPixel", description_message,
                                         _("Information specific to compressed data"));
    g_free (description_message);
}

void
analyze_shutterspeedvalue_tag (AnalyzerFile *file,
                               AnalyzerTab *exif_tab,
                               guint16 field_type,
                               guint32 count,
                               guint32 value_offset,
                               gboolean is_little_endian,
                               GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    gint32 shutterspeed[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ShutterSpeedValue"));

    value_offset = common_rational_tag_structure (file, field_type, 10, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&shutterspeed[0], file, 4);
    read_success = analyzer_utils_read (&shutterspeed[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "ShutterSpeedValue");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "ShutterSpeedValue");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            shutterspeed[0] = g_ntohl (shutterspeed[0]);
            shutterspeed[1] = g_ntohl (shutterspeed[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) shutterspeed[0] / shutterspeed[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "ShutterSpeedValue", description_message,
                _("The shutter speed, using the APEX (Additive System of Photographic Exposure) value"));
    g_free (description_message);
}

void
analyze_aperturevalue_tag (AnalyzerFile *file,
                           AnalyzerTab *exif_tab,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian,
                           GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 aperture[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ApertureValue"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&aperture[0], file, 4);
    read_success = analyzer_utils_read (&aperture[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "ApertureValue");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "ApertureValue");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            aperture[0] = g_ntohl (aperture[0]);
            aperture[1] = g_ntohl (aperture[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) aperture[0] / aperture[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "ApertureValue", description_message,
                 _("The lens aperture, using the APEX (Additive System of Photographic Exposure) value"));
    g_free (description_message);
}

void
analyze_brightnessvalue_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian,
                             GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    gint32 exposurebias[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: BrightnessValue"));

    value_offset = common_rational_tag_structure (file, field_type, 10, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&exposurebias[0], file, 4);
    read_success = analyzer_utils_read (&exposurebias[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "BrightnessValue");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "BrightnessValue");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            exposurebias[0] = g_ntohl (exposurebias[0]);
            exposurebias[1] = g_ntohl (exposurebias[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) exposurebias[0] / exposurebias[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "BrightnessValue", description_message,
                _("The brigthness value, using the APEX (Additive System of Photographic Exposure) value"));
    g_free (description_message);
}

void
analyze_exposurebiasvalue_tag (AnalyzerFile *file,
                               AnalyzerTab *exif_tab,
                               guint16 field_type,
                               guint32 count,
                               guint32 value_offset,
                               gboolean is_little_endian,
                               GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    gint32 exposurebias[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExposureBiasValue"));

    value_offset = common_rational_tag_structure (file, field_type, 10, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&exposurebias[0], file, 4);
    read_success = analyzer_utils_read (&exposurebias[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "ExposureBiasValue");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "ExposureBiasValue");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            exposurebias[0] = g_ntohl (exposurebias[0]);
            exposurebias[1] = g_ntohl (exposurebias[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) exposurebias[0] / exposurebias[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "ExposureBiasValue", description_message,
                _("The exposure bias, using the APEX (Additive System of Photographic Exposure) value"));
    g_free (description_message);
}

void
analyze_maxaperturevalue_tag (AnalyzerFile *file,
                              AnalyzerTab *exif_tab,
                              guint16 field_type,
                              guint32 count,
                              guint32 value_offset,
                              gboolean is_little_endian,
                              GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 maxaperture[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: MaxApertureValue"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&maxaperture[0], file, 4);
    read_success = analyzer_utils_read (&maxaperture[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "MaxApertureValue");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "MaxApertureValue");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            maxaperture[0] = g_ntohl (maxaperture[0]);
            maxaperture[1] = g_ntohl (maxaperture[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) maxaperture[0] / maxaperture[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "MaxApertureValue", description_message,
                _("The smallest F number of the lens, using the APEX (Additive System of Photographic Exposure) value"));
    g_free (description_message);
}

void
analyze_subjectdistance_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian,
                             GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 subjectdistance[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SubjectDistance"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&subjectdistance[0], file, 4);
    read_success = analyzer_utils_read (&subjectdistance[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "SubjectDistance");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "SubjectDistance");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            subjectdistance[0] = g_ntohl (subjectdistance[0]);
            subjectdistance[1] = g_ntohl (subjectdistance[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) subjectdistance[0] / subjectdistance[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "SubjectDistance", description_message,
                                         _("The distance to the subject, in meters"));
    g_free (description_message);
}

void
analyze_focallength_tag (AnalyzerFile *file,
                         AnalyzerTab *exif_tab,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian,
                         GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 focallength[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FocalLength"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&focallength[0], file, 4);
    read_success = analyzer_utils_read (&focallength[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "FocalLength");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "FocalLength");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            focallength[0] = g_ntohl (focallength[0]);
            focallength[1] = g_ntohl (focallength[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) focallength[0] / focallength[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "FocalLength", description_message,
                                         _("Focal length of the lens, in milimeters"));
    g_free (description_message);
}

void
analyze_exposureindex_tag (AnalyzerFile *file,
                           AnalyzerTab *exif_tab,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian,
                           GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 exposureindex[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExposureIndex"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&exposureindex[0], file, 4);
    read_success = analyzer_utils_read (&exposureindex[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "ExposureIndex");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "ExposureIndex");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            exposureindex[0] = g_ntohl (exposureindex[0]);
            exposureindex[1] = g_ntohl (exposureindex[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) exposureindex[0] / exposureindex[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "ExposureIndex", description_message,
                         _("Indicates the exposure index selected on the camera or input device"));
    g_free (description_message);
}


void
analyze_digitalzoomratio_tag (AnalyzerFile *file,
                              AnalyzerTab *exif_tab,
                              guint16 field_type,
                              guint32 count,
                              guint32 value_offset,
                              gboolean is_little_endian,
                              GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 digitalzoomratio[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: DigitalZoomRatio"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&digitalzoomratio[0], file, 4);
    read_success = analyzer_utils_read (&digitalzoomratio[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "DigitalZoomRatio");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "DigitalZoomRatio");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            digitalzoomratio[0] = g_ntohl (digitalzoomratio[0]);
            digitalzoomratio[1] = g_ntohl (digitalzoomratio[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) digitalzoomratio[0] / digitalzoomratio[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "DigitalZoomRatio", description_message,
                                         _("Digital zoom ratio when the image was shot"));
    g_free (description_message);
}

void
analyze_lensspecification_tag (AnalyzerFile *file,
                               AnalyzerTab *exif_tab,
                               guint16 field_type,
                               guint32 count,
                               guint32 value_offset,
                               gboolean is_little_endian,
                               GSList **tagged_bytes)
{
    gchar *lens_specification[] = {
        _("Minimum focal length"),
        _("Maximum focal length"),
        _("Minimum F number in the minimum focal length"),
        _("Minimum F number in the maximum focal length")
    };

    gboolean read_success;
    gsize save_pointer;
    GString *description_message;
    guint32 speficication[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: LensSpecification"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 4,
                                                  value_offset, is_little_endian);

    description_message = g_string_new (NULL);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < 4; i++)
    {
        analyzer_utils_read (&speficication[0], file, 4);
        read_success = analyzer_utils_read (&speficication[1], file, 4);

        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, lens_specification[i]);

        description_message = g_string_append (description_message, lens_specification[i]);

        if (!read_success)
        {
            g_string_append_printf (description_message, ": %s",
                                    _("<span foreground=\"red\">INVALID OFFSET</span>"));
            break;
        }

        if (!is_little_endian)
        {
            speficication[0] = g_ntohl (speficication[0]);
            speficication[1] = g_ntohl (speficication[1]);
        }

        if (!speficication[1])
            g_string_append_printf (description_message, ": %u/0", speficication[0]);
        else
            g_string_append_printf (description_message, ": %f",
                                    (gfloat) speficication[0] / speficication[1]);

        if (i != 3)
            description_message = g_string_append_c (description_message, '\n');
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "LensSpecification", description_message->str,
                         _("Minimum and maximum focal length in milimeters, unknown minimum F number = 0/0"));
    g_string_free (description_message, TRUE);

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, lens_specification[0]);

    SET_POINTER (file, save_pointer);
}

void
analyze_gamma_tag (AnalyzerFile *file,
                   AnalyzerTab *exif_tab,
                   guint16 field_type,
                   guint32 count,
                   guint32 value_offset,
                   gboolean is_little_endian,
                   GSList **tagged_bytes)
{
    gboolean read_success;
    gchar *description_message;
    gsize save_pointer;
    guint32 gamma[2];

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Gamma"));

    value_offset = common_rational_tag_structure (file, field_type, 5, count, 1,
                                                  value_offset, is_little_endian);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_read (&gamma[0], file, 4);
    read_success = analyzer_utils_read (&gamma[1], file, 4);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 8, "Gamma");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "Gamma");

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (!is_little_endian)
        {
            gamma[0] = g_ntohl (gamma[0]);
            gamma[1] = g_ntohl (gamma[1]);
        }

        description_message = g_strdup_printf ("%f", (gfloat) gamma[0] / gamma[1]);
    }
    else
    {
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID OFFSET</span>"));
    }
    analyzer_utils_describe_tooltip_tab (exif_tab, "Gamma", description_message,
                                         _("Value of coefficient gamma"));
    g_free (description_message);
}
