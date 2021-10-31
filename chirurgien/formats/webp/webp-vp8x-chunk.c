/* webp-vp8x-chunk.c
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
webp_vp8x_chunk (FormatsFile *file,
                 G_GNUC_UNUSED RiffFile *riff_file,
                 guint32      chunk_size)
{
    gchar *value;

    guchar flags[4];
    guint32 four_bytes;

    format_utils_start_section (file, "Extended image details");

    /* Flags */
    if (!format_utils_read (file, &flags, 4))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 4,
                            "Flags\n"
                            "Bit 0: Reserved\n"
                            "Bit 1: Image is animated\n"
                            "Bit 2: XMP metadata available\n"
                            "Bit 3: Exif metadata available\n"
                            "Bit 4: Alpha used\n"
                            "Bit 5: ICC profile used\n"
                            "Bits 6-7: Reserved\n"
                            "Bits 8-31: Reserved/unused", NULL);

    format_utils_add_line (file, "ICC profile used", (flags[0] >> 5) & 0x1 ? "Yes" : "No",
                           "ICC profile used flag\n"
                           "Set if the file contains an ICC profile");
    format_utils_add_line (file, "Alpha used", (flags[0] >> 4) & 0x1 ? "Yes" : "No",
                           "Alpha used flag\n"
                           "Set if any of the frames of the image contain transparency information");
    format_utils_add_line (file, "Exif metadata available", (flags[0] >> 3) & 0x1 ? "Yes" : "No",
                           "Exif metadata available flag\n"
                           "Set if the file contains Exif metadata.");
    format_utils_add_line (file, "XMP metadata available", (flags[0] >> 2) & 0x1 ? "Yes" : "No",
                           "XMP metadata available flag\n"
                           "Set if the file contains XMP metadata");
    format_utils_add_line (file, "Image is animated", (flags[0] >> 1) & 0x1 ? "Yes" : "No",
                           "Image is animated flag\n"
                           "Set if this is an animated image");

    four_bytes = 0;

    /* Canvas width */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 3,
                            "Canvas width", NULL);

    value = g_strdup_printf ("%u", four_bytes + 1);
    format_utils_add_line (file, "Canvas width", value, NULL);
    g_free (value);

    /* Canvas height */
    if (!format_utils_read (file, &four_bytes, 3))
        return;

    format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 3,
                            "Canvas height", NULL);

    value = g_strdup_printf ("%u", four_bytes + 1);
    format_utils_add_line (file, "Canvas height", value, NULL);
    g_free (value);

    if (chunk_size > 10)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_size - 10,
                                "Unrecognized data", NULL);
}
