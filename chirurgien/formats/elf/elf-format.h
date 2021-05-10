/* elf-format.h
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
#define SEGMENT_TYPE_COLOR       4
#define START_SECTION_DATA_COLOR 5
#define ERROR_COLOR              6

gboolean    process_elf_field          (FormatsFile *,
                                        DescriptionTab *,
                                        const gchar *,
                                        const gchar *,
                                        const gchar *,
                                        gint,
                                        guint,
                                        gboolean,
                                        guint,
                                        guint32 *,
                                        const gchar **,
                                        const gchar *,
                                        gboolean,
                                        gpointer);

/* elf-program-header.c */
void        elf_program_header         (FormatsFile *,
                                        gboolean,
                                        gboolean,
                                        guint64,
                                        guint16);

/* elf-program-header.c */
void        elf_section_header         (FormatsFile *,
                                        gboolean,
                                        gboolean,
                                        guint64,
                                        guint16,
                                        guint64);

G_END_DECLS
