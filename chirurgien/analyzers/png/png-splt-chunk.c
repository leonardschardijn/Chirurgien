/* png-splt-chunk.c
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

#include "png-analyzer.h"


gboolean
analyze_splt_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    AnalyzerTab tab;

    gchar *description_message;

    g_autofree gchar *splt_chunk = NULL;

    g_autofree gchar *palette_name = NULL;

    gsize i;
    gsize palette_name_length = 0, palette_name_length_utf8,
          palette_entries;

    guint8 sample_depth = 0;

    if (!chunk_length)
        return TRUE;

    chunk_counts[sPLT]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                  _("The first chunk must be the IHDR chunk"));
        return TRUE;
    }

    analyzer_utils_init_tab (&tab);

    splt_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (splt_chunk, file, chunk_length))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1,
                                  _("Chunk length exceeds available data"));
        return FALSE;
    }

    /* The null character separes the palette name and the sample depth + palette entries */
    /* The palette name must the 1-79 bytes long */
    for (i = 0; i < chunk_length; i++)
    {
        if (splt_chunk[i] == '\0')
        {
            palette_name = splt_chunk;
            palette_name_length = i;

            if (i + 2 <= chunk_length)
                sample_depth = splt_chunk[i + 1];

            break;
        }
    }

    if (palette_name_length == 0 || palette_name_length >= 80)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                            _("Invalid palette name length"));
        palette_name = NULL;

        return TRUE;
    }

    palette_name = g_convert (palette_name, palette_name_length, "UTF-8", "ISO-8859-1", 
                         NULL, &palette_name_length_utf8, NULL);

    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, palette_name_length, _("Palette name"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1, _("Null separator"));
    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1, _("Sample depth"));

    analyzer_utils_add_text_tab (&tab, _("Palette name"), palette_name, palette_name_length);

    if (sample_depth == 8 || sample_depth == 16)
        description_message = g_strdup_printf (_("%u bits"), sample_depth);
    else
        description_message = g_strdup_printf ("%s", _("<span foreground=\"red\">INVALID</span>"));

    analyzer_utils_describe_tooltip_tab (&tab, _("Sample depth"), description_message,
                                         _("Sample depth\n"
                                         "<tt>08<sub>16</sub></tt>\t8 bits\n"
                                         "<tt>10<sub>16</sub></tt>\t16 bits"));
    g_free (description_message);

    chunk_length -= palette_name_length + 2;

    if (sample_depth == 8)
    {
        if (chunk_length % 6)
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length,
                                _("Invalid palette entry length"));
            return TRUE;
        }
        else
        {
            palette_entries = chunk_length / 6;

            for (i = 0; i < palette_entries; i++)
            {
                if (i % 2)
                {
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                        _("Palette entry blue sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                        _("Palette entry alpha sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry frequency"));
                }
                else
                {
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 1,
                                        _("Palette entry blue sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 1,
                                        _("Palette entry alpha sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry frequency"));
                }
            }
        }
    }
    else if (sample_depth == 16)
    {
        if (chunk_length % 10)
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid palette entry length"));

            return TRUE;
        }
        else
        {
            palette_entries = chunk_length / 10;

            for (i = 0; i < palette_entries; i++)
            {
                if (i % 2)
                {
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry blue sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry alpha sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry frequency"));
                }
                else
                {
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry red sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry green sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry blue sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_2, 2,
                                        _("Palette entry alpha sample"));
                    analyzer_utils_tag (file, CHUNK_DATA_COLOR_1, 2,
                                        _("Palette entry frequency"));
                }
            }
        }
    }
    else
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, chunk_length, _("Invalid sample depth"));

        return TRUE;
    }

    analyzer_utils_add_footer_tab (&tab, _("NOTE: Palette names are encoded using ISO-8859-1"));

    analyzer_utils_insert_tab (file, &tab, chunk_types[sPLT]);

    return TRUE;
}
