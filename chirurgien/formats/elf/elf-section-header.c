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

#include "elf-format.h"

#include <glib/gi18n.h>


static gboolean     get_section_name          (FormatsFile *,
                                               DescriptionTab *,
                                               gsize,
                                               gboolean,
                                               const gchar **);
static gboolean     process_section_flags     (FormatsFile *,
                                               DescriptionTab *,
                                               gboolean,
                                               gboolean);


void
elf_section_header (FormatsFile *file,
                    gboolean     is_64bits,
                    gboolean     is_little_endian,
                    guint64      section_header_offset,
                    guint16      section_header_entries,
                    guint64      section_names_offset)
{
    DescriptionTab tab;

    const gchar *section_name;

    gchar *value;
    guint32 section_type;
    guint64 section_size = 0, section_offset = 0, string_table_pointer = 0;

    gsize save_index;

    format_utils_init_tab (&tab, _("Section header table"));

    value = g_strdup_printf ("%u", section_header_entries);
    format_utils_add_line_tab (&tab, _("Number of entries"), value, NULL);
    g_free (value);

    if (is_64bits)
    {
        /* 24 bytes offset to sh_offset */
        SET_INDEX (file, section_header_offset + section_names_offset + 24);
        format_utils_read (file, &string_table_pointer, 8);

        if (!is_little_endian)
            string_table_pointer = GUINT64_FROM_BE (string_table_pointer);
    }
    else
    {
        /* 16 bytes offset to sh_offset */
        SET_INDEX (file, section_header_offset + section_names_offset + 16);
        format_utils_read (file, &string_table_pointer, 4);

        if (!is_little_endian)
            string_table_pointer = g_ntohl (string_table_pointer);
    }

    SET_INDEX (file, section_header_offset);

    for (guint i = 0; i < section_header_entries; i++)
    {
        value = g_strdup_printf (_("Section index %u"), i);
        format_utils_start_section_tab (&tab, value);
        g_free (value);

        /* Section name (sh_name) */
        if (!get_section_name (file, &tab, string_table_pointer, is_little_endian, &section_name))
            break;

        /* Section type (sh_type) */
        guint32 section_type_values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
                                          0x9, 0xA, 0xB, 0xE, 0xF, 0x10, 0x11 };
        const gchar *section_type_value_description[] = {
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
                HEADER_DATA_COLOR_1, 4, is_little_endian, G_N_ELEMENTS (section_type_values),
                section_type_values, section_type_value_description, NULL, FALSE, &section_type))
            break;

        /* Section flags (sh_flags) */
        if (!process_section_flags (file, &tab, is_64bits, is_little_endian))
            break;

        if (is_64bits)
        {
            /* Section memory address (sh_addr) */
            if (!process_elf_field (file, &tab,
                    _("Section memory address"), _("Section memory address (sh_addr)"),
                    _("Address of the section in the memory image of the process"),
                    HEADER_DATA_COLOR_1, 8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, NULL))
                break;

            /* Section offset (sh_offset) */
            if (!process_elf_field (file, &tab,
                    _("Section offset"), _("Section offset (sh_offset)"),
                    _("Offset of the section in the file"),
                    HEADER_DATA_COLOR_2, 8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", TRUE, &section_offset))
                break;

            /* Section size (sh_size) */
            if (!process_elf_field (file, &tab,
                    _("Section size"), _("Section size (sh_size)"),
                    _("Size of the section in the file"),
                    HEADER_DATA_COLOR_1, 8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, &section_size))
                break;
        }
        else
        {
            /* Section memory address (sh_addr) */
            if (!process_elf_field (file, &tab,
                    _("Section memory address"), _("Section memory address (sh_addr)"),
                    _("Address of the section in the memory image of the process"),
                    HEADER_DATA_COLOR_1, 4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, NULL))
                break;

            /* Section offset (sh_offset) */
            if (!process_elf_field (file, &tab,
                    _("Section offset"), _("Section offset (sh_offset)"),
                    _("Offset of the section in the file"),
                    HEADER_DATA_COLOR_2, 4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", TRUE, &section_offset))
                break;

            /* Section size (sh_size) */
            if (!process_elf_field (file, &tab,
                    _("Section size"), _("Section size (sh_size)"),
                    _("Size of the section in the file"),
                    HEADER_DATA_COLOR_1, 4, is_little_endian, 0, NULL, NULL, "%u", TRUE, &section_size))
                break;
        }

        if (section_offset && section_size && (section_type != 0x8)) /* 0x8 = Empty (SHT_EMPTY) */
        {
            if (!section_name)
                section_name = _("Section data");

            save_index = GET_INDEX (file);

            SET_INDEX (file, section_offset);

            format_utils_add_field_full (file, HEADER_DATA_COLOR_1, TRUE, section_size,
                                         section_name, section_name, START_SECTION_DATA_COLOR);

            SET_INDEX (file, save_index);
        }

        /* Section link index (sh_link) */
        if (!process_elf_field (file, &tab,
                _("Section link index"), _("Section link index (sh_link)"),
                _("Some section types link to other sections"),
                HEADER_DATA_COLOR_2, 4, is_little_endian, 0, NULL, NULL, _("Section index %u"), TRUE, NULL))
            break;

        /* Section information (sh_info) */
        if (!FILE_HAS_DATA_N (file, 4))
            break;

        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4, _("Section information (sh_info)"), NULL);

        if (is_64bits)
        {
            /* Section address alignment (sh_addralign) */
            if (!process_elf_field (file, &tab,
                    _("Section address alignment"), _("Section address alignment (sh_addralign)"),
                    _("Some sections have address alignment constraints"),
                    HEADER_DATA_COLOR_2, 8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, NULL))
                break;

            /* Table entry size (sh_entsize) */
            if (!process_elf_field (file, &tab,
                    _("Table entry size"), _("Table entry size (sh_entsize)"),
                    _("Some sections hold a table of fixed-size entries, this gives the size of the entries"),
                    HEADER_DATA_COLOR_1, 8, is_little_endian, 0, NULL, NULL, "%lu", TRUE, &section_offset))
                break;
        }
        else
        {
            /* Section address alignment (sh_addralign) */
            if (!process_elf_field (file, &tab,
                    _("Section address alignment"), _("Section address alignment (sh_addralign)"),
                    _("Some sections have address alignment constraints"),
                    HEADER_DATA_COLOR_2, 4, is_little_endian, 0, NULL, NULL, "%u", TRUE, NULL))
                break;

            /* Table entry size (sh_entsize) */
            if (!process_elf_field (file, &tab,
                    _("Table entry size"), _("Table entry size (sh_entsize)"),
                    _("Some sections hold a table of fixed-size entries, this gives the size of the entries"),
                    HEADER_DATA_COLOR_1, 4, is_little_endian, 0, NULL, NULL, "%u", TRUE, &section_offset))
                break;
        }
    }

    format_utils_insert_tab (file, &tab, _("Section header"));
}

static gboolean
get_section_name (FormatsFile    *file,
                  DescriptionTab *tab,
                  gsize           string_table_pointer,
                  gboolean        is_little_endian,
                  const gchar   **section_name)
{
    guint32 four_bytes;

    gsize section_name_offset;

    if (!format_utils_read (file, &four_bytes, 4))
        return FALSE;

    if (!is_little_endian)
        four_bytes = g_ntohl (four_bytes);

    format_utils_add_field (file, SECTION_NAME_COLOR, TRUE, 4,
                          _("Section name offset (sh_name)"), NULL);

    *section_name = NULL;

    section_name_offset = string_table_pointer + four_bytes;

    if (string_table_pointer && section_name_offset < GET_FILE_SIZE (file))
    {
        *section_name = (const gchar *) file->file_contents + section_name_offset;
        four_bytes = strnlen (*section_name, GET_FILE_SIZE (file) - section_name_offset);

        format_utils_add_line_tab (tab, _("Section name"), *section_name, NULL);
    }

    return TRUE;
}


static gboolean
process_section_flags (FormatsFile    *file,
                       DescriptionTab *tab,
                       gboolean        is_64bits,
                       gboolean        is_little_endian)
{
    GString *description_string;
    guint64 section_flags = 0;

    if (is_64bits)
    {
        if (!format_utils_read (file, &section_flags, 8))
            return FALSE;

        if (!is_little_endian)
            section_flags = GUINT64_FROM_BE (section_flags);

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 8,
                              _("Section attributes (sh_flags)"), NULL);
    }
    else
    {
        if (!format_utils_read (file, &section_flags, 4))
            return FALSE;

        if (!is_little_endian)
            section_flags = g_ntohl (section_flags);

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 4,
                              _("Section attributes (sh_flags)"), NULL);
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
        format_utils_add_line_tab (tab, _("Section attributes"), description_string->str,
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
