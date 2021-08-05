/* elf-format.c
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

#include "chirurgien-elf.h"


void
chirurgien_elf (FormatsFile *file)
{
    guint8 byte;
    gboolean is_64bits;
    gboolean is_little_endian;

    guint field_length;
    guint16 program_header_entries, section_header_entries;
    guint64 program_header_offset = 0, section_header_offset = 0;

    guint16 section_header_entry_size, section_names_index;

    format_utils_set_title (file, "Executable and Linkable Format");

    format_utils_add_field (file, SIGNATURE_COLOR, TRUE, 4, "ELF file signature", "ELF");

    format_utils_start_section (file, "ELF header");

    /* Class*/
    guint32 class_values[] = { 1, 2 };
    const gchar *class_value_description[] = {
        "ELFCLASS32 (32 bits)",
        "ELFCLASS64 (64 bits)",
        "<span foreground=\"red\">INVALID</span>"
    };
    if (!process_elf_field (file, NULL,
              "Class", NULL,
              "Class\n"
              "<tt>01<sub>16</sub></tt>\tELFCLASS32 (32 bits)\n"
              "<tt>02<sub>16</sub></tt>\tELFCLASS64 (64 bits)",
            HEADER_DATA_COLOR_1, 1, FALSE,
            G_N_ELEMENTS (class_values), class_values, class_value_description,
            NULL, FALSE, &byte))
        return;

    if (byte == 1)
    {
        is_64bits = FALSE;
        field_length = 4;
    }
    else if (byte == 2)
    {
        is_64bits = TRUE;
        field_length = 8;
    }
    else
        return;

    /* Endianness */
    guint32 endianness_values[] = { 1, 2 };
    const gchar *endianness_value_description[] = {
        "ELFDATA2LSB (Little-endian)",
        "ELFDATA2MSB (Big-endian)",
        "<span foreground=\"red\">INVALID</span>"
    };
    if (!process_elf_field (file, NULL,
              "Endianness", NULL,
              "Endianness\n"
              "<tt>01<sub>16</sub></tt>\tELFDATA2LSB (Little-endian)\n"
              "<tt>02<sub>16</sub></tt>\tELFDATA2MSB (Big-endian)",
            HEADER_DATA_COLOR_2, 1, FALSE,
            G_N_ELEMENTS (endianness_values), endianness_values, endianness_value_description,
            NULL, FALSE, &byte))
        return;

    if (byte == 1)
        is_little_endian = TRUE;
    else if (byte == 2)
        is_little_endian = FALSE;
    else
        return;

    /* ELF header version */
    guint32 elf_header_version_values[] = { 1 };
    const gchar *elf_header_version_value_description[] = {
        "EV_CURRENT (Current version)",
        "<span foreground=\"red\">INVALID</span>"
    };
    if (!process_elf_field (file, NULL,
              "ELF header version", NULL,
              "ELF header version\n"
              "<tt>01<sub>16</sub></tt>\tEV_CURRENT (Current version)",
            HEADER_DATA_COLOR_1, 1, FALSE,
            G_N_ELEMENTS (elf_header_version_values), elf_header_version_values, elf_header_version_value_description,
            NULL, FALSE, NULL))
        return;

    /* Application Binary Interface */
    guint32 abi_values[] = { 0x0, 0x1, 0x2, 0x3, 0x6, 0x7, 0x8, 0x9, 0xC };
    const gchar *abi_value_description[] = {
          "Unspecified/System V",
          "HP-UX",
          "NetBSD",
          "Linux",
          "Solaris",
          "AIX",
          "IRIX",
          "FreeBSD",
          "OpenBSD",
          "<span foreground=\"red\">\?\?\?</span>"
    };
    if (!process_elf_field (file, NULL,
              "Application Binary Interface", NULL,
              "Application Binary Interface (incomplete list)\n"
              "<tt>00<sub>16</sub></tt>\tUnspecified/System V\n"
              "<tt>01<sub>16</sub></tt>\tHP-UX\n"
              "<tt>02<sub>16</sub></tt>\tNetBSD\n"
              "<tt>03<sub>16</sub></tt>\tLinux\n"
              "<tt>06<sub>16</sub></tt>\tSolaris\n"
              "<tt>07<sub>16</sub></tt>\tAIX\n"
              "<tt>08<sub>16</sub></tt>\tIRIX\n"
              "<tt>09<sub>16</sub></tt>\tFreeBSD\n"
              "<tt>0C<sub>16</sub></tt>\tOpenBSD",
            HEADER_DATA_COLOR_2, 1, FALSE,
            G_N_ELEMENTS (abi_values), abi_values, abi_value_description,
            NULL, FALSE, NULL))
        return;

    /* ABI version */
    if (!process_elf_field (file, NULL,
            "ABI version", NULL,
            "Identifies the ABI version, if incompatibilities exist",
            HEADER_DATA_COLOR_1, 1, FALSE,
            0, NULL, NULL,
            "%u", TRUE, NULL))
        return;

    /* Unused data */
    if (!FILE_HAS_DATA_N (file, 7))
        return;

    format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 7, "Unused", NULL);

    /* Object file type */
    guint32 object_file_type_values[] = { 0, 1, 2, 3, 4 };
    const gchar *object_file_type_value_description[] = {
        "ET_NONE (Unspecified)",
        "ET_REL (Relocatable file)",
        "ET_EXEC (Executable file)",
        "ET_DYN (Shared object file)",
        "ET_CORE (Core file)",
        "<span foreground=\"red\">INVALID</span>"
    };
    if (!process_elf_field (file, NULL,
              "Object file type", NULL,
              "Object file type\n"
              "<tt>00 00<sub>16</sub></tt>\tET_NONE (Unspecified)\n"
              "<tt>00 01<sub>16</sub></tt>\tET_REL (Relocatable file)\n"
              "<tt>00 02<sub>16</sub></tt>\tET_EXEC (Executable file)\n"
              "<tt>00 03<sub>16</sub></tt>\tET_DYN (Shared object file)\n"
              "<tt>00 04<sub>16</sub></tt>\tET_CORE (Core file)",
            HEADER_DATA_COLOR_1, 2, is_little_endian,
            G_N_ELEMENTS (object_file_type_values), object_file_type_values, object_file_type_value_description,
            NULL, FALSE, NULL))
        return;

    /* Instruction Set Architecture */
    guint32 isa_values[] = { 0x0, 0x2, 0x3, 0x4, 0x5, 0x14, 0x15, 0x16, 0x28,
        0x2B, 0x32, 0x3E, 0xB7, 0xF3 };
    const gchar *isa_value_description[] = {
          "No machine",
          "SPARC",
          "Intel 80386",
          "Motorola 68000",
          "Motorola 88000",
          "PowerPC",
          "PowerPC 64-bit",
          "S390",
          "ARM 32-bit",
          "SPARC Version 9",
          "Intel IA-64",
          "AMD64",
          "ARM 64-bit",
          "RISC-V",
          "<span foreground=\"red\">Unknown</span>"
    };
    if (!process_elf_field (file, NULL,
              "Instruction Set Architecture", NULL,
              "Instruction Set Architecture (incomplete list)\n"
              "<tt>00 00<sub>16</sub></tt>\tNo machine\n"
              "<tt>00 02<sub>16</sub></tt>\tSPARC\n"
              "<tt>00 03<sub>16</sub></tt>\tIntel 80386\n"
              "<tt>00 04<sub>16</sub></tt>\tMotorola 68000\n"
              "<tt>00 05<sub>16</sub></tt>\tMotorola 88000\n"
              "<tt>00 14<sub>16</sub></tt>\tPowerPC\n"
              "<tt>00 15<sub>16</sub></tt>\tPowerPC 64-bit\n"
              "<tt>00 16<sub>16</sub></tt>\tS390\n"
              "<tt>00 28<sub>16</sub></tt>\tARM 32-bit\n"
              "<tt>00 2B<sub>16</sub></tt>\tSPARC Version 9\n"
              "<tt>00 32<sub>16</sub></tt>\tIntel IA-64\n"
              "<tt>00 3E<sub>16</sub></tt>\tAMD64\n"
              "<tt>00 B7<sub>16</sub></tt>\tARM 64-bit\n"
              "<tt>00 F3<sub>16</sub></tt>\tRISC-V",
            HEADER_DATA_COLOR_2, 2, is_little_endian,
            G_N_ELEMENTS (isa_values), isa_values, isa_value_description,
            NULL, FALSE, NULL))
        return;

    /* Object file version */
    guint32 obj_file_version_values[] = { 1 };
    const gchar *obj_file_value_description[] = {
        "EV_CURRENT (Current version)",
        "<span foreground=\"red\">INVALID</span>"
    };
    if (!process_elf_field (file, NULL,
              "Object file version", NULL,
              "Object file version\n"
              "<tt>00 00 00 01<sub>16</sub></tt>\tEV_CURRENT (Current version)",
            HEADER_DATA_COLOR_1, 4, is_little_endian,
            G_N_ELEMENTS (obj_file_version_values), obj_file_version_values, obj_file_value_description,
            NULL, FALSE, NULL))
        return;

    /* Entry point */
    if (!process_elf_field (file, NULL,
            "Entry point", NULL,
            "Virtual address of the entry point",
            HEADER_DATA_COLOR_2, field_length, is_little_endian,
            0, NULL, NULL,
            "%lX<sub>16</sub>" , FALSE, NULL))
        return;

    /* Program header table offset */
    if (!process_elf_field (file, NULL,
            "Program header table offset", NULL,
            "Offset of the program header table",
            HEADER_DATA_COLOR_1, field_length, is_little_endian,
            0, NULL, NULL,
            "%lX<sub>16</sub>", FALSE, &program_header_offset))
        return;

    /* Section header table offset */
    if (!process_elf_field (file, NULL,
            "Section header table offset", NULL,
            "Offset of the section header table",
            HEADER_DATA_COLOR_2, field_length, is_little_endian,
            0, NULL, NULL,
            "%lX<sub>16</sub>", FALSE, &section_header_offset))
        return;

    /* Flags */
    if (!FILE_HAS_DATA_N (file, 4))
        return;

    format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4, "Flags", NULL);

    /* ELF header size */
    if (!process_elf_field (file, NULL,
            "ELF header size", NULL,
            "Size of the ELF header, in bytes",
            HEADER_DATA_COLOR_2, 2, is_little_endian,
            0, NULL, NULL,
            "%u", FALSE, NULL))
        return;

    /* Program header table entry size */
    if (!process_elf_field (file, NULL,
            "Program header table entry size", NULL,
            "Size of the entries in the program header table, in bytes",
            HEADER_DATA_COLOR_1, 2, is_little_endian,
            0, NULL, NULL,
            "%u", FALSE, NULL))
        return;

    /* Number of entries in the program header table */
    if (!process_elf_field (file, NULL,
            "Number of entries in the program header table", NULL,
            NULL,
            HEADER_DATA_COLOR_2, 2, is_little_endian,
            0, NULL, NULL,
            NULL, FALSE, &program_header_entries))
        return;

    /* Section header table entry size */
    if (!process_elf_field (file, NULL,
            "Section header table entry size", NULL,
            "Size of the entries in the section header table, in bytes",
            HEADER_DATA_COLOR_1, 2, is_little_endian,
            0, NULL, NULL,
            "%u", FALSE, &section_header_entry_size))
        return;

    /* Number of entries in the section header table */
    if (!process_elf_field (file, NULL,
            "Number of entries in the section header table", NULL,
            NULL,
            HEADER_DATA_COLOR_2, 2, is_little_endian,
            0, NULL, NULL,
            NULL, FALSE, &section_header_entries))
        return;

    /* Section names index */
    if (!process_elf_field (file, NULL,
            "Section names index", NULL,
            "Section header index of the section with the section names",
            HEADER_DATA_COLOR_1, 2, is_little_endian,
            0, NULL, NULL,
            "Section index %u", FALSE, &section_names_index))
        return;

    elf_program_header (file, is_64bits, is_little_endian, program_header_offset, program_header_entries);

    elf_section_header (file, is_64bits, is_little_endian, section_header_offset, section_header_entries,
                        section_header_entry_size * section_names_index);
}

