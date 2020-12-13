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
#include <arpa/inet.h>
#include <glib/gi18n.h>

#include "chirurgien-analyze-png.h"


GdkRGBA png_colors[] =
{
    /* #2567C1 */
    { 0.145, 0.403, 0.756, 1.0 }, // SIGNATURE_COLOR
    /* #DDE70D */
    { 0.866, 0.905, 0.050, 0.5 }, // CHUNK_LENGTH_COLOR
    /* #BB40CE */
    { 0.733, 0.250, 0.807, 0.5 }, // CHUNK_TYPE_COLOR
    /* #1CAF08 */
    { 0.109, 0.686, 0.031, 0.7 }, // CHUNK_DATA_COLOR_1
    /* #1C6E11 */
    { 0.109, 0.431, 0.066, 0.7 }, // CHUNK_DATA_COLOR_2
    /* #0DE7BA */
    { 0.050, 0.905, 0.729, 0.5 }, // CHUNK_CRC_COLOR
    /* #FF0000 */
    { 1.0, 0.0, 0.0, 1.0 },       // ERROR_COLOR_1
    /* #E84FDB */
    { 0.909, 0.309, 0.858, 1.0 }  // ERROR_COLOR_2
};

void
chirurgien_analyze_png (AnalyzerFile *file)
{
    const guchar chunk_types[CHUNK_TYPES][4] =
    {
    /* Critical chunks */
        { 0x49,0x48,0x44,0x52 }, // IHDR
        { 0x50,0x4c,0x54,0x45 }, // PLTE
        { 0x49,0x44,0x41,0x54 }, // IDAT
        { 0x49,0x45,0x4e,0x44 }, // IEND
    /* Arcillary chunks */
        { 0x74,0x52,0x4E,0x53 }, // tRNS
        { 0x63,0x48,0x52,0x4D }, // cHRM
        { 0x67,0x41,0x4D,0x41 }, // gAMA
        { 0x69,0x43,0x43,0x50 }, // iCCP
        { 0x73,0x42,0x49,0x54 }, // sBIT
        { 0x73,0x52,0x47,0x42 }, // sRGB
        { 0x74,0x45,0x58,0x74 }, // tEXt
        { 0x7A,0x54,0x58,0x74 }, // zTXt
        { 0x69,0x54,0x58,0x74 }, // iTXt
        { 0x62,0x4B,0x47,0x44 }, // bKGD
        { 0x68,0x49,0x53,0x54 }, // hIST
        { 0x70,0x48,0x59,0x73 }, // pHYs
        { 0x73,0x50,0x4C,0x54 }, // sPLT
        { 0x74,0x49,0x4D,0x45 }  // tIME
    };

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

    analyzer_utils_add_description (file,
                    "<span weight=\"bold\" size=\"larger\">Portable Network Graphics</span>",
                    NULL, NULL, 0, 20);

    analyzer_utils_create_tag (file, &png_colors[SIGNATURE_COLOR], TRUE, 8, _("PNG file signature"), NULL);
    file->file_contents_index = 8;

    /* Chunk loop */
    while (file->file_contents_index < file->file_size)
    {
        /* Loop should have ended at IEND chunk */
        if (chunk_counts[IEND])
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Unrecognized data, file ends at IEND chunk"), NULL);
            break;
        }

        /* Chunk length */
        if (!analyzer_utils_read (&four_bytes, file , 4))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Chunk length was expected"), NULL);
            break;
        }
        four_bytes = ntohl (four_bytes);
        analyzer_utils_create_tag (file, &png_colors[CHUNK_LENGTH_COLOR], TRUE, 4,
                                   _("Chunk length"), NULL);

        /* Chunk type */
        if (!analyzer_utils_read (&chunk_type, file , 4))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Chunk type was expected"), NULL);
            break;
        }

        /* Analyze chunk data */
        if (!memcmp (chunk_type, chunk_types[IDAT], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: IDAT"), "IDAT");

            if (!collect_idat_chunk (file, four_bytes, chunk_counts, &zlib_data))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IHDR], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: IHDR"), "IHDR");

            if (!analyze_ihdr_chunk (file, four_bytes, chunk_counts, &colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[IEND], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: IEND"), "IEND");
            /* IEND chunks have no data, increase counter and leave */
            chunk_counts[IEND]++;
        }
        else if (!memcmp (chunk_type, chunk_types[PLTE], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: PLTE"), "PLTE");

            if (!analyze_plte_chunk (file, four_bytes, chunk_counts, &palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tRNS], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: tRNS"), "tRNS");
            if (!analyze_trns_chunk (file, four_bytes, chunk_counts, colortype, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[cHRM], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: cHRM"), "cHRM");
            if (!analyze_chrm_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[gAMA], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: gAMA"), "gAMA");
            if (!analyze_gama_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iCCP], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: iCCP"), "iCCP");
            if (!analyze_iccp_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sBIT], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: sBIT"), "sBIT");
            if (!analyze_sbit_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sRGB], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: sRGB"), "sRGB");
            if (!analyze_srgb_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tEXt], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: tEXt"), "tEXt");
            if (!analyze_text_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[zTXt], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: zTXt"), "zTXt");
            if (!analyze_ztxt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[iTXt], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: iTXt"), "iTXt");
            if (!analyze_itxt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[bKGD], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: bKGD"), "bKGD");
            if (!analyze_bkgd_chunk (file, four_bytes, chunk_counts, colortype))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[hIST], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: hIST"), "hIST");
            if (!analyze_hist_chunk (file, four_bytes, chunk_counts, palette_entries))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[pHYs], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: pHYs"), "pHYs");
            if (!analyze_phys_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[sPLT], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: sPLT"), "sPLT");
            if (!analyze_splt_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else if (!memcmp (chunk_type, chunk_types[tIME], 4))
        {
            analyzer_utils_create_tag (file, &png_colors[CHUNK_TYPE_COLOR], TRUE, 4,
                                       _("Chunk type: tIME"), "tIME");
            if (!analyze_time_chunk (file, four_bytes, chunk_counts))
                break;
        }
        else
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_2], FALSE, 4,
                                       _("Chunk type: Unknown"), "???");
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, four_bytes,
                                       _("Unrecognized data"), NULL);
            file->file_contents_index += four_bytes;
        }

        /* Chunk CRC*/
        if (!analyzer_utils_read (&four_bytes, file , 4))
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                       _("Cyclic redundancy check was expected"), NULL);
            break;
        }
        analyzer_utils_create_tag (file, &png_colors[CHUNK_CRC_COLOR], TRUE, 4,
                                   _("Cyclic redundancy check"), NULL);
    }

    if (chunk_counts[PLTE])
    {
        description_message = g_strdup_printf ("%u", palette_entries);
        analyzer_utils_add_description (file, _("Palette entries"), description_message, 
                                        _("Number of available colors in the palette"), 10, 0);
        g_free (description_message);
    }

    analyzer_utils_add_description (file, _("<b>Chunk count</b>"), NULL, NULL, 20, 20);

    if (chunk_counts[IHDR])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[IHDR]);
        analyzer_utils_add_description (file, _("IHDR chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[PLTE])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[PLTE]);
        analyzer_utils_add_description (file, _("PLTE chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[IDAT])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[IDAT]);
        analyzer_utils_add_description (file, _("IDAT chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[IEND])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[IEND]);
        analyzer_utils_add_description (file, _("IEND chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[tRNS])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[tRNS]);
        analyzer_utils_add_description (file, _("tRNS chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[cHRM])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[cHRM]);
        analyzer_utils_add_description (file, _("cHRM chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[gAMA])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[gAMA]);
        analyzer_utils_add_description (file, _("gAMA chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[iCCP])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[iCCP]);
        analyzer_utils_add_description (file, _("iCCP chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[sBIT])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[sBIT]);
        analyzer_utils_add_description (file, _("sBIT chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[tEXt])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[tEXt]);
        analyzer_utils_add_description (file, _("tEXt chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[zTXt])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[zTXt]);
        analyzer_utils_add_description (file, _("zTXt chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[iTXt])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[iTXt]);
        analyzer_utils_add_description (file, _("iTXt chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[bKGD])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[bKGD]);
        analyzer_utils_add_description (file, _("bKGD chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[hIST])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[hIST]);
        analyzer_utils_add_description (file, _("hIST chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[pHYs])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[pHYs]);
        analyzer_utils_add_description (file, _("pHYs chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[sPLT])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[sPLT]);
        analyzer_utils_add_description (file, _("sPLT chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }
    if (chunk_counts[tIME])
    {
        description_message = g_strdup_printf ("%u", chunk_counts[tIME]);
        analyzer_utils_add_description (file, _("tIME chunks"), description_message, NULL, 0, 0);
        g_free (description_message);
    }

    analyze_idat_chunk(file, &zlib_data);
}
