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

#include "png-analyzer.h"


gboolean
analyze_ihdr_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts,
                    guint8 *colortype)
{
    const guint8 bitdepths_for_colortype_0[5] = { 1, 2, 4, 8, 16 };
    const guint8 bitdepths_for_colortype_2_4_6[2] = { 8, 16 };
    const guint8 bitdepths_for_colortype_3[4] = { 1, 2, 4, 8 };

    gchar *description_message;

    guint32 four_bytes;
    guint8 one_byte;
    guint8 bitdepth;

    if (!chunk_length)
        return TRUE;

    if (chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Only one IHDR chunk is allowed"));
        return TRUE;
    }

    /* Image width */
    if (!analyzer_utils_read (&four_bytes, file , 4))
        goto END_ERROR;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 4, _("Image width"));

    four_bytes = g_ntohl (four_bytes);
    description_message = g_strdup_printf ("%u", four_bytes);
    analyzer_utils_describe_tooltip (file, _("Image width"), description_message,
                                     _("Minimum value: 1\n"
                                     "Maximum value: 2<sup>31</sup> - 1 (signed 32-bit integer)"));
    g_free (description_message);

    /* Image height */
    if (!analyzer_utils_read (&four_bytes, file , 4))
        goto END_ERROR;

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 4, _("Image height"));

    four_bytes = g_ntohl (four_bytes);
    description_message = g_strdup_printf ("%u", four_bytes);
    analyzer_utils_describe_tooltip (file, _("Image height"), description_message,
                                     _("Minimum value: 1\n"
                                     "Maximum value: 2<sup>31</sup> - 1 (signed 32-bit integer)"));
    g_free (description_message);

    /* Bit depth */
    if (!analyzer_utils_read (&bitdepth, file , 1))
        return FALSE;

    switch (bitdepth)
    {
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
            analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Bit depth"));
            description_message = g_strdup_printf (_("%hhu bits"), bitdepth);
            break;
        default:
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid bit depth"));
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));
    }
    analyzer_utils_describe_tooltip (file, _("Bit depth"), description_message,
                                     _("Bit depth\n"
                                     "<tt>01<sub>16</sub></tt>\t1 bit\n"
                                     "<tt>02<sub>16</sub></tt>\t2 bits\n"
                                     "<tt>04<sub>16</sub></tt>\t4 bits\n"
                                     "<tt>08<sub>16</sub></tt>\t8 bits\n"
                                     "<tt>10<sub>16</sub></tt>\t16 bits"));
    g_free (description_message);

    /* Color type */
    if (!analyzer_utils_read (colortype, file, 1))
        return FALSE;

    switch (*colortype)
    {
        case 0:
            description_message = _("Grayscale sample");
            goto VALID_COLORTYPE;
        case 2:
            description_message = _("RGB triple");
            goto VALID_COLORTYPE;
        case 3:
            description_message = _("Palette index");
            goto VALID_COLORTYPE;
        case 4:
            description_message = _("Grayscale sample + alpha sample");
            goto VALID_COLORTYPE;
        case 6:
            description_message = _("RGB triple + alpha sample");
            goto VALID_COLORTYPE;
        default:
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid color type"));
            description_message = _("<span foreground=\"red\">INVALID</span>");
            goto END_COLORTYPE;
    }
    VALID_COLORTYPE:
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Color type"));

    END_COLORTYPE:
    analyzer_utils_describe_tooltip (file, _("Color type"), description_message,
                                     _("Color type\n"
                                     "<tt>00<sub>16</sub></tt>\tGrayscale sample\n"
                                     "<tt>02<sub>16</sub></tt>\tRGB triple\n"
                                     "<tt>03<sub>16</sub></tt>\tPalette index (a PLTE chunk must appear)\n"
                                     "<tt>04<sub>16</sub></tt>\tGrayscale sample + alpha sample\n"
                                     "<tt>06<sub>16</sub></tt>\tRGB triple + alpha sample"));

    /* Compression method */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    if (!one_byte)
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Compression method"));
        description_message = _("zlib-format DEFLATE");
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid compression method"));
        description_message = _("<span foreground=\"red\">INVALID</span>");
    }
    analyzer_utils_describe_tooltip (file, _("Compression method"), description_message,
                                     _("Compression method\n"
                                     "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"));

    /* Filter method */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    if (!one_byte)
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Filter method"));
        description_message = _("Adaptative filtering (five basic types)");
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid filter method"));
        description_message = _("<span foreground=\"red\">INVALID</span>");
    }
    analyzer_utils_describe_tooltip (file, _("Filter method"), description_message,
                                     _("Filter method\n"
                                     "<tt>00<sub>16</sub></tt>\tAdaptative filtering (five basic types)"));

    /* Interlace method  */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    if (!one_byte)
    {
        description_message = _("No interlace");
        goto VALID_INTERLACE;
    }
    else if (one_byte == 1)
    {
        description_message = _("Adam7 interlace");
        goto VALID_INTERLACE;
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Invalid interlace method"));
        description_message = _("<span foreground=\"red\">INVALID</span>");
        goto END_INTERLACE;
    }
    VALID_INTERLACE:
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Interlace method"));

    END_INTERLACE:
    analyzer_utils_describe_tooltip (file, _("Interlace method"), description_message,
                                     _("Interlace method\n"
                                     "<tt>00<sub>16</sub></tt>\tNo interlace\n"
                                     "<tt>01<sub>16</sub></tt>\tAdam7 interlace"));

    /* Fixed length chunk */
    if (chunk_length > 13)
    {
        chunk_length -= 13;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    gint i = 0;

    /* Assume invalid*/
    description_message = _("<span foreground=\"red\">INVALID</span>");
    switch (*colortype)
    {
        case 0:
            while (i < 5)
            {
                if (bitdepth == bitdepths_for_colortype_0[i])
                {
                    description_message = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 2:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    description_message = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 3:
            while (i < 4)
            {
                if (bitdepth == bitdepths_for_colortype_3[i])
                {
                    description_message = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 4:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    description_message = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
        case 6:
            while (i < 2)
            {
                if (bitdepth == bitdepths_for_colortype_2_4_6[i])
                {
                    description_message = _("<span foreground=\"green\">VALID</span>");
                    goto END;
                }
                i++;
            }
    }
    END:
    analyzer_utils_add_description (file, _("Color type/bit depth combination"), description_message,
                _("Color type and bit depth combinations\n"
                "Color type: valid bit depths\n"
                "<tt>00<sub>16</sub>: 01<sub>16</sub>, 02<sub>16</sub>, 04<sub>16</sub>, 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                "<tt>02<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                "<tt>03<sub>16</sub>: 01<sub>16</sub>, 02<sub>16</sub>, 04<sub>16</sub>, 08<sub>16</sub></tt>\n"
                "<tt>04<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>\n"
                "<tt>06<sub>16</sub>: 08<sub>16</sub>, 10<sub>16</sub></tt>"),
                10, 0);

    chunk_counts[IHDR]++;

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
