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
                                               gchar **);
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

    g_autofree gchar *section_name = NULL;

    gchar *text_value;
    guint32 section_type;
    guint64 section_size = 0, section_offset = 0, string_table_pointer = 0;

    guint field_length;

    gsize save_index;

    format_utils_init_tab (&tab, "Section header table");

    text_value = g_strdup_printf ("%u", section_header_entries);
    format_utils_add_line_tab (&tab, _("Number of entries"), text_value, NULL);
    g_free (text_value);

    if (is_64bits)
    {
        /* 24 bytes offset to sh_offset */
        SET_INDEX (file, section_header_offset + section_names_offset + 24);
        format_utils_read (file, &string_table_pointer, 8);

        if (!is_little_endian)
            string_table_pointer = GUINT64_FROM_BE (string_table_pointer);

        field_length = 8;
    }
    else
    {
        /* 16 bytes offset to sh_offset */
        SET_INDEX (file, section_header_offset + section_names_offset + 16);
        format_utils_read (file, &string_table_pointer, 4);

        if (!is_little_endian)
            string_table_pointer = g_ntohl (string_table_pointer);

        field_length = 4;
    }

    SET_INDEX (file, section_header_offset);

    for (guint i = 0; i < section_header_entries; i++)
    {
        text_value = g_strdup_printf ("Section index %u", i);
        format_utils_start_section_tab (&tab, text_value);
        g_free (text_value);

        /* Section name (sh_name) */
        if (!get_section_name (file, &tab, string_table_pointer, is_little_endian, &section_name))
            break;

        /* Section type (sh_type) */
        guint32 section_type_values[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
                                          0x9, 0xA, 0xB, 0xE, 0xF, 0x10, 0x11 };
        const gchar *section_type_value_description[] = {
            "SHT_NULL (Inactive section)",
            "SHT_PROGBITS (Program information)",
            "SHT_SYMTAB (Symbol table)",
            "SHT_STRTAB (String table)",
            "SHT_RELA (Explicit relocation)",
            "SHT_HASH (Hash table)",
            "SHT_DYNAMIC (Dynamic linking)",
            "SHT_NOTE (Note)",
            "SHT_NOBITS (Empty)",
            "SHT_REL (Implicit relocation)",
            "SHT_SHLIB (Unspecified semantics)",
            "SHT_DYNSYM (Dynamic linking symbol table)",
            "SHT_INIT_ARRAY (Initialization functions)",
            "SHT_FINI_ARRAY (Termination functions)",
            "SHT_PREINIT_ARRAY (Pre-initialization functions)",
            "SHT_GROUP (Section group)",
            "<span foreground=\"red\">Unknown</span>"
        };
        if (!process_elf_field (file, &tab,
                  "Section type", "Section type (sh_type)",
                  "Section type\n"
                  "<tt>00 00 00 00<sub>16</sub></tt>\tSHT_NULL (Inactive section)\n"
                  "<tt>00 00 00 01<sub>16</sub></tt>\tSHT_PROGBITS (Program information)\n"
                  "<tt>00 00 00 02<sub>16</sub></tt>\tSHT_SYMTAB (Symbol table)\n"
                  "<tt>00 00 00 03<sub>16</sub></tt>\tSHT_STRTAB (String table)\n"
                  "<tt>00 00 00 04<sub>16</sub></tt>\tSHT_RELA (Explicit relocation)\n"
                  "<tt>00 00 00 05<sub>16</sub></tt>\tSHT_HASH (Hash table)\n"
                  "<tt>00 00 00 06<sub>16</sub></tt>\tSHT_DYNAMIC (Dynamic linking)\n"
                  "<tt>00 00 00 07<sub>16</sub></tt>\tSHT_NOTE (Note)\n"
                  "<tt>00 00 00 08<sub>16</sub></tt>\tSHT_NOBITS (Empty)\n"
                  "<tt>00 00 00 09<sub>16</sub></tt>\tSHT_REL (Implicit relocation)\n"
                  "<tt>00 00 00 0A<sub>16</sub></tt>\tSHT_SHLIB (Unspecified semantics)\n"
                  "<tt>00 00 00 0B<sub>16</sub></tt>\tSHT_DYNSYM (Dynamic linking symbol table)\n"
                  "<tt>00 00 00 0E<sub>16</sub></tt>\tSHT_INIT_ARRAY (Initialization functions)\n"
                  "<tt>00 00 00 0F<sub>16</sub></tt>\tSHT_FINI_ARRAY (Termination functions)\n"
                  "<tt>00 00 00 10<sub>16</sub></tt>\tSHT_PREINIT_ARRAY (Pre-initialization functions)\n"
                  "<tt>00 00 00 11<sub>16</sub></tt>\tSHT_GROUP (Section group)",
                HEADER_DATA_COLOR_1, 4, is_little_endian,
                G_N_ELEMENTS (section_type_values), section_type_values, section_type_value_description,
                NULL, FALSE, &section_type))
            break;

        /* Section flags (sh_flags) */
        if (!process_section_flags (file, &tab, is_64bits, is_little_endian))
            break;

        /* Section memory address (sh_addr) */
        if (!process_elf_field (file, &tab,
                "Section memory address", "Section memory address (sh_addr)",
                _("Address of the section in the memory image of the process"),
                HEADER_DATA_COLOR_1, field_length, is_little_endian,
                0, NULL, NULL,
                "%lX<sub>16</sub>", TRUE, NULL))
            break;

        /* Section offset (sh_offset) */
        if (!process_elf_field (file, &tab,
                "Section offset", "Section offset (sh_offset)",
                _("Offset of the section in the file"),
                HEADER_DATA_COLOR_2, field_length, is_little_endian,
                0, NULL, NULL,
                "%lX<sub>16</sub>", TRUE, &section_offset))
            break;

        /* Section size (sh_size) */
        if (!process_elf_field (file, &tab,
                "Section size", "Section size (sh_size)",
                _("Size of the section in the file"),
                HEADER_DATA_COLOR_1, field_length, is_little_endian,
                0, NULL, NULL,
                "%lu", TRUE, &section_size))
            break;

        /* Section link index (sh_link) */
        if (!process_elf_field (file, &tab,
                "Section link index", "Section link index (sh_link)",
                _("Some section types link to other sections"),
                HEADER_DATA_COLOR_2, 4, is_little_endian,
                0, NULL, NULL,
                "Section index %u", TRUE, NULL))
            break;

        /* Section information (sh_info) */
        if (!FILE_HAS_DATA_N (file, 4))
            break;

        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4, "Section information (sh_info)", NULL);

        /* Section address alignment (sh_addralign) */
        if (!process_elf_field (file, &tab,
                "Section address alignment", "Section address alignment (sh_addralign)",
                _("Some sections have address alignment constraints"),
                HEADER_DATA_COLOR_2, field_length, is_little_endian,
                0, NULL, NULL,
                "%lu", TRUE, NULL))
            break;

        /* Table entry size (sh_entsize) */
        if (!process_elf_field (file, &tab,
                "Table entry size", "Table entry size (sh_entsize)",
                _("Some sections hold a table of fixed-size entries, this gives the size of the entries"),
                HEADER_DATA_COLOR_1, field_length, is_little_endian,
                0, NULL, NULL,
                "%lu", TRUE, NULL))
            break;

        if (section_offset && section_size && (section_type != 0x8)) /* 0x8 = Empty (SHT_EMPTY) */
        {
            if (!section_name || section_name[0] == '\0')
                section_name = g_strdup_printf ("Section %d", i);

            save_index = GET_INDEX (file);

            SET_INDEX (file, section_offset);

            format_utils_add_field_full (file, HEADER_DATA_COLOR_1, TRUE, section_size,
                                         section_name, section_name, START_SECTION_DATA_COLOR);

            SET_INDEX (file, save_index);
        }

        g_free (g_steal_pointer (&section_name));
    }

    format_utils_insert_tab (file, &tab, _("Section header"));
}

