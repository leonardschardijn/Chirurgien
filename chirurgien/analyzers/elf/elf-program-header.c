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

#include <config.h>

#include "elf-analyzer.h"

#include <glib/gi18n.h>


static gboolean     process_segment_flags     (AnalyzerFile *,
                                               AnalyzerTab *,
                                               gboolean,
                                               GdkRGBA *);


void
analyze_program_header (AnalyzerFile *file,
                        gboolean is_64bits,
                        gboolean is_little_endian,
                        guint64 program_header_offset,
                        guint16 program_header_entries,
                        GSList **tagged_bytes)
{
    AnalyzerTab tab;
    gchar *description_message;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Program header table</b>"));

    description_message = g_strdup_printf ("%u", program_header_entries);
    analyzer_utils_describe_tab (&tab, _("Number of entries"), description_message);
    g_free (description_message);

    SET_POINTER (file, (gsize) program_header_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < program_header_entries; i++)
    {
        description_message = g_strdup_printf (_("<b>Program header index %u</b>"), i);
        analyzer_utils_add_description_tab (&tab, description_message, NULL, NULL, 15, 10);
        g_free (description_message);

        /* Segment type (p_type) */
        guint32 segment_type_values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
        gchar *segment_type_value_description[] = {
            _("Unused segment (PT_NULL)"),
            _("Loadable segment (PT_LOAD)"),
            _("Dynamic linking information (PT_DYNAMIC)"),
            _("Interpreter location (PT_INTERP)"),
            _("Auxiliary information (PT_NOTE)"),
            _("Unspecified semantics (PT_SHLIB)"),
            _("Program header table (PT_PHDR)"),
            _("Thread-Local Storage template (PT_TLS)"),
            "<span foreground=\"red\">Unknown</span>"
        };
        if (!process_elf_field (file, &tab,
                _("Segment type"), _("Segment type (p_type)"),
                _("Segment type\n"
                 "<tt>00 00 00 00<sub>16</sub></tt>\tUnused segment (PT_NULL)\n"
                 "<tt>00 00 00 01<sub>16</sub></tt>\tLoadable segment (PT_LOAD)\n"
                 "<tt>00 00 00 02<sub>16</sub></tt>\tDynamic linking information (PT_DYNAMIC)\n"
                 "<tt>00 00 00 03<sub>16</sub></tt>\tInterpreter location (PT_INTERP)\n"
                 "<tt>00 00 00 04<sub>16</sub></tt>\tAuxiliary information (PT_NOTE)\n"
                 "<tt>00 00 00 05<sub>16</sub></tt>\tUnspecified semantics (PT_SHLIB)\n"
                 "<tt>00 00 00 06<sub>16</sub></tt>\tProgram header table (PT_PHDR)\n"
                 "<tt>00 00 00 07<sub>16</sub></tt>\tThread-Local Storage template (PT_TLS)"),
                SEGMENT_TYPE_COLOR,
                4, is_little_endian, sizeof (segment_type_values) >> 2, segment_type_values, segment_type_value_description, NULL, FALSE, NULL))
            goto END_PROGRAM_HEADER;

        if (is_64bits)
        {
            /* Segment flags (p_flags) */
            if (!process_segment_flags (file, &tab, is_little_endian, HEADER_DATA_COLOR_1))
                goto END_PROGRAM_HEADER;

            /* Segment offset (p_offset) */
            if (!process_elf_field (file, &tab,
                    _("Segment offset"), _("Segment offset (p_offset)"),
                    _("File offset of the segment"),
                    HEADER_DATA_COLOR_2,
                    8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment virtual address (p_vaddr) */
            if (!process_elf_field (file, &tab,
                    _("Segment virtual address"), _("Segment virtual address (p_vaddr)"),
                    _("Virtual address of the segment in memory"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment physical address (p_paddr) */
            if (!process_elf_field (file, &tab,
                    _("Segment physical address"), _("Segment physical address (p_paddr)"),
                    _("Physical address of the segment in memory"),
                    HEADER_DATA_COLOR_2,
                    8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment size in file (p_filesz) */
            if (!process_elf_field (file, &tab,
                    _("Segment size (file)"), _("File segment size (p_filesz)"),
                    _("Size of the segment in the file, in bytes"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment size in memory (p_memsz) */
            if (!process_elf_field (file, &tab,
                    _("Segment size (memory)"), _("Memory segment size (p_memsz)"),
                    _("Size of the segment in memory, in bytes"),
                    HEADER_DATA_COLOR_2,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* "Segment address alignment (p_align)" */
            if (!process_elf_field (file, &tab,
                    _("Segment address alignment"), _("Segment address alignment (p_align)"),
                    _("Loadable segments have address alignment constraints"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, NULL))
                goto END_PROGRAM_HEADER;
        }
        else
        {
            /* Segment offset (p_offset) */
            if (!process_elf_field (file, &tab,
                    _("Segment offset "), _("Segment offset (p_offset)"),
                    _("File offset of the segment"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment virtual address (p_vaddr) */
            if (!process_elf_field (file, &tab,
                    _("Segment virtual address"), _("Segment virtual address (p_vaddr)"),
                    _("Virtual address of the segment in memory"),
                    HEADER_DATA_COLOR_2,
                    4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment physical address (p_paddr) */
            if (!process_elf_field (file, &tab,
                    _("Segment physical address"), _("Segment physical address (p_paddr)"),
                    _("Physical address of the segment in memory"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment size in file (p_filesz) */
            if (!process_elf_field (file, &tab,
                    _("Segment size (file)"), _("File segment size (p_filesz)"),
                    _("Size of the segment in the file"),
                    HEADER_DATA_COLOR_2,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment size in memory (p_memsz) */
            if (!process_elf_field (file, &tab,
                    _("Segment size (memory)"), _("Memory segment size (p_memsz)"),
                    _("Size of the segment in memory"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, NULL))
                goto END_PROGRAM_HEADER;

            /* Segment flags (p_flags) */
            if (!process_segment_flags (file, &tab, is_little_endian, HEADER_DATA_COLOR_2))
                goto END_PROGRAM_HEADER;

            /* "Segment address alignment (p_align)" */
            if (!process_elf_field (file, &tab,
                    _("Segment address alignment"), _("Segment address alignment (p_align)"),
                    _("Loadable segments have address alignment constraints"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, NULL))
                goto END_PROGRAM_HEADER;
        }
    }
    END_PROGRAM_HEADER:
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_insert_tab (file, &tab, _("Program header"));
}

static gboolean
process_segment_flags (AnalyzerFile *file,
                       AnalyzerTab *tab,
                       gboolean is_little_endian,
                       GdkRGBA *color)
{
    GString *description_string;
    guint32 segment_flags;

    if (!analyzer_utils_read (&segment_flags, file, 4))
        return FALSE;

    if (!is_little_endian)
        segment_flags = g_ntohl (segment_flags);

    analyzer_utils_tag (file, color, 4, _("Segment flags (p_flags)"));

    description_string = g_string_new (NULL);
    if (segment_flags & 0x1)
        g_string_append_printf (description_string, "%s\n", _("Execute"));
    if (segment_flags & 0x2)
        g_string_append_printf (description_string, "%s\n", _("Write"));
    if (segment_flags & 0x4)
        g_string_append_printf (description_string, "%s\n", _("Read"));

    if (description_string->len)
    {
        g_string_truncate (description_string, description_string->len - 1);
        analyzer_utils_describe_tooltip_tab (tab, _("Segment flags"), description_string->str,
                                             _("Segment flags (relevant bit masks)\n"
                                               "<tt>1<sub>16</sub></tt>\tExecute\n"
                                               "<tt>2<sub>16</sub></tt>\tWrite\n"
                                               "<tt>4<sub>16</sub></tt>\tRead"));
    }
    g_string_free (description_string, TRUE);

    return TRUE;
}
