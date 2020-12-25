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

#include <arpa/inet.h>
#include <glib/gi18n.h>

#include "jpeg-analyzer.h"


gboolean
analyze_sofn_marker (AnalyzerFile *file, guint *marker_counts, gint sof_marker)
{
    gchar *description_message;

    guint16 data_length, data_used = 0;
    guint8 components;

    guint8 one_byte, sampling_factor;
    guint16 two_bytes;

    /* Data length */
    if (!analyzer_utils_read (&data_length, file , 2))
        goto END_ERROR;

    data_length = ntohs (data_length);
    analyzer_utils_tag (file, MARKER_LENGTH_COLOR, 2, _("Data length"));

    if (marker_counts[sof_marker])
    {
        if (data_length > 2)
            analyzer_utils_create_tag (file, ERROR_COLOR_1, FALSE, data_length,
                                       _("Segment already defined"), NULL);
        return TRUE;
    }

    /* Sample precision */
    if (!analyzer_utils_read (&one_byte, file , 1))
        return FALSE;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1, _("Sample precision"));

    description_message = g_strdup_printf (_("%hhu bits"), one_byte);
    analyzer_utils_describe_tooltip (file, _("Sample precision"), description_message,
                             _("Number of bits in the samples of the components"));
    g_free (description_message);

    /* Image height */
    if (!analyzer_utils_read (&two_bytes, file , 2))
        goto END_ERROR;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 2, _("Image height"));

    two_bytes = ntohs (two_bytes);
    description_message = g_strdup_printf ("%hu", two_bytes);
    analyzer_utils_describe_tooltip (file, _("Image height"), description_message,
                                     _("Minimum value: 0 (DNL marker defines height)\n"
                                     "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"));
    g_free (description_message);

    /* Image width */
    if (!analyzer_utils_read (&two_bytes, file , 2))
        goto END_ERROR;

    analyzer_utils_create_tag (file, MARKER_DATA_COLOR_1, TRUE, 2,
                               _("Image width"), NULL);

    two_bytes = ntohs (two_bytes);
    description_message = g_strdup_printf ("%hu", two_bytes);
    analyzer_utils_describe_tooltip (file, _("Image width"), description_message,
                                     _("Minimum value: 1\n"
                                     "Maximum value: 2<sup>16</sup> - 1 (unsigned 16-bit integer)"));
    g_free (description_message);

    /* Number of components */
    if (!analyzer_utils_read (&components, file , 1))
        return FALSE;

    analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Number of components"));

    data_used += 8;

    if (components)
        description_message = g_strdup_printf ("%hhu", components);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip (file, _("Number of components"), description_message,
                                     _("Scan components in the image\n"
                                     "Progressive DCT\t1-4 components\n"
                                     "All other cases\t1-255 components"));
    g_free (description_message);

    analyzer_utils_add_description (file, _("<b>Components</b>"), NULL, NULL, 20, 10);

    while (components)
    {
        /* Component identifier */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, COMPONENT_COLOR, 1, _("Component identifier"));

        description_message = g_strdup_printf ("%hhu", one_byte);
        analyzer_utils_add_description (file, _("Component identifier"), description_message, NULL, 10, 0);
        g_free (description_message);

        /* Sampling factor */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_1, 1,
                            _("Sampling factor\n"
                            "Lower four bits: Vertical sampling factor\n"
                            "Upper four bits: Horizontal sampling factor"));

        sampling_factor = one_byte & 0x0F;
        if (sampling_factor && sampling_factor < 4)
            description_message = g_strdup_printf ("%hhu", sampling_factor);
        else
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

        analyzer_utils_describe_tooltip (file, _("Vertical sampling factor"), description_message,
                                        _("Sampling factor\n"
                                         "<tt>1<sub>16</sub></tt>\t1\n"
                                         "<tt>2<sub>16</sub></tt>\t2\n"
                                         "<tt>3<sub>16</sub></tt>\t3\n"
                                         "<tt>4<sub>16</sub></tt>\t4"));
        g_free (description_message);

        sampling_factor = one_byte >> 4;
        if (sampling_factor && sampling_factor < 4)
            description_message = g_strdup_printf ("%hhu", sampling_factor);
        else
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

        analyzer_utils_describe_tooltip (file, _("Horizontal sampling factor"), description_message,
                                         _("Sampling factor\n"
                                         "<tt>1<sub>16</sub></tt>\t1\n"
                                         "<tt>2<sub>16</sub></tt>\t2\n"
                                         "<tt>3<sub>16</sub></tt>\t3\n"
                                         "<tt>4<sub>16</sub></tt>\t4"));
        g_free (description_message);

        /* Quantization table selector */
        if (!analyzer_utils_read (&one_byte, file , 1))
            return FALSE;

        analyzer_utils_tag (file, MARKER_DATA_COLOR_2, 1, _("Quantization table selector"));

        if (one_byte < 4)
            description_message = g_strdup_printf ("%hhu", one_byte);
        else
            description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

        analyzer_utils_describe (file, _("Quantization table selector"), description_message);
        g_free (description_message);

        components--;
        data_used += 3;
    }

    if (data_used < data_length)
    {
        data_length -= data_used;
        analyzer_utils_tag_error (file, ERROR_COLOR_1, data_length, _("Unrecognized data"));

        ADVANCE_POINTER (file, data_length);
    }

    marker_counts[sof_marker]++;

    return TRUE;

    END_ERROR:
    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
    return FALSE;
}
