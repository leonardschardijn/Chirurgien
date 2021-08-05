/* jpeg-dac-marker.c
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
jpeg_dac_marker (FormatsFile *file,
                 gint  *marker_counts)
{
    DescriptionTab tab;

    gchar *value;

    guint16 data_length;
    guint8 arithmetic_info;

    marker_counts[DAC]++;

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
        /* Arithmetic conditioning table information */
        if (!format_utils_read (file, &arithmetic_info, 1))
            return FALSE;

        format_utils_add_field (file, HUFFMAN_ARITH_TABLE_COLOR, TRUE, 1,
                                "Arithmetic conditioning table information\n"
                                "Lower four bits: Arithmetic conditioning table identifier\n"
                                "Upper four bits: Table class", NULL);

        format_utils_start_section_tab (&tab, "Arithmetic conditioning table");

        value = g_strdup_printf ("%u", arithmetic_info & 0x0F);
        format_utils_add_line_tab (&tab, "Arithmetic conditioning table identifier", value, NULL);
        g_free (value);

        switch (arithmetic_info >> 4)
        {
            case 0:
                value = "DC table (or lossless table)";
                break;
            case 1:
                value = "AC table";
                break;
            default:
                value = "<span foreground=\"red\">INVALID</span>";
        }

        format_utils_add_line_tab (&tab, "Table class", value,
                                   "Table class\n"
                                   "<tt>0<sub>16</sub></tt>\tDC table (or lossless table)\n"
                                   "<tt>1<sub>16</sub></tt>\tAC table");

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                                "Conditioning table value", NULL);

        if (data_length > 1)
            data_length -= 2;
        else
            data_length = 0;
    }

    format_utils_insert_tab (file, &tab, marker_names[DAC]);

    return TRUE;
}
