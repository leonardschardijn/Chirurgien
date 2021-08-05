/* pe-format.c
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

#include "chirurgien-pe.h"


void
chirurgien_pe (FormatsFile *file,
               guint32      pe_offset)
{
    guint16 sections, characteristics, optional_header_size, magic;
    GString *characteristics_string;

    guint field_length;
    guint32 sym_table_offset, table_length, data_directory_entries;

    gsize save_index;

    format_utils_add_field (file, SIGNATURE_COLOR, TRUE, 2, "DOS file signature", "DOS");
    format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 62, "DOS Header", NULL);
    format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, pe_offset - 64, "DOS Stub", NULL);

    format_utils_set_title (file, "Portable Executable");

    format_utils_add_field (file, SIGNATURE_COLOR, TRUE, 4, "PE file signature", "PE");

    format_utils_start_section (file, "COFF header");

    /* Machine */
    guint32 machine_values[] = { 0x0, 0x8664, 0x1C0, 0xAA64, 0x1C4, 0xEBC, 0x14C, 0x200, 0x1F0,
        0x1F1, 0x5032, 0x5064, 0x5128 };
    const gchar *machine_value_description[] = {
          "Any machine",
          "x64 (amd64/x86_64)",
          "ARM little-endian",
          "ARM64 little-endian",
          "ARM Thumb-2 little-endian",
          "EFI byte code",
          "Intel 80386",
          "Intel Itanium",
          "PowerPC little-endian",
          "PowerPC with floating point support",
          "RISC-V 32-bit address space",
          "RISC-V 64-bit address space",
          "RISC-V 128-bit address space",
          "<span foreground=\"red\">Unknown</span>"
    };
    if (!process_pe_field (file, NULL,
              "Machine", NULL,
              "Machine types (incomplete list)\n"
              "<tt>00 00<sub>16</sub></tt>\tAny machine\n"
              "<tt>86 64<sub>16</sub></tt>\tx64 (amd64/x86_64)\n"
              "<tt>01 C0<sub>16</sub></tt>\tARM little-endian\n"
              "<tt>AA 64<sub>16</sub></tt>\tARM64 little-endian\n"
              "<tt>01 C4<sub>16</sub></tt>\tARM Thumb-2 little-endian\n"
              "<tt>0E BC<sub>16</sub></tt>\tEFI byte code\n"
              "<tt>01 4C<sub>16</sub></tt>\tIntel 80386\n"
              "<tt>02 00<sub>16</sub></tt>\tIntel Itanium\n"
              "<tt>01 F0<sub>16</sub></tt>\tPowerPC little-endian\n"
              "<tt>01 F1<sub>16</sub></tt>\tPowerPC with floating point support\n"
              "<tt>50 32<sub>16</sub></tt>\tRISC-V 32-bit address space\n"
              "<tt>50 64<sub>16</sub></tt>\tRISC-V 64-bit address space\n"
              "<tt>51 28<sub>16</sub></tt>\tRISC-V 128-bit address space",
            HEADER_DATA_COLOR_1, 2,
            G_N_ELEMENTS (machine_values), machine_values, machine_value_description,
            NULL, NULL))
        return;

    /* NumberOfSections */
    if (!process_pe_field (file, NULL,
            "NumberOfSections", NULL,
            _("The number of sections"),
            HEADER_DATA_COLOR_2, 2,
            0, NULL, NULL,
            "%u", &sections))
        return;

    /* TimeDateStamp  */
    if (!process_pe_field_simple (file, NULL,
            "TimeDateStamp",
            _("File creation date (seconds since January 1, 1970)"),
            HEADER_DATA_COLOR_1, 4, "%u"))
        return;

    /* PointerToSymbolTable */
    if (!process_pe_field (file, NULL,
            "PointerToSymbolTable", NULL,
            _("File offset to the COFF symbol table"),
            HEADER_DATA_COLOR_2, 4,
            0, NULL, NULL,
           "%X<sub>16</sub>", &sym_table_offset))
        return;

    /* NumberOfSymbols */
    if (!process_pe_field (file, NULL,
            "NumberOfSymbols", NULL,
            _("Number of entries in the symbol table"),
            HEADER_DATA_COLOR_1, 4,
            0, NULL, NULL,
            "%u", &table_length))
        return;

    /* SizeOfOptionalHeader */
    if (!process_pe_field (file, NULL,
            "SizeOfOptionalHeader", NULL,
            _("Size of the optional header (required for executable files)"),
            HEADER_DATA_COLOR_2, 2,
            0, NULL, NULL,
            "%u", &optional_header_size))
        return;

    /* Characteristics */
    if (!format_utils_read (file, &characteristics, 2))
        return;

    format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 2,
                            "Characteristics", NULL);

    characteristics_string = g_string_new (NULL);
    if (characteristics & 0x1)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_RELOCS_STRIPPED");
    if (characteristics & 0x2)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_EXECUTABLE_IMAGE");
    if (characteristics & 0x20)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_LARGE_ADDRESS_AWARE");
    if (characteristics & 0x100)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_32BIT_MACHINE");
    if (characteristics & 0x200)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_DEBUG_STRIPPED");
    if (characteristics & 0x400)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP");
    if (characteristics & 0x800)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_NET_RUN_FROM_SWAP");
    if (characteristics & 0x1000)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_SYSTEM");
    if (characteristics & 0x2000)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_DLL");
    if (characteristics & 0x4000)
        g_string_append_printf (characteristics_string, "%s\n", "IMAGE_FILE_UP_SYSTEM_ONLY");

    if (characteristics_string->len)
    {
        g_string_truncate (characteristics_string, characteristics_string->len - 1);
        format_utils_add_line (file, "Characteristics", characteristics_string->str,
               "Characteristic flags (bit mask)\n"
               "IMAGE_FILE_RELOCS_STRIPPED (<tt>1<sub>16</sub></tt>): File has no base relocations\n"
               "IMAGE_FILE_EXECUTABLE_IMAGE (<tt>2<sub>16</sub></tt>): File can be run\n"
               "IMAGE_FILE_LARGE_ADDRESS_AWARE (<tt>20<sub>16</sub></tt>): Application can handle >2GiB addresses\n"
               "IMAGE_FILE_32BIT_MACHINE (<tt>100<sub>16</sub></tt>): Machine is based on a 32-bit-word architecture\n"
               "IMAGE_FILE_DEBUG_STRIPPED (<tt>200<sub>16</sub></tt>): Debugging information is removed\n"
               "IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP (<tt>400<sub>16</sub></tt>): If the image is on removable media, fully load it and copy it to the swap file\n"
               "IMAGE_FILE_NET_RUN_FROM_SWAP (<tt>800<sub>16</sub></tt>): If the image is on network media, fully load it and copy it to the swap file\n"
               "IMAGE_FILE_SYSTEM (<tt>1000<sub>16</sub></tt>): The image file is a system file, not a user program\n"
               "IMAGE_FILE_DLL (<tt>2000<sub>16</sub></tt>): The image file is a dynamic-link library (DLL)\n"
               "IMAGE_FILE_UP_SYSTEM_ONLY (<tt>4000<sub>16</sub></tt>): The file should be run only on a uniprocessor machine");
    }
    g_string_free (characteristics_string, TRUE);

    if (optional_header_size)
    {
        format_utils_start_section (file, _("Optional header standard fields"));

        /* Magic */
        guint32 magic_values[] = { 0x107, 0x10B,0x20B };
        const gchar *magic_value_description[] = {
              "ROM image",
              "PE32",
              "PE32+",
              "<span foreground=\"red\">Unknown</span>"
        };
        if (!process_pe_field (file, NULL,
                  "Magic", NULL,
                  "Magic\n"
                  "<tt>01 07<sub>16</sub></tt>\tROM image\n"
                  "<tt>01 0B<sub>16</sub></tt>\tPE32\n"
                  "<tt>02 0B<sub>16</sub></tt>\tPE32+",
                HEADER_DATA_COLOR_2, 2,
                G_N_ELEMENTS (magic_values), magic_values, magic_value_description,
                NULL, &magic))
            return;

        /* MajorLinkerVersion */
        if (!process_pe_field_simple (file, NULL,
                "MajorLinkerVersion",
                _("The linker major version number"),
                HEADER_DATA_COLOR_1, 1, "%u"))
            return;

        /* MinorLinkerVersion */
        if (!process_pe_field_simple (file, NULL,
                "MinorLinkerVersion",
                _("The linker minor version number"),
                HEADER_DATA_COLOR_2, 1, "%u"))
            return;

        /* SizeOfCode */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfCode",
                _("The size of the code (text) section, or the sum of all code sections if there are multiple sections"),
                HEADER_DATA_COLOR_1, 4, "%u"))
            return;

        /* SizeOfInitializedData */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfInitializedData",
                _("The size of the initialized data section, or the sum of all such sections if there are multiple data sections"),
                HEADER_DATA_COLOR_2, 4, "%u"))
            return;

        /* SizeOfUninitializedData */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfUninitializedData",
                _("The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections"),
                HEADER_DATA_COLOR_1, 4, "%u"))
            return;

        /* AddressOfEntryPoint */
        if (!process_pe_field_simple (file, NULL,
                "AddressOfEntryPoint",
                _("The address of the entry point relative to the image base when the executable file is loaded into memory"),
                HEADER_DATA_COLOR_2, 4, "%X<sub>16</sub>"))
            return;

        /* BaseOfCode */
        if (!process_pe_field_simple (file, NULL,
                "BaseOfCode",
                _("The address that is relative to the image base of the beginning-of-code section when it is loaded into memory"),
                HEADER_DATA_COLOR_1, 4, "%X<sub>16</sub>"))
            return;

        /* Only present on PE32: BaseOfData */
        if (magic == 0x10B)
        {
            if (!process_pe_field_simple (file, NULL,
                "BaseOfData",
                _("The address that is relative to the image base of the beginning-of-data section when it is loaded into memory"),
                HEADER_DATA_COLOR_1, 4, "%X<sub>16</sub>"))
            return;
        }

        format_utils_start_section (file, "Optional header Windows-specific fields");

        if (magic == 0x20B)
            field_length = 8;
        else
            field_length = 4;

        /* ImageBase */
        if (!process_pe_field_simple (file, NULL,
                "ImageBase",
                _("The preferred address of the first byte of image when loaded into memory; must be a multiple of 64KiB"),
                HEADER_DATA_COLOR_2, field_length, "%lX<sub>16</sub>"))
            return;

        /* SectionAlignment */
        if (!process_pe_field_simple (file, NULL,
                "SectionAlignment",
                _("The alignment (in bytes) of sections when they are loaded into memory"),
                HEADER_DATA_COLOR_1, 4, "%u"))
            return;

        /* FileAlignment */
        if (!process_pe_field_simple (file, NULL,
                "FileAlignment",
                _("The alignment factor (in bytes) that is used to align the raw data of sections in the image file"),
                HEADER_DATA_COLOR_2, 4,"%u"))
            return;

        /* MajorOperatingSystemVersion */
        if (!process_pe_field_simple (file, NULL,
                "MajorOperatingSystemVersion",
                _("The major version number of the required operating system"),
                HEADER_DATA_COLOR_1, 2, "%u"))
            return;

        /* MinorOperatingSystemVersion  */
        if (!process_pe_field_simple (file, NULL,
                "MinorOperatingSystemVersion",
                _("The minor version number of the required operating system"),
                HEADER_DATA_COLOR_2, 2, "%u"))
            return;

        /* MajorImageVersion */
        if (!process_pe_field_simple (file, NULL,
                "MajorImageVersion",
                _("The major version number of the image"),
                HEADER_DATA_COLOR_1, 2, "%u"))
            return;

        /* MinorImageVersion */
        if (!process_pe_field_simple (file, NULL,
                "MinorImageVersion",
                _("The minor version number of the image"),
                HEADER_DATA_COLOR_2, 2, "%u"))
            return;

        /* MajorSubsystemVersion */
        if (!process_pe_field_simple (file, NULL,
                "MajorSubsystemVersion",
                _("The major version number of the subsystem"),
                HEADER_DATA_COLOR_1, 2, "%u"))
            return;

        /* MinorSubsystemVersion */
        if (!process_pe_field_simple (file, NULL,
                "MinorSubsystemVersion",
                _("The minor version number of the subsystem"),
                HEADER_DATA_COLOR_2, 2, "%u"))
            return;

        /* process_pe_field_simple */
        if (!process_pe_field_simple (file, NULL,
                "Win32VersionValue",
                NULL,
                HEADER_DATA_COLOR_1, 4, NULL))
            return;

        /* SizeOfImage */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfImage",
                _("The size (in bytes) of the image, including all headers, as the image is loaded in memory"),
                HEADER_DATA_COLOR_2, 4, "%u"))
            return;

        /* SizeOfHeaders */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfHeaders",
                _("The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment"),
                HEADER_DATA_COLOR_1, 4, "%u"))
            return;

        /* CheckSum */
        if (!process_pe_field_simple (file, NULL,
                "CheckSum",
                NULL,
                HEADER_DATA_COLOR_2, 4, NULL))
            return;

        /* Subsystem */
        guint32 subsystem_values[] = { 0x0, 0x1, 0x2, 0x3, 0x5, 0x7, 0x8,
            0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0x10 };
        const gchar *subsystem_value_description[] = {
              "Unknown subsystem",
              "Device drivers and native Windows processes",
              "The Windows graphical user interface (GUI) subsystem",
              "The Windows character subsystem",
              "The OS/2 character subsystem",
              "The Posix character subsystem",
              "Native Win9x driver",
              "Windows CE",
              "An Extensible Firmware Interface (EFI) application",
              "An EFI driver with boot services",
              "An EFI driver with run-time services",
              "An EFI ROM image",
              "XBOX",
              "Windows boot application",
              "<span foreground=\"red\">Unknown</span>"
        };
        if (!process_pe_field (file, NULL,
                  "Subsystem", NULL,
                  "Subsystem\n"
                  "<tt>00 00<sub>16</sub></tt>\tUnknown subsystem\n"
                  "<tt>00 01<sub>16</sub></tt>\tDevice drivers and native Windows processes\n"
                  "<tt>00 02<sub>16</sub></tt>\tThe Windows graphical user interface (GUI) subsystem\n"
                  "<tt>00 03<sub>16</sub></tt>\tThe Windows character subsystem\n"
                  "<tt>00 05<sub>16</sub></tt>\tThe OS/2 character subsystem\n"
                  "<tt>00 07<sub>16</sub></tt>\tThe Posix character subsystem\n"
                  "<tt>00 08<sub>16</sub></tt>\tNative Win9x driver\n"
                  "<tt>00 09<sub>16</sub></tt>\tWindows CE\n"
                  "<tt>00 0A<sub>16</sub></tt>\tAn Extensible Firmware Interface (EFI) application\n"
                  "<tt>00 0B<sub>16</sub></tt>\tAn EFI driver with boot services\n"
                  "<tt>00 0C<sub>16</sub></tt>\tAn EFI driver with run-time services\n"
                  "<tt>00 0D<sub>16</sub></tt>\tAn EFI ROM image\n"
                  "<tt>00 0E<sub>16</sub></tt>\tXBOX\n"
                  "<tt>00 10<sub>16</sub></tt>\tWindows boot application",
                HEADER_DATA_COLOR_1, 2,
                G_N_ELEMENTS (subsystem_values), subsystem_values, subsystem_value_description,
                NULL, NULL))
            return;

        /* DllCharacteristics */
        if (!format_utils_read (file, &characteristics, 2))
            return;

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 2,
                                "DllCharacteristics", NULL);

        characteristics_string = g_string_new (NULL);
        if (characteristics & 0x20)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA");
        if (characteristics & 0x40)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE");
        if (characteristics & 0x80)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY");
        if (characteristics & 0x100)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_NX_COMPAT");
        if (characteristics & 0x200)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_NO_ISOLATION");
        if (characteristics & 0x400)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_NO_SEH");
        if (characteristics & 0x800)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_NO_BIND");
        if (characteristics & 0x1000)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_APPCONTAINER");
        if (characteristics & 0x2000)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_WDM_DRIVER");
        if (characteristics & 0x4000)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_GUARD_CF");
        if (characteristics & 0x8000)
            g_string_append_printf (characteristics_string, "%s\n", "IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE");

        if (characteristics_string->len)
        {
            g_string_truncate (characteristics_string, characteristics_string->len - 1);
            format_utils_add_line (file, "DllCharacteristics", characteristics_string->str,
                   "DllCharacteristics flags (bit mask)\n"
                   "IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA (<tt>20<sub>16</sub></tt>): Image can handle a high entropy 64-bit virtual address space\n"
                   "IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE (<tt>40<sub>16</sub></tt>): DLL can be relocated at load time\n"
                   "IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY (<tt>80<sub>16</sub></tt>): Code Integrity checks are enforced\n"
                   "IMAGE_DLLCHARACTERISTICS_NX_COMPAT (<tt>100<sub>16</sub></tt>): Image is NX compatible\n"
                   "IMAGE_DLLCHARACTERISTICS_NO_ISOLATION (<tt>200<sub>16</sub></tt>): Isolation aware, but do not isolate the image\n"
                   "IMAGE_DLLCHARACTERISTICS_NO_SEH (<tt>400<sub>16</sub></tt>): Does not use structured exception (SE) handling\n"
                   "IMAGE_DLLCHARACTERISTICS_NO_BIND (<tt>800<sub>16</sub></tt>): Do not bind the image\n"
                   "IMAGE_DLLCHARACTERISTICS_APPCONTAINER (<tt>1000<sub>16</sub></tt>): Image must execute in an AppContainer\n"
                   "IMAGE_DLLCHARACTERISTICS_WDM_DRIVER (<tt>2000<sub>16</sub></tt>): A WDM driver\n"
                   "IMAGE_DLLCHARACTERISTICS_GUARD_CF (<tt>4000<sub>16</sub></tt>): Image supports Control Flow Guard\n"
                   "IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE (<tt>8000<sub>16</sub></tt>): Terminal Server aware");
        }
        g_string_free (characteristics_string, TRUE);

        /* SizeOfStackReserve */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfStackReserve",
                _("The size of the stack to reserve"),
                HEADER_DATA_COLOR_1, field_length, "%lu"))
            return;

        /* SizeOfStackCommit */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfStackCommit",
                _("The size of the stack to commit"),
                HEADER_DATA_COLOR_2, field_length, "%lu"))
            return;

        /* SizeOfHeapReserve */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfHeapReserve",
                _("The size of the local heap space to reserve"),
                HEADER_DATA_COLOR_1, field_length, "%lu"))
            return;

        /* SizeOfHeapCommit */
        if (!process_pe_field_simple (file, NULL,
                "SizeOfHeapCommit",
                _("The size of the local heap space to commit"),
                HEADER_DATA_COLOR_2, field_length, "%lu"))
            return;

        /* LoaderFlags */
        if (!process_pe_field_simple (file, NULL,
                "LoaderFlags",
                NULL,
                HEADER_DATA_COLOR_1, 4, NULL))
            return;

        /* NumberOfRvaAndSizes */
        if (!process_pe_field (file, NULL,
                "NumberOfRvaAndSizes", NULL,
                _("The number of data-directory entries in the remainder of the optional header"),
                HEADER_DATA_COLOR_2, 4,
                0, NULL, NULL,
                "%u", &data_directory_entries))
            return;

        if (sym_table_offset)
        {
            /* Symbol table length, 18 bytes per record */
            table_length *= 18;

            save_index = GET_INDEX (file);

            SET_INDEX (file, sym_table_offset);

            /* String table offset */
            sym_table_offset += table_length;

            format_utils_add_field_full (file, HEADER_DATA_COLOR_1, TRUE, table_length,
                                         "Symbol table", "Sym.", START_SECTION_DATA_COLOR);

            /* String table length */
            if (format_utils_read (file, &table_length, 4))
            {
                format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4,
                                        "String table size", "Str.");

                if (table_length >= 4)
                    table_length -= 4;
                else
                    table_length = 0;

                format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, table_length,
                                        "String table", NULL);
            }

            SET_INDEX (file, save_index);
        }

        pe_data_directories (file, data_directory_entries);

        pe_section_table (file, sections, sym_table_offset);
    }

}

gboolean
process_pe_field (FormatsFile    *file,
                  DescriptionTab *tab,
                  const gchar    *field_name,
                  const gchar    *field_name_tag,
                  const gchar    *field_tooltip,
                  gint            color_index,
                  guint           field_length,
                  guint           possible_values,
                  guint32        *field_values,
                  const gchar   **value_descriptions,
                  const gchar    *field_format,
                  gpointer        read_value)
{
    const gchar *field_description = NULL;
    guint64 eight_bytes = 0;

    if (!format_utils_read (file, &eight_bytes, field_length))
        return FALSE;

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
        field_description = g_strdup_printf (field_format, eight_bytes);
        if (tab)
            format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
        else
            format_utils_add_line (file, field_name, field_description, field_tooltip);
        g_free ((gpointer) field_description);
    }

    if (read_value)
    {
        if (field_length == 2)
            *(guint16 *) read_value = eight_bytes;
        else if (field_length == 4)
            *(guint32 *) read_value = eight_bytes;
    }

    return TRUE;
}
