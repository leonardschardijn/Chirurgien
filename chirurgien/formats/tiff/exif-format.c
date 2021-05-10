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

#include <config.h>

#include "exif-format.h"

#include <glib/gi18n.h>

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
                              _("Number of directory entries (Exif)"), "Exif IFD");
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
                format_utils_add_field (file, IFD_COLOR, TRUE, 4, _("IFD end (Exif)"), NULL);

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
            process_rational_tag (file, exif_tab, _("Tag: ExposureTime"), "ExposureTime",
                                  _("Exposure time, given in seconds"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[FNumber])
        {
            process_rational_tag (file, exif_tab, _("Tag: FNumber"), "FNumber",
                                  _("The focal ratio"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureProgram])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
            const gchar *value_description[] = {
                _("Not defined"),
                _("Manual"),
                _("Normal program"),
                _("Aperture priority"),
                _("Shutter priority"),
                _("Creative program"),
                _("Action program"),
                _("Portrait mode"),
                _("Landscape mode"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: ExposureProgram"), "ExposureProgram",
                   _("ExposureProgram\n"
                     "<tt>00 00<sub>16</sub></tt>\tNot defined\n"
                     "<tt>00 01<sub>16</sub></tt>\tManual\n"
                     "<tt>00 02<sub>16</sub></tt>\tNormal program\n"
                     "<tt>00 03<sub>16</sub></tt>\tAperture priority\n"
                     "<tt>00 04<sub>16</sub></tt>\tShutter priority\n"
                     "<tt>00 05<sub>16</sub></tt>\tCreative program\n"
                     "<tt>00 06<sub>16</sub></tt>\tAction program\n"
                     "<tt>00 07<sub>16</sub></tt>\tPortrait mode\n"
                     "<tt>00 08<sub>16</sub></tt>\tLandscape mode"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SpectralSensitivity])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: SpectralSensitivity"), "SpectralSensitivity", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[PhotographicSensitivity])
        {
            process_short_tag (file, exif_tab, _("Tag: PhotographicSensitivity"), "PhotographicSensitivity",
                               _("Sensitivity of the camera or input device when the image was shot"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SensitivityType])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
            const gchar *value_description[] = {
                _("Unknown"),
                _("Standard output sensitivity (SOS)"),
                _("Recommended exposure index (REI)"),
                _("ISO speed"),
                _("SOS and REI"),
                _("SOS and ISO speed"),
                _("REI and ISO speed"),
                _("SOS, REI and ISO speed"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: SensitivityType"), "SensitivityType",
                   _("SensitivityType\n"
                     "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                     "<tt>00 01<sub>16</sub></tt>\tStandard output sensitivity (SOS)\n"
                     "<tt>00 02<sub>16</sub></tt>\tRecommended exposure index (REI)\n"
                     "<tt>00 03<sub>16</sub></tt>\tISO speed\n"
                     "<tt>00 04<sub>16</sub></tt>\tSOS and REI\n"
                     "<tt>00 05<sub>16</sub></tt>\tSOS and ISO speed\n"
                     "<tt>00 06<sub>16</sub></tt>\tREI and ISO speed\n"
                     "<tt>00 07<sub>16</sub></tt>\tSOS, REI and ISO speed"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[StandardOutputSensitivity])
        {
            process_long_tag (file, exif_tab, _("Tag: StandardOutputSensitivity"), "StandardOutputSensitivity",
                              _("Standard output sensitivity value of the camera or input device defined in ISO 12232"), field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[RecommendedExposureIndex])
        {
            process_long_tag (file, exif_tab, _("Tag: RecommendedExposureIndex"), "RecommendedExposureIndex",
                              _("Recommended exposure index value of the camera or input device defined in ISO 12232"), field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeed])
        {
            process_long_tag (file, exif_tab, _("Tag: ISOSpeed"), "ISOSpeed",
                              _("ISO speed value of the camera or input device defined in ISO 12232"), field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudeyyy])
        {
            process_long_tag (file, exif_tab, _("Tag: ISOSpeedLatitudeyyy"), "ISOSpeedLatitudeyyy",
                              _("ISO speed latitude yyy value of the camera or input device defined in ISO 12232"), field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ISOSpeedLatitudezzz])
        {
            process_long_tag (file, exif_tab, _("Tag: ISOSpeedLatitudezzz"), "ISOSpeedLatitudezzz",
                              _("ISO speed latitude zzz value of the camera or input device defined in ISO 12232"), field_type,
                              LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExifVersion])
        {
            guint8 values[] = { 0x30, 0x32, 0x32, 0x30,
                                0x30, 0x32, 0x32, 0x31,
                                0x30, 0x32, 0x33, 0x30 };
            const gchar *value_description[] = {
                _("Exif v2.2"),
                _("Exif v2.21"),
                _("Exif v2.3"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, exif_tab, _("Tag: ExifVersion"), "ExifVersion",
                                        _("ExifVersion\n"
                                          "<tt>30 32 32 30<sub>16</sub></tt>\tExif v2.2\n"
                                          "<tt>30 32 32 31<sub>16</sub></tt>\tExif v2.21\n"
                                          "<tt>30 32 33 30<sub>16</sub></tt>\tExif v2.3"),
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[DateTimeOriginal])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: DateTimeOriginal"), "DateTimeOriginal", field_type,
                               count, 20, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[DateTimeDigitized])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: DateTimeDigitized"), "DateTimeDigitized", field_type,
                               count, 20, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[ComponentsConfiguration])
        {
            guint8 values[] = { 0x04, 0x05, 0x06, 0x00,
                                0x01, 0x02, 0x03, 0x00 };
            const gchar *value_description[] = {
                _("Uncompressed RGB"),
                _("Other cases"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, exif_tab, _("Tag: ComponentsConfiguration"), "ComponentsConfiguration",
                                        _("ComponentsConfiguration\n"
                                          "<tt>04 05 06 00<sub>16</sub></tt>\tUncompressed RGB\n"
                                          "<tt>01 02 03 00<sub>16</sub></tt>\tOther cases"),
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[CompressedBitsPerPixel])
        {
            process_rational_tag (file, exif_tab, _("Tag: CompressedBitsPerPixel"), "CompressedBitsPerPixel",
                                  _("Information specific to compressed data"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ShutterSpeedValue])
        {
            process_rational_tag (file, exif_tab, _("Tag: ShutterSpeedValue"), "ShutterSpeedValue",
                                  _("The shutter speed, using the APEX (Additive System of Photographic Exposure) value"),
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ApertureValue])
        {
            process_rational_tag (file, exif_tab, _("Tag: ApertureValue"), "ApertureValue",
                                  _("The lens aperture, using the APEX (Additive System of Photographic Exposure) value"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[BrightnessValue])
        {
            process_rational_tag (file, exif_tab, _("Tag: BrightnessValue"), "BrightnessValue",
                                  _("The brigthness value, using the APEX (Additive System of Photographic Exposure) value"),
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureBiasValue])
        {
            process_rational_tag (file, exif_tab, _("Tag: ExposureBiasValue"), "ExposureBiasValue",
                                  _("The exposure bias, using the APEX (Additive System of Photographic Exposure) value"),
                                  NULL, field_type, SRATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MaxApertureValue])
        {
            process_rational_tag (file, exif_tab, _("Tag: MaxApertureValue"), "MaxApertureValue",
                                  _("The smallest F number of the lens, using the APEX (Additive System of Photographic Exposure) value"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectDistance])
        {
            process_rational_tag (file, exif_tab, _("Tag: SubjectDistance"), "SubjectDistance",
                                  _("The distance to the subject, in meters"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MeteringMode])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF };
            const gchar *value_description[] = {
                _("Unknown"),
                _("Average"),
                _("CenterWeightedAverage"),
                _("Spot"),
                _("MultiSpot"),
                _("Pattern"),
                _("Partial"),
                _("Other"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: MeteringMode"), "MeteringMode",
                   _("MeteringMode\n"
                     "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                     "<tt>00 01<sub>16</sub></tt>\tAverage\n"
                     "<tt>00 02<sub>16</sub></tt>\tCenterWeightedAverage\n"
                     "<tt>00 03<sub>16</sub></tt>\tSpot\n"
                     "<tt>00 04<sub>16</sub></tt>\tMultiSpot\n"
                     "<tt>00 05<sub>16</sub></tt>\tPattern\n"
                     "<tt>00 06<sub>16</sub></tt>\tPartial\n"
                     "<tt>00 FF<sub>16</sub></tt>\tOther"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[LightSource])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x9, 0xA, 0xB, 0xC, 0xD,
                0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0xFF };
            const gchar *value_description[] = {
                _("Unknown"),
                _("Daylight"),
                _("Fluorescent"),
                _("Tungsten (incandescent light)"),
                _("Flash"),
                _("Fine weather"),
                _("Cloudy weather"),
                _("Shade"),
                _("Daylight fluorescent (D 5700 - 7100K)"),
                _("Day white fluorescent (N 4600 - 5500K)"),
                _("Cool white fluorescent (W 3800 - 4500K)"),
                _("White fluorescent (WW 3250 - 3800K)"),
                _("Warm white fluorescent (L 2600 - 3250K)"),
                _("Standard light A"),
                _("Standard light B"),
                _("Standard light C"),
                "D55",
                "D65",
                "D75",
                "D50",
                _("ISO studio tungsten"),
                _("Other"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: LightSource"), "LightSource",
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
                     "<tt>00 FF<sub>16</sub></tt>\tOther"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Flash])
        {
            process_short_tag (file, exif_tab, _("Tag: Flash"), "Flash",
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
                     "<tt>1<sub>2</sub></tt>\tRed-eye reduction supported"), field_type,
                   SHORT, count, 1, value_offset, is_little_endian,
                   0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[FocalLength])
        {
            process_rational_tag (file, exif_tab, _("Tag: FocalLength"), "FocalLength",
                                  _("Focal length of the lens, in milimeters"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[MakerNote])
        {
            process_byte_undefined_tag (file, exif_ascii_tab, _("Tag: MakerNote"), "MakerNote",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[UserComment])
        {
            process_byte_undefined_tag (file, exif_ascii_tab, _("Tag: UserComment"), "UserComment",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SubSecTime])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: SubSecTime"), "SubSecTime", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeOriginal])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: SubSecTimeOriginal"), "SubSecTimeOriginal", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[SubSecTimeDigitized])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: SubSecTimeDigitized"), "SubSecTimeDigitized", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FlashpixVersion])
        {
            guint8 values[] = { 0x30, 0x31, 0x030, 0x30 };
            const gchar *value_description[] = {
                _("Flashpix Format v1.0"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, exif_tab, _("Tag: FlashpixVersion"), "FlashpixVersion",
                                        _("FlashpixVersion\n"
                                          "<tt>30 31 30 30<sub>16</sub></tt>\tFlashpix Format v1.0"),
                                        field_type, UNDEFINED, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[ColorSpace])
        {
            guint16 values[] = { 0x1, 0xFFFF };
            const gchar *value_description[] = {
                _("sRGB"),
                _("Uncalibrated"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: ColorSpace"), "ColorSpace",
                   _("ColorSpace\n"
                     "<tt>00 01<sub>16</sub></tt>\tsRGB\n"
                     "<tt>FF FF<sub>16</sub></tt>\tUncalibrated"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[PixelXDimension])
        {
            if (field_type == SHORT)
                process_short_tag (file, exif_tab, _("Tag: PixelXDimension"), "PixelXDimension",
                                   _("Valid width of the meaningful image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, exif_tab, _("Tag: PixelXDimension"), "PixelXDimension",
                                  _("Valid width of the meaningful image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[PixelYDimension])
        {
            if (field_type == SHORT)
                process_short_tag (file, exif_tab, _("Tag: PixelYDimension"), "PixelYDimension",
                                   _("Valid height of the meaningful image"), field_type,
                                   SHORT, count, 1, value_offset, is_little_endian,
                                   0, NULL, NULL, NULL);
            else
                process_long_tag (file, exif_tab, _("Tag: PixelYDimension"), "PixelYDimension",
                                  _("Valid height of the meaningful image"), field_type,
                                  LONG, count, 1, value_offset, is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[RelatedSoundFile])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: RelatedSoundFile"), "RelatedSoundFile", field_type,
                               count, 13, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[FocalPlaneResolutionUnit])
        {
            guint16 values[] = { 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                _("No unit"),
                _("Inch"),
                _("Centimeter"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: FocalPlaneResolutionUnit"), "FocalPlaneResolutionUnit",
                   _("FocalPlaneResolutionUnit\n"
                     "<tt>00 01<sub>16</sub></tt>\tNo unit\n"
                     "<tt>00 02<sub>16</sub></tt>\tInch\n"
                     "<tt>00 03<sub>16</sub></tt>\tCentimeter"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectLocation])
        {
            process_short_tag (file, exif_tab, _("Tag: SubjectLocation"), "SubjectLocation",
                               _("X column number - Y row number"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureIndex])
        {
            process_rational_tag (file, exif_tab, _("Tag: ExposureIndex"), "ExposureIndex",
                                  _("Indicates the exposure index selected on the camera or input device"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[SensingMethod])
        {
            guint16 values[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x7, 0x8 };
            const gchar *value_description[] = {
                _("Not defined"),
                _("One-chip color area sensor"),
                _("Two-chip color area sensor"),
                _("Three-chip color area sensor"),
                _("Color sequential area sensor"),
                _("Trilinear sensor"),
                _("Color sequential linear sensor"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: SensingMethod"), "SensingMethod",
                   _("SensingMethod\n"
                     "<tt>00 01<sub>16</sub></tt>\tNot defined\n"
                     "<tt>00 02<sub>16</sub></tt>\tOne-chip color area sensor\n"
                     "<tt>00 03<sub>16</sub></tt>\tTwo-chip color area sensor\n"
                     "<tt>00 04<sub>16</sub></tt>\tThree-chip color area sensor\n"
                     "<tt>00 05<sub>16</sub></tt>\tColor sequential area sensor\n"
                     "<tt>00 07<sub>16</sub></tt>\tTrilinear sensor\n"
                     "<tt>00 08<sub>16</sub></tt>\tColor sequential linear sensor"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[FileSource])
        {
            guint8 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                _("Others"),
                _("Scanner of transparent type"),
                _("Scanner of reflex type"),
                _("DSC"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, exif_tab, _("Tag: FileSource"), "FileSource",
                                        _("FileSource\n"
                                          "<tt>00<sub>16</sub></tt>\tOthers\n"
                                          "<tt>01<sub>16</sub></tt>\tScanner of transparent type\n"
                                          "<tt>02<sub>16</sub></tt>\tScanner of reflex type\n"
                                          "<tt>03<sub>16</sub></tt>\tDSC"),
                                        field_type, UNDEFINED, count, 1, value_offset, is_little_endian,
                                        sizeof (values), values, value_description);
        }
        else if (tiff_tag == tiff_tags[SceneType])
        {
            guint8 values[] = { 0x1 };
            const gchar *value_description[] = {
                _("A directly photographed image"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, exif_tab, _("Tag: SceneType"), "SceneType",
                                        _("SceneType\n"
                                          "<tt>01<sub>16</sub></tt>\tA directly photographed image"),
                                        field_type, UNDEFINED, count, 1, value_offset, is_little_endian,
                                        sizeof (values), values, value_description);
        }
        else if (tiff_tag == tiff_tags[CFAPattern])
        {
            process_byte_undefined_tag (file, exif_tab, _("Tag: CFAPattern"), "CFAPattern",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[CustomRendered])
        {
            guint16 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                _("Normal process"),
                _("Custom process"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: CustomRendered"), "CustomRendered",
                   _("CustomRendered\n"
                     "<tt>00 00<sub>16</sub></tt>\tNormal process\n"
                     "<tt>00 01<sub>16</sub></tt>\tCustom process"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ExposureMode])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Auto exposure"),
                _("Manual exposure"),
                _("Auto bracket"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: ExposureMode"), "ExposureMode",
                   _("ExposureMode\n"
                     "<tt>00 00<sub>16</sub></tt>\tAuto exposure\n"
                     "<tt>00 01<sub>16</sub></tt>\tManual exposure\n"
                     "<tt>00 02<sub>16</sub></tt>\tAuto bracket"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[WhiteBalance])
        {
            guint16 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                _("Auto white balance"),
                _("Manual white balance"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: WhiteBalance"), "WhiteBalance",
                   _("WhiteBalance\n"
                     "<tt>00 00<sub>16</sub></tt>\tAuto white balance\n"
                     "<tt>00 01<sub>16</sub></tt>\tManual white balance"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[DigitalZoomRatio])
        {
            process_rational_tag (file, exif_tab, _("Tag: DigitalZoomRatio"), "DigitalZoomRatio",
                                  _("Digital zoom ratio when the image was shot"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[FocalLengthIn35mmFilm])
        {
            process_short_tag (file, exif_tab, _("Tag: FocalLengthIn35mmFilm"), "FocalLengthIn35mmFilm",
                               _("Focal length assuming a 35mm film camera, in milimeters"), field_type,
                               SHORT, count, 1, value_offset, is_little_endian,
                               0, NULL, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[SceneCaptureType])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                _("Standard"),
                _("Landspace"),
                _("Portrait"),
                _("Night scene"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: SceneCaptureType"), "SceneCaptureType",
                   _("SceneCaptureType\n"
                     "<tt>00 00<sub>16</sub></tt>\tStandard\n"
                     "<tt>00 01<sub>16</sub></tt>\tLandscape\n"
                     "<tt>00 02<sub>16</sub></tt>\tPortrait\n"
                     "<tt>00 03<sub>16</sub></tt>\tNight scene"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[GainControl])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3, 0x4 };
            const gchar *value_description[] = {
                _("None"),
                _("Low gain up"),
                _("High gain up"),
                _("Low gain down"),
                _("High gain down"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: GainControl"), "GainControl",
                   _("GainControl\n"
                     "<tt>00 00<sub>16</sub></tt>\tNone\n"
                     "<tt>00 01<sub>16</sub></tt>\tLow gain up\n"
                     "<tt>00 02<sub>16</sub></tt>\tHigh gain up\n"
                     "<tt>00 03<sub>16</sub></tt>\tLow gain down\n"
                     "<tt>00 04<sub>16</sub></tt>\tHigh gain down"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Contrast])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Normal"),
                _("Soft"),
                _("Hard"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: Contrast"), "Contrast",
                   _("Contrast\n"
                     "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                     "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                     "<tt>00 02<sub>16</sub></tt>\tHard"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Saturation])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Normal"),
                _("Low saturation"),
                _("High saturation"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: Saturation"), "Saturation",
                   _("Saturation\n"
                     "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                     "<tt>00 01<sub>16</sub></tt>\tLow saturation\n"
                     "<tt>00 02<sub>16</sub></tt>\tHigh saturation"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[Sharpness])
        {
            guint16 values[] = { 0x0, 0x1, 0x2 };
            const gchar *value_description[] = {
                _("Normal"),
                _("Soft"),
                _("Hard"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: Sharpness"), "Sharpness",
                   _("Sharpness\n"
                     "<tt>00 00<sub>16</sub></tt>\tNormal\n"
                     "<tt>00 01<sub>16</sub></tt>\tSoft\n"
                     "<tt>00 02<sub>16</sub></tt>\tHard"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[SubjectDistanceRange])
        {
            guint16 values[] = { 0x0, 0x1, 0x2, 0x3 };
            const gchar *value_description[] = {
                _("Unknown"),
                _("Macro"),
                _("Close view"),
                _("Distant view"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, exif_tab, _("Tag: SubjectDistanceRange"), "SubjectDistanceRange",
                   _("SubjectDistanceRange\n"
                     "<tt>00 00<sub>16</sub></tt>\tUnknown\n"
                     "<tt>00 01<sub>16</sub></tt>\tMacro\n"
                     "<tt>00 02<sub>16</sub></tt>\tClose view\n"
                     "<tt>00 03<sub>16</sub></tt>\tDistant view"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[ImageUniqueID])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: ImageUniqueID"), "ImageUniqueID", field_type,
                               count, 33, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[CameraOwnerName])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: CameraOwnerName"), "CameraOwnerName", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[BodySerialNumber])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: BodySerialNumber"), "BodySerialNumber", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensSpecification])
        {
            const gchar *value_names[] = {
                _("Minimum focal length"),
                _("Maximum focal length"),
                _("Minimum F number in the minimum focal length"),
                _("Minimum F number in the maximum focal length")
            };
            process_rational_tag (file, exif_tab, _("Tag: LensSpecification"), "LensSpecification",
                                  _("Minimum and maximum focal length in milimeters, unknown minimum F number = 0/0"),
                                  value_names, field_type, RATIONAL, count, 4, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[LensMake])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: LensMake"), "LensMake", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensModel])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: LensModel"), "LensModel", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[LensSerialNumber])
        {
            process_ascii_tag (file, exif_ascii_tab, _("Tag: LensSerialNumber"), "LensSerialNumber", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[Gamma])
        {
            process_rational_tag (file, exif_tab, _("Tag: Gamma"), "Gamma",
                                  _("Value of coefficient gamma"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2, _("Tag: Unknown"), NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 2, _("Field type"), NULL);
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4, _("Count"), NULL);
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 4, _("Tag value or offset"), NULL);

            if (unknown_tag_count > 10 && ifd_entries > 100)
                return FALSE;

            unknown_tag_count++;
        }

        ifd_entries--;
    }

    return TRUE;
}
