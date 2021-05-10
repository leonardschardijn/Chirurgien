/* gif-application-ext-block.c
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
gif_application_ext_block (FormatsFile *file)
{
    DescriptionTab tab;

    /* Block size */
    if (!process_gif_field (file, NULL, NULL, _("Data block size"),
                            NULL, DATA_SUBBLOCK_START_COLOR, 1, NULL, NULL))
        return FALSE;

    /* Application identifier */
    if (!FILE_HAS_DATA_N (file, 8))
        return FALSE;

    format_utils_init_tab (&tab, NULL);

    format_utils_add_text_tab (&tab, _("Application identifier"),
                              (const gchar *) GET_CONTENT_POINTER (file), 8);
    format_utils_add_field (file, BLOCK_DATA_COLOR_1, TRUE, 8,
                          _("Application identifier"), NULL);

    format_utils_start_section_tab (&tab, _("Authentication code"));

    /* Application authentication code */
    if (!process_gif_field (file, &tab, _("Authentication code"), NULL,
                            _("Used to authenticate the application identifier"),
                            BLOCK_DATA_COLOR_2, 3, "%X", NULL))
        return FALSE;

    if (!process_data_subblocks (file, _("Application Extension data block"), NULL,
                                 DATA_SUBBLOCK_START_COLOR, BLOCK_DATA_COLOR_1, TRUE))
        return FALSE;

    format_utils_insert_tab (file, &tab, _("Application ext."));

    return TRUE;
}
