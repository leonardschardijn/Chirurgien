/* chirurgien-view.h
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
#include "chirurgien-window.h"
#include "chirurgien-utils.h"

G_BEGIN_DECLS

#define CHIRURGIEN_TYPE_VIEW (chirurgien_view_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienView, chirurgien_view, CHIRURGIEN, VIEW, GtkWidget)

ChirurgienView *     chirurgien_view_new                          (ChirurgienWindow *);

void                 chirurgien_view_set_file                     (ChirurgienView *,
                                                                   GFile *);

void                 chirurgien_view_do_analysis                  (ChirurgienView *);
void                 chirurgien_view_redo_analysis                (ChirurgienView *);

void                 chirurgien_view_select_view                  (ChirurgienView *,
                                                                   ChirurgienViewType);

gboolean             chirurgien_view_save                         (ChirurgienView *,
                                                                   GFile *);

void                 chirurgien_view_undo                         (ChirurgienView *);
void                 chirurgien_view_redo                         (ChirurgienView *);
void                 chirurgien_view_query_modifications          (ChirurgienView *,
                                                                   gboolean *,
                                                                   gboolean *);

void                 chirurgien_view_refresh                      (ChirurgienView *);

GtkWidget *          chirurgien_view_get_view_tab                 (ChirurgienView *);
const gchar *        chirurgien_view_get_file_path                (ChirurgienView *);
gboolean             chirurgien_view_has_file                     (ChirurgienView *);
gboolean             chirurgien_view_unsaved                      (ChirurgienView *);

G_END_DECLS
