/* jpeg-app0-marker.c
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

#include "jpeg-format.h"

#define JFIF_SEGMENT_LENGTH             16
#define JFXX_INDEX_COLOR_SEGMENT_LENGTH 778
#define JFXX_RGB_SEGMENT_LENGTH         10


gboolean
jpeg_app0_marker (FormatsFile *file,
                  gint        *marker_counts)
{
    const gchar * const jfif_identifier = "JFIF"; // JFIF\0
    const gchar * const jfxx_identifier = "JFXX"; // JFXX\0

    const gchar *identifier;
    gchar *value;

    guint16 data_length, data_used = 2;
    guchar version[2];

    guint8 one_byte;

    gsize thumbnail_data_length = 3;

    marker_counts[APP0]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    for (gint i = SOF0; i <= SOF15; i++)
    {
        if (marker_counts[i])
        {
            if (data_length > 1)
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                        "APP0 segments should be defined before SOF segments", NULL);
            return TRUE;
        }
    }
    if (marker_counts[EXIF])
    {
        if (data_length > 1)
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                    "APP0 segments cannot follow an Exif APP1 segment", NULL);
        return TRUE;
    }

    identifier = (const gchar *) GET_CONTENT_POINTER (file);

    if (FILE_HAS_DATA_N (file, 5) && !memcmp (identifier, jfif_identifier, 5))
    {
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 5,
                                "JFIF identifier", NULL);

        marker_counts[JFIF]++;

        if (marker_counts[JFIF] > 1)
        {
            if (data_length > 1)
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                        "JFIF APP0 segment already defined", NULL);
            return TRUE;
        }

        /* JFIF version */
        if (!format_utils_read (file, &version, 2))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                                "JFIF major version", NULL);
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                                "JFIF minor version", NULL);

        value = g_strdup_printf ("%u.%.2u", version[0], version[1]);
        format_utils_add_line (file, "JFIF version", value,
                               "JFIF version\n"
                               "<tt>01 00<sub>16</sub></tt>\tv1.00\n"
                               "<tt>01 01<sub>16</sub></tt>\tv1.01\n"
                               "<tt>01 02<sub>16</sub></tt>\tv1.02");
        g_free (value);

        /* Density units */
        guint8 density_units_values[] = { 0x0, 0x1, 0x2 };
        const gchar *density_units_value_description[] = {
            "No units",
            "Pixels per inch",
            "Pixels per centimeter",
            "<span foreground=\"red\">INVALID</span>"
        };
        if (!process_jpeg_field (file, NULL, "Density units", NULL,
                           "Density units\n"
                           "<tt>00<sub>16</sub></tt>\tNo units\n"
                           "<tt>01<sub>16</sub></tt>\tPixels per inch\n"
                           "<tt>02<sub>16</sub></tt>\tPixels per centimeter",
                           MARKER_DATA_COLOR_2, 1,
                           sizeof (density_units_values), density_units_values, density_units_value_description,
                           NULL, NULL))
            return FALSE;

        /* Xdensity */
        if (!process_jpeg_field (file, NULL, "Xdensity", NULL,
                                 "Must not be zero",
                                 MARKER_DATA_COLOR_1, 2,
                                 0, NULL, NULL,
                                 "%u", NULL))
            return FALSE;

        /* Ydensity */
        if (!process_jpeg_field (file, NULL, "Ydensity", NULL,
                                 "Must not be zero",
                                 MARKER_DATA_COLOR_2, 2,
                                 0, NULL, NULL,
                                 "%u", NULL))
            return FALSE;

        /* Xthumbnail */
        if (!process_jpeg_field (file, NULL, "Xthumbnail", NULL,
                                 "Thumbnail width",
                                 MARKER_DATA_COLOR_1, 1,
                                 0, NULL, NULL,
                                 "%u", &one_byte))
            return FALSE;

        thumbnail_data_length *= one_byte;

        /* Ythumbnail */
        if (!process_jpeg_field (file, NULL, "Ythumbnail", NULL,
                                 "Thumbnail height",
                                 MARKER_DATA_COLOR_2, 1,
                                 0, NULL, NULL,
                                 "%u", &one_byte))
            return FALSE;

        thumbnail_data_length *= one_byte;

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, thumbnail_data_length,
                                "Thumbnail image", NULL);

        data_used = JFIF_SEGMENT_LENGTH + thumbnail_data_length;
    }
    else if (FILE_HAS_DATA_N (file, 5) && !memcmp (identifier, jfxx_identifier, 5))
    {
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 5,
                                "JFIF extension identifier", NULL);

        /* Thumbnail format */
        guint8 thumbnail_values[] = { 0x0, 0x11, 0x13 };
        const gchar *thumbnail_value_description[] = {
            "JPEG image",
            "8-bit indexed-color image",
            "8-bit RGB image",
            "<span foreground=\"red\">INVALID</span>"
        };
        if (!process_jpeg_field (file, NULL, "Thumbnail format", NULL,
                             "Thumbnail format\n"
                             "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                             "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                             "<tt>13<sub>16</sub></tt>\t8-bit RGB image",
                           MARKER_DATA_COLOR_2, 1,
                           sizeof (thumbnail_values), thumbnail_values, thumbnail_value_description,
                           NULL, &one_byte))
            return FALSE;

        marker_counts[JFXX]++;
        data_used += 6;

        /* JPEG image */
        if (one_byte == 0x00)
        {
            if (data_length > data_used)
            {
                format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, data_length - data_used,
                                        "Embedded JPEG image", NULL);
                data_used = data_length;
            }
        }
        /* 8-bit indexed-color image */
        else if (one_byte == 0x11)
        {
            thumbnail_data_length = 1;

            /* Xthumbnail */
            if (!process_jpeg_field (file, NULL, "Xthumbnail", NULL,
                                     "Thumbnail width",
                                     MARKER_DATA_COLOR_1, 1,
                                     0, NULL, NULL,
                                     "%u", &one_byte))
                return FALSE;

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!process_jpeg_field (file, NULL, "Ythumbnail", NULL,
                                     "Thumbnail height",
                                     MARKER_DATA_COLOR_2, 1,
                                     0, NULL, NULL,
                                     "%u", &one_byte))
                return FALSE;

            thumbnail_data_length *= one_byte;

            for (gint i = 0; i < 256; i++)
            {
                if (i % 2)
                    format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 3,
                                            "RGB palette entry", NULL);
                else
                    format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 3,
                                            "RGB palette entry", NULL);
            }

            format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, thumbnail_data_length,
                                    "Thumbnail image", NULL);

            data_used = JFXX_INDEX_COLOR_SEGMENT_LENGTH + thumbnail_data_length;
        }
        /* 8-bit RGB image */
        else if (one_byte == 0x13)
        {
            thumbnail_data_length = 1;

            /* Xthumbnail */
            if (!process_jpeg_field (file, NULL, "Xthumbnail", NULL,
                                     "Thumbnail width",
                                     MARKER_DATA_COLOR_1, 1,
                                     0, NULL, NULL,
                                     "%u", &one_byte))
                return FALSE;

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!process_jpeg_field (file, NULL, "Ythumbnail", NULL,
                                     "Thumbnail height",
                                     MARKER_DATA_COLOR_2, 1,
                                     0, NULL, NULL,
                                     "%u", &one_byte))
                return FALSE;

            thumbnail_data_length *= one_byte;

            format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, thumbnail_data_length,
                                    "Thumbnail image", NULL);

            data_used += JFXX_RGB_SEGMENT_LENGTH + thumbnail_data_length;
        }
    }

    if (data_used < data_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                "Unrecognized data", NULL);

    return TRUE;
}
