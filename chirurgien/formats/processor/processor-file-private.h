/* processor-file-private.h
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

struct _ProcessorFile
{
    /* File contents and size */
    gconstpointer   file_contents;
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

};

G_END_DECLS
