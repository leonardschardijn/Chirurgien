/* gif-analyzer.h
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

#pragma once

#include <gtk/gtk.h>

#include <chirurgien-analyzer-file.h>
#include <analyzer-utils.h>

G_BEGIN_DECLS

#define HEADER_BLOCK_COLOR &colors[0]
#define BLOCK_LABEL_COLOR &colors[1]
#define BLOCK_DATA_COLOR_1 &colors[2]
#define BLOCK_DATA_COLOR_2 &colors[3]
#define DATA_SUBBLOCK_START_COLOR &colors[4]
#define ERROR_COLOR_1 &colors[6]
#define ERROR_COLOR_2 &colors[7]

enum {
    Header,
    LogicalScreenDescriptor,
    ImageDescriptor,
    GraphicalControlExtension,
    PlainTextExtension,
    ApplicationExtension,
    CommentExtension,
    Trailer,
    BLOCKS
};

gboolean    process_gif_field                        (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar *,
                                                      GdkRGBA *,
                                                      guint,
                                                      gchar *,
                                                      void *);
gboolean    process_data_subblocks                   (AnalyzerFile *,
                                                      gchar *,
                                                      GByteArray **,
                                                      gboolean);

/* gif-logical-screen-descriptor-block.c */
gboolean   analyze_logical_screen_descriptor_block   (AnalyzerFile *);

/* gif-image-descriptor-block.c */
gboolean   analyze_image_descriptor_block            (AnalyzerFile *,
                                                      AnalyzerTab *);

/* gif-graphic-control-ext-block.c */
gboolean   analyze_graphic_control_ext_block         (AnalyzerFile *,
                                                      AnalyzerTab *);

/* gif-plain-text-block.c */
gboolean   analyze_plain_text_ext_block              (AnalyzerFile *,
                                                      AnalyzerTab *);

/* gif-application-ext-block.c */
gboolean   analyze_application_ext_block             (AnalyzerFile *);

/* gif-comment-ext-block.c */
gboolean   analyze_comment_ext_block                 (AnalyzerFile *);

G_END_DECLS
