/* jpeg-dnl-marker.c
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
analyze_dnl_marker (AnalyzerFile *file, guint *marker_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint16 data_length;
    guint16 lines;

    marker_counts[DNL]++;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Define number of lines</b>"));

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    /* Number of lines */
    if (!analyzer_utils_read (&lines, file , 2))
        goto END_ERROR;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 2, _("Number of lines"));

    lines = ntohs (lines);
    description_message = g_strdup_printf ("%hu", lines);
    analyzer_utils_describe_tab (&tab, _("Number of lines"), description_message);
    g_free (description_message);

    /* Fixed length marker segment */
    if (data_length > 4)
    {
        data_length -= 4;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, data_length);
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[DNL]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
