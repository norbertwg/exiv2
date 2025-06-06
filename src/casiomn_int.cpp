// SPDX-License-Identifier: GPL-2.0-or-later
/*
  File:      Casiomn.cpp
  History:   30-Oct-13, ahu: created
  Credits:   See header file
 */
// included header files
#include "casiomn_int.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <cstring>
#include <ostream>
#include <vector>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! RecordingMode, tag 0x0001
constexpr TagDetails casioRecordingMode[] = {
    {1, N_("Single Shutter")}, {2, N_("Panorama")},  {3, N_("Night Scene")},
    {4, N_("Portrait")},       {5, N_("Landscape")}, {7, N_("Panorama")},
    {10, N_("Night Scene")},   {15, N_("Portrait")}, {16, N_("Landscape")},
};

//! Quality, tag 0x0002
constexpr TagDetails casioQuality[] = {
    {1, N_("Economy")},
    {2, N_("Normal")},
    {3, N_("Fine")},
};

//! Focus Mode, tag 0x0003
constexpr TagDetails casioFocusMode[] = {
    {2, N_("Macro")}, {3, N_("Auto")}, {4, N_("Manual")}, {5, N_("Infinity")}, {7, N_("Sport AF")},
};

//! FlashMode, tag 0x0004
constexpr TagDetails casioFlashMode[] = {
    {1, N_("Auto")}, {2, N_("On")}, {3, N_("Off")}, {4, N_("Off")}, {5, N_("Red-eye Reduction")},
};

//! Flash intensity, tag 0x0005
constexpr TagDetails casioFlashIntensity[] = {
    {11, N_("Weak")}, {12, N_("Low")}, {13, N_("Normal")}, {14, N_("High")}, {15, N_("Strong")},
};

//! white balance, tag 0x0007
constexpr TagDetails casioWhiteBalance[] = {
    {1, N_("Auto")},        {2, N_("Tungsten")}, {3, N_("Daylight")},
    {4, N_("Fluorescent")}, {5, N_("Shade")},    {129, N_("Manual")},
};

//! Flash intensity, tag 0x0005
constexpr TagDetails casioDigitalZoom[] = {
    {0x10000, N_("Off")},  {0x10001, N_("2x")}, {0x13333, N_("1.2x")}, {0x13ae1, N_("1.23x")},
    {0x19999, N_("1.6x")}, {0x20000, N_("2x")}, {0x33333, N_("3.2x")}, {0x40000, N_("4x")},
};

//! Sharpness, tag 0x000b
constexpr TagDetails casioSharpness[] = {
    {0, N_("Normal")}, {1, N_("Soft")}, {2, N_("Hard")}, {16, N_("Normal")}, {17, N_("+1")}, {18, N_("-1")},
};

//! Contrast, tag 0x000c
constexpr TagDetails casioContrast[] = {
    {0, N_("Normal")}, {1, N_("Low")}, {2, N_("High")}, {16, N_("Normal")}, {17, N_("+1")}, {18, N_("-1")},
};

//! Saturation, tag 0x000d
constexpr TagDetails casioSaturation[] = {
    {0, N_("Normal")}, {1, N_("Low")}, {2, N_("High")}, {16, N_("Normal")}, {17, N_("+1")}, {18, N_("-1")},
};

//! Enhancement, tag 0x0016
constexpr TagDetails casioEnhancement[] = {
    {1, N_("Off")}, {2, N_("Red")}, {3, N_("Green")}, {4, N_("Blue")}, {5, N_("Flesh Tones")},
};

//! Color filter, tag 0x0017
constexpr TagDetails casioColorFilter[] = {
    {1, N_("Off")},  {2, N_("Black & White")}, {3, N_("Sepia")}, {4, N_("Red")},    {5, N_("Green")},
    {6, N_("Blue")}, {7, N_("Yellow")},        {8, N_("Pink")},  {9, N_("Purple")},
};

//! flash intensity 2, tag 0x0019
constexpr TagDetails casioFlashIntensity2[] = {
    {1, N_("Normal")},
    {2, N_("Weak")},
    {3, N_("Strong")},
};

//! CCD Sensitivity intensity, tag 0x0020
constexpr TagDetails casioCCDSensitivity[] = {
    {64, N_("Normal")}, {125, N_("+1.0")}, {250, N_("+2.0")}, {244, N_("+3.0")}, {80, N_("Normal (ISO 80 equivalent)")},
    {100, N_("High")},
};

