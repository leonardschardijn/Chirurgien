/* png-plte-chunk.c
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
analyze_plte_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    guint *palette_entries)
{
    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[PLTE]++;

    if (chunk_counts[PLTE] != 1)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                   _("Only one PLTE chunk is allowed"));
        return TRUE;
    }

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    if (chunk_length % 3)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("Incorrect palette length"));
        return TRUE;
    }

    *palette_entries = chunk_length / 3;

    if (*palette_entries > 256)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("Palettes cannot have more than 256 entries"));
        return TRUE;
    }

    for (i = 0; i < *palette_entries; i++)
    {
        if (i % 2)
        {
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                _("Palette entry red sample"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                _("Palette entry green sample"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                _("Palette entry blue sample"));
        }
        else
        {
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                _("Palette entry red sample"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                _("Palette entry green sample"));
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                _("Palette entry blue sample"));
        }
    }

    ADVANCE_POINTER (file, chunk_length);

    return TRUE;
}
