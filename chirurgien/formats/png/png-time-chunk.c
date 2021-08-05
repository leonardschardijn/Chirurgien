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

#include "png-format.h"

#define TIME_CHUNK_LENGTH 7


gboolean
png_time_chunk (FormatsFile *file,
                guint32      chunk_length,
                gint        *chunk_counts)
{
    DescriptionTab tab;

    gchar *full_date;

    gint color_toggle;

    guint16 year;
    guint8 date[5];

    const gchar * const fields[] = {
        "Month",
        "Day",
        "Hour",
        "Minute",
        "Second"
    };

    if (!chunk_length)
        return TRUE;

    chunk_counts[tIME]++;

    if (!chunk_counts[IHDR])
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                "The first chunk must be the IHDR chunk", NULL);
        return TRUE;
    }

    format_utils_init_tab (&tab, "Last-modification time");

    if (!process_png_field (file, &tab, "Year", NULL,
                            NULL,
                            CHUNK_DATA_COLOR_1, 2,
                            0, NULL, NULL,
                            "%u", &year))
        return FALSE;

    for (gint i = 0; i < 5; i++)
    {
        if (i % 2)
            color_toggle = CHUNK_DATA_COLOR_1;
        else
            color_toggle = CHUNK_DATA_COLOR_2;

        if (!process_png_field (file, &tab, fields[i], NULL,
                                NULL,
                                color_toggle, 1,
                                0, NULL, NULL,
                                "%u", &date[i]))
            return FALSE;
    }

    full_date = g_strdup_printf ("%.4u-%.2u-%.2u %.2u:%.2u:%.2u",
                                 year, date[0], date[1],
                                 date[2], date[3], date[4]);
    format_utils_add_line_full_tab (&tab, "Date", full_date,
                                    NULL, 10, 0);
    g_free (full_date);

    /* Fixed length chunk */
    if (chunk_length > TIME_CHUNK_LENGTH)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length - TIME_CHUNK_LENGTH,
                                "Unrecognized data", NULL);

    format_utils_insert_tab (file, &tab, chunk_types[tIME]);

    return TRUE;
}
