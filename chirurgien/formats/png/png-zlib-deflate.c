/* png-zlib-deflate.c
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

#include <glib/gi18n.h>

#include "png-format.h"

#define MEBIBYTE 1048576


void
png_zlib_deflate (FormatsFile    *file,
                  DescriptionTab *tab,
                  const gchar    *zlib_data,
                  guint32         zlib_length,
                  guint          *real_zlib_length,
                  gchar         **inflate_data_out,
                  guint          *inflate_size_out,
                  gboolean       *compression_method_found,
                  gboolean       *flags_found,
                  gboolean       *decompression_success,
                  gboolean       *checksum_found)
{
    g_autoptr (GZlibDecompressor) decompressor = NULL;
    GConverterResult result;
    GError *error = NULL;
    gsize bytes_read, bytes_written, allocated_inflate_size,
          inflate_size, inflate_size_increment;

    guint8 compression_method, info_or_flags;

    const gchar *compression_method_value = NULL;
    g_autofree gchar *slidingwindow_size = NULL;
    const gchar *compression_level_value = NULL;
    const gchar *decompression_result = NULL;
    gchar *value;

    *compression_method_found = FALSE;
    *flags_found = FALSE;
    *decompression_success = FALSE;
    *checksum_found = FALSE;

    g_autofree gchar *inflate_data = NULL;

    if (!zlib_length)
        return;

    /* ZLIB compression method/info */
    compression_method = *zlib_data & 0x0F;
    info_or_flags = (*zlib_data & 0xF0) >> 4;

    zlib_data++;
    zlib_length--;
    *compression_method_found = TRUE;

    if (compression_method == 8) // DEFLATE
    {
        compression_method_value = "DEFLATE";

        if (info_or_flags > 7)
            slidingwindow_size = g_strdup (_("<span foreground=\"red\">INVALID</span>"));
        else
            slidingwindow_size = g_strdup_printf ("%u", 1 << (info_or_flags + 8));

        if (!zlib_length)
            goto END;

        /* ZLIB Flags */
        info_or_flags = (*zlib_data & 0xC0) >> 6;

        zlib_data++;
        zlib_length--;
        *flags_found = TRUE;

        switch (info_or_flags)
        {
            case 0:
                compression_level_value = _("Compressor used fastest algorithm");
                break;
            case 1:
                compression_level_value = _("Compressor used fast algorithm");
                break;
            case 2:
                compression_level_value = _("Compressor used default algorithm");
                break;
            case 3:
                compression_level_value = _("Compressor used maximum compression, slowest algorithm");
        }

        if (!zlib_length)
            goto END;

        /* Compressed data */
        decompressor = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_RAW);

        if (zlib_length < MEBIBYTE)
            allocated_inflate_size = inflate_size_increment = MEBIBYTE;
        else
            allocated_inflate_size = inflate_size_increment = zlib_length * 5;

        inflate_data = g_malloc (allocated_inflate_size);

        *decompression_success = TRUE;

        *real_zlib_length = inflate_size = bytes_read = bytes_written = 0;

        do
        {
            result = g_converter_convert (G_CONVERTER (decompressor),
                                          zlib_data + *real_zlib_length,
                                          zlib_length - *real_zlib_length,
                                          inflate_data + inflate_size,
                                          inflate_size_increment,
                                          G_CONVERTER_INPUT_AT_END | G_CONVERTER_FLUSH,
                                          &bytes_read,
                                          &bytes_written,
                                          &error);

            *real_zlib_length += bytes_read;
            inflate_size += bytes_written;

            if (result == G_CONVERTER_CONVERTED)
            {
                if (bytes_written == inflate_size_increment)
                {
                    allocated_inflate_size += inflate_size_increment;
                    inflate_data = g_realloc (inflate_data, allocated_inflate_size);
                }
            }

            if (result == G_CONVERTER_ERROR)
            {
                if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NO_SPACE))
                {
                    allocated_inflate_size += inflate_size_increment;
                    inflate_data = g_realloc (inflate_data, allocated_inflate_size);
                }
                else
                {
                    g_error_free (g_steal_pointer (&error));
                    *decompression_success = FALSE;
                    break;
                }

                g_error_free (g_steal_pointer (&error));
            }

        } while (result != G_CONVERTER_FINISHED &&
                 result != G_CONVERTER_FLUSHED);

        if (*decompression_success)
        {
            decompression_result = _("<span foreground=\"green\">SUCCESS</span>");

            if ((zlib_length - *real_zlib_length) >= 4)
                *checksum_found = TRUE;
        }
        else
        {
            decompression_result = _("<span foreground=\"red\">FAILED</span>");
        }
    }
    else
    {
        compression_method_value = _("<span foreground=\"red\">INVALID</span>");
    }

    END:
    if (inflate_data_out)
        *inflate_data_out = g_steal_pointer (&inflate_data);
    if (inflate_size_out)
        *inflate_size_out = inflate_size;

    if (!tab)
    {
        if (compression_method_value)
            format_utils_add_line (file, _("Compression method"), compression_method_value,
                                 _("Compression method (CM): Lower four bits of CMF\n"
                                   "Compression method\n"
                                   "<tt>8<sub>16</sub></tt>\tDEFLATE"));
        if (slidingwindow_size)
            format_utils_add_line (file, _("LZ77 sliding window size"), slidingwindow_size,
                         _("Compression info (CINFO): Upper four bits of CMF, only valid when CM = 8\n"
                           "Used to calculate the sliding window size as:\n"
                           "2<sup>CINFO + 8</sup> if CINFO &lt; 8"));
        if (compression_level_value)
            format_utils_add_line_full (file, _("Compression level"), compression_level_value,
                                      _("Compression level: upper two bits of FLG\n"
                                        "Compression levels\n"
                                        "<tt>0</tt>\tCompressor used fastest algorithm\n"
                                        "<tt>1</tt>\tCompressor used fast algorithm\n"
                                        "<tt>2</tt>\tCompressor used default algorithm\n"
                                        "<tt>3</tt>\tCompressor used maximum compression, slowest algorithm"),
                                        10, 0);
        if (decompression_result)
            format_utils_add_line_full (file, _("Decompression"), decompression_result,
                                        NULL, 10, 10);

        if (*decompression_success)
        {
            value = g_strdup_printf (_("%u B (%.2f MiB)"), (guint) inflate_size, (gfloat) inflate_size / MEBIBYTE);
            format_utils_add_line (file, _("Inflate size"), value,
                                 _("Size of the raw data"));
            g_free (value);

            value = g_strdup_printf (_("%u B (%.2f MiB)"), *real_zlib_length, (gfloat) *real_zlib_length / MEBIBYTE);
            format_utils_add_line (file, _("Deflate size"), value,
                                 _("Size of the compressed data"));
            g_free (value);

            value = g_strdup_printf ("%.2f", (gfloat) inflate_size / *real_zlib_length);
            format_utils_add_line (file, _("Compression ratio"), value, NULL);
            g_free (value);
        }
    }
    else
    {
        if (compression_method_value)
            format_utils_add_line_tab (tab, _("Compression method"), compression_method_value,
                                     _("Compression method (CM): Lower four bits of CMF\n"
                                       "Compression method\n"
                                       "<tt>8<sub>16</sub></tt>\tDEFLATE"));
        if (slidingwindow_size)
            format_utils_add_line_tab (tab, _("LZ77 sliding window size"), slidingwindow_size,
                         _("Compression info (CINFO): Upper four bits of CMF, only valid when CM = 8\n"
                           "Used to calculate the sliding window size as:\n"
                           "2<sup>CINFO + 8</sup> if CINFO &lt; 8"));
        if (compression_level_value)
            format_utils_add_line_full_tab (tab, _("Compression level"), compression_level_value,
                                      _("Compression level: upper two bits of FLG\n"
                                        "Compression levels\n"
                                        "<tt>0</tt>\tCompressor used fastest algorithm\n"
                                        "<tt>1</tt>\tCompressor used fast algorithm\n"
                                        "<tt>2</tt>\tCompressor used default algorithm\n"
                                        "<tt>3</tt>\tCompressor used maximum compression, slowest algorithm"),
                                        10, 0);
        if (decompression_result)
            format_utils_add_line_full_tab (tab, _("Decompression"), decompression_result,
                                            NULL, 10, 10);

        if (*decompression_success)
        {
            value = g_strdup_printf (_("%u B (%.2f MiB)"), (guint) inflate_size, (gfloat) inflate_size / MEBIBYTE);
            format_utils_add_line_tab (tab, _("Inflate size"), value,
                                 _("Size of the raw data"));
            g_free (value);

            value = g_strdup_printf (_("%u B (%.2f MiB)"), *real_zlib_length, (gfloat) *real_zlib_length / MEBIBYTE);
            format_utils_add_line_tab (tab, _("Deflate size"), value,
                                 _("Size of the compressed data"));
            g_free (value);

            value = g_strdup_printf ("%.2f", (gfloat) inflate_size / *real_zlib_length);
            format_utils_add_line_tab (tab, _("Compression ratio"), value, NULL);
            g_free (value);
        }
    }
}
