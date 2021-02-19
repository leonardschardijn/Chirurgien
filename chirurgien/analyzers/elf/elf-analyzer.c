/* elf-analyzer.c
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

#include "chirurgien-analyze-elf.h"


void
chirurgien_analyze_elf (AnalyzerFile *file)
{
    guint8 byte;
    gboolean is_64bits;
    gboolean is_little_endian;

    guint16 program_header_entries, section_header_entries;
    guint64 program_header_offset = 0, section_header_offset = 0;

    guint16 section_header_entry_size, section_names_index;

    guint32 untagged_area;

    /* As sections use offsets to point to their contents, the tagged_bytes list
     * keeps track of all color tagged bytes, this let's us find bytes not part
     * of any section (or bytes that are part of more than one section) */
    GSList *tagged_bytes = NULL, *index;

    analyzer_utils_set_title (file, "<span weight=\"bold\" size=\"larger\">"
                                    "Executable and Linkable Format"
                                    "</span>");

    analyzer_utils_tag_navigation (file, SIGNATURE_COLOR, 4, _("ELF file signature"), "ELF");
    ADVANCE_POINTER (file, 4);
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    /* Class*/
    guint32 class_values[] = { 1, 2 };
    gchar *class_value_description[] = {
        _("32 bits"),
        _("64 bits"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_elf_field (file, NULL,
            _("Class"), NULL,
            _("Class\n"
              "<tt>01<sub>16</sub></tt>\t32 bits\n"
              "<tt>02<sub>16</sub></tt>\t64 bits"),
            HEADER_DATA_COLOR_1,
            1, FALSE, sizeof (class_values) >> 2, class_values, class_value_description, NULL, FALSE, &byte))
        goto END_ERROR;

    if (byte == 1)
        is_64bits = FALSE;
    else if (byte == 2)
        is_64bits = TRUE;
    else
        goto END_ERROR;

    /* Endianness */
    guint32 endianness_values[] = { 1, 2 };
    gchar *endianness_value_description[] = {
        _("Little-endian"),
        _("Big-endian"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_elf_field (file, NULL,
            _("Endianness"), NULL,
            _("Endianness\n"
              "<tt>01<sub>16</sub></tt>\tLittle-endian\n"
              "<tt>02<sub>16</sub></tt>\tBig-endian"),
            HEADER_DATA_COLOR_2,
            1, FALSE, sizeof (endianness_values) >> 2, endianness_values, endianness_value_description,
            NULL, FALSE, &byte))
        goto END_ERROR;

    if (byte == 1)
        is_little_endian = TRUE;
    else if (byte == 2)
        is_little_endian = FALSE;
    else
        goto END_ERROR;

    /* ELF header version */
    guint32 elf_header_version_values[] = { 1 };
    gchar *elf_header_version_value_description[] = {
        _("Version 1"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_elf_field (file, NULL,
            _("ELF header version"), NULL,
            _("ELF header version\n"
              "<tt>01<sub>16</sub></tt>\tVersion 1"),
            HEADER_DATA_COLOR_1,
            1, FALSE, sizeof (elf_header_version_values) >> 2, elf_header_version_values, elf_header_version_value_description,
            NULL, FALSE, NULL))
        goto END_ERROR;

    /* Application Binary Interface */
    guint32 abi_values[] = { 0x0, 0x1, 0x2, 0x3, 0x6, 0x7, 0x8, 0x9, 0xC };
    gchar *abi_value_description[] = {
        _("Unspecified/System V"),
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
            _("Application Binary Interface"), NULL,
            _("Application Binary Interface (incomplete list)\n"
              "<tt>00<sub>16</sub></tt>\tUnspecified/System V\n"
              "<tt>01<sub>16</sub></tt>\tHP-UX\n"
              "<tt>02<sub>16</sub></tt>\tNetBSD\n"
              "<tt>03<sub>16</sub></tt>\tLinux\n"
              "<tt>06<sub>16</sub></tt>\tSolaris\n"
              "<tt>07<sub>16</sub></tt>\tAIX\n"
              "<tt>08<sub>16</sub></tt>\tIRIX\n"
              "<tt>09<sub>16</sub></tt>\tFreeBSD\n"
              "<tt>0C<sub>16</sub></tt>\tOpenBSD"),
            HEADER_DATA_COLOR_2,
            1, FALSE, sizeof (abi_values) >> 2, abi_values, abi_value_description, NULL, FALSE, NULL))
        goto END_ERROR;

    /* ABI version */
    if (!process_elf_field (file, NULL,
            _("ABI version"), NULL,
            _("Identifies the ABI version, if incompatibilities exist"),
            HEADER_DATA_COLOR_1,
            1, FALSE, 0, NULL, NULL, "%u", TRUE, NULL))
        goto END_ERROR;

    /* Unused data */
    if (!FILE_HAS_DATA_N (file, 7))
        goto END_ERROR;

    ADVANCE_POINTER (file, 7);
    analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 7, _("Unused"));

    /* Object file type */
    guint32 object_file_type_values[] = { 0, 1, 2, 3, 4 };
    gchar *object_file_type_value_description[] = {
        _("Unspecified"),
        _("Relocatable file"),
        _("Executable file"),
        _("Shared object file"),
        _("Core file"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_elf_field (file, NULL,
            _("Object file type"), NULL,
            _("Object file type\n"
              "<tt>00 00<sub>16</sub></tt>\tUnspecified\n"
              "<tt>00 01<sub>16</sub></tt>\tRelocatable file\n"
              "<tt>00 02<sub>16</sub></tt>\tExecutable file\n"
              "<tt>00 03<sub>16</sub></tt>\tShared object file\n"
              "<tt>00 04<sub>16</sub></tt>\tCore file"),
            HEADER_DATA_COLOR_1,
            2, is_little_endian, sizeof (object_file_type_values) >> 2, object_file_type_values, object_file_type_value_description,
            NULL, FALSE, NULL))
        goto END_ERROR;

    /* Instruction Set Architecture */
    guint32 isa_values[] = { 0x0, 0x2, 0x3, 0x4, 0x5, 0x14, 0x15, 0x16, 0x28,
        0x2B, 0x32, 0x3E, 0xB7, 0xF3 };
    gchar *isa_value_description[] = {
        _("No machine"),
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
        _("<span foreground=\"red\">Unknown</span>")
    };
    if (!process_elf_field (file, NULL,
            _("Instruction Set Architecture"), NULL,
            _("Instruction Set Architecture (incomplete list)\n"
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
              "<tt>00 F3<sub>16</sub></tt>\tRISC-V"),
            HEADER_DATA_COLOR_2,
            2, is_little_endian, sizeof (isa_values) >> 2, isa_values, isa_value_description,
            NULL, FALSE,  NULL))
        goto END_ERROR;

    /* Object file version */
    guint32 obj_file_version_values[] = { 1 };
    gchar *obj_file_value_description[] = {
        _("Version 1"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_elf_field (file, NULL,
            _("Object file version"), NULL,
            _("Object file version\n"
             "<tt>00 00 00 01<sub>16</sub></tt>\tVersion 1"),
            HEADER_DATA_COLOR_1,
            4, is_little_endian, sizeof (obj_file_version_values) >> 2, obj_file_version_values, obj_file_value_description,
            NULL, FALSE,  NULL))
        goto END_ERROR;

    if (is_64bits)
    {
        /* Entry point */
        if (!process_elf_field (file, NULL,
                _("Entry point"), NULL,
                _("Virtual address of the entry point"),
                HEADER_DATA_COLOR_2,
                8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", FALSE, NULL))
            goto END_ERROR;

        /* Program header table offset */
        if (!process_elf_field (file, NULL,
                _("Program header table offset"), NULL,
                _("Offset of the program header table"),
                HEADER_DATA_COLOR_1,
                8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", FALSE, &program_header_offset))
            goto END_ERROR;

        /* Section header table offset */
        if (!process_elf_field (file, NULL,
                _("Section header table offset"), NULL,
                _("Offset of the section header table"),
                HEADER_DATA_COLOR_2,
                8, is_little_endian, 0, NULL, NULL, "%lX<sub>16</sub>", FALSE, &section_header_offset))
            goto END_ERROR;
    }
    else
    {
        /* Entry point */
        if (!process_elf_field (file, NULL,
                _("Entry point"), NULL,
                _("Virtual address of the entry point"),
                HEADER_DATA_COLOR_2,
                4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", FALSE, NULL))
            goto END_ERROR;

        /* Program header table offset */
        if (!process_elf_field (file, NULL,
                _("Program header table offset"), NULL,
                _("File offset of the program header table"),
                HEADER_DATA_COLOR_1,
                4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", FALSE, &program_header_offset))
            goto END_ERROR;

        /* Section header table offset */
        if (!process_elf_field (file, NULL,
                _("Section header table offset"), NULL,
                _("File offset of the section header table"),
                HEADER_DATA_COLOR_2,
                4, is_little_endian, 0, NULL, NULL, "%X<sub>16</sub>", FALSE, &section_header_offset))
            goto END_ERROR;
    }

    /* Flags */
    if (!FILE_HAS_DATA_N (file, 4))
        goto END_ERROR;

    ADVANCE_POINTER (file, 4);
    analyzer_utils_tag (file, HEADER_DATA_COLOR_1, 4, _("Flags"));

    /* ELF header size */
    if (!process_elf_field (file, NULL,
            _("ELF header size"), NULL,
            _("Size of the ELF header, in bytes"),
            HEADER_DATA_COLOR_2,
            2, is_little_endian, 0, NULL, NULL, "%u", FALSE, NULL))
        goto END_ERROR;

    /* Program header table entry size */
    if (!process_elf_field (file, NULL,
            _("Program header table entry size"), NULL,
            _("Size of the entries in the program header table, in bytes"),
            HEADER_DATA_COLOR_1,
            2, is_little_endian, 0, NULL, NULL, "%u", FALSE, NULL))
        goto END_ERROR;

    /* Number of entries in the program header table */
    if (!process_elf_field (file, NULL,
            _("Number of entries in the program header table"), NULL,
            NULL, HEADER_DATA_COLOR_2,
            2, is_little_endian, 0, NULL, NULL, NULL, FALSE, &program_header_entries))
        goto END_ERROR;

    /* Section header table entry size */
    if (!process_elf_field (file, NULL,
            _("Section header table entry size"), NULL,
            _("Size of the entries in the section header table, in bytes"),
            HEADER_DATA_COLOR_1,
            2, is_little_endian, 0, NULL, NULL, "%u", FALSE, &section_header_entry_size))
        goto END_ERROR;

    /* Number of entries in the section header table */
    if (!process_elf_field (file, NULL,
            _("Number of entries in the section header table"), NULL,
            NULL, HEADER_DATA_COLOR_2,
            2, is_little_endian, 0, NULL, NULL, NULL, FALSE, &section_header_entries))
        goto END_ERROR;

    /* Section names index */
    if (!process_elf_field (file, NULL,
            _("Section names index"), NULL,
            _("Section header index of the section with the section names"),
            HEADER_DATA_COLOR_1,
            2, is_little_endian, 0, NULL, NULL, _("Section index %u"), FALSE, &section_names_index))
        goto END_ERROR;

    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    analyze_program_header (file, is_64bits, is_little_endian, program_header_offset, program_header_entries,
                            &tagged_bytes);

    analyze_section_header (file, is_64bits, is_little_endian, section_header_offset, section_header_entries,
                            section_header_entry_size * section_names_index, &tagged_bytes);

    /* Add the tagged area boundaries: 4 (as the initial 4-bytes signature is already tagged)
     * and file size */
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (4));
    tagged_bytes = g_slist_append (tagged_bytes, GUINT_TO_POINTER (GET_FILE_SIZE (file)));

    /* Sort the tagged area file indices */
    tagged_bytes = g_slist_sort (tagged_bytes, tagged_bytes_compare);

    for (index = tagged_bytes;
    index != NULL;
    index = index->next->next)
    {
        /* Malformed file: offset/file index exceeds file size */
        if (GPOINTER_TO_UINT (index->data) > GET_FILE_SIZE (file))
            break;

        /* All pairs must have the same file index */
        untagged_area = GPOINTER_TO_UINT (index->next->data) - GPOINTER_TO_UINT (index->data);

        if (untagged_area)
            analyzer_utils_create_tag_index (file, UNUSED_OVERLAPPING_COLOR, FALSE, untagged_area,
                                 GPOINTER_TO_UINT (index->data), _("Unused or overlapping data"));
    }
    g_slist_free (tagged_bytes);
    return;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    g_slist_free (tagged_bytes);
}

gboolean
process_elf_field (AnalyzerFile *file,
                   AnalyzerTab *tab,
                   gchar *field_name,
                   gchar *field_name_tag,
                   gchar *field_tooltip,
                   GdkRGBA *color,
                   guint field_length,
                   gboolean is_little_endian,
                   guint possible_values,
                   guint32 *field_values,
                   gchar **value_descriptions,
                   gchar *description_message,
                   gboolean ignore_empty,
                   void *read_value)
{
    gchar *field_description = NULL;
    guint64 eight_bytes = 0;

    if (!analyzer_utils_read (&eight_bytes, file, field_length))
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
        analyzer_utils_tag (file, color, field_length, field_name_tag);
    else
        analyzer_utils_tag (file, color, field_length, field_name);

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
            analyzer_utils_describe_tooltip_tab (tab, field_name, field_description, field_tooltip);
        else
            analyzer_utils_describe_tooltip (file, field_name, field_description, field_tooltip);
    }
    else if (description_message)
    {
        if (!(ignore_empty && !eight_bytes))
        {
            field_description = g_strdup_printf (description_message, eight_bytes);
            if (tab)
                analyzer_utils_describe_tooltip_tab (tab, field_name, field_description, field_tooltip);
            else
                analyzer_utils_describe_tooltip (file, field_name, field_description, field_tooltip);
            g_free (field_description);
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

