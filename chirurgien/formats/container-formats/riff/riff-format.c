/* riff-format.c
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

#include "riff-format.h"


void
chirurgien_riff (FormatsFile *file,
                 RiffFile    *riff_file)
{
    GString *chunk_field;

    guint32 riff_chunk_size, chunk_size;
    guchar chunk_fourcc[4];

    gboolean valid_chunk = FALSE;

    chunk_field = g_string_new ("Chunk FourCC: ");

    format_utils_add_field (file, CHUNK_FOURCC_COLOR, TRUE, 4,
                            "Chunk FourCC: RIFF", NULL);

    format_utils_read (file, &riff_chunk_size, 4);

    format_utils_add_field (file, CHUNK_SIZE_COLOR, TRUE, 4,
                            "Chunk size", NULL);

    /* The FourCC of the actual file format using the RIFF container */
    chunk_field = g_string_append (chunk_field, riff_file->chunk_types[0]);

    format_utils_add_field (file, CHUNK_FOURCC_COLOR, TRUE, 4,
                            chunk_field->str, riff_file->chunk_types[0]);

    chunk_field = g_string_truncate (chunk_field, chunk_field->len - 4);

    /* Chunk loop */
    while (FILE_HAS_DATA (file))
    {
        /* RIFF chunk size exceeded */
        if (!riff_chunk_size)
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                                    "Unrecognized data, RIFF chunk size exceeded", NULL);
            break;
        }

        /* Chunk FourCC */
        if (!format_utils_read (file, &chunk_fourcc, 4))
            break;

        valid_chunk = FALSE;

        for (guint i = 1; i < riff_file->chunk_type_count; i++)
        {
            if (!memcmp (chunk_fourcc, riff_file->chunk_types[i], 4))
            {
                riff_file->chunk_selection = i;
                valid_chunk = TRUE;

                chunk_field = g_string_append (chunk_field, riff_file->chunk_types[i]);

                format_utils_add_field (file, CHUNK_FOURCC_COLOR, TRUE, 4,
                                        chunk_field->str, riff_file->chunk_types[i]);

                chunk_field = g_string_truncate (chunk_field, chunk_field->len - 4);

                /* Chunk size */
                if (!format_utils_read (file, &chunk_size, 4))
                {
                    chunk_size = 0;
                    break;
                }

                format_utils_add_field (file, CHUNK_SIZE_COLOR, TRUE, 4,
                                        "Chunk size", NULL);

                if (riff_file->chunk_type_functions[i - 1])
                {
                    riff_file->chunk_type_functions[i - 1] (file, riff_file, chunk_size);
                }

                break;
            }
        }

        if (!valid_chunk)
        {
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 4,
                                    "Chunk FourCC: Unknown", "???");

            /* Chunk size */
            if (!format_utils_read (file, &chunk_size, 4))
                break;

            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 4,
                                    "Unknown chunk size", NULL);
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_size,
                                    "Unrecognized data", NULL);
        }

        if (chunk_size & 0x1)
            format_utils_add_field (file, PADDING_COLOR, FALSE, 1,
                                    "Padding", NULL);
    }

    /* If there is still data available after the loop, tag it as unrecognized */
    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                            "Unrecognized data", NULL);
}
