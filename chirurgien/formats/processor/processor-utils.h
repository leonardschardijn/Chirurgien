/* processor-utils.h
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

#include <chirurgien-types.h>

#include "processor-file.h"
#include "processor-file-private.h"

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

typedef enum
{
    READ_VARIABLE = 1,
    READ_NUMERIC = 1 << 1

} ReadValueType;

/* File macros */

#define FILE_HAS_DATA(file) (file->file_contents_index < file->file_size)
#define FILE_HAS_DATA_N(file, count) ((file->file_contents_index + count) <= file->file_size)

#define FILE_AVAILABLE_DATA(file) (FILE_HAS_DATA(file) ? file->file_size - file->file_contents_index : 0)

#define GET_CONTENT_POINTER(file) (file->file_contents + file->file_contents_index)

/* Description panel functions */

void                processor_utils_set_title             (ProcessorFile *,
                                                           const char *);
void                processor_utils_start_section         (ProcessorFile *,
                                                           const char *);
void                processor_utils_add_line              (ProcessorFile *,
                                                           const char *,
                                                           const char *,
                                                           const char *,
                                                           gint,
                                                           gint);

/* Description panel tab functions */

DescriptionTab *    processor_utils_new_tab               (const gchar *);
void                processor_utils_start_section_tab     (DescriptionTab *,
                                                           const char *);
void                processor_utils_add_line_tab          (DescriptionTab *,
                                                           const char *,
                                                           const char *,
                                                           const char *,
                                                           gint,
                                                           gint);
void                processor_utils_add_note_tab          (DescriptionTab *,
                                                           const gchar *);
void                processor_utils_add_text_tab          (DescriptionTab *,
                                                           const gchar *,
                                                           const gchar *,
                                                           gsize,
                                                           TextEncoding);

void                processor_utils_insert_tab            (ProcessorFile *,
                                                           DescriptionTab *,
                                                           const gchar *);

/* Processor execution helper functions */

void                processor_utils_add_field             (ProcessorFile *,
                                                           guint,
                                                           gboolean,
                                                           gsize,
                                                           const gchar *,
                                                           const gchar *,
                                                           guint);
gboolean            processor_utils_read                  (const FormatDefinition *,
                                                           const ProcessorState *,
                                                           const ProcessorFile *,
                                                           const FieldDefinition *,
                                                           gboolean,
                                                           gpointer);
void                processor_utils_format_byte_order     (const FormatDefinition *,
                                                           const ProcessorState *,
                                                           gpointer,
                                                           gsize);
void                processor_utils_read_value            (const ProcessorState *,
                                                           const gchar *,
                                                           ReadValueType,
                                                           ProcessorVariable **,
                                                           gpointer,
                                                           gboolean);
GSList *            processor_utils_skip_steps            (GSList *,
                                                           RunStepType,
                                                           RunStepType);
void                processor_utils_sort_find_unused      (const FormatDefinition *,
                                                           ProcessorFile *);

/* Destroy functions */

void                selection_scope_destroy               (gpointer);
void                processor_variable_destroy            (gpointer);
void                description_tab_destroy               (gpointer);

G_END_DECLS
