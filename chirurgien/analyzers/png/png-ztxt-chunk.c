/* png-ztxt-chunk.c
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

#include "puff.h"
#include "png-analyzer.h"


gboolean
analyze_ztxt_chunk (AnalyzerFile *file, gsize chunk_length, guint *chunk_counts)
{
    g_autofree guchar *ztxt_chunk = NULL;

    g_autofree gchar *keyword = NULL;
    g_autofree gchar *text = NULL;

    g_autofree guchar *inflate_text = NULL;
    guchar *compressed_text = NULL;

    gsize i;
    gsize keyword_length = 0, keyword_length_utf8, text_length_utf8;

    gsize deflate_size, inflate_size, expected_deflate_size;

    gint compression_method = -1;

    if (!chunk_length)
        return TRUE;

    chunk_counts[zTXt]++;

    if (!chunk_counts[IHDR])
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("The first chunk must be the IHDR chunk"), NULL);
        return TRUE;
    }

    ztxt_chunk = g_malloc (chunk_length);

    if (!analyzer_utils_read (ztxt_chunk, file, chunk_length))
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, -1,
                                   _("Chunk length exceeds available data"), NULL);
        return FALSE;
    }

    /* The null character separes the keyword and the text string */
    /* The keyword must the 1-79 bytes long */
    if (*ztxt_chunk == '\0')
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid keyword length"), NULL);
        return TRUE;
    }

    for (i = 0; i < chunk_length; i++)
    {
        if (ztxt_chunk[i] == '\0')
        {
            keyword = (gchar *) ztxt_chunk;
            keyword_length = i;

            /* null separator (1) + compression method (1) + ZLIB CMF (1) +
             * ZLIB FLG (1) + at least a single byte of compressed data (1) */
            if (i + 5 < chunk_length)
            {
                compression_method = ztxt_chunk[i + 1];

                compressed_text = ztxt_chunk + i + 4;
            }
            break;
        }
    }

    if (keyword == NULL)
    {
        keyword = (gchar *) ztxt_chunk;
        keyword_length = chunk_length;
    }

    if (keyword_length >= 80)
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length,
                                   _("Invalid keyword length"), NULL);
        keyword = NULL;
        text = NULL;

        return TRUE;
    }

    keyword = g_convert (keyword, keyword_length, "UTF-8", "ISO-8859-1", 
                         NULL, &keyword_length_utf8, NULL);

    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, keyword_length,
                               _("Keyword"), NULL);
    analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                               _("Null separator"), NULL);

    if (compression_method == 0) // zlib-format DEFLATE
    {
        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("Compression method"), NULL);

        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, 1,
                                   _("ZLIB compression method and flags (CMF)\n"
                                   "Lower four bits: compression method (CM)\n"
                                   "Upper four bits: compression info (CINFO)"), NULL);

        analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 1,
                                   _("ZLIB flags (FLG)"), NULL);

        /* deflate size = chunk_length - keyword - null separator (1) -
         * compression method (1) - ZLIB CMF (1) - ZLIB FLG (1) - ZLIB Adler32 chechsum (4) */
        expected_deflate_size = deflate_size = chunk_length - keyword_length - 8;

        if (!puff (NULL, NULL, NULL, NULL, &inflate_size, compressed_text, &deflate_size))
        {
            inflate_text = g_malloc (inflate_size);
            puff (NULL, NULL, NULL, inflate_text, &inflate_size, compressed_text, &deflate_size);

            analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_2], TRUE, deflate_size,
                                       _("ZLIB compressed text string"), NULL);
            analyzer_utils_create_tag (file, &png_colors[CHUNK_DATA_COLOR_1], TRUE, 4,
                                       _("ZLIB Adler32 checksum"), NULL);

            expected_deflate_size -= deflate_size;
            if (expected_deflate_size)
                analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, expected_deflate_size,
                                       _("Unrecognized data"), NULL);

            text = g_convert ((gchar *) inflate_text, inflate_size, "UTF-8", "ISO-8859-1", 
                              NULL, &text_length_utf8, NULL);
        }
        else
        {
            analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length - keyword_length - 4,
                                       _("ZLIB Compressed data (inflate failed)"), NULL);
        }
    }
    else
    {
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_2], FALSE, 1,
                                   _("Compression method"), NULL);
        analyzer_utils_create_tag (file, &png_colors[ERROR_COLOR_1], FALSE, chunk_length - keyword_length - 2,
                                   _("Unrecognized data"), NULL);
    }

    if (file->description_notebook != NULL)
    {
        GtkWidget *scrolled, *grid, *box, *textview, *frame, *label;
        GtkTextBuffer *buffer;
        GtkStyleContext *context;

        gchar *description_message;

        guint description_lines_count = 0;

        scrolled = gtk_scrolled_window_new (NULL, NULL);

        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
        gtk_widget_set_margin_start (box, 10);
        gtk_widget_set_margin_end (box, 10);
        gtk_widget_set_margin_bottom (box, 10);
        gtk_widget_set_margin_top (box, 10);

        grid = gtk_grid_new ();
        gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
        gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);

        frame = gtk_frame_new (_("Keyword"));
        gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

        textview = gtk_text_view_new ();
        gtk_widget_set_margin_start (textview, 10);
        gtk_widget_set_margin_end (textview, 10);
        gtk_widget_set_margin_bottom (textview, 10);
        gtk_widget_set_margin_top (textview, 10);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
        gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        gtk_text_buffer_set_text (buffer, keyword, keyword_length_utf8);

        gtk_container_add (GTK_CONTAINER (frame), textview);

        gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

        if (text != NULL)
        {
            frame = gtk_frame_new (_("Text string"));
            gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

            textview = gtk_text_view_new ();
            gtk_widget_set_margin_start (textview, 10);
            gtk_widget_set_margin_end (textview, 10);
            gtk_widget_set_margin_bottom (textview, 10);
            gtk_widget_set_margin_top (textview, 10);
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
            gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
            gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
            buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
            gtk_text_buffer_set_text (buffer, text, text_length_utf8);

            gtk_container_add (GTK_CONTAINER (frame), textview);

            gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);
        }

        if (compression_method == 0) // zlib-format DEFLATE
            description_message = _("zlib-format DEFLATE");
        else
            description_message = _("<span foreground=\"red\">INVALID</span>");

        analyzer_utils_add_description_here (GTK_GRID (grid), &description_lines_count,
                         _("Compression method"), description_message,
                         _("Text string compression methods\n"
                           "<tt>00<sub>16</sub></tt>\tzlib-format DEFLATE"),
                         0, 10);

        gtk_box_pack_start (GTK_BOX (box), grid, FALSE, FALSE, 0);

        label = gtk_label_new (_("NOTE: zTXt chunks are encoded using ISO-8859-1"));
        context = gtk_widget_get_style_context (label);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_DIM_LABEL);
        gtk_label_set_line_wrap (GTK_LABEL (label), GTK_WRAP_WORD);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

        gtk_container_add (GTK_CONTAINER (scrolled), box);
        gtk_widget_show_all (scrolled);

        label = gtk_label_new ("zTXt");
        gtk_notebook_insert_page (file->description_notebook, scrolled, label, -1);
    }

    return TRUE;
}
