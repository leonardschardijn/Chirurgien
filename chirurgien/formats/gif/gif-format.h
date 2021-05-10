/* gif-format.h
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

#include <format-utils.h>

G_BEGIN_DECLS

#define HEADER_BLOCK_COLOR        0
#define BLOCK_LABEL_COLOR         1
#define BLOCK_DATA_COLOR_1        2
#define BLOCK_DATA_COLOR_2        3
#define DATA_SUBBLOCK_START_COLOR 4
#define ERROR_COLOR_1             6
#define ERROR_COLOR_2             7

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

gboolean    process_gif_field                    (FormatsFile *,
                                                  DescriptionTab *,
                                                  const gchar *,
                                                  const gchar *,
                                                  const gchar *,
                                                  gint,
                                                  guint,
                                                  const gchar *,
                                                  guint8 *);
gboolean    process_data_subblocks               (FormatsFile *,
                                                  const gchar *,
                                                  GByteArray **,
                                                  gint,
                                                  gint,
                                                  gboolean);

/* gif-logical-screen-descriptor-block.c */
gboolean   gif_logical_screen_descriptor_block   (FormatsFile *);

/* gif-image-descriptor-block.c */
gboolean   gif_image_descriptor_block            (FormatsFile *,
                                                  DescriptionTab *);

/* gif-graphic-control-ext-block.c */
gboolean   gif_graphic_control_ext_block         (FormatsFile *,
                                                  DescriptionTab *);

/* gif-plain-text-block.c */
gboolean   gif_plain_text_ext_block              (FormatsFile *,
                                                  DescriptionTab *);

/* gif-application-ext-block.c */
gboolean   gif_application_ext_block             (FormatsFile *);

/* gif-comment-ext-block.c */
gboolean   gif_comment_ext_block                 (FormatsFile *);

G_END_DECLS
