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


guint32
common_short_tag_structure (AnalyzerFile *file,
                            guint16 field_type,
                            guint32 count,
                            guint32 expected_count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    guint16 short_value1, short_value2;

    if (field_type == 3) // SHORT
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: SHORT"));
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

    if ((count << 1) > 4)
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));
    else
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));

    if (!is_little_endian && expected_count == 1)
    {
        value_offset = g_ntohs (value_offset);
    }
    else if (!is_little_endian && expected_count == 2)
    {
        short_value1 = g_ntohs (value_offset);
        short_value2 = g_ntohs (value_offset >> 16);
        value_offset = (short_value2 << 16) & short_value1;
    }

    return value_offset;
}

/*
 * Standard TIFF SHORT tags
 */

void
analyze_subfiletype_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SubfileType"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Full-resolution image data");
        break;
        case 2:
            description_message = _("Reduced-resolution image data");
        break;
        case 3:
            description_message = _("A single page of a multi-page image");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "SubfileType", description_message,
                 _("SubfileType\n"
                 "<tt>00 01<sub>16</sub></tt>\tFull-resolution image data\n"
                 "<tt>00 02<sub>16</sub></tt>\tReduced-resolution image data\n"
                 "<tt>00 03<sub>16</sub></tt>\tA single page of a multi-page image"));
}

void
analyze_bitspersample_tag (AnalyzerFile *file,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian,
                           GSList **tagged_bytes)
{
    gsize save_pointer;
    GString *description_message;
    guint16 bits;

    guint i;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: BitsPerSample"));

    /* This is a variable count tag, so the tag value might actually be an offset
     * Ignore the returned value */
    common_short_tag_structure (file, field_type, count, 0,
                                0, is_little_endian);

    description_message = g_string_new (NULL);

    if ((count << 1) <= 4)
    {
        for (i = 0; i < count; i++)
        {
            if (i % 2)
                bits = value_offset >> 16;
            else
                bits = value_offset;

            if (!is_little_endian)
                bits = g_ntohs (bits);

            if (i == 0)
                g_string_append_printf (description_message, "%u", bits);
            else
                g_string_append_printf (description_message, ",%u", bits);
        }
    }
    else
    {
        save_pointer = GET_POINTER (file);

        if (!is_little_endian)
            value_offset = g_ntohl (value_offset);

        SET_POINTER (file, value_offset);
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        for (i = 0; i < count; i++)
        {
            if (!analyzer_utils_read (&bits, file, 2))
            {
                description_message = g_string_assign (description_message, _("<span foreground=\"red\">INVALID OFFSET</span>"));
                break;
            }

            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 2, "BitsPerSample");

            if (!is_little_endian)
                bits = g_ntohs (bits);

            if (i == 0)
                g_string_append_printf (description_message, "%u", bits);
            else
                g_string_append_printf (description_message, ",%u", bits);
        }

        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        SET_POINTER (file, value_offset);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "BitsPerSample");

        SET_POINTER (file, save_pointer);
    }
    analyzer_utils_describe_tooltip (file, "BitsPerSample", description_message->str,
                                     _("Bit depth for each component"));

    g_string_free (description_message, TRUE);
}

void
analyze_compression_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Compression"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("No compression");
        break;
        case 2:
            description_message = _("CCITT Group 3 1-Dimensional Modified Huffman RLE");
        break;
        case 3:
            description_message = _("CCITT T.4 bi-level encoding");
        break;
        case 4:
            description_message = _("CCITT T.6 bi-level encoding");
        break;
        case 5:
            description_message = "Lempel-Ziv-Welch (LZW)";
        break;
        case 6:
            description_message = _("JPEG (TIFF 6.0 - obsolete)");
        break;
        case 7:
            description_message = "JPEG";
        break;
        case 8:
            description_message = _("zlib-format DEFLATE");
        break;
        case 0x8005:
            description_message = _("PackBits compression");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "Compression", description_message,
                                     _("Compression\n"
                                     "<tt>00 01<sub>16</sub></tt>\tNo compression\n"
                                     "<tt>00 02<sub>16</sub></tt>\tCCITT Group 3 1-Dimensional Modified Huffman RLE\n"
                                     "<tt>00 03<sub>16</sub></tt>\tCCITT T.4 bi-level encoding\n"
                                     "<tt>00 04<sub>16</sub></tt>\tCCITT T.6 bi-level encoding\n"
                                     "<tt>00 05<sub>16</sub></tt>\tLempel-Ziv-Welch (LZW)\n"
                                     "<tt>00 06<sub>16</sub></tt>\tJPEG (TIFF 6.0 - obsolete)\n"
                                     "<tt>00 07<sub>16</sub></tt>\tJPEG\n"
                                     "<tt>00 08<sub>16</sub></tt>\tzlib-format DEFLATE\n"
                                     "<tt>80 05<sub>16</sub></tt>\tPackBits compression"));
}

