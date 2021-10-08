/* chirurgien-actions.h
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
#include "chirurgien-view.h"

G_BEGIN_DECLS

/* Application actions */
void       chirurgien_actions_disable_csd        (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_preferences        (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_formats            (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_shortcuts          (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_about              (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_quit               (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);

/* Window actions */
void       chirurgien_actions_open               (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_save               (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_close              (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_reanalyze          (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_hex_view           (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_text_view          (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_next_tab           (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_undo               (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_redo               (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_previous_tab       (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);
void       chirurgien_actions_recent_open        (GSimpleAction *,
                                                  GVariant *,
                                                  gpointer);

/* Others */
void       chirurgien_actions_new_view           (ChirurgienWindow *,
                                                  GFile *);
void       chirurgien_actions_show_view          (ChirurgienWindow *,
                                                  ChirurgienView *);

G_END_DECLS
