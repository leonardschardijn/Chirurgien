/* tar-analyzer.c
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

#include "tar-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-tar.h"


static gboolean    process_tar_field       (AnalyzerFile *,
                                            gchar *,
                                            gchar *,
                                            GdkRGBA *,
                                            guint,
                                            gboolean,
                                            gboolean,
                                            guint64 *);
static gboolean    process_file_type       (AnalyzerFile *);


void
chirurgien_analyze_tar (AnalyzerFile *file)
{
    guint64 file_size;
    guint file_padding;

    guchar empty_block[512];

    memset (empty_block, 0, 512);

    analyzer_utils_set_title (file, _("<span weight=\"bold\" size=\"larger\">"
                                      "tar archive"
                                      "</span>"));

    while (FILE_HAS_DATA (file))
    {
        if (FILE_HAS_DATA_N (file, 512) &&
            !memcmp (empty_block, file->file_contents + GET_POINTER(file), 512))
        {
            analyzer_utils_tag_navigation (file, EMPTY_BLOCK_COLOR, 512,
                                           _("Final empty block"), _("End"));
            break;
        }

        /* File name */
        if (!process_tar_field (file, _("File name"), NULL, HEADER_DATA_COLOR_1, 100,
                                FALSE, TRUE, NULL))
            goto END_ERROR;

        /* File mode */
        if (!process_tar_field (file, _("File mode"), _("File mode in octal format"),
                                HEADER_DATA_COLOR_2, 8, FALSE, TRUE, NULL))
            goto END_ERROR;

        /* User ID */
        if (!process_tar_field (file, _("User ID"), NULL, HEADER_DATA_COLOR_1, 8,
                                TRUE, TRUE, NULL))
            goto END_ERROR;

        /* Group ID */
        if (!process_tar_field (file, _("Group ID"), NULL, HEADER_DATA_COLOR_2, 8,
                                TRUE, TRUE, NULL))
            goto END_ERROR;

        /* File size */
        if (!process_tar_field (file, _("File size"), _("The file size, in bytes"),
                                HEADER_DATA_COLOR_1, 12, TRUE, TRUE, &file_size))
            goto END_ERROR;

        /* Last modification time */
        if (!process_tar_field (file, _("Last modification time"), NULL, HEADER_DATA_COLOR_2, 12,
                                TRUE, TRUE, NULL))
            goto END_ERROR;

        /* Header checksum */
        if (!process_tar_field (file, _("Header checksum"), NULL, HEADER_DATA_COLOR_1, 8,
                                FALSE, FALSE, NULL))
            goto END_ERROR;

        /* File type */
        if (!process_file_type (file))
            return;

        /* Linked file name */
        if (!process_tar_field (file, _("Linked file name"), _("Only relevant on hard/symbolic link type files"),
                                HEADER_DATA_COLOR_1, 100, FALSE, TRUE, NULL))
            goto END_ERROR;

        /* UStar identifier */
        if (!process_tar_field (file, _("UStar identifier"), NULL, HEADER_DATA_COLOR_2, 6,
                                FALSE, FALSE, NULL))
            goto END_ERROR;

        /* UStar version */
        if (!process_tar_field (file, _("UStar version"), NULL, HEADER_DATA_COLOR_1, 2,
                                FALSE, FALSE, NULL))
            goto END_ERROR;

        /* Owner user name */
        if (!process_tar_field (file, _("Owner user name"), NULL, HEADER_DATA_COLOR_2, 32,
                                FALSE, TRUE, NULL))
            goto END_ERROR;

        /* Owner group name */
        if (!process_tar_field (file, _("Owner group name"), NULL, HEADER_DATA_COLOR_1, 32,
                                FALSE, TRUE, NULL))
            goto END_ERROR;

        /* Device major number */
        if (!process_tar_field (file, _("Device major number"), NULL, HEADER_DATA_COLOR_2, 8,
                                TRUE, TRUE, NULL))
            goto END_ERROR;

        /* Device minor number */
        if (!process_tar_field (file, _("Device minor number"), NULL, HEADER_DATA_COLOR_1, 8,
                                TRUE, TRUE, NULL))
            goto END_ERROR;

        /* File name prefix */
        if (!process_tar_field (file, _("File name prefix"), NULL, HEADER_DATA_COLOR_2, 155,
                                FALSE, TRUE, NULL))
            goto END_ERROR;

        /* Header block padding */
        if (FILE_HAS_DATA_N (file, 12))
        {
            analyzer_utils_tag_error (file, PADDING_COLOR, 12, _("Header block padding"));
            ADVANCE_POINTER (file, 12);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete header block padding"));
            return;
        }

        /* File contents */
        if (file_size)
        {
            if (FILE_HAS_DATA_N (file, file_size))
            {
                analyzer_utils_tag (file, FILE_CONTENTS_COLOR, file_size, _("File contents"));
                ADVANCE_POINTER (file, file_size);
            }
            else
            {
                analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Incomplete file contents"));
                return;
            }
        }

        if (file_size % 512)
        {
            file_padding = 512 - file_size % 512;

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

    if (FILE_HAS_DATA (file))
        analyzer_utils_tag_error (file, PADDING_COLOR, -1, _("Archive padding"));
    return;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR, -1, _("Unrecognized data"));
}