static gboolean
get_section_name (FormatsFile    *file,
                  DescriptionTab *tab,
                  gsize           string_table_pointer,
                  gboolean        is_little_endian,
                  gchar         **section_name)
{
    guint32 four_bytes;

    GString *section_name_string;
    gsize section_name_offset;

    if (!format_utils_read (file, &four_bytes, 4))
        return FALSE;

    if (!is_little_endian)
        four_bytes = g_ntohl (four_bytes);

    format_utils_add_field (file, SECTION_NAME_COLOR, TRUE, 4,
                          _("Section name offset (sh_name)"), NULL);

    section_name_offset = string_table_pointer + four_bytes;

    if (string_table_pointer && section_name_offset < GET_FILE_SIZE (file))
    {
        section_name_string = g_string_new (NULL);

        four_bytes = strnlen ((const gchar *) file->file_contents + section_name_offset,
                              GET_FILE_SIZE (file) - section_name_offset);

        section_name_string = g_string_append_len (section_name_string,
                                  (const gchar *) file->file_contents + section_name_offset, four_bytes);
        if (section_name_string->len > 20)
        {
            section_name_string = g_string_truncate (section_name_string, 10);
            section_name_string = g_string_append (section_name_string, " [...]");
        }

        if (g_utf8_validate (section_name_string->str, -1, NULL))
            *section_name = g_string_free (section_name_string, FALSE);
        else
            g_string_free (section_name_string, TRUE);
    }

    format_utils_add_line_tab (tab, "Section name", *section_name, NULL);

    return TRUE;
}


