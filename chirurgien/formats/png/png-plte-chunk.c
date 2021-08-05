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

#include "png-format.h"


gboolean
png_plte_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint       *palette_entries)
{
    if (!chunk_length)
        return TRUE;

    chunk_counts[PLTE]++;

    if (chunk_counts[PLTE] != 1)
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "Only one PLTE chunk is allowed", NULL);
        return TRUE;
    }

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    if (chunk_length % 3)
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "Incorrect palette length (not a multiple of 3)", NULL);
        return TRUE;
    }

    *palette_entries = chunk_length / 3;

    if (*palette_entries > 256)
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "Palettes cannot have more than 256 entries", NULL);
        return TRUE;
    }

    for (guint i = 0; i < *palette_entries; i++)
    {
        if (i % 2)
            format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 3,
                                    "Palette entry", NULL );
        else
            format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, 3,
                                    "Palette entry", NULL );
    }

    return TRUE;
}
