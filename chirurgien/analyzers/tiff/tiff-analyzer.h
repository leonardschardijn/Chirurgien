/* tiff-analyzer.h
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

#define IFD_COLOR &colors[0]
#define TIFF_TAG_COLOR &colors[1]
#define VALUE_OFFSET_COLOR_1 &colors[2]
#define VALUE_OFFSET_COLOR_2 &colors[3]
#define FIELD_TYPE_COLOR &colors[4]
#define COUNT_COLOR &colors[5]
#define ERROR_COLOR_1 &colors[6]
#define ERROR_COLOR_2 &colors[7]
#define UNUSED_OVERLAPPING_COLOR &colors[8]

enum {
    NewSubfileType,
    SubfileType,
    ImageWidth,
    ImageLength,
    BitsPerSample,
    Compression,
    PhotometricInterpretation,
    Threshholding,
    CellWidth,
    CellLength,
    FillOrder,
    DocumentName,
    ImageDescription,
    Make,
    Model,
    StripOffsets,
    Orientation,
    SamplesPerPixel,
    RowsPerStrip,
    StripByteCounts,
    MinSampleValue,
    MaxSampleValue,
    XResolution,
    YResolution,
    PlanarConfiguration,
    PageName,
    XPosition,
    YPosition,
    ResolutionUnit,
    PageNumber,
    Software,
    DateTime,
    Artist,
    HostComputer,
    Predictor,
    WhitePoint,
    PrimaryChromaticities,
    JPEGInterchangeFormat,
    JPEGInterchangeFormatLength,
    YCbCrCoefficients,
    YCbCrSubSampling,
    YCbCrPositioning,
    ReferenceBlackWhite,
    Copyright,
    ExifIFD,
    GPSInfoIFD,
    TIFF_TAGS
};

/* Field types */
enum {
    BYTE = 1,
    ASCII,
    SHORT,
    LONG,
    RATIONAL,
    SBYTE,
    UNDEFINED,
    SSHORT,
    SLONG,
    SRATIONAL,
    FLOAT,
    DOUBLE
};


/* tiff-ascii-tag.c */
void         process_ascii_tag                       (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

/* tiff-short-tag.c */
guint32      process_short_tag                       (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar *,
                                                      guint16,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      guint,
                                                      guint16 *,
                                                      gchar **,
                                                      GSList **,
                                                      guint32 **);

/* tiff-long-tag.c */
guint32      process_long_tag                        (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar *,
                                                      guint16,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      guint32 **);
void         process_exififd_tag                     (AnalyzerFile *file,
                                                      gchar *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      AnalyzerTab *,
                                                      AnalyzerTab *,
                                                      gboolean);

/* tiff-rational-tag.c */
void         process_rational_tag                    (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar **,
                                                      guint16,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      gchar *,
                                                      GSList **);

/* tiff-byte-undefined-tag.c */
void         process_byte_undefined_tag              (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      gchar *,
                                                      gchar *,
                                                      gchar *,
                                                      guint16,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      guint,
                                                      guint8 *,
                                                      gchar **,
                                                      GSList **);

G_END_DECLS
