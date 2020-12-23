/* png-gama-chunk.c
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

#include "png-analyzer.h"


gboolean
analyze_gama_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint32 gamma;

    if (!chunk_length)
        return TRUE;

    chunk_counts[gAMA]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Image gamma</b>"));

    if (!analyzer_utils_read (&gamma, file , 4))
        goto END_ERROR;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4, _("Image gamma"));

    gamma = ntohl (gamma);
    description_message = g_strdup_printf ("%u", gamma);
    analyzer_utils_describe_tab (&tab, _("Gamma"), description_message);
    g_free (description_message);

    /* Fixed length chunk */
    if (chunk_length > 4)
    {
        chunk_length -= 4;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: Value represents the image gamma times 100000"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[gAMA]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
