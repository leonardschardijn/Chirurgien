/* gif-comment-ext-block.c
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
analyze_comment_ext_block (AnalyzerFile *file)
{
    AnalyzerTab tab;

    GByteArray *comment;

    analyzer_utils_init_tab (&tab);

    if (!process_data_subblocks (file, _("Comment Extension data block"), &comment, FALSE))
        goto END_ERROR;

    if (g_utf8_validate ((gchar *) comment->data, comment->len, NULL))
        analyzer_utils_add_text_tab (&tab, _("Comment"), (gchar *) comment->data, comment->len);
    else
        analyzer_utils_add_text_tab (&tab, _("Comment"), "", 0);

    analyzer_utils_insert_tab (file, &tab, _("Comment ext."));

    g_byte_array_free (comment, TRUE);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
