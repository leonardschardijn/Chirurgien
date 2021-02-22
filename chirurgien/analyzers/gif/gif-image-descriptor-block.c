/* gif-image-descriptor-block.c
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
analyze_image_descriptor_block (AnalyzerFile *file,
                                AnalyzerTab *tab)
{
    guint8 packed_fields;
    guint local_color_table_size;

    gchar *description_message;

    analyzer_utils_add_description_tab (tab, _("<b>Image descriptor</b>"), NULL, NULL, 10, 10);

    /* Image left position */
    if (!process_gif_field (file, tab, _("Image left position"), NULL,
                            _("Image column number in the Logical Screen"), BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        goto END_ERROR;

    /* Image top position */
    if (!process_gif_field (file, tab, _("Image top position"), NULL,
                            _("Image row number in the Logical Screen"), BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        goto END_ERROR;

    /* Image width */
    if (!process_gif_field (file, tab, _("Image width"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        goto END_ERROR;

    /* Image height */
    if (!process_gif_field (file, tab, _("Image height"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        goto END_ERROR;

    /* Packed fields */
    if (!process_gif_field (file, NULL, NULL,
                            _("Image Descriptor packed fields\n"
                              "Bit 0-2: Local Color Table size\n"
                              "Bit 5: Sorted flag\n"
                              "Bit 6: Interlace flag\n"
                              "Bit 7: Local Color Table flag"),
                            NULL, BLOCK_DATA_COLOR_1, 1, NULL, &packed_fields))
        return FALSE;

    /* Interlace */
    if ((packed_fields >> 6) & 0x1)
        description_message = _("Yes");
    else
        description_message = _("No");
    analyzer_utils_describe_tooltip_tab (tab, _("Interlaced image"), description_message,
                                     _("Interlace flag (bit 6 of the packed fields)\n"
                                       "If the image is interlaced"));

    /* Local Color Table */
    if (packed_fields >> 7)
    {
        local_color_table_size = 1 << ((packed_fields & 0x7) + 1);

        for (guint i = 0; i < local_color_table_size; i++)
        {
            if (i % 2)
                analyzer_utils_tag (file, BLOCK_DATA_COLOR_1, 3,
                                    _("Local Color Table entry (RGB triplet)"));
            else
                analyzer_utils_tag (file, BLOCK_DATA_COLOR_2, 3,
                                    _("Local Color Table entry (RGB triplet)"));
        }

        ADVANCE_POINTER (file, local_color_table_size * 3);
    }

    /* LZW minimum code size */
    if (!process_gif_field (file, tab, _("LZW minimum code size"), NULL,
                            _("Initial number of bits for LZW codes"), BLOCK_DATA_COLOR_2, 1, "%u", NULL))
        return FALSE;

    /* Print Local Color Table fields */
    if (packed_fields >> 7)
    {
        analyzer_utils_add_description_tab (tab, _("<b>Local color table</b>"), NULL,
                         _("Local Color Table flag (bit 7 of the packed fields)"), 10, 10);

        if ((packed_fields >> 5) & 0x1)
            description_message = _("Yes");
        else
            description_message = _("No");
        analyzer_utils_describe_tooltip_tab (tab, _("Sorted"), description_message,
                             _("Sorted flag (bit 5 of the packed fields)\n"
                               "If the Local Color Table is sorted, in order of decreasing importance"));

        description_message = g_strdup_printf ("%u", local_color_table_size);
        analyzer_utils_describe_tooltip_tab (tab, _("Local color table entries"), description_message,
                             _("Number of entries in the Local Color Table\n"
                               "Calculated using the Local Color Table size (bits 0-2 of the packed fields):\n"
                               "2<sup>size + 1</sup>"));
        g_free (description_message);
    }

    if (!process_data_subblocks (file, _("Image Descriptor data block (LZW compressed image)"), NULL, FALSE))
        goto END_ERROR;

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
