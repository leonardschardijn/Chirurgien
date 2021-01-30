/* jpeg-dri-dnl-marker.c
 *
 * Copyright (C) 2021 - Daniel Léonard Schardijn
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
analyze_dri_dnl_marker (AnalyzerFile *file,
                        guint *marker_counts,
                        gint marker_type)
{
    AnalyzerTab tab;

    gchar *field_name;
    guint16 data_length;

    marker_counts[marker_type]++;

    analyzer_utils_init_tab (&tab);

    if (marker_type == DRI)
    {
        analyzer_utils_set_title_tab (&tab, _("<b>Restart interval</b>"));
        field_name = _("Restart interval");
    }
    else
    {
        analyzer_utils_set_title_tab (&tab, _("<b>Define number of lines</b>"));
        field_name = _("Number of lines");
    }

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    /* Number of lines */
    if (!process_jpeg_field (file, &tab, field_name, NULL, NULL,
                             MARKER_DATA_COLOR_1, 2,
                             0, NULL, NULL, "%u", NULL))
        goto END_ERROR;

    /* Fixed length marker segment */
    if (data_length > 4)
    {
        data_length -= 4;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, data_length);
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[marker_type]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
