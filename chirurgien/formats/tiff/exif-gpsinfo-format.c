/* exif-gpsinfo-format.c
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

#include "exif-gpsinfo-format.h"

#include <glib/gi18n.h>

#include "chirurgien-exif.h"


gboolean
chirurgien_gpsinfo (FormatsFile    *file,
                    gboolean        is_little_endian,
                    DescriptionTab *gpsinfo_tab,
                    DescriptionTab *gpsinfo_ascii_tab)
{
    const guint16 tiff_tags[EXIF_GPSINFO_TAGS] =
    {
        0x0000, // GPSVersionID
        0x0001, // GPSLatitudeRef
        0x0002, // GPSLatitude
        0x0003, // GPSLongitudeRef
        0x0004, // GPSLongitude
        0x0005, // GPSAltitudeRef
        0x0006, // GPSAltitude
        0x0007, // GPSTimeStamp
        0x0008, // GPSSatellites
        0x0009, // GPSStatus
        0x000A, // GPSMeasureMode
        0x000B, // GPSDOP
        0x000C, // GPSSpeedRef
        0x000D, // GPSSpeed
        0x000E, // GPSTrackRef
        0x000F, // GPSTrack
        0x0010, // GPSImgDirectionRef
        0x0011, // GPSImgDirection
        0x0012, // GPSMapDatum
        0x0013, // GPSDestLatitudeRef
        0x0014, // GPSDestLatitude
        0x0015, // GPSDestLongitudeRef
        0x0016, // GPSDestLongitude
        0x0017, // GPSDestBearingRef
        0x0018, // GPSDestBearing
        0x0019, // GPSDestDistanceRef
        0x001A, // GPSDestDistance
        0x001B, // GPSProcessingMethod
        0x001C, // GPSAreaInformation
        0x001D, // GPSDateStamp
        0x001E, // GPSDifferential
        0x001F  // GPSHPositioningError
    };

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    guint unknown_tag_count = 0;

    if (FILE_HAS_DATA_N (file, 2))
    {
        format_utils_read (file, &ifd_entries, 2);
        format_utils_add_field (file, IFD_COLOR, TRUE, 2,
                              _("Number of directory entries (GPSInfo)"), "GPSInfo IFD");
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
                format_utils_add_field (file, IFD_COLOR, TRUE, 4, _("IFD end (GPSInfo)"), NULL);

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
        if (tiff_tag == tiff_tags[GPSVersionID])
        {
            guint8 values[] = { 0x2, 0x3, 0x0, 0x0 };
            const gchar *value_description[] = {
                _("v2.3"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, gpsinfo_tab, _("Tag: GPSVersionID"), "GPSVersionID",
                                        _("GPSVersionID\n"
                                          "<tt>02 03 00 00<sub>16</sub></tt>\tv2.3"),
                                        field_type, BYTE, count, 4, value_offset, is_little_endian,
                                        sizeof (values) >> 2, values, value_description);
        }
        else if (tiff_tag == tiff_tags[GPSLatitudeRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSLatitudeRef"), "GPSLatitudeRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSLatitude])
        {
            const gchar *value_names[] = {
                _("GPSLatitude degrees"),
                _("GPSLatitude minutes"),
                _("GPSLatitude seconds")
            };
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSLatitude"), "GPSLatitude",
                                  _("Indicates the latitude"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSLongitudeRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSLongitudeRef"), "GPSLongitudeRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSLongitude])
        {
            const gchar *value_names[] = {
                _("GPSLongitude degrees"),
                _("GPSLongitude minutes"),
                _("GPSLongitude seconds")
            };
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSLongitude"), "GPSLongitude",
                                  _("Indicates the longitude"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSAltitudeRef])
        {
            guint8 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                _("Sea level"),
                _("Sea level reference (negative value)"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_byte_undefined_tag (file, gpsinfo_tab, _("Tag: GPSAltitudeRef"), "GPSAltitudeRef",
                                        _("GPSAltitudeRef\n"
                                          "<tt>00<sub>16</sub></tt>\tSea level\n"
                                          "<tt>01<sub>16</sub></tt>\tSea level reference (negative value)"),
                                        field_type, BYTE, count, 1, value_offset, is_little_endian,
                                        sizeof (values), values, value_description);
        }
        else if (tiff_tag == tiff_tags[GPSAltitude])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSAltitude"), "GPSAltitude",
                                  _("Indicates the altitude based on the reference in GPSAltitudeRef"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSTimeStamp])
        {
            const gchar *value_names[] = {
                _("Hour"),
                _("Minutes"),
                _("Seconds")
            };
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSTimeStamp"), "GPSTimeStamp",
                                  _("Indicates the time as UTC (Coordinated Universal Time)"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSSatellites])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSSatellites"), "GPSSatellites", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSStatus])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSStatus"), "GPSStatus", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSMeasureMode])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSMeasureMode"), "GPSMeasureMode", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDOP])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSDOP"), "GPSDOP",
                                  _("Indicates the GPS DOP (data degree of precision)"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSSpeedRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSSpeedRef"), "GPSSpeedRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSSpeed])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSSpeed"), "GPSSpeed",
                                  _("Indicates the speed of GPS receiver movement"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSTrackRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSTrackRef"), "GPSTrackRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSTrack])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSTrack"), "GPSTrack",
                                  _("Indicates the direction of GPS receiver movement"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSImgDirectionRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSImgDirectionRef"), "GPSImgDirectionRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSImgDirection])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSImgDirection"), "GPSImgDirection",
                                  _("Indicates the reference for giving the direction of the image when it is captured"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSMapDatum])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSMapDatum"), "GPSMapDatum", field_type,
                               count, 0, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDestLatitudeRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSDestLatitudeRef"), "GPSDestLatitudeRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDestLatitude])
        {
            const gchar *value_names[] = {
                _("GPSDestLatitude degrees"),
                _("GPSDestLatitude minutes"),
                _("GPSDestLatitude seconds")
            };
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSDestLatitude"), "GPSDestLatitude",
                                  _("Indicates the latitude of the destination point"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSDestLongitudeRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSDestLongitudeRef"), "GPSDestLongitudeRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDestLongitude])
        {
            const gchar *value_names[] = {
                _("GPSDestLongitude degrees"),
                _("GPSDestLongitude minutes"),
                _("GPSDestLongitude seconds")
            };
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSDestLongitude"), "GPSDestLongitude",
                                  _("Indicates the longitude of the destination point"),
                                  value_names, field_type, RATIONAL, count, 3, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSDestBearingRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSDestBearingRef"), "GPSDestBearingRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDestBearing])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSDestBearing"), "GPSDestBearing",
                                  _("Indicates the bearing to the destination point"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSDestDistanceRef])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSDestDistanceRef"), "GPSDestDistanceRef", field_type,
                               count, 2, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDestDistance])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSDestDistance"), "GPSDestDistance",
                                  _("Indicates the distance to the destination point"),
                                  NULL, field_type, RATIONAL, count, 1, value_offset,
                                  is_little_endian, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSProcessingMethod])
        {
            process_byte_undefined_tag (file, gpsinfo_ascii_tab, _("Tag: GPSProcessingMethod"), "GPSProcessingMethod",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSAreaInformation])
        {
            process_byte_undefined_tag (file, gpsinfo_ascii_tab, _("Tag: GPSAreaInformation"), "GPSAreaInformation",
                                        NULL, field_type, UNDEFINED, count, 0, value_offset, is_little_endian,
                                        0, NULL, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSDateStamp])
        {
            process_ascii_tag (file, gpsinfo_ascii_tab, _("Tag: GPSDateStamp"), "GPSDateStamp", field_type,
                               count, 11, value_offset, is_little_endian);
        }
        else if (tiff_tag == tiff_tags[GPSDifferential])
        {
            guint16 values[] = { 0x0, 0x1 };
            const gchar *value_description[] = {
                _("Unknown"),
                _("Macro"),
                _("Close view"),
                _("Distant view"),
                _("<span foreground=\"red\">INVALID</span>")
            };
            process_short_tag (file, gpsinfo_tab, _("Tag: GPSDifferential"), "GPSDifferential",
                   _("GPSDifferential\n"
                     "<tt>00 00<sub>16</sub></tt>\tMeasurement without differential correction\n"
                     "<tt>00 01<sub>16</sub></tt>\tDifferential correction applied"),
                   field_type, SHORT, count, 1, value_offset, is_little_endian,
                   G_N_ELEMENTS (values), values, value_description, NULL);
        }
        else if (tiff_tag == tiff_tags[GPSHPositioningError])
        {
            process_rational_tag (file, gpsinfo_tab, _("Tag: GPSHPositioningError"), "GPSHPositioningError",
                                  _("Indicates horizontal positioning errors, in meters"),
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
