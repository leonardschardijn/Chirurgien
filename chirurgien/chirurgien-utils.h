/* chirurgien-utils.h
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

typedef enum
{
    CHIRURGIEN_HEX_VIEW,
    CHIRURGIEN_TEXT_VIEW
} ChirurgienViewType;

void    chirurgien_utils_hex_print     (gchar  *,
                                        const guchar *,
                                        gsize,
                                        gsize,
                                        gsize,
                                        gsize);
void    chirurgien_utils_text_print    (gchar  *,
                                        const guchar *,
                                        gsize,
                                        gsize,
                                        gsize,
                                        gsize);

G_END_DECLS
