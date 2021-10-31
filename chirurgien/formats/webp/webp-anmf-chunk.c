/* webp-anmf-chunk.c
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
webp_anmf_chunk (FormatsFile *file,
                 RiffFile    *riff_file,
                 G_GNUC_UNUSED guint32 chunk_size)
{
    WebpExtraData *extra_data;
    gchar *value;

    guchar flags;
    guint32 four_bytes = 0;

    extra_data = riff_file->extra_data;

    format_utils_start_section_tab (extra_data->frames_tab, "Animation frame details");
    extra_data->anmf_found = TRUE;

    /* Frame X */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 3,
                            "Frame X", NULL);

    value = g_strdup_printf ("%u", four_bytes << 1);
    format_utils_add_line_tab (extra_data->frames_tab, "Frame X", value,
                               "Calculated as Frame X * 2");
    g_free (value);

    /* Frame Y */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 3,
                            "Frame Y", NULL);

    value = g_strdup_printf ("%u", four_bytes << 1);
    format_utils_add_line_tab (extra_data->frames_tab, "Frame Y", value,
                               "Calculated as Frame Y * 2");
    g_free (value);

    /* Frame width */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 3,
                            "Frame width", NULL);

    value = g_strdup_printf ("%u", four_bytes + 1);
    format_utils_add_line_tab (extra_data->frames_tab, "Frame width", value,
                               "Calculated as Frame width + 1");
    g_free (value);

    /* Frame height */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 3,
                            "Frame height", NULL);

    value = g_strdup_printf ("%u", four_bytes + 1);
    format_utils_add_line_tab (extra_data->frames_tab, "Frame height", value,
                               "Calculated as Frame height + 1");
    g_free (value);

    /* Frame duration */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 3,
                            "Frame duration", NULL);

    value = g_strdup_printf ("%u", four_bytes);
    format_utils_add_line_tab (extra_data->frames_tab, "Frame duration", value,
                               "The time to wait before displaying the next frame, in milliseconds\n"
                               "A duration of 0 is implementation defined");
    g_free (value);

    /* Flags */
    if (!format_utils_read (file, &flags, 1))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 1,
                            "Flags\n"
                            "Bit 0: Disposal method\n"
                            "Bit 1: Blending method\n"
                            "Bits 2-7: Reserved", NULL);

    format_utils_add_line_tab (extra_data->frames_tab, "Blending method",
                               (flags >> 1) & 0x1 ? "Do not blend" : "Alpha blending",
                               "Blending method\n"
                               "<tt>0<sub>2</sub></tt>\tAlpha blending\n"
                               "<tt>1<sub>2</sub></tt>\tDo not blend");

    format_utils_add_line_tab (extra_data->frames_tab, "Disposal method",
                               flags & 0x1 ? "Dispose to background color" : "Do not dispose",
                               "Disposal method\n"
                               "<tt>0<sub>2</sub></tt>\tDo not dispose\n"
                               "<tt>1<sub>2</sub></tt>\tDispose to background color");
}
