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
#include "chirurgien-application.h"

G_BEGIN_DECLS

#define CHIRURGIEN_WINDOW_TYPE (chirurgien_window_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienWindow, chirurgien_window, CHIRURGIEN, WINDOW, GtkApplicationWindow)

struct _ChirurgienWindow
{
    GtkApplicationWindow   parent_instance;

    /* Client-side decorations */
    GtkWidget              *headerbar;
    /* Window Manager decorations */
    GtkWidget              *recent_files;
    GMenu                  *recent_menu;

    GtkWidget              *notebook;

    GdkWindowState         window_state;

    GSettings              *preferences_settings;
    GSettings              *window_settings;

    GtkWidget              *analyzing_message;
    GtkWidget              *processing_message;
};


ChirurgienWindow *    chirurgien_window_new              (ChirurgienApplication *);

void                  chirurgien_window_update_recent    (ChirurgienWindow *);

G_END_DECLS

