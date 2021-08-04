/* pe-format.h
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

#include <format-utils.h>

G_BEGIN_DECLS

#define SIGNATURE_COLOR          0
#define SECTION_NAME_COLOR       1
#define HEADER_DATA_COLOR_1      2
#define HEADER_DATA_COLOR_2      3
#define DATA_DIRECTORY_COLOR     4
#define START_SECTION_DATA_COLOR 5

#define process_pe_field_simple(file, tab, field_name, field_tooltip, color_index, field_length, field_format) \
        process_pe_field(file, tab, field_name, NULL, field_tooltip, color_index, field_length, 0, NULL, NULL, field_format, NULL)

gboolean    process_pe_field          (FormatsFile *,
                                       DescriptionTab *,
                                       const gchar *,
                                       const gchar *,
                                       const gchar *,
                                       gint,
                                       guint,
                                       guint,
                                       guint32 *,
                                       const gchar **,
                                       const gchar *,
                                       gpointer);

/* pe-data-directory.c */
void       pe_data_directories        (FormatsFile *,
                                       guint32);

/* pe-section-table.c */
void       pe_section_table           (FormatsFile *,
                                       guint16,
                                       guint32);

G_END_DECLS
