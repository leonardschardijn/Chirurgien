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

#include "png-format.h"

#define GAMA_CHUNK_LENGTH 4


gboolean
png_gama_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    if (!chunk_length)
        return TRUE;

    chunk_counts[gAMA]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, "Image gamma");

    if (!process_png_field (file, &tab, "Image gamma", NULL,
                            NULL,
                            CHUNK_DATA_COLOR_1, 4,
                            0, NULL, NULL,
                            "%u", NULL))
        return FALSE;

    /* Fixed length chunk */
    if (chunk_length > GAMA_CHUNK_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - GAMA_CHUNK_LENGTH,
                                "Unrecognized data", NULL);

    format_utils_add_line_no_section_tab (&tab, "NOTE: Value represents the image gamma times 100000");

    format_utils_insert_tab (file, &tab, chunk_types[gAMA]);

    return TRUE;
}
