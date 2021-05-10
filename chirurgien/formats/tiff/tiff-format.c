/* tiff-format.c
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

#include "tiff-format.h"

#include <glib/gi18n.h>

#include "chirurgien-tiff.h"


void
chirurgien_tiff (FormatsFile *file)
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

    DescriptionTab ascii_tab, exif_tab, exif_ascii_tab,
                   exif_gpsinfo_tab, exif_gpsinfo_ascii_tab;

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    guint strip_offsets_count = 0, strip_byte_counts_count = 0;
    g_autofree guint32 *strip_offsets = NULL;
    g_autofree guint32 *strip_byte_counts = NULL;

    guint unknown_tag_count = 0;
    guint32 jpeg_offset = 0, jpeg_length = 0;

    gboolean is_little_endian;

    format_utils_set_title (file, "Tag Image File Format");

    format_utils_init_tab (&ascii_tab, NULL);
    format_utils_init_tab (&exif_tab, _("Exif-specific tags"));
    format_utils_init_tab (&exif_ascii_tab, NULL);
    format_utils_init_tab (&exif_gpsinfo_tab, _("Exif GPSInfo-specific tags"));
    format_utils_init_tab (&exif_gpsinfo_ascii_tab, NULL);

    format_utils_start_section (file, _("Endianness"));

    /* Use the tiff_tag variable because it is available */
    format_utils_read (file, &tiff_tag, 2);
    if (tiff_tag == 0x4949)
    {
        is_little_endian = TRUE;
        format_utils_add_line (file, _("Endianness"), _("Little-endian"),
                             _("Endianness\n"
                               "<tt>49 49<sub>16</sub></tt>\tLittle-endian\n"
                               "<tt>4D 4D<sub>16</sub></tt>\tBig-endian"));
    }
    else
    {
        is_little_endian = FALSE;
        format_utils_add_line (file, _("Endianness"), _("Big-endian"),
                             _("Endianness\n"
                               "<tt>49 49<sub>16</sub></tt>\tLittle-endian\n"
                               "<tt>4D 4D<sub>16</sub></tt>\tBig-endian"));
    }

    format_utils_start_section (file, _("Standard TIFF tags"));

    format_utils_add_field (file, TIFF_TAG_COLOR, TRUE, 2, _("Endianness"), _("End."));
    format_utils_add_field (file, VALUE_OFFSET_COLOR_1, TRUE, 2, _("TIFF identifier"), NULL);

    if (!format_utils_read (file, &value_offset, 4))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Unrecognized data"), NULL);
        return;
    }
    format_utils_add_field (file, IFD_COLOR, TRUE, 4, _("IFD offset"), NULL);

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    SET_INDEX (file, value_offset);

    if (!format_utils_read (file, &ifd_entries, 2))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Unrecognized data"), NULL);
        return;
    }
    format_utils_add_field (file, IFD_COLOR, TRUE, 2, _("Number of directory entries"), _("IFD"));

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
            if (!format_utils_read (file, &count, 4))
                break;

            if (!count)
            {
                format_utils_add_field (file, IFD_COLOR, TRUE, 4, _("IFD end"), NULL);
                break;
            }
            else
            {
                if (!is_little_endian)
                    count = g_ntohl (count);

                format_utils_add_field (file, IFD_COLOR, TRUE, 4, _("IFD offset"), NULL);

                SET_INDEX (file, count);

                if (!format_utils_read (file, &ifd_entries, 2))
                    break;

                format_utils_add_field (file, IFD_COLOR, TRUE, 2, _("Number of directory entries"), "IFD");

                if (!is_little_endian)
                    ifd_entries = g_ntohs (ifd_entries);
                ifd_entries++;
                continue;
            }
        }

        /* TIFF tag */
        if (!format_utils_read (file, &tiff_tag, 2))
            break;
        ADVANCE_INDEX (file, 2);

        /* Field type */
        if (!format_utils_read (file, &field_type, 2))
            break;
        ADVANCE_INDEX (file, 2);

        /* Count */
        if (!format_utils_read (file, &count, 4))
            break;
        ADVANCE_INDEX (file, 4);

        /* Tag value or offset */
        if (!format_utils_read (file, &value_offset, 4))
            break;
        ADVANCE_INDEX (file, -8);

        if (!is_little_endian)
        {
            tiff_tag = g_ntohs (tiff_tag);
            field_type = g_ntohs (field_type);
            count = g_ntohl (count);
        }

        /* Process tag */
        if (tiff_tag == tiff_tags[NewSubfileType])
        {
            process_long_tag (file, NULL, _("Tag: NewSubfileType"), "NewSubfileType",
                              NULL, field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[SubfileType])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageWidth])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: ImageWidth"), "ImageWidth",
                                   _("Number of columns in the image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: ImageWidth"), "ImageWidth",
                                  _("Number of columns in the image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageLength])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: ImageLength"), "ImageLength",
                                   _("Number of rows in the image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: ImageLength"), "ImageLength",
                                  _("Number of rows in the image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[BitsPerSample])
        {
            process_short_tag (file, NULL, _("Tag: BitsPerSample"), "BitsPerSample",
                               _("Bit depth for each component"), field_type,
                               SHORT, count, 0, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[Compression])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x8005 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[PhotometricInterpretation])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Threshholding])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[CellWidth])
        {
            process_short_tag (file, NULL, _("Tag: CellWidth"), "CellWidth",
                               _("The width of the dithering or halftoning matrix"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[CellLength])
        {
            process_short_tag (file, NULL, _("Tag: CellLength"), "CellLength",
                               _("The length of the dithering or halftoning matrix"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[FillOrder])
        {
            guint16 values[] = { 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Higher-order bits"),
                _("Lower-order bits"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: FillOrder"), "FillOrder",
                   _("FillOrder\n"
                     "<tt>00 01<sub>16</sub></tt>\tPixels with lower column values are stored in the higher-order bits of the byte\n"
                     "<tt>00 02<sub>16</sub></tt>\tPixels with lower column values are stored in the lower-order bits of the byte"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[DocumentName])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: DocumentName"), "DocumentName", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ImageDescription])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: ImageDescription"), "ImageDescription", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Make])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Make"), "Make", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Model])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Model"), "Model", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[StripOffsets])
        {
            if (field_type == SHORT)
                strip_offsets_count = process_short_tag (file, NULL, _("Tag: StripOffsets"), "StripOffsets",
                                          NULL, field_type,
                                          SHORT, count, 0, value_offset, is_little_endian,
                                          0, NULL, NULL, &strip_offsets);
            else
                strip_offsets_count = process_long_tag (file, NULL, _("Tag: StripOffsets"), "StripOffsets",
                                          NULL, field_type,
                                          LONG, count, 0, value_offset, is_little_endian,
                                          &strip_offsets);
        }
        else if (tiff_tag == tiff_tags[Orientation])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SamplesPerPixel])
        {
            process_short_tag (file, NULL, _("Tag: SamplesPerPixel"), "SamplesPerPixel",
                               _("Number of components per pixel"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[RowsPerStrip])
        {
            if (field_type == SHORT)
                process_short_tag (file, NULL, _("Tag: RowsPerStrip"), "RowsPerStrip",
                                   _("Number of rows per strip"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, NULL, _("Tag: RowsPerStrip"), "RowsPerStrip",
                                  _("Number of rows per strip"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[StripByteCounts])
        {
            if (field_type == SHORT)
                strip_byte_counts_count = process_short_tag (file, NULL, _("Tag: StripByteCounts"), "StripByteCounts",
                                              NULL, field_type,
                                              SHORT, count, 0, value_offset, is_little_endian,
                                              0, NULL, NULL, &strip_byte_counts);
            else
                strip_byte_counts_count = process_long_tag (file, NULL, _("Tag: StripByteCounts"), "StripByteCounts",
                                              NULL, field_type,
                                              LONG, count, 0, value_offset, is_little_endian,
                                              &strip_byte_counts);
        }
        else if (tiff_tag == tiff_tags[MinSampleValue])
        {
            process_short_tag (file, NULL, _("Tag: MinSampleValue"), "MinSampleValue",
                               _("The minimum component value used"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[MaxSampleValue])
        {
            process_short_tag (file, NULL, _("Tag: MaxSampleValue"), "MaxSampleValue",
                               _("The maximum component value used"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[XResolution])
        {
            process_rational_tag (file, NULL, _("Tag: XResolution"), "XResolution",
                                  _("Number of pixels per ResolutionUnit in the ImageWidth direction"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f");
        }
        else if (tiff_tag == tiff_tags[YResolution])
        {
            process_rational_tag (file, NULL, _("Tag: YResolution"), "YResolution",
                                  _("Number of pixels per ResolutionUnit in the ImageLength direction"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f");
        }
        else if (tiff_tag == tiff_tags[PlanarConfiguration])
        {
            guint16 values[] = { 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Chunky format"),
                _("Planar format"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: PlanarConfiguration"), "PlanarConfiguration",
                   _("PlanarConfiguration\n"
                     "<tt>00 01<sub>16</sub></tt>\tChunky format (components are stored contiguously)\n"
                     "<tt>00 02<sub>16</sub></tt>\tPlanar format (components are stored in separate component planes)"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[PageName])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: PageName"), "PageName", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[XPosition])
        {
            process_rational_tag (file, NULL, _("Tag: XPosition"), "XPosition",
                                  _("The X offset in ResolutionUnits of the left side of the image"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f");
        }
        else if (tiff_tag == tiff_tags[YPosition])
        {
            process_rational_tag (file, NULL, _("Tag: YPosition"), "YPosition",
                                  _("The Y offset in ResolutionUnits of the top of the image"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, "%.1f");
        }
        else if (tiff_tag == tiff_tags[ResolutionUnit])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
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
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[PageNumber])
        {
            process_short_tag (file, NULL, _("Tag: PageNumber"), "PageNumber",
                               _("Page number / Total pages\n"
                                 "Page numbers start at 0\n"
                                 "Total = 0 if it is unknown"), field_type,
                               SHORT, count, 2, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[Software])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Software"), "Software", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DateTime])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: DateTime"), "DateTime", field_type,
                               count, 20, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Artist])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Artist"), "Artist", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[HostComputer])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: HostComputer"), "HostComputer", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Predictor])
        {
            guint16 values[] = { 0x1, 0x2 };
            const gchar *value_description[] = {
                _("No prediction scheme used before coding"),
                _("Horizontal differencing"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: Predictor"), "Predictor",
                   _("Predictor\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo prediction scheme used before coding\n"
                     "<tt>00 02<sub>16</sub></tt>\tHorizontal differencing"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[WhitePoint])
        {
            const gchar *value_names[] = {
                _("White point x"),
                _("White point y")
            };
            process_rational_tag (file, NULL, _("Tag: WhitePoint"), "WhitePoint",
                                  _("The values are described using the 1931 CIE xy chromaticity diagram"),
                                  value_names, field_type, RATIONAL, count, 2, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[PrimaryChromaticities])
        {
            const gchar *value_names[] = {
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
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormat])
        {
            jpeg_offset = process_long_tag (file, NULL, _("Tag: JPEGInterchangeFormat"), "JPEGInterchangeFormat",
                                            NULL, field_type, LONG, count, 1, value_offset, is_little_endian, NULL);

        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormatLength])
        {
            jpeg_length = process_long_tag (file, NULL, _("Tag: JPEGInterchangeFormatLength"), "JPEGInterchangeFormatLength",
                                            NULL, field_type, LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[YCbCrCoefficients])
        {
            const gchar *value_names[] = {
                _("LumaRed"),
                _("LumaGreen"),
                _("LumaBlue")
            };
            process_rational_tag (file, NULL, _("Tag: YCbCrCoefficients"), "YCbCrCoefficients",
                                  _("Coefficients used to compute luminance Y"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
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
                   0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[YCbCrPositioning])
        {
            guint16 values[] = { 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Centered"),
                _("Cosited"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, NULL, _("Tag: YCbCrPositioning"), "YCbCrPositioning",
                   _("YCbCrPositioning\n"
                     "<tt>00 01<sub>16</sub></tt>\tCentered\n"
                     "<tt>00 02<sub>16</sub></tt>\tCosited"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ReferenceBlackWhite])
        {
            const gchar *value_names[] = {
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
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[Copyright])
        {
            process_ascii_tag (file, &ascii_tab, _("Tag: Copyright"), "Copyright", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ExifIFD])
        {
            if (!process_exififd_tag (file, _("Tag: ExifIFD"), field_type, count, value_offset,
                                      is_little_endian, &exif_tab, &exif_ascii_tab, EXIF_IFD))
            {
                format_utils_add_line_no_section (file, _("<span foreground=\"red\" weight=\"bold\">ANALYSIS FAILED</span>"));
                format_utils_add_line_no_section (file, _("<span foreground=\"red\">Over ten unknown tags and excessive number of IFD entries</span>"));
                break;
            }
        }
        else if (tiff_tag == tiff_tags[GPSInfoIFD])
        {
            if (!process_exififd_tag (file, _("Tag: GPSInfoIFD"), field_type, count, value_offset,
                                      is_little_endian, &exif_gpsinfo_tab, &exif_gpsinfo_ascii_tab, GPSINFO_IFD))
            {
                format_utils_add_line_no_section (file, _("<span foreground=\"red\" weight=\"bold\">ANALYSIS FAILED</span>"));
                format_utils_add_line_no_section (file, _("<span foreground=\"red\">Over ten unknown tags and excessive number of IFD entries</span>"));
                break;
            }
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, _("Tag: Unknown"), NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 2, _("Field type"), NULL);
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, _("Count"), NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 4, _("Tag value or offset"), NULL);

            if (unknown_tag_count > 10 && ifd_entries > 100)
            {
                format_utils_add_line_no_section (file, _("<span foreground=\"red\" weight=\"bold\">ANALYSIS FAILED</span>"));
                format_utils_add_line_no_section (file, _("<span foreground=\"red\">Over ten unknown tags and excessive number of IFD entries</span>"));
                break;
            }

            unknown_tag_count++;
        }

        ifd_entries--;
    }

    if (jpeg_offset && jpeg_length)
    {
        SET_INDEX (file, jpeg_offset);

        if (FILE_HAS_DATA_N (file, jpeg_length))
            format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE,
                                         jpeg_length, _("Embedded JPEG file"), NULL, VALUE_OFFSET_COLOR_2);
    }

    format_utils_insert_tab (file, &ascii_tab, _("ASCII tags"));
    format_utils_insert_tab (file, &exif_tab, _("Exif tags"));
    format_utils_insert_tab (file, &exif_ascii_tab, _("Exif ASCII tags"));
    format_utils_insert_tab (file, &exif_gpsinfo_tab, _("GPSInfo tags"));
    format_utils_insert_tab (file, &exif_gpsinfo_ascii_tab, _("GPSInfo ASCII tags"));

    /* StripOffsets and StripByteCounts must declare the same number of strips */
    if (strip_offsets_count == strip_byte_counts_count)
    {
        /* Loop and tag all strips */
        for (guint i = 0; i < strip_offsets_count; i++)
        {
            /* The strip offset */
            value_offset = strip_offsets[i];
            /* The length of the strip */
            count = strip_byte_counts[i];

            SET_INDEX (file, value_offset);

            if (FILE_HAS_DATA_N (file, count))
                format_utils_add_field_full (file, VALUE_OFFSET_COLOR_1, TRUE, count,
                                           _("Image data strip"), _("Strip"), VALUE_OFFSET_COLOR_2);
        }
    }
    else
    {
        format_utils_add_line_no_section (file, _("Could not determine the number of strips in the image"));
        format_utils_add_line_no_section (file, _("The StripOffsets and StripByteCounts tags define a different number of strips or one of the tags is missing"));
    }
}
