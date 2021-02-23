/* jpeg-analyzer.c
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

#include "jpeg-analyzer.h"

#include <string.h>
#include <glib/gi18n.h>

#include "chirurgien-analyze-jpeg.h"


gchar *marker_names[MARKER_TYPES] = {
    "SOI",
    "APP0",
    "APP1",
    "APP2",
    "DQT",
    "DHT",
    "DAC",
    "SOF0",
    "SOF1",
    "SOF2",
    "SOF3",
    "SOF5",
    "SOF6",
    "SOF7",
    "SOF9",
    "SOF10",
    "SOF11",
    "SOF13",
    "SOF14",
    "SOF15",
    "DRI",
    "SOS",
    "DNL",
    "COM",
    "EOI"
};

void
chirurgien_analyze_jpeg (AnalyzerFile *file)
{
    const guchar marker_types[MARKER_TYPES][2] =
    {
        { 0xFF,0xD8 }, // SOI
        { 0xFF,0xE0 }, // APP0
        { 0xFF,0xE1 }, // APP1
        { 0xFF,0xE2 }, // APP2
        { 0xFF,0xDB }, // DQT
        { 0xFF,0xC4 }, // DHT
        { 0xFF,0xCC }, // DAC
        { 0xFF,0xC0 }, // SOF0
        { 0xFF,0xC1 }, // SOF1
        { 0xFF,0xC2 }, // SOF2
        { 0xFF,0xC3 }, // SOF3
        { 0xFF,0xC5 }, // SOF5
        { 0xFF,0xC6 }, // SOF6
        { 0xFF,0xC7 }, // SOF7
        { 0xFF,0xC9 }, // SOF9
        { 0xFF,0xCA }, // SOF10
        { 0xFF,0xCB }, // SOF11
        { 0xFF,0xCD }, // SOF13
        { 0xFF,0xCE }, // SOF14
        { 0xFF,0xCF }, // SOF15
        { 0xFF,0xDD }, // DRI
        { 0xFF,0xDA }, // SOS
        { 0xFF,0xDC }, // DNL
        { 0xFF,0xFE }, // COM
        { 0xFF,0xD9 }, // EOI
    };

    guint marker_counts[MARKER_TYPES];

    gchar *description_message;

    guchar marker_type[2];
    guint16 unrecognized_data;

    memset (marker_counts, 0, sizeof (guint) * MARKER_TYPES);

    analyzer_utils_set_title (file, "<span weight=\"bold\" size=\"larger\">"
                                    "Joint Photographic Experts Group"
                                    "</span>");

    /* Marker loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at EOI marker */
        if (marker_counts[EOI])
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data, file ends at EOI marker"));
            break;
        }

        /* Marker type */
        if (!analyzer_utils_read (&marker_type, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            break;
        }

        /* Analyze marker data */
        if (!memcmp (marker_type, marker_types[SOI], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOI"), marker_names[SOI]);
            marker_counts[SOI]++;
        }
        else if (!memcmp (marker_type, marker_types[APP0], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: APP0"), marker_names[APP0]);

            if (!analyze_app0_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[APP1], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: APP1"), marker_names[APP1]);

            if (!analyze_app1_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[APP2], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: APP2"), marker_names[APP2]);

            if (!analyze_app2_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DQT], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: DQT"), marker_names[DQT]);

            if (!analyze_dqt_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DHT], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: DHT"), marker_names[DHT]);

            if (!analyze_dht_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DAC], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: DAC"), marker_names[DAC]);

            if (!analyze_dac_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF0], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF0"), marker_names[SOF0]);

            if (!marker_counts[SOF0])
                analyzer_utils_set_subtitle (file, _("<b>Baseline DCT</b>"),
                                             _("Baseline Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF0))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF1], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF1"), marker_names[SOF1]);

            if (!marker_counts[SOF1])
                analyzer_utils_set_subtitle (file, _("<b>Extended sequential DCT - Huffman coding</b>"),
                                             _("Extended sequential Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF1))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF2], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF2"), marker_names[SOF2]);

            if (!marker_counts[SOF2])
                analyzer_utils_set_subtitle (file, _("<b>Progressive DCT - Huffman coding</b>"),
                                             _("Progressive Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF2))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF3], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF3"), marker_names[SOF3]);

            if (!marker_counts[SOF3])
                analyzer_utils_set_subtitle (file, _("<b>Lossless (sequential) - Huffman coding</b>"),
                                             NULL);

            if (!analyze_sofn_marker (file, marker_counts, SOF3))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF5], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF5"), marker_names[SOF5]);

            if (!marker_counts[SOF5])
                analyzer_utils_set_subtitle (file, _("<b>Differential sequential DCT - Huffman coding</b>"),
                                             _("Differential sequential Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF5))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF6], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF6"), marker_names[SOF6]);

            if (!marker_counts[SOF6])
                analyzer_utils_set_subtitle (file, _("<b>Differential progressive DCT - Huffman coding</b>"),
                                             _("Differential progressive Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF6))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF7], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF7"), marker_names[SOF7]);

            if (!marker_counts[SOF7])
                analyzer_utils_set_subtitle (file, _("<b>Differential lossless (sequential) - Huffman coding</b>"),
                                             NULL);

            if (!analyze_sofn_marker (file, marker_counts, SOF7))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF9], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF9"), marker_names[SOF9]);

            if (!marker_counts[SOF9])
                analyzer_utils_set_subtitle (file, _("<b>Extended sequential DCT - Arithmetic coding</b>"),
                                             _("Extended sequential Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF9))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF10], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF10"), marker_names[SOF10]);

            if (!marker_counts[SOF10])
                analyzer_utils_set_subtitle (file, _("<b>Progressive DCT - Arithmetic coding</b>"),
                                             _("Progressive Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF10))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF11], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF11"), marker_names[SOF11]);

            if (!marker_counts[SOF11])
                analyzer_utils_set_subtitle (file, _("<b>Lossless (sequential) - Arithmetic coding</b>"),
                                             NULL);

            if (!analyze_sofn_marker (file, marker_counts, SOF11))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF13], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF13"), marker_names[SOF13]);

            if (!marker_counts[SOF13])
                analyzer_utils_set_subtitle (file, _("<b>Differential sequential DCT - Arithmetic coding</b>"),
                                             _("Differential sequential Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF13))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF14], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF14"), marker_names[SOF14]);

            if (!marker_counts[SOF14])
                analyzer_utils_set_subtitle (file, _("<b>Differential progressive DCT - Arithmetic coding</b>"),
                                             _("Differential progressive Discrete Cosine Transform"));

            if (!analyze_sofn_marker (file, marker_counts, SOF14))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF15], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOF15"), marker_names[SOF15]);

            if (!marker_counts[SOF15])
                analyzer_utils_set_subtitle (file, _("<b>Differential lossless (sequential) - Arithmetic coding</b>"),
                                             NULL);

            if (!analyze_sofn_marker (file, marker_counts, SOF15))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DRI], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: DRI"), marker_names[DRI]);

            if (!analyze_dri_dnl_marker (file, marker_counts, DRI))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOS], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: SOS"), marker_names[SOS]);

            if (!analyze_sos_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DNL], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: DNL"), marker_names[DNL]);

            if (!analyze_dri_dnl_marker (file, marker_counts, DNL))
                break;
        }
        else if (!memcmp (marker_type, marker_types[COM], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: COM"), marker_names[COM]);

            if (!analyze_com_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[EOI], 2))
        {
            analyzer_utils_tag_navigation (file, MARKER_TYPE_COLOR, 2, _("Marker type: EOI"), marker_names[EOI]);
            marker_counts[EOI]++;
        }
        else
        {
            analyzer_utils_tag_navigation_error (file, ERROR_COLOR_2, 2, _("Marker type: Unknown"), "???");

            /* Data length */
            if (!analyzer_utils_read (&unrecognized_data, file , 2))
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
                break;
            }
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Unknown marker data length"));

            unrecognized_data = g_ntohs (unrecognized_data);
            if (unrecognized_data > 2)
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, unrecognized_data - 2, _("Unrecognized data"));
                ADVANCE_POINTER (file, unrecognized_data - 2);
            }
        }
    }

    analyzer_utils_set_subtitle (file, _("<b>Marker count</b>"), NULL);

    for (gint i = SOI; i < EOI; i++)
    {
        if (marker_counts[i])
        {
            description_message = g_strdup_printf ("%u", marker_counts[i]);
            analyzer_utils_describe (file, marker_names[i], description_message);
            g_free (description_message);
        }
    }
}

