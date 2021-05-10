/* chirurgien-window.h
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIRURGIEN_TYPE_WINDOW (chirurgien_window_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienWindow, chirurgien_window, CHIRURGIEN, WINDOW, GtkApplicationWindow)

ChirurgienWindow *    chirurgien_window_new                (GtkApplication *);

void                  chirurgien_window_update_recent      (ChirurgienWindow *,
                                                            GFile *);

void                  chirurgien_window_load_view_font     (ChirurgienWindow *);

void                  chirurgien_window_set_undo           (ChirurgienWindow *,
                                                            gboolean);
void                  chirurgien_window_set_redo           (ChirurgienWindow *,
                                                            gboolean);

GSettings *           chirurgien_window_get_preferences    (ChirurgienWindow *);
GSettings *           chirurgien_window_get_state          (ChirurgienWindow *);

G_END_DECLS
