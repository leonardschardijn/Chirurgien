/* webp-vp8l-chunk.c
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
webp_vp8l_chunk (FormatsFile *file,
                 RiffFile    *riff_file,
                 guint32      chunk_size)
{
    WebpExtraData *extra_data;
    gchar *value;

    guchar transformations[4];
    guint32 four_bytes;
    gint width_height = 0, transformation_type;

    extra_data = riff_file->extra_data;

    if (!extra_data->anmf_found)
        format_utils_start_section (file, "Image details (lossless compression)");
    else
        format_utils_start_section_tab (extra_data->frames_tab, "Image details (lossless compression)");

    /* Signature byte */
    if (!format_utils_read (file, &transformations, 1))
        return;

    /* Use this variable because it is available */
    if (transformations[0] == 0x2F)
    {
        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, 1,
                                "Signature byte", NULL);

        /* Dimensions, alpha, version */
        if (!format_utils_read (file, &four_bytes, 4))
            return;

        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_2, TRUE, 4,
                                "Dimensions, alpha, version\n"
                                "Bits 0-14: Image width\n"
                                "Bits 15-27: Image height\n"
                                "Bits 28: Alpha used flag\n"
                                "Bits 29-31: Version (not used)", NULL);

        /* First 14 bits */
        width_height = four_bytes & 0x3FFF;

        value = g_strdup_printf ("%u", width_height + 1);
        if (!extra_data->anmf_found)
            format_utils_add_line (file, "Image width", value,
                                   "Calculated as Image width + 1\n"
                                   "Minimum value: 1\n"
                                   "Maximum value: 16384");
        else
            format_utils_add_line_tab (extra_data->frames_tab, "Image width", value,
                                       "Calculated as Image width + 1\n"
                                       "Minimum value: 1\n"
                                       "Maximum value: 16384");
        g_free (value);

        /* Next 14 bits */
        width_height = (four_bytes >> 14) & 0x3FFF;

        value = g_strdup_printf ("%u", width_height + 1);
        if (!extra_data->anmf_found)
            format_utils_add_line (file, "Image height", value,
                                   "Calculated as Image height + 1\n"
                                   "Minimum value: 1\n"
                                   "Maximum value: 16384");
        else
            format_utils_add_line_tab (extra_data->frames_tab, "Image height", value,
                                       "Calculated as Image height + 1\n"
                                       "Minimum value: 1\n"
                                       "Maximum value: 16384");
        g_free (value);

        value = (four_bytes >> 28) & 0x1 ? "Yes" : "No";
        if (!extra_data->anmf_found)
            format_utils_add_line (file, "Alpha used", value,
                                   "Alpha used flag\n"
                                   "Set to 0 if all alpha values are 255");
        else
            format_utils_add_line_tab (extra_data->frames_tab, "Alpha used", value,
                                       "Alpha used flag\n"
                                       "Set to 0 if all alpha values are 255");

        if (chunk_size >= 4)
            chunk_size -= 4;

        if (chunk_size && format_utils_read (file, &four_bytes, 4))
        {
            transformations[0] =
            transformations[1] =
            transformations[2] =
            transformations[3] = 0;

            while (four_bytes & 0x1)
            {
                four_bytes = four_bytes >> 1;
                transformation_type = four_bytes & 0x3;
                four_bytes = four_bytes >> 2;

                switch (transformation_type)
                {
                    /* PREDICTOR_TRANSFORM */
                    case 0:
                        if (transformations[0])
                            four_bytes = 0;
                        four_bytes = four_bytes >> 3;
                        transformations[0] = 1;
                    break;
                    /* COLOR_TRANSFORM */
                    case 1:
                        if (transformations[1])
                            four_bytes = 0;
                        four_bytes = four_bytes >> 3;
                        transformations[1] = 1;
                    break;
                    /* SUBTRACT_GREEN */
                    case 2:
                        if (transformations[2])
                            four_bytes = 0;
                        transformations[2] = 1;
                    break;
                    /* COLOR_INDEXING_TRANSFORM */
                    case 3:
                        if (transformations[3])
                            four_bytes = 0;
                        four_bytes = four_bytes >> 8;
                        transformations[3] = 1;
                    break;
                }
            }

            if (!extra_data->anmf_found)
            {
                format_utils_start_section (file, "Transformations");
                format_utils_add_line (file, "Predictor transform",
                                       transformations[0] ? "Used" : "Not used",
                                       "If the Predictor transform is used");
                format_utils_add_line (file, "Color transform",
                                       transformations[1] ? "Used" : "Not used",
                                       "If the Color transform is used");
                format_utils_add_line (file, "Subtract Green transform",
                                       transformations[2] ? "Used" : "Not used",
                                       "If the Subtract Green transform is used");
                format_utils_add_line (file, "Color Indexing transform",
                                       transformations[3] ? "Used" : "Not used",
                                       "If the Color Indexing transform is used");
            }
            else
            {
                format_utils_start_section_tab (extra_data->frames_tab, "Transformations");
                format_utils_add_line_tab (extra_data->frames_tab, "Predictor transform",
                                           transformations[0] ? "Used" : "Not used",
                                           "If the Predictor transform is used");
                format_utils_add_line_tab (extra_data->frames_tab, "Color transform",
                                           transformations[1] ? "Used" : "Not used",
                                           "If the Color transform is used");
                format_utils_add_line_tab (extra_data->frames_tab, "Subtract Green transform",
                                           transformations[2] ? "Used" : "Not used",
                                           "If the Subtract Green transform is used");
                format_utils_add_line_tab (extra_data->frames_tab, "Color Indexing transform",
                                           transformations[3] ? "Used" : "Not used",
                                           "If the Color Indexing transform is used");
            }
        }

        format_utils_add_field (file, CHUNK_PAYLOAD_COLOR_1, TRUE, chunk_size,
                                "Transformations and compressed image data", NULL);
    }
    else
    {
        format_utils_add_field (file, ERROR_COLOR_2, FALSE, 1,
                                "Invalid signature byte", NULL);

        if (chunk_size > 1)
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_size - 1,
                                    "Unrecognized data", NULL);
    }
}
