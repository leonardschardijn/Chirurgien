/* processor-file.h
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

G_BEGIN_DECLS

/* The processor's output: a list of FileFields */
typedef struct
{
    /* The name of the field */
    gchar         *field_name;

    /* Field offset */
    gsize          field_offset;

    /* Field size */
    gsize          field_size;

    /* Field color */
    guint          color_index;

    /* Color is background or foreground color */
    gboolean       background; /* TRUE = background, FALSE = foregound*/

    /* Navigation label */
    gchar         *navigation_label;

    /* Additional color
     * If defined, it is used to color the first byte of the field
     * This helps identify fields when the same color is used to color
     * many adjacent fields, something that happens often in file formats
     * that use offset to point to values somewhere else in the file */
    guint          additional_color_index;

} FileField;

typedef struct _ProcessorFile ProcessorFile;

ProcessorFile *    processor_file_create            (gconstpointer,
                                                     gsize,
                                                     GtkNotebook *,
                                                     GtkBox *);
GSList *           processor_file_get_field_list    (ProcessorFile *);
void               processor_file_destroy           (ProcessorFile *);

G_END_DECLS
