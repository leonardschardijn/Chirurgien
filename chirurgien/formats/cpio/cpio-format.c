/* cpio-format.c
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

#include "cpio-format.h"

#include <glib/gi18n.h>

#include "chirurgien-cpio.h"


static gboolean    process_cpio_field       (FormatsFile *,
                                             const gchar *,
                                             const gchar *,
                                             gint,
                                             guint,
                                             gboolean,
                                             guint64 *,
                                             CpioFormat);
static gboolean    process_file_type_mode   (FormatsFile *,
                                             guint,
                                             CpioFormat);
static void        build_final_header       (gchar *,
                                             CpioFormat);


void
chirurgien_cpio (FormatsFile *file,
                 CpioFormat   cpio_format)
{
    const gchar *file_name;
    guint64 file_name_size, file_size;
    guint file_padding;

    gchar final_header[121];

    if (cpio_format == BinaryLittleEndian)
        format_utils_set_title (file, _("Little-endian binary cpio archive"));
    else if (cpio_format == BinaryBigEndian)
        format_utils_set_title (file, _("Big-endian binary cpio archive"));
    else if (cpio_format == OldASCII)
        format_utils_set_title (file, _("ASCII-encoded octal cpio archive"));
    else if (cpio_format == NewASCII)
        format_utils_set_title (file, _("ASCII-encoded hexadecimal cpio archive"));
    else if (cpio_format == NewCRC)
        format_utils_set_title (file, _("ASCII-encoded hexadecimal CRC cpio archive"));

    build_final_header (final_header, cpio_format);

    while (FILE_HAS_DATA (file))
    {
        if (cpio_format == BinaryLittleEndian ||
            cpio_format == BinaryBigEndian)
        {
            if (FILE_HAS_DATA_N (file, BINARY_HEADER_SIZE) &&
                !memcmp (final_header, GET_CONTENT_POINTER (file), BINARY_HEADER_SIZE))
            {
                format_utils_add_field (file, FINAL_HEADER_COLOR, TRUE, BINARY_HEADER_SIZE,
                                      _("Special final header"), _("End"));
                break;
            }

            format_utils_start_section (file, _("File"));

            /* Magic number */
            if (!process_cpio_field (file, _("Magic number"), NULL, HEADER_DATA_COLOR_1, 2,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* Device number */
            if (!process_cpio_field (file, _("Device number"), NULL, HEADER_DATA_COLOR_2, 2,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* inode number */
            if (!process_cpio_field (file, _("inode number"), NULL, HEADER_DATA_COLOR_1, 2,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File mode */
            if (!process_file_type_mode (file, 2, cpio_format))
                goto END_ERROR;

            /* User ID */
            if (!process_cpio_field (file, _("User ID"), NULL, HEADER_DATA_COLOR_1, 2,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Group ID */
            if (!process_cpio_field (file, _("Group ID"), NULL, HEADER_DATA_COLOR_2, 2,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Number of links */
            if (!process_cpio_field (file, _("Number of links"), NULL, HEADER_DATA_COLOR_1, 2,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Character/block special file device number */
            if (!process_cpio_field (file, _("Character/block special file device number"), NULL, HEADER_DATA_COLOR_2, 2,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* Last modification time */
            if (!process_cpio_field (file, _("Last modification time"), NULL, HEADER_DATA_COLOR_1, 4,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File name size */
            if (!process_cpio_field (file, _("File name size"), _("The file name size, in bytes"), HEADER_DATA_COLOR_2, 2,
                                     TRUE, &file_name_size, cpio_format))
                goto END_ERROR;

            /* File size */
            if (!process_cpio_field (file, _("File size"), _("The file size, in bytes"), HEADER_DATA_COLOR_1, 4,
                                     TRUE, &file_size, cpio_format))
                goto END_ERROR;
        }
        else if (cpio_format == OldASCII)
        {
            if (FILE_HAS_DATA_N (file, OLD_ASCII_HEADER_SIZE) &&
                !memcmp (final_header, GET_CONTENT_POINTER (file), OLD_ASCII_HEADER_SIZE))
            {
                format_utils_add_field (file, FINAL_HEADER_COLOR, TRUE, OLD_ASCII_HEADER_SIZE,
                                      _("Special final header"), _("End"));
                break;
            }

            format_utils_start_section (file, _("File"));

            /* Magic number */
            if (!process_cpio_field (file, _("Magic number"), NULL, HEADER_DATA_COLOR_1, 6,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* Device number */
            if (!process_cpio_field (file, _("Device number"), NULL, HEADER_DATA_COLOR_2, 6,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* inode number */
            if (!process_cpio_field (file, _("inode number"), NULL, HEADER_DATA_COLOR_1, 6,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File mode */
            if (!process_file_type_mode (file, 6, cpio_format))
                goto END_ERROR;

            /* User ID */
            if (!process_cpio_field (file, _("User ID"), NULL, HEADER_DATA_COLOR_2, 6,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Group ID */
            if (!process_cpio_field (file, _("Group ID"), NULL, HEADER_DATA_COLOR_1, 6,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Number of links */
            if (!process_cpio_field (file, _("Number of links"), NULL, HEADER_DATA_COLOR_2, 6,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Character/block special file device number */
            if (!process_cpio_field (file, _("Character/block special file device number"), NULL, HEADER_DATA_COLOR_1, 6,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* Last modification time */
            if (!process_cpio_field (file, _("Last modification time"), NULL, HEADER_DATA_COLOR_2, 11,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File name size */
            if (!process_cpio_field (file, _("File name size"), _("The file name size, in bytes"), HEADER_DATA_COLOR_1, 6,
                                     TRUE, &file_name_size, cpio_format))
                goto END_ERROR;

            /* File size */
            if (!process_cpio_field (file, _("File size"), _("The file size, in bytes"), HEADER_DATA_COLOR_2, 11,
                                     TRUE, &file_size, cpio_format))
                goto END_ERROR;
        }
        else // NewASCII || NewCRC
        {
            if (FILE_HAS_DATA_N (file, NEW_ASCII_HEADER_SIZE) &&
                !memcmp (final_header, GET_CONTENT_POINTER (file), NEW_ASCII_HEADER_SIZE))
            {
                format_utils_add_field (file, FINAL_HEADER_COLOR, TRUE, NEW_ASCII_HEADER_SIZE,
                                      _("Special final header"), _("End"));
                break;
            }

            format_utils_start_section (file, _("File"));

            /* Magic number */
            if (!process_cpio_field (file, _("Magic number"), NULL, HEADER_DATA_COLOR_1, 6,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* inode number */
            if (!process_cpio_field (file, _("inode number"), NULL, HEADER_DATA_COLOR_2, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File mode */
            if (!process_file_type_mode (file, 8, cpio_format))
                goto END_ERROR;

            /* User ID */
            if (!process_cpio_field (file, _("User ID"), NULL, HEADER_DATA_COLOR_2, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Group ID */
            if (!process_cpio_field (file, _("Group ID"), NULL, HEADER_DATA_COLOR_1, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Number of links */
            if (!process_cpio_field (file, _("Number of links"), NULL, HEADER_DATA_COLOR_2, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Last modification time */
            if (!process_cpio_field (file, _("Last modification time"), NULL, HEADER_DATA_COLOR_1, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* File size */
            if (!process_cpio_field (file, _("File size"), _("The file size, in bytes"), HEADER_DATA_COLOR_2, 8,
                                     TRUE, &file_size, cpio_format))
                goto END_ERROR;

            /* Device major number */
            if (!process_cpio_field (file, _("Device major number"), NULL, HEADER_DATA_COLOR_1, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Device minor number */
            if (!process_cpio_field (file, _("Device minor number"), NULL, HEADER_DATA_COLOR_2, 8,
                                     TRUE, NULL, cpio_format))
                goto END_ERROR;

            /* Character/block special file device major number */
            if (!process_cpio_field (file, _("Character/block special file device major number"), NULL, HEADER_DATA_COLOR_1, 8,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* Character/block special file device minor number */
            if (!process_cpio_field (file, _("Character/block special file device minor number"), NULL, HEADER_DATA_COLOR_2, 8,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;

            /* File name size */
            if (!process_cpio_field (file, _("File name size"), _("The file name size, in bytes"), HEADER_DATA_COLOR_1, 8,
                                     TRUE, &file_name_size, cpio_format))
                goto END_ERROR;

            /* Use the file_name variable because it is available */
            if (cpio_format == NewASCII)
                file_name = _("Unused");
            else
                file_name = _("Checksum");

            /* Unused / Checksum */
            if (!process_cpio_field (file, file_name, NULL, HEADER_DATA_COLOR_2, 8,
                                     FALSE, NULL, cpio_format))
                goto END_ERROR;
        }

        /* File name */
        if (file_name_size)
        {
            if (FILE_HAS_DATA_N (file, file_name_size))
            {
                file_name = (const gchar *) GET_CONTENT_POINTER (file);

                format_utils_add_field (file, FILE_NAME_COLOR, TRUE, file_name_size,
                                      _("File name"), NULL);

                /* Padding */
                if (cpio_format == BinaryLittleEndian || cpio_format == BinaryBigEndian)
                {
                    if (file_name_size % 2)
                    {
                        if (FILE_HAS_DATA (file))
                            format_utils_add_field (file, PADDING_COLOR, FALSE, 1,
                                                  _("File name padding"), NULL);
                        else
                            return;
                    }
                }
                else if (cpio_format == NewASCII || cpio_format == NewCRC)
                {
                    file_padding = 4 - (file_name_size + 110) % 4;

                    if (file_padding != 4)
                    {
                        if (FILE_HAS_DATA_N (file, file_padding))
                        {
                            format_utils_add_field (file, PADDING_COLOR, FALSE, file_padding,
                                                  _("File name padding"), NULL);
                        }
                        else
                        {
                            format_utils_add_field (file, ERROR_COLOR, FALSE, G_MAXUINT,
                                                  _("Incomplete file name padding"), NULL);
                            return;
                        }
                    }
                }

                file_name_size--;
                if (file_name[file_name_size] != '\0' || !g_utf8_validate (file_name, file_name_size, NULL))
                    file_name = "";
            }
            else
            {
                format_utils_add_field (file, ERROR_COLOR, FALSE, G_MAXUINT, _("Incomplete file name"), NULL);
                return;
            }
        }
        else
        {
            file_name = "";
        }

        format_utils_add_line (file, _("File name"), file_name, NULL);

        /* File contents */
        if (file_size)
        {
            if (FILE_HAS_DATA_N (file, file_size))
            {
                format_utils_add_field (file, FILE_CONTENTS_COLOR, TRUE, file_size, _("File contents"), NULL);

                /* Padding */
                if (cpio_format == BinaryLittleEndian || cpio_format == BinaryBigEndian)
                {
                    if (file_size % 2)
                    {
                        if (FILE_HAS_DATA (file))
                            format_utils_add_field (file, PADDING_COLOR, FALSE, 1,
                                                  _("File contents padding"), NULL);
                        else
                            return;
                    }
                }
                if (cpio_format == NewASCII || cpio_format == NewCRC)
                {
                    file_padding = 4 - file_size % 4;

                    if (file_padding != 4)
                    {
                        if (FILE_HAS_DATA_N (file, file_padding))
                        {
                            format_utils_add_field (file, PADDING_COLOR, FALSE, file_padding,
                                                  _("File contents padding"), NULL);
                        }
                        else
                        {
                            format_utils_add_field (file, ERROR_COLOR, FALSE, G_MAXUINT,
                                                  _("Incomplete file contents padding"), NULL);
                            return;
                        }
                    }
                }
            }
            else
            {
                format_utils_add_field (file, ERROR_COLOR, FALSE, G_MAXUINT,
                                      _("Incomplete file contents"), NULL);
                return;
            }
        }
    }

    if (FILE_HAS_DATA (file))
        format_utils_add_field (file, PADDING_COLOR, FALSE, G_MAXUINT,
                              _("Archive padding"), NULL);
    return;

    END_ERROR:
    format_utils_add_field (file, ERROR_COLOR, FALSE, G_MAXUINT,
                          _("Unrecognized data"), NULL);
}

static gboolean
process_cpio_field (FormatsFile *file,
                    const gchar *field_name,
                    const gchar *field_tooltip,
                    gint         color_index,
                    guint        field_length,
                    gboolean     print_field,
                    guint64     *read_value,
                    CpioFormat   cpio_format)
{
    gchar *field_description = NULL;
    gboolean free_string = FALSE;

    GDateTime *date;
    guint64 numeric_value = 0;

    gchar cpio_field[12];

    if (!format_utils_read (file, cpio_field, field_length))
        return FALSE;

    if (!g_strcmp0 (_("Magic number"), field_name))
        format_utils_add_field (file, color_index, TRUE, field_length, field_name, _("File"));
    else
        format_utils_add_field (file, color_index, TRUE, field_length, field_name, NULL);

    if (print_field)
    {
        if (cpio_format == BinaryLittleEndian)
        {
            if (field_length == 2)
            {
                numeric_value = *(guint16 *) cpio_field;
            }
            else
            {
                numeric_value = (*(guint16 *) cpio_field) << 16;
                numeric_value += *(guint16 *) (cpio_field + 2);
            }
        }
        else if (cpio_format == BinaryBigEndian)
        {
            if (field_length == 2)
            {
                numeric_value = g_ntohs (*(guint16 *) cpio_field);
            }
            else
            {
                numeric_value = g_ntohs ((*(guint16 *) cpio_field)) << 16;
                numeric_value += g_ntohs (*(guint16 *) (cpio_field + 2));
            }
        }
        else if (*cpio_field != '\0')
        {
            cpio_field[field_length] = '\0';

            if (cpio_format == OldASCII)
                numeric_value = g_ascii_strtoull (cpio_field, NULL, 8);
            else // NewASCII || NewCRC
                numeric_value = g_ascii_strtoull (cpio_field, NULL, 16);
        }
        else
        {
            field_description = _("n/a");
        }

        if (field_description == NULL)
        {
            if (!g_strcmp0 (_("Last modification time"), field_name))
            {
                date = g_date_time_new_from_unix_utc (numeric_value);

                field_description = g_date_time_format_iso8601 (date);

                g_date_time_unref (date);
            }
            else
            {
                field_description = g_strdup_printf ("%lu", numeric_value);
            }
            free_string = TRUE;
        }

        format_utils_add_line (file, field_name, field_description, field_tooltip);
    }

    if (read_value)
        *read_value = numeric_value;

    if (free_string)
        g_free (field_description);

    return TRUE;
}

static gboolean
process_file_type_mode (FormatsFile *file,
                        guint        field_length,
                        CpioFormat   cpio_format)
{
    gchar file_type_mode[9];

    const gchar *value;
    const gchar *file_type_tooltip;

    guint64 numeric_value;

    if (!format_utils_read (file, &file_type_mode, field_length))
        return FALSE;

    if (cpio_format == OldASCII)
    {
        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 2, _("File type"), NULL);
        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4, _("File mode"), NULL);

        file_type_tooltip = _("File type\n"
                              "<tt>30 31<sub>16</sub></tt>\tFIFO special file\n"
                              "<tt>30 32<sub>16</sub></tt>\tCharacter special file\n"
                              "<tt>30 34<sub>16</sub></tt>\tDirectory\n"
                              "<tt>30 36<sub>16</sub></tt>\tBlock special file\n"
                              "<tt>31 30<sub>16</sub></tt>\tRegular file\n"
                              "<tt>31 32<sub>16</sub></tt>\tSymbolic link\n"
                              "<tt>31 34<sub>16</sub></tt>\tSocket");
    }
    else if (cpio_format == BinaryLittleEndian ||
             cpio_format == BinaryBigEndian)
    {
        /* Convert to octal representation, so its treatement is the same as in the OldASCII format */
        if (cpio_format == BinaryLittleEndian)
            numeric_value = *(guint16*) file_type_mode;
        else
            numeric_value = g_ntohs (*(guint16*) file_type_mode);
        g_snprintf (file_type_mode, 7, "%6.6lo", numeric_value);

        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 2, _("File mode and type"), NULL);

        file_type_tooltip = _("File type (upper 4 bits)\n"
                              "<tt>1<sub>16</sub></tt>\tFIFO special file\n"
                              "<tt>2<sub>16</sub></tt>\tCharacter special file\n"
                              "<tt>4<sub>16</sub></tt>\tDirectory\n"
                              "<tt>6<sub>16</sub></tt>\tBlock special file\n"
                              "<tt>8<sub>16</sub></tt>\tRegular file\n"
                              "<tt>A<sub>16</sub></tt>\tSymbolic link\n"
                              "<tt>C<sub>16</sub></tt>\tSocket");
    }
    else
    {
        /* Convert to octal representation, so its treatement is the same as in the OldASCII format */
        file_type_mode[field_length] = '\0';
        numeric_value = g_ascii_strtoull (file_type_mode, NULL, 16);
        g_snprintf (file_type_mode, 7, "%6.6lo", numeric_value);

        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 4, _("Unused"), NULL);
        format_utils_add_field (file, HEADER_DATA_COLOR_2, TRUE, 1, _("File type"), NULL);
        format_utils_add_field (file, HEADER_DATA_COLOR_1, TRUE, 3, _("File mode"), NULL);

        file_type_tooltip = _("File type\n"
                              "<tt>31<sub>16</sub></tt>\tFIFO special file\n"
                              "<tt>32<sub>16</sub></tt>\tCharacter special file\n"
                              "<tt>34<sub>16</sub></tt>\tDirectory\n"
                              "<tt>36<sub>16</sub></tt>\tBlock special file\n"
                              "<tt>38<sub>16</sub></tt>\tRegular file\n"
                              "<tt>41<sub>16</sub></tt>\tSymbolic link\n"
                              "<tt>43<sub>16</sub></tt>\tSocket");
    }

    if (!memcmp (file_type_mode, "01", 2))
        value = _("FIFO special file");
    else if (!memcmp (file_type_mode, "02", 2))
        value = _("Character special file");
    else if (!memcmp (file_type_mode, "04", 2))
        value = _("Directory");
    else if (!memcmp (file_type_mode, "06", 2))
        value = _("Block special file");
    else if (!memcmp (file_type_mode, "10", 2))
        value = _("Regular file");
    else if (!memcmp (file_type_mode, "12", 2))
        value = _("Symbolic link");
    else if (!memcmp (file_type_mode, "14", 2))
        value = _("Socket");
    else
        value = _("<span foreground=\"red\">INVALID</span>");

    format_utils_add_line (file, _("File type"), value, file_type_tooltip);

    file_type_mode[6] = '\0';
    if (g_utf8_validate (file_type_mode + 2, 4, NULL))
        value = file_type_mode + 2;
    else
        value = _("<span foreground=\"red\">INVALID</span>");

    format_utils_add_line (file, _("File mode"), value,
                         _("File mode in octal format"));

    return TRUE;
}

static void
build_final_header (gchar     *final_header,
                    CpioFormat cpio_format)
{
    if (cpio_format == BinaryLittleEndian ||
        cpio_format == BinaryBigEndian)
    {
        memset (final_header, 0, 26);
        if (cpio_format == BinaryLittleEndian)
        {
            final_header[0] = 0xC7;
            final_header[1] = 0x71;
            final_header[12] = 0x01;
            final_header[20] = 0x0B;
        }
        else
        {
            final_header[0] = 0x71;
            final_header[1] = 0xC7;
            final_header[13] = 0x01;
            final_header[21] = 0x0B;
        }
        g_snprintf (final_header + 26, 11, "TRAILER!!!");
    }
    else if (cpio_format == OldASCII)
    {
        memset (final_header, '0', 76);
        memcpy (final_header, "070707", 6);
        memcpy (final_header + 36, "000001", 6);
        memcpy (final_header + 59, "000013", 6);
        g_snprintf (final_header + 76, 11, "TRAILER!!!");
    }
    else
    {
        memset (final_header, '0', 110);
        if (cpio_format == NewASCII)
            memcpy (final_header, "070701", 6);
        else // NewCRC
            memcpy (final_header, "070702", 6);
        memcpy (final_header + 38, "00000001", 8);
        memcpy (final_header + 94, "0000000B", 8);
        g_snprintf (final_header + 110, 11, "TRAILER!!!");
    }
}