// Casio MakerNote Tag Info
constexpr TagInfo CasioMakerNote::tagInfo_[] = {
    {0x0001, "RecordingMode", N_("Recording Mode"), N_("Recording Mode"), IfdId::casioId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casioRecordingMode)},
    {0x0002, "Quality", N_("Quality"), N_("Quality"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioQuality)},
    {0x0003, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioFocusMode)},
    {0x0004, "FlashMode", N_("Flash Mode"), N_("Flash Mode"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioFlashMode)},
    {0x0005, "FlashIntensity", N_("Flash Intensity"), N_("Flash Intensity"), IfdId::casioId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casioFlashIntensity)},
    {0x0006, "ObjectDistance", N_("Object Distance"), N_("Distance to object"), IfdId::casioId, SectionId::makerTags,
     unsignedLong, -1, print0x0006},
    {0x0007, "WhiteBalance", N_("White Balance"), N_("White balance settings"), IfdId::casioId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casioWhiteBalance)},
    {0x000a, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), IfdId::casioId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(casioDigitalZoom)},
    {0x000b, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioSharpness)},
    {0x000c, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioContrast)},
    {0x000d, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casioSaturation)},
    {0x0014, "ISO", N_("ISO"), N_("ISO"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0015, "FirmwareDate", N_("Firmware date"), N_("Firmware date"), IfdId::casioId, SectionId::makerTags,
     asciiString, -1, print0x0015},
    {0x0016, "Enhancement", N_("Enhancement"), N_("Enhancement"), IfdId::casioId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(casioEnhancement)},
    {0x0017, "ColorFilter", N_("Color Filter"), N_("Color Filter"), IfdId::casioId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(casioColorFilter)},
    {0x0018, "AFPoint", N_("AF Point"), N_("AF Point"), IfdId::casioId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0019, "FlashIntensity2", N_("Flash Intensity"), N_("Flash Intensity"), IfdId::casioId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casioFlashIntensity2)},
    {0x0020, "CCDSensitivity", N_("CCDSensitivity"), N_("CCDSensitivity"), IfdId::casioId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casioCCDSensitivity)},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::casioId, SectionId::makerTags, undefined, -1,
     printValue},
    {0xffff, "(UnknownCasioMakerNoteTag)", "(UnknownCasioMakerNoteTag)", N_("Unknown CasioMakerNote tag"),
     IfdId::casioId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* CasioMakerNote::tagList() {
  return tagInfo_;
}

std::ostream& CasioMakerNote::print0x0006(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:.2f} m", value.toInt64() / 1000.0);
}

std::ostream& CasioMakerNote::print0x0015(std::ostream& os, const Value& value, const ExifData*) {
  // format is:  "YYMM#00#00DDHH#00#00MM#00#00#00#00" or  "YYMM#00#00DDHH#00#00MMSS#00#00#00"
  std::vector<char> numbers;
  for (size_t i = 0; i < value.size(); i++) {
    const auto l = value.toInt64(i);
    if (l != 0) {
      numbers.push_back(l);
    }
  }

  if (numbers.size() >= 10) {
    // year
    long l = ((numbers[0] - 48) * 10) + (numbers[1] - 48);
    if (l < 70)
      l += 2000;
    else
      l += 1900;
    os << l << ":";
    // month, day, hour, minutes
    os << numbers[2] << numbers[3] << ":" << numbers[4] << numbers[5] << " " << numbers[6] << numbers[7] << ":"
       << numbers[8] << numbers[9];
    // optional seconds
    if (numbers.size() == 12) {
      os << ":" << numbers[10] << numbers[11];
    }
  } else
    os << value;
  return os;
}

// Casio Makernotes, Type 2
//! Quality Mode, tag 0x0004
constexpr TagDetails casio2QualityMode[] = {
    {0, N_("Economy")},
    {1, N_("Normal")},
    {2, N_("Fine")},
};

//! Image Size, tag 0x0009
constexpr TagDetails casio2ImageSize[] = {
    {0, "640x480"},    {4, "1600x1200"},  {5, "2048x1536"},  {20, "2288x1712"},
    {21, "2592x1944"}, {22, "2304x1728"}, {36, "3008x2008"},
};

//! Focus Mode, tag 0x000d
constexpr TagDetails casio2FocusMode[] = {
    {0, N_("Normal")},
    {1, N_("Macro")},
};

