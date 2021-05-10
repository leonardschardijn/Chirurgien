/* png-text-chunk.c
 *
 * Copyright (C) 2020 - Daniel Léonard Schardijn
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

#include "png-format.h"


gboolean
png_text_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    const gchar *text_chunk;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    guint32 keyword_length = 0, text_length = 0;
    gsize utf8_length;

    gboolean null_found = FALSE;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tEXt]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                              _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    if (!FILE_HAS_DATA_N (file, chunk_length))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Chunk length exceeds available data"), NULL);
        return FALSE;
    }

    format_utils_init_tab (&tab, NULL);

    text_chunk = (const gchar *) GET_CONTENT_POINTER (file);

    /* The null character separes the keyword and the text string */
    /* The keyword must the 1-79 bytes long */
    for (guint32 i = 0; i < chunk_length; i++)
    {
        if (text_chunk[i] == '\0')
        {
            keyword_length = i;

            null_found = TRUE;

            if (i != chunk_length - 1)
                text_length = chunk_length - i - 1;

            break;
        }
    }

    /* If 0, no null separator was found: there is no text string */
    if (!keyword_length)
        keyword_length = chunk_length;

    keyword = g_convert (text_chunk, keyword_length, "UTF-8", "ISO-8859-1",
                         NULL, &utf8_length, NULL);
    format_utils_add_text_tab (&tab, _("Keyword"), keyword, utf8_length);
    format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, keyword_length,
                          _("Keyword"), NULL);

    if (!keyword_length)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: No keyword defined, keywords should be at least 1 byte long"));
    else if (keyword_length >= 80)
        format_utils_add_line_no_section_tab (&tab, _("NOTE: The keyword exceeds its 79 bytes long limit"));

    if (null_found)
        format_utils_add_field (file, CHUNK_DATA_COLOR_2, TRUE, 1,
                              _("Null separator"), NULL);

    if (text_length)
    {
        text = g_convert (text_chunk + keyword_length + 1, text_length, "UTF-8", "ISO-8859-1",
                          NULL, &utf8_length, NULL);
        format_utils_add_text_tab (&tab, _("Text string"), text, utf8_length);
        format_utils_add_field (file, CHUNK_DATA_COLOR_1, TRUE, text_length,
                              _("Text string"), NULL);
    }

    format_utils_add_line_no_section_tab (&tab, _("NOTE: tEXt chunks are encoded using ISO-8859-1"));

    format_utils_insert_tab (file, &tab, chunk_types[tEXt]);

    return TRUE;
}
