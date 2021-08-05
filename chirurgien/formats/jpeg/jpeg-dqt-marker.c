/* jpeg-dqt-marker.c
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
jpeg_dqt_marker (FormatsFile *file,
                 gint        *marker_counts)
{
    DescriptionTab tab;

    gchar *value;

    guint16 data_length;
    guint8 qt_id_precision, qt_identifier;
    guint qt_length;

    marker_counts[DQT]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    if (data_length > 1)
        data_length -= 2;
    else
        return TRUE;

    format_utils_init_tab (&tab, NULL);

    while (data_length)
    {
        /* Quantization table information */
        if (!format_utils_read (file, &qt_id_precision, 1))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                                "Quantization table information\n"
                                "Lower four bits: Quantization table identifier\n"
                                "Upper four bits: Quantization table precision", NULL);

        format_utils_start_section_tab (&tab, "Quantization table");

        /* Quantization table identifier */
        qt_identifier = qt_id_precision & 0x0F;
        if (qt_identifier < 4)
            value = g_strdup_printf ("%u", qt_identifier);
        else
            value = g_strdup ("<span foreground=\"red\">INVALID</span>");
        format_utils_add_line_tab (&tab, "Quantization table identifier", value,
                                   "Valid quantization table destination identifiers: 0-3");
        g_free (value);

        /* Quantization table precision */
        switch (qt_id_precision >> 4)
        {
            case 0:
                qt_length = 64;
                value = "8 bit values";
                break;
            case 1:
                qt_length = 128;
                value = "16 bit values";
                break;
            default:
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length,
                                        "Unrecognized data", NULL);
                value = "<span foreground=\"red\">INVALID</span>";
                data_length = 0;
        }

        format_utils_add_line_tab (&tab, "Quantization table precision", value,
                                   "Quantization table precision\n"
                                   "<tt>0<sub>16</sub></tt>\t8 bit values\n"
                                   "<tt>1<sub>16</sub></tt>\t16 bit values");

        if (!data_length)
            break;

        format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, qt_length,
                                "Quantization table values", NULL);

        if (data_length > qt_length)
            data_length -= (qt_length + 1);
        else
            data_length = 0;
    }

    format_utils_insert_tab (file, &tab, marker_names[DQT]);

    return TRUE;
}
