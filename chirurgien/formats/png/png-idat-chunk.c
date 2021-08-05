/* png-idat-chunk.c
 *
 * Copyright (C) 2020 - Daniel Léonard Schardijn
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

#include "png-format.h"


static void
idat_chunk_field (IdatChunks  *idat_chunks,
                  FormatsFile *file,
                  gint         color_index,
                  gboolean     background,
                  gsize        count,
                  const gchar *field_name)
{
    gsize chunk_available, save_index;

    chunk_available = idat_chunks->current_chunk_length - idat_chunks->chunk_used;

    save_index = GET_INDEX (file);
    SET_INDEX (file, idat_chunks->current_chunk_index + idat_chunks->chunk_used);

    if (count <= chunk_available)
    {
        format_utils_add_field (file, color_index, background, count, field_name, NULL);
        SET_INDEX (file, save_index);

        idat_chunks->chunk_used += count;
    }
    else
    {
        format_utils_add_field (file, color_index, background, chunk_available, field_name, NULL);
        SET_INDEX (file, save_index);

        if (idat_chunks->chunks)
        {
            /* Assign pair and advance list */
            idat_chunks->current_chunk_index = GPOINTER_TO_SIZE (idat_chunks->chunks->data);
            idat_chunks->current_chunk_length = GPOINTER_TO_SIZE (idat_chunks->chunks->next->data);
            idat_chunks->chunks = idat_chunks->chunks->next->next;
            idat_chunks->chunk_used = 0;
        }
        idat_chunk_field (idat_chunks, file, color_index, background, count - chunk_available, field_name);
    }
}

gboolean
collect_idat_chunk (FormatsFile *file,
                    guint32      chunk_length,
                    gint        *chunk_counts,
                    IdatChunks  *idat_data)
{
    if (!chunk_length)
        return TRUE;

    chunk_counts[IDAT]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    if (FILE_HAS_DATA_N (file, chunk_length))
    {
        idat_data->chunks = g_slist_append (idat_data->chunks,
                                            GSIZE_TO_POINTER (GET_INDEX (file)));
        idat_data->chunks = g_slist_append (idat_data->chunks,
                                            GSIZE_TO_POINTER (chunk_length));
        idat_data->compressed_image_size += chunk_length;

        ADVANCE_INDEX (file, chunk_length);
        return TRUE;
    }

    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                            "Chunk length exceeds available data", NULL);
    return FALSE;
}

void
png_idat_chunk (FormatsFile *file,
                IdatChunks  *idat_data,
                guint8       compression_method)
{
    g_autofree gchar *compressed_data = NULL;
    gchar *compressed_data_pointer;

    guint deflate_size;

    gboolean compression_method_found, flags_found,
             decompression_success, checksum_found;

    if (idat_data->compressed_image_size)
    {
        compressed_data = g_malloc (idat_data->compressed_image_size);

        compressed_data_pointer = compressed_data;
        for (GSList *i = idat_data->chunks; i != NULL; i = i->next->next)
        {
            memmove (compressed_data_pointer,
                     file->file_contents + GPOINTER_TO_SIZE (i->data),
                     GPOINTER_TO_SIZE (i->next->data));

            compressed_data_pointer += GPOINTER_TO_SIZE (i->next->data);
        }

        /* Assign first pair and advance list */
        idat_data->current_chunk_index = GPOINTER_TO_SIZE (idat_data->chunks->data);
        idat_data->current_chunk_length = GPOINTER_TO_SIZE (idat_data->chunks->next->data);
        idat_data->chunks = idat_data->chunks->next->next;
        idat_data->chunk_used = 0;

        if (!compression_method) /* zlib-format DEFLATE */
        {
            format_utils_start_section (file, "ZLIB compressed image");

            png_zlib_deflate (file,
                              NULL,
                              compressed_data,
                              idat_data->compressed_image_size,
                              &deflate_size,
                              NULL,
                              NULL,
                              &compression_method_found,
                              &flags_found,
                              &decompression_success,
                              &checksum_found);

            if (compression_method_found)
            {
                idat_chunk_field (idat_data, file, CHUNK_DATA_COLOR_1, TRUE, 1,
                                  "ZLIB compression method and flags (CMF)\n"
                                  "Lower four bits: compression method (CM)\n"
                                  "Upper four bits: compression info (CINFO)");
                idat_data->compressed_image_size--;

                if (flags_found)
                {
                    idat_chunk_field (idat_data, file, CHUNK_DATA_COLOR_2, TRUE, 1,
                                      "ZLIB flags (FLG)");
                    idat_data->compressed_image_size--;

                    if (decompression_success)
                    {
                        idat_chunk_field (idat_data, file, CHUNK_DATA_COLOR_1, TRUE, deflate_size,
                                          "ZLIB compressed image");
                        idat_data->compressed_image_size -= deflate_size;

                        if (checksum_found)
                        {
                            idat_chunk_field (idat_data, file, CHUNK_DATA_COLOR_2, TRUE, 4,
                                              "ZLIB Adler32 checksum");
                            idat_data->compressed_image_size -= 4;
                        }
                    }
                }
            }
        }

        /* If there is data left, tag it as unrecognized */
        idat_chunk_field (idat_data, file, ERROR_COLOR_1, FALSE,
                          idat_data->compressed_image_size, "Unrecognized data");
    }

    g_slist_free (idat_data->chunks);
}
