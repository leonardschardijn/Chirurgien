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

#include "png-format.h"


gboolean
png_bkgd_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint8       colortype)
{
    DescriptionTab tab;

    guint32 chunk_used = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[bKGD]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, _("Background color"));

    if (colortype == 0 || colortype == 4)
    {
        if (!process_png_field (file, &tab, _("Grayscale background"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            return FALSE;

        chunk_used += 2;
    }
    else if (colortype == 2 || colortype == 6)
    {
        if (!process_png_field (file, &tab, _("Background red sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, _("Background green sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_2, 2, 0, NULL, NULL, "%u", NULL))
            return FALSE;

        if (!process_png_field (file, &tab, _("Background blue sample"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", NULL))
            return FALSE;

        chunk_used += 6;
    }
    else if (colortype == 3)
    {
        if (!process_png_field (file, &tab, _("Background palette index"), NULL,
                           NULL, CHUNK_DATA_COLOR_1, 1, 0, NULL, NULL, "%u", NULL))
            return FALSE;

        chunk_used++;
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("Invalid color type"), NULL);
        return TRUE;
    }

    if (chunk_used < chunk_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - chunk_used,
                              _("Unrecognized data"), NULL);

    format_utils_insert_tab (file, &tab, chunk_types[bKGD]);

    return TRUE;
}
