/* png-srgb-chunk.c
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
png_srgb_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sRGB]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, _("Standard RGB color space"));

    guint8 rendering_intent_values[] = { 0x0, 0x1, 0x2, 0x3 };
    const gchar *rendering_intent_value_description[] = {
        _("Perceptual"),
        _("Relative colorimetric"),
        _("Saturation"),
        _("Absolute colorimetric"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, &tab, _("Rendering intent"), NULL,
                       _("Rendering intent\n"
                         "<tt>00<sub>16</sub></tt>\tPerceptual\n"
                         "<tt>01<sub>16</sub></tt>\tRelative colorimetric\n"
                         "<tt>02<sub>16</sub></tt>\tSaturation\n"
                         "<tt>03<sub>16</sub></tt>\tAbsolute colorimetric"),
                       CHUNK_DATA_COLOR_1, 1,
                       sizeof (rendering_intent_values), rendering_intent_values, rendering_intent_value_description,
                       NULL, NULL))
        return FALSE;

    /* Fixed length chunk */
    if (chunk_length > 1)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - 1,
                              _("Unrecognized data"), NULL);

    format_utils_insert_tab (file, &tab, chunk_types[sRGB]);

    return TRUE;
}
