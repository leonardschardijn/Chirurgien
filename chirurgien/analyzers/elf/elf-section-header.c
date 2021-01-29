/* elf-section-header.c
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


static gboolean     get_section_name          (AnalyzerFile *,
                                               AnalyzerTab *,
                                               gsize,
                                               gboolean,
                                               gchar **);

static gboolean     process_section_flags     (AnalyzerFile *,
                                               AnalyzerTab *,
                                               gboolean,
                                               gboolean);


void
analyze_section_header (AnalyzerFile *file,
                        gboolean is_64bits,
                        gboolean is_little_endian,
                        guint64 section_header_offset,
                        guint16 section_header_entries,
                        guint64 section_names_offset,
                        GSList **tagged_bytes)
{
    AnalyzerTab tab;
    gchar *description_message, *section_name;
    guint32 section_type;
    guint64 section_size = 0, section_offset = 0, string_table_pointer = 0;
    gsize save_pointer;

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Section header table</b>"));

    description_message = g_strdup_printf ("%u", section_header_entries);
    analyzer_utils_describe_tab (&tab, _("Number of entries"), description_message);
    g_free (description_message);

    if (is_64bits)
    {
        SET_POINTER (file, (gsize) (section_header_offset + section_names_offset + 24)); // 24 bytes offset to sh_offset
        analyzer_utils_read (&string_table_pointer, file, 8);

        if (!is_little_endian)
            string_table_pointer = GUINT64_FROM_BE (string_table_pointer);
    }
    else
    {
        SET_POINTER (file, (gsize) (section_header_offset + section_names_offset + 16)); // 16 bytes offset to sh_offset
        analyzer_utils_read (&string_table_pointer, file, 4);

        if (!is_little_endian)
            string_table_pointer = g_ntohl (string_table_pointer);
    }

    SET_POINTER (file, (gsize) section_header_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    for (guint i = 0; i < section_header_entries; i++)
    {
        description_message = g_strdup_printf (_("<b>Section index %u</b>"), i);
        analyzer_utils_add_description_tab (&tab, description_message, NULL, NULL, 15, 10);
        g_free (description_message);

        /* Section name (sh_name) */
        if (!get_section_name (file, &tab, string_table_pointer, is_little_endian, &section_name))
            goto END_SECTION_HEADER;

        /* Section type (sh_type) */
        guint section_type_values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
                                        0x9, 0xA, 0xB, 0xE, 0xF, 0x10, 0x11 };
        gchar *section_type_value_description[] = {
            _("Inactive section (SHT_NULL)"),
            _("Program information (SHT_PROGBITS)"),
            _("Symbol table (SHT_SYMTAB)"),
            _("String table (SHT_STRTAB)"),
            _("Explicit relocation (SHT_RELA)"),
            _("Hash table (SHT_HASH)"),
            _("Dynamic linking (SHT_DYNAMIC)"),
            _("Note (SHT_NOTE)"),
            _("Empty (SHT_NOBITS)"),
            _("Implicit relocation (SHT_REL)"),
            _("Unspecified semantics (SHT_SHLIB)"),
            _("Dynamic linking symbol table (SHT_DYNSYM)"),
            _("Initialization functions (SHT_INIT_ARRAY"),
            _("Termination functions (SHT_FINI_ARRAY)"),
            _("Pre-initialization functions (SHT_PREINIT_ARRAY)"),
            _("Section group (SHT_GROUP)"),
            _("<span foreground=\"red\">Unknown</span>")
        };
        if (!process_elf_field (file, &tab,
                _("Section type"), _("Section type (sh_type)"),
                _("Section type\n"
                  "<tt>00 00 00 00<sub>16</sub></tt>\tInactive section (SHT_NULL)\n"
                  "<tt>00 00 00 01<sub>16</sub></tt>\tProgram information (SHT_PROGBITS)\n"
                  "<tt>00 00 00 02<sub>16</sub></tt>\tSymbol table (SHT_SYMTAB)\n"
                  "<tt>00 00 00 03<sub>16</sub></tt>\tString table (SHT_STRTAB)\n"
                  "<tt>00 00 00 04<sub>16</sub></tt>\tExplicit relocation (SHT_RELA)\n"
                  "<tt>00 00 00 05<sub>16</sub></tt>\tHash table (SHT_HASH)\n"
                  "<tt>00 00 00 06<sub>16</sub></tt>\tDynamic linking (SHT_DYNAMIC)\n"
                  "<tt>00 00 00 07<sub>16</sub></tt>\tNote (SHT_NOTE)\n"
                  "<tt>00 00 00 08<sub>16</sub></tt>\tEmpty (SHT_NOBITS)\n"
                  "<tt>00 00 00 09<sub>16</sub></tt>\tImplicit relocation (SHT_REL)\n"
                  "<tt>00 00 00 0A<sub>16</sub></tt>\tUnspecified semantics (SHT_SHLIB)\n"
                  "<tt>00 00 00 0B<sub>16</sub></tt>\tDynamic linking symbol table (SHT_DYNSYM)\n"
                  "<tt>00 00 00 0E<sub>16</sub></tt>\tInitialization functions (SHT_INIT_ARRAY)\n"
                  "<tt>00 00 00 0F<sub>16</sub></tt>\tTermination functions (SHT_FINI_ARRAY)\n"
                  "<tt>00 00 00 10<sub>16</sub></tt>\tPre-initialization functions (SHT_PREINIT_ARRAY)\n"
                  "<tt>00 00 00 11<sub>16</sub></tt>\tSection group (SHT_GROUP)"),
                HEADER_DATA_COLOR_1,
                4, is_little_endian, 16, section_type_values, section_type_value_description, NULL, FALSE, &section_type))
            goto END_SECTION_HEADER;

        /* Section flags (sh_flags) */
        if (!process_section_flags (file, &tab, is_64bits, is_little_endian))
            goto END_SECTION_HEADER;

        if (is_64bits)
        {
            /* Section memory address (sh_addr) */
            if (!process_elf_field (file, &tab,
                    _("Section memory address"), _("Section memory address (sh_addr)"),
                    _("Address of the section in the memory image of the process"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, NULL))
                goto END_SECTION_HEADER;

            /* Section offset (sh_offset) */
            if (!process_elf_field (file, &tab,
                    _("Section offset"), _("Section offset (sh_offset)"),
                    _("Offset of the section in the file"),
                    HEADER_DATA_COLOR_2,
                    8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, &section_offset))
                goto END_SECTION_HEADER;

            /* Section size (sh_size) */
            if (!process_elf_field (file, &tab,
                    _("Section size"), _("Section size (sh_size)"),
                    _("Size of the section in the file"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, &section_size))
                goto END_SECTION_HEADER;
        }
        else
        {
            /* Section memory address (sh_addr) */
            if (!process_elf_field (file, &tab,
                    _("Section memory address"), _("Section memory address (sh_addr)"),
                    _("Address of the section in the memory image of the process"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, NULL))
                goto END_SECTION_HEADER;

            /* Section offset (sh_offset) */
            if (!process_elf_field (file, &tab,
                    _("Section offset"), _("Section offset (sh_offset)"),
                    _("Offset of the section in the file"),
                    HEADER_DATA_COLOR_2,
                    4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, &section_offset))
                goto END_SECTION_HEADER;

            /* Section size (sh_size) */
            if (!process_elf_field (file, &tab,
                    _("Section size"), _("Section size (sh_size)"),
                    _("Size of the section in the file"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, &section_size))
                goto END_SECTION_HEADER;
        }

        if (section_offset && section_size && (section_type != 0x8)) // 0x8 = Empty (SHT_EMPTY)
        {
            if (!section_name)
                section_name = _("Section data");

            save_pointer = GET_POINTER (file);

            SET_POINTER (file, (gsize) section_offset);
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

            analyzer_utils_tag (file, HEADER_DATA_COLOR_1, section_size, section_name);

            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER
                                           (GET_POINTER (file) + section_size));

            SET_POINTER (file, (gsize) section_offset);
            analyzer_utils_tag_navigation (file, START_SECTION_DATA, 1,
                                           section_name, section_name);

            SET_POINTER (file, save_pointer);
        }

        /* Section link index (sh_link) */
        if (!process_elf_field (file, &tab,
                _("Section link index"), _("Section link index (sh_link)"),
                _("Some section types link to other sections"),
                HEADER_DATA_COLOR_2,
                4, is_little_endian, 0, NULL, NULL, _("Section index %u"), TRUE, NULL))
            goto END_SECTION_HEADER;

        /* Section information (sh_info) */
        if (!FILE_HAS_DATA_N (file, 4))
            goto END_SECTION_HEADER;

        ADVANCE_POINTER (file, 4);
        analyzer_utils_tag (file, HEADER_DATA_COLOR_1, 4, _("Section information (sh_info)"));

        if (is_64bits)
        {
            /* Section address alignment (sh_addralign) */
            if (!process_elf_field (file, &tab,
                    _("Section address alignment"), _("Section address alignment (sh_addralign)"),
                    _("Some sections have address alignment constraints"),
                    HEADER_DATA_COLOR_2,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, NULL))
                goto END_SECTION_HEADER;

            /* Table entry size (sh_entsize) */
            if (!process_elf_field (file, &tab,
                    _("Table entry size"), _("Table entry size (sh_entsize)"),
                    _("Some sections hold a table of fixed-size entries, this gives the size of the entries"),
                    HEADER_DATA_COLOR_1,
                    8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, &section_offset))
                goto END_SECTION_HEADER;
        }
        else
        {
            /* Section address alignment (sh_addralign) */
            if (!process_elf_field (file, &tab,
                    _("Section address alignment"), _("Section address alignment (sh_addralign)"),
                    _("Some sections have address alignment constraints"),
                    HEADER_DATA_COLOR_2,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, NULL))
                goto END_SECTION_HEADER;

            /* Table entry size (sh_entsize) */
            if (!process_elf_field (file, &tab,
                    _("Table entry size"), _("Table entry size (sh_entsize)"),
                    _("Some sections hold a table of fixed-size entries, this gives the size of the entries"),
                    HEADER_DATA_COLOR_1,
                    4, is_little_endian, 0, NULL, NULL, "%u", TRUE, &section_offset))
                goto END_SECTION_HEADER;
        }
    }
    END_SECTION_HEADER:
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyzer_utils_insert_tab (file, &tab, _("Section header"));
}

static gboolean
get_section_name (AnalyzerFile *file,
                  AnalyzerTab *tab,
                  gsize string_table_pointer,
                  gboolean is_little_endian,
                  gchar **section_name)
{
    guint32 four_bytes;

    gsize section_name_offset;

    if (!analyzer_utils_read (&four_bytes, file, 4))
        return FALSE;

    if (!is_little_endian)
        four_bytes = g_ntohl (four_bytes);

    analyzer_utils_tag (file, SECTION_NAME_COLOR, 4, _("Section name offset (sh_name)"));

    *section_name = NULL;

    section_name_offset = string_table_pointer + four_bytes;

    if (string_table_pointer && section_name_offset < GET_FILE_SIZE (file))
    {
        *section_name = (gchar *) file->file_contents + section_name_offset;
        four_bytes = strnlen (*section_name, GET_FILE_SIZE (file) - section_name_offset);

        if (g_utf8_validate (*section_name, four_bytes, NULL))
            analyzer_utils_describe_tab (tab, _("Section name"), *section_name);
        else
            *section_name = NULL;
    }

    if (!*section_name)
        analyzer_utils_describe_tab (tab, _("Section name"), "");

    return TRUE;
}


static gboolean
process_section_flags (AnalyzerFile *file,
                       AnalyzerTab *tab,
                       gboolean is_64bits,
                       gboolean is_little_endian)
{
    GString *description_string;
    guint64 section_flags = 0;

    if (is_64bits)
    {
        if (!analyzer_utils_read (&section_flags, file, 8))
            return FALSE;

        if (!is_little_endian)
            section_flags = GUINT64_FROM_BE (section_flags);

        analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 8, _("Section attributes (sh_flags)"));
    }
    else
    {
        if (!analyzer_utils_read (&section_flags, file, 4))
            return FALSE;

        if (!is_little_endian)
            section_flags = g_ntohl (section_flags);

        analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 4, _("Section attributes (sh_flags)"));
    }

    description_string = g_string_new (NULL);
    if (section_flags & 0x1)
        g_string_append_printf (description_string, "%s\n", _("Has writable data"));
    if (section_flags & 0x2)
        g_string_append_printf (description_string, "%s\n", _("Uses memory during execution"));
    if (section_flags & 0x4)
        g_string_append_printf (description_string, "%s\n", _("Has executable machine instructions"));
    if (section_flags & 0x10)
        g_string_append_printf (description_string, "%s\n", _("May be merged to eliminate duplicates"));
    if (section_flags & 0x20)
        g_string_append_printf (description_string, "%s\n", _("Has null-terminated strings"));
    if (section_flags & 0x40)
        g_string_append_printf (description_string, "%s\n", _("Holds a section header table index (sh_info is a section index)"));
    if (section_flags & 0x80)
        g_string_append_printf (description_string, "%s\n", _("Has ordering requirements"));
    if (section_flags & 0x100)
        g_string_append_printf (description_string, "%s\n", _("Requires OS-specific processing"));
    if (section_flags & 0x200)
        g_string_append_printf (description_string, "%s\n", _("Is a member of a section group"));
    if (section_flags & 0x400)
        g_string_append_printf (description_string, "%s\n", _("Holds Thread-Local Storage"));
    if (section_flags & 0x800)
        g_string_append_printf (description_string, "%s\n", _("Has compressed data"));

    if (description_string->len)
    {
        g_string_truncate (description_string, description_string->len - 1);
        analyzer_utils_describe_tooltip_tab (tab, _("Section attributes"), description_string->str,
             _("Section attributes (relevant bit masks)\n"
             "<tt>1<sub>16</sub></tt>    Has writable data\n"
             "<tt>2<sub>16</sub></tt>    Uses memory during execution\n"
             "<tt>4<sub>16</sub></tt>    Has executable machine instructions\n"
             "<tt>10<sub>16</sub></tt>    May be merged to eliminate duplicates\n"
             "<tt>20<sub>16</sub></tt>    Has null-terminated strings\n"
             "<tt>40<sub>16</sub></tt>    Holds a section header table index (sh_info is a section index)\n"
             "<tt>80<sub>16</sub></tt>    Has ordering requirements\n"
             "<tt>100<sub>16</sub></tt>    Requires OS-specific processing\n"
             "<tt>200<sub>16</sub></tt>    Is a member of a section group\n"
             "<tt>400<sub>16</sub></tt>    Holds Thread-Local Storage\n"
             "<tt>800<sub>16</sub></tt>    Has compressed data"));
    }
    g_string_free (description_string, TRUE);

    return TRUE;
}
