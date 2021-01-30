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

#include "png-analyzer.h"


gboolean
analyze_text_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *text_chunk = NULL;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    gsize i;
    gsize keyword_length = 0, keyword_length_utf8;
    gsize text_length = 0, text_length_utf8;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tEXt]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    if (!FILE_HAS_DATA_N (file, chunk_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                                  _("Chunk length exceeds available data"));
        return FALSE;
    }

    analyzer_utils_init_tab (&tab);

    text_chunk = (gchar *) file->file_contents + GET_POINTER (file);
    ADVANCE_POINTER (file, chunk_length);

    /* The null character separes the keyword and the text string */
    /* The keyword must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (text_chunk[i] == '\0')
        {
            keyword = text_chunk;
            keyword_length = i;

            if (i != chunk_length - 1)
            {
                text = text_chunk + i + 1;
                text_length = chunk_length - i - 1;
            }
            break;
        }
    }

    if (keyword == NULL)
    {
        keyword = text_chunk;
        keyword_length = chunk_length;
    }

    if (keyword_length == 0 || keyword_length >= 80)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid keyword length"));
        keyword = NULL;
        text = NULL;

        return TRUE;
    }

    keyword = g_convert (keyword, keyword_length, "UTF-8", "ISO-8859-1",
                         NULL, &keyword_length_utf8, NULL);

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, keyword_length, _("Keyword"));

    analyzer_utils_add_text_tab (&tab, _("Keyword"), keyword, keyword_length_utf8);

    if (text != NULL)
    {
        analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));

        text = g_convert (text, text_length, "UTF-8", "ISO-8859-1",
                          NULL, &text_length_utf8, NULL);

        analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, text_length, _("Text string"));

        analyzer_utils_add_text_tab (&tab, _("Text string"), text, text_length_utf8);
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: tEXt chunks are encoded using ISO-8859-1"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[tEXt]);

    return TRUE;
}
