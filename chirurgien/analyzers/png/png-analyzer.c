/* png-analyzer.c
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

#include "png-analyzer.h"

#include <string.h>
#include <glib/gi18n.h>

#include "chirurgien-analyze-png.h"


gchar *chunk_types[CHUNK_TYPES] = {
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
chirurgien_analyze_png (AnalyzerFile *file)
{
    guint chunk_counts[CHUNK_TYPES];

    ZlibData zlib_data;

    gchar *description_message;

    guint32 four_bytes;
    guchar chunk_type[4];

    guint8 colortype = 0;
    guint palette_entries = 0;

    zlib_data.idat_chunks = NULL;
    zlib_data.compressed_image_size = 0;

    memset (chunk_counts, 0, sizeof (guint) * CHUNK_TYPES);

    analyzer_utils_set_title (file, "<span weight=\"bold\" size=\"larger\">"
                                    "Portable Network Graphics"
                                    "</span>");

    analyzer_utils_tag (file, SIGNATURE_COLOR, 8, _("PNG file signature"));
    ADVANCE_POINTER (file, 8);

    /* Chunk loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at IEND chunk */
        if (chunk_counts[IEND])
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data, file ends at IEND chunk"));
            break;
        }

        /* Chunk length */
        if (!analyzer_utils_read (&four_bytes, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            break;
        }
        four_bytes = g_ntohl (four_bytes);
        analyzer_utils_tag (file, CHUNK_LENGTH_COLOR, 4, _("Chunk length"));

        /* Chunk type */
        if (!analyzer_utils_read (&chunk_type, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            break;
        }

        /* Analyze chunk data */
        if (!memcmp (chunk_type, chunk_types[IDAT], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: IDAT"), chunk_types[IDAT]);

            if (!collect_idat_chunk (file, four_bytes, chunk_counts, &zlib_data))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IHDR], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: IHDR"), chunk_types[IHDR]);

            if (!analyze_ihdr_chunk (file, four_bytes, chunk_counts, &colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IEND], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: IEND"), chunk_types[IEND]);

            /* IEND chunks have no data, increase counter and leave */
            chunk_counts[IEND]++;
        }
        else if (!memcmp (chunk_type, chunk_types[PLTE], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: PLTE"), chunk_types[PLTE]);

            if (!analyze_plte_chunk (file, four_bytes, chunk_counts, &palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tRNS], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: tRNS"), chunk_types[tRNS]);

            if (!analyze_trns_chunk (file, four_bytes, chunk_counts, colortype, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[cHRM], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: cHRM"), chunk_types[cHRM]);

            if (!analyze_chrm_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[gAMA], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: gAMA"), chunk_types[gAMA]);

            if (!analyze_gama_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iCCP], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: iCCP"), chunk_types[iCCP]);

            if (!analyze_iccp_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sBIT], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: sBIT"), chunk_types[sBIT]);

            if (!analyze_sbit_chunk (file, four_bytes, chunk_counts, colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sRGB], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: sRGB"), chunk_types[sRGB]);

            if (!analyze_srgb_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tEXt], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: tEXt"), chunk_types[tEXt]);

            if (!analyze_text_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[zTXt], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: zTXt"), chunk_types[zTXt]);

            if (!analyze_ztxt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iTXt], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: iTXt"), chunk_types[iTXt]);

            if (!analyze_itxt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[bKGD], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: bKGD"), chunk_types[bKGD]);

            if (!analyze_bkgd_chunk (file, four_bytes, chunk_counts, colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[hIST], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: hIST"), chunk_types[hIST]);

            if (!analyze_hist_chunk (file, four_bytes, chunk_counts, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[pHYs], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: pHYs"), chunk_types[pHYs]);

            if (!analyze_phys_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sPLT], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: sPLT"), chunk_types[sPLT]);

            if (!analyze_splt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tIME], 4))
        {
            analyzer_utils_tag_navigation (file, CHUNK_TYPE_COLOR, 4, _("Chunk type: tIME"), chunk_types[tIME]);

            if (!analyze_time_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else
        {
            analyzer_utils_tag_navigation_error (file, ERROR_COLOR_2, 4, _("Chunk type: Unknown"), "???");
            analyzer_utils_tag_error (file, ERROR_COLOR_1, four_bytes, _("Unrecognized data"));

            ADVANCE_POINTER (file, four_bytes);
        }

        /* Chunk CRC*/
        if (!analyzer_utils_read (&four_bytes, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            break;
        }
        analyzer_utils_tag (file, CHUNK_CRC_COLOR, 4, _("Cyclic redundancy check"));
    }

    if (chunk_counts[PLTE])
    {
        description_message = g_strdup_printf ("%u", palette_entries);
        analyzer_utils_add_description (file, _("Palette entries"), description_message, 
                                        _("Number of available colors in the palette"), 10, 0);
        g_free (description_message);
    }

    analyze_idat_chunk(file, &zlib_data);

    analyzer_utils_set_subtitle (file, _("<b>Chunk count</b>"));

    for (gint i = IHDR; i < tIME; i++)
    {
        if (chunk_counts[i])
        {
            description_message = g_strdup_printf ("%u", chunk_counts[i]);
            analyzer_utils_describe (file, chunk_types[i], description_message);
            g_free (description_message);
        }
    }
}

gboolean
process_png_field (AnalyzerFile *file,
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