void
analyze_photometricinterpretation_tag (AnalyzerFile *file,
                                       guint16 field_type,
                                       guint32 count,
                                       guint32 value_offset,
                                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PhotometricInterpretation"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("WhiteIsZero");
        break;
        case 1:
            description_message = _("BlackIsZero");
        break;
        case 2:
            description_message = _("RGB");
        break;
        case 3:
            description_message = _("Palette color");
        break;
        case 4:
            description_message = _("Transparency mask");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "PhotometricInterpretation", description_message,
                                     _("PhotometricInterpretation\n"
                                     "<tt>00 00<sub>16</sub></tt>\tWhiteIsZero\n"
                                     "<tt>00 01<sub>16</sub></tt>\tBlackIsZero\n"
                                     "<tt>00 02<sub>16</sub></tt>\tRGB\n"
                                     "<tt>00 03<sub>16</sub></tt>\tPalette color\n"
                                     "<tt>00 04<sub>16</sub></tt>\tTransparency mask"));
}

void
analyze_threshholding_tag (AnalyzerFile *file,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Threshholding"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("No dithering or halftoning");
        break;
        case 2:
            description_message = _("Ordered dither or halftone technique");
        break;
        case 3:
            description_message = _("Randomized process");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "Threshholding", description_message,
                 _("Threshholding\n"
                 "<tt>00 01<sub>16</sub></tt>\tNo dithering or halftoning has been applied to the image data\n"
                 "<tt>00 02<sub>16</sub></tt>\tAn ordered dither or halftone technique has been applied to the image data\n"
                 "<tt>00 03<sub>16</sub></tt>\tA randomized process such as error diffusion has been applied to the image data"));
}

void
analyze_cellwidth_tag (AnalyzerFile *file,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: CellWidth"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip (file, "CellWidth", description_message,
                                     _("The width of the dithering or halftoning matrix"));
    g_free (description_message);
}

void
analyze_celllength_tag (AnalyzerFile *file,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: CellLength"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip (file, "CellLength", description_message,
                                     _("The length of the dithering or halftoning matrix"));
    g_free (description_message);
}

void
analyze_fillorder_tag (AnalyzerFile *file,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FillOrder"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Higher-order bits");
        break;
        case 2:
            description_message = _("Lower-order bits");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "FillOrder", description_message,
                 _("FillOrder\n"
                 "<tt>00 01<sub>16</sub></tt>\tPixels with lower column values are stored in the higher-order bits of the byte\n"
                 "<tt>00 02<sub>16</sub></tt>\tPixels with lower column values are stored in the lower-order bits of the byte"));
}

void
analyze_orientation_tag (AnalyzerFile *file,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Orientation"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Top-left");
        break;
        case 2:
            description_message = _("Top-right");
        break;
        case 3:
            description_message = _("Bottom-right");
        break;
        case 4:
            description_message = _("Bottom-left");
        break;
        case 5:
            description_message = _("Left-top");
        break;
        case 6:
            description_message = _("Right-top");
        break;
        case 7:
            description_message = _("Right-bottom");
        break;
        case 8:
            description_message = _("Left-bottom");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "Orientation", description_message,
                                     _("Orientation\n"
                                     "<tt>00 01<sub>16</sub></tt>\tTop-left\n"
                                     "<tt>00 02<sub>16</sub></tt>\tTop-right\n"
                                     "<tt>00 03<sub>16</sub></tt>\tBottom-right\n"
                                     "<tt>00 04<sub>16</sub></tt>\tBottom-left\n"
                                     "<tt>00 05<sub>16</sub></tt>\tLeft-top\n"
                                     "<tt>00 06<sub>16</sub></tt>\tRight-top\n"
                                     "<tt>00 07<sub>16</sub></tt>\tRight-bottom\n"
                                     "<tt>00 08<sub>16</sub></tt>\tLeft-bottom"));
}

