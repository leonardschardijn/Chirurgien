/* exif-format.h
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

#include "tiff-format.h"

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

G_END_DECLS
