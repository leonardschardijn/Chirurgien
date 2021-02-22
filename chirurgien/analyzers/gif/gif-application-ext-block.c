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

#include "gif-analyzer.h"


gboolean
analyze_application_ext_block (AnalyzerFile *file)
{
    AnalyzerTab tab;

    gchar *description_message;

    analyzer_utils_init_tab (&tab);

    /* Block size */
    if (!process_gif_field (file, NULL, NULL, _("Data block size"),
                            NULL, DATA_SUBBLOCK_START_COLOR, 1, NULL, NULL))
        return FALSE;

    /* Application identifier */
    if (!FILE_HAS_DATA_N (file, 8))
        goto END_ERROR;

    description_message = (gchar *) file->file_contents + GET_POINTER (file);
    if (g_utf8_validate ((gchar *) file->file_contents + GET_POINTER (file), 8, NULL))
        analyzer_utils_add_text_tab (&tab, _("Application identifier"), description_message, 8);
    else
        analyzer_utils_add_text_tab (&tab, _("Application identifier"), "", 0);

    analyzer_utils_tag (file, BLOCK_DATA_COLOR_2, 8, _("Application identifier"));
    ADVANCE_POINTER (file, 8);

    /* Application authentication code */
    if (!process_gif_field (file, &tab, _("Authentication code"), NULL,
                            _("Used to authenticate the application identifier"),
                            BLOCK_DATA_COLOR_1, 3, "%X", NULL))
        goto END_ERROR;

    if (!process_data_subblocks (file, _("Application Extension data block"), NULL, FALSE))
        goto END_ERROR;

    analyzer_utils_insert_tab (file, &tab, _("Application ext."));

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
