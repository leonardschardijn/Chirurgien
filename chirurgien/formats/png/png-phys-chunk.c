/* png-phys-chunk.c
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

#define PHYS_CHUNK_LENGTH 9


gboolean
png_phys_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    if (!chunk_length)
        return TRUE;

    chunk_counts[pHYs]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, _("Intended pixel size or aspect ratio"));

    if (!process_png_field (file, &tab, _("X axis"), _("X axis (pixels per unit)"),
                       NULL, CHUNK_DATA_COLOR_1, 4, 0, NULL, NULL, "%u", NULL))
        return FALSE;

    if (!process_png_field (file, &tab, _("Y axis"), _("Y axis (pixels per unit)"),
                       NULL, CHUNK_DATA_COLOR_2, 4, 0, NULL, NULL, "%u", NULL))
        return FALSE;

    guint8 unit_values[] = { 0x0, 0x1 };
    const gchar *unit_value_description[] = {
        _("Unknown"),
        _("Meter"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, &tab, _("Unit specifier"), NULL,
                       _("Unit specifier\n"
                         "<tt>00<sub>16</sub></tt>\tUnknown (pHYs chunk defines aspect ratio)\n"
                         "<tt>01<sub>16</sub></tt>\tMeter"),
                       CHUNK_DATA_COLOR_1, 1,
                       sizeof (unit_values), unit_values, unit_value_description,
                       NULL, NULL))
        return FALSE;

    /* Fixed length chunk */
    if (chunk_length > PHYS_CHUNK_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - PHYS_CHUNK_LENGTH,
                              _("Unrecognized data"), NULL);

    format_utils_insert_tab (file, &tab, chunk_types[pHYs]);

    return TRUE;
}
