/* jpeg-app2-marker.c
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
jpeg_app2_marker (FormatsFile *file,
                  gint        *marker_counts)
{
    const gchar * const icc_profile_identifier = "ICC_PROFILE"; // ICC_PROFILE\0

    const gchar *identifier;
    guint16 data_length, data_used = 2;

    marker_counts[APP2]++;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    identifier = (const gchar *) GET_CONTENT_POINTER (file);

    if (FILE_HAS_DATA_N (file, 12) && !memcmp (identifier, icc_profile_identifier, 12))
    {
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 12,
                              _("ICC profile identifier"), NULL);

        format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                              _("ICC profile chunk number"), NULL);
        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                              _("Total ICC profile chunks"), NULL);

        data_used += 14;

        if (data_length > data_used)
        {
            format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, data_length - data_used,
                                  _("Embedded ICC profile"), NULL);
            data_used = data_length;
        }
    }

    if (data_used < data_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                              _("Unrecognized data"), NULL);

    return TRUE;
}