static gboolean
process_section_flags (FormatsFile    *file,
                       DescriptionTab *tab,
                       gboolean        is_64bits,
                       gboolean        is_little_endian)
{
    GString *attributes_string;
    guint64 section_flags = 0;

    if (is_64bits)
    {
        if (!format_utils_read (file, &section_flags, 8))
            return FALSE;

        if (!is_little_endian)
            section_flags = GUINT64_FROM_BE (section_flags);

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 8,
                                "Section attributes (sh_flags)", NULL);
    }
    else
    {
        if (!format_utils_read (file, &section_flags, 4))
            return FALSE;

        if (!is_little_endian)
            section_flags = g_ntohl (section_flags);

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 4,
                                "Section attributes (sh_flags)", NULL);
    }

    attributes_string = g_string_new (NULL);
    if (section_flags & 0x1)
        g_string_append_printf (attributes_string, "%s\n", "SHF_WRITE");
    if (section_flags & 0x2)
        g_string_append_printf (attributes_string, "%s\n", "SHF_ALLOC");
    if (section_flags & 0x4)
        g_string_append_printf (attributes_string, "%s\n", "SHF_EXECINSTR");
    if (section_flags & 0x10)
        g_string_append_printf (attributes_string, "%s\n", "SHF_MERGE");
    if (section_flags & 0x20)
        g_string_append_printf (attributes_string, "%s\n", "SHF_STRINGS");
    if (section_flags & 0x40)
        g_string_append_printf (attributes_string, "%s\n", "SHF_INFO_LINK");
    if (section_flags & 0x80)
        g_string_append_printf (attributes_string, "%s\n", "SHF_LINK_ORDER");
    if (section_flags & 0x100)
        g_string_append_printf (attributes_string, "%s\n", "SHF_OS_NONCONFORMING");
    if (section_flags & 0x200)
        g_string_append_printf (attributes_string, "%s\n", "SHF_GROUP");
    if (section_flags & 0x400)
        g_string_append_printf (attributes_string, "%s\n", "SHF_TLS");
    if (section_flags & 0x800)
        g_string_append_printf (attributes_string, "%s\n", "SHF_COMPRESSED");

    if (attributes_string->len)
    {
        g_string_truncate (attributes_string, attributes_string->len - 1);
        format_utils_add_line_tab (tab, "Section attributes", attributes_string->str,
               "Section attributes (relevant bit masks)\n"
               "SHF_WRITE (<tt>1<sub>16</sub></tt>): Has writable data\n"
               "SHF_ALLOC (<tt>2<sub>16</sub></tt>): Uses memory during execution\n"
               "SHF_EXECINSTR (<tt>4<sub>16</sub></tt>): Has executable machine instructions\n"
               "SHF_MERGE (<tt>10<sub>16</sub></tt>): May be merged to eliminate duplicates\n"
               "SHF_STRINGS (<tt>20<sub>16</sub></tt>): Has null-terminated strings\n"
               "SHF_INFO_LINK (<tt>40<sub>16</sub></tt>): Holds a section header table index (sh_info is a section index)\n"
               "SHF_LINK_ORDER (<tt>80<sub>16</sub></tt>): Has ordering requirements\n"
               "SHF_OS_NONCONFORMING (<tt>100<sub>16</sub></tt>): Requires OS-specific processing\n"
               "SHF_GROUP (<tt>200<sub>16</sub></tt>): Is a member of a section group\n"
               "SHF_TLS (<tt>400<sub>16</sub></tt>): Holds Thread-Local Storage\n"
               "SHF_COMPRESSED (<tt>800<sub>16</sub></tt>): Has compressed data");
    }
    g_string_free (attributes_string, TRUE);

    return TRUE;
}
