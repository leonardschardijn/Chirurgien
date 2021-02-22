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

#include "gif-analyzer.h"


gboolean
analyze_graphic_control_ext_block (AnalyzerFile *file,
                                   AnalyzerTab *tab)
{
    guint8 packed_fields;
    gchar *description_message;

    analyzer_utils_add_description_tab (tab, _("<b>Graphic control extension</b>"), NULL, NULL, 20, 10);

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

    /* Disposal method */
    switch ((packed_fields >> 2) & 0x7)
    {
        case 0:
            description_message = _("No disposal specified");
        break;
        case 1:
            description_message = _("Do not dispose");
        break;
        case 2:
            description_message = _("Restore to background color");
        break;
        case 3:
            description_message = _("Restore to previous");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }
    analyzer_utils_describe_tooltip_tab (tab, _("Disposal method"), description_message,
                                         _("Disposal method (bits 2-4 of the packed fields)\n"
                                           "<tt>000<sub>2</sub></tt>\tNo disposal specified\n"
                                           "<tt>001<sub>2</sub></tt>\tDo not dispose\n"
                                           "<tt>010<sub>2</sub></tt>\tRestore to background color\n"
                                           "<tt>011<sub>2</sub></tt>\tRestore to previous"));

    /* User input */
    if ((packed_fields >> 1) & 0x1)
        description_message = _("Expected");
    else
        description_message = _("Not expected");
    analyzer_utils_describe_tooltip_tab (tab, _("User input"), description_message,
                                         _("User input (bit 1 of the packed fields)\n"
                                           "If user input is expected"));

    /* Delay time */
    if (!process_gif_field (file, tab, _("Delay time"), NULL,
                            _("Data stream processing delay time, in centiseconds"),
                            BLOCK_DATA_COLOR_2, 2, "%u", NULL))
        goto END_ERROR;

    /* Transparency index */
    if (packed_fields & 0x1)
    {
        if (!process_gif_field (file, tab, _("Transparency index"), NULL,
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

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
