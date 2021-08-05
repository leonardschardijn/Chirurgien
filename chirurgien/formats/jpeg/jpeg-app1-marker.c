/* jpeg-app1-marker.c
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


gboolean
jpeg_app1_marker (FormatsFile *file,
                  gint        *marker_counts)
{
    const gchar exif_identifier[] = { 'E','x','i','f',0x00,0x00 }; // Exif\0\0
    const gchar * const xmp_identifier = "http://ns.adobe.com/xap/1.0/";

    const gchar *identifier;

    guint16 data_length, data_used = 2;

    marker_counts[APP1]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    identifier = (const gchar *) GET_CONTENT_POINTER (file);

    if (FILE_HAS_DATA_N (file, 6) && !memcmp (identifier, exif_identifier, 6))
    {
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 6,
                                "Exif identifier", NULL);

        marker_counts[EXIF]++;
        data_used += 6;

        for (gint i = SOF0; i <= SOF15; i++)
        {
            if (marker_counts[i])
            {
                if (data_length > data_used)
                    format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                            "Exif APP1 segments should be defined before SOF segments", NULL);
                data_used = data_length;
            }
        }

        if (data_length > data_used)
        {
            if (marker_counts[JFIF] || marker_counts[JFIF])
                format_utils_add_line_full (file, "Exif metadata available", NULL, NULL, 10, 0);
            else
                format_utils_add_line (file, "Exif metadata available", NULL, NULL);

            format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, data_length - data_used,
                                    "Embedded TIFF file", NULL);
            data_used = data_length;
        }
    }
    else if (FILE_HAS_DATA_N (file, 29) && !memcmp (identifier, xmp_identifier, 29))
    {
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 29,
                                "XMP identifier", NULL);

        data_used += 29;

        if (data_length > data_used)
        {
            format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, data_length - data_used,
                                    "Embedded XMP file", NULL);
            data_used = data_length;
        }
    }

    if (data_used < data_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                                "Unrecognized data", NULL);

    return TRUE;
}
