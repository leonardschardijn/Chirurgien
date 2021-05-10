/* tiff-format.h
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

#define IFD_COLOR            0
#define TIFF_TAG_COLOR       1
#define VALUE_OFFSET_COLOR_1 2
#define VALUE_OFFSET_COLOR_2 3
#define FIELD_TYPE_COLOR     4
#define COUNT_COLOR          5
#define ERROR_COLOR_1        6
#define ERROR_COLOR_2        7

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
typedef enum {
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
} TiffFieldType;

/* Exif IFDs */
typedef enum {
    EXIF_IFD,
    GPSINFO_IFD
} ExifIFDSelector;


/* tiff-ascii-tag.c */
void         process_ascii_tag             (FormatsFile *,
                                            DescriptionTab *,
                                            const gchar *,
                                            const gchar *,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            guint32,
                                            gboolean);

/* tiff-short-tag.c */
guint32      process_short_tag             (FormatsFile *,
                                            DescriptionTab *,
                                            const gchar *,
                                            const gchar *,
                                            const gchar *,
                                            TiffFieldType,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            guint32,
                                            gboolean,
                                            guint,
                                            guint16 *,
                                            const gchar **,
                                            guint32 **);

/* tiff-long-tag.c */
guint32      process_long_tag              (FormatsFile *,
                                            DescriptionTab *,
                                            const gchar *,
                                            const gchar *,
                                            const gchar *,
                                            TiffFieldType,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            guint32,
                                            gboolean,
                                            guint32 **);
gboolean     process_exififd_tag           (FormatsFile *,
                                            const gchar *,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            gboolean,
                                            DescriptionTab *,
                                            DescriptionTab *,
                                            ExifIFDSelector);

/* tiff-rational-tag.c */
void         process_rational_tag          (FormatsFile *,
                                            DescriptionTab *,
                                            const gchar *,
                                            const gchar *,
                                            const gchar *,
                                            const gchar **,
                                            TiffFieldType,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            guint32,
                                            gboolean,
                                            const gchar *);

/* tiff-byte-undefined-tag.c */
void         process_byte_undefined_tag    (FormatsFile *,
                                            DescriptionTab *,
                                            const gchar *,
                                            const gchar *,
                                            const gchar *,
                                            TiffFieldType,
                                            TiffFieldType,
                                            guint32,
                                            guint32,
                                            guint32,
                                            gboolean,
                                            guint,
                                            guint8 *,
                                            const gchar **);

G_END_DECLS
