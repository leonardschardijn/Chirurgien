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

#include <config.h>

#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_app1_marker (AnalyzerFile *file,
                     guint *marker_counts)
{
    const guchar exif_identifier[] = { 'E','x','i','f',0x00,0x00 }; // Exif\0\0
    const guchar xmp_identifier[] = { "http://ns.adobe.com/xap/1.0/" };

    AnalyzerTab tab;

    guint16 data_length, data_used = 0;
    guchar identifier[29];

    gsize segment_end;

    marker_counts[APP1]++;

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    segment_end = GET_POINTER (file) + (data_length - 2);

    /* Identifier */
    if (!analyzer_utils_read (&identifier, file, 6))
        goto END_ERROR;

    if (!memcmp (identifier, exif_identifier, 6))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 6, "Exif identifier");

        data_used = 8;

        analyzer_utils_init_tab (&tab);
        analyzer_utils_set_title_tab (&tab, _("<b>Exchangeable image file format</b>"));

        if (data_length > 8)
            data_length -= 8;
        else
            data_length = 0;

        if (data_length && FILE_HAS_DATA_N (file, data_length))
        {
            if (file->description_lines_count == 1)
                analyzer_utils_describe (file, _("Exif metadata available"), NULL);
            else
                analyzer_utils_add_description (file, _("Exif metadata available"), NULL, NULL, 10, 0);

            analyzer_utils_tag (file, MARKER_DATA_COLOR_2, data_length, _("Embedded TIFF file"));

            analyzer_utils_add_description_tab (&tab, _("There is an embedded TIFF (Exif) file, analyze it in another tab"),
                                                NULL, NULL, 0, 0);
            analyzer_utils_embedded_file (file, &tab, data_length);

            data_used += data_length;
        }

        analyzer_utils_insert_tab (file, &tab, "Exif");
    }
    else
    {
        ADVANCE_POINTER (file, -6);

        if (!analyzer_utils_read (&identifier, file, 29))
            goto END_ERROR;

        if (!memcmp (identifier, xmp_identifier, 29))
        {
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 29, "XMP identifier");

            data_used = 31;

            if (data_length > 31)
                data_length -= 31;
            else
                data_length = 0;

            if (data_length && FILE_HAS_DATA_N (file, data_length))
            {
                analyzer_utils_tag (file, MARKER_DATA_COLOR_2, data_length, _("Embedded XMP file"));

                ADVANCE_POINTER (file, data_length);
                data_used += data_length;
            }
        }
    }

    if (!data_used)
        data_used = 2;

    if (data_used < data_length)
    {
        data_length -= data_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        SET_POINTER (file, segment_end);
    }

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
