/* format-utils.h
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

#include <chirurgien-formats-file.h>

G_BEGIN_DECLS

typedef struct
{
    /* The description tab */
    GtkBox         *contents;

    /* Current description section grid and lines count */
    GtkGrid        *section;
    guint           description_lines_count;

    /* If the tab has had items added since it was initialized */
    gboolean        used;

} DescriptionTab;


/* File macros */

#define FILE_HAS_DATA(file) (file->file_contents_index < file->file_size)
#define FILE_HAS_DATA_N(file, count) ((file->file_contents_index + count) <= file->file_size)

#define GET_FILE_SIZE(file) (file->file_size)

#define GET_INDEX(file) (file->file_contents_index)
#define SET_INDEX(file, idx) (file->file_contents_index = idx)

#define ADVANCE_INDEX(file, count) (file->file_contents_index += count)

#define GET_CONTENT_POINTER(file) (file->file_contents + file->file_contents_index)
#define GET_CONTENT_POINTER_AT(file, offset) (file->file_contents + offset)


void       format_utils_set_title                 (FormatsFile *,
                                                   const char *);

void       format_utils_start_section             (FormatsFile *,
                                                   const char *);
void       format_utils_start_section_tab         (DescriptionTab *,
                                                   const char *);

void       format_utils_init_tab                  (DescriptionTab *,
                                                   const gchar *);
void       format_utils_insert_tab                (FormatsFile *,
                                                   DescriptionTab *,
                                                   const gchar *);

#define format_utils_add_line(file, field_name, field_value, field_tooltip) \
        format_utils_add_line_full(file, field_name, field_value, field_tooltip, 0, 0)

void       format_utils_add_line_full             (FormatsFile *,
                                                   const char *,
                                                   const char *,
                                                   const char *,
                                                   gint,
                                                   gint);

#define format_utils_add_line_tab(tab, field_name, field_value, field_tooltip) \
        format_utils_add_line_full_tab(tab, field_name, field_value, field_tooltip, 0, 0)

void       format_utils_add_line_full_tab         (DescriptionTab *,
                                                   const char *,
                                                   const char *,
                                                   const char *,
                                                   gint,
                                                   gint);
void       format_utils_add_text_tab              (DescriptionTab *,
                                                   const gchar *,
                                                   const gchar *,
                                                   gsize);

void       format_utils_add_line_no_section       (FormatsFile *,
                                                   const gchar *);
void       format_utils_add_line_no_section_tab   (DescriptionTab *,
                                                   const gchar *);

#define format_utils_add_field(file, color_index, background, field_length, field_name, navigation_label) \
        format_utils_add_field_full(file, color_index, background, field_length, field_name, navigation_label, -1)

void       format_utils_add_field_full            (FormatsFile *,
                                                   gint,
                                                   gboolean,
                                                   guint,
                                                   const gchar *,
                                                   const gchar *,
                                                   gint);

gboolean   format_utils_read                      (FormatsFile *,
                                                   gpointer,
                                                   guint);

G_END_DECLS
