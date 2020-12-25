/* chirurgien-analyzer-file.h
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
    /* Hexadecimal/text view buffers */
    GtkTextBuffer   *hex_buffer;
    GtkTextBuffer   *text_buffer;

    /* Navigation mark lists, used to create the navigation buttons */
    GSList          *hex_navigation_marks;
    GSList          *text_navigation_marks;

    /* The main description tab */
    GtkGrid         *file_description;

    /* Notebook with all description tab, used by some analyzers to add further tabs */
    GtkNotebook     *description_notebook;

    /* Analyzed file contents and size */
    guchar          *file_contents;
    gsize           file_size;

    /* How many bytes have been used: file and hex/text buffers */
    gsize           file_contents_index;
    gsize           hex_buffer_index;

    /* Lines in the main description tab */
    guint           description_lines_count;

    /* List of (offset, size) embedded file pairs */
    GSList          *embedded_files;

    /* Number of embedded files */
    guint           embedded_files_count;
} AnalyzerFile;

G_END_DECLS
