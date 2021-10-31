/* webp-anim-chunk.c
 *
 * Copyright (C) 2021 - Daniel Léonard Schardijn
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

#include "webp-format.h"


void
webp_anim_chunk (FormatsFile *file,
                 RiffFile    *riff_file,
                 guint32      chunk_size)
{
    DescriptionTab tab;
    gchar *value;

    guint16 loop_count;

    if (!FILE_HAS_DATA_N (file, 4))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 4,
                            "Background color (BGRA order)", NULL);

    /* Loop count */
    if (!format_utils_read (file, &loop_count, 2))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 2,
                            "Loop count", NULL);

    format_utils_init_tab (&tab, "Animation");

    if (!loop_count)
        value = g_strdup ("Loop infinitely");
    else
        value = g_strdup_printf ("%hu", loop_count);

    format_utils_add_line_tab (&tab, "Loop count", value,
                               "The number of times to loop the animation, 0 means infinitely");
    g_free (value);

    format_utils_insert_tab (file, &tab,
                             riff_file->chunk_types[riff_file->chunk_selection]);

    if (chunk_size > 6)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_size - 6,
                                "Unrecognized data", NULL);
}