//! ISO Speed, tag 0x0014
constexpr TagDetails casio2IsoSpeed[] = {
    {3, "50"},
    {4, "64"},
    {6, "100"},
    {9, "200"},
};

//! White Balance, tag 0x0019
constexpr TagDetails casio2WhiteBalance[] = {
    {0, N_("Auto")},     {1, N_("Daylight")},    {2, N_("Shade")},
    {3, N_("Tungsten")}, {4, N_("Fluorescent")}, {5, N_("Manual")},
};

//! Saturation, tag 0x001f
constexpr TagDetails casio2Saturation[] = {
    {0, N_("Low")},
    {1, N_("Normal")},
    {2, N_("High")},
};

//! Contrast, tag 0x0020
constexpr TagDetails casio2Contrast[] = {
    {0, N_("Low")},
    {1, N_("Normal")},
    {2, N_("High")},
};

//! Sharpness, tag 0x0021
constexpr TagDetails casio2Sharpness[] = {
    {0, N_("Soft")},
    {1, N_("Normal")},
    {2, N_("Hard")},
};

//! White Balance2, tag 0x2012
constexpr TagDetails casio2WhiteBalance2[] = {
    {0, N_("Manual")},      {1, N_("Daylight")}, {2, N_("Cloudy")},    {3, N_("Shade")},  {4, N_("Flash")},
    {6, N_("Fluorescent")}, {9, N_("Tungsten")}, {10, N_("Tungsten")}, {12, N_("Flash")},
};

//! Release Mode, tag 0x3001
constexpr TagDetails casio2ReleaseMode[] = {
    {1, N_("Normal")},
    {3, N_("AE Bracketing")},
    {11, N_("WB Bracketing")},
    {13, N_("Contrast Bracketing")},
    {19, N_("High Speed Burst")},
};

//! Quality, tag 0x3002
constexpr TagDetails casio2Quality[] = {
    {1, N_("Economy")},
    {2, N_("Normal")},
    {3, N_("Fine")},
};

//! Focus Mode 2, tag 0x3003
constexpr TagDetails casio2FocusMode2[] = {
    {0, N_("Manual")},      {1, N_("Focus Lock")},
    {2, N_("Macro")},       {3, N_("Single-Area Auto Focus")},
    {5, N_("Infinity")},    {6, N_("Multi-Area Auto Focus")},
    {8, N_("Super Macro")},
};

//! AutoISO, tag 0x3008
constexpr TagDetails casio2AutoISO[] = {
    {1, N_("On")}, {2, N_("Off")}, {7, N_("On (high sensitivity)")}, {8, N_("On (anti-shake)")}, {10, N_("High Speed")},
};

//! AFMode, tag 0x3009
constexpr TagDetails casio2AFMode[] = {
    {0, N_("Off")},      {1, N_("Spot")},        {2, N_("Multi")}, {3, N_("Face Detection")},
    {4, N_("Tracking")}, {5, N_("Intelligent")},
};

//! ColorMode, tag 0x3015
constexpr TagDetails casio2ColorMode[] = {
    {0, N_("Off")},
    {2, N_("Black & White")},
    {3, N_("Sepia")},
};

//! Enhancement, tag 0x3016
constexpr TagDetails casio2Enhancement[] = {
    {0, N_("Off")}, {1, N_("Scenery")}, {3, N_("Green")}, {5, N_("Underwater")}, {9, N_("Flesh Tones")}

};

//! Color Filter, tag 0x3017
constexpr TagDetails casio2ColorFilter[] = {
    {0, N_("Off")}, {1, N_("Blue")},   {3, N_("Green")}, {4, N_("Yellow")},
    {5, N_("Red")}, {6, N_("Purple")}, {7, N_("Pink")},
};

//! Art Mode, tag 0x301b
constexpr TagDetails casio2ArtMode[] = {
    {0, N_("Normal")},        {8, N_("Silent Movie")}, {39, N_("HDR")},
    {45, N_("Premium Auto")}, {47, N_("Painting")},    {49, N_("Crayon Drawing")},
    {51, N_("Panorama")},     {52, N_("Art HDR")},     {62, N_("High Speed Night Shot")},
    {64, N_("Monochrome")},   {67, N_("Toy Camera")},  {68, N_("Pop Art")},
    {69, N_("Light Tone")},
};

//! Lighting Mode, tag 0x302a
constexpr TagDetails casio2LightingMode[] = {
    {0, N_("Off")},
    {1, N_("High Dynamic Range")},
    {5, N_("Shadow Enhance Low")},
    {6, N_("Shadow Enhance High")},
};