gboolean
process_elf_field (FormatsFile    *file,
                   DescriptionTab *tab,
                   const gchar    *field_name,
                   const gchar    *field_name_tag,
                   const gchar    *field_tooltip,
                   gint            color_index,
                   guint           field_length,
                   gboolean        is_little_endian,
                   guint           possible_values,
                   guint32        *field_values,
                   const gchar   **value_descriptions,
                   const gchar    *field_format,
                   gboolean        ignore_empty,
                   gpointer        read_value)
{
    const gchar *field_description = NULL;
    guint64 eight_bytes = 0;

    if (!format_utils_read (file, &eight_bytes, field_length))
        return FALSE;

    if (!is_little_endian)
    {
        if (field_length == 2)
            eight_bytes = g_ntohs (eight_bytes);
        else if (field_length == 4)
            eight_bytes = g_ntohl (eight_bytes);
        else if (field_length == 8)
            eight_bytes = GUINT64_FROM_BE (eight_bytes);
    }

    if (field_name_tag)
        format_utils_add_field (file, color_index, TRUE, field_length,
                                field_name_tag, NULL);
    else
        format_utils_add_field (file, color_index, TRUE, field_length,
                                field_name, NULL);

    if (possible_values)
    {
        for (guint i = 0; i < possible_values; i++)
        {
            if (eight_bytes == field_values[i])
            {
                field_description = value_descriptions[i];
                break;
            }
        }

        if (!field_description)
            field_description = value_descriptions[possible_values];

        if (tab)
            format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
        else
            format_utils_add_line (file, field_name, field_description, field_tooltip);
    }
    else if (field_format)
    {
        if (!ignore_empty || eight_bytes)
        {
            field_description = g_strdup_printf (field_format, eight_bytes);
            if (tab)
                format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
            else
                format_utils_add_line (file, field_name, field_description, field_tooltip);
            g_free ((gpointer) field_description);
        }
    }

    if (read_value)
    {
        if (field_length == 1)
            *(guint8 *) read_value = eight_bytes;
        else if (field_length == 2)
            *(guint16 *) read_value = eight_bytes;
        else if (field_length == 4)
            *(guint32 *) read_value = eight_bytes;
        else if (field_length == 8)
            *(guint64 *) read_value = eight_bytes;
    }

    return TRUE;
}
