/* cpio-analyzer.c
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

#include "cpio-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-cpio.h"


static gboolean    process_cpio_field       (AnalyzerFile *,
                                             gchar *,
                                             gchar *,
                                             GdkRGBA *,
                                             guint,
                                             gboolean,
                                             guint64 *,
                                             gint);
static gboolean    process_file_type_mode   (AnalyzerFile *,
                                             guint,
                                             gint);
static void        build_final_header       (gchar *,
                                             gint);


void
chirurgien_analyze_cpio (AnalyzerFile *file,
                         gint cpio_format)
{
    gchar *file_name;
    guint64 file_name_size, file_size;
    guint file_padding;

    gchar final_header[121];

    if (cpio_format == BinaryLittleEndian)
        analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                          "Little-endian binary cpio archive"
                                          "</span>"));
    else if (cpio_format == BinaryBigEndian)
        analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                          "Big-endian binary cpio archive"
                                          "</span>"));
    else if (cpio_format == OldASCII)
        analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                          "ASCII-encoded octal cpio archive"
                                          "</span>"));
    else if (cpio_format == NewASCII)
        analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                          "ASCII-encoded hexadecimal cpio archive"
                                          "</span>"));
    else if (cpio_format == NewCRC)
        analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                          "ASCII-encoded hexadecimal CRC cpio archive"
                                          "</span>"));

    build_final_header (final_header, cpio_format);

    while (FILE_HAS_DATA (file))
    {
        if (cpio_format == BinaryLittleEndian ||
            cpio_format == BinaryBigEndian)
        {
            if (FILE_HAS_DATA_N (file, BINARY_HEADER_SIZE) &&
                !memcmp (file->file_contents + GET_POINTER (file), final_header, BINARY_HEADER_SIZE))
            {
                analyzer_utils_tag_navigation (file, FINAL_HEADER_COLOR, BINARY_HEADER_SIZE,
                                               _("Special final header"), _("End"));
                break;
            }

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
                !memcmp (file->file_contents + GET_POINTER (file), final_header, OLD_ASCII_HEADER_SIZE))
            {
                analyzer_utils_tag_navigation (file, FINAL_HEADER_COLOR, OLD_ASCII_HEADER_SIZE,
                                               _("Special final header"), _("End"));
                break;
            }

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
                !memcmp (file->file_contents + GET_POINTER (file), final_header, NEW_ASCII_HEADER_SIZE))
            {
                analyzer_utils_tag_navigation (file, FINAL_HEADER_COLOR, NEW_ASCII_HEADER_SIZE,
                                               _("Special final header"), _("End"));
                break;
            }

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
                file_name = (gchar *) file->file_contents + GET_POINTER (file);

                analyzer_utils_tag (file, FILE_NAME_COLOR, file_name_size, _("File name"));
                ADVANCE_POINTER (file, file_name_size);

                /* Padding */
                if (cpio_format == BinaryLittleEndian || cpio_format == BinaryBigEndian)
                {
                    if (file_name_size % 2)
                    {
                        if (FILE_HAS_DATA (file))
                        {
                            analyzer_utils_tag_error (file, PADDING_COLOR, 1, _("File name padding"));
                            ADVANCE_POINTER (file, 1);
                        }
                        else
                        {
                            return;
                        }
                    }
                }
                else if (cpio_format == NewASCII || cpio_format == NewCRC)
                {
                    file_padding = 4 - (file_name_size + 110) % 4;

                    if (file_padding != 4)
                    {
                        if (FILE_HAS_DATA_N (file, file_padding))
                        {
                            analyzer_utils_tag_error (file, PADDING_COLOR, file_padding, _("File name padding"));
                            ADVANCE_POINTER (file, file_padding);
                        }
                        else
                        {
                            analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete file name padding"));
                            return;
                        }
                    }
                }

                file_name_size--;
                if (*(file_name + file_name_size) != '\0' || !g_utf8_validate (file_name, file_name_size, NULL))
                    file_name = "";
            }
            else
            {
                analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete file name"));
                return;
            }
        }
        else
        {
            file_name = "";
        }

        analyzer_utils_add_description (file, _("File name"), file_name, NULL, 0, 10);

        /* File contents */
        if (file_size)
        {
            if (FILE_HAS_DATA_N (file, file_size))
            {
                analyzer_utils_tag (file, FILE_CONTENTS_COLOR, file_size, _("File contents"));
                ADVANCE_POINTER (file, file_size);

                /* Padding */
                if (cpio_format == BinaryLittleEndian || cpio_format == BinaryBigEndian)
                {
                    if (file_size % 2)
                    {
                        if (FILE_HAS_DATA (file))
                        {
                            analyzer_utils_tag_error (file, PADDING_COLOR, 1, _("File contents padding"));
                            ADVANCE_POINTER (file, 1);
                        }
                        else
                        {
                            return;
                        }
                    }
                }
                if (cpio_format == NewASCII || cpio_format == NewCRC)
                {
                    file_padding = 4 - file_size % 4;

                    if (file_padding != 4)
                    {
                        if (FILE_HAS_DATA_N (file, file_padding))
                        {
                            analyzer_utils_tag_error (file, PADDING_COLOR, file_padding, _("File contents padding"));
                            ADVANCE_POINTER (file, file_padding);
                        }
                        else
                        {
                            analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete file contents padding"));
                            return;
                        }
                    }
                }
            }
            else
            {
                analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete file contents"));
                return;
            }
        }
    }

    if (FILE_HAS_DATA (file))
        analyzer_utils_tag_error (file, PADDING_COLOR, -1, _("Archive padding"));
    return;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Unrecognized data"));
}

