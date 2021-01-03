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

#include <config.h>

#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_dht_marker (AnalyzerFile *file,
                    guint *marker_counts)
{
    AnalyzerTab tab;

    gchar *description_message1, *description_message2;

    guint16 data_length, table_size;
    guint8 code_counts[16], ht_info;
    guint i;

    gboolean color_toggle;

    marker_counts[DHT]++;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_describe_tab (&tab, _("<b>Huffman code tables</b>"), NULL);

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    if (data_length > 1)
        data_length -= 2;
    else
        return TRUE;

    while (data_length > 0)
    {
        /* Huffman table information */
        if (!analyzer_utils_read (&ht_info, file , 1))
            return FALSE;

        table_size = 1;
        analyzer_utils_tag (file, HUFFMAN_TABLE_COLOR, 1,
                            _("Huffman table information\n"
                            "Lower four bits: Huffman table identifier\n"
                            "Upper four bits: Table class"));

        description_message1 = g_strdup_printf ("%u", ht_info & 0x0F);
        analyzer_utils_add_description_tab (&tab, _("Huffman table identifier"),
                                            description_message1, NULL,
                                            10, 0);
        g_free (description_message1);

        ht_info = ht_info >> 4;
        if (!ht_info)
            description_message1 = _("DC table (or lossless table)");
        else if (ht_info == 1)
            description_message1 = _("AC table");
        else
            description_message1 = _("<span foreground=\"red\">INVALID</span>");

        analyzer_utils_describe_tooltip_tab (&tab, _("Table class"), description_message1,
                                             _("Table class\n"
                                             "<tt>0<sub>16</sub></tt>\tDC table (or lossless table)\n"
                                             "<tt>1<sub>16</sub></tt>\tAC table"));

        for (i = 0; i < 16;)
        {
            if (!analyzer_utils_read (&code_counts[i], file , 1))
                return FALSE;

            description_message2 = g_strdup_printf ("%hhu", code_counts[i]);
            description_message1 = g_strdup_printf (_("Codes of length %u"), ++i);

            analyzer_utils_describe_tab (&tab, description_message1, description_message2);

            g_free (description_message1);
            g_free (description_message2);
        }

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 16,
                            _("Number of Huffman codes for 1-16 bit lengths"));

        table_size += 16;

        color_toggle = TRUE;
        for (i = 0; i < 16; i++)
        {
            if (code_counts[i])
            {
                if (color_toggle)
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, code_counts[i], _("Huffman code values"));
                else
                    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, code_counts[i], _("Huffman code values"));

                color_toggle = !color_toggle;

                ADVANCE_POINTER (file, code_counts[i]);
                table_size += code_counts[i];
            }
        }

        if (data_length >= table_size)
            data_length -= table_size;
        else
            data_length = 0;
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[DHT]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