void
analyze_samplesperpixel_tag (AnalyzerFile *file,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SamplesPerPixel"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip (file, "SamplesPerPixel", description_message,
                                     _("Number of components per pixel"));
    g_free (description_message);
}

void
analyze_minsamplevalue_tag (AnalyzerFile *file,
                            guint16 field_type,
                            guint32 count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: MinSampleValue"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip (file, "MinSampleValue", description_message,
                                     _("The minimum component value used"));
    g_free (description_message);
}

void
analyze_maxsamplevalue_tag (AnalyzerFile *file,
                            guint16 field_type,
                            guint32 count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: MaxSampleValue"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset);
    analyzer_utils_describe_tooltip (file, "MaxSampleValue", description_message,
                                     _("The maximum component value used"));
    g_free (description_message);
}

void
analyze_planarconfiguration_tag (AnalyzerFile *file,
                                 guint16 field_type,
                                 guint32 count,
                                 guint32 value_offset,
                                 gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PlanarConfiguration"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Chunky format");
        break;
        case 2:
            description_message = _("Planar format");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "PlanarConfiguration", description_message,
         _("PlanarConfiguration\n"
         "<tt>00 01<sub>16</sub></tt>\tChunky format (components are stored contiguously)\n"
         "<tt>00 02<sub>16</sub></tt>\tPlanar format (components are stored in separate component planes)"));
}

void
analyze_resolutionunit_tag (AnalyzerFile *file,
                            guint16 field_type,
                            guint32 count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ResolutionUnit"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("No unit");
        break;
        case 2:
            description_message = _("Inch");
        break;
        case 3:
            description_message = _("Centimeter");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "ResolutionUnit", description_message,
                                     _("ResolutionUnit\n"
                                     "<tt>00 01<sub>16</sub></tt>\tNo unit\n"
                                     "<tt>00 02<sub>16</sub></tt>\tInch\n"
                                     "<tt>00 03<sub>16</sub></tt>\tCentimeter"));
}

void
analyze_pagenumber_tag (AnalyzerFile *file,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PageNumber"));

    value_offset = common_short_tag_structure (file, field_type, count, 2,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u/%u", value_offset & 0x0000FFFF, value_offset >> 16);
    analyzer_utils_describe_tooltip (file, "PageNumber", description_message,
                                     _("Page number / Total pages\n"
                                     "Page numbers start at 0\n"
                                     "Total = 0 if it is unknown"));

    g_free (description_message);
}

void
analyze_predictor_tag (AnalyzerFile *file,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Predictor"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("No prediction scheme used before coding");
        break;
        case 2:
            description_message = _("Horizontal differencing");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "Predictor", description_message,
                                     _("Predictor\n"
                                     "<tt>00 01<sub>16</sub></tt>\tNo prediction scheme used before coding\n"
                                     "<tt>00 02<sub>16</sub></tt>\tHorizontal differencing"));
}

void
analyze_ycbcrsubsampling_tag (AnalyzerFile *file,
                              guint16 field_type,
                              guint32 count,
                              guint32 value_offset,
                              gboolean is_little_endian)
{
    GString *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: YCbCrSubSampling"));

    value_offset = common_short_tag_structure (file, field_type, count, 2,
                                               value_offset, is_little_endian);

    description_message = g_string_new ("YCbCrSubsampleHoriz: ");
    switch (value_offset & 0x0000FFFF)
    {
        case 1:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageWidth = Luma image ImageWidth"));
        break;
        case 2:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageWidth = 1/2 Luma image ImageWidth"));
        break;
        case 4:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageWidth = 1/4 Luma image ImageWidth"));
        break;
        default:
            description_message = g_string_append (description_message,
                                                   _("<span foreground=\"red\">INVALID</span>"));
    }

    g_string_append (description_message, "\nYCbCrSubsampleVert: ");
    switch (value_offset >> 16)
    {
        case 1:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageLength = Luma image ImageLength"));
        break;
        case 2:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageLength = 1/2 Luma image ImageLength"));
        break;
        case 4:
            description_message = g_string_append (description_message,
                                                   _("Chroma image ImageLength = 1/4 Luma image ImageLength"));
        break;
        default:
            description_message = g_string_append (description_message,
                                                   _("<span foreground=\"red\">INVALID</span>"));
    }

    analyzer_utils_describe_tooltip (file, "YCbCrSubSampling", description_message->str,
                 _("YCbCrSubSamplingHoriz\n"
                 "<tt>00 01<sub>16</sub></tt>\tChroma image ImageWidth = Luma image ImageWidth\n"
                 "<tt>00 02<sub>16</sub></tt>\tChroma image ImageWidth = 1/2 Luma image ImageWidth\n"
                 "<tt>00 04<sub>16</sub></tt>\tChroma image ImageWidth = 1/4 Luma image ImageWidth\n"
                 "YCbCrSubSamplingVert\n"
                 "<tt>00 01<sub>16</sub></tt>\tChroma image ImageLength = Luma image ImageLength\n"
                 "<tt>00 02<sub>16</sub></tt>\tChroma image ImageLength = 1/2 Luma image ImageLength\n"
                 "<tt>00 04<sub>16</sub></tt>\tChroma image ImageLength = 1/4 Luma image ImageLength"));

    g_string_free (description_message, TRUE);
}

