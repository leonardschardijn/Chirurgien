/* webp-alph-chunk.c
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


void
webp_alph_chunk (FormatsFile *file,
                 RiffFile    *riff_file,
                 guint32      chunk_size)
{
    WebpExtraData *extra_data;
    DescriptionTab tab, *selected_tab;
    const gchar *value, *alpha_bitstream;

    guchar flags;

    /* Flags */
    if (!format_utils_read (file, &flags, 1))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 1,
                            "Flags\n"
                            "Bits 0-1: Compression method\n"
                            "Bits 2-3: Filtering method\n"
                            "Bits 4-5: Pre-processing\n"
                            "Bits 6-7: Reserved", NULL);

    extra_data = riff_file->extra_data;

    if (!extra_data->anmf_found)
    {
        format_utils_init_tab (&tab, "Image alpha");
        selected_tab = &tab;
    }
    else
    {
        format_utils_start_section_tab (extra_data->frames_tab, "Image alpha");
        selected_tab = extra_data->frames_tab;
    }

    /* Pre-processing */
    switch ((flags >> 4) & 0x3)
    {
        case 0:
        value = "No pre-processing";
        break;
        case 1:
        value = "Level reduction";
        break;
        default:
        value = "<span foreground=\"red\">INVALID</span>";
    }
    format_utils_add_line_tab (selected_tab, "Pre-processing", value,
                               "Pre-processing\n"
                               "<tt>00<sub>2</sub></tt>\tNo pre-processing\n"
                               "<tt>01<sub>2</sub></tt>\tLevel reduction");

    /* Filtering method */
    switch ((flags >> 2) & 0x3)
    {
        case 0:
        value = "None";
        break;
        case 1:
        value = "Horizontal filter";
        break;
        case 2:
        value = "Vertical filter";
        break;
        case 3:
        value = "Gradient filter";
        break;
    }
    format_utils_add_line_tab (selected_tab, "Filtering method", value,
                               "Filtering method\n"
                               "<tt>00<sub>2</sub></tt>\tNone\n"
                               "<tt>01<sub>2</sub></tt>\tHorizontal filter\n"
                               "<tt>10<sub>2</sub></tt>\tVertical filter\n"
                               "<tt>11<sub>2</sub></tt>\tGradient filter");

    /* Compression method */
    switch (flags & 0x3)
    {
        case 0:
        value = "No compression";
        alpha_bitstream = "Alpha values";
        break;
        case 1:
        value = "WebP lossless format";
        alpha_bitstream = "Compressed alpha values";
        break;
        default:
        value = "<span foreground=\"red\">INVALID</span>";
        alpha_bitstream = "Unrecognized data";
    }
    format_utils_add_line_tab (selected_tab, "Compression method", value,
                               "Compression method\n"
                               "<tt>00<sub>2</sub></tt>\tNo compression\n"
                               "<tt>01<sub>2</sub></tt>\tWebP lossless format");

    if (!extra_data->anmf_found)
        format_utils_insert_tab (file, &tab,
                                 riff_file->chunk_types[riff_file->chunk_selection]);

    if (chunk_size)
        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, chunk_size - 1,
                                alpha_bitstream, NULL);
}
