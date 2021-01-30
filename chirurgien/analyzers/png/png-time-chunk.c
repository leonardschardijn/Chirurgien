/* png-time-chunk.c
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
analyze_time_chunk (AnalyzerFile *file,
                    gsize chunk_length,
                    guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    GdkRGBA *color_toggle;

    guint16 year;
    guint8 date[5];

    gchar *fields[] = {
        _("Month"),
        _("Day"),
        _("Hour"),
        _("Minute"),
        _("Second")
    };

    guint i;

    if (!chunk_length)
        return TRUE;

    chunk_counts[tIME]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        ADVANCE_POINTER (file, chunk_length);
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    analyzer_utils_set_title_tab (&tab, _("<b>Last-modification time</b>"));

    if (!process_png_field (file, &tab, _("Year"), NULL,
                            NULL, CHUNK_DATA_COLOR_1, 2, 0, NULL, NULL, "%u", &year))
        goto END_ERROR;

    for (i = 0; i < 5; i++)
    {
        if (i % 2)
            color_toggle = CHUNK_DATA_COLOR_1;
        else
            color_toggle = CHUNK_DATA_COLOR_2;

        if (!process_png_field (file, &tab, fields[i], NULL,
                        NULL, color_toggle, 1, 0, NULL, NULL, "%u", &date[i]))
            return FALSE;
    }

    description_message = g_strdup_printf ("%.4u-%.2u-%.2u %.2u:%.2u:%.2u",
                                           year, date[0], date[1],
                                           date[2], date[3], date[4]);
    analyzer_utils_add_description_tab (&tab, _("Date"), description_message,
                                        NULL, 10, 0);
    g_free (description_message);

    /* Fixed length chunk */
    if (chunk_length > 7)
    {
        chunk_length -= 7;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, chunk_length);
    }

    analyzer_utils_insert_tab (file, &tab, chunk_types[tIME]);

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