void
analyze_ycbcrpositioning_tag (AnalyzerFile *file,
                              guint16 field_type,
                              guint32 count,
                              guint32 value_offset,
                              gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: YCbCrPositioning"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Centered");
        break;
        case 2:
            description_message = _("Cosited");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip (file, "YCbCrPositioning", description_message,
                 _("YCbCrPositioning\n"
                 "<tt>00 01<sub>16</sub></tt>\tCentered\n"
                 "<tt>00 02<sub>16</sub></tt>\tCosited"));
}

/*
 * Exif SHORT tags
 */

void
analyze_exposureprogram_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExposureProgram"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Not defined");
        break;
        case 1:
            description_message = _("Manual");
        break;
        case 2:
            description_message = _("Normal program");
        break;
        case 3:
            description_message = _("Aperture priority");
        break;
        case 4:
            description_message = _("Shutter priority");
        break;
        case 5:
            description_message = _("Creative program");
        break;
        case 6:
            description_message = _("Action program");
        break;
        case 7:
            description_message = _("Portrait mode");
        break;
        case 8:
            description_message = _("Landscape mode");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "ExposureProgram", description_message,
                                         _("ExposureProgram\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNot defined\n"
                                         "<tt>00 01<sub>16</sub></tt>\tManual\n"
                                         "<tt>00 02<sub>16</sub></tt>\tNormal program\n"
                                         "<tt>00 03<sub>16</sub></tt>\tAperture priority\n"
                                         "<tt>00 04<sub>16</sub></tt>\tShutter priority\n"
                                         "<tt>00 05<sub>16</sub></tt>\tCreative program\n"
                                         "<tt>00 06<sub>16</sub></tt>\tAction program\n"
                                         "<tt>00 07<sub>16</sub></tt>\tPortrait mode\n"
                                         "<tt>00 08<sub>16</sub></tt>\tLandscape mode"));
}

void
analyze_photographicsensitivity_tag (AnalyzerFile *file,
                                     AnalyzerTab *exif_tab,
                                     guint16 field_type,
                                     guint32 count,
                                     guint32 value_offset,
                                     gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: PhotographicSensitivity"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset & 0x0000FFFF);
    analyzer_utils_describe_tooltip_tab (exif_tab, "PhotographicSensitivity", description_message,
                                         _("Sensitivity of the camera or input device when the image was shot"));
    g_free (description_message);
}

void
analyze_sensitivitytype_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SensitivityType"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Unknown");
        break;
        case 1:
            description_message = _("Standard output sensitivity (SOS)");
        break;
        case 2:
            description_message = _("Recommended exposure index (REI)");
        break;
        case 3:
            description_message = _("ISO speed");
        break;
        case 4:
            description_message = _("SOS and REI");
        break;
        case 5:
            description_message = _("SOS and ISO speed");
        break;
        case 6:
            description_message = _("REI and ISO speed");
        break;
        case 7:
            description_message = _("SOS, REI and ISO speed");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "SensitivityType", description_message,
                                         _("SensitivityType\n"
                                         "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                                         "<tt>00 01<sub>16</sub></tt>\tStandard output sensitivity (SOS)\n"
                                         "<tt>00 02<sub>16</sub></tt>\tRecommended exposure index (REI)\n"
                                         "<tt>00 03<sub>16</sub></tt>\tISO speed\n"
                                         "<tt>00 04<sub>16</sub></tt>\tSOS and REI\n"
                                         "<tt>00 05<sub>16</sub></tt>\tSOS and ISO speed\n"
                                         "<tt>00 06<sub>16</sub></tt>\tREI and ISO speed\n"
                                         "<tt>00 07<sub>16</sub></tt>\tSOS, REI and ISO speed"));
}

