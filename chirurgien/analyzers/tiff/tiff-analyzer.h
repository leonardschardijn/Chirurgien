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

/* All recognized ASCII tags */
enum {
    /* TIFF ASCII Tags*/
    ASCII_DocumentName,
    ASCII_ImageDescription,
    ASCII_Make,
    ASCII_Model,
    ASCII_PageName,
    ASCII_Software,
    ASCII_DateTime,
    ASCII_Artist,
    ASCII_HostComputer,
    ASCII_Copyright,
    /* Exif ASCII Tags*/
    ASCII_SpectralSensitivity,
    ASCII_DateTimeOriginal,
    ASCII_DateTimeDigitized,
    ASCII_SubSecTime,
    ASCII_SubSecTimeOriginal,
    ASCII_SubSecTimeDigitized,
    ASCII_RelatedSoundFile,
    ASCII_ImageUniqueID,
    ASCII_CameraOwnerName,
    ASCII_BodySerialNumber,
    ASCII_LensMake,
    ASCII_LensModel,
    ASCII_LensSerialNumber,
    /* Exif GPSInfo ASCII Tags */
    ASCII_GPSLatitudeRef
};


/* tiff-ascii-tag.c */
void         analyze_ascii_tag                       (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      gint,
                                                      AnalyzerTab *);

void         analyze_newsubfiletype_tag              (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_subfiletype_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_imagewidth_tag                  (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_imagelength_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_bitspersample_tag               (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_compression_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_photometricinterpretation_tag   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_threshholding_tag               (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_cellwidth_tag                   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_celllength_tag                  (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_fillorder_tag                   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

guint        analyze_stripoffsets_tag                (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      guint32 **,
                                                      GSList **);

void         analyze_orientation_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_samplesperpixel_tag             (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_rowsperstrip_tag                (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

guint        analyze_stripbytecounts_tag             (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      guint32 **,
                                                      GSList **);

void         analyze_minsamplevalue_tag              (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_maxsamplevalue_tag              (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_xresolution_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_yresolution_tag                 (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_planarconfiguration_tag         (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_xposition_tag                   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_yposition_tag                   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_resolutionunit_tag              (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_pagenumber_tag                  (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_predictor_tag                   (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_whitepoint_tag                  (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_primarychromaticities_tag       (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

guint32      analyze_jpeginterchangeformat_tag       (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_jpeginterchangeformatlength_tag (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      guint32,
                                                      AnalyzerTab *);

void         analyze_ycbcrcoefficients_tag           (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_ycbcrsubsampling_tag            (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_ycbcrpositioning_tag            (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_referenceblackwhite_tag         (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_exififd_tag                     (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      AnalyzerTab *,
                                                      AnalyzerTab *);

void         analyze_gpsinfoifd_tag                  (AnalyzerFile *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **,
                                                      AnalyzerTab *,
                                                      AnalyzerTab *);

G_END_DECLS