//! Portrait Refiner, tag 0x302b
constexpr TagDetails casio2PortraitRefiner[] = {
    {0, N_("Off")},
    {1, N_("+1")},
    {2, N_("+2")},
};

//! Special Effect Setting, tag 0x3031
constexpr TagDetails casio2SpecialEffectSetting[] = {
    {0, N_("Off")}, {1, N_("Makeup")}, {2, N_("Mist Removal")}, {3, N_("Vivid Landscape")}, {16, N_("Art Shot")},
};

//! Drive Mode, tag 0x3103
constexpr TagDetails casio2DriveMode[] = {
    {0, N_("Single Shot")},          {1, N_("Continuous Shooting")},
    {2, N_("Continuous (2 fps)")},   {3, N_("Continuous (3 fps)")},
    {4, N_("Continuous (4 fps)")},   {5, N_("Continuous (5 fps)")},
    {6, N_("Continuous (6 fps)")},   {7, N_("Continuous (7 fps)")},
    {10, N_("Continuous (10 fps)")}, {12, N_("Continuous (12 fps)")},
    {15, N_("Continuous (15 fps)")}, {20, N_("Continuous (20 fps)")},
    {30, N_("Continuous (30 fps)")}, {40, N_("Continuous (40 fps)")},
    {60, N_("Continuous (60 fps)")}, {240, N_("Auto-N")},
};

//! Video Quality, tag 0x4003
constexpr TagDetails casio2VideoQuality[] = {
    {1, N_("Standard")},
    {3, N_("HD (720p)")},
    {4, N_("Full HD (1080p)")},
    {5, N_("Low")},
};

