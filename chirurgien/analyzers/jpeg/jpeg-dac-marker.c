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

#include <config.h>

#include <arpa/inet.h>
#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_dac_marker (AnalyzerFile *file, guint *marker_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint16 data_length;
    guint8 arithmetic_info;

    marker_counts[DAC]++;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Arithmetic conditioning tables</b>"));

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    if (data_length > 1)
        data_length -= 2;
    else
        return TRUE;

    while (data_length > 0)
    {
        /* Arithmetic conditioning table information */
        if (!analyzer_utils_read (&arithmetic_info, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                            _("Arithmetic conditioning table information\n"
                            "Lower four bits: Arithmetic conditioningtable identifier\n"
                            "Upper four bits: Table class"));

        description_message = g_strdup_printf ("%u", arithmetic_info & 0x0F);
        analyzer_utils_add_description_tab (&tab, _("Arithmetic conditioning table identifier"),
                                            description_message, NULL,
                                            10, 0);
        g_free (description_message);

        arithmetic_info = arithmetic_info >> 4;
        if (!arithmetic_info)
            description_message = _("DC table (or lossless table)");
        else if (arithmetic_info == 1)
            description_message = _("AC table");
        else
            description_message = _("<span foreground=\"red\">INVALID</span>");
        analyzer_utils_describe_tooltip_tab (&tab, _("Table class"), description_message,
                                             _("Table class\n"
                                             "<tt>0<sub>16</sub></tt>\tDC table (or lossless table)\n"
                                             "<tt>1<sub>16</sub></tt>\tAC table"));

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Conditioning table value"));
        ADVANCE_POINTER (file, 1);

        if (data_length > 1)
            data_length -= 2;
        else
            data_length = 0;
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[DAC]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
