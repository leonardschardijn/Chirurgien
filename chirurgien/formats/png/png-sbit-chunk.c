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

#include "png-format.h"


gboolean
png_sbit_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint8       colortype)
{
    DescriptionTab tab;

    guint32 chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sBIT]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, "Original number of significant bits");

    if (colortype == 0 || colortype == 4)
    {
        if (!process_png_field (file, &tab, "Grayscale sample",
                                "Grayscale sample significant bits",
                                NULL,
                                CHUNK_DATA_COLOR_1, 1,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        chunk_used++;
    }
    else if (colortype == 2 || colortype == 3 || colortype == 6)
    {
        if (!process_png_field (file, &tab, "Red sample",
                                "Red sample significant bits",
                                NULL,
                                CHUNK_DATA_COLOR_1, 1,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, "Green sample",
                               "Green sample significant bits",
                                NULL,
                                CHUNK_DATA_COLOR_2, 1,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, "Blue sample",
                                "Blue sample significant bits",
                                NULL,
                                CHUNK_DATA_COLOR_1, 1,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        chunk_used += 3;
    }

    if (colortype == 4 || colortype == 6)
    {
        if (!process_png_field (file, &tab, "Alpha sample",
                                "Alpha sample significant bits",
                                NULL,
                                CHUNK_DATA_COLOR_2, 1,
                                0, NULL, NULL,
                                "%u bits", NULL))
            return FALSE;

        chunk_used++;
    }

    if (chunk_used < chunk_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - chunk_used,
                                "Unrecognized data", NULL);

    format_utils_insert_tab (file, &tab, chunk_types[sBIT]);

    return TRUE;
}
