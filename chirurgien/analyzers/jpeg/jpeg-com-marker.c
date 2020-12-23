/* jpeg-com-marker.c
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
analyze_com_marker (AnalyzerFile *file, guint *marker_counts)
{
    AnalyzerTab tab;

    guint16 data_length;
    g_autofree gchar *comment = NULL;

    marker_counts[COM]++;

    analyzer_utils_init_tab (&tab);

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    if (data_length > 1)
        data_length -= 2;
    else
        return TRUE;

    comment = g_malloc (data_length);

    /* Data length */
    if (!analyzer_utils_read (comment, file , data_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Segment length exceeds available data"));

        return FALSE;
    }
    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, data_length, _("Comment"));

    if (g_utf8_validate (comment, data_length, NULL))
    {
        analyzer_utils_add_text_tab (&tab, _("Comment"), comment, data_length);
    }
    else
    {
        analyzer_utils_add_text_tab (&tab, _("Comment"), "", strlen(""));
        analyzer_utils_add_footer_tab (&tab, _("Failed to interpret comment, it is not valid UTF-8"));
    }

    analyzer_utils_insert_tab (file, &tab, marker_names[COM]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
