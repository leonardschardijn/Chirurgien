/* gif-graphic-control-ext-block.c
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
gif_graphic_control_ext_block (FormatsFile    *file,
                               DescriptionTab *graphics_tab)
{
    guint8 packed_fields;
    const gchar *value;

    /* Block size */
    if (!process_gif_field (file, NULL, NULL, _("Data block size"),
                            NULL, DATA_SUBBLOCK_START_COLOR, 1, NULL, NULL))
        return FALSE;

    /* Packed fields */
    if (!process_gif_field (file, NULL, NULL,
                            _("Graphic Control Extension packed fields\n"
                              "Bit 0: Transparency color flag\n"
                              "Bit 1: User input flag\n"
                              "Bits 2-4: Disposal method"),
                            NULL, BLOCK_DATA_COLOR_1, 1, NULL, &packed_fields))
        return FALSE;

    format_utils_start_section_tab (graphics_tab, _("Graphic control extension"));

    /* Disposal method */
    switch ((packed_fields >> 2) & 0x7)
    {
        case 0:
            value = _("No disposal specified");
        break;
        case 1:
            value = _("Do not dispose");
        break;
        case 2:
            value = _("Restore to background color");
        break;
        case 3:
            value = _("Restore to previous");
        break;
        default:
            value = _("<span foreground=\"red\">INVALID</span>");
    }
    format_utils_add_line_tab (graphics_tab, _("Disposal method"), value,
                             _("Disposal method (bits 2-4 of the packed fields)\n"
                               "<tt>000<sub>2</sub></tt>\tNo disposal specified\n"
                               "<tt>001<sub>2</sub></tt>\tDo not dispose\n"
                               "<tt>010<sub>2</sub></tt>\tRestore to background color\n"
                               "<tt>011<sub>2</sub></tt>\tRestore to previous"));

    /* User input */
    if ((packed_fields >> 1) & 0x1)
        value = _("Expected");
    else
        value = _("Not expected");
    format_utils_add_line_tab (graphics_tab, _("User input"), value,
                             _("User input (bit 1 of the packed fields)\n"
                               "If user input is expected"));

    /* Delay time */
    if (!process_gif_field (file, graphics_tab, _("Delay time"), NULL,
                            _("Data stream processing delay time, in centiseconds"),
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        return FALSE;

    /* Transparency index */
    if (packed_fields & 0x1)
    {
        if (!process_gif_field (file, graphics_tab, _("Transparency index"), NULL,
                                _("Used when the transparency color flag is set (bit 0 of packed field)"),
                                BLOCK_DATA_COLOR_1, 1, "%u", NULL))
            return FALSE;
    }
    else
    {
        if (!process_gif_field (file, NULL, _("Transparency index"), NULL, NULL,
                                BLOCK_DATA_COLOR_1, 1, NULL, NULL))
            return FALSE;
    }

    /* Block terminator */
    if (!process_gif_field (file, NULL, NULL, _("Data block terminator"),
                            NULL, DATA_SUBBLOCK_START_COLOR, 1, NULL, NULL))
        return FALSE;

    return TRUE;
}
