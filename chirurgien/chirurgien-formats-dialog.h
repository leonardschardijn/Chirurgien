/* chirurgien-formats-dialog.h
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

#define CHIRURGIEN_TYPE_FORMATS_DIALOG (chirurgien_formats_dialog_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienFormatsDialog, chirurgien_formats_dialog, CHIRURGIEN, FORMATS_DIALOG, GtkDialog)

GtkWidget *    chirurgien_formats_dialog_new            (GtkWindow *);

void           chirurgien_formats_dialog_set_message    (ChirurgienFormatsDialog *,
                                                         const gchar *);
void           chirurgien_formats_dialog_add_format     (ChirurgienFormatsDialog *);

G_END_DECLS
