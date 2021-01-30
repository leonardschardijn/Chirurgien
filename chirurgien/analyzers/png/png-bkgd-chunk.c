/* png-bkgd-chunk.c
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
analyze_bkgd_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    guint8 colortype)
{
    AnalyzerTab tab;

    gsize chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[bKGD]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Background color</b>"));

    if (colortype == 0 || colortype == 4)
    {
        if (!process_png_field (file, &tab, _("Grayscale background"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        chunk_used += 2;
    }
    else if (colortype == 2 || colortype == 6)
    {
        if (!process_png_field (file, &tab, _("Background red sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        if (!process_png_field (file, &tab, _("Background green sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_2, 2, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        if (!process_png_field (file, &tab, _("Background blue sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        chunk_used += 6;
    }
    else if (colortype == 3)
    {
        if (!process_png_field (file, &tab, _("Background palette index"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 1, 0, NULL, NULL, "%u", NULL))
            goto END_ERROR;

        chunk_used++;
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid color type"));

        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (chunk_used < chunk_length)
    {
        chunk_length -= chunk_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[bKGD]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