void
analyze_meteringmode_tag (AnalyzerFile *file,
                          AnalyzerTab *exif_tab,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: MeteringMode"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Unknown");
        break;
        case 1:
            description_message = _("Average");
        break;
        case 2:
            description_message = _("CenterWeightedAverage");
        break;
        case 3:
            description_message = _("Spot");
        break;
        case 4:
            description_message = _("MultiSpot");
        break;
        case 5:
            description_message = _("Pattern");
        break;
        case 6:
            description_message = _("Partial");
        break;
        case 255:
            description_message = _("Other");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "MeteringMode", description_message,
                                         _("MeteringMode\n"
                                         "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                                         "<tt>00 01<sub>16</sub></tt>\tAverage\n"
                                         "<tt>00 02<sub>16</sub></tt>\tCenterWeightedAverage\n"
                                         "<tt>00 03<sub>16</sub></tt>\tSpot\n"
                                         "<tt>00 04<sub>16</sub></tt>\tMultiSpot\n"
                                         "<tt>00 05<sub>16</sub></tt>\tPattern\n"
                                         "<tt>00 06<sub>16</sub></tt>\tPartial\n"
                                         "<tt>00 FF<sub>16</sub></tt>\tOther"));
}

void
analyze_lightsource_tag (AnalyzerFile *file,
                         AnalyzerTab *exif_tab,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: LightSource"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Unknown");
        break;
        case 1:
            description_message = _("Daylight");
        break;
        case 2:
            description_message = _("Fluorescent");
        break;
        case 3:
            description_message = _("Tungsten (incandescent light)");
        break;
        case 4:
            description_message = _("Flash");
        break;
        case 9:
            description_message = _("Fine weather");
        break;
        case 10:
            description_message = _("Cloudy weather");
        break;
        case 11:
            description_message = _("Shade");
        break;
        case 12:
            description_message = _("Daylight fluorescent (D 5700 - 7100K)");
        break;
        case 13:
            description_message = _("Day white fluorescent (N 4600 - 5500K)");
        break;
        case 14:
            description_message = _("Cool white fluorescent (W 3800 - 4500K)");
        break;
        case 15:
            description_message = _("White fluorescent (WW 3250 - 3800K)");
        break;
        case 16:
            description_message = _("Warm white fluorescent (L 2600 - 3250K)");
        break;
        case 17:
            description_message = _("Standard light A");
        break;
        case 18:
            description_message = _("Standard light B");
        break;
        case 19:
            description_message = _("Standard light C");
        break;
        case 20:
            description_message = "D55";
        break;
        case 21:
            description_message = "D65";
        break;
        case 22:
            description_message = "D75";
        break;
        case 23:
            description_message = "D50";
        break;
        case 24:
            description_message = _("ISO studio tungsten");
        break;
        case 255:
            description_message = _("Other");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "LightSource", description_message,
                                         _("LightSource\n"
                                         "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                                         "<tt>00 01<sub>16</sub></tt>\tDaylight\n"
                                         "<tt>00 02<sub>16</sub></tt>\tFluorescent\n"
                                         "<tt>00 03<sub>16</sub></tt>\tTungsten (incandescent light)\n"
                                         "<tt>00 04<sub>16</sub></tt>\tFlash\n"
                                         "<tt>00 09<sub>16</sub></tt>\tFine weather\n"
                                         "<tt>00 0A<sub>16</sub></tt>\tCloudy weather\n"
                                         "<tt>00 0B<sub>16</sub></tt>\tShade\n"
                                         "<tt>00 0C<sub>16</sub></tt>\tDaylight fluorescent (D 5700 - 7100K)\n"
                                         "<tt>00 0D<sub>16</sub></tt>\tDay white fluorescent (N 4600 - 5500K)\n"
                                         "<tt>00 0E<sub>16</sub></tt>\tCool white fluorescent (W 3800 - 4500K)\n"
                                         "<tt>00 0F<sub>16</sub></tt>\tWhite fluorescent (WW 3250 - 3800K)\n"
                                         "<tt>00 10<sub>16</sub></tt>\tWarm white fluorescent (L 2600 - 3250K)\n"
                                         "<tt>00 11<sub>16</sub></tt>\tStandard light A\n"
                                         "<tt>00 12<sub>16</sub></tt>\tStandard light B\n"
                                         "<tt>00 13<sub>16</sub></tt>\tStandard light C\n"
                                         "<tt>00 14<sub>16</sub></tt>\tD55\n"
                                         "<tt>00 15<sub>16</sub></tt>\tD65\n"
                                         "<tt>00 16<sub>16</sub></tt>\tD75\n"
                                         "<tt>00 17<sub>16</sub></tt>\tD50\n"
                                         "<tt>00 18<sub>16</sub></tt>\tISO studio tungsten\n"
                                         "<tt>00 FF<sub>16</sub></tt>\tOther"));
}

