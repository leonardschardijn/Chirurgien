/* pe-section-table.c
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
pe_section_table (FormatsFile *file,
                  guint16      sections,
                  guint32      string_table_offset)
{
    DescriptionTab tab;

    g_autofree gchar *section_name;
    gchar section_name_buffer[9], *section_name_in_panel;
    guint64 section_name_offset;

    GString *multipurpose_string;

    guint32 characteristics;

    guint32 section_size, section_offset;

    gsize save_index;

    format_utils_init_tab (&tab, NULL);

    section_name_buffer[8] = '\0';
    for (guint16 i = 0; i < sections; i++)
    {
        /* Section index, starting at 1 */
        section_name = g_strdup_printf ("Section index %u", i + 1);
        format_utils_start_section_tab (&tab, section_name);
        g_free (g_steal_pointer(&section_name));

        /* Name */
        if (!format_utils_read(file, section_name_buffer, 8))
            return;

        /* If the name start with a / character, it holds an ASCII decimal number that
         * is an offset into the string table */
        if (section_name_buffer[0] == '/')
        {
            section_name_offset = g_ascii_strtoull (&section_name_buffer[1], NULL, 10);

            if (section_name_offset && (string_table_offset + section_name_offset < GET_FILE_SIZE (file)))
            {
                /* Use this variable, it is available */
                section_size = strnlen ((const gchar *) file->file_contents + string_table_offset + section_name_offset,
                                        GET_FILE_SIZE (file) - string_table_offset - section_name_offset);

                multipurpose_string = g_string_new (NULL);
                multipurpose_string = g_string_append_len (multipurpose_string,
                                                 (const gchar *) file->file_contents +
                                                  string_table_offset + section_name_offset, section_size);
                if (multipurpose_string->len > 15)
                {
                    multipurpose_string = g_string_truncate (multipurpose_string, 10);
                    multipurpose_string = g_string_append (multipurpose_string, " [...]");
                }
                else if (multipurpose_string->len)
                {
                    g_string_append_printf (multipurpose_string, " [%s]", section_name_buffer);
                }

                if (!multipurpose_string->len || !g_utf8_validate (multipurpose_string->str, -1, NULL))
                {
                    section_name = g_strdup_printf ("Section %d [%s]", i + 1, section_name_buffer);
                    section_name_in_panel = NULL;
                }
                else
                {
                    section_name = g_string_free (multipurpose_string, FALSE);
                    section_name_in_panel = section_name;
                }
            }
        }

        if (!section_name)
        {
            if (section_name_buffer[0] == '\0' || !g_utf8_validate (section_name_buffer, -1, NULL))
            {
                section_name = g_strdup_printf ("Section %d", i + 1);
                section_name_in_panel = NULL;
            }
            else
            {
                section_name = g_strdup (section_name_buffer);
                section_name_in_panel = section_name;
            }
        }

        format_utils_add_field (file, SECTION_NAME_COLOR, TRUE, 8,
                                "Section name", NULL);
        format_utils_add_line_tab (&tab, "Section name", section_name_in_panel, NULL);

        /* VirtualSize */
        if (!process_pe_field_simple (file, &tab,
                "VirtualSize",
                _("The total size of the section when loaded into memory"),
                 HEADER_DATA_COLOR_1, 4, "%u"))
            return;

        /* VirtualAddress */
        if (!process_pe_field_simple (file, &tab,
                "VirtualAddress",
                _("Address of the first byte of the section relative to the image base when the section is loaded into memory"),
                HEADER_DATA_COLOR_2, 4, NULL))
            return;

        /* SizeOfRawData */
        if (!process_pe_field (file, &tab,
                "SizeOfRawData", NULL,
                _("The size of the section on disk"),
                HEADER_DATA_COLOR_1, 4,
                0, NULL, NULL,
                "%u", &section_size))
            return;

        /* PointerToRawData */
        if (!process_pe_field (file, &tab,
                "PointerToRawData", NULL,
                _("The file pointer to the first page of the section within the COFF file"),
                HEADER_DATA_COLOR_2, 4,
                0, NULL, NULL,
                "%X<sub>16</sub>", &section_offset))
            return;

        /* PointerToRelocations */
        if (!process_pe_field_simple (file, &tab,
                "PointerToRelocations",
                _("The file pointer to the beginning of relocation entries for the section"),
                HEADER_DATA_COLOR_1, 4, "%X<sub>16</sub>"))
            return;

        /* PointerToLinenumbers */
        if (!process_pe_field_simple (file, &tab,
                "PointerToLinenumbers",
                _("The file pointer to the beginning of line-number entries for the section"),
                HEADER_DATA_COLOR_2, 4, "%X<sub>16</sub>"))
            return;

        /* NumberOfRelocations */
        if (!process_pe_field (file, &tab,
                "NumberOfRelocations", NULL,
                _("The number of relocation entries for the section"),
                HEADER_DATA_COLOR_1, 2,
                0, NULL, NULL,
                "%u", NULL))
            return;

        /* NumberOfLinenumbers */
        if (!process_pe_field_simple (file, &tab,
                "NumberOfLinenumbers",
                _("The number of line-number entries for the section"),
                HEADER_DATA_COLOR_2, 2, "%u"))
            return;

        /* Characteristics */
        if (!format_utils_read (file, &characteristics, 4))
            return;

        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4,
                                "Characteristics", NULL);

        multipurpose_string = g_string_new (NULL);
        if (characteristics & 0x8)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_TYPE_NO_PAD");
        if (characteristics & 0x20)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_CNT_CODE");
        if (characteristics & 0x40)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_CNT_INITIALIZED_DATA");
        if (characteristics & 0x80)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_CNT_UNINITIALIZED_DATA");
        if (characteristics & 0x200)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_LNK_INFO");
        if (characteristics & 0x800)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_LNK_REMOVE");
        if (characteristics & 0x1000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_LNK_COMDAT");
        if (characteristics & 0x8000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_GPREL");
        if (characteristics & 0x100000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_1BYTES");
        if (characteristics & 0x200000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_2BYTES");
        if ((characteristics & 0x300000) == 0x300000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_4BYTES");
        if ((characteristics & 0x400000) == 0x400000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_8BYTES");
        if ((characteristics & 0x500000) == 0x500000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_16BYTES");
        if ((characteristics & 0x600000) == 0x600000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_32BYTES");
        if ((characteristics & 0x700000) == 0x700000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_64BYTES");
        if ((characteristics & 0x800000) == 0x800000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_128BYTES");
        if ((characteristics & 0x900000) == 0x900000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_256BYTES");
        if ((characteristics & 0xA00000) == 0xA00000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_512BYTES");
        if ((characteristics & 0xB00000) == 0xB00000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_1024BYTES");
        if ((characteristics & 0xC00000) == 0xC00000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_2048BYTES");
        if ((characteristics & 0xD00000) == 0xD00000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_4096BYTES");
        if ((characteristics & 0xE00000) == 0xE00000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_ALIGN_8192BYTES");
        if (characteristics & 0x1000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_LNK_NRELOC_OVFL");
        if (characteristics & 0x2000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_DISCARDABLE");
        if (characteristics & 0x4000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_NOT_CACHED");
        if (characteristics & 0x8000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_NOT_PAGED");
        if (characteristics & 0x10000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_SHARED");
        if (characteristics & 0x20000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_EXECUTE");
        if (characteristics & 0x40000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_READ");
        if (characteristics & 0x80000000)
            g_string_append_printf (multipurpose_string, "%s\n", "IMAGE_SCN_MEM_WRITE");

        if (multipurpose_string->len)
        {
            g_string_truncate (multipurpose_string, multipurpose_string->len - 1);
            format_utils_add_line_tab (&tab, "Characteristics", multipurpose_string->str,
                   "Characteristic flags (bit mask)\n"
                   "IMAGE_SCN_TYPE_NO_PAD (<tt>8<sub>16</sub></tt>): The section should not be padded to the next boundary\n"
                   "IMAGE_SCN_CNT_CODE (<tt>20<sub>16</sub></tt>): The section contains executable code\n"
                   "IMAGE_SCN_CNT_INITIALIZED_DATA (<tt>40<sub>16</sub></tt>): The section contains initialized data\n"
                   "IMAGE_SCN_CNT_UNINITIALIZED_DATA (<tt>80<sub>16</sub></tt>): The section contains uninitialized data\n"
                   "IMAGE_SCN_LNK_INFO (<tt>200<sub>16</sub></tt>): The section contains comments or other information\n"
                   "IMAGE_SCN_LNK_REMOVE (<tt>800<sub>16</sub></tt>): The section will not become part of the image\n"
                   "IMAGE_SCN_LNK_COMDAT (<tt>1000<sub>16</sub></tt>): The section contains COMDAT data\n"
                   "IMAGE_SCN_GPREL (<tt>8000<sub>16</sub></tt>): The section contains data referenced through the global pointer (GP)\n"
                   "IMAGE_SCN_ALIGN_1BYTES (<tt>100000<sub>16</sub></tt>): Align data on a 1-byte boundary\n"
                   "IMAGE_SCN_ALIGN_2BYTES (<tt>200000<sub>16</sub></tt>): Align data on a 2-byte boundary\n"
                   "IMAGE_SCN_ALIGN_4BYTES (<tt>300000<sub>16</sub></tt>): Align data on a 4-byte boundary\n"
                   "IMAGE_SCN_ALIGN_8BYTES (<tt>400000<sub>16</sub></tt>): Align data on a 8-byte boundary\n"
                   "IMAGE_SCN_ALIGN_16BYTES (<tt>500000<sub>16</sub></tt>): Align data on a 16-byte boundary\n"
                   "IMAGE_SCN_ALIGN_32BYTES (<tt>600000<sub>16</sub></tt>): Align data on a 32-byte boundary\n"
                   "IMAGE_SCN_ALIGN_64BYTES (<tt>700000<sub>16</sub></tt>): Align data on a 64-byte boundary\n"
                   "IMAGE_SCN_ALIGN_128BYTES (<tt>800000<sub>16</sub></tt>): Align data on a 128-byte boundary\n"
                   "IMAGE_SCN_ALIGN_256BYTES (<tt>900000<sub>16</sub></tt>): Align data on a 256-byte boundary\n"
                   "IMAGE_SCN_ALIGN_512BYTES (<tt>A00000<sub>16</sub></tt>): Align data on a 512-byte boundary\n"
                   "IMAGE_SCN_ALIGN_1024BYTES (<tt>B00000<sub>16</sub></tt>): Align data on a 1024-byte boundary\n"
                   "IMAGE_SCN_ALIGN_2048BYTES (<tt>C00000<sub>16</sub></tt>): Align data on a 2048-byte boundary\n"
                   "IMAGE_SCN_ALIGN_4096BYTES (<tt>D00000<sub>16</sub></tt>): Align data on a 4096-byte boundary\n"
                   "IMAGE_SCN_ALIGN_8192BYTES (<tt>E00000<sub>16</sub></tt>): Align data on a 8192-byte boundary\n"
                   "IMAGE_SCN_LNK_NRELOC_OVFL (<tt>1000000<sub>16</sub></tt>): The section contains extended relocations\n"
                   "IMAGE_SCN_MEM_DISCARDABLE (<tt>2000000<sub>16</sub></tt>): The section can be discarded as needed\n"
                   "IMAGE_SCN_MEM_NOT_CACHED (<tt>4000000<sub>16</sub></tt>): The section cannot be cached\n"
                   "IMAGE_SCN_MEM_NOT_PAGED (<tt>8000000<sub>16</sub></tt>): The section is not pageable\n"
                   "IMAGE_SCN_MEM_SHARED (<tt>10000000<sub>16</sub></tt>): The section can be executed as code\n"
                   "IMAGE_SCN_MEM_EXECUTE (<tt>20000000<sub>16</sub></tt>): The section can be shared in memory\n"
                   "IMAGE_SCN_MEM_READ (<tt>40000000<sub>16</sub></tt>): The section can be read\n"
                   "IMAGE_SCN_MEM_WRITE (<tt>80000000<sub>16</sub></tt>): The section can be written to");
        }
        g_string_free (multipurpose_string, TRUE);

        if (section_offset && section_size)
        {
            save_index = GET_INDEX (file);

            SET_INDEX (file, section_offset);

            format_utils_add_field_full (file, HEADER_DATA_COLOR_1, TRUE, section_size,
                                         section_name, section_name, START_SECTION_DATA_COLOR);

            SET_INDEX (file, save_index);
        }
    }

    format_utils_insert_tab (file, &tab, "Section table");
}
