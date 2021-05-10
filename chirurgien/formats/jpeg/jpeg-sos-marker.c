/* jpeg-sos-marker.c
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

#include "jpeg-format.h"

#define SOS_LENGTH_WITHOUT_COMPONENTS 6
#define SOS_COMPONENT_LENGTH          2


gboolean
jpeg_sos_marker (FormatsFile *file,
                 gint        *marker_counts)
{
    DescriptionTab tab;

    const guchar *image_data;
    gchar *value;

    gboolean run = TRUE;

    guint16 data_length, data_used = SOS_LENGTH_WITHOUT_COMPONENTS;
    guint8 one_byte, components;

    gsize entropy_coded_data_length;

    marker_counts[SOS]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    format_utils_init_tab (&tab, _("Scan header"));

    /* Number of components in scan */
    if (!process_jpeg_field (file, &tab, _("Components in scan"), _("Number of components in scan"),
                             NULL, MARKER_DATA_COLOR_1, 1,
                             0, NULL, NULL, "%u", &components))
        return FALSE;

    while (components)
    {
        /* Scan component selector */
        if (!format_utils_read (file, &one_byte, 1))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                              _("Scan component selector"), NULL);

        value = g_strdup_printf ("%hhu", one_byte);
        format_utils_add_line_full_tab (&tab, _("Scan component selector"),
                                        value, NULL, 10, 0);
        g_free (value);

        /* Component table selectors */
        if (!format_utils_read (file, &one_byte, 1))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                              _("Component table selectors\n"
                                "Lower four bits: AC entropy coding table selector\n"
                                "Upper four bits: DC entropy coding table selector"), NULL);

        value = g_strdup_printf ("%u", one_byte & 0x0F);
        format_utils_add_line_tab (&tab, _("AC entropy coding table selector"), value, NULL);
        g_free (value);

        value = g_strdup_printf ("%u", one_byte >> 4);
        format_utils_add_line_tab (&tab, _("DC entropy coding table selector"), value, NULL);
        g_free (value);

        components--;
        data_used += SOS_COMPONENT_LENGTH;
    }

    /* Start of spectral or predictor selection */
    if (!format_utils_read (file, &one_byte, 1))
        return FALSE;

    format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                          _("Start of spectral (or predictor) selection"), NULL);

    value = g_strdup_printf ("%hhu", one_byte);
    format_utils_add_line_full_tab (&tab, _("Start of spectral (or predictor) selection"),
                                    value, NULL,
                                    10, 0);
    g_free (value);

    /* End of spectral selection */
    if (!format_utils_read (file, &one_byte, 1))
        return FALSE;

    format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                          _("End of spectral selection"), NULL);

    value = g_strdup_printf ("%hhu", one_byte);
    format_utils_add_line_tab (&tab, _("End of spectral selection"), value, NULL);
    g_free (value);

    /* Successive approximation bit positions */
    if (!FILE_HAS_DATA (file))
        return FALSE;

    format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                          _("Successive approximation bit positions\n"
                            "DCT mode:\n"
                            "\tLower four bits: Point transform used (for the specified band)\n"
                            "\tUpper four bits: Point transform used in the preceding scan (for the specified band)\n"
                            "Lossless mode:\n"
                            "\tLower four bits: Point transform used\n"
                            "\tUpper four bits: No meaning"), NULL);

    if (data_used < data_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                              _("Unrecognized data"), NULL);

    /* Entropy-encoded image */
    entropy_coded_data_length = 0;

    while (run)
    {
        image_data = GET_CONTENT_POINTER (file) + entropy_coded_data_length;

        for (gsize i = 0; i + GET_INDEX (file) < GET_FILE_SIZE (file); i++)
        {
            if (G_UNLIKELY (image_data[i] == 0xFF))
            {
                if (i + GET_INDEX (file) + 1 < GET_FILE_SIZE (file))
                {
                    one_byte = image_data[i + 1];
                }
                else
                {
                    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                                          _("Unexpected end of file"), NULL);
                    return FALSE;
                }

                entropy_coded_data_length += i;
                break;
            }
        }

        if (!one_byte)
        {
            entropy_coded_data_length += 2;
        }
        else
        {
            format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, entropy_coded_data_length,
                                  _("Entropy-encoded image"), NULL);

            entropy_coded_data_length = 0;

            switch (one_byte)
            {
                case 0xD0:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST0"), "RST0");
                    continue;
                case 0xD1:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST1"), "RST1");
                    continue;
                case 0xD2:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST2"), "RST2");
                    continue;
                case 0xD3:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST3"), "RST3");
                    continue;
                case 0xD4:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST4"), "RST4");
                    continue;
                case 0xD5:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST5"), "RST5");
                    continue;
                case 0xD6:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST6"), "RST6");
                    continue;
                case 0xD7:
                    format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                          _("Marker type: RST7"), "RST7");
                    continue;
                default:
                    run = FALSE;
            }
        }
    }

    format_utils_insert_tab (file, &tab, marker_names[SOS]);

    return TRUE;
}
