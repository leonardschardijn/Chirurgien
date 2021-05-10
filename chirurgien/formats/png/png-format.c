/* png-format.c
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

#include "png-format.h"

#include <glib/gi18n.h>


const gchar * const chunk_types[CHUNK_TYPES] = {
/* Critical chunks */
    "IHDR",
    "PLTE",
    "IDAT",
    "IEND",
/* Arcillary chunks */
    "tRNS",
    "cHRM",
    "gAMA",
    "iCCP",
    "sBIT",
    "sRGB",
    "tEXt",
    "zTXt",
    "iTXt",
    "bKGD",
    "hIST",
    "pHYs",
    "sPLT",
    "tIME"
};

void
chirurgien_png (FormatsFile *file)
{
    gint chunk_counts[CHUNK_TYPES];

    IdatChunks idat_data = IDAT_CHUNKS_INIT;

    gchar *value;

    guint32 chunk_length;
    guchar chunk_type[4];

    guint8 colortype = 0, compression_method = G_MAXUINT8;
    guint palette_entries = 0;

    memset (chunk_counts, 0, sizeof (gint) * CHUNK_TYPES);

    format_utils_set_title (file, "Portable Network Graphics");

    format_utils_add_field (file, SIGNATURE_COLOR, TRUE, 8,
                          _("PNG file signature"), NULL);

    format_utils_start_section (file, _("Image details"));

    /* Chunk loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at IEND chunk */
        if (chunk_counts[IEND])
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                                  _("Unrecognized data, file ends at IEND chunk"), NULL);
            break;
        }

        /* Chunk length */
        if (!format_utils_read (file, &chunk_length, 4))
            break;

        chunk_length = g_ntohl (chunk_length);
        format_utils_add_field (file, CHUNK_LENGTH_COLOR, TRUE, 4,
                              _("Chunk length"), NULL);

        /* Chunk type */
        if (!format_utils_read (file, &chunk_type, 4))
            break;

        /* Chunk data */
        if (!memcmp (chunk_type, chunk_types[IDAT], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: IDAT"), chunk_types[IDAT]);

            if (!collect_idat_chunk (file, chunk_length, chunk_counts, &idat_data))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IHDR], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: IHDR"), chunk_types[IHDR]);

            if (!png_ihdr_chunk (file, chunk_length, chunk_counts, &colortype, &compression_method))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IEND], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: IEND"), chunk_types[IEND]);

            /* IEND chunks have no data, increase counter and move on */
            chunk_counts[IEND]++;
        }
        else if (!memcmp (chunk_type, chunk_types[PLTE], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: PLTE"), chunk_types[PLTE]);

            if (!png_plte_chunk (file, chunk_length, chunk_counts, &palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tRNS], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: tRNS"), chunk_types[tRNS]);

            if (!png_trns_chunk (file, chunk_length, chunk_counts, colortype, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[cHRM], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: cHRM"), chunk_types[cHRM]);

            if (!png_chrm_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[gAMA], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: gAMA"), chunk_types[gAMA]);

            if (!png_gama_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iCCP], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: iCCP"), chunk_types[iCCP]);

            if (!png_iccp_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sBIT], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: sBIT"), chunk_types[sBIT]);

            if (!png_sbit_chunk (file, chunk_length, chunk_counts, colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sRGB], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: sRGB"), chunk_types[sRGB]);

            if (!png_srgb_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tEXt], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: tEXt"), chunk_types[tEXt]);

            if (!png_text_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[zTXt], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: zTXt"), chunk_types[zTXt]);

            if (!png_ztxt_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iTXt], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: iTXt"), chunk_types[iTXt]);

            if (!png_itxt_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[bKGD], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: bKGD"), chunk_types[bKGD]);

            if (!png_bkgd_chunk (file, chunk_length, chunk_counts, colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[hIST], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: hIST"), chunk_types[hIST]);

            if (!png_hist_chunk (file, chunk_length, chunk_counts, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[pHYs], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: pHYs"), chunk_types[pHYs]);

            if (!png_phys_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sPLT], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: sPLT"), chunk_types[sPLT]);

            if (!png_splt_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tIME], 4))
        {
            format_utils_add_field (file, CHUNK_TYPE_COLOR, TRUE, 4,
                                  _("Chunk type: tIME"), chunk_types[tIME]);

            if (!png_time_chunk (file, chunk_length, chunk_counts))
                break;
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_2, FALSE, 4,
                                  _("Chunk type: Unknown"), "???");
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, chunk_length,
                                  _("Unrecognized data"), NULL);
        }

        /* Chunk CRC */
        if (!FILE_HAS_DATA_N (file, 4))
            break;

        format_utils_add_field (file, CHUNK_CRC_COLOR, TRUE, 4,
                              _("Cyclic redundancy check"), NULL);
    }

    /* If there is still data available after the loop, tag it as unrecognized */
    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                          _("Unrecognized data"), NULL);

    if (chunk_counts[PLTE])
    {
        value = g_strdup_printf ("%u", palette_entries);
        format_utils_add_line_full (file, _("Palette entries"), value,
                                  _("Number of available colors in the palette"), 10, 0);
        g_free (value);
    }

    png_idat_chunk (file, &idat_data, compression_method);

    if (chunk_counts[IHDR])
    {
        format_utils_start_section (file, _("Chunk count"));

        for (gint i = IHDR; i <= tIME; i++)
        {
            if (chunk_counts[i])
            {
                value = g_strdup_printf ("%u", chunk_counts[i]);
                format_utils_add_line (file, chunk_types[i], value, NULL);
                g_free (value);
            }
        }
    }
}

gboolean
process_png_field (FormatsFile    *file,
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
                   gpointer        read_value)
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
