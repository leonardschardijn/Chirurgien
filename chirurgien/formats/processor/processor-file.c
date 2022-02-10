/* processor-file.c
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

#include "processor-file.h"
#include "processor-file-private.h"


ProcessorFile *
processor_file_create (gconstpointer file_contents,
                       gsize         file_size,
                       GtkNotebook  *description,
                       GtkBox       *overview)
{
    ProcessorFile *processor_file;

    processor_file = g_slice_new0 (ProcessorFile);
    processor_file->file_contents = file_contents;
    processor_file->file_size = file_size;
    processor_file->description = description;
    processor_file->overview = overview;

    return processor_file;
}

GSList *
processor_file_get_field_list (ProcessorFile *processor_file)
{
    return processor_file->file_fields;
}

void
processor_file_destroy (ProcessorFile *processor_file)
{
    g_slice_free (ProcessorFile, processor_file);
}
