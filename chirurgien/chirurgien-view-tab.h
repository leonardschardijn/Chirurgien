/* chirurgien-view-tab.h
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
#include "chirurgien-view.h"

G_BEGIN_DECLS

#define CHIRURGIEN_TYPE_VIEW_TAB (chirurgien_view_tab_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienViewTab, chirurgien_view_tab, CHIRURGIEN, VIEW_TAB, GtkWidget)

GtkWidget *     chirurgien_view_tab_new                     (void);

void            chirurgien_view_tab_set_view                (ChirurgienViewTab *,
                                                             ChirurgienView *);
void            chirurgien_view_tab_set_label               (ChirurgienViewTab *,
                                                             const gchar *,
                                                             const gchar *);

void            chirurgien_view_tab_set_locked              (ChirurgienViewTab *,
                                                             gboolean);
void            chirurgien_view_tab_set_modified            (ChirurgienViewTab *,
                                                             gboolean);
void            chirurgien_view_tab_set_unsaved             (ChirurgienViewTab *,
                                                             gboolean);

void            chirurgien_view_tab_close                   (ChirurgienViewTab *);

G_END_DECLS
