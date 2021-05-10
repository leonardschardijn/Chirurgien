/* jpeg-format.c
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

#include "jpeg-format.h"

#include <glib/gi18n.h>

#include "chirurgien-jpeg.h"


const gchar * const marker_names[MARKER_TYPES] = {
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
chirurgien_jpeg (FormatsFile *file)
{
    const guchar marker_types [MARKER_TYPES][2] =
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

    gint marker_counts[MARKER_TYPES + 3];

    gchar *value;

    guchar marker_type[2];
    guint16 unrecognized_data;

    memset (marker_counts, 0, sizeof (gint) * (MARKER_TYPES + 3));

    format_utils_set_title (file, "Joint Photographic Experts Group");

    format_utils_start_section (file, _("JFIF and/or Exif header"));

    /* Marker loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at EOI marker */
        if (marker_counts[EOI])
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                                  _("Unrecognized data, file ends at EOI marker"), NULL);
            break;
        }

        /* Marker type */
        if (!format_utils_read (file, &marker_type, 2))
            break;

        /* Marker data */
        if (!memcmp (marker_type, marker_types[SOI], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOI"), marker_names[SOI]);
            marker_counts[SOI]++;
        }
        else if (!memcmp (marker_type, marker_types[APP0], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: APP0"), marker_names[APP0]);

            if (!jpeg_app0_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[APP1], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: APP1"), marker_names[APP1]);

            if (!jpeg_app1_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[APP2], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: APP2"), marker_names[APP2]);

            if (!jpeg_app2_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DQT], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: DQT"), marker_names[DQT]);

            if (!jpeg_dqt_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DHT], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: DHT"), marker_names[DHT]);

            if (!jpeg_dht_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DAC], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: DAC"), marker_names[DAC]);

            if (!jpeg_dac_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF0], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF0"), marker_names[SOF0]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF0, "Baseline DCT"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF1], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF1"), marker_names[SOF1]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF1, "Extended sequential DCT - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF2], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF2"), marker_names[SOF2]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF2, "Progressive DCT - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF3], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF3"), marker_names[SOF3]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF3, "Lossless (sequential) - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF5], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF5"), marker_names[SOF5]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF5, "Differential sequential DCT - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF6], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF6"), marker_names[SOF6]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF6, "Differential progressive DCT - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF7], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF7"), marker_names[SOF7]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF7, "Differential lossless (sequential) - Huffman coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF9], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF9"), marker_names[SOF9]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF9, "Extended sequential DCT - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF10], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF10"), marker_names[SOF10]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF10, "Progressive DCT - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF11], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF11"), marker_names[SOF11]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF11, "Lossless (sequential) - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF13], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF13"), marker_names[SOF13]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF13, "Differential sequential DCT - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF14], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF14"), marker_names[SOF14]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF14, "Differential progressive DCT - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOF15], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOF15"), marker_names[SOF15]);

            if (!jpeg_sofn_marker (file, marker_counts, SOF15, "Differential lossless (sequential) - Arithmetic coding"))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DRI], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: DRI"), marker_names[DRI]);

            if (!jpeg_dri_dnl_marker (file, marker_counts, DRI))
                break;
        }
        else if (!memcmp (marker_type, marker_types[SOS], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: SOS"), marker_names[SOS]);

            if (!jpeg_sos_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[DNL], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: DNL"), marker_names[DNL]);

            if (!jpeg_dri_dnl_marker (file, marker_counts, DNL))
                break;
        }
        else if (!memcmp (marker_type, marker_types[COM], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: COM"), marker_names[COM]);

            if (!jpeg_com_marker (file, marker_counts))
                break;
        }
        else if (!memcmp (marker_type, marker_types[EOI], 2))
        {
            format_utils_add_field (file, MARKER_TYPE_COLOR, TRUE, 2,
                                  _("Marker type: EOI"), marker_names[EOI]);
            marker_counts[EOI]++;
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2,
                                  _("Marker type: Unknown"), "???");

            /* Data length */
            if (!format_utils_read (file, &unrecognized_data, 2))
                break;

            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 2,
                                  _("Unknown marker data length"), NULL);

            unrecognized_data = g_ntohs (unrecognized_data);
            if (unrecognized_data > 2)
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, unrecognized_data - 2,
                                      _("Unrecognized data"), NULL);
        }
    }

    /* If there is still data available after the loop, tag it as unrecognized */
    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                          _("Unrecognized data"), NULL);

    format_utils_start_section (file, _("Marker count"));

    for (gint i = SOI; i <= EOI; i++)
    {
        if (marker_counts[i])
        {
            value = g_strdup_printf ("%u", marker_counts[i]);
            format_utils_add_line (file, marker_names[i], value, NULL);
            g_free (value);
        }
    }
}

gboolean
process_jpeg_field (FormatsFile    *file,
                    DescriptionTab *tab,
                    const gchar    *field_name,
                    const gchar    *field_name_tag,
                    const gchar    *field_tooltip,
                    gint            color_index,
                    gint            field_length,
                    gint            possible_values,
                    guint8         *field_values,
                    const gchar   **value_descriptions,
                    const gchar    *value_format,
                    guint8         *read_value)
{
    const gchar *field_description = NULL;
    guint32 four_bytes = 0;

    if (!format_utils_read (file, &four_bytes, field_length))
        return FALSE;

    if (field_length == 2)
        four_bytes = g_ntohs (four_bytes);
    else if (field_length == 4)
        four_bytes = g_ntohl (four_bytes);

    if (field_name_tag)
        format_utils_add_field (file, color_index, TRUE, field_length,
                                field_name_tag, NULL);
    else
        format_utils_add_field (file, color_index, TRUE, field_length,
                                field_name, NULL);

    /* The field has a fixed number of valid values */
    if (possible_values)
    {
        for (gint i = 0; i < possible_values; i++)
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
            format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
        else
            format_utils_add_line (file, field_name, field_description, field_tooltip);
    }
    /* The field does not have fixed values */
    else if (value_format)
    {
        field_description = g_strdup_printf (value_format, four_bytes);
        if (tab)
            format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
        else
            format_utils_add_line (file, field_name, field_description, field_tooltip);
        g_free ((gpointer) field_description);
    }

    if (read_value)
        *read_value = four_bytes;

    return TRUE;
}

gboolean
jpeg_data_length (FormatsFile *file,
                  guint16     *data_length_out)
{
    if (!format_utils_read (file, data_length_out, 2))
        return FALSE;

    format_utils_add_field (file, MARKER_LENGTH_COLOR, TRUE, 2,
                          _("Data length"), NULL);

    *data_length_out = g_ntohs (*data_length_out);

    return TRUE;
}
