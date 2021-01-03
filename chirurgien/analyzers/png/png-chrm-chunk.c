/* png-chrm-chunk.c
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
analyze_chrm_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint32 chromaticity;

    gchar *chromaticities[] = {
        _("White point x"),
        _("White point y"),
        _("Red x"),
        _("Red y"),
        _("Green x"),
        _("Green y"),
        _("Blue x"),
        _("Blue y")
    };

    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[cHRM]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Primary chromaticities and white point</b>"));

    for (i = 0; i < 8; i++)
    {
        if (!analyzer_utils_read (&chromaticity, file , 4))
            goto END_ERROR;

        if (i % 2)
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 4, chromaticities[i]);
        else
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4, chromaticities[i]);

        chromaticity = g_ntohl (chromaticity);
        description_message = g_strdup_printf ("%u", chromaticity);
        analyzer_utils_describe_tab (&tab, chromaticities[i], description_message);
        g_free (description_message);
    }

    /* Fixed length chunk */
    if (chunk_length > 32)
    {
        chunk_length -= 32;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: Values represent the 1931 CIE x,y chromaticities times 100000"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[cHRM]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
