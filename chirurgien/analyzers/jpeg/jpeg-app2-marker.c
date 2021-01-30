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

#include "jpeg-analyzer.h"


gboolean
analyze_app2_marker (AnalyzerFile *file,
                     guint *marker_counts)
{
    const guchar icc_profile_identifier[] = "ICC_PROFILE"; // ICC_PROFILE\0

    guint16 data_length;
    guchar identifier[12];

    marker_counts[APP2]++;

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = g_ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    /* Identifier */
    if (!analyzer_utils_read (&identifier, file , 12))
        goto END_ERROR;

    if (!memcmp (identifier, icc_profile_identifier, 12))
    {
        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 12, _("ICC profile identifier"));

        if (FILE_HAS_DATA_N (file, 2))
        {
            analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("ICC profile chunk number"));
            analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("Total ICC profile chunks"));
            ADVANCE_POINTER (file, 2);
        }
        else
        {
            goto END_ERROR;
        }

        if (data_length > 16)
        {
            data_length -= 16;
            analyzer_utils_tag (file, MARKER_DATA_COLOR_2, data_length, _("Embedded ICC profile"));

            ADVANCE_POINTER (file, data_length);
        }
    }
    else
    {
        if (data_length > 14)
        {
            data_length -= 2;
            analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

            ADVANCE_POINTER (file, data_length - 12);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 12, _("Unrecognized data"));
        }
    }

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
