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

#include "gif-format.h"


gboolean
gif_comment_ext_block (FormatsFile *file)
{
    DescriptionTab tab;

    GByteArray *comment;

    format_utils_init_tab (&tab, NULL);

    if (!process_data_subblocks (file, "Comment Extension data block", &comment,
                                 DATA_SUBBLOCK_START_COLOR, BLOCK_DATA_COLOR_1, TRUE))
        return FALSE;

    format_utils_add_text_tab (&tab, "Comment",
                              (const gchar *) comment->data, comment->len);

    format_utils_insert_tab (file, &tab, "Comment ext.");

    g_byte_array_free (comment, TRUE);

    return TRUE;
}
