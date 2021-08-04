/* pe-data-directories.c
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

#include <config.h>

#include "pe-format.h"

#include <glib/gi18n.h>


void
pe_data_directories (FormatsFile *file,
                     guint32      data_directory_entries)
{
    DescriptionTab tab;

    const gchar *directory;
    GString *directory_field_string;
    g_autofree gchar *directory_field;

    guint32 directory_offset, directory_size;

    gsize save_index;

    const gchar * const data_directories[] = {
        "Export Table",
        "Import Table",
        "Resource Table",
        "Exception Table",
        "Certificate Table",
        "Base Relocation Table",
        "Debug",
        "Architecture",
        "Global Ptr",
        "TLS Table",
        "Load Config Table",
        "Bound Import",
        "Import Address Table",
        "Delay Import Descriptor",
        "CLR Runtime Header",
        "Reserved",
        "???"
    };

    format_utils_init_tab (&tab, NULL);

    for (guint16 i = 0; i < data_directory_entries; i++)
    {
        directory = i <= 16 ? data_directories[i] : data_directories[16];

        format_utils_start_section_tab (&tab, directory);

        directory_field_string = g_string_new (directory);
        directory_field_string = g_string_append_c (directory_field_string, ' ');
        directory_field_string = g_string_append (directory_field_string, "VirtualAddress");

        directory_field = g_string_free (directory_field_string, FALSE);

        /* VirtualAddress */
        if (!process_pe_field (file, &tab,
                "VirtualAddress", directory_field,
                _("Relative virtual address of the data directory"),
                DATA_DIRECTORY_COLOR, 4,
                0, NULL, NULL,
                "%X<sub>16</sub>", &directory_offset))
            return;

        g_free (directory_field);

        directory_field_string = g_string_new (directory);
        directory_field_string = g_string_append_c (directory_field_string, ' ');
        directory_field_string = g_string_append (directory_field_string, "Size");

        directory_field = g_string_free (directory_field_string, FALSE);

        /* Size */
        if (!process_pe_field (file, &tab,
                "Size", directory_field,
                _("Size in bytes"),
                HEADER_DATA_COLOR_1, 4,
                0, NULL, NULL,
                "%u", &directory_size))
            return;

        /* Only for the Certificate Table */
        if ((i == 4) && directory_offset && directory_size)
        {
            save_index = GET_INDEX (file);

            SET_INDEX (file, directory_offset);

            format_utils_add_field_full (file, HEADER_DATA_COLOR_1, TRUE, directory_size,
                                         directory, "Cert.", START_SECTION_DATA_COLOR);

            SET_INDEX (file, save_index);
        }
    }

    format_utils_insert_tab (file, &tab, "Data directories");
}
