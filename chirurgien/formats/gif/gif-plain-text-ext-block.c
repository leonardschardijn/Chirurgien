/* gif-plain-text-ext-block.c
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

#include "gif-format.h"


gboolean
gif_plain_text_ext_block (FormatsFile    *file,
                          DescriptionTab *graphics_tab)
{
    DescriptionTab tab;

    GByteArray *plain_text_data;

    /* Block size */
    if (!process_gif_field (file, NULL, NULL, "Data block size",
                            NULL, DATA_SUBBLOCK_START_COLOR, 1, NULL, NULL))
        return FALSE;

    format_utils_start_section_tab (graphics_tab, "Plain text extension");
    format_utils_add_line_tab (graphics_tab,
           "The preceding Graphic control extension (if present) affects a Plain text extension", NULL, NULL);
    format_utils_init_tab (&tab, "Plain text extension");

    /* Text grid left position  */
    if (!process_gif_field (file, &tab, "Text grid left position", NULL,
                            "Text grid column number in the Logical Screen", BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        return FALSE;

    /* Text grid top position */
    if (!process_gif_field (file, &tab, "Text grid top position", NULL,
                            "Text grid row number in the Logical Screen", BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        return FALSE;

    /* Text grid width */
    if (!process_gif_field (file, &tab, "Text grid width", NULL,
                            "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)",
                            BLOCK_DATA_COLOR_1, 2, "%u", NULL))
        return FALSE;

    /* Text grid height */
    if (!process_gif_field (file, &tab, "Text grid height", NULL,
                            "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)",
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        return FALSE;

    /* Character cell width */
    if (!process_gif_field (file, &tab, "Character cell width", NULL,
                            "Maximum value: 2<sup>8</sup> - 1 (unsigned 8-bit integer)",
                            BLOCK_DATA_COLOR_1, 1, "%u", NULL))
        return FALSE;

    /* Character cell height */
    if (!process_gif_field (file, &tab, "Character cell height", NULL,
                            "Maximum value: 2<sup>8</sup> - 1 (unsigned 8-bit integer)",
                            BLOCK_DATA_COLOR_2, 1, "%u", NULL))
        return FALSE;

    /* Text foreground color index */
    if (!process_gif_field (file, &tab, "Text foreground color index", NULL,
                            "Global Color Table index of the text foreground color",
                            BLOCK_DATA_COLOR_1, 1, "%u", NULL))
        return FALSE;

    /* Text background color index */
    if (!process_gif_field (file, &tab, "Text background color index", NULL,
                            "Global Color Table index of the text background color",
                            BLOCK_DATA_COLOR_2, 1, "%u", NULL))
        return FALSE;

    if (!process_data_subblocks (file, "Plain Text Extension data block", &plain_text_data,
                                 DATA_SUBBLOCK_START_COLOR, BLOCK_DATA_COLOR_1, TRUE))
        return FALSE;

    format_utils_add_text_tab (&tab, "Plain text",
                              (const gchar *) plain_text_data->data, plain_text_data->len);

    format_utils_insert_tab (file, &tab, "Plain text ext.");

    g_byte_array_free (plain_text_data, TRUE);

    return TRUE;
}
