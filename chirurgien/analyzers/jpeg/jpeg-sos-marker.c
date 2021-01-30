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

#include "jpeg-analyzer.h"


gboolean
analyze_sos_marker (AnalyzerFile *file,
                    guint *marker_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    gboolean run = TRUE;

    guint16 data_length, data_used = 0;
    guint8 one_byte, components;
    guchar two_bytes[2];
    gsize entropy_coded_data_length = 0;

    marker_counts[SOS]++;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Scan header</b>"));

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    /* Number of components in scan */
    if (!process_jpeg_field (file, &tab, _("Components in scan"), _("Number of components in scan"),
                             NULL, MARKER_DATA_COLOR_1, 1,
                             0, NULL, NULL, "%u", &components))
        return FALSE;

    while (components)
    {
        /* Scan component selector */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                            _("Scan component selector"));

        description_message = g_strdup_printf ("%hhu", one_byte);
        analyzer_utils_add_description_tab (&tab, _("Scan component selector"),
                                            description_message, NULL,
                                            10, 0);
        g_free (description_message);

        /* Component table selectors */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                            _("Component table selectors\n"
                              "Lower four bits: AC entropy coding table selector\n"
                              "Upper four bits: DC entropy coding table selector"));

        description_message = g_strdup_printf ("%u", one_byte & 0x0F);
        analyzer_utils_describe_tab (&tab, _("AC entropy coding table selector"), description_message);
        g_free (description_message);

        description_message = g_strdup_printf ("%u", one_byte >> 4);
        analyzer_utils_describe_tab (&tab, _("DC entropy coding table selector"), description_message);
        g_free (description_message);

        components--;
        data_used += 2;
    }

    /* Start of spectral or predictor selection */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                        _("Start of spectral (or predictor) selection"));

    description_message = g_strdup_printf ("%hhu", one_byte);
    analyzer_utils_add_description_tab (&tab, _("Start of spectral (or predictor) selection"),
                                        description_message, NULL,
                                        10, 0);
    g_free (description_message);

    /* End of spectral selection */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                        _("End of spectral selection"));

    description_message = g_strdup_printf ("%hhu", one_byte);
    analyzer_utils_describe_tab (&tab, _("End of spectral selection"), description_message);
    g_free (description_message);

    /* Successive approximation bit positions */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1,
                        _("Successive approximation bit positions\n"
                          "DCT mode:\n"
                          "\tLower four bits: Point transform used (for the specified band)\n"
                          "\tUpper four bits: Point transform used in the preceding scan (for the specified band)\n"
                          "Lossless mode:\n"
                          "\tLower four bits: Point transform used\n"
                          "\tUpper four bits: No meaning"));

    data_used += 6;

    if (data_used < data_length)
    {
        data_length -= data_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, data_length);
    }

    while (run)
    {
        entropy_coded_data_length += analyzer_utils_advance_to (file, 0xFF);

        if (!analyzer_utils_read (&two_bytes, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unexpected end of file"));
            return FALSE;
        }

        if (two_bytes[1] == 0x00)
        {
            entropy_coded_data_length += 2;
        }
        else
        {
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, entropy_coded_data_length,
                                _("Entropy encoded image"));

            entropy_coded_data_length = 0;

            switch (two_bytes[1])
            {
                case 0xD0:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST0"), "RST0");
                    continue;
                case 0xD1:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST1"), "RST1");
                    continue;
                case 0xD2:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST2"), "RST2");
                    continue;
                case 0xD3:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST3"), "RST3");
                    continue;
                case 0xD4:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST4"), "RST4");
                    continue;
                case 0xD5:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST5"), "RST5");
                    continue;
                case 0xD6:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST6"), "RST6");
                    continue;
                case 0xD7:
                    analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2,
                                                   _("Marker type: RST7"), "RST7");
                    continue;
                default:
                    ADVANCE_POINTER (file, -2);
                    run = FALSE;
            }
        }
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[SOS]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
