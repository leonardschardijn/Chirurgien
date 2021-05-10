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

#include "jpeg-format.h"

#define DRI_DNL_SEGMENT_LENGTH 4


gboolean
jpeg_dri_dnl_marker (FormatsFile *file,
                     gint        *marker_counts,
                     gint         marker_type)
{
    DescriptionTab tab;

    const gchar *field_name;
    guint16 data_length;

    marker_counts[marker_type]++;

    if (marker_type == DRI)
    {
        format_utils_init_tab (&tab, _("Restart interval"));
        field_name = _("Restart interval");
    }
    else
    {
        format_utils_init_tab (&tab, _("Define number of lines"));
        field_name = _("Number of lines");
    }

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    /* Restart interval / Number of lines */
    if (!process_jpeg_field (file, &tab, field_name, NULL, NULL,
                             MARKER_DATA_COLOR_1, 2,
                             0, NULL, NULL, "%u", NULL))
        return FALSE;

    /* Fixed length marker segment */
    if (data_length > DRI_DNL_SEGMENT_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - DRI_DNL_SEGMENT_LENGTH,
                              _("Unrecognized data"), NULL);

    format_utils_insert_tab (file, &tab, marker_names[marker_type]);

    return TRUE;
}
