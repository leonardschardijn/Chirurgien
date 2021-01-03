/* png-trns-chunk.c
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
analyze_trns_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    guint8 colortype,
                    guint palette_entries)
{
    AnalyzerTab tab;

    gchar *description_message;

    guint16 alpha;

    guint i;
    gsize chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tRNS]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Transparency</b>"));

    if (colortype == 0)
    {
        if (!analyzer_utils_read (&alpha, file , 2))
            goto END_ERROR;

        chunk_used += 2;
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2, _("Grayscale alpha channel"));

        alpha = g_ntohs (alpha);
        description_message = g_strdup_printf (_("%u bits"), alpha);
        analyzer_utils_describe_tab (&tab, _("Grayscale alpha channel"), description_message);
        g_free (description_message);
    }
    else if (colortype == 2)
    {
        if (!analyzer_utils_read (&alpha, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2, _("Red alpha channel"));

        alpha = g_ntohs (alpha);
        description_message = g_strdup_printf (_("%u bits"), alpha);
        analyzer_utils_describe_tab (&tab, _("Red alpha channel"), description_message);
        g_free (description_message);

        if (!analyzer_utils_read (&alpha, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2, _("Green alpha channel"));

        alpha = g_ntohs (alpha);
        description_message = g_strdup_printf (_("%u bits"), alpha);
        analyzer_utils_describe_tab (&tab, _("Green alpha channel"), description_message);
        g_free (description_message);

        if (!analyzer_utils_read (&alpha, file , 2))
            goto END_ERROR;

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2, _("Blue alpha channel"));

        alpha = g_ntohs (alpha);
        description_message = g_strdup_printf (_("%u bits"), alpha);
        analyzer_utils_describe_tab (&tab, _("Blue alpha channel"), description_message);
        g_free (description_message);

        chunk_used += 6;
    }
    else if (colortype == 3)
    {
        if (palette_entries < chunk_length)
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                      _("The tRNS chunk has more alpha values than palette entries"));
            return TRUE;
        }

        for (i = 0; i < chunk_length; i++)
        {
            if (i % 2)
                analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Palette entry alpha"));
            else
                analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Palette entry alpha"));
        }

        ADVANCE_POINTER (file, chunk_length);
        chunk_length -= chunk_length;

        description_message = g_strdup_printf ("%u", palette_entries);
        analyzer_utils_describe_tab (&tab, _("Palette entries"), description_message);
        g_free (description_message);

        description_message = g_strdup_printf ("%lu", chunk_length);
        analyzer_utils_describe_tab (&tab, _("Alpha entries"), description_message);
        g_free (description_message);

        description_message = g_strdup_printf ("%lu", chunk_length - palette_entries);
        analyzer_utils_describe_tab (&tab, _("Palette entries without alpha"), description_message);
        g_free (description_message);

        chunk_used += chunk_length;
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("tRNS chunks are only valid in grayscale, RGB and indexed-color images"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (chunk_used < chunk_length)
    {
        chunk_length -= chunk_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[tRNS]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
