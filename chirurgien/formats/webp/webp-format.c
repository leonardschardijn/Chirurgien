/* webp-format.c
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

#include "webp-format.h"


const gchar * const webp_chunk_types[CHUNK_TYPES] = {
    "WEBP",
    "VP8 ",
    "VP8L",
    "VP8X",
    "ANIM",
    "ANMF",
    "ALPH",
    "ICCP",
    "EXIF",
    "XMP "
};

void
chirurgien_webp (FormatsFile *file)
{
    RiffFile riff_file;
    WebpExtraData extra_data;
    DescriptionTab frames_tab;

    void (*chunk_type_functions[CHUNK_TYPES - 1]) (FormatsFile *, RiffFile *, guint32);

    format_utils_set_title (file, "WebP image");

    riff_file.chunk_types = webp_chunk_types;
    riff_file.chunk_type_count = CHUNK_TYPES;

    chunk_type_functions[0] = webp_vp8_chunk;     // VP8
    chunk_type_functions[1] = webp_vp8l_chunk;    // VP8L
    chunk_type_functions[2] = webp_vp8x_chunk;    // VP8X
    chunk_type_functions[3] = webp_anim_chunk;    // ANIM
    chunk_type_functions[4] = webp_anmf_chunk;    // ANIM
    chunk_type_functions[5] = webp_alph_chunk;    // ALPH
    chunk_type_functions[6] = webp_generic_chunk; // ICCP
    chunk_type_functions[7] = webp_generic_chunk; // EXIF
    chunk_type_functions[8] = webp_generic_chunk; // XMP

    riff_file.chunk_type_functions = chunk_type_functions;

    format_utils_init_tab (&frames_tab, NULL);
    extra_data.frames_tab = &frames_tab;
    extra_data.anmf_found = FALSE;

    riff_file.extra_data = &extra_data;

    chirurgien_riff (file, &riff_file);

    format_utils_insert_tab (file, &frames_tab, "ANMF(s)");
}
