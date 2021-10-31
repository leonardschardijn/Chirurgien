/* riff-format.h
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

#pragma once

#include <gtk/gtk.h>

#include <format-utils.h>

G_BEGIN_DECLS

#define CHUNK_FOURCC_COLOR    0
#define CHUNK_SIZE_COLOR      1
#define CHUNK_PAYLOAD_COLOR_1 2
#define CHUNK_PAYLOAD_COLOR_2 3
#define ERROR_COLOR_1         6
#define ERROR_COLOR_2         7
#define PADDING_COLOR         8

typedef struct RiffFile RiffFile;

struct RiffFile
{
    /* Chunk FourCCs/types */
    const gchar * const   *chunk_types;
    /* Number of types */
    guint                  chunk_type_count;

    /* Functions to handle each chunk type */
    void (**chunk_type_functions) (FormatsFile *, RiffFile *, guint32);

    /* The last chunk type that matched */
    guint chunk_selection;

    /* Additional data */
    gpointer extra_data;

};

void    chirurgien_riff    (FormatsFile *,
                            RiffFile *);

G_END_DECLS
