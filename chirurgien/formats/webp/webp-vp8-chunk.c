/* webp-vp8-chunk.c
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


static void dimension_scaling (FormatsFile *,
                               guint16,
                               const char *,
                               WebpExtraData *);

void
webp_vp8_chunk (FormatsFile *file,
                RiffFile    *riff_file,
                guint32      chunk_size)
{
    WebpExtraData *extra_data;
    gchar *value;

    guchar three_bytes[3];
    guint frame_tag;
    guint16 width_height;

    extra_data = riff_file->extra_data;

    if (!extra_data->anmf_found)
        format_utils_start_section (file, "Frame details (lossy compression)");
    else
        format_utils_start_section_tab (extra_data->frames_tab, "Frame details (lossy compression)");

    /* Frame tag */
    if (!format_utils_read (file, &three_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 3,
                            "Frame tag\n"
                            "Bit 0: Frame type\n"
                            "Bits 1-3: Version\n"
                            "Bit 4: Show frame flag\n"
                            "Bits 5-23: First data partition size", NULL);

    frame_tag = three_bytes[0] |
               (three_bytes[1] << 8) |
               (three_bytes[2] << 16);

    /* Frame type */
    value = (frame_tag & 0x1) ? "Interframe" : "Key frame";

    if (!extra_data->anmf_found)
        format_utils_add_line (file, "Frame type", value,
                               "Frame type (bit 0 of Frame tag)\n"
                               "<tt>0<sub>2</sub></tt>\tKey frame\n"
                               "<tt>1<sub>2</sub></tt>\tInterframe");
    else
        format_utils_add_line_tab (extra_data->frames_tab, "Frame type", value,
                                   "Frame type (bit 0 of Frame tag)\n"
                                   "<tt>0<sub>2</sub></tt>\tKey frame\n"
                                   "<tt>1<sub>2</sub></tt>\tInterframe");

    /* Version */
    switch ((frame_tag >> 1) & 0x7)
    {
        case 0:
        value = "Version 0 (Bicubic - Normal)";
        break;
        case 1:
        value = "Version 1 (Bilinear - Simple)";
        break;
        case 2:
        value = "Version 2 (Bilinear - None)";
        break;
        case 3:
        value = "Version 3 (None - None)";
        break;
        default:
        value = "<span foreground=\"red\">INVALID</span>";
    }
    if (!extra_data->anmf_found)
        format_utils_add_line (file, "Version", value,
                               "Version (Reconstruction filter - Loop filter)\n"
                               "<tt>000<sub>2</sub></tt>\tVersion 0 (Bicubic - Normal)\n"
                               "<tt>001<sub>2</sub></tt>\tVersion 1 (Bilinear - Simple)\n"
                               "<tt>010<sub>2</sub></tt>\tVersion 2 (Bilinear - None)\n"
                               "<tt>011<sub>2</sub></tt>\tVersion 3 (None - None)");
    else
        format_utils_add_line_tab (extra_data->frames_tab, "Version", value,
                                   "Version (Reconstruction filter - Loop filter)\n"
                                   "<tt>000<sub>2</sub></tt>\tVersion 0 (Bicubic - Normal)\n"
                                   "<tt>001<sub>2</sub></tt>\tVersion 1 (Bilinear - Simple)\n"
                                   "<tt>010<sub>2</sub></tt>\tVersion 2 (Bilinear - None)\n"
                                   "<tt>011<sub>2</sub></tt>\tVersion 3 (None - None)");

    /* Show frame */
    value = (frame_tag >> 4) & 0x1 ? "Yes" : "No";

    if (!extra_data->anmf_found)
        format_utils_add_line (file, "Show frame", value,
                               "Show frame (bit 4 of Frame tag)\n"
                               "If the current frame is for display");
    else
        format_utils_add_line_tab (extra_data->frames_tab, "Show frame", value,
                                   "Show frame (bit 4 of Frame tag)\n"
                                   "If the current frame is for display");

    /* Sync code */
    if (!format_utils_read (file, &three_bytes, 3))
        return;

    if (three_bytes[0] == 0x9D &&
        three_bytes[1] == 0x01 &&
        three_bytes[2] == 0x2A)
    {
        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 3,
                                "Sync code", NULL);

        /* Width */
        if (!format_utils_read (file, &width_height, 2))
            return;

        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 2,
                                "Frame width and horizontal scale\n"
                                "Bits 0-13: Width\n"
                                "Bits 14-15: Horizontal scale", NULL);

        value = g_strdup_printf ("%hu", (guint16) (width_height & 0x3FFF));
        if (!extra_data->anmf_found)
            format_utils_add_line (file, "Frame width", value, NULL);
        else
            format_utils_add_line_tab (extra_data->frames_tab, "Frame width", value, NULL);
        g_free (value);

        dimension_scaling (file, width_height, "Horizontal scale", extra_data);

        /* Height */
        if (!format_utils_read (file, &width_height, 2))
            return;

        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 2,
                                "Frame height and vertical scale\n"
                                "Bits 0-13: Height\n"
                                "Bits 14-15: Vertical scale", NULL);

        value = g_strdup_printf ("%hu", (guint16) (width_height & 0x3FFF));
        if (!extra_data->anmf_found)
            format_utils_add_line (file, "Frame height", value, NULL);
        else
            format_utils_add_line_tab (extra_data->frames_tab, "Frame height", value, NULL);
        g_free (value);

        dimension_scaling (file, width_height, "Vertical scale", extra_data);

        if (chunk_size > 10)
            format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, chunk_size - 10,
                                    "Compressed VP8 data partitions", NULL);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_2, FALSE, 3,
                                "Invalid sync code", NULL);

        if (chunk_size > 6)
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_size - 6,
                                    "Unrecognized data", NULL);
    }
}

static void
dimension_scaling (FormatsFile   *file,
                   guint16        scale,
                   const char    *dimension,
                   WebpExtraData *extra_data)
{
    const gchar *value;

    switch (scale >> 14)
    {
        case 0:
        value = "No scaling";
        break;
        case 1:
        value = "5/4";
        break;
        case 2:
        value = "5/3";
        break;
        case 3:
        value = "2";
        break;
    }

    if (!extra_data->anmf_found)
        format_utils_add_line (file, dimension, value,
                               "Scaling\n"
                               "<tt>00<sub>2</sub></tt>\tNo upscaling\n"
                               "<tt>01<sub>2</sub></tt>\tUpscale by 5/4\n"
                               "<tt>10<sub>2</sub></tt>\tUpscale by 5/3\n"
                               "<tt>11<sub>2</sub></tt>\tUpscale by 2");
    else
        format_utils_add_line_tab (extra_data->frames_tab, dimension, value,
                                   "Scaling\n"
                                   "<tt>00<sub>2</sub></tt>\tNo upscaling\n"
                                   "<tt>01<sub>2</sub></tt>\tUpscale by 5/4\n"
                                   "<tt>10<sub>2</sub></tt>\tUpscale by 5/3\n"
                                   "<tt>11<sub>2</sub></tt>\tUpscale by 2");
}
