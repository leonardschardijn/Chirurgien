/* webp-format.h
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
#include "container-formats/riff/riff-format.h"

G_BEGIN_DECLS

enum {
    WEBP,
    VP8,
    VP8L,
    VP8X,
    ANIM,
    ANMF,
    ALPH,
    ICCP,
    EXIF,
    XMP,
    CHUNK_TYPES
};

typedef struct
{
    DescriptionTab *frames_tab;

    gboolean anmf_found;

} WebpExtraData;

void    webp_vp8_chunk      (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_vp8l_chunk     (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_vp8x_chunk     (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_anim_chunk     (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_anmf_chunk     (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_alph_chunk     (FormatsFile *,
                             RiffFile *,
                             guint32);

void    webp_generic_chunk  (FormatsFile *,
                             RiffFile *,
                             guint32);

G_END_DECLS
