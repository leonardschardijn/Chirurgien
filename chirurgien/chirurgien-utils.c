/* chirurgien-utils.c
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

#include "chirurgien-utils.h"

#include "chirurgien-globals.h"


void
chirurgien_utils_hex_print (gchar        *destination,
                            const guchar *raw_source,
                            gsize         start_offset,
                            gsize         destination_size,
                            gsize         raw_source_size,
                            gsize         line_break)
{
    gsize raw_index, formatted_index;

    for (raw_index = start_offset, formatted_index = 0;
         formatted_index < destination_size && raw_index < raw_source_size;
         raw_index++)
    {
        destination[formatted_index++] = hex_chars[ raw_source[raw_index] >> 4 ];
        destination[formatted_index++] = hex_chars[ raw_source[raw_index] & 0x0F ];
        destination[formatted_index++] = ' ';
    }

    if (formatted_index < destination_size)
        destination[formatted_index] = '\0';

    for (formatted_index = line_break - 1;
         formatted_index < destination_size;
         formatted_index += line_break)
    {
        destination[formatted_index] = '\n';
    }
}

void
chirurgien_utils_text_print (gchar        *destination,
                             const guchar *raw_source,
                             gsize         start_offset,
                             gsize         destination_size,
                             gsize         raw_source_size,
                             gsize         line_break)
{
    gsize raw_index, formatted_index;

    for (raw_index = start_offset, formatted_index = 0;
         formatted_index < destination_size && raw_index < raw_source_size;
         raw_index++)
    {
        if (g_ascii_isgraph (raw_source[raw_index]))
            destination[formatted_index++] = raw_source[raw_index];
        else
            destination[formatted_index++] = '.';
        destination[formatted_index++] = ' ';
        destination[formatted_index++] = ' ';
    }

    if (formatted_index < destination_size)
        destination[formatted_index] = '\0';

    for (formatted_index = line_break - 1;
         formatted_index < destination_size;
         formatted_index += line_break)
    {
        destination[formatted_index] = '\n';
    }
}
