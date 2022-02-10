/* chirurgien-globals.h
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIRURGIEN_TOTAL_COLORS 9

extern const gchar * const hex_chars;

extern GdkRGBA    chirurgien_colors[CHIRURGIEN_TOTAL_COLORS];

extern PangoColor pango_colors[CHIRURGIEN_TOTAL_COLORS];
extern guint16    pango_alphas[CHIRURGIEN_TOTAL_COLORS];

extern GSList    *chirurgien_system_format_definitions;
extern GSList    *chirurgien_system_format_descriptions;

extern GList     *chirurgien_user_format_definitions;
extern GList     *chirurgien_user_format_descriptions;


/* Access color names */
const gchar *     get_color_name (gint);

G_END_DECLS
