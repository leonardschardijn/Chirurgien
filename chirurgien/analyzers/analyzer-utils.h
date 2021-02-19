/* analyzer-utils.h
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
#include "chirurgien-analyzer-file.h"

G_BEGIN_DECLS

typedef struct
{
    /* The description tab */
    GtkWidget *contents;

    /* Current description grid */
    GtkGrid *description;

    /* Lines in the current description tab */
    guint description_lines_count;

} AnalyzerTab;


/* The following macros serve as frequent uses of the functions declared below */

/*
 * Set title, the title cannot be translated
 */
#define analyzer_utils_set_title(file, title) \
        analyzer_utils_add_description(file, title, NULL, NULL, 0, 20)

/*
 * Set subtitle
 */
#define analyzer_utils_set_subtitle(file, title) \
        analyzer_utils_add_description(file, title, NULL, NULL, 20, 20)

/*
 * Set tab subtitle
 */
#define analyzer_utils_set_title_tab(tab, title) \
        analyzer_utils_add_description_tab(tab, title, NULL, NULL, 0, 20)

/*
 * Create a color tag in hexadecimal/text view
 */
#define analyzer_utils_tag(file, color, count, tag) \
        analyzer_utils_create_tag(file, color, TRUE, count, tag, NULL)

/*
 * Create a color tag in hexadecimal/text view and create a navigation button
 */
#define analyzer_utils_tag_navigation(file, color, count, tag, navigation_tag) \
        analyzer_utils_create_tag(file, color, TRUE, count, tag, navigation_tag)

/*
 * Create an error color tag in the hexadecimal/text view
 */
#define analyzer_utils_tag_error(file, color, count, tag) \
        analyzer_utils_create_tag(file, color, FALSE, count, tag, NULL)

/*
 * Create an error color tag in the hexadecimal/text view and create a navigation button
 */
#define analyzer_utils_tag_navigation_error(file, color, count, tag, navigation_tag) \
        analyzer_utils_create_tag(file, color, FALSE, count, tag, navigation_tag)

/*
 * Add a description line
 */
#define analyzer_utils_describe(file, field, value) \
        analyzer_utils_add_description (file, field, value, NULL, 0, 0)

/*
 * Add a description line with an informative tooltip
 */
#define analyzer_utils_describe_tooltip(file, field, value, tooltip) \
        analyzer_utils_add_description (file, field, value, tooltip, 0, 0)

/*
 * Add a description line to a tab
 */
#define analyzer_utils_describe_tab(tab, field, value) \
        analyzer_utils_add_description_tab (tab, field, value, NULL, 0, 0)

/*
 * Add a description line to a tab with an informative tooltip
 */
#define analyzer_utils_describe_tooltip_tab(tab, field, value, tooltip) \
        analyzer_utils_add_description_tab (tab, field, value, tooltip, 0, 0)

/* Additional macros */

/*
 * Get file size
 */
#define GET_FILE_SIZE(file) file->file_size

/*
 * Get contents pointer
 */
#define GET_POINTER(file) file->file_contents_index

/*
 * Sets the contents pointer and the hex/text color tags pointer
 */
#define SET_POINTER(file, count) file->file_contents_index = count;\
                                 file->hex_buffer_index = (count) * 3

/*
 * Advance contents pointer, useful when the data is of no interest as a substitute of analyzer_utils_read
 */
#define ADVANCE_POINTER(file, count) file->file_contents_index += count

/*
 * Skip data, advances the contents pointer and the hex/text color tags pointer
 */
#define SKIP_DATA(file, count) file->file_contents_index += count;\
                               file->hex_buffer_index += (count) * 3

/*
 * If the file has data
 */
#define FILE_HAS_DATA(file) (file->file_contents_index < file->file_size)

/*
 * If the file has 'count' bytes available
 */
#define FILE_HAS_DATA_N(file, count) ((file->file_contents_index + count) <= file->file_size)

extern GdkRGBA colors[9];

void       analyzer_utils_create_tag             (AnalyzerFile *,
                                                  GdkRGBA *,
                                                  gboolean,
                                                  glong,
                                                  gchar *,
                                                  gchar *);
void       analyzer_utils_create_tag_index       (AnalyzerFile *,
                                                  GdkRGBA *,
                                                  gboolean,
                                                  glong,
                                                  gsize,
                                                  gchar *);

void       analyzer_utils_add_description        (AnalyzerFile *,
                                                  gchar *,
                                                  gchar *,
                                                  gchar *,
                                                  guint,
                                                  guint);
void       analyzer_utils_insert_notice          (AnalyzerFile *,
                                                  gchar *,
                                                  guint,
                                                  guint);

void       analyzer_utils_add_description_tab    (AnalyzerTab *,
                                                  gchar *,
                                                  gchar *,
                                                  gchar *,
                                                  guint,
                                                  guint);
void       analyzer_utils_add_text_tab           (AnalyzerTab *,
                                                  gchar *,
                                                  gchar *,
                                                  gsize);
void       analyzer_utils_add_footer_tab         (AnalyzerTab *,
                                                  gchar *);
void       analyzer_utils_embedded_file          (AnalyzerFile *,
                                                  AnalyzerTab *,
                                                  gsize);
void       analyzer_utils_init_tab               (AnalyzerTab *);
void       analyzer_utils_insert_tab             (AnalyzerFile *,
                                                  AnalyzerTab *,
                                                  gchar *);

gsize      analyzer_utils_advance_to             (AnalyzerFile *,
                                                  guchar);
gboolean   analyzer_utils_read                   (void *,
                                                  AnalyzerFile *,
                                                  gsize);

gint       tagged_bytes_compare                  (gconstpointer,
                                                  gconstpointer);


G_END_DECLS