void
analyze_flash_tag (AnalyzerFile *file,
                   AnalyzerTab *exif_tab,
                   guint16 field_type,
                   guint32 count,
                   guint32 value_offset,
                   gboolean is_little_endian)
{
    GString *description_message;
    guint16 flash_field;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Flash"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_string_new (NULL);

    flash_field = value_offset & 0x1;
    if (!flash_field)
        g_string_append_printf (description_message, "%s\n", _("Flash did not fire"));
    else
        g_string_append_printf (description_message, "%s\n", _("Flash fired"));

    flash_field = (value_offset >> 1) & 0x3;
    switch (flash_field)
    {
        case 0:
            g_string_append_printf (description_message, "%s\n", _("No strobe return detection function"));
        break;
        case 1:
            g_string_append_printf (description_message, "%s\n", _("<span foreground=\"red\">INVALID</span>"));
        break;
        case 2:
            g_string_append_printf (description_message, "%s\n", _("Strobe return light not detected"));
        break;
        case 3:
            g_string_append_printf (description_message, "%s\n", _("Strobe return light detected"));
        break;
    }

    flash_field = (value_offset >> 3) & 0x3;
    switch (flash_field)
    {
        case 0:
            g_string_append_printf (description_message, "%s\n", _("Unknown"));
        break;
        case 1:
            g_string_append_printf (description_message, "%s\n", _("Compulsory flash firing"));
        break;
        case 2:
            g_string_append_printf (description_message, "%s\n", _("Compulsory flash suppression"));
        break;
        case 3:
            g_string_append_printf (description_message, "%s\n", _("Auto mode"));
        break;
    }

    flash_field = (value_offset >> 5) & 0x1;
    if (!flash_field)
        g_string_append_printf (description_message, "%s\n", _("Flash function present"));
    else
        g_string_append_printf (description_message, "%s\n", _("No flash function"));

    flash_field = (value_offset >> 6) & 0x1;
    if (!flash_field)
        g_string_append_printf (description_message, "%s", _("No red-eye reduction mode or unknown"));
    else
        g_string_append_printf (description_message, "%s", _("Red-eye reduction supported"));

    analyzer_utils_describe_tooltip_tab (exif_tab, "Flash", description_message->str,
                                         _("Flash bit fields\n"
                                         "Bit 0\n"
                                         "<tt>0<sub>2</sub></tt>\tFlash did not fire\n"
                                         "<tt>1<sub>2</sub></tt>\tFlash fired\n"
                                         "Bits 1 and 2\n"
                                         "<tt>00<sub>2</sub></tt>\tNo strobe return detection function\n"
                                         "<tt>10<sub>2</sub></tt>\tStrobe return light not detected\n"
                                         "<tt>11<sub>2</sub></tt>\tStrobe return light detected\n"
                                         "Bits 3 and 4\n"
                                         "<tt>00<sub>2</sub></tt>\tUnknown\n"
                                         "<tt>01<sub>2</sub></tt>\tCompulsory flash firing\n"
                                         "<tt>10<sub>2</sub></tt>\tCompulsory flash suppression\n"
                                         "<tt>11<sub>2</sub></tt>\tAuto mode\n"
                                         "Bit 5\n"
                                         "<tt>0<sub>2</sub></tt>\tFlash function present\n"
                                         "<tt>1<sub>2</sub></tt>\tNo flash function\n"
                                         "Bit 6\n"
                                         "<tt>0<sub>2</sub></tt>\tNo red-eye reduction mode or unknown\n"
                                         "<tt>1<sub>2</sub></tt>\tRed-eye reduction supported"));

    g_string_free (description_message, TRUE);
}

void
analyze_colorspace_tag (AnalyzerFile *file,
                        AnalyzerTab *exif_tab,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ColorSpace"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("sRGB");
        break;
        case 0xFFFF:
            description_message = _("Uncalibrated");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "ColorSpace", description_message,
                                         _("ColorSpace\n"
                                         "<tt>00 01<sub>16</sub></tt>\tsRGB\n"
                                         "<tt>FF FF<sub>16</sub></tt>\tUncalibrated"));
}

