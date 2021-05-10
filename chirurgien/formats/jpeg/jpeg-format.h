/* jpeg-format.h
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

#pragma once

#include <gtk/gtk.h>

#include <format-utils.h>

G_BEGIN_DECLS

#define MARKER_TYPE_COLOR         0
#define MARKER_LENGTH_COLOR       1
#define MARKER_DATA_COLOR_1       2
#define MARKER_DATA_COLOR_2       3
#define HUFFMAN_ARITH_TABLE_COLOR 4
#define COMPONENT_COLOR           5
#define ERROR_COLOR_1             6
#define ERROR_COLOR_2             7

enum {
    SOI,
    APP0,
    APP1,
    APP2,
    DQT,
    DHT,
    DAC,
    SOF0,
    SOF1,
    SOF2,
    SOF3,
    SOF5,
    SOF6,
    SOF7,
    SOF9,
    SOF10,
    SOF11,
    SOF13,
    SOF14,
    SOF15,
    DRI,
    SOS,
    DNL,
    COM,
    EOI,
    MARKER_TYPES
};

enum {
    JFIF = MARKER_TYPES,
    JFXX,
    EXIF
};

extern const gchar * const marker_names[MARKER_TYPES];

gboolean    process_jpeg_field       (FormatsFile *,
                                      DescriptionTab *,
                                      const gchar *,
                                      const gchar *,
                                      const gchar *,
                                      gint,
                                      gint,
                                      gint,
                                      guint8 *,
                                      const gchar **,
                                      const gchar *,
                                      guint8 *);
gboolean    jpeg_data_length         (FormatsFile *,
                                      guint16 *);

/* jpeg-app0-marker.c */
gboolean    jpeg_app0_marker         (FormatsFile *,
                                      gint *);

/* jpeg-app1-marker.c */
gboolean    jpeg_app1_marker         (FormatsFile *,
                                      gint *);

/* jpeg-app2-marker.c */
gboolean    jpeg_app2_marker         (FormatsFile *,
                                      gint *);

/* jpeg-dqt-marker.c */
gboolean    jpeg_dqt_marker          (FormatsFile *,
                                      gint *);

/* jpeg-dht-marker.c */
gboolean    jpeg_dht_marker          (FormatsFile *,
                                      gint *);

/* jpeg-dac-marker.c */
gboolean    jpeg_dac_marker          (FormatsFile *,
                                      gint *);

/* jpeg-sofn-marker.c */
gboolean    jpeg_sofn_marker         (FormatsFile *,
                                      gint *,
                                      gint,
                                      const gchar *);

/* jpeg-dri-dnl-marker.c */
gboolean    jpeg_dri_dnl_marker      (FormatsFile *,
                                      gint *,
                                      gint);

/* jpeg-sos-marker.c */
gboolean    jpeg_sos_marker          (FormatsFile *,
                                      gint *);

/* jpeg-com-marker.c */
gboolean    jpeg_com_marker          (FormatsFile *,
                                      gint *);

G_END_DECLS
