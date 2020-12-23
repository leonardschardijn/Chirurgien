/* png-sbit-chunk.c
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
analyze_sbit_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts, guint8 colortype)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint8 significant_bits;
    gsize chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sBIT]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Original number of significant bits</b>"));

    if (!analyzer_utils_read (&significant_bits, file, 1))
        return FALSE;

    if (colortype == 0 || colortype == 4)
    {
        chunk_used++;
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                            _("Grayscale sample significant bits"));

        description_message = g_strdup_printf (_("%u bits"), significant_bits);
        analyzer_utils_describe_tab (&tab, _("Grayscale sample"), description_message);
        g_free (description_message);
    }
    else if (colortype == 2 || colortype == 3 || colortype == 6)
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                            _("Red sample significant bits"));

        description_message = g_strdup_printf (_("%u bits"), significant_bits);
        analyzer_utils_describe_tab (&tab, _("Red sample"), description_message);
        g_free (description_message);

        if (!analyzer_utils_read (&significant_bits, file, 1))
            return FALSE;

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                            _("Green sample significant bits"));

        description_message = g_strdup_printf (_("%u bits"), significant_bits);
        analyzer_utils_describe_tab (&tab, _("Green sample"), description_message);
        g_free (description_message);

        if (!analyzer_utils_read (&significant_bits, file, 1))
            return FALSE;

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                            _("Blue sample significant bits"));

        description_message = g_strdup_printf (_("%u bits"), significant_bits);
        analyzer_utils_describe_tab (&tab, _("Blue sample"), description_message);
        g_free (description_message);

        chunk_used += 3;
    }

    if (colortype == 4 || colortype == 6)
    {
        if (!analyzer_utils_read (&significant_bits, file, 1))
            return FALSE;

        chunk_used++;
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                            _("Alpha sample significant bits"));

        description_message = g_strdup_printf (_("%u bits"), significant_bits);
        analyzer_utils_describe_tab (&tab, _("Alpha sample"), description_message);
        g_free (description_message);
    }

    if (chunk_used < chunk_length)
    {
        chunk_length -= chunk_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[sBIT]);

    return TRUE;
}
