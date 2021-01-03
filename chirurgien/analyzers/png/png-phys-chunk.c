/* png-phys-chunk.c
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

#include "png-analyzer.h"


gboolean
analyze_phys_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint32 axis;
    guint8 unit;

    if (!chunk_length)
        return TRUE;

    chunk_counts[pHYs]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Intended pixel size or aspect ratio</b>"));

    if (!analyzer_utils_read (&axis, file , 4))
        goto END_ERROR;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4, _("X axis (pixels per unit)"));

    axis = g_ntohl (axis);
    description_message = g_strdup_printf ("%u", axis);
    analyzer_utils_describe_tab (&tab, _("X axis"), description_message);
    g_free (description_message);

    if (!analyzer_utils_read (&axis, file , 4))
            goto END_ERROR;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 4, _("Y axis (pixels per unit)"));

    axis = g_ntohl (axis);
    description_message = g_strdup_printf ("%u", axis);
    analyzer_utils_describe_tab (&tab, _("Y axis"), description_message);
    g_free (description_message);

    if (!analyzer_utils_read (&unit, file, 1))
        return FALSE;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Unit specifier"));

    if (unit == 0)
        description_message = _("Unknown");
    else if (unit == 1)
        description_message = _("Meter");
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (&tab, _("Unit specifier"), description_message,
                                         _("Unit specifier\n"
                                         "<tt>00<sub>16</sub></tt>\tUnknown (pHYs chunk defines aspect ratio)\n"
                                         "<tt>01<sub>16</sub></tt>\tMeter"));

    /* Fixed length chunk */
    if (chunk_length > 9)
    {
        chunk_length -= 9;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[pHYs]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
