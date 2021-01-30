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

#include <config.h>

#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_dqt_marker (AnalyzerFile *file,
                    guint *marker_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint16 data_length;
    guint8 qt_id_precision;
    guint qt_length;

    marker_counts[DQT]++;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Quantization tables</b>"));

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
        /* Quantization table information */
        if (!analyzer_utils_read (&qt_id_precision, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                            _("Quantization table information\n"
                            "Lower four bits: Quantization table identifier\n"
                            "Upper four bits: Quantization table precision"));

        description_message = g_strdup_printf ("%u", qt_id_precision & 0x0F);
        analyzer_utils_describe_tab (&tab, _("Quantization table identifier"), description_message);
        g_free (description_message);

        if (!(qt_id_precision & 0xF0))
            qt_length = 64;
        else
            qt_length = 128;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, qt_length,
                            _("Quantization table values"));

        ADVANCE_POINTER (file, qt_length);

        if (data_length >= qt_length + 1)
            data_length -= (qt_length + 1);
        else
            data_length = 0;

        qt_id_precision = qt_id_precision >> 4;
        if (!qt_id_precision)
            description_message = _("8 bit values");
        else if (qt_id_precision == 1)
            description_message = _("16 bit values");
        else
            description_message = _("<span foreground=\"red\">INVALID</span>");

        analyzer_utils_add_description_tab (&tab, _("Quantization table precision"), description_message,
                                            _("Quantization table precision\n"
                                              "<tt>0<sub>16</sub></tt>\t8 bit values\n"
                                              "<tt>1<sub>16</sub></tt>\t16 bit values"),
                                            0, 10);
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[DQT]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