void
analyze_focalplaneresolutionunit_tag (AnalyzerFile *file,
                                      AnalyzerTab *exif_tab,
                                      guint16 field_type,
                                      guint32 count,
                                      guint32 value_offset,
                                      gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FocalPlaneResolutionUnit"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("No unit");
        break;
        case 2:
            description_message = _("Inch");
        break;
        case 3:
            description_message = _("Centimeter");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "FocalPlaneResolutionUnit", description_message,
                                         _("FocalPlaneResolutionUnit\n"
                                         "<tt>00 01<sub>16</sub></tt>\tNo unit\n"
                                         "<tt>00 02<sub>16</sub></tt>\tInch\n"
                                         "<tt>00 03<sub>16</sub></tt>\tCentimeter"));
}

void
analyze_subjectlocation_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SubjectLocation"));

    value_offset = common_short_tag_structure (file, field_type, count, 2,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u - %u", value_offset & 0x0000FFFF, value_offset >> 16);
    analyzer_utils_describe_tooltip_tab (exif_tab, "SubjectLocation", description_message,
                                         _("X column number - Y row number"));

    g_free (description_message);
}

void
analyze_sensingmethod_tag (AnalyzerFile *file,
                           AnalyzerTab *exif_tab,
                           guint16 field_type,
                           guint32 count,
                           guint32 value_offset,
                           gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SensingMethod"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("Not defined");
        break;
        case 2:
            description_message = _("One-chip color area sensor");
        break;
        case 3:
            description_message = _("Two-chip color area sensor");
        break;
        case 4:
            description_message = _("Three-chip color area sensor");
        break;
        case 5:
            description_message = _("Color sequential area sensor");
        break;
        case 7:
            description_message = _("Trilinear sensor");
        break;
        case 8:
            description_message = _("Color sequential linear sensor");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "SensingMethod", description_message,
                                         _("SensingMethod\n"
                                         "<tt>00 01<sub>16</sub></tt>\tNot defined\n"
                                         "<tt>00 02<sub>16</sub></tt>\tOne-chip color area sensor\n"
                                         "<tt>00 03<sub>16</sub></tt>\tTwo-chip color area sensor\n"
                                         "<tt>00 04<sub>16</sub></tt>\tThree-chip color area sensor\n"
                                         "<tt>00 05<sub>16</sub></tt>\tColor sequential area sensor\n"
                                         "<tt>00 07<sub>16</sub></tt>\tTrilinear sensor\n"
                                         "<tt>00 08<sub>16</sub></tt>\tColor sequential linear sensor"));
}

void
analyze_customrendered_tag (AnalyzerFile *file,
                            AnalyzerTab *exif_tab,
                            guint16 field_type,
                            guint32 count,
                            guint32 value_offset,
                            gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: CustomRendered"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Normal process");
        break;
        case 1:
            description_message = _("Custom process");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "CustomRendered", description_message,
                                         _("CustomRendered\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNormal process\n"
                                         "<tt>00 01<sub>16</sub></tt>\tCustom process"));
}

void
analyze_exposuremode_tag (AnalyzerFile *file,
                          AnalyzerTab *exif_tab,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExposureMode"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Auto exposure");
        break;
        case 1:
            description_message = _("Manual exposure");
        break;
        case 2:
            description_message = _("Auto bracket");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "ExposureMode", description_message,
                                         _("ExposureMode\n"
                                         "<tt>00 00<sub>16</sub></tt>\tAuto exposure\n"
                                         "<tt>00 01<sub>16</sub></tt>\tManual exposure\n"
                                         "<tt>00 02<sub>16</sub></tt>\tAuto bracket"));
}

void
analyze_whitebalance_tag (AnalyzerFile *file,
                          AnalyzerTab *exif_tab,
                          guint16 field_type,
                          guint32 count,
                          guint32 value_offset,
                          gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: WhiteBalance"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Auto white balance");
        break;
        case 1:
            description_message = _("Manual white balance");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "WhiteBalance", description_message,
                                         _("WhiteBalance\n"
                                         "<tt>00 00<sub>16</sub></tt>\tAuto white balance\n"
                                         "<tt>00 01<sub>16</sub></tt>\tManual white balance"));
}

