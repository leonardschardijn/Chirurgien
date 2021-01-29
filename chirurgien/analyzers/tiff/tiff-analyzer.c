/* tiff-analyzer.c
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

#include "tiff-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-tiff.h"


void
chirurgien_analyze_tiff (AnalyzerFile *file)
{
    const guint16 tiff_tags[TIFF_TAGS] =
    {
        0x00FE, // NewSubfileType
        0x00FF, // SubfileType
        0x0100, // ImageWidth
        0x0101, // ImageLength
        0x0102, // BitsPerSample
        0x0103, // Compression
        0x0106, // PhotometricInterpretation
        0x0107, // Threshholding
        0x0108, // CellWidth
        0x0109, // CellLength
        0x010A, // FillOrder
        0x010D, // DocumentName
        0x010E, // ImageDescription
        0x010F, // Make
        0x0110, // Model
        0x0111, // StipOffsets
        0x0112, // Orientation
        0x0115, // SamplesPerPixel
        0x0116, // RowsPerStrip
        0x0117, // StripByteCounts
        0x0118, // MinSampleValue
        0x0119, // MaxSampleValue
        0x011A, // XResolution
        0x011B, // YResolution
        0x011C, // PlanarConfiguration
        0x011D, // PageName
        0x011E, // XPosition
        0x011F, // YPosition
        0x0128, // ResolutionUnit
        0x0129, // PageNumber
        0x0131, // Software
        0x0132, // DateTime
        0x013B, // Artist
        0x013C, // HostComputer
        0x013D, // Predictor
        0x013E, // WhitePoint
        0x013F, // PrimaryChromaticities
        0x0201, // JPEGInterchangeFormat
        0x0202, // JPEGInterchangeFormatLength
        0x0211, // YCbCrCoefficients
        0x0212, // YCbCrSubSampling
        0x0213, // YCbCrPositioning
        0x0214, // ReferenceBlackWhite
        0x8298, // Copyright
        0x8769, // Exif IFD
        0x8825  // GPSInfo IFD
    };

    AnalyzerTab ascii_tab, exif_tab, exif_ascii_tab,
                exif_gpsinfo_tab, exif_gpsinfo_ascii_tab, jpeg_tab;

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    /* As many tags use offsets to point to their values, the tagged_bytes list
     * keeps track of all color tagged bytes, this let's us locate the stray bytes not referenced
     * by any tag */
    GSList *tagged_bytes = NULL, *index;

    guint strip_offsets_count = 0, strip_byte_counts_count = 0, i;
    g_autofree guint32 *strip_offsets = NULL;
    g_autofree guint32 *strip_byte_counts = NULL;

    guint unknown_tag_count = 0;
    guint32 jpeg_offset = 0, jpeg_length = 0;

    gboolean is_little_endian;

    analyzer_utils_set_title (file, "Tag Image File Format");

    analyzer_utils_init_tab (&ascii_tab);
    analyzer_utils_add_description_tab (&ascii_tab, _("<b>Standard TIFF ASCII tags</b>"), NULL, NULL, 0, 0);
    analyzer_utils_init_tab (&exif_tab);
    analyzer_utils_set_title_tab (&exif_tab, _("<b>Exif-specific tags</b>"));
    analyzer_utils_init_tab (&exif_ascii_tab);
    analyzer_utils_add_description_tab (&exif_ascii_tab, _("<b>Exif-specific ASCII tags</b>"), NULL, NULL, 0, 0);
    analyzer_utils_init_tab (&exif_gpsinfo_tab);
    analyzer_utils_set_title_tab (&exif_gpsinfo_tab, _("<b>Exif GPSInfo-specific tags</b>"));
    analyzer_utils_init_tab (&exif_gpsinfo_ascii_tab);
    analyzer_utils_add_description_tab (&exif_gpsinfo_ascii_tab, _("<b>Exif GPSInfo-specific ASCII tags</b>"), NULL, NULL, 0, 0);
    analyzer_utils_init_tab (&jpeg_tab);
    analyzer_utils_set_title_tab (&jpeg_tab, "<b>Joint Photographic Experts Group</b>");


    /* Use the tiff_tag variable because it is available */
    analyzer_utils_read (&tiff_tag, file, 2);
    if (tiff_tag == 0x4949)
    {
        is_little_endian = TRUE;
        analyzer_utils_describe_tooltip (file, _("Endianness"), _("Little-endian"),
                                         _("Endianness\n"
                                         "<tt>49 49<sub>16</sub></tt>\tLittle-endian\n"
                                         "<tt>4D 4D<sub>16</sub></tt>\tBig-endian"));
    }
    else
    {
        is_little_endian = FALSE;
        analyzer_utils_describe_tooltip (file, _("Endianness"), _("Big-endian"),
                                         _("Endianness\n"
                                         "<tt>49 49<sub>16</sub></tt>\tLittle-endian\n"
                                         "<tt>4D 4D<sub>16</sub></tt>\tBig-endian"));
    }

    analyzer_utils_set_subtitle (file, _("<b>Standard TIFF tags</b>"));

    analyzer_utils_tag_navigation (file, TIFF_TAG_COLOR, 2, _("Endianness"), _("End."));

    ADVANCE_POINTER (file, 2);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 2, _("TIFF identifier"));

    if (!analyzer_utils_read (&value_offset, file , 4))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
        return;
    }
    analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD offset"));

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    SET_POINTER (file, value_offset);
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    if (!analyzer_utils_read (&ifd_entries, file , 2))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
        g_slist_free (tagged_bytes);
        return;
    }
    analyzer_utils_tag_navigation (file, IFD_COLOR, 2, _("Number of directory entries"), "IFD");

    if (!is_little_endian)
        ifd_entries = g_ntohs (ifd_entries);
    ifd_entries++;

    /* Tag loop */
    while (ifd_entries)
    {
        /* Last entry
         * 0x00000000 = IFD end
         * Anything else = offset to next IFD */
        if (ifd_entries == 1)
        {
            if (!analyzer_utils_read (&count, file, 4))
                goto END_ERROR_LOOP;

            if (!count)
            {
                analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD end"));
                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
                break;
            }
            else
            {
                if (!is_little_endian)
                    count = g_ntohl (count);

                analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD offset"));
                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

                SET_POINTER (file, count);
                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

                if (!analyzer_utils_read (&ifd_entries, file , 2))
                    goto END_ERROR_LOOP;

                analyzer_utils_tag_navigation (file, IFD_COLOR, 2, _("Number of directory entries"), "IFD");

                if (!is_little_endian)
                    ifd_entries = g_ntohs (ifd_entries);
                ifd_entries++;
                continue;
            }
        }

        /* TIFF tag */
        if (!analyzer_utils_read (&tiff_tag, file, 2))
            goto END_ERROR_LOOP;

        /* Field type */
        if (!analyzer_utils_read (&field_type, file, 2))
            goto END_ERROR_LOOP;

        /* Count */
        if (!analyzer_utils_read (&count, file, 4))
            goto END_ERROR_LOOP;

        if (!is_little_endian)
        {
            tiff_tag = g_ntohs (tiff_tag);
            field_type = g_ntohs (field_type);
            count = g_ntohl (count);
        }

        /* Tag value or offset */
        if (!analyzer_utils_read (&value_offset, file, 4))
            goto END_ERROR_LOOP;

        /* Analyze tag data */
        if (tiff_tag == tiff_tags[NewSubfileType])
        {
            process_long_tag (file, NULL, _("Tag: NewSubfileType"), "NewSubfileType",
                              NULL, field_type,
                              LONG, count, 1, value_offset, is_little_endian,
                              NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SubfileType])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            gchar *value_description[] = {
                _("Full-resolution image data"),
                _("Reduced-resolution image data"),
                _("A single page of a multi-page image"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: SubfileType"), "SubfileType",
                   _("SubfileType\n"
                     "<tt>00 01<sub>16</sub></tt>\tFull-resolution image data\n"
                     "<tt>00 02<sub>16</sub></tt>\tReduced-resolution image data\n"
                     "<tt>00 03<sub>16</sub></tt>\tA single page of a multi-page image"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageWidth])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: ImageWidth"), "ImageWidth",
                                   _("Number of columns in the image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: ImageWidth"), "ImageWidth",
                                  _("Number of columns in the image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian,
                                  NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageLength])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: ImageLength"), "ImageLength",
                                   _("Number of rows in the image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: ImageLength"), "ImageLength",
                                  _("Number of rows in the image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian,
                                  NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[BitsPerSample])
        {
            process_short_tag (file, NULL, _("Tag: BitsPerSample"), "BitsPerSample",
                               _("Bit depth for each component"), field_type,
                               SHORT, count, 0, value_offset, is_little_endian,
                               0, NULL, NULL, &tagged_bytes, NULL);
        }
        else if (tiff_tag == tiff_tags[Compression])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x8005 };
            gchar *value_description[] = {
                _("No compression"),
                _("CCITT Group 3 1-Dimensional Modified Huffman RLE"),
                _("CCITT T.4 bi-level encoding"),
                _("CCITT T.6 bi-level encoding"),
                _("Lempel-Ziv-Welch (LZW)"),
                _("JPEG (TIFF 6.0 - obsolete)"),
                _("JPEG"),
                _("zlib-format DEFLATE"),
                _("PackBits compression"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: Compression"), "Compression",
                   _("Compression\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo compression\n"
                     "<tt>00 02<sub>16</sub></tt>\tCCITT Group 3 1-Dimensional Modified Huffman RLE\n"
                     "<tt>00 03<sub>16</sub></tt>\tCCITT T.4 bi-level encoding\n"
                     "<tt>00 04<sub>16</sub></tt>\tCCITT T.6 bi-level encoding\n"
                     "<tt>00 05<sub>16</sub></tt>\tLempel-Ziv-Welch (LZW)\n"
                     "<tt>00 06<sub>16</sub></tt>\tJPEG (TIFF 6.0 - obsolete)\n"
                     "<tt>00 07<sub>16</sub></tt>\tJPEG\n"
                     "<tt>00 08<sub>16</sub></tt>\tzlib-format DEFLATE\n"
                     "<tt>80 05<sub>16</sub></tt>\tPackBits compression"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[PhotometricInterpretation])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            gchar *value_description[] = {
                _("WhiteIsZero"),
                _("BlackIsZero"),
                _("RGB"),
                _("Palette color"),
                _("Transparency mask"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: PhotometricInterpretation"), "PhotometricInterpretation",
                   _("PhotometricInterpretation\n"
                     "<tt>00 00<sub>16</sub></tt>\tWhiteIsZero\n"
                     "<tt>00 01<sub>16</sub></tt>\tBlackIsZero\n"
                     "<tt>00 02<sub>16</sub></tt>\tRGB\n"
                     "<tt>00 03<sub>16</sub></tt>\tPalette color\n"
                     "<tt>00 04<sub>16</sub></tt>\tTransparency mask"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[Threshholding])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            gchar *value_description[] = {
                _("No dithering or halftoning has been applied to the image data"),
                _("An ordered dither or halftone technique has been applied to the image data"),
                _("A randomized process such as error diffusion has been applied to the image data"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: Threshholding"), "Threshholding",
                   _("Threshholding\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo dithering or halftoning has been applied to the image data\n"
                     "<tt>00 02<sub>16</sub></tt>\tAn ordered dither or halftone technique has been applied to the image data\n"
                     "<tt>00 03<sub>16</sub></tt>\tA randomized process such as error diffusion has been applied to the image data"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[CellWidth])
        {
            process_short_tag (file, NULL, _("Tag: CellWidth"), "CellWidth",
                               _("The width of the dithering or halftoning matrix"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[CellLength])
        {
            process_short_tag (file, NULL, _("Tag: CellLength"), "CellLength",
                               _("The length of the dithering or halftoning matrix"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[FillOrder])
        {
            guint16 values[] = { 0x1, 0x2 };
            gchar *value_description[] = {
                _("Higher-order bits"),
                _("Lower-order bits"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: FillOrder"), "FillOrder",
                   _("FillOrder\n"
                     "<tt>00 01<sub>16</sub></tt>\tPixels with lower column values are stored in the higher-order bits of the byte\n"
                     "<tt>00 02<sub>16</sub></tt>\tPixels with lower column values are stored in the lower-order bits of the byte"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[DocumentName])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: DocumentName"), "DocumentName", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ImageDescription])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: ImageDescription"), "ImageDescription", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Make])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Make"), "Make", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Model])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Model"), "Model", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[StripOffsets])
        {
            if (field_type == SHORT)
                strip_offsets_count = process_short_tag (file, NULL, _("Tag: StripOffsets"), "StripOffsets",
                                          NULL, field_type,
                                          SHORT, count, 0, value_offset, is_little_endian,
                                          0, NULL, NULL, &tagged_bytes, &strip_offsets);
            else
                strip_offsets_count = process_long_tag (file, NULL, _("Tag: StripOffsets"), "StripOffsets",
                                          NULL, field_type,
                                          LONG, count, 0, value_offset, is_little_endian,
                                          &tagged_bytes, &strip_offsets);
        }
        else if (tiff_tag == tiff_tags[Orientation])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
            gchar *value_description[] = {
                _("Top-left"),
                _("Top-right"),
                _("Bottom-right"),
                _("Bottom-left"),
                _("Left-top"),
                _("Right-top"),
                _("Right-bottom"),
                _("Left-bottom"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: Orientation"), "Orientation",
                   _("Orientation\n"
                     "<tt>00 01<sub>16</sub></tt>\tTop-left\n"
                     "<tt>00 02<sub>16</sub></tt>\tTop-right\n"
                     "<tt>00 03<sub>16</sub></tt>\tBottom-right\n"
                     "<tt>00 04<sub>16</sub></tt>\tBottom-left\n"
                     "<tt>00 05<sub>16</sub></tt>\tLeft-top\n"
                     "<tt>00 06<sub>16</sub></tt>\tRight-top\n"
                     "<tt>00 07<sub>16</sub></tt>\tRight-bottom\n"
                     "<tt>00 08<sub>16</sub></tt>\tLeft-bottom"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SamplesPerPixel])
        {
            process_short_tag (file, NULL, _("Tag: SamplesPerPixel"), "SamplesPerPixel",
                               _("Number of components per pixel"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[RowsPerStrip])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: RowsPerStrip"), "RowsPerStrip",
                                   _("Number of rows per strip"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: RowsPerStrip"), "RowsPerStrip",
                                  _("Number of rows per strip"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian,
                                  NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[StripByteCounts])
        {
            if (field_type == SHORT)
                strip_byte_counts_count = process_short_tag (file, NULL, _("Tag: StripByteCounts"), "StripByteCounts",
                                              NULL, field_type,
                                              SHORT, count, 0, value_offset, is_little_endian,
                                              0, NULL, NULL, &tagged_bytes, &strip_byte_counts);
            else
                strip_byte_counts_count = process_long_tag (file, NULL, _("Tag: StripByteCounts"), "StripByteCounts",
                                              NULL, field_type,
                                              LONG, count, 0, value_offset, is_little_endian,
                                              &tagged_bytes, &strip_byte_counts);
        }
        else if (tiff_tag == tiff_tags[MinSampleValue])
        {
            process_short_tag (file, NULL, _("Tag: MinSampleValue"), "MinSampleValue",
                               _("The minimum component value used"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[MaxSampleValue])
        {
            process_short_tag (file, NULL, _("Tag: MaxSampleValue"), "MaxSampleValue",
                               _("The maximum component value used"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[XResolution])
        {
            process_rational_tag (file, NULL, _("Tag: XResolution"), "XResolution",
                                  _("Number of pixels per ResolutionUnit in the ImageWidth direction"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f", &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YResolution])
        {
            process_rational_tag (file, NULL, _("Tag: YResolution"), "YResolution",
                                  _("Number of pixels per ResolutionUnit in the ImageLength direction"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f", &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[PlanarConfiguration])
        {
            guint16 values[] = { 0x1, 0x2 };
            gchar *value_description[] = {
                _("Chunky format"),
                _("Planar format"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: PlanarConfiguration"), "PlanarConfiguration",
                   _("PlanarConfiguration\n"
                     "<tt>00 01<sub>16</sub></tt>\tChunky format (components are stored contiguously)\n"
                     "<tt>00 02<sub>16</sub></tt>\tPlanar format (components are stored in separate component planes)"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[PageName])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: PageName"), "PageName", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[XPosition])
        {
            process_rational_tag (file, NULL, _("Tag: XPosition"), "XPosition",
                                  _("The X offset in ResolutionUnits of the left side of the image"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f", &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YPosition])
        {
            process_rational_tag (file, NULL, _("Tag: YPosition"), "YPosition",
                                  _("The Y offset in ResolutionUnits of the top of the image"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f", &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ResolutionUnit])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            gchar *value_description[] = {
                _("No unit"),
                _("Inch"),
                _("Centimeter"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: ResolutionUnit"), "ResolutionUnit",
                   _("ResolutionUnit\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo unit\n"
                     "<tt>00 02<sub>16</sub></tt>\tInch\n"
                     "<tt>00 03<sub>16</sub></tt>\tCentimeter"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[PageNumber])
        {
            process_short_tag (file, NULL, _("Tag: PageNumber"), "PageNumber",
                               _("Page number / Total pages\n"
                                 "Page numbers start at 0\n"
                                 "Total = 0 if it is unknown"), field_type,
                               SHORT, count, 2, value_offset, is_little_endian,
                               0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[Software])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Software"), "Software", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[DateTime])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: DateTime"), "DateTime", field_type,
                               count, 20, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Artist])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Artist"), "Artist", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[HostComputer])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: HostComputer"), "HostComputer", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Predictor])
        {
            guint16 values[] = { 0x1, 0x2 };
            gchar *value_description[] = {
                _("No prediction scheme used before coding"),
                _("Horizontal differencing"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: Predictor"), "Predictor",
                   _("Predictor\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo prediction scheme used before coding\n"
                     "<tt>00 02<sub>16</sub></tt>\tHorizontal differencing"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[WhitePoint])
        {
            gchar *value_names[] = {
                _("White point x"),
                _("White point y")
            };
            process_rational_tag (file, NULL, _("Tag: WhitePoint"), "WhitePoint",
                                  _("The values are described using the 1931 CIE xy chromaticity diagram"),
                                  value_names, field_type, RATIONAL, count, 2, value_offset,
                                  is_little_endian, NULL, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[PrimaryChromaticities])
        {
            gchar *value_names[] = {
                _("Red x"),
                _("Red y"),
                _("Green x"),
                _("Green y"),
                _("Blue x"),
                _("Blue y")
            };
            process_rational_tag (file, NULL, _("Tag: PrimaryChromaticities"), "PrimaryChromaticities",
                                  _("The values are described using the 1931 CIE xy chromaticity diagram"),
                                  value_names, field_type, RATIONAL, count, 6, value_offset,
                                  is_little_endian, NULL, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormat])
        {
            jpeg_offset = process_long_tag (file, NULL, _("Tag: JPEGInterchangeFormat"), "JPEGInterchangeFormat",
                                            NULL, field_type,
                                            LONG, count, 1, value_offset, is_little_endian,
                                            NULL, NULL);

        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormatLength])
        {
            jpeg_length = process_long_tag (file, NULL, _("Tag: JPEGInterchangeFormatLength"), "JPEGInterchangeFormatLength",
                                            NULL, field_type,
                                            LONG, count, 1, value_offset, is_little_endian,
                                            NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[YCbCrCoefficients])
        {
            gchar *value_names[] = {
                _("LumaRed"),
                _("LumaGreen"),
                _("LumaBlue")
            };
            process_rational_tag (file, NULL, _("Tag: YCbCrCoefficients"), "YCbCrCoefficients",
                                  _("Coefficients used to compute luminance Y"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YCbCrSubSampling])
        {
            process_short_tag (file, NULL, _("Tag: YCbCrSubSampling"), "YCbCrSubSampling",
                   _("YCbCrSubSamplingHoriz\n"
                     "<tt>00 01<sub>16</sub></tt>\tChroma image ImageWidth = Luma image ImageWidth\n"
                     "<tt>00 02<sub>16</sub></tt>\tChroma image ImageWidth = 1/2 Luma image ImageWidth\n"
                     "<tt>00 04<sub>16</sub></tt>\tChroma image ImageWidth = 1/4 Luma image ImageWidth\n"
                     "YCbCrSubSamplingVert\n"
                     "<tt>00 01<sub>16</sub></tt>\tChroma image ImageLength = Luma image ImageLength\n"
                     "<tt>00 02<sub>16</sub></tt>\tChroma image ImageLength = 1/2 Luma image ImageLength\n"
                     "<tt>00 04<sub>16</sub></tt>\tChroma image ImageLength = 1/4 Luma image ImageLength"),
                   field_type, SHORT, count, 2, value_offset, is_little_endian,
                   0, NULL, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[YCbCrPositioning])
        {
            guint16 values[] = { 0x1, 0x2 };
            gchar *value_description[] = {
                _("Centered"),
                _("Cosited"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: YCbCrPositioning"), "YCbCrPositioning",
                   _("YCbCrPositioning\n"
                     "<tt>00 01<sub>16</sub></tt>\tCentered\n"
                     "<tt>00 02<sub>16</sub></tt>\tCosited"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   sizeof (values) >> 1, values, value_description, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[ReferenceBlackWhite])
        {
            gchar *value_names[] = {
                _("ReferenceBlackWhite footroom (component 1)"),
                _("ReferenceBlackWhite headroom (component 1)"),
                _("ReferenceBlackWhite footroom (component 2)"),
                _("ReferenceBlackWhite headroom (component 2)"),
                _("ReferenceBlackWhite footroom (component 3)"),
                _("ReferenceBlackWhite headroom (component 3)")
            };
            process_rational_tag (file, NULL, _("Tag: ReferenceBlackWhite"), "ReferenceBlackWhite",
                                  _("Headroom and footroom pairs for each component"),
                                  value_names, field_type, RATIONAL, count, 6, value_offset,
                                  is_little_endian, NULL, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Copyright])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Copyright"), "Copyright", field_type,
                               count, 0, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ExifIFD])
        {
            process_exififd_tag (file, _("Tag: ExifIFD"), field_type, count, value_offset, is_little_endian, &tagged_bytes,
                                 &exif_tab, &exif_ascii_tab, TRUE);
        }
        else if (tiff_tag == tiff_tags[GPSInfoIFD])
        {
            process_exififd_tag (file, _("Tag: GPSInfoIFD"), field_type, count, value_offset, is_little_endian, &tagged_bytes,
                                 &exif_gpsinfo_tab, &exif_gpsinfo_ascii_tab, FALSE);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Tag: Unknown"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 2, _("Field type"));
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Count"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 4, _("Tag value or offset"));

            if (unknown_tag_count > 10 && ifd_entries > 100)
            {
                analyzer_utils_insert_notice (file,
                              _("<span foreground=\"red\">Over ten unknown tags and excessive number of IFD entries</span>"),
                              0, 20);
                analyzer_utils_insert_notice (file, _("<span foreground=\"red\" weight=\"bold\">ANALYSIS FAILED</span>"), 0, 0);
                goto END_ERROR_LOOP;
            }

            unknown_tag_count++;
        }

        ifd_entries--;
        continue;

        END_ERROR_LOOP:
        tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
        break;
    }

    if (jpeg_offset && jpeg_length)
    {
        SET_POINTER (file, jpeg_offset);

        if (FILE_HAS_DATA_N (file, jpeg_length))
        {
            tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, jpeg_length, _("Embedded JPEG file"));

            analyzer_utils_describe_tab (&jpeg_tab, _("There is an embedded JPEG file, analyze it in another tab"), NULL);
            analyzer_utils_embedded_file (file, &jpeg_tab, jpeg_length);

            tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            SET_POINTER (file, jpeg_offset);
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Embedded JPEG file"));
        }
        else
        {
            analyzer_utils_describe_tooltip_tab (&jpeg_tab, _("Failed to extract the embedded JPEG file"), NULL,
                             _("Malformed offset (JPEGInterchangeFormat tag) or length (JPEGInterchangeFormatLength tag)"));
        }
    }

    /* Used ASCII tabs reset description_lines_count as they hold text fields
     * Used key-value tabs have more than one description line (the title) */
    if (ascii_tab.description_lines_count == 0)
        analyzer_utils_insert_tab (file, &ascii_tab, _("ASCII tags"));

    /* Embedded JPEG tab*/
    if (jpeg_tab.description_lines_count != 1)
        analyzer_utils_insert_tab (file, &jpeg_tab, "JPEG");

    if (exif_tab.description_lines_count > 1)
        analyzer_utils_insert_tab (file, &exif_tab, _("Exif tags"));
    if (exif_ascii_tab.description_lines_count == 0)
        analyzer_utils_insert_tab (file, &exif_ascii_tab, _("Exif ASCII tags"));
    if (exif_gpsinfo_tab.description_lines_count > 1)
        analyzer_utils_insert_tab (file, &exif_gpsinfo_tab, _("GPSInfo tags"));
    if (exif_gpsinfo_ascii_tab.description_lines_count == 0)
        analyzer_utils_insert_tab (file, &exif_gpsinfo_ascii_tab, _("GPSInfo ASCII tags"));

    /* StripOffsets and StripByteCounts must declare the same number of strips */
    if (strip_offsets_count == strip_byte_counts_count)
    {
        /* Loop and tag all strips */
        for (i = 0; i < strip_offsets_count; i++)
        {
            /* The strip offset */
            value_offset = strip_offsets[i];
            /* The length of the strip */
            count = strip_byte_counts[i];

            SET_POINTER (file, value_offset);

            if (FILE_HAS_DATA_N (file, count))
            {
                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (value_offset));

                analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, _("Image data strip"));

                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (value_offset + count));

                SET_POINTER (file, value_offset);
                analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Image data strip"));
            }
        }
    }
    else
    {
        analyzer_utils_add_description (file, _("The StripOffsets and StripByteCounts tags do not agree"),
                                        NULL, NULL, 10, 0);
        analyzer_utils_describe (file, _("Could not determine the number of strips in the image"), NULL);
    }

    /* Add the tagged area boundaries: 8 (as the initial 8 bytes are already tagged) and file size */
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (8));
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_FILE_SIZE (file)));

    /* Sort the tagged area file indices */
    tagged_bytes = g_slist_sort (tagged_bytes, tagged_bytes_compare);

    for (index = tagged_bytes;
    index != NULL;
    index = index->next->next)
    {
        /* Malformed file: offset/file index exceeds file size */
        if (GPOINTER_TO_UINT (index->data) > GET_FILE_SIZE (file))
            break;

        /* All pairs must have the same file index */
        count = GPOINTER_TO_UINT (index->next->data) - GPOINTER_TO_UINT (index->data);

        if (count)
            analyzer_utils_create_tag_index (file, UNUSED_OVERLAPPING_COLOR, FALSE, count,
                                 GPOINTER_TO_UINT (index->data), _("Unused or overlapping data"));
    }
    g_slist_free (tagged_bytes);
}
