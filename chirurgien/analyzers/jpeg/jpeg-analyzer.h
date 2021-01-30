/* jpeg-analyzer.h
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

#include <chirurgien-analyzer-file.h>
#include <analyzer-utils.h>

G_BEGIN_DECLS

#define MARKER_TYPE_COLOR &colors[0]
#define MARKER_LENGTH_COLOR &colors[1]
#define MARKER_DATA_COLOR_1 &colors[2]
#define MARKER_DATA_COLOR_2 &colors[3]
#define HUFFMAN_TABLE_COLOR &colors[4]
#define COMPONENT_COLOR &colors[5]
#define ERROR_COLOR_1 &colors[6]
#define ERROR_COLOR_2 &colors[7]

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

extern gchar *marker_names[MARKER_TYPES];

gboolean    process_jpeg_field       (AnalyzerFile *,
                                      AnalyzerTab *,
                                      gchar *,
                                      gchar *,
                                      gchar *,
                                      GdkRGBA *,
                                      guint,
                                      guint,
                                      guint8 *,
                                      gchar **,
                                      gchar *,
                                      void *);

/* jpeg-app0-marker.c */
gboolean     analyze_app0_marker      (AnalyzerFile *,
                                       guint *);

/* jpeg-app1-marker.c */
gboolean     analyze_app1_marker      (AnalyzerFile *,
                                       guint *);

/* jpeg-app2-marker.c */
gboolean     analyze_app2_marker      (AnalyzerFile *,
                                       guint *);

/* jpeg-dqt-marker.c */
gboolean     analyze_dqt_marker       (AnalyzerFile *,
                                       guint *);

/* jpeg-dht-marker.c */
gboolean     analyze_dht_marker       (AnalyzerFile *,
                                       guint *);

/* jpeg-dac-marker.c */
gboolean     analyze_dac_marker       (AnalyzerFile *,
                                       guint *);

/* jpeg-sofn-marker.c */
gboolean     analyze_sofn_marker      (AnalyzerFile *,
                                       guint *,
                                       gint);

/* jpeg-dri-dnl-marker.c */
gboolean     analyze_dri_dnl_marker   (AnalyzerFile *,
                                       guint *,
                                       gint);

/* jpeg-sos-marker.c */
gboolean     analyze_sos_marker       (AnalyzerFile *,
                                       guint *);

/* jpeg-com-marker.c */
gboolean     analyze_com_marker       (AnalyzerFile *,
                                       guint *);

G_END_DECLS
