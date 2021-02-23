/* gif-logical-screen-descriptor-block.c
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

#include "gif-analyzer.h"


gboolean
analyze_logical_screen_descriptor_block (AnalyzerFile *file)
{
    guint8 fields[3];
    guint global_color_table_size;

    gchar *description_message;

    /* Logical screen width */
    if (!process_gif_field (file, NULL, _("Logical screen width"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        goto END_ERROR;

    /* Logical screen height */
    if (!process_gif_field (file, NULL, _("Logical screen height"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        goto END_ERROR;

    /* Packed fields */
    if (!process_gif_field (file, NULL, NULL,
                            _("Logical Screen Descriptor packed fields\n"
                              "Bit 0-2: Global Color Table size\n"
                              "Bit 3: Sorted flag\n"
                              "Bit 4-6: Color resolution\n"
                              "Bit 7: Global Color Table flag"),
                            NULL, BLOCK_DATA_COLOR_1, 1, NULL, &fields[0]))
        return FALSE;

    /* Background color index */
    if (!process_gif_field (file, NULL, _("Background color index"), NULL, NULL,
                            BLOCK_DATA_COLOR_2, 1, NULL, &fields[1]))
        return FALSE;

    /* Pixel aspect ratio */
    if (!process_gif_field (file, NULL, _("Pixel aspect ratio"), NULL, NULL,
                            BLOCK_DATA_COLOR_1, 1, NULL, &fields[2]))
        return FALSE;

    /* Color resolution */
    description_message = g_strdup_printf (_("%u bits"), ((fields[0] >> 4) & 0x7) + 1);
    analyzer_utils_describe_tooltip (file, _("Color resolution"), description_message,
                                     _("Number of bits per primary color available to the original image\n"
                                       "Calculated using the color resolution (bits 4-6 of the packed fields):\n"
                                       "color resolution + 1"));
    g_free (description_message);

    /* Pixel aspect ratio */
    if (fields[2])
    {
        description_message = g_strdup_printf ("%f", (gfloat) ((fields[2] + 15 ) / 64));
        analyzer_utils_describe_tooltip (file, _("Aspect ratio"), description_message,
                                         _("Aspect ratio aproximation, calculated as:"
                                           "Pixel aspect ratio + 15 / 64"));
        g_free (description_message);
    }

    /* Global Color Table */
    if (fields[0] >> 7)
    {
        analyzer_utils_set_subtitle (file, _("<b>Global Color Table</b>"),
                                     _("Global Color Table flag (bit 7 of the packed fields)"));

        if ((fields[0] >> 3) & 0x1)
            description_message = _("Yes");
        else
            description_message = _("No");
        analyzer_utils_describe_tooltip (file, _("Sorted"), description_message,
                             _("Sorted flag (bit 3 of the packed fields)\n"
                               "If the Global Color Table is sorted, in order of decreasing importance"));

        description_message = g_strdup_printf ("%u", fields[1]);
        analyzer_utils_describe_tooltip (file, _("Background color index"), description_message,
                             _("Global Color Table index of the background color"));
        g_free (description_message);

        global_color_table_size = 1 << ((fields[0] & 0x7) + 1);

        description_message = g_strdup_printf ("%u", global_color_table_size);
        analyzer_utils_describe_tooltip (file, _("Global Color Table entries"), description_message,
                             _("Number of entries in the Global Color Table\n"
                               "Calculated using the Global Color Table size (bits 0-2 of the packed fields):\n"
                               "2<sup>size + 1</sup>"));
        g_free (description_message);

        for (guint i = 0; i < global_color_table_size; i++)
        {
            if (i % 2)
                analyzer_utils_tag (file, BLOCK_DATA_COLOR_1, 3,
                                    _("Global Color Table entry (RGB triplet)"));
            else
                analyzer_utils_tag (file, BLOCK_DATA_COLOR_2, 3,
                                    _("Global Color Table entry (RGB triplet)"));
        }

        ADVANCE_POINTER (file, global_color_table_size * 3);
    }

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
