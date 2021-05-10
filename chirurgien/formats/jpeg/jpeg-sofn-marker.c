/* jpeg-sofn-marker.c
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

#include <config.h>

#include <glib/gi18n.h>

#include "jpeg-format.h"

#define SOF_LENGTH_WITHOUT_COMPONENTS 8
#define SOF_COMPONENT_LENGTH          3


gboolean
jpeg_sofn_marker (FormatsFile *file,
                  gint        *marker_counts,
                  gint         sof_marker,
                  const gchar *compression_method)
{
    gchar *value;

    guint16 data_length, data_used = SOF_LENGTH_WITHOUT_COMPONENTS;
    guint8 components;

    guint8 one_byte, sampling_factor;

    /* Data length */
    if (!jpeg_data_length (file, &data_length))
        return FALSE;

    for (gint i = SOF0; i <= SOF15; i++)
    {
        if (marker_counts[i])
        {
            if (data_length > 1)
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - 2,
                                      _("SOF segment already defined"), NULL);
            return TRUE;
        }
    }

    marker_counts[sof_marker]++;

    format_utils_start_section (file, _("Image details"));

    format_utils_add_line (file, _("Compression method"), compression_method,
                         _("Compression method\n"
                           "SOF0: Baseline DCT\n"
                           "SOF1: Extended sequential DCT - Huffman coding\n"
                           "SOF2: Progressive DCT - Huffman coding\n"
                           "SOF3: Lossless (sequential) - Huffman coding\n"
                           "SOF5: Differential sequential DCT - Huffman coding\n"
                           "SOF6: Differential progressive DCT - Huffman coding\n"
                           "SOF7: Differential lossless (sequential) - Huffman coding\n"
                           "SOF9: Extended sequential DCT - Arithmetic coding\n"
                           "SOF10: Progressive DCT - Arithmetic coding\n"
                           "SOF11: Lossless (sequential) - Arithmetic coding\n"
                           "SOF13: Differential sequential DCT - Arithmetic coding\n"
                           "SOF14: Differential progressive DCT - Arithmetic coding\n"
                           "SOF15: Differential lossless (sequential) - Arithmetic coding"));

    /* Sample precision */
    if (!process_jpeg_field (file, NULL, _("Sample precision"), NULL,
                             _("Number of bits in the samples of the components"),
                             MARKER_DATA_COLOR_1, 1,
                             0, NULL, NULL, _("%u bits"), NULL))
        return FALSE;

    /* Image height */
    if (!process_jpeg_field (file, NULL, _("Image height"), NULL,
                             _("Minimum value: 0 (DNL marker defines height)\n"
                               "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                             MARKER_DATA_COLOR_2, 2,
                             0, NULL, NULL, "%u", NULL))
        return FALSE;

    /* Image width */
    if (!process_jpeg_field (file, NULL, _("Image width"), NULL,
                             _("Minimum value: 1\n"
                               "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"),
                             MARKER_DATA_COLOR_1, 2,
                             0, NULL, NULL, "%u", NULL))
        return FALSE;

    /* Number of components */
    if (!process_jpeg_field (file, NULL, _("Number of components"), NULL,
                             _("Scan components in the image\n"
                               "Progressive DCT: 1-4 components\n"
                               "All other cases: 1-255 components"),
                             MARKER_DATA_COLOR_2, 1,
                             0, NULL, NULL, "%u", &components))
        return FALSE;

    format_utils_start_section (file, _("Image components"));

    while (components)
    {
        /* Component identifier */
        if (!format_utils_read (file, &one_byte, 1))
            return FALSE;

        format_utils_add_field (file, COMPONENT_COLOR, TRUE, 1,
                              _("Component identifier"), NULL);

        value = g_strdup_printf ("%hhu", one_byte);
        format_utils_add_line (file, _("Component identifier"),
                               value, NULL);
        g_free (value);

        /* Sampling factor */
        if (!format_utils_read (file, &one_byte, 1))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_1, TRUE, 1,
                              _("Sampling factor\n"
                                "Lower four bits: Vertical sampling factor\n"
                                "Upper four bits: Horizontal sampling factor"), NULL);

        sampling_factor = one_byte & 0x0F;
        if (sampling_factor && sampling_factor < 4)
            value = g_strdup_printf ("%hhu", sampling_factor);
        else
            value = g_strdup (_("<span foreground=\"red\">INVALID</span>"));

        format_utils_add_line (file, _("Vertical sampling factor"), value,
                             _("Relationship between the component vertical dimension and maximum image dimension Y\n"
                               "Valid values: 1-4"));
        g_free (value);

        sampling_factor = one_byte >> 4;
        if (sampling_factor && sampling_factor < 4)
            value = g_strdup_printf ("%hhu", sampling_factor);
        else
            value = g_strdup (_("<span foreground=\"red\">INVALID</span>"));

        format_utils_add_line (file, _("Horizontal sampling factor"), value,
                             _("Relationship between the component horizontal dimension and maximum image dimension X\n"
                               "Valid values: 1-4"));
        g_free (value);

        /* Quantization table selector */
        if (!format_utils_read (file, &one_byte, 1))
            return FALSE;

        format_utils_add_field (file, MARKER_DATA_COLOR_2, TRUE, 1,
                              _("Quantization table selector"), NULL);

        if (one_byte < 4)
            value = g_strdup_printf ("%hhu", one_byte);
        else
            value = g_strdup (_("<span foreground=\"red\">INVALID</span>"));

        if (components == 1)
            format_utils_add_line (file, _("Quantization table selector"), value, NULL);
        else
            format_utils_add_line_full (file, _("Quantization table selector"), value, NULL, 0, 10);
        g_free (value);

        components--;
        data_used += SOF_COMPONENT_LENGTH;
    }

    if (data_used < data_length)
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, data_length - data_used,
                              _("Unrecognized data"), NULL);

    return TRUE;
}