static gboolean
process_cpio_field (AnalyzerFile *file,
                    gchar *field_name,
                    gchar *field_tooltip,
                    GdkRGBA *color,
                    guint field_length,
                    gboolean print_field,
                    guint64* read_value,
                    gint cpio_format)
{
    gchar *description_message = NULL;
    gboolean free_string = FALSE;

    GDateTime *date;
    guint64 numeric_value = 0;

    gchar cpio_field[12];

    if (!analyzer_utils_read (cpio_field, file, field_length))
        return FALSE;

    if (!g_strcmp0 (_("Magic number"), field_name))
        analyzer_utils_tag_navigation (file, color, field_length, field_name, _("File"));
    else
        analyzer_utils_tag (file, color, field_length, field_name);

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
            description_message = _("n/a");
        }

        if (description_message == NULL)
        {
            if (!g_strcmp0 (_("Last modification time"), field_name))
            {
                date = g_date_time_new_from_unix_utc (numeric_value);

                description_message = g_date_time_format_iso8601 (date);

                g_date_time_unref (date);
            }
            else
            {
                description_message = g_strdup_printf ("%lu", numeric_value);
            }
            free_string = TRUE;
        }

        analyzer_utils_describe_tooltip (file, field_name, description_message, field_tooltip);
    }

    if (read_value)
        *read_value = numeric_value;

    if (free_string)
        g_free (description_message);

    return TRUE;
}

static gboolean
process_file_type_mode (AnalyzerFile *file,
                        guint field_length,
                        gint cpio_format)
{
    gchar file_type_mode[9];
    gchar *description_message, *file_type_tooltip;

    guint64 numeric_value;

    if (!analyzer_utils_read (&file_type_mode, file, field_length))
        return FALSE;

    if (cpio_format == OldASCII)
    {
        analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 2, _("File type"));
        analyzer_utils_tag (file, HEADER_DATA_COLOR_1, 4, _("File mode"));

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

        analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 2, _("File mode and type"));

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

        analyzer_utils_tag (file, HEADER_DATA_COLOR_1, 4, _("Unused"));
        analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 1, _("File type"));
        analyzer_utils_tag (file, HEADER_DATA_COLOR_1, 3, _("File mode"));

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
        description_message = _("FIFO special file");
    else if (!memcmp (file_type_mode, "02", 2))
        description_message = _("Character special file");
    else if (!memcmp (file_type_mode, "04", 2))
        description_message = _("Directory");
    else if (!memcmp (file_type_mode, "06", 2))
        description_message = _("Block special file");
    else if (!memcmp (file_type_mode, "10", 2))
        description_message = _("Regular file");
    else if (!memcmp (file_type_mode, "12", 2))
        description_message = _("Symbolic link");
    else if (!memcmp (file_type_mode, "14", 2))
        description_message = _("Socket");
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip (file, _("File type"), description_message,
                                     file_type_tooltip);

    file_type_mode[6] = '\0';
    if (g_utf8_validate (file_type_mode + 2, 4, NULL))
        description_message = file_type_mode + 2;
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip (file, _("File mode"), description_message,
                                     _("File mode in octal format"));

    return TRUE;
}

static void
build_final_header (gchar *final_header,
                    gint cpio_format)
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
        g_snprintf (final_header + 26, 11, "%s", "TRAILER!!!");
    }
    else if (cpio_format == OldASCII)
    {
        memset (final_header, '0', 76);
        memcpy (final_header, "070707", 6);
        memcpy (final_header + 36, "000001", 6);
        memcpy (final_header + 59, "000013", 6);
        g_snprintf (final_header + 76, 11, "%s", "TRAILER!!!");
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
        g_snprintf (final_header + 110, 11, "%s", "TRAILER!!!");
    }
}
