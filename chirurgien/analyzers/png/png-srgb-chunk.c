/* png-srgb-chunk.c
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
analyze_srgb_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint8 intent;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sRGB]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Standard RGB color space</b>"));

    if (!analyzer_utils_read (&intent, file , 1))
        return FALSE;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Rendering intent"));

    if (intent == 0)
        description_message = _("Perceptual");
    else if (intent == 1)
        description_message = _("Relative colorimetric");
    else if (intent == 2)
        description_message = _("Saturation");
    else if (intent == 3)
        description_message = _("Absolute colorimetric");
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (&tab, _("Rendering intent"), description_message,
                                         _("Rendering intent\n"
                                         "<tt>00<sub>16</sub></tt>\tPerceptual\n"
                                         "<tt>01<sub>16</sub></tt>\tRelative colorimetric\n"
                                         "<tt>02<sub>16</sub></tt>\tSaturation\n"
                                         "<tt>03<sub>16</sub></tt>\tAbsolute colorimetric"));

    /* Fixed length chunk */
    if (chunk_length > 1)
    {
        chunk_length--;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[sRGB]);

    return TRUE;
}
