/* exif-analyzer.c
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

#include "exif-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-exif.h"


void
chirurgien_analyze_exif (AnalyzerFile *file,
                         gboolean is_little_endian,
                         GSList **tagged_bytes,
                         AnalyzerTab *exif_tab,
                         AnalyzerTab *exif_ascii_tab)
{
    const guint16 tiff_tags[EXIF_TAGS] =
    {
        0x829A, // ExposureTime
        0x829D, // FNumber
        0x8822, // ExposureProgram
        0x8824, // SpectralSensitivity
        0x8827, // PhotographicSensitivity
        0x8830, // SensitivityType
        0x8831, // StandardOutputSensitivity
        0x8832, // RecommendedExposureIndex
        0x8833, // ISOSpeed
        0x8834, // ISOSpeedLatitudeyyy
        0x8835, // ISOSpeedLatitudezzz
        0x9000, // ExifVersion
        0x9003, // DateTimeOriginal
        0x9004, // DateTimeDigitized
        0x9101, // ComponentsConfiguration
        0x9102, // CompressedBitsPerPixel
        0x9201, // ShutterSpeedValue
        0x9202, // ApertureValue
        0x9203, // BrightnessValue
        0x9204, // ExposureBiasValue
        0x9205, // MaxApertureValue
        0x9206, // SubjectDistance
        0x9207, // MeteringMode
        0x9208, // LightSource
        0x9209, // Flash
        0x920A, // FocalLength
        0x927C, // MakerNote
        0x9286, // UserComment
        0x9290, // SubSecTime
        0x9291, // SubSecTimeOriginal
        0x9292, // SubSecTimeDigitized
        0xA000, // FlashpixVersion
        0xA001, // ColorSpace
        0xA002, // PixelXDimension
        0xA003, // PixelYDimension
        0xA004, // RelatedSoundFile
        0xA210, // FocalPlaneResolutionUnit
        0xA214, // SubjectLocation
        0xA215, // ExposureIndex
        0xA217, // SensingMethod
        0xA300, // FileSource
        0xA301, // SceneType
        0xA302, // CFAPattern
        0xA401, // CustomRendered
        0xA402, // ExposureMode
        0xA403, // WhiteBalance
        0xA404, // DigitalZoomRatio
        0xA405, // FocalLengthIn35mmFilm
        0xA406, // SceneCaptureType
        0xA407, // GainControl
        0xA408, // Contrast
        0xA409, // Saturation
        0xA40A, // Sharpness
        0xA40C, // SubjectDistanceRange
        0xA420, // ImageUniqueID
        0xA430, // CameraOwnerName
        0xA431, // BodySerialNumber
        0xA432, // LensSpecification
        0xA433, // LensMake
        0xA434, // LensModel
        0xA435, // LensSerialNumber
        0xA500, // Gamma
    };

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    if (!analyzer_utils_read (&ifd_entries, file , 2))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
        return;
    }
    analyzer_utils_tag_navigation (file, IFD_COLOR, 2, _("Number of directory entries (Exif)"), "IFD");

    if (!is_little_endian)
        ifd_entries = g_ntohs (ifd_entries);
    ifd_entries++;

    /* Tag loop */
    while (ifd_entries)
    {
        if (ifd_entries == 1)
        {
            if (!analyzer_utils_read (&count, file, 4))
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
                *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
                return;
            }

            analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD end (Exif)"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            break;
        }

        /* TIFF tag */
        if (!analyzer_utils_read (&tiff_tag, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            tiff_tag = g_ntohs (tiff_tag);

        /* Field type */
        if (!analyzer_utils_read (&field_type, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            field_type = g_ntohs (field_type);

        /* Count */
        if (!analyzer_utils_read (&count, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            count = g_ntohl (count);

        /* Tag value or offset */
        if (!analyzer_utils_read (&value_offset, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }

        /* Analyze tag data */
        if (tiff_tag == tiff_tags[ExposureTime])
        {
            analyze_exposuretime_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[FNumber])
        {
            analyze_fnumber_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ExposureProgram])
        {
            analyze_exposureprogram_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SpectralSensitivity])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_SpectralSensitivity, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[PhotographicSensitivity])
        {
            analyze_photographicsensitivity_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SensitivityType])
        {
            analyze_sensitivitytype_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[StandardOutputSensitivity])
        {
            analyze_standardoutputsensitivity_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[RecommendedExposureIndex])
        {
            analyze_recommendedexposureindex_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ISOSpeed])
        {
            analyze_isospeed_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudeyyy])
        {
            analyze_isospeedlatitudeyyy_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudezzz])
        {
            analyze_isospeedlatitudezzz_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ExifVersion])
        {
            analyze_exifversion_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DateTimeOriginal])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_DateTimeOriginal, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[DateTimeDigitized])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_DateTimeDigitized, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[ComponentsConfiguration])
        {
            analyze_componentsconfiguration_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CompressedBitsPerPixel])
        {
            analyze_compressedbitsperpixel_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ShutterSpeedValue])
        {
            analyze_shutterspeedvalue_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ApertureValue])
        {
            analyze_aperturevalue_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[BrightnessValue])
        {
            analyze_brightnessvalue_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[ExposureBiasValue])
        {
            analyze_exposurebiasvalue_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[MaxApertureValue])
        {
            analyze_maxaperturevalue_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[SubjectDistance])
        {
            analyze_subjectdistance_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[MeteringMode])
        {
            analyze_meteringmode_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LightSource])
        {
            analyze_lightsource_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Flash])
        {
            analyze_flash_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FocalLength])
        {
            analyze_focallength_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[MakerNote])
        {
            analyze_makernote_tag (file, exif_ascii_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[UserComment])
        {
            analyze_usercomment_tag (file, exif_ascii_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[SubSecTime])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_SubSecTime, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeOriginal])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_SubSecTimeOriginal, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeDigitized])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_SubSecTimeDigitized, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[FlashpixVersion])
        {
            analyze_flashpixversion_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ColorSpace])
        {
            analyze_colorspace_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PixelXDimension])
        {
            analyze_pixelxdimension_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PixelYDimension])
        {
            analyze_pixelydimension_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[RelatedSoundFile])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_RelatedSoundFile, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[FocalPlaneResolutionUnit])
        {
            analyze_focalplaneresolutionunit_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubjectLocation])
        {
            analyze_subjectlocation_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ExposureIndex])
        {
            analyze_exposureindex_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[SensingMethod])
        {
            analyze_sensingmethod_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FileSource])
        {
            analyze_filesource_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SceneType])
        {
            analyze_scenetype_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CFAPattern])
        {
            analyze_cfapattern_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[CustomRendered])
        {
            analyze_customrendered_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ExposureMode])
        {
            analyze_exposuremode_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[WhiteBalance])
        {
            analyze_whitebalance_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DigitalZoomRatio])
        {
            analyze_digitalzoomratio_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[FocalLengthIn35mmFilm])
        {
            analyze_focallengthin35mmfilm_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SceneCaptureType])
        {
            analyze_scenecapturetype_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GainControl])
        {
            analyze_gaincontrol_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Contrast])
        {
            analyze_contrast_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Saturation])
        {
            analyze_saturation_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Sharpness])
        {
            analyze_sharpness_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubjectDistanceRange])
        {
            analyze_subjectdistancerange_tag (file, exif_tab, field_type, count, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ImageUniqueID])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_ImageUniqueID, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[CameraOwnerName])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_CameraOwnerName, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[BodySerialNumber])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_BodySerialNumber, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[LensSpecification])
        {
            analyze_lensspecification_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else if (tiff_tag == tiff_tags[LensMake])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_LensMake, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[LensModel])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_LensModel, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[LensSerialNumber])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_LensSerialNumber, exif_ascii_tab);
        }
        else if (tiff_tag == tiff_tags[Gamma])
        {
            analyze_gamma_tag (file, exif_tab, field_type, count, value_offset, is_little_endian, tagged_bytes);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Tag: Unknown"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 2, _("Field type"));
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Count"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 4, _("Tag value or offset"));
        }

        ifd_entries--;
    }

}
