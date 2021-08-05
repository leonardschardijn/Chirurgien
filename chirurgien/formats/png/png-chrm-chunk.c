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

#include "png-format.h"

#define CHRM_CHUNK_LENGTH 32


gboolean
png_chrm_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    gint color_toggle;

    const gchar * const chromaticities[] = {
        "White point x",
        "White point y",
        "Red x",
        "Red y",
        "Green x",
        "Green y",
        "Blue x",
        "Blue y"
    };

    if (!chunk_length)
        return TRUE;

    chunk_counts[cHRM]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, "Primary chromaticities and white point");

    for (gint i = 0; i < 8; i++)
    {
        if (i % 2)
            color_toggle = CHUNK_DATA_COLOR_2;
        else
            color_toggle = CHUNK_DATA_COLOR_1;

        if (!process_png_field (file, &tab, chromaticities[i], NULL,
                                NULL,
                                color_toggle, 4,
                                0, NULL, NULL,
                                "%u", NULL))
            return FALSE;
    }

    /* Fixed length chunk */
    if (chunk_length > CHRM_CHUNK_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - CHRM_CHUNK_LENGTH,
                                "Unrecognized data", NULL);

    format_utils_add_line_no_section_tab (&tab, "NOTE: Values represent the 1931 CIE x,y chromaticities times 100000");

    format_utils_insert_tab (file, &tab, chunk_types[cHRM]);

    return TRUE;
}
