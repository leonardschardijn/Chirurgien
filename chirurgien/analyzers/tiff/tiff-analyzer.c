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

    AnalyzerTab ascii_tab, exif_tab, exif_ascii_tab, exif_gpsinfo_tab, exif_gpsinfo_ascii_tab, jpeg_tab;

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    /* As many tags use offsets to point to their values, the tagged_bytes list
     * keeps track of all color tagged bytes, this let's us find the stray bytes not referenced
     * by any tag */
    GSList *tagged_bytes = NULL, *index;

    gsize save_pointer;

    guint strip_offsets_count = 0, strip_byte_counts_count = 0, i;
    g_autofree guint32 *strip_offsets = NULL;
    g_autofree guint32 *strip_byte_counts = NULL;

    guint unknown_tag_count = 0;
    guint32 jpeg_offset;

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

            if (!is_little_endian)
                count = g_ntohl (count);

            if (count == 0)
            {
                analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD end"));
                tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
                break;
            }
            else
            {
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
        if (!is_little_endian)
            tiff_tag = g_ntohs (tiff_tag);

        /* Field type */
        if (!analyzer_utils_read (&field_type, file, 2))
            goto END_ERROR_LOOP;
        if (!is_little_endian)
            field_type = g_ntohs (field_type);

        /* Count */
        if (!analyzer_utils_read (&count, file, 4))
            goto END_ERROR_LOOP;
        if (!is_little_endian)
            count = g_ntohl (count);

        /* Tag value or offset */
        if (!analyzer_utils_read (&value_offset, file, 4))
            goto END_ERROR_LOOP;

        /* Analyze tag data */
        if (tiff_tag == tiff_tags[NewSubfileType])
        {
            analyze_newsubfiletype_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubfileType])
        {
            analyze_subfiletype_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ImageWidth])
        {
            analyze_imagewidth_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ImageLength])
        {
            analyze_imagelength_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[BitsPerSample])
        {
            analyze_bitspersample_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Compression])
        {
            analyze_compression_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PhotometricInterpretation])
        {
            analyze_photometricinterpretation_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Threshholding])
        {
            analyze_threshholding_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CellWidth])
        {
            analyze_cellwidth_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CellLength])
        {
            analyze_celllength_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FillOrder])
        {
            analyze_fillorder_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DocumentName])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_DocumentName, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[ImageDescription])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                   &tagged_bytes, ASCII_ImageDescription, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[Make])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_Make, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[Model])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_Model, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[StripOffsets])
        {
            /* Number of strip offsets */
            strip_offsets_count = analyze_stripoffsets_tag (file, field_type, count, value_offset, is_little_endian,
                                  &strip_offsets, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Orientation])
        {
            analyze_orientation_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SamplesPerPixel])
        {
            analyze_samplesperpixel_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[RowsPerStrip])
        {
            analyze_rowsperstrip_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[StripByteCounts])
        {
            /* Number of strip byte counts */
            strip_byte_counts_count = analyze_stripbytecounts_tag (file, field_type, count, value_offset, is_little_endian,
                                      &strip_byte_counts, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[MinSampleValue])
        {
            analyze_minsamplevalue_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[MaxSampleValue])
        {
            analyze_maxsamplevalue_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[XResolution])
        {
            analyze_xresolution_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YResolution])
        {
            analyze_yresolution_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[PlanarConfiguration])
        {
            analyze_planarconfiguration_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[XPosition])
        {
            analyze_xposition_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YPosition])
        {
            analyze_yposition_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ResolutionUnit])
        {
            analyze_resolutionunit_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PageNumber])
        {
            analyze_pagenumber_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Software])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_Software, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[DateTime])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_DateTime, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[Artist])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_Artist, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[HostComputer])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_HostComputer, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[Predictor])
        {
            analyze_predictor_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[WhitePoint])
        {
            analyze_whitepoint_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[PrimaryChromaticities])
        {
            analyze_primarychromaticities_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormat])
        {
            jpeg_offset = analyze_jpeginterchangeformat_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[JPEGInterchangeFormatLength])
        {
            analyze_jpeginterchangeformatlength_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes,
                                                     jpeg_offset, &jpeg_tab);
        }
        else if (tiff_tag == tiff_tags[YCbCrCoefficients])
        {
            analyze_ycbcrcoefficients_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[YCbCrSubSampling])
        {
            analyze_ycbcrsubsampling_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[YCbCrPositioning])
        {
            analyze_ycbcrpositioning_tag (file, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ReferenceBlackWhite])
        {
            analyze_referenceblackwhite_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[Copyright])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               &tagged_bytes, ASCII_Copyright, &ascii_tab);
        }
        else if (tiff_tag == tiff_tags[ExifIFD])
        {
            analyze_exififd_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes,
                                 &exif_tab, &exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[GPSInfoIFD])
        {
            analyze_gpsinfoifd_tag (file, field_type, count, value_offset, is_little_endian, &tagged_bytes,
                                    &exif_gpsinfo_tab, &exif_gpsinfo_ascii_tab);
        }
        else
        {
            if (unknown_tag_count > 10 && ifd_entries > 100)
            {
                analyzer_utils_insert_notice (file,
                              _("<span foreground=\"red\">Over ten unknown tags and excessive number of IFD entries</span>"),
                              0, 20);
                analyzer_utils_insert_notice (file, _("<span foreground=\"red\" weight=\"bold\">ANALYSIS FAILED</span>"), 0, 0);
                goto END_ERROR_LOOP;
            }

            analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Tag: Unknown"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 2, _("Field type"));
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Count"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 4, _("Tag value or offset"));

            unknown_tag_count++;
        }

        ifd_entries--;
        continue;

        END_ERROR_LOOP:
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
        tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
        break;
    }

    /* Used ASCII tabs reset description_lines_count as they hold text fields
     * Used key-value tabs have more than one description line (the title) */
    if (ascii_tab.description_lines_count == 0)
        analyzer_utils_insert_tab (file, &ascii_tab, _("ASCII tags"));

    /* Embedded JPEG tab*/
    if (jpeg_tab.description_lines_count == 0)
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

            save_pointer = GET_POINTER (file);

            SET_POINTER (file, value_offset);
            tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (value_offset));

            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, _("Image data strip"));

            tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (value_offset + count));

            SET_POINTER (file, value_offset);
            analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, _("Image data strip"));

            SET_POINTER (file, save_pointer);
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
