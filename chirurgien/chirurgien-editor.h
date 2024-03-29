/* chirurgien-editor.h
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

#define CHIRURGIEN_TYPE_EDITOR (chirurgien_editor_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienEditor, chirurgien_editor, CHIRURGIEN, EDITOR, GtkWidget)

GtkWidget *       chirurgien_editor_new                  (void);

void              chirurgien_editor_set_contents         (ChirurgienEditor *,
                                                          const guchar *,
                                                          gsize);
const guchar *    chirurgien_editor_get_contents         (ChirurgienEditor *);
gsize             chirurgien_editor_get_contents_size    (ChirurgienEditor *);

G_END_DECLS
