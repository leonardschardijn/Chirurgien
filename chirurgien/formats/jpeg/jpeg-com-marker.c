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

#include <glib/gi18n.h>

#include "jpeg-format.h"


gboolean
jpeg_com_marker (FormatsFile *file,
                 gint        *marker_counts)
{
    DescriptionTab tab;

    guint16 data_length;
    const gchar *comment;

    marker_counts[COM]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    if (data_length > 1)
        data_length -= 2;
    else
        return TRUE;

    if (!FILE_HAS_DATA_N (file, data_length))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Segment length exceeds available data"), NULL);
        return FALSE;
    }

    format_utils_init_tab (&tab, NULL);

    comment = (const gchar *) GET_CONTENT_POINTER (file);

    format_utils_add_text_tab (&tab, _("Comment"), comment, data_length);
    format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, data_length,
                          _("Comment"), NULL);

    if (!g_utf8_validate (comment, data_length, NULL))
        format_utils_add_line_no_section_tab (&tab, _("Failed to interpret comment, it is not valid UTF-8"));

    format_utils_insert_tab (file, &tab, marker_names[COM]);

    return TRUE;
}