// Casio2 MakerNote Tag Info
constexpr TagInfo Casio2MakerNote::tagInfo_[] = {
    {0x0002, "PreviewImageSize", N_("Preview Image Size"), N_("Preview Image Size"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0003, "PreviewImageLength", N_("Preview Image Length"), N_("Preview Image Length"), IfdId::casio2Id,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0004, "PreviewImageStart", N_("Preview Image Start"), N_("Preview Image Start"), IfdId::casio2Id,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0008, "QualityMode", N_("Quality Mode"), N_("Quality Mode"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2QualityMode)},
    {0x0009, "ImageSize", N_("Image Size"), N_("Image Size"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2ImageSize)},
    {0x000d, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2FocusMode)},
    {0x0014, "ISOSpeed", N_("ISO Speed"), N_("ISO Speed"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2IsoSpeed)},
    {0x0019, "WhiteBalance", N_("White Balance"), N_("White Balance Setting"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2WhiteBalance)},
    {0x001d, "FocalLength", N_("Focal Length"), N_("Focal Length"), IfdId::casio2Id, SectionId::makerTags,
     unsignedRational, -1, printValue},
    {0x001f, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2Saturation)},
    {0x0020, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2Contrast)},
    {0x0021, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2Sharpness)},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::casio2Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x2000, "PreviewImage", N_("Preview Image"), N_("Preview Image"), IfdId::casio2Id, SectionId::makerTags, undefined,
     -1, printValue},
    {0x2001, "FirmwareDate", N_("Firmware Date"), N_("Firmware Date"), IfdId::casio2Id, SectionId::makerTags,
     asciiString, -1, print0x2001},
    {0x2011, "WhiteBalanceBias", N_("White Balance Bias"), N_("White Balance Bias"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x2012, "WhiteBalance2", N_("White Balance"), N_("White Balance Setting"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2WhiteBalance2)},
    {0x2021, "AFPointPosition", N_("AF Point Position"), N_("AF Point Position"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x2022, "ObjectDistance", N_("Object Distance"), N_("Object Distance"), IfdId::casio2Id, SectionId::makerTags,
     unsignedLong, -1, print0x2022},
    {0x2034, "FlashDistance", N_("Flash Distance"), N_("Flash Distance"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x2076, "SpecialEffectMode", N_("Special Effect Mode"), N_("Special Effect Mode"), IfdId::casio2Id,
     SectionId::makerTags, unsignedByte, -1, printValue},
    {0x2089, "FaceInfo", N_("Face Info"), N_("Face Info"), IfdId::casio2Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x211c, "FacesDetected", N_("Faces detected"), N_("Faces detected"), IfdId::casio2Id, SectionId::makerTags,
     unsignedByte, -1, printValue},
    {0x3000, "RecordMode", N_("Record Mode"), N_("Record Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x3001, "ReleaseMode", N_("Release Mode"), N_("Release Mode"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2ReleaseMode)},
    {0x3002, "Quality", N_("Quality"), N_("Quality"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2Quality)},
    {0x3003, "FocusMode2", N_("Focus Mode2"), N_("Focus Mode2"), IfdId::casio2Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(casio2FocusMode2)},
    {0x3006, "HometownCity", N_("Home town city"), N_("Home town city"), IfdId::casio2Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x3007, "BestShotMode", N_("Best Shot Mode"), N_("Best Shot Mode"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x3008, "AutoISO", N_("Auto ISO"), N_("Auto ISO"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2AutoISO)},
    {0x3009, "AFMode", N_("AF Mode"), N_("AF Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2AFMode)},
    {0x3011, "Sharpness2", N_("Sharpness"), N_("Sharpness"), IfdId::casio2Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x3012, "Contrast2", N_("Contrast"), N_("Contrast"), IfdId::casio2Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x3013, "Saturation2", N_("Saturation"), N_("Saturation"), IfdId::casio2Id, SectionId::makerTags, undefined, -1,
     printValue},
    {0x3014, "ISO", N_("ISO"), N_("ISO"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x3015, "ColorMode", N_("Color Mode"), N_("Color Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2ColorMode)},
    {0x3016, "Enhancement", N_("Enhancement"), N_("Enhancement"), IfdId::casio2Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(casio2Enhancement)},
    {0x3017, "ColorFilter", N_("Color Filter"), N_("Color Filter"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2ColorFilter)},
    {0x301b, "ArtMode", N_("Art Mode"), N_("Art Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2ArtMode)},
    {0x301c, "SequenceNumber", N_("Sequence Number"), N_("Sequence Number"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x3020, "ImageStabilization", N_("Image Stabilization"), N_("Image Stabilization"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x302a, "LightingMode", N_("Lighting Mode"), N_("Lighting Mode"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2LightingMode)},
    {0x302b, "PortraitRefiner", N_("Portrait Refiner"), N_("Portrait Refiner settings"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2PortraitRefiner)},
    {0x3030, "SpecialEffectLevel", N_("Special Effect Level"), N_("Special Effect Level"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x3031, "SpecialEffectSetting", N_("Special Effect Setting"), N_("Special Effect Setting"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(casio2SpecialEffectSetting)},
    {0x3103, "DriveMode", N_("Drive Mode"), N_("Drive Mode"), IfdId::casio2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(casio2DriveMode)},
    {0x310b, "ArtModeParameters", N_("Art Mode Parameters"), N_("Art Mode Parameters"), IfdId::casio2Id,
     SectionId::makerTags, undefined, -1, printValue},
    {0x4001, "CaptureFrameRate", N_("Capture Frame Rate"), N_("Capture Frame Rate"), IfdId::casio2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x4003, "VideoQuality", N_("Video Quality"), N_("Video Quality"), IfdId::casio2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(casio2VideoQuality)},
    {0xffff, "(UnknownCasio2MakerNoteTag)", "(UnknownCasio2MakerNoteTag)", N_("Unknown Casio2MakerNote tag"),
     IfdId::casio2Id, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* Casio2MakerNote::tagList() {
  return tagInfo_;
}

std::ostream& Casio2MakerNote::print0x2001(std::ostream& os, const Value& value, const ExifData*) {
  // format is:  "YYMM#00#00DDHH#00#00MM#00#00#00#00"
  std::vector<char> numbers;
  for (size_t i = 0; i < value.size(); i++) {
    const auto l = value.toInt64(i);
    if (l != 0) {
      numbers.push_back(l);
    }
  }

  if (numbers.size() >= 10) {
    // year
    long l = ((numbers[0] - 48) * 10) + (numbers[1] - 48);
    if (l < 70)
      l += 2000;
    else
      l += 1900;
    os << l << ":";
    // month, day, hour, minutes
    os << numbers[2] << numbers[3] << ":" << numbers[4] << numbers[5] << " " << numbers[6] << numbers[7] << ":"
       << numbers[8] << numbers[9];
  } else
    os << value;
  return os;
}

std::ostream& Casio2MakerNote::print0x2022(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toInt64() >= 0x20000000) {
    return os << N_("Inf");
  }
  return os << stringFormat("{:.2f} m", value.toInt64() / 1000.0);
}

}  // namespace Exiv2::Internal
