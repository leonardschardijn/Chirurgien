/* exif-format.c
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

#include "exif-format.h"

#include "chirurgien-exif.h"


gboolean
chirurgien_exif (FormatsFile    *file,
                 gboolean        is_little_endian,
                 DescriptionTab *exif_tab,
                 DescriptionTab *exif_ascii_tab)
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

    guint unknown_tag_count = 0;

    if (FILE_HAS_DATA_N (file, 2))
    {
        format_utils_read (file, &ifd_entries, 2);

        format_utils_add_field (file, IFD_COLOR, TRUE, 2,
                                "Number of directory entries (Exif)", "Exif IFD");
    }
    else
    {
       return TRUE;
    }

    if (!is_little_endian)
        ifd_entries = g_ntohs (ifd_entries);
    ifd_entries++;

    /* Tag loop */
    while (ifd_entries)
    {
        if (ifd_entries == 1)
        {
            if (FILE_HAS_DATA_N (file, 4))
                format_utils_add_field (file, IFD_COLOR, TRUE, 4, "IFD end (Exif)", NULL);

            break;
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
        if (tiff_tag == tiff_tags[ExposureTime])
        {
            process_rational_tag (file, exif_tab, "Tag: ExposureTime", "ExposureTime",
                                  "Exposure time, given in seconds",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[FNumber])
        {
            process_rational_tag (file, exif_tab, "Tag: FNumber", "FNumber",
                                  "The focal ratio",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureProgram])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
            const gchar *value_description[] = {
                "Not defined",
                "Manual",
                "Normal program",
                "Aperture priority",
                "Shutter priority",
                "Creative program",
                "Action program",
                "Portrait mode",
                "Landscape mode",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: ExposureProgram", "ExposureProgram",
                     "ExposureProgram\n"
                     "<tt>00 00<sub>16</sub></tt>\tNot defined\n"
                     "<tt>00 01<sub>16</sub></tt>\tManual\n"
                     "<tt>00 02<sub>16</sub></tt>\tNormal program\n"
                     "<tt>00 03<sub>16</sub></tt>\tAperture priority\n"
                     "<tt>00 04<sub>16</sub></tt>\tShutter priority\n"
                     "<tt>00 05<sub>16</sub></tt>\tCreative program\n"
                     "<tt>00 06<sub>16</sub></tt>\tAction program\n"
                     "<tt>00 07<sub>16</sub></tt>\tPortrait mode\n"
                     "<tt>00 08<sub>16</sub></tt>\tLandscape mode",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SpectralSensitivity])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: SpectralSensitivity", "SpectralSensitivity", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PhotographicSensitivity])
        {
            process_short_tag (file, exif_tab, "Tag: PhotographicSensitivity", "PhotographicSensitivity",
                               "Sensitivity of the camera or input device when the image was shot", field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SensitivityType])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
            const gchar *value_description[] = {
                "Unknown",
                "Standard output sensitivity (SOS)",
                "Recommended exposure index (REI)",
                "ISO speed",
                "SOS and REI",
                "SOS and ISO speed",
                "REI and ISO speed",
                "SOS, REI and ISO speed",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: SensitivityType", "SensitivityType",
                     "SensitivityType\n"
                     "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                     "<tt>00 01<sub>16</sub></tt>\tStandard output sensitivity (SOS)\n"
                     "<tt>00 02<sub>16</sub></tt>\tRecommended exposure index (REI)\n"
                     "<tt>00 03<sub>16</sub></tt>\tISO speed\n"
                     "<tt>00 04<sub>16</sub></tt>\tSOS and REI\n"
                     "<tt>00 05<sub>16</sub></tt>\tSOS and ISO speed\n"
                     "<tt>00 06<sub>16</sub></tt>\tREI and ISO speed\n"
                     "<tt>00 07<sub>16</sub></tt>\tSOS, REI and ISO speed",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[StandardOutputSensitivity])
        {
            process_long_tag (file, exif_tab, "Tag: StandardOutputSensitivity", "StandardOutputSensitivity",
                              "Standard output sensitivity value of the camera or input device defined in ISO 12232", field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[RecommendedExposureIndex])
        {
            process_long_tag (file, exif_tab, "Tag: RecommendedExposureIndex", "RecommendedExposureIndex",
                              "Recommended exposure index value of the camera or input device defined in ISO 12232", field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeed])
        {
            process_long_tag (file, exif_tab, "Tag: ISOSpeed", "ISOSpeed",
                              "ISO speed value of the camera or input device defined in ISO 12232", field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudeyyy])
        {
            process_long_tag (file, exif_tab, "Tag: ISOSpeedLatitudeyyy", "ISOSpeedLatitudeyyy",
                              "ISO speed latitude yyy value of the camera or input device defined in ISO 12232", field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudezzz])
        {
            process_long_tag (file, exif_tab, "Tag: ISOSpeedLatitudezzz", "ISOSpeedLatitudezzz",
                              "ISO speed latitude zzz value of the camera or input device defined in ISO 12232", field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExifVersion])
        {
            guint8 values[] = { 0x30, 0x32, 0x32, 0x30,
                                0x30, 0x32, 0x32, 0x31,
                                0x30, 0x32, 0x33, 0x30 };
            const gchar *value_description[] = {
                "Exif v2.2",
                "Exif v2.21",
                "Exif v2.3",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_byte_undefined_tag (file, exif_tab, "Tag: ExifVersion", "ExifVersion",
                                        "ExifVersion\n"
                                        "<tt>30 32 32 30<sub>16</sub></tt>\tExif v2.2\n"
                                        "<tt>30 32 32 31<sub>16</sub></tt>\tExif v2.21\n"
                                        "<tt>30 32 33 30<sub>16</sub></tt>\tExif v2.3",
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[DateTimeOriginal])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: DateTimeOriginal", "DateTimeOriginal", field_type,
                               count, 20, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DateTimeDigitized])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: DateTimeDigitized", "DateTimeDigitized", field_type,
                               count, 20, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ComponentsConfiguration])
        {
            guint8 values[] = { 0x04, 0x05, 0x06, 0x00,
                                0x01, 0x02, 0x03, 0x00 };
            const gchar *value_description[] = {
                "Uncompressed RGB",
                "Other cases",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_byte_undefined_tag (file, exif_tab, "Tag: ComponentsConfiguration", "ComponentsConfiguration",
                                        "ComponentsConfiguration\n"
                                        "<tt>04 05 06 00<sub>16</sub></tt>\tUncompressed RGB\n"
                                        "<tt>01 02 03 00<sub>16</sub></tt>\tOther cases",
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[CompressedBitsPerPixel])
        {
            process_rational_tag (file, exif_tab, "Tag: CompressedBitsPerPixel", "CompressedBitsPerPixel",
                                  "Information specific to compressed data",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ShutterSpeedValue])
        {
            process_rational_tag (file, exif_tab, "Tag: ShutterSpeedValue", "ShutterSpeedValue",
                                  "The shutter speed, using the APEX (Additive System of Photographic Exposure) value",
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ApertureValue])
        {
            process_rational_tag (file, exif_tab, "Tag: ApertureValue", "ApertureValue",
                                  "The lens aperture, using the APEX (Additive System of Photographic Exposure) value",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[BrightnessValue])
        {
            process_rational_tag (file, exif_tab, "Tag: BrightnessValue", "BrightnessValue",
                                  "The brigthness value, using the APEX (Additive System of Photographic Exposure) value",
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureBiasValue])
        {
            process_rational_tag (file, exif_tab, "Tag: ExposureBiasValue", "ExposureBiasValue",
                                  "The exposure bias, using the APEX (Additive System of Photographic Exposure) value",
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MaxApertureValue])
        {
            process_rational_tag (file, exif_tab, "Tag: MaxApertureValue", "MaxApertureValue",
                                  "The smallest F number of the lens, using the APEX (Additive System of Photographic Exposure) value",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectDistance])
        {
            process_rational_tag (file, exif_tab, "Tag: SubjectDistance", "SubjectDistance",
                                  "The distance to the subject, in meters",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MeteringMode])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF };
            const gchar *value_description[] = {
                "Unknown",
                "Average",
                "CenterWeightedAverage",
                "Spot",
                "MultiSpot",
                "Pattern",
                "Partial",
                "Other",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: MeteringMode", "MeteringMode",
                   "MeteringMode\n"
                   "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                   "<tt>00 01<sub>16</sub></tt>\tAverage\n"
                   "<tt>00 02<sub>16</sub></tt>\tCenterWeightedAverage\n"
                   "<tt>00 03<sub>16</sub></tt>\tSpot\n"
                   "<tt>00 04<sub>16</sub></tt>\tMultiSpot\n"
                   "<tt>00 05<sub>16</sub></tt>\tPattern\n"
                   "<tt>00 06<sub>16</sub></tt>\tPartial\n"
                   "<tt>00 FF<sub>16</sub></tt>\tOther",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[LightSource])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x9, 0xA, 0xB, 0xC, 0xD,
                0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0xFF };
            const gchar *value_description[] = {
                "Unknown",
                "Daylight",
                "Fluorescent",
                "Tungsten (incandescent light)",
                "Flash",
                "Fine weather",
                "Cloudy weather",
                "Shade",
                "Daylight fluorescent (D 5700 - 7100K)",
                "Day white fluorescent (N 4600 - 5500K)",
                "Cool white fluorescent (W 3800 - 4500K)",
                "White fluorescent (WW 3250 - 3800K)",
                "Warm white fluorescent (L 2600 - 3250K)",
                "Standard light A",
                "Standard light B",
                "Standard light C",
                "D55",
                "D65",
                "D75",
                "D50",
                "ISO studio tungsten",
                "Other",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: LightSource", "LightSource",
                   "LightSource\n"
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
                   "<tt>00 FF<sub>16</sub></tt>\tOther",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Flash])
        {
            process_short_tag (file, exif_tab, "Tag: Flash", "Flash",
                   "Flash bit fields\n"
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
                   "<tt>1<sub>2</sub></tt>\tRed-eye reduction supported", field_type,
                   SHORT, count, 1, value_offset, is_little_endian,
                   0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[FocalLength])
        {
            process_rational_tag (file, exif_tab, "Tag: FocalLength", "FocalLength",
                                  "Focal length of the lens, in milimeters",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MakerNote])
        {
            process_byte_undefined_tag (file, exif_ascii_tab, "Tag: MakerNote", "MakerNote",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[UserComment])
        {
            process_byte_undefined_tag (file, exif_ascii_tab, "Tag: UserComment", "UserComment",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SubSecTime])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: SubSecTime", "SubSecTime", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeOriginal])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: SubSecTimeOriginal", "SubSecTimeOriginal", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeDigitized])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: SubSecTimeDigitized", "SubSecTimeDigitized", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FlashpixVersion])
        {
            guint8 values[] = { 0x30, 0x31, 0x030, 0x30 };
            const gchar *value_description[] = {
                "Flashpix Format v1.0",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_byte_undefined_tag (file, exif_tab, "Tag: FlashpixVersion", "FlashpixVersion",
                                        "FlashpixVersion\n"
                                        "<tt>30 31 30 30<sub>16</sub></tt>\tFlashpix Format v1.0",
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[ColorSpace])
        {
            guint16 values[] = { 0x1, 0xFFFF };
            const gchar *value_description[] = {
                "sRGB",
                "Uncalibrated",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: ColorSpace", "ColorSpace",
                   "ColorSpace\n"
                   "<tt>00 01<sub>16</sub></tt>\tsRGB\n"
                   "<tt>FF FF<sub>16</sub></tt>\tUncalibrated",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[PixelXDimension])
        {
            if (field_type == SHORT)
                process_short_tag (file, exif_tab, "Tag: PixelXDimension", "PixelXDimension",
                                   "Valid width of the meaningful image", field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, exif_tab, "Tag: PixelXDimension", "PixelXDimension",
                                  "Valid width of the meaningful image", field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[PixelYDimension])
        {
            if (field_type == SHORT)
                process_short_tag (file, exif_tab, "Tag: PixelYDimension", "PixelYDimension",
                                   "Valid height of the meaningful image", field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, exif_tab, "Tag: PixelYDimension", "PixelYDimension",
                                  "Valid height of the meaningful image", field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[RelatedSoundFile])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: RelatedSoundFile", "RelatedSoundFile", field_type,
                               count, 13, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FocalPlaneResolutionUnit])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                "No unit",
                "Inch",
                "Centimeter",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: FocalPlaneResolutionUnit", "FocalPlaneResolutionUnit",
                   "FocalPlaneResolutionUnit\n"
                   "<tt>00 01<sub>16</sub></tt>\tNo unit\n"
                   "<tt>00 02<sub>16</sub></tt>\tInch\n"
                   "<tt>00 03<sub>16</sub></tt>\tCentimeter",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectLocation])
        {
            process_short_tag (file, exif_tab, "Tag: SubjectLocation", "SubjectLocation",
                               "X column number - Y row number", field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureIndex])
        {
            process_rational_tag (file, exif_tab, "Tag: ExposureIndex", "ExposureIndex",
                                  "Indicates the exposure index selected on the camera or input device",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[SensingMethod])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8 };
            const gchar *value_description[] = {
                "Not defined",
                "One-chip color area sensor",
                "Two-chip color area sensor",
                "Three-chip color area sensor",
                "Color sequential area sensor",
                "Trilinear sensor",
                "Color sequential linear sensor",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: SensingMethod", "SensingMethod",
                   "SensingMethod\n"
                   "<tt>00 01<sub>16</sub></tt>\tNot defined\n"
                   "<tt>00 02<sub>16</sub></tt>\tOne-chip color area sensor\n"
                   "<tt>00 03<sub>16</sub></tt>\tTwo-chip color area sensor\n"
                   "<tt>00 04<sub>16</sub></tt>\tThree-chip color area sensor\n"
                   "<tt>00 05<sub>16</sub></tt>\tColor sequential area sensor\n"
                   "<tt>00 07<sub>16</sub></tt>\tTrilinear sensor\n"
                   "<tt>00 08<sub>16</sub></tt>\tColor sequential linear sensor",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[FileSource])
        {
            guint8 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                "Others",
                "Scanner of transparent type",
                "Scanner of reflex type",
                "DSC",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_byte_undefined_tag (file, exif_tab, "Tag: FileSource", "FileSource",
                                        "FileSource\n"
                                        "<tt>00<sub>16</sub></tt>\tOthers\n"
                                        "<tt>01<sub>16</sub></tt>\tScanner of transparent type\n"
                                        "<tt>02<sub>16</sub></tt>\tScanner of reflex type\n"
                                        "<tt>03<sub>16</sub></tt>\tDSC",
                                        field_type, UNDEFINED, count, 1, value_offset, is_little_endian,
                                        sizeof (values), values, value_description);
        }
        else if (tiff_tag == tiff_tags[SceneType])
        {
            guint8 values[] = { 0x1 };
            const gchar *value_description[] = {
                "A directly photographed image",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_byte_undefined_tag (file, exif_tab, "Tag: SceneType", "SceneType",
                                        "SceneType\n"
                                        "<tt>01<sub>16</sub></tt>\tA directly photographed image",
                                        field_type, UNDEFINED, count, 1, value_offset, is_little_endian,
                                        sizeof (values), values, value_description);
        }
        else if (tiff_tag == tiff_tags[CFAPattern])
        {
            process_byte_undefined_tag (file, exif_tab, "Tag: CFAPattern", "CFAPattern",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[CustomRendered])
        {
            guint16 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                "Normal process",
                "Custom process",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: CustomRendered", "CustomRendered",
                   "CustomRendered\n"
                   "<tt>00 00<sub>16</sub></tt>\tNormal process\n"
                   "<tt>00 01<sub>16</sub></tt>\tCustom process",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureMode])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                "Auto exposure",
                "Manual exposure",
                "Auto bracket",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: ExposureMode", "ExposureMode",
                   "ExposureMode\n"
                   "<tt>00 00<sub>16</sub></tt>\tAuto exposure\n"
                   "<tt>00 01<sub>16</sub></tt>\tManual exposure\n"
                   "<tt>00 02<sub>16</sub></tt>\tAuto bracket",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[WhiteBalance])
        {
            guint16 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                "Auto white balance",
                "Manual white balance",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: WhiteBalance", "WhiteBalance",
                   "WhiteBalance\n"
                   "<tt>00 00<sub>16</sub></tt>\tAuto white balance\n"
                   "<tt>00 01<sub>16</sub></tt>\tManual white balance",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[DigitalZoomRatio])
        {
            process_rational_tag (file, exif_tab, "Tag: DigitalZoomRatio", "DigitalZoomRatio",
                                  "Digital zoom ratio when the image was shot",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[FocalLengthIn35mmFilm])
        {
            process_short_tag (file, exif_tab, "Tag: FocalLengthIn35mmFilm", "FocalLengthIn35mmFilm",
                               "Focal length assuming a 35mm film camera, in milimeters", field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SceneCaptureType])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                "Standard",
                "Landspace",
                "Portrait",
                "Night scene",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: SceneCaptureType", "SceneCaptureType",
                   "SceneCaptureType\n"
                   "<tt>00 00<sub>16</sub></tt>\tStandard\n"
                   "<tt>00 01<sub>16</sub></tt>\tLandscape\n"
                   "<tt>00 02<sub>16</sub></tt>\tPortrait\n"
                   "<tt>00 03<sub>16</sub></tt>\tNight scene",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[GainControl])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            const gchar *value_description[] = {
                "None",
                "Low gain up",
                "High gain up",
                "Low gain down",
                "High gain down",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: GainControl", "GainControl",
                   "GainControl\n"
                   "<tt>00 00<sub>16</sub></tt>\tNone\n"
                   "<tt>00 01<sub>16</sub></tt>\tLow gain up\n"
                   "<tt>00 02<sub>16</sub></tt>\tHigh gain up\n"
                   "<tt>00 03<sub>16</sub></tt>\tLow gain down\n"
                   "<tt>00 04<sub>16</sub></tt>\tHigh gain down",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Contrast])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                "Normal",
                "Soft",
                "Hard",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: Contrast", "Contrast",
                   "Contrast\n"
                   "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                   "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                   "<tt>00 02<sub>16</sub></tt>\tHard",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Saturation])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                "Normal",
                "Low saturation",
                "High saturation",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: Saturation", "Saturation",
                   "Saturation\n"
                   "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                   "<tt>00 01<sub>16</sub></tt>\tLow saturation\n"
                   "<tt>00 02<sub>16</sub></tt>\tHigh saturation",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Sharpness])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                "Normal",
                "Soft",
                "Hard",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: Sharpness", "Sharpness",
                   "Sharpness\n"
                   "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                   "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                   "<tt>00 02<sub>16</sub></tt>\tHard",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectDistanceRange])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                "Unknown",
                "Macro",
                "Close view",
                "Distant view",
                "<span foreground=\"red\">INVALID</span>"
            };
            process_short_tag (file, exif_tab, "Tag: SubjectDistanceRange", "SubjectDistanceRange",
                   "SubjectDistanceRange\n"
                   "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                   "<tt>00 01<sub>16</sub></tt>\tMacro\n"
                   "<tt>00 02<sub>16</sub></tt>\tClose view\n"
                   "<tt>00 03<sub>16</sub></tt>\tDistant view",
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageUniqueID])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: ImageUniqueID", "ImageUniqueID", field_type,
                               count, 33, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CameraOwnerName])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: CameraOwnerName", "CameraOwnerName", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[BodySerialNumber])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: BodySerialNumber", "BodySerialNumber", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensSpecification])
        {
            const gchar *value_names[] = {
                "Minimum focal length",
                "Maximum focal length",
                "Minimum F number in the minimum focal length",
                "Minimum F number in the maximum focal length"
            };
            process_rational_tag (file, exif_tab, "Tag: LensSpecification", "LensSpecification",
                                  "Minimum and maximum focal length in milimeters, unknown minimum F number = 0/0",
                                  value_names, field_type, RATIONAL, count, 4, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[LensMake])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: LensMake", "LensMake", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensModel])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: LensModel", "LensModel", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensSerialNumber])
        {
            process_ascii_tag (file, exif_ascii_tab, "Tag: LensSerialNumber", "LensSerialNumber", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Gamma])
        {
            process_rational_tag (file, exif_tab, "Tag: Gamma", "Gamma",
                                  "Value of coefficient gamma",
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, "Tag: Unknown", NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 2, "Field type", NULL);
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, "Count", NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 4, "Tag value or offset", NULL);

            if (unknown_tag_count > 10 && ifd_entries > 100)
                return FALSE;

            unknown_tag_count++;
        }

        ifd_entries--;
    }

    return TRUE;
}
