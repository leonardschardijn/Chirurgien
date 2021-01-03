/* exif-analyzer.h
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

#include "tiff-analyzer.h"

G_BEGIN_DECLS

enum {
    ExposureTime,
    FNumber,
    ExposureProgram,
    SpectralSensitivity,
    PhotographicSensitivity,
    SensitivityType,
    StandardOutputSensitivity,
    RecommendedExposureIndex,
    ISOSpeed,
    ISOSpeedLatitudeyyy,
    ISOSpeedLatitudezzz,
    ExifVersion,
    DateTimeOriginal,
    DateTimeDigitized,
    ComponentsConfiguration,
    CompressedBitsPerPixel,
    ShutterSpeedValue,
    ApertureValue,
    BrightnessValue,
    ExposureBiasValue,
    MaxApertureValue,
    SubjectDistance,
    MeteringMode,
    LightSource,
    Flash,
    FocalLength,
    MakerNote,
    UserComment,
    SubSecTime,
    SubSecTimeOriginal,
    SubSecTimeDigitized,
    FlashpixVersion,
    ColorSpace,
    PixelXDimension,
    PixelYDimension,
    RelatedSoundFile,
    FocalPlaneResolutionUnit,
    SubjectLocation,
    ExposureIndex,
    SensingMethod,
    FileSource,
    SceneType,
    CFAPattern,
    CustomRendered,
    ExposureMode,
    WhiteBalance,
    DigitalZoomRatio,
    FocalLengthIn35mmFilm,
    SceneCaptureType,
    GainControl,
    Contrast,
    Saturation,
    Sharpness,
    SubjectDistanceRange,
    ImageUniqueID,
    CameraOwnerName,
    BodySerialNumber,
    LensSpecification,
    LensMake,
    LensModel,
    LensSerialNumber,
    Gamma,
    EXIF_TAGS
};


void         analyze_exposuretime_tag                (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_fnumber_tag                     (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_exposureprogram_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_photographicsensitivity_tag     (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_sensitivitytype_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_standardoutputsensitivity_tag   (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_recommendedexposureindex_tag    (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_isospeed_tag                    (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_isospeedlatitudeyyy_tag         (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_isospeedlatitudezzz_tag         (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_exifversion_tag                 (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_componentsconfiguration_tag     (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_compressedbitsperpixel_tag      (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_shutterspeedvalue_tag           (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_aperturevalue_tag               (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_brightnessvalue_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_exposurebiasvalue_tag           (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_maxaperturevalue_tag            (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_subjectdistance_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_meteringmode_tag                (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_lightsource_tag                 (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_flash_tag                       (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_focallength_tag                 (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_makernote_tag                   (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);


void         analyze_usercomment_tag                 (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_flashpixversion_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_colorspace_tag                  (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_pixelxdimension_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_pixelydimension_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_focalplaneresolutionunit_tag    (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_subjectlocation_tag             (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_exposureindex_tag               (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_sensingmethod_tag               (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_filesource_tag                  (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_scenetype_tag                   (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_cfapattern_tag                  (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_customrendered_tag              (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_exposuremode_tag                (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_whitebalance_tag                (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_digitalzoomratio_tag            (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_focallengthin35mmfilm_tag       (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_scenecapturetype_tag            (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_gaincontrol_tag                 (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_contrast_tag                    (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_saturation_tag                  (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_sharpness_tag                   (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_subjectdistancerange_tag        (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean);

void         analyze_lensspecification_tag           (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

void         analyze_gamma_tag                       (AnalyzerFile *,
                                                      AnalyzerTab *,
                                                      guint16,
                                                      guint32,
                                                      guint32,
                                                      gboolean,
                                                      GSList **);

G_END_DECLS
