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

#include "gif-format.h"


gboolean
gif_image_descriptor_block (FormatsFile    *file,
                            DescriptionTab *tab)
{
    guint8 packed_fields;
    gint local_color_table_size;

    gchar *value;

    format_utils_start_section_tab (tab, _("Image descriptor"));

    /* Image left position */
    if (!process_gif_field (file, tab, _("Image left position"), NULL,
                _("Image column number in the Logical Screen"), BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        return FALSE;

    /* Image top position */
    if (!process_gif_field (file, tab, _("Image top position"), NULL,
                _("Image row number in the Logical Screen"), BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        return FALSE;

    /* Image width */
    if (!process_gif_field (file, tab, _("Image width"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        return FALSE;

    /* Image height */
    if (!process_gif_field (file, tab, _("Image height"), NULL,
                            _("Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        return FALSE;

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
        value = _("Yes");
    else
        value = _("No");
    format_utils_add_line_tab (tab, _("Interlaced image"), value,
                             _("Interlace flag (bit 6 of the packed fields)\n"
                               "If the image is interlaced"));

    /* Local Color Table */
    if (packed_fields >> 7)
    {
        local_color_table_size = 1 << ((packed_fields & 0x7) + 1);

        for (gint i = 0; i < local_color_table_size; i++)
        {
            if (i % 2)
                format_utils_add_field (file, BLOCK_DATA_COLOR_1, TRUE, 3,
                                      _("Local Color Table entry (RGB triplet)"), NULL);
            else
                format_utils_add_field (file, BLOCK_DATA_COLOR_2, TRUE, 3,
                                      _("Local Color Table entry (RGB triplet)"), NULL);
        }
    }

    /* LZW minimum code size */
    if (!process_gif_field (file, tab, _("LZW minimum code size"), NULL,
                _("Initial number of bits for LZW codes"), BLOCK_DATA_COLOR_2, 1, "%u", NULL))
        return FALSE;

    /* Print Local Color Table fields */
    if (packed_fields >> 7)
    {
        format_utils_start_section_tab (tab, _("Local color table"));

        if ((packed_fields >> 5) & 0x1)
            value = _("Yes");
        else
            value = _("No");
        format_utils_add_line_tab (tab, _("Sorted"), value,
                         _("Sorted flag (bit 5 of the packed fields)\n"
                           "If the Local Color Table is sorted, in order of decreasing importance"));

        value = g_strdup_printf ("%u", local_color_table_size);
        format_utils_add_line_tab (tab, _("Local color table entries"), value,
                         _("Number of entries in the Local Color Table\n"
                           "Calculated using the Local Color Table size (bits 0-2 of the packed fields):\n"
                           "2<sup>size + 1</sup>"));
        g_free (value);
    }

    if (!process_data_subblocks (file, _("Image Descriptor data block (LZW compressed image)"),
                                 NULL, DATA_SUBBLOCK_START_COLOR, BLOCK_DATA_COLOR_1, TRUE))
        return FALSE;

    return TRUE;
}
