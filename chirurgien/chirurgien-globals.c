/* chirurgien-globals.c
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

#include "chirurgien-globals.h"

const gchar * const hex_chars = "0123456789ABCDEF";

GdkRGBA    chirurgien_colors[CHIRURGIEN_TOTAL_COLORS];
PangoColor pango_colors[CHIRURGIEN_TOTAL_COLORS];
guint16    pango_alphas[CHIRURGIEN_TOTAL_COLORS];

GSList    *chirurgien_system_format_definitions = NULL;
GSList    *chirurgien_system_format_descriptions = NULL;

GList     *chirurgien_user_format_definitions = NULL;
GList     *chirurgien_user_format_descriptions = NULL;

/* Color name */
gchar      chirurgien_color_name[7] = {'c', 'o', 'l', 'o', 'r', '\0', '\0'};

const gchar *
get_color_name (gint color_index)
{
    if (color_index >= CHIRURGIEN_TOTAL_COLORS)
        return NULL;

    chirurgien_color_name[5] = '0' + color_index;
    return chirurgien_color_name;
}
