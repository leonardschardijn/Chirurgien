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
    const guchar jfif_identifier[] = { 0x4A,0x46,0x49,0x46,0x00 }; // JFIF\0
    const guchar jfxx_identifier[] = { 0x4A,0x46,0x58,0x58,0x00 }; // JFXX\0

    AnalyzerTab tab;

    gchar *description_message;

    guint16 data_length, data_used = 0;
    guchar identifier[5], version[2];

    guint8 one_byte;
    guint16 two_bytes;

    guint i;
    gsize thumbnail_data_length = 3;

    marker_counts[APP0]++;

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    /* Identifier */
    if (!analyzer_utils_read (&identifier, file , 5))
        goto END_ERROR;

    if (!memcmp (identifier, jfif_identifier, 5))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 5, _("JFIF identifier"));

        if (marker_counts[APP0] > 1)
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("JFIF APP0 segment already defined"));
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
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        switch (one_byte)
        {
            case 0:
                description_message = _("No units");
                goto VALID_DENSITY_UNITS;
            case 1:
                description_message = _("Pixels per inch");
                goto VALID_DENSITY_UNITS;
            case 2:
                description_message = _("Pixels per centimeter");
                goto VALID_DENSITY_UNITS;
            default:
                analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid density units"));
                description_message = _("<span foreground=\"red\">INVALID</span>");
                goto END_DENSITY_UNITS;
        }
        VALID_DENSITY_UNITS:
        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Density units"));

        END_DENSITY_UNITS:
        analyzer_utils_describe_tooltip (file, _("Density units"), description_message,
                                         _("Density units\n"
                                         "<tt>00<sub>16</sub></tt>\tNo units\n"
                                         "<tt>01<sub>16</sub></tt>\tPixels per inch\n"
                                         "<tt>02<sub>16</sub></tt>\tPixels per centimeter"));

        /* Xdensity */
        if (!analyzer_utils_read (&two_bytes, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 2, _("Xdensity"));

        two_bytes = g_ntohs (two_bytes);
        description_message = g_strdup_printf ("%u", two_bytes);
        analyzer_utils_describe_tooltip (file, _("Xdensity"), description_message,
                                         _("Must not be zero"));
        g_free (description_message);

        /* Ydensity */
        if (!analyzer_utils_read (&two_bytes, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 2, _("Ydensity"));

        two_bytes = g_ntohs (two_bytes);
        description_message = g_strdup_printf ("%u", two_bytes);
        analyzer_utils_describe_tooltip (file, _("Ydensity"), description_message,
                                         _("Must not be zero"));
        g_free (description_message);

        /* Xthumbnail */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("Xthumbnail"));

        description_message = g_strdup_printf ("%u", one_byte);
        analyzer_utils_describe_tooltip (file, _("Xthumbnail"), description_message,
                                         _("Thumbnail width"));
        g_free (description_message);

        thumbnail_data_length *= one_byte;

        /* Ythumbnail */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Ythumbnail"));

        description_message = g_strdup_printf ("%u", one_byte);
        analyzer_utils_describe_tooltip (file, _("Ythumbnail"), description_message,
                                         _("Thumbnail height"));
        g_free (description_message);

        thumbnail_data_length *= one_byte;

        if (thumbnail_data_length)
        {
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));

            data_used += thumbnail_data_length;
            ADVANCE_POINTER (file, thumbnail_data_length);
        }
        data_used = 16 + thumbnail_data_length;
    }
    else if (!memcmp (identifier, jfxx_identifier, 5))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 5, _("JFIF extension identifier"));

        analyzer_utils_init_tab (&tab);

        analyzer_utils_set_title_tab (&tab, _("<b>JFIF extension</b>"));

        /* Thumbnail format */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Thumbnail format"));

        if (one_byte == 0x00)
        {
            analyzer_utils_describe_tooltip_tab (&tab, _("Thumbnail format"), _("JPEG image"),
                                                 _("Thumbnail format\n"
                                                 "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                                                 "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                                                 "<tt>13<sub>16</sub></tt>\t8-bit RGB image"));

            if (data_length > 8)
                data_length -= 8;
            else
                data_length = 0;

            if (data_length)
            {
                analyzer_utils_tag (file, MARKER_DATA_COLOR_1, data_length, _("Embedded JPEG image"));

                analyzer_utils_add_description_tab (&tab, _("There is an embedded JPEG file, analyze it in another tab"),
                                                    NULL, NULL, 10, 0);
                analyzer_utils_embedded_file (file, &tab, data_length);

                data_used = 8 + data_length;
            }
        }
        else if (one_byte == 0x11)
        {
            thumbnail_data_length = 1;

            analyzer_utils_describe_tooltip_tab (&tab, _("Thumbnail format"), _("8-bit indexed-color image"),
                                                 _("Thumbnail format\n"
                                                 "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                                                 "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                                                 "<tt>13<sub>16</sub></tt>\t8-bit RGB image"));

            /* Xthumbnail */
            if (!analyzer_utils_read (&one_byte, file , 1))
                return FALSE;

            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("Xthumbnail"));

            description_message = g_strdup_printf ("%u", one_byte);
            analyzer_utils_describe_tooltip_tab (&tab, _("Xthumbnail"), description_message,
                                                 _("Thumbnail width"));
            g_free (description_message);

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!analyzer_utils_read (&one_byte, file , 1))
                return FALSE;

            analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Ythumbnail"));

            description_message = g_strdup_printf ("%u", one_byte);
            analyzer_utils_describe_tooltip_tab (&tab, _("Ythumbnail"), description_message,
                                                 _("Thumbnail height"));
            g_free (description_message);

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
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));
            ADVANCE_POINTER (file, 768 + thumbnail_data_length);
            data_used += 778 + thumbnail_data_length;
        }
        else if (one_byte == 0x13)
        {
            thumbnail_data_length = 1;

            analyzer_utils_describe_tooltip_tab (&tab, _("Thumbnail format"), _("8-bit RGB image"),
                                                 _("Thumbnail format\n"
                                                 "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                                                 "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                                                 "<tt>13<sub>16</sub></tt>\t8-bit RGB image"));

            /* Xthumbnail */
            if (!analyzer_utils_read (&one_byte, file , 1))
                return FALSE;

            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("Xthumbnail"));

            description_message = g_strdup_printf ("%u", one_byte);
            analyzer_utils_describe_tooltip_tab (&tab, _("Xthumbnail"), description_message,
                                                 _("Thumbnail width"));
            g_free (description_message);

            thumbnail_data_length *= one_byte;

            /* Ythumbnail */
            if (!analyzer_utils_read (&one_byte, file , 1))
                return FALSE;

            analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Ythumbnail"));

            description_message = g_strdup_printf ("%u", one_byte);
            analyzer_utils_describe_tooltip_tab (&tab, _("Ythumbnail"), description_message,
                                                 _("Thumbnail height"));
            g_free (description_message);

            thumbnail_data_length *= one_byte;

            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, thumbnail_data_length, _("Thumbnail image"));
            ADVANCE_POINTER (file, thumbnail_data_length);
            data_used += 10 + thumbnail_data_length;
        }
        else
        {
            analyzer_utils_describe_tooltip_tab (&tab, _("Thumbnail format"), _("<span foreground=\"red\">INVALID</span>"),
                                                 _("Thumbnail format\n"
                                                 "<tt>00<sub>16</sub></tt>\tJPEG image\n"
                                                 "<tt>11<sub>16</sub></tt>\t8-bit indexed-color image\n"
                                                 "<tt>13<sub>16</sub></tt>\t8-bit RGB image"));
            data_used += 8;
        }

        analyzer_utils_insert_tab (file, &tab, "JFXX");
    }

    if (!data_used)
        data_used = 2;

    if (data_used < data_length)
    {
        data_length -= data_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, data_length - 5);
    }

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
