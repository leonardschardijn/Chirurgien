/* chirurgien-analyzer-info-dialog.h
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

#define CHIRURGIEN_ANALYZER_INFO_DIALOG_TYPE (chirurgien_analyzer_info_dialog_get_type ())

G_DECLARE_FINAL_TYPE (ChirurgienAnalyzerInfoDialog, chirurgien_analyzer_info, CHIRURGIEN, ANALYZER_INFO_DIALOG, GtkDialog)


GtkWidget * chirurgien_analyzer_info_dialog_new (GtkWindow *);


G_END_DECLS