gboolean
process_jpeg_field (AnalyzerFile *file,
                   AnalyzerTab *tab,
                   gchar *field_name,
                   gchar *field_name_tag,
                   gchar *field_tooltip,
                   GdkRGBA *color,
                   guint field_length,
                   guint possible_values,
                   guint8 *field_values,
                   gchar **value_descriptions,
                   gchar *description_message,
                   void *read_value)
{
    gchar *field_description = NULL;
    guint32 four_bytes = 0;

    if (!analyzer_utils_read (&four_bytes, file, field_length))
        return FALSE;

    if (field_length == 2)
        four_bytes = g_ntohs (four_bytes);
    else if (field_length == 4)
        four_bytes = g_ntohl (four_bytes);

    if (field_name_tag)
        analyzer_utils_tag (file, color, field_length, field_name_tag);
    else
        analyzer_utils_tag (file, color, field_length, field_name);

    if (possible_values)
    {
        for (guint i = 0; i < possible_values; i++)
        {
            if (four_bytes == field_values[i])
            {
                field_description = value_descriptions[i];
                break;
            }
        }

        if (!field_description)
            field_description = value_descriptions[possible_values];

        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, field_name, field_description, field_tooltip);
        else
            analyzer_utils_describe_tooltip (file, field_name, field_description, field_tooltip);
    }
    else if (description_message)
    {

        field_description = g_strdup_printf (description_message, four_bytes);
        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, field_name, field_description, field_tooltip);
        else
            analyzer_utils_describe_tooltip (file, field_name, field_description, field_tooltip);
        g_free (field_description);
    }

    if (read_value)
    {
        if (field_length == 1)
            *(guint8 *) read_value = four_bytes;
        else if (field_length == 2)
            *(guint16 *) read_value = four_bytes;
        else if (field_length == 4)
            *(guint32 *) read_value = four_bytes;
    }

    return TRUE;
}
