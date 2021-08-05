/* elf-program-header.c
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

#include "elf-format.h"


static gboolean process_segment_flags (FormatsFile *,
                                       DescriptionTab *,
                                       gboolean,
                                       gint);


void
elf_program_header (FormatsFile *file,
                    gboolean     is_64bits,
                    gboolean     is_little_endian,
                    guint64      program_header_offset,
                    guint16      program_header_entries)
{
    DescriptionTab tab;
    gchar *text_value;

    format_utils_init_tab (&tab, "Program header table");

    text_value = g_strdup_printf ("%u", program_header_entries);
    format_utils_add_line_tab (&tab, "Number of entries", text_value, NULL);
    g_free (text_value);

    SET_INDEX (file, program_header_offset);

    for (guint i = 0; i < program_header_entries; i++)
    {
        text_value = g_strdup_printf ("Program header index %u", i);
        format_utils_start_section_tab (&tab, text_value);
        g_free (text_value);

        /* Segment type (p_type) */
        guint32 segment_type_values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
        const gchar *segment_type_value_description[] = {
            "PT_NULL (Unused segment)",
            "PT_LOAD (Loadable segment)",
            "PT_DYNAMIC (Dynamic linking information)",
            "PT_INTERP (Interpreter location)",
            "PT_NOTE (Auxiliary information)",
            "PT_SHLIB (Unspecified semantics)",
            "PT_PHDR (Program header table)",
            "PT_TLS (Thread-Local Storage template)",
            "<span foreground=\"red\">Unknown</span>"
        };
        if (!process_elf_field (file, &tab,
                  "Segment type", "Segment type (p_type)",
                  "Segment type\n"
                  "<tt>00 00 00 00<sub>16</sub></tt>\tPT_NULL (Unused segment)\n"
                  "<tt>00 00 00 01<sub>16</sub></tt>\tPT_LOAD (Loadable segment)\n"
                  "<tt>00 00 00 02<sub>16</sub></tt>\tPT_DYNAMIC (Dynamic linking information)\n"
                  "<tt>00 00 00 03<sub>16</sub></tt>\tPT_INTERP (Interpreter location)\n"
                  "<tt>00 00 00 04<sub>16</sub></tt>\tPT_NOTE (Auxiliary information)\n"
                  "<tt>00 00 00 05<sub>16</sub></tt>\tPT_SHLIB (Unspecified semantics)\n"
                  "<tt>00 00 00 06<sub>16</sub></tt>\tPT_PHDR (Program header table)\n"
                  "<tt>00 00 00 07<sub>16</sub></tt>\tPT_TLS (Thread-Local Storage template)",
                SEGMENT_TYPE_COLOR, 4, is_little_endian,
                G_N_ELEMENTS (segment_type_values), segment_type_values, segment_type_value_description,
                NULL, FALSE, NULL))
            break;

        if (is_64bits)
        {
            /* Segment flags (p_flags) */
            if (!process_segment_flags (file, &tab, is_little_endian, HEADER_DATA_COLOR_1))
                break;

            /* Segment offset (p_offset) */
            if (!process_elf_field (file, &tab,
                    "Segment offset", "Segment offset (p_offset)",
                    "File offset of the segment",
                    HEADER_DATA_COLOR_2, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lX<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment virtual address (p_vaddr) */
            if (!process_elf_field (file, &tab,
                    "Segment virtual address", "Segment virtual address (p_vaddr)",
                    "Virtual address of the segment in memory",
                    HEADER_DATA_COLOR_1, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lX<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment physical address (p_paddr) */
            if (!process_elf_field (file, &tab,
                    "Segment physical address", "Segment physical address (p_paddr)",
                    "Physical address of the segment in memory",
                    HEADER_DATA_COLOR_2, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lX<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment size in file (p_filesz) */
            if (!process_elf_field (file, &tab,
                    "Segment size (file)", "File segment size (p_filesz)",
                    "Size of the segment in the file, in bytes",
                    HEADER_DATA_COLOR_1, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lu", TRUE, NULL))
                break;

            /* Segment size in memory (p_memsz) */
            if (!process_elf_field (file, &tab,
                    "Segment size (memory)", "Memory segment size (p_memsz)",
                    "Size of the segment in memory, in bytes",
                    HEADER_DATA_COLOR_2, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lu", TRUE, NULL))
                break;

            /* Segment address alignment (p_align) */
            if (!process_elf_field (file, &tab,
                    "Segment address alignment", "Segment address alignment (p_align)",
                    "Loadable segments have address alignment constraints",
                    HEADER_DATA_COLOR_1, 8, is_little_endian,
                    0, NULL, NULL,
                    "%lu", TRUE, NULL))
                break;
        }
        else
        {
            /* Segment offset (p_offset) */
            if (!process_elf_field (file, &tab,
                    "Segment offset ", "Segment offset (p_offset)",
                    "File offset of the segment",
                    HEADER_DATA_COLOR_1, 4, is_little_endian,
                    0, NULL, NULL,
                    "%X<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment virtual address (p_vaddr) */
            if (!process_elf_field (file, &tab,
                    "Segment virtual address", "Segment virtual address (p_vaddr)",
                    "Virtual address of the segment in memory",
                    HEADER_DATA_COLOR_2, 4, is_little_endian,
                    0, NULL, NULL,
                    "%X<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment physical address (p_paddr) */
            if (!process_elf_field (file, &tab,
                    "Segment physical address", "Segment physical address (p_paddr)",
                    "Physical address of the segment in memory",
                    HEADER_DATA_COLOR_1, 4, is_little_endian,
                    0, NULL, NULL,
                    "%X<sub>16</sub>", TRUE, NULL))
                break;

            /* Segment size in file (p_filesz) */
            if (!process_elf_field (file, &tab,
                    "Segment size (file)", "File segment size (p_filesz)",
                    "Size of the segment in the file",
                    HEADER_DATA_COLOR_2, 4, is_little_endian,
                    0, NULL, NULL,
                    "%u", TRUE, NULL))
                break;

            /* Segment size in memory (p_memsz) */
            if (!process_elf_field (file, &tab,
                    "Segment size (memory)", "Memory segment size (p_memsz)",
                    "Size of the segment in memory",
                    HEADER_DATA_COLOR_1, 4, is_little_endian,
                    0, NULL, NULL,
                    "%u", TRUE, NULL))
                break;

            /* Segment flags (p_flags) */
            if (!process_segment_flags (file, &tab, is_little_endian, HEADER_DATA_COLOR_2))
                break;

            /* Segment address alignment (p_align) */
            if (!process_elf_field (file, &tab,
                    "Segment address alignment", "Segment address alignment (p_align)",
                    "Loadable segments have address alignment constraints",
                    HEADER_DATA_COLOR_1, 4, is_little_endian,
                    0, NULL, NULL,
                    "%u", TRUE, NULL))
                break;
        }
    }

    format_utils_insert_tab (file, &tab, "Program header");
}

static gboolean
process_segment_flags (FormatsFile    *file,
                       DescriptionTab *tab,
                       gboolean        is_little_endian,
                       gint            color_index)
{
    GString *string_value;
    guint32 segment_flags;

    if (!format_utils_read (file, &segment_flags, 4))
        return FALSE;

    if (!is_little_endian)
        segment_flags = g_ntohl (segment_flags);

    format_utils_add_field (file, color_index, TRUE, 4, "Segment flags (p_flags)", NULL);

    string_value = g_string_new (NULL);
    if (segment_flags & 0x1)
        g_string_append_printf (string_value, "%s\n", "PF_X");
    if (segment_flags & 0x2)
        g_string_append_printf (string_value, "%s\n", "PF_W");
    if (segment_flags & 0x4)
        g_string_append_printf (string_value, "%s\n", "PF_R");

    if (string_value->len)
    {
        g_string_truncate (string_value, string_value->len - 1);
        format_utils_add_line_tab (tab, "Segment flags", string_value->str,
                                   "Segment flags (bit masks)\n"
                                   "PF_X (<tt>1<sub>16</sub></tt>): Execute\n"
                                   "PF_W (<tt>2<sub>16</sub></tt>): Write\n"
                                   "PF_R (<tt>4<sub>16</sub></tt>): Read");
    }
    g_string_free (string_value, TRUE);

    return TRUE;
}
