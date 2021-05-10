/* chirurgien-formats-file.h
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

typedef struct
{
    /* The name of the field */
    const gchar    *field_name;

    /* Field offset */
    gsize           field_offset;

    /* Field length */
    guint           field_length;

    /* Field color */
    gint            color_index;

    /* Color is background or foreground color */
    gboolean        background; /* TRUE = background, FALSE = foregound*/

    /* Navigation label */
    const gchar    *navigation_label;

    /* Additional color
     * If defined, it is used to color the first byte of the field
     * This helps identify fields when the same color is used to color
     * many adjacent fields, something that happens often in file formats
     * that use offset to point to values somewhere else in the file */
    gint            additional_color_index;

} FileField;

typedef struct
{
    /* File contents and size */
    const guchar   *file_contents;
    gsize           file_size;

    /* Current file contents index */
    gsize           file_contents_index;

    /* File fields */
    GSList         *file_fields;

    /* Description panel container */
    GtkNotebook    *description;

    /* Main description page: 'Overview' page */
    GtkBox         *overview;

    /* Current description section grid and lines count */
    GtkGrid        *section;
    guint           description_lines_count;

} FormatsFile;

G_END_DECLS