void
analyze_focallengthin35mmfilm_tag (AnalyzerFile *file,
                                   AnalyzerTab *exif_tab,
                                   guint16 field_type,
                                   guint32 count,
                                   guint32 value_offset,
                                   gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FocalLengthIn35mmFilm"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    description_message = g_strdup_printf ("%u", value_offset & 0x0000FFFF);
    analyzer_utils_describe_tooltip_tab (exif_tab, "FocalLengthIn35mmFilm", description_message,
                                         _("Focal length assuming a 35mm film camera, in milimeters"));
    g_free (description_message);
}

void
analyze_scenecapturetype_tag (AnalyzerFile *file,
                              AnalyzerTab *exif_tab,
                              guint16 field_type,
                              guint32 count,
                              guint32 value_offset,
                              gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SceneCaptureType"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Standard");
        break;
        case 1:
            description_message = _("Landscape");
        break;
        case 2:
            description_message = _("Portrait");
        break;
        case 3:
            description_message = _("Night scene");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "SceneCaptureType", description_message,
                                         _("SceneCaptureType\n"
                                         "<tt>00 00<sub>16</sub></tt>\tStandard\n"
                                         "<tt>00 01<sub>16</sub></tt>\tLandscape\n"
                                         "<tt>00 02<sub>16</sub></tt>\tPortrait\n"
                                         "<tt>00 03<sub>16</sub></tt>\tNight scene"));
}

void
analyze_gaincontrol_tag (AnalyzerFile *file,
                         AnalyzerTab *exif_tab,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: GainControl"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("None");
        break;
        case 1:
            description_message = _("Low gain up");
        break;
        case 2:
            description_message = _("High gain up");
        break;
        case 3:
            description_message = _("Low gain down");
        break;
        case 4:
            description_message = _("High gain down");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "GainControl", description_message,
                                         _("GainControl\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNone\n"
                                         "<tt>00 01<sub>16</sub></tt>\tLow gain up\n"
                                         "<tt>00 02<sub>16</sub></tt>\tHigh gain up\n"
                                         "<tt>00 02<sub>16</sub></tt>\tLow gain down\n"
                                         "<tt>00 03<sub>16</sub></tt>\tHigh gain down"));
}

void
analyze_contrast_tag (AnalyzerFile *file,
                      AnalyzerTab *exif_tab,
                      guint16 field_type,
                      guint32 count,
                      guint32 value_offset,
                      gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Contrast"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Normal");
        break;
        case 1:
            description_message = _("Soft");
        break;
        case 2:
            description_message = _("Hard");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "Contrast", description_message,
                                         _("Contrast\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                                         "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                                         "<tt>00 02<sub>16</sub></tt>\tHard"));
}

void
analyze_saturation_tag (AnalyzerFile *file,
                        AnalyzerTab *exif_tab,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Saturation"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Normal");
        break;
        case 1:
            description_message = _("Low saturation");
        break;
        case 2:
            description_message = _("High saturation");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "Saturation", description_message,
                                         _("Saturation\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                                         "<tt>00 01<sub>16</sub></tt>\tLow saturation\n"
                                         "<tt>00 02<sub>16</sub></tt>\tHigh saturation"));
}

void
analyze_sharpness_tag (AnalyzerFile *file,
                       AnalyzerTab *exif_tab,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: Sharpness"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Normal");
        break;
        case 1:
            description_message = _("Soft");
        break;
        case 2:
            description_message = _("Hard");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "Sharpness", description_message,
                                         _("Sharpness\n"
                                         "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                                         "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                                         "<tt>00 02<sub>16</sub></tt>\tHard"));
}

void
analyze_subjectdistancerange_tag (AnalyzerFile *file,
                                  AnalyzerTab *exif_tab,
                                  guint16 field_type,
                                  guint32 count,
                                  guint32 value_offset,
                                  gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SubjectDistanceRange"));

    value_offset = common_short_tag_structure (file, field_type, count, 1,
                                               value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Unknown");
        break;
        case 1:
            description_message = _("Macro");
        break;
        case 2:
            description_message = _("Close view");
        break;
        case 3:
            description_message = _("Distant view");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "SubjectDistanceRange", description_message,
                                         _("SubjectDistanceRange\n"
                                         "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                                         "<tt>00 01<sub>16</sub></tt>\tMacro\n"
                                         "<tt>00 02<sub>16</sub></tt>\tClose view\n"
                                         "<tt>00 03<sub>16</sub></tt>\tDistant view"));
}
