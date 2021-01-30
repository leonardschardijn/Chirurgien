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

#include <config.h>

#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_app0_marker (AnalyzerFile *file,
                     guint *marker_counts)
{
    const guchar jfif_identifier[] = "JFIF"; // JFIF\0
    const guchar jfxx_identifier[] = "JFXX"; // JFXX\0

    AnalyzerTab tab;

    gchar *description_message;

    guint16 data_length, data_used = 0;
    guchar identifier[5], version[2];

    guint8 one_byte;

    guint i;
    gsize thumbnail_data_length = 3;

    gsize segment_end;

    marker_counts[APP0]++;

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    segment_end = GET_POINTER (file) + (data_length - 2);

    /* Identifier */
    if (!analyzer_utils_read (&identifier, file , 5))
        goto END_ERROR;

    if (!memcmp (identifier, jfif_identifier, 5))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 5, _("JFIF identifier"));

        if (marker_counts[APP0] > 1)
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("JFIF APP0 segment already defined"));
            ADVANCE_POINTER (file, data_length);
            return TRUE;
        }

        /* JFIF version */
        if (!analyzer_utils_read (&version, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("JFIF major version"));
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("JFIF minor version"));

        description_message = g_strdup_printf ("%u.%.2u", version[0], version[1]);
        analyzer_utils_describe_tooltip (file, _("JFIF version"), description_message,
                                         _("JFIF versions\n"
                                           "<tt>01 00<sub>16</sub></tt>\tv1.00\n"
                                           "<tt>01 01<sub>16</sub></tt>\tv1.01\n"
                                           "<tt>01 02<sub>16</sub></tt>\tv1.02"));
        g_free (description_message);

        /* Density units */
        guint8 density_units_values[] = { 0x0, 0x1, 0x2 };
        gchar *density_units_value_description[] = {
            _("No units"),
            _("Pixels per inch"),
            _("Pixels per centimeter"),
            _("<span foreground=\"red\">INVALID</span>")
        };
        if (!process_jpeg_field (file, NULL, _("Density units"), NULL,
                           _("Density units\n"
                             "<tt>00<sub>16</sub></tt>\tNo units\n"
                             "<tt>01<sub>16</sub></tt>\tPixels per inch\n"
                             "<tt>02<sub>16</sub></tt>\tPixels per centimeter"),
                           MARKER_DATA_COLOR_2, 1,
                           sizeof (density_units_values), density_units_values, density_units_value_description,
                           NULL, NULL))
            goto END_ERROR;

        /* Xdensity */
        if (!process_jpeg_field (file, NULL, _("Xdensity"), NULL,
                                 _("Must not be zero"),
                                 MARKER_DATA_COLOR_1, 2,
                                 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        /* Ydensity */
        if (!process_jpeg_field (file, NULL, _("Ydensity"), NULL,
                                 _("Must not be zero"),
                                 MARKER_DATA_COLOR_2, 2,
                                 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        /* Xthumbnail */
        if (!process_jpeg_field (file, NULL, _("Xthumbnail"), NULL,
                                 _("Thumbnail width"),
                                 MARKER_DATA_COLOR_1, 1,
                                 0, NULL, NULL, "%u", &one_byte))
            goto END_ERROR;

        thumbnail_data_length *= one_byte;

        /* Ythumbnail */
        if (!process_jpeg_field (file, NULL, _("Ythumbnail"), NULL,
                                 _("Thumbnail height"),
                                 MARKER_DATA_COLOR_2, 1,
                                 0, NULL, NULL, "%u", &one_byte))
            goto END_ERROR;

        thumbnail_data_length *= one_byte;

        if (thumbnail_data_length)
        {
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));
            ADVANCE_POINTER (file, thumbnail_data_length);
        }

        /* data_length (2) + JFIF id. (5) + JFIF version (2) + density units (1) +
         * Xdensity (2) + Ydensity (2) + Xthumbnail (1) + Ythumbnail (1) = 16 */
        data_used = 16 + thumbnail_data_length;
    }
    else if (!memcmp (identifier, jfxx_identifier, 5))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 5, _("JFIF extension identifier"));

        analyzer_utils_init_tab (&tab);

        analyzer_utils_set_title_tab (&tab, _("<b>JFIF extension</b>"));

        /* Thumbnail format */
        guint8 thumbnail_values[] = { 0x0, 0x11, 0x13 };
        gchar *thumbnail_value_description[] = {
            _("JPEG image"),
            _("8-bit indexed-color image"),
            _("8-bit RGB image"),
            _("<span foreground=\"red\">INVALID</span>")
        };
        if (!process_jpeg_field (file, &tab, _("Thumbnail format"), NULL,
                           _("Thumbnail format\n"
                             "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                             "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                             "<tt>13<sub>16</sub></tt>\t8-bit RGB image"),
                           MARKER_DATA_COLOR_2, 1,
                           sizeof (thumbnail_values), thumbnail_values, thumbnail_value_description,
                           NULL, &one_byte))
            goto END_ERROR;

        if (one_byte == 0x00)
        {
            if (data_length > 8)
                data_length -= 8;
            else
                data_length = 0;

            if (data_length && FILE_HAS_DATA_N (file, data_length))
            {
                analyzer_utils_tag (file, MARKER_DATA_COLOR_1, data_length, _("Embedded JPEG image"));

                analyzer_utils_add_description_tab (&tab, _("There is an embedded JPEG file, analyze it in another tab"),
                                                    NULL, NULL, 10, 0);
                analyzer_utils_embedded_file (file, &tab, data_length);
            }

            /* data_length (2) + JFXX id. (5) + thumbnail format (1) = 8 */
            data_used = 8 + data_length;
        }
        else if (one_byte == 0x11)
        {
            thumbnail_data_length = 1;

            /* Xthumbnail */
            if (!process_jpeg_field (file, &tab, _("Xthumbnail"), NULL,
                                     _("Thumbnail width"),
                                     MARKER_DATA_COLOR_1, 1,
                                     0, NULL, NULL, "%u", &one_byte))
                goto END_ERROR;

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!process_jpeg_field (file, &tab, _("Ythumbnail"), NULL,
                                     _("Thumbnail height"),
                                     MARKER_DATA_COLOR_2, 1,
                                     0, NULL, NULL, "%u", &one_byte))
                goto END_ERROR;

            thumbnail_data_length *= one_byte;

            for (i = 0; i < 256; i++)
            {
                if (i % 2)
                {
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                                        _("Palette entry blue sample"));
                }
                else
                {
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                                        _("Palette entry blue sample"));
                }
            }

            if (thumbnail_data_length)
                analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));

            /* palette (256*3) = 768 */
            ADVANCE_POINTER (file, 768 + thumbnail_data_length);

            /* data_length (2) + JFXX id. (5) + thumbnail format (1) + Xthumbnail (1) +
             * Ythumbnail (1) + palette (256*3) = 778 */
            data_used += 778 + thumbnail_data_length;
        }
        else if (one_byte == 0x13)
        {
            thumbnail_data_length = 1;

            /* Xthumbnail */
            if (!process_jpeg_field (file, &tab, _("Xthumbnail"), NULL,
                                     _("Thumbnail width"),
                                     MARKER_DATA_COLOR_1, 1,
                                     0, NULL, NULL, "%u", &one_byte))
                goto END_ERROR;

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!process_jpeg_field (file, &tab, _("Ythumbnail"), NULL,
                                     _("Thumbnail height"),
                                     MARKER_DATA_COLOR_2, 1,
                                     0, NULL, NULL, "%u", &one_byte))
                goto END_ERROR;

            thumbnail_data_length *= one_byte;

            if (thumbnail_data_length)
            {
                analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));
                ADVANCE_POINTER (file, thumbnail_data_length);
            }

            /* data_length (2) + JFXX id. (5) + thumbnail format (1) + Xthumbnail (1) +
             * Ythumbnail (1) = 10 */
            data_used += 10 + thumbnail_data_length;
        }
        else
        {
            /* data_length (2) + JFIF id. (5) + thumbnail format (1) */
            data_used += 8;
        }

        analyzer_utils_insert_tab (file, &tab, "JFXX");
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_2, 5, _("Unrecognized identifier"));
        /* data_length (2) + Unknown id. (5) */
        data_used = 7;
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
