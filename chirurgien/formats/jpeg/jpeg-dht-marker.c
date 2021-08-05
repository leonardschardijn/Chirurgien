/* jpeg-dht-marker.c
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

#define POSSIBLE_CODE_LENGTHS 16


gboolean
jpeg_dht_marker (FormatsFile *file,
                 gint        *marker_counts)
{
    DescriptionTab tab;

    gchar *value1, *value2;

    guint16 data_length, table_size;
    guint8 code_counts[16], ht_info;
    gint i;

    gboolean color_toggle;

    marker_counts[DHT]++;

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
        /* Huffman table information */
        if (!format_utils_read (file, &ht_info, 1))
            return FALSE;

        table_size = 1;
        format_utils_add_field (file, HUFFMAN_ARITH_TABLE_COLOR, TRUE, 1,
                                "Huffman table information\n"
                                "Lower four bits: Huffman table identifier\n"
                                "Upper four bits: Table class", NULL);

        format_utils_start_section_tab (&tab, "Huffman code table");

        value1 = g_strdup_printf ("%u", ht_info & 0x0F);
        format_utils_add_line_tab (&tab, "Huffman table identifier", value1, NULL);
        g_free (value1);

        switch (ht_info >> 4)
        {
            case 0:
                value1 = "DC table (or lossless table)";
                break;
            case 1:
                value1 = "AC table";
                break;
            default:
                value1 = "<span foreground=\"red\">INVALID</span>";
        }

        format_utils_add_line_tab (&tab, "Table class", value1,
                                   "Table class\n"
                                   "<tt>0<sub>16</sub></tt>\tDC table (or lossless table)\n"
                                   "<tt>1<sub>16</sub></tt>\tAC table");

        if (!format_utils_read (file, &code_counts, POSSIBLE_CODE_LENGTHS))
            return FALSE;

        for (i = 0; i < POSSIBLE_CODE_LENGTHS;)
        {
            value2 = g_strdup_printf ("%hhu", code_counts[i++]);
            value1 = g_strdup_printf ("Codes of length %u", i);

            format_utils_add_line_tab (&tab, value1, value2, NULL);

            g_free (value1);
            g_free (value2);
        }

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, POSSIBLE_CODE_LENGTHS,
                                "Number of Huffman codes for 1-16 bit lengths", NULL);

        table_size += POSSIBLE_CODE_LENGTHS;

        color_toggle = TRUE;
        for (i = 0; i < POSSIBLE_CODE_LENGTHS; i++)
        {
            if (code_counts[i])
            {
                if (color_toggle)
                    format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, code_counts[i],
                                            "Huffman code values", NULL);
                else
                    format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, code_counts[i],
                                            "Huffman code values", NULL);

                color_toggle = !color_toggle;
                table_size += code_counts[i];
            }
        }

        if (data_length >= table_size)
            data_length -= table_size;
        else
            data_length = 0;
    }

    format_utils_insert_tab (file, &tab, marker_names[DHT]);

    return TRUE;
}
