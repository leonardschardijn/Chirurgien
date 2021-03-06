/* png-ihdr-chunk.c
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

#include <config.h>

#include <glib/gi18n.h>

#include "png-format.h"

#define IHDR_CHUNK_LENGTH 13


gboolean
png_ihdr_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts,
                guint8      *colortype,
                guint8      *compression_method)
{
    const guint8 bitdepths_for_colortype_0[5] = { 1, 2, 4, 8, 16 };
    const guint8 bitdepths_for_colortype_2_4_6[2] = { 8, 16 };
    const guint8 bitdepths_for_colortype_3[4] = { 1, 2, 4, 8 };

    guint8 bitdepth;

    if (!chunk_length)
        return TRUE;

    chunk_counts[IHDR]++;

    if (chunk_counts[IHDR] != 1)
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("Only one IHDR chunk is allowed"), NULL);
        return TRUE;
    }

    /* Image width */
    if (!process_png_field (file, NULL, _("Image width"), NULL,
                       _("Minimum value: 1\n"
                         "Maximum value: 2<sup>31</sup> - 1 (signed 32-bit integer)"),
                       CHUNK_DATA_COLOR_1, 4, 0, NULL, NULL, "%d", NULL))
        return FALSE;

    /* Image height */
    if (!process_png_field (file, NULL, _("Image height"), NULL,
                       _("Minimum value: 1\n"
                         "Maximum value: 2<sup>31</sup> - 1 (signed 32-bit integer)"),
                       CHUNK_DATA_COLOR_2, 4, 0, NULL, NULL, "%d", NULL))
        return FALSE;

    /* Bit depth */
    guint8 bit_depth_values[] = { 0x1, 0x2, 0x4, 0x8, 0x10 };
    const gchar *bit_depth_value_description[] = {
        _("1 bit"),
        _("2 bits"),
        _("4 bits"),
        _("8 bits"),
        _("16 bits"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, NULL, _("Bit depth"), NULL,
                       _("Bit depth\n"
                         "<tt>01<sub>16</sub></tt>\t1 bit\n"
                         "<tt>02<sub>16</sub></tt>\t2 bits\n"
                         "<tt>04<sub>16</sub></tt>\t4 bits\n"
                         "<tt>08<sub>16</sub></tt>\t8 bits\n"
                         "<tt>10<sub>16</sub></tt>\t16 bits"),
                       CHUNK_DATA_COLOR_1, 1,
                       sizeof (bit_depth_values), bit_depth_values, bit_depth_value_description,
                       NULL, &bitdepth))
        return FALSE;

    /* Color type */
    guint8 color_type_values[] = { 0x0, 0x2, 0x3, 0x4, 0x6 };
    const gchar *color_type_value_description[] = {
        _("Grayscale sample"),
        _("RGB triple"),
        _("Palette index (a PLTE chunk must appear)"),
        _("Grayscale sample + alpha sample"),
        _("RGB triple + alpha sample"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, NULL, _("Color type"), NULL,
                       _("Color type\n"
                         "<tt>00<sub>16</sub></tt>\tGrayscale sample\n"
                         "<tt>02<sub>16</sub></tt>\tRGB triple\n"
                         "<tt>03<sub>16</sub></tt>\tPalette index (a PLTE chunk must appear)\n"
                         "<tt>04<sub>16</sub></tt>\tGrayscale sample + alpha sample\n"
                         "<tt>06<sub>16</sub></tt>\tRGB triple + alpha sample"),
                       CHUNK_DATA_COLOR_2, 1,
                       sizeof (color_type_values), color_type_values, color_type_value_description,
                       NULL, colortype))
        return FALSE;

    /* Compression method */
    guint8 compression_values[] = { 0x0 };
    const gchar *compression_value_description[] = {
        _("zlib-format DEFLATE"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, NULL, _("Compression method"), NULL,
                       _("Compression method\n"
                         "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"),
                       CHUNK_DATA_COLOR_1, 1,
                       sizeof (compression_values), compression_values, compression_value_description,
                       NULL, compression_method))
        return FALSE;

    /* Filter method */
    guint8 filter_values[] = { 0x0 };
    const gchar *filter_value_description[] = {
        _("Adaptative filtering (five basic types)"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, NULL, _("Filter method"), NULL,
                       _("Filter method\n"
                         "<tt>00<sub>16</sub></tt>\tAdaptative filtering (five basic types)"),
                       CHUNK_DATA_COLOR_2, 1,
                       sizeof (filter_values), filter_values, filter_value_description,
                       NULL, NULL))
        return FALSE;

    /* Interlace method  */
    guint8 interlace_values[] = { 0x0, 0x1 };
    const gchar *interlace_description[] = {
        _("No interlace"),
        _("Adam7 interlace"),
        _("<span foreground=\"red\">INVALID</span>")
    };
    if (!process_png_field (file, NULL, _("Interlace method"), NULL,
                       _("Interlace method\n"
                         "<tt>00<sub>16</sub></tt>\tNo interlace\n"
                         "<tt>01<sub>16</sub></tt>\tAdam7 interlace"),
                       CHUNK_DATA_COLOR_1, 1,
                       sizeof (interlace_values), interlace_values, interlace_description,
                       NULL, NULL))
        return FALSE;

    /* Fixed length chunk */
    if (chunk_length > IHDR_CHUNK_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - IHDR_CHUNK_LENGTH,
                              _("Unrecognized data"), NULL);

    gint i = 0;
    const gchar *combination;

    /* Assume invalid*/
    combination = _("<span foreground=\"red\">INVALID</span>");
    switch (*colortype)
    {
        case 0:
            while (i < 5)
            {
                if (bitdepth == bitdepths_for_colortype_0[i])
                {
                    combination = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 2:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    combination = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 3:
            while (i < 4)
            {
                if (bitdepth == bitdepths_for_colortype_3[i])
                {
                    combination = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 4:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    combination = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 6:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    combination = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
    }
    END:
    format_utils_add_line_full (file, _("Color type/bit depth combination"), combination,
                _("Color type and bit depth combinations\n"
                  "Color type: valid bit depths\n"
                  "<tt>00<sub>16</sub>: 01<sub>16</sub>, 02<sub>16</sub>, 04<sub>16</sub>, 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                  "<tt>02<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                  "<tt>03<sub>16</sub>: 01<sub>16</sub>, 02<sub>16</sub>, 04<sub>16</sub>, 08<sub>16</sub></tt>\n"
                  "<tt>04<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                  "<tt>06<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>"), 10, 0);

    return TRUE;
}