static gboolean
process_tar_field (AnalyzerFile *file,
                   gchar *field_name,
                   gchar *field_tooltip,
                   GdkRGBA *color,
                   guint field_length,
                   gboolean is_numeric,
                   gboolean print_field,
                   guint64* read_value)
{
    gchar *description_message;
    gboolean free_string = FALSE;

    GDateTime *date;
    guint64 numeric_value;

    gchar tar_field[155];

    if (!analyzer_utils_read (tar_field, file, field_length))
        return FALSE;

    if (!g_strcmp0 (_("File name"), field_name))
        analyzer_utils_tag_navigation (file, color, field_length, field_name, _("File"));
    else
        analyzer_utils_tag (file, color, field_length, field_name);

    tar_field[field_length] = '\0';

    if (print_field)
    {
        if (!(*tar_field == '\0'))
        {
            if (is_numeric)
            {
                numeric_value = g_ascii_strtoull (tar_field, NULL, 8);

                if (!g_strcmp0 (_("Last modification time"), field_name))
                {
                    date = g_date_time_new_from_unix_utc (numeric_value);

                    description_message = g_date_time_format_iso8601 (date);
                    free_string = TRUE;

                    g_date_time_unref (date);
                }
                else
                {
                    description_message = g_strdup_printf ("%lu", numeric_value);
                    free_string = TRUE;
                }
            }
            else
            {
                if (g_utf8_validate (tar_field, -1, NULL))
                    description_message = tar_field;
                else
                    description_message = _("<span foreground=\"red\">INVALID ENCODING</span>");
            }
        }
        else
        {
            description_message = _("n/a");
        }

        if (!g_strcmp0 (_("File name prefix"), field_name))
            analyzer_utils_add_description (file, field_name, description_message, field_tooltip,
                                            0, 20);
        else
            analyzer_utils_describe_tooltip (file, field_name, description_message, field_tooltip);
    }

    if (read_value && is_numeric)
        *read_value = numeric_value;

    if (free_string)
        g_free (description_message);

    return TRUE;
}

static gboolean
process_file_type (AnalyzerFile *file)
{
    gchar file_type;
    gchar *description_message;

    if (!analyzer_utils_read (&file_type, file, 1))
        return FALSE;

    analyzer_utils_tag (file, HEADER_DATA_COLOR_2, 1, _("File type"));

    if (file_type == 0x0 || file_type == '0')
        description_message = _("Regular file");
    else if (file_type == '1')
        description_message = _("Hard link");
    else if (file_type == '2')
        description_message = _("Symbolic link");
    else if (file_type == '3')
        description_message = _("Character special file");
    else if (file_type == '4')
        description_message = _("Block special file");
    else if (file_type == '5')
        description_message = _("Directory");
    else if (file_type == '6')
        description_message = _("FIFO special file");
    else if (file_type == 'g')
        description_message = _("Global extended header");
    else if (file_type == 'x')
        description_message = _("Extended header");
    else if (file_type == '7' || (file_type > 'A' && file_type < 'Z'))
        description_message = _("Reserved");
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip (file, _("File type"), description_message,
                             _("File type\n"
                               "<tt>00<sub>16</sub></tt>\tRegular file\n"
                               "<tt>30<sub>16</sub></tt>\tRegular file\n"
                               "<tt>31<sub>16</sub></tt>\tHard link\n"
                               "<tt>32<sub>16</sub></tt>\tSymbolic link\n"
                               "<tt>33<sub>16</sub></tt>\tCharacter special file\n"
                               "<tt>34<sub>16</sub></tt>\tBlock special file\n"
                               "<tt>35<sub>16</sub></tt>\tDirectory\n"
                               "<tt>36<sub>16</sub></tt>\tFIFO special file\n"
                               "<tt>67<sub>16</sub></tt>\tGlobal extended header\n"
                               "<tt>78<sub>16</sub></tt>\tExtended header"));

    return TRUE;
}
