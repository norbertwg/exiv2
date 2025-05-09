// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "minoltamn_int.hpp"
#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "makernote_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"

#include <cstdio>  // popen to call exiftool

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
// -- Standard Minolta Makernotes tags ---------------------------------------------------------------

//! Lookup table to translate Minolta Std (tag 0x0115) white balance values to readable labels
constexpr TagDetails minoltaWhiteBalanceStd0x0115[] = {
    {0x00, N_("Auto")},     {0x01, N_("Color Temperature/Color Filter")},
    {0x10, N_("Daylight")}, {0x20, N_("Cloudy")},
    {0x30, N_("Shade")},    {0x40, N_("Tungsten")},
    {0x50, N_("Flash")},    {0x60, N_("Fluorescent")},
    {0x70, N_("Custom")},
};

//! Lookup table to translate Minolta color mode values to readable labels
constexpr TagDetails minoltaColorMode[] = {
    {0, N_("Natural Color")}, {1, N_("Black & White")},   {2, N_("Vivid Color")}, {3, N_("Solarization")},
    {4, N_("AdobeRGB")},      {5, N_("Sepia")},           {9, N_("Natural")},     {12, N_("Portrait")},
    {13, N_("Natural sRGB")}, {14, N_("Natural+ sRGB")},  {15, N_("Landscape")},  {16, N_("Evening")},
    {17, N_("Night Scene")},  {18, N_("Night Portrait")},
};

//! Lookup table to translate Minolta image quality values to readable labels
constexpr TagDetails minoltaImageQuality[] = {
    {0, N_("Raw")},      {1, N_("Super Fine")}, {2, N_("Fine")},
    {3, N_("Standard")}, {4, N_("Economy")},    {5, N_("Extra Fine")},
};

//! Lookup table to translate Minolta image stabilization values
constexpr TagDetails minoltaImageStabilization[] = {
    {1, N_("Off")},
    {5, N_("On")},
};

// Minolta Tag Info
constexpr TagInfo MinoltaMakerNote::tagInfo_[] = {
    {0x0000, "Version", N_("Makernote Version"), N_("String 'MLT0' (not null terminated)"), IfdId::minoltaId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0001, "CameraSettingsStdOld", N_("Camera Settings (Std Old)"),
     N_("Standard Camera settings (Old Camera models like D5, D7, S304, and S404)"), IfdId::minoltaId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x0003, "CameraSettingsStdNew", N_("Camera Settings (Std New)"),
     N_("Standard Camera settings (New Camera Models like D7u, D7i, and D7hi)"), IfdId::minoltaId, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0004, "CameraSettings7D", N_("Camera Settings (7D)"), N_("Camera Settings (for Dynax 7D model)"),
     IfdId::minoltaId, SectionId::makerTags, undefined, -1, printValue},
    {0x0018, "ImageStabilizationData", N_("Image Stabilization Data"), N_("Image stabilization data"), IfdId::minoltaId,
     SectionId::makerTags, undefined, -1, printValue},

    // TODO: Implement WB Info A100 tags decoding.
    {0x0020, "WBInfoA100", N_("WB Info A100"), N_("White balance information for the Sony DSLR-A100"), IfdId::minoltaId,
     SectionId::makerTags, undefined, -1, printValue},

    {0x0040, "CompressedImageSize", N_("Compressed Image Size"), N_("Compressed image size"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0081, "Thumbnail", N_("Thumbnail"), N_("Jpeg thumbnail 640x480 pixels"), IfdId::minoltaId, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0088, "ThumbnailOffset", N_("Thumbnail Offset"), N_("Offset of the thumbnail"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0089, "ThumbnailLength", N_("Thumbnail Length"), N_("Size of the thumbnail"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0100, "SceneMode", N_("Scene Mode"), N_("Scene Mode"), IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1,
     printMinoltaSonySceneMode},

    // TODO: for A100, use Sony table from printMinoltaSonyColorMode().
    {0x0101, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1,
     EXV_PRINT_TAG(minoltaColorMode)},

    {0x0102, "Quality", N_("Image Quality"), N_("Image quality"), IfdId::minoltaId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(minoltaImageQuality)},

    // TODO: Tag 0x0103 : quality or image size (see ExifTool doc).
    {0x0103, "0x0103", N_("0x0103"), N_("Unknown"), IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1,
     printValue},

    {0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"), N_("Flash exposure compensation in EV"),
     IfdId::minoltaId, SectionId::makerTags, signedRational, -1, print0x9204},
    {0x0105, "Teleconverter", N_("Teleconverter Model"), N_("Teleconverter Model"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel},
    {0x0107, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(minoltaImageStabilization)},
    {0x0109, "RawAndJpgRecording", N_("RAW+JPG Recording"), N_("RAW and JPG files recording"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printMinoltaSonyBoolValue},
    {0x010a, "ZoneMatching", N_("Zone Matching"), N_("Zone matching"), IfdId::minoltaId, SectionId::makerTags,
     unsignedLong, -1, printMinoltaSonyZoneMatching},
    {0x010b, "ColorTemperature", N_("Color Temperature"), N_("Color temperature"), IfdId::minoltaId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x010c, "LensID", N_("Lens ID"), N_("Lens identifier"), IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1,
     printMinoltaSonyLensID},
    {0x0111, "ColorCompensationFilter", N_("Color Compensation Filter"),
     N_("Color Compensation Filter: negative is green, positive is magenta"), IfdId::minoltaId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"), N_("White Balance Fine Tune Value"),
     IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0113, "ImageStabilizationA100", N_("Image Stabilization A100"), N_("Image Stabilization for the Sony DSLR-A100"),
     IfdId::minoltaId, SectionId::makerTags, unsignedLong, -1, printMinoltaSonyBoolValue},

    // TODO: implement CameraSettingsA100 tags decoding.
    {0x0114, "CameraSettings5D", N_("Camera Settings (5D)"), N_("Camera Settings (for Dynax 5D model)"),
     IfdId::minoltaId, SectionId::makerTags, undefined, -1, printValue},

    {0x0115, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::minoltaId, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(minoltaWhiteBalanceStd0x0115)},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::minoltaId, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0f00, "CameraSettingsZ1", N_("Camera Settings (Z1)"), N_("Camera Settings (for Z1, DImage X, and F100 models)"),
     IfdId::minoltaId, SectionId::makerTags, undefined, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownMinoltaMakerNoteTag)", "(UnknownMinoltaMakerNoteTag)", N_("Unknown Minolta MakerNote tag"),
     IfdId::minoltaId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* MinoltaMakerNote::tagList() {
  return tagInfo_;
}

// -- Standard Minolta camera settings ---------------------------------------------------------------

//! Lookup table to translate Minolta Std camera settings exposure mode values to readable labels
constexpr TagDetails minoltaExposureModeStd[] = {
    {0, N_("Program")},
    {1, N_("Aperture priority")},
    {2, N_("Shutter priority")},
    {3, N_("Manual")},
};

//! Lookup table to translate Minolta Std camera settings flash mode values to readable labels
constexpr TagDetails minoltaFlashModeStd[] = {
    {0, N_("Fill flash")}, {1, N_("Red-eye reduction")}, {2, N_("Rear flash sync")}, {3, N_("Wireless")},
    {4, N_("Off")},
};

//! Lookup table to translate Minolta Std camera settings white balance values to readable labels
constexpr TagDetails minoltaWhiteBalanceStd[] = {
    {0, N_("Auto")},        {1, N_("Daylight")},      {2, N_("Cloudy")},    {3, N_("Tungsten")},  {5, N_("Custom")},
    {7, N_("Fluorescent")}, {8, N_("Fluorescent 2")}, {11, N_("Custom 2")}, {12, N_("Custom 3")},
};

//! Lookup table to translate Minolta Std camera settings image size values to readable labels
constexpr TagDetails minoltaImageSizeStd[] = {
    {0, N_("Full size")}, {1, "1600x1200"}, {2, "1280x960"},  {3, "640x480"},
    {6, "2080x1560"},     {7, "2560x1920"}, {8, "3264x2176"},
};

//! Lookup table to translate Minolta Std camera settings image quality values to readable labels
constexpr TagDetails minoltaImageQualityStd[] = {
    {0, N_("Raw")},      {1, N_("Super fine")}, {2, N_("Fine")},
    {3, N_("Standard")}, {4, N_("Economy")},    {5, N_("Extra fine")},
};

//! Lookup table to translate Minolta Std camera settings drive mode values to readable labels
constexpr TagDetails minoltaDriveModeStd[] = {
    {0, N_("Single Frame")}, {1, N_("Continuous")},     {2, N_("Self-timer")},    {4, N_("Bracketing")},
    {5, N_("Interval")},     {6, N_("UHS continuous")}, {7, N_("HS continuous")},
};

//! Lookup table to translate Minolta Std camera settings metering mode values to readable labels
constexpr TagDetails minoltaMeteringModeStd[] = {
    {0, N_("Multi-segment")},
    {1, N_("Center weighted average")},
    {2, N_("Spot")},
};

//! Lookup table to translate Minolta Std camera settings digital zoom values to readable labels
constexpr TagDetails minoltaDigitalZoomStd[] = {
    {0, N_("Off")},
    {1, N_("Electronic magnification")},
    {2, "2x"},
};

//! Lookup table to translate Minolta Std camera bracket step mode values to readable labels
constexpr TagDetails minoltaBracketStepStd[] = {
    {0, "1/3 EV"},
    {1, "2/3 EV"},
    {2, "1 EV"},
};

//! Lookup table to translate Minolta Std camera settings AF points values to readable labels
[[maybe_unused]] constexpr TagDetails minoltaAFPointsStd[] = {
    {0, N_("Center")}, {1, N_("Top")},         {2, N_("Top-right")}, {3, N_("Right")},    {4, N_("Bottom-right")},
    {5, N_("Bottom")}, {6, N_("Bottom-left")}, {7, N_("Left")},      {8, N_("Top-left")},
};

//! Lookup table to translate Minolta Std camera settings flash fired values to readable labels
constexpr TagDetails minoltaFlashFired[] = {
    {0, N_("Did not fire")},
    {1, N_("Fired")},
};

//! Lookup table to translate Minolta Std camera settings sharpness values to readable labels
constexpr TagDetails minoltaSharpnessStd[] = {
    {0, N_("Hard")},
    {1, N_("Normal")},
    {2, N_("Soft")},
};

//! Lookup table to translate Minolta Std camera settings subject program values to readable labels
constexpr TagDetails minoltaSubjectProgramStd[] = {
    {0, N_("None")},           {1, N_("Portrait")}, {2, N_("Text")},
    {3, N_("Night portrait")}, {4, N_("Sunset")},   {5, N_("Sports action")},
};

//! Lookup table to translate Minolta Std camera settings ISO settings values to readable labels
constexpr TagDetails minoltaISOSettingStd[] = {
    {0, "100"}, {1, "200"}, {2, "400"}, {3, "800"}, {4, N_("Auto")}, {5, "64"},
};

//! Lookup table to translate Minolta Std camera settings model values to readable labels
constexpr TagDetails minoltaModelStd[] = {
    {0, "DiMAGE 7 | X1 | X21 | X31"},
    {1, "DiMAGE 5"},
    {2, "DiMAGE S304"},
    {3, "DiMAGE S404"},
    {4, "DiMAGE 7i"},
    {5, "DiMAGE 7Hi"},
    {6, "DiMAGE A1"},
    {7, "DiMAGE A2 | S414"},
};

//! Lookup table to translate Minolta Std camera settings interval mode values to readable labels
constexpr TagDetails minoltaIntervalModeStd[] = {
    {0, N_("Still image")},
    {1, N_("Time-lapse movie")},
};

//! Lookup table to translate Minolta Std camera settings folder name values to readable labels
constexpr TagDetails minoltaFolderNameStd[] = {
    {0, N_("Standard form")},
    {1, N_("Data form")},
};

//! Lookup table to translate Minolta Std camera settings color mode values to readable labels
constexpr TagDetails minoltaColorModeStd[] = {
    {0, N_("Natural color")}, {1, N_("Black and white")}, {2, N_("Vivid color")},
    {3, N_("Solarization")},  {4, N_("Adobe RGB")},
};

//! Lookup table to translate Minolta Std camera settings wide focus zone values to readable labels
constexpr TagDetails minoltaWideFocusZoneStd[] = {
    {0, N_("No zone")},
    {1, N_("Center zone (horizontal orientation)")},
    {1, N_("Center zone (vertical orientation)")},
    {1, N_("Left zone")},
    {4, N_("Right zone")},
};

//! Lookup table to translate Minolta Std camera settings focus mode values to readable labels
constexpr TagDetails minoltaFocusModeStd[] = {
    {0, N_("Auto focus")},
    {1, N_("Manual focus")},
};

//! Lookup table to translate Minolta Std camera settings focus area values to readable labels
constexpr TagDetails minoltaFocusAreaStd[] = {
    {0, N_("Wide focus (normal)")},
    {1, N_("Spot focus")},
};

//! Lookup table to translate Minolta Std camera settings DEC switch position values to readable labels
constexpr TagDetails minoltaDECPositionStd[] = {
    {0, N_("Exposure")},
    {1, N_("Contrast")},
    {2, N_("Saturation")},
    {3, N_("Filter")},
};

//! Lookup table to translate Minolta Std camera settings color profile values to readable labels
constexpr TagDetails minoltaColorProfileStd[] = {
    {0, N_("Not embedded")},
    {1, N_("Embedded")},
};

//! Lookup table to translate Minolta Std camera settings data Imprint values to readable labels
constexpr TagDetails minoltaDataImprintStd[] = {
    {0, N_("None")}, {1, "YYYY/MM/DD"}, {2, "MM/DD/HH:MM"}, {3, N_("Text")}, {4, N_("Text + ID#")},
};

//! Lookup table to translate Minolta Std camera settings flash metering values to readable labels
constexpr TagDetails minoltaFlashMeteringStd[] = {
    {0, N_("ADI (Advanced Distance Integration)")},
    {1, N_("Pre-flash TTl")},
    {2, N_("Manual flash control")},
};

std::ostream& MinoltaMakerNote::printMinoltaExposureSpeedStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() / 8) - 1;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaExposureTimeStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() / 8) - 6;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaFNumberStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() / 8) - 1;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaExposureCompensationStd(std::ostream& os, const Value& value,
                                                                    const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << value.toInt64() / 256;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaFocalLengthStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() / 3) - 2;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaDateStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  auto val = value.toInt64();
  os << stringFormat("{}:{:02}:{:02}", val / 65536, (val % 65536) / 256, val % 256);
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaTimeStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  auto val = value.toInt64();
  os << stringFormat("{:02}:{:02}:{:02}", val / 65536, (val % 65536) / 256, val % 256);
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaFlashExposureCompStd(std::ostream& os, const Value& value,
                                                                 const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() - 6) / 3;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaWhiteBalanceStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << value.toInt64() / 256;
  return os;
}

std::ostream& MinoltaMakerNote::printMinoltaBrightnessStd(std::ostream& os, const Value& value, const ExifData*) {
  // From the PHP JPEG Metadata Toolkit
  os << (value.toInt64() / 8) - 6;
  return os;
}

// Minolta Standard Camera Settings Tag Info (Old and New)
constexpr TagInfo MinoltaMakerNote::tagInfoCsStd_[] = {
    {0x0001, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaExposureModeStd)},
    {0x0002, "FlashMode", N_("Flash Mode"), N_("Flash mode"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaFlashModeStd)},
    {0x0003, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaWhiteBalanceStd)},
    {0x0004, "ImageSize", N_("Image Size"), N_("Image size"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaImageSizeStd)},
    {0x0005, "Quality", N_("Image Quality"), N_("Image quality"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaImageQualityStd)},
    {0x0006, "DriveMode", N_("Drive Mode"), N_("Drive mode"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaDriveModeStd)},
    {0x0007, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaMeteringModeStd)},
    {0x0008, "ISO", N_("ISO"), N_("ISO Value"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong, 1,
     printMinoltaExposureSpeedStd},
    {0x0009, "ExposureTime", N_("Exposure Time"), N_("Exposure time"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printMinoltaExposureTimeStd},
    {0x000A, "FNumber", N_("FNumber"), N_("The F-Number"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong, 1,
     printMinoltaFNumberStd},
    {0x000B, "MacroMode", N_("Macro Mode"), N_("Macro mode"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, printMinoltaSonyBoolValue},
    {0x000C, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaDigitalZoomStd)},
    {0x000D, "ExposureCompensation", N_("Exposure Compensation"), N_("Exposure compensation"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printMinoltaExposureCompensationStd},
    {0x000E, "BracketStep", N_("Bracket Step"), N_("Bracket step"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaBracketStepStd)},
    {0x0010, "IntervalLength", N_("Interval Length"), N_("Interval length"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x0011, "IntervalNumber", N_("Interval Number"), N_("Interval number"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x0012, "FocalLength", N_("Focal Length"), N_("Focal length"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printMinoltaFocalLengthStd},
    {0x0013, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {0x0014, "FlashFired", N_("Flash Fired"), N_("Flash fired"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)},
    {0x0015, "MinoltaDate", N_("Minolta Date"), N_("Minolta date"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printMinoltaDateStd},
    {0x0016, "MinoltaTime", N_("Minolta Time"), N_("Minolta time"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printMinoltaTimeStd},
    {0x0017, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {0x001A, "FileNumberMemory", N_("File Number Memory"), N_("File number memory"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printMinoltaSonyBoolValue},
    {0x001B, "LastFileNumber", N_("Last Image Number"), N_("Last image number"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x001C, "ColorBalanceRed", N_("Color Balance Red"), N_("Color balance red"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd},
    {0x001D, "ColorBalanceGreen", N_("Color Balance Green"), N_("Color balance green"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd},
    {0x001E, "ColorBalanceBlue", N_("Color Balance Blue"), N_("Color balance blue"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printMinoltaWhiteBalanceStd},
    {0x001F, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {0x0020, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong, 1,
     printValue},
    {0x0021, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaSharpnessStd)},
    {0x0022, "SubjectProgram", N_("Subject Program"), N_("Subject program"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaSubjectProgramStd)},
    {0x0023, "FlashExposureComp", N_("Flash Exposure Compensation"), N_("Flash exposure compensation in EV"),
     IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong, 1, printMinoltaFlashExposureCompStd},
    {0x0024, "ISOSetting", N_("ISO Settings"), N_("ISO setting"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaISOSettingStd)},
    {0x0025, "MinoltaModel", N_("Minolta Model"), N_("Minolta model"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaModelStd)},
    {0x0026, "IntervalMode", N_("Interval Mode"), N_("Interval mode"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaIntervalModeStd)},
    {0x0027, "FolderName", N_("Folder Name"), N_("Folder name"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaFolderNameStd)},
    {0x0028, "ColorMode", N_("ColorMode"), N_("ColorMode"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaColorModeStd)},
    {0x0029, "ColorFilter", N_("Color Filter"), N_("Color filter"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printValue},
    {0x002A, "BWFilter", N_("Black and White Filter"), N_("Black and white filter"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x002B, "Internal Flash", N_("Internal Flash"), N_("Internal Flash"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)},
    {0x002C, "Brightness", N_("Brightness"), N_("Brightness"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, printMinoltaBrightnessStd},
    {0x002D, "SpotFocusPointX", N_("Spot Focus Point X"), N_("Spot focus point X"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x002E, "SpotFocusPointY", N_("Spot Focus Point Y"), N_("Spot focus point Y"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x002F, "WideFocusZone", N_("Wide Focus Zone"), N_("Wide focus zone"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaWideFocusZoneStd)},
    {0x0030, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaFocusModeStd)},
    {0x0031, "FocusArea", N_("Focus area"), N_("Focus area"), IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong,
     1, EXV_PRINT_TAG(minoltaFocusAreaStd)},
    {0x0032, "DECPosition", N_("DEC Switch Position"), N_("DEC switch position"), IfdId::minoltaCsNewId,
     SectionId::makerTags, unsignedLong, 1, EXV_PRINT_TAG(minoltaDECPositionStd)},
    {0x0033, "ColorProfile", N_("Color Profile"), N_("Color profile"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaColorProfileStd)},
    {0x0034, "DataImprint", N_("Data Imprint"), N_("Data Imprint"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaDataImprintStd)},
    {0x003F, "FlashMetering", N_("Flash Metering"), N_("Flash metering"), IfdId::minoltaCsNewId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashMeteringStd)},
    // End of list marker
    {0xffff, "(UnknownMinoltaCsStdTag)", "(UnknownMinoltaCsStdTag)", N_("Unknown Minolta Camera Settings tag"),
     IfdId::minoltaCsNewId, SectionId::makerTags, unsignedLong, 1, printValue},
};

const TagInfo* MinoltaMakerNote::tagListCsStd() {
  return tagInfoCsStd_;
}

// -- Minolta Dynax 7D camera settings ---------------------------------------------------------------

//! Lookup table to translate Minolta Dynax 7D camera settings exposure mode values to readable labels
constexpr TagDetails minoltaExposureMode7D[] = {
    {0, N_("Program")}, {1, N_("Aperture priority")}, {2, N_("Shutter priority")}, {3, N_("Manual")},
    {4, N_("Auto")},    {5, N_("Program-shift A")},   {6, N_("Program-shift S")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings image size values to readable labels
constexpr TagDetails minoltaImageSize7D[] = {
    {0, N_("Large")},
    {1, N_("Medium")},
    {2, N_("Small")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings image quality values to readable labels
constexpr TagDetails minoltaImageQuality7D[] = {
    {0, N_("Raw")}, {16, N_("Fine")}, {32, N_("Normal")}, {34, N_("Raw+Jpeg")}, {48, N_("Economy")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings white balance values to readable labels
constexpr TagDetails minoltaWhiteBalance7D[] = {
    {0, N_("Auto")},     {1, N_("Daylight")},    {2, N_("Shade")},    {3, N_("Cloudy")},
    {4, N_("Tungsten")}, {5, N_("Fluorescent")}, {256, N_("Kelvin")}, {512, N_("Manual")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings focus mode values to readable labels
constexpr TagDetails minoltaFocusMode7D[] = {
    {0, N_("Single-shot AF")},
    {1, N_("Continuous AF")},
    {3, N_("Manual")},
    {4, N_("Automatic AF")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings AF points values to readable labels
constexpr TagDetails minoltaAFPoints7D[] = {
    {1, N_("Center")},  {2, N_("Top")},          {4, N_("Top-right")}, {8, N_("Right")},      {16, N_("Bottom-right")},
    {32, N_("Bottom")}, {64, N_("Bottom-left")}, {128, N_("Left")},    {256, N_("Top-left")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings ISO settings values to readable labels
constexpr TagDetails minoltaISOSetting7D[] = {
    {0, N_("Auto")}, {1, "100"}, {3, "200"}, {4, "400"}, {5, "800"}, {6, "1600"}, {7, "3200"},
};

//! Lookup table to translate Minolta Dynax 7D camera settings color space values to readable labels
constexpr TagDetails minoltaColorSpace7D[] = {
    {0, N_("sRGB (Natural)")},
    {1, N_("sRGB (Natural+)")},
    {4, N_("Adobe RGB")},
};

//! Lookup table to translate Minolta Dynax 7D camera settings rotation values to readable labels
constexpr TagDetails minoltaRotation7D[] = {
    {72, N_("Horizontal (normal)")},
    {76, N_("Rotate 90 CW")},
    {82, N_("Rotate 270 CW")},
};

// Minolta Dynax 7D Camera Settings Tag Info
constexpr TagInfo MinoltaMakerNote::tagInfoCs7D_[] = {
    {0x0000, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaExposureMode7D)},
    {0x0002, "ImageSize", N_("Image Size"), N_("Image size"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort,
     1, EXV_PRINT_TAG(minoltaImageSize7D)},
    {0x0003, "Quality", N_("Image Quality"), N_("Image quality"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaImageQuality7D)},
    {0x0004, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaWhiteBalance7D)},
    {0x000E, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort,
     1, EXV_PRINT_TAG(minoltaFocusMode7D)},
    {0x0010, "AFPoints", N_("AF Points"), N_("AF points"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(minoltaAFPoints7D)},
    {0x0015, "FlashFired", N_("Flash Fired"), N_("Flash fired"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedLong, 1, EXV_PRINT_TAG(minoltaFlashFired)},
    {0x0016, "FlashMode", N_("Flash Mode"), N_("Flash mode"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x001C, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaISOSetting7D)},
    {0x001E, "ExposureCompensation", N_("Exposure Compensation"), N_("Exposure compensation"), IfdId::minoltaCs7DId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0025, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaColorSpace7D)},
    {0x0026, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0027, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x0028, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x002D, "FreeMemoryCardImages", N_("Free Memory Card Images"), N_("Free memory card images"), IfdId::minoltaCs7DId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x003F, "ColorTemperature", N_("Color Temperature"), N_("Color temperature"), IfdId::minoltaCs7DId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0040, "Hue", N_("Hue"), N_("Hue"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0046, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(minoltaRotation7D)},
    {0x0047, "FNumber", N_("FNumber"), N_("The F-Number"), IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x0048, "ExposureTime", N_("Exposure Time"), N_("Exposure time"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    // 0x004A is a duplicate than 0x002D.
    {0x004A, "FreeMemoryCardImages2", N_("Free Memory Card Images 2"), N_("Free memory card images 2"),
     IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x005E, "ImageNumber", N_("Image Number"), N_("Image number"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0060, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyBoolValue},
    // 0x0062 is a duplicate than 0x005E.
    {0x0062, "ImageNumber2", N_("Image Number 2"), N_("Image number 2"), IfdId::minoltaCs7DId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0071, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::minoltaCs7DId,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x0075, "ZoneMatchingOn", N_("Zone Matching On"), N_("Zone matching on"), IfdId::minoltaCs7DId,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    // End of list marker
    {0xffff, "(UnknownMinoltaCs7DTag)", "(UnknownMinoltaCs7DTag)", N_("Unknown Minolta Camera Settings 7D tag"),
     IfdId::minoltaCs7DId, SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* MinoltaMakerNote::tagListCs7D() {
  return tagInfoCs7D_;
}

// -- Minolta Dynax 5D camera settings ---------------------------------------------------------------

//! Lookup table to translate Minolta Dynax 5D camera settings exposure mode values to readable labels
constexpr TagDetails minoltaExposureMode5D[] = {
    {0, N_("Program")},
    {1, N_("Aperture priority")},
    {2, N_("Shutter priority")},
    {3, N_("Manual")},
    {4, N_("Auto")},
    {5, N_("Program Shift A")},
    {6, N_("Program Shift S")},
    {0x1013, N_("Portrait")},
    {0x1023, N_("Sports")},
    {0x1033, N_("Sunset")},
    {0x1043, N_("Night View/Portrait")},
    {0x1053, N_("Landscape")},
    {0x1083, N_("Macro")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings image size values to readable labels
constexpr TagDetails minoltaImageSize5D[] = {
    {0, N_("Large")},
    {1, N_("Medium")},
    {2, N_("Small")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings image quality values to readable labels
constexpr TagDetails minoltaImageQuality5D[] = {
    {0, N_("Raw")}, {16, N_("Fine")}, {32, N_("Normal")}, {34, N_("Raw+Jpeg")}, {48, N_("Economy")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings white balance values to readable labels
constexpr TagDetails minoltaWhiteBalance5D[] = {
    {0, N_("Auto")},        {1, N_("Daylight")}, {2, N_("Cloudy")},   {3, N_("Shade")},    {4, N_("Tungsten")},
    {5, N_("Fluorescent")}, {6, N_("Flash")},    {256, N_("Kelvin")}, {512, N_("Manual")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings metering mode values to readable labels
constexpr TagDetails minoltaMeteringMode5D[] = {
    {0, N_("Multi-segment")},
    {1, N_("Center weighted")},
    {2, N_("Spot")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings ISO settings values to readable labels
constexpr TagDetails minoltaISOSetting5D[] = {
    {0, N_("Auto")},
    {1, "100"},
    {3, "200"},
    {4, "400"},
    {5, "800"},
    {6, "1600"},
    {7, "3200"},
    {8, N_("200 (Zone Matching High)")},
    {10, N_("80 (Zone Matching Low)")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings color space values to readable labels
constexpr TagDetails minoltaColorSpace5D[] = {
    {0, N_("sRGB (Natural)")},  {1, N_("sRGB (Natural+)")}, {2, N_("Monochrome")},
    {3, N_("Adobe RGB (ICC)")}, {4, N_("Adobe RGB")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings rotation values to readable labels
constexpr TagDetails minoltaRotation5D[] = {
    {72, N_("Horizontal (normal)")},
    {76, N_("Rotate 90 CW")},
    {82, N_("Rotate 270 CW")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings focus position values to readable labels
constexpr TagDetails minoltaFocusPosition5D[] = {
    {0, N_("Wide")},       {1, N_("Central")}, {2, N_("Up")},        {3, N_("Up right")}, {4, N_("Right")},
    {5, N_("Down right")}, {6, N_("Down")},    {7, N_("Down left")}, {8, N_("Left")},     {9, N_("Up left")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings focus area values to readable labels
constexpr TagDetails minoltaFocusArea5D[] = {
    {0, N_("Wide")},
    {1, N_("Selection")},
    {2, N_("Spot")},
};

//! Lookup table to translate Minolta Dynax 5D camera settings focus mode values to readable labels
constexpr TagDetails minoltaAFMode5D[] = {
    {0, "AF-A"},
    {1, "AF-S"},
    {2, "AF-D"},
    {3, "DMF"},
};

//! Lookup table to translate Minolta Dynax 5D camera settings picture finish values to readable labels
constexpr TagDetails minoltaPictureFinish5D[] = {
    {0, N_("Natural")},       {1, N_("Natural+")},        {2, N_("Portrait")},       {3, N_("Wind Scene")},
    {4, N_("Evening Scene")}, {5, N_("Night Scene")},     {6, N_("Night Portrait")}, {7, N_("Monochrome")},
    {8, N_("Adobe RGB")},     {9, N_("Adobe RGB (ICC)")},
};

//! Method to convert Minolta Dynax 5D exposure manual bias values.
std::ostream& MinoltaMakerNote::printMinoltaExposureManualBias5D(std::ostream& os, const Value& value,
                                                                 const ExifData*) {
  // From Xavier Raynaud: the value is converted from 0:256 to -5.33:5.33
  return os << stringFormat("{:.2f}", static_cast<float>(value.toInt64() - 128) / 24);
}

//! Method to convert Minolta Dynax 5D exposure compensation values.
std::ostream& MinoltaMakerNote::printMinoltaExposureCompensation5D(std::ostream& os, const Value& value,
                                                                   const ExifData*) {
  return os << stringFormat("{:.2f}", static_cast<float>(value.toInt64() - 300) / 100);
}

// Minolta Dynax 5D Camera Settings Tag Info
constexpr TagInfo MinoltaMakerNote::tagInfoCs5D_[] = {
    {0x000A, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaExposureMode5D)},
    {0x000C, "ImageSize", N_("Image Size"), N_("Image size"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(minoltaImageSize5D)},
    {0x000D, "Quality", N_("Image Quality"), N_("Image quality"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaImageQuality5D)},
    {0x000E, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaWhiteBalance5D)},
    {0x001A, "FocusPosition", N_("Focus Position"), N_("Focus position"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaFocusPosition5D)},
    {0x001B, "FocusArea", N_("Focus Area"), N_("Focus area"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(minoltaFocusArea5D)},
    {0x001F, "FlashFired", N_("Flash Fired"), N_("Flash fired"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaFlashFired)},
    {0x0025, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaMeteringMode5D)},
    {0x0026, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaISOSetting5D)},
    {0x002F, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaColorSpace5D)},
    {0x0030, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0031, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0032, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0035, "ExposureTime", N_("Exposure Time"), N_("Exposure time"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0036, "FNumber", N_("FNumber"), N_("The F-Number"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0037, "FreeMemoryCardImages", N_("Free Memory Card Images"), N_("Free memory card images"), IfdId::minoltaCs5DId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0038, "ExposureRevision", N_("Exposure Revision"), N_("Exposure revision"), IfdId::minoltaCs5DId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0048, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(minoltaFocusModeStd)},
    {0x0049, "ColorTemperature", N_("Color Temperature"), N_("Color temperature"), IfdId::minoltaCs5DId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0050, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(minoltaRotation5D)},
    {0x0053, "ExposureCompensation", N_("Exposure Compensation"), N_("Exposure compensation"), IfdId::minoltaCs5DId,
     SectionId::makerTags, unsignedShort, -1, printMinoltaExposureCompensation5D},
    {0x0054, "FreeMemoryCardImages2", N_("Free Memory Card Images 2"), N_("Free memory card images 2"),
     IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0065, "Rotation2", N_("Rotation2"), N_("Rotation2"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort,
     -1, printMinoltaSonyRotation},
    {0x006E, "Color Temperature", N_("Color Temperature"), N_("Color Temperature"), IfdId::minoltaCs5DId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0071, "PictureFinish", N_("Picture Finish"), N_("Picture Finish"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(minoltaPictureFinish5D)},
    {0x0091, "ExposureManualBias", N_("Exposure Manual Bias"), N_("Exposure manual bias"), IfdId::minoltaCs5DId,
     SectionId::makerTags, unsignedShort, -1, printMinoltaExposureManualBias5D},
    {0x009E, "AFMode", N_("AF Mode"), N_("AF mode"), IfdId::minoltaCs5DId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(minoltaAFMode5D)},
    {0x00AE, "ImageNumber", N_("Image Number"), N_("Image number"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00B0, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::minoltaCs5DId, SectionId::makerTags,
     unsignedShort, -1, printMinoltaSonyBoolValue},
    {0x00BD, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::minoltaCs5DId,
     SectionId::makerTags, unsignedShort, -1, printMinoltaSonyBoolValue},

    // From Xavier Raynaud: some notes on missing tags.
    // 0x0051 seems to be identical to FNumber (0x0036). An approx. relation between Tag value
    // and Fstop is exp(-0.335+value*0.043)
    // 0x0052 seems to be identical to ExposureTime (0x0035). An approx. relation between Tag
    // value and Exposure time is exp(-4+value*0.085)

    // End of list marker
    {0xFFFF, "(UnknownMinoltaCs5DTag)", "(UnknownMinoltaCs5DTag)", N_("Unknown Minolta Camera Settings 5D tag"),
     IfdId::minoltaCs5DId, SectionId::makerTags, invalidTypeId, -1, printValue},
};

const TagInfo* MinoltaMakerNote::tagListCs5D() {
  return tagInfoCs5D_;
}

// -- Sony A100 camera settings ---------------------------------------------------------------

//! Lookup table to translate Sony A100 camera settings drive mode 2 values to readable labels
constexpr TagDetails sonyDriveMode2A100[] = {
    {0, N_("Self-timer 10 sec")},
    {1, N_("Continuous")},
    {4, N_("Self-timer 2 sec")},
    {5, N_("Single Frame")},
    {8, N_("White Balance Bracketing Low")},
    {9, N_("White Balance Bracketing High")},
    {770, N_("Single-frame Bracketing Low")},
    {771, N_("Continuous Bracketing Low")},
    {1794, N_("Single-frame Bracketing High")},
    {1795, N_("Continuous Bracketing High")},
};

//! Lookup table to translate Sony A100 camera settings focus mode values to readable labels
constexpr TagDetails sonyFocusModeA100[] = {
    {0, "AF-S"}, {1, "AF-C"}, {4, "AF-A"}, {5, "Manual"}, {6, "DMF"},
};

//! Lookup table to translate Sony A100 camera settings flash mode values to readable labels
constexpr TagDetails sonyFlashModeA100[] = {
    {0, N_("Auto")},
    {2, N_("Rear flash sync")},
    {3, N_("Wireless")},
    {4, N_("Fill flash")},
};

//! Lookup table to translate Sony A100 camera settings metering mode values to readable labels
constexpr TagDetails sonyMeteringModeA100[] = {
    {0, N_("Multi-segment")},
    {1, N_("Center weighted average")},
    {2, N_("Spot")},
};

//! Lookup table to translate Sony A100 camera settings zone matching mode values to readable labels
constexpr TagDetails sonyZoneMatchingModeA100[] = {
    {0, N_("Off")},
    {1, N_("Standard")},
    {2, N_("Advanced")},
};

//! Lookup table to translate Sony A100 camera settings color space values to readable labels

constexpr TagDetails sonyColorSpaceA100[] = {
    {0, N_("sRGB")},
    {5, N_("Adobe RGB")},
};

//! Lookup table to translate Sony A100 camera settings drive mode values to readable labels
constexpr TagDetails sonyDriveModeA100[] = {
    {0, N_("Single Frame")},
    {1, N_("Continuous")},
    {2, N_("Self-timer")},
    {3, N_("Continuous Bracketing")},
    {4, N_("Single-Frame Bracketing")},
    {5, N_("White Balance Bracketing")},
};

//! Lookup table to translate Sony A100 camera settings self timer time values to readable labels
constexpr TagDetails sonySelfTimerTimeA100[] = {
    {0, "10s"},
    {4, "2s"},
};

//! Lookup table to translate Sony A100 camera settings continuous bracketing values to readable labels
constexpr TagDetails sonyContinuousBracketingA100[] = {
    {0x303, N_("Low")},
    {0x703, N_("High")},
};

//! Lookup table to translate Sony A100 camera settings single frame bracketing values to readable labels
constexpr TagDetails sonySingleFrameBracketingA100[] = {
    {0x302, N_("Low")},
    {0x702, N_("High")},
};

//! Lookup table to translate Sony A100 camera settings white balance bracketing values to readable labels
constexpr TagDetails sonyWhiteBalanceBracketingA100[] = {
    {0x8, N_("Low")},
    {0x9, N_("High")},
};

//! Lookup table to translate Sony A100 camera settings white balance setting values to readable labels
constexpr TagDetails sonyWhiteBalanceSettingA100[] = {
    {0x0000, N_("Auto")},
    {0x0001, N_("Preset")},
    {0x0002, N_("Custom")},
    {0x0003, N_("Color Temperature/Color Filter")},
    {0x8001, N_("Preset")},
    {0x8002, N_("Custom")},
    {0x8003, N_("Color Temperature/Color Filter")},
};

//! Lookup table to translate Sony A100 camera settings preset white balance values to readable labels
constexpr TagDetails sonyPresetWhiteBalanceA100[] = {
    {1, N_("Daylight")}, {2, N_("Cloudy")},      {3, N_("Shade")},
    {4, N_("Tungsten")}, {5, N_("Fluorescent")}, {6, N_("Flash")},
};

//! Lookup table to translate Sony A100 camera settings color temperature setting values to readable labels
constexpr TagDetails sonyColorTemperatureSettingA100[] = {
    {0, N_("Temperature")},
    {2, N_("Color Filter")},
};

//! Lookup table to translate Sony A100 camera settings custom WB setting values to readable labels
constexpr TagDetails sonyCustomWBSettingA100[] = {
    {0, N_("Setup")},
    {2, N_("Recall")},
};

//! Lookup table to translate Sony A100 camera settings custom WB error values to readable labels
constexpr TagDetails sonyCustomWBErrorA100[] = {
    {0, N_("Ok")},
    {2, N_("Error")},
};

//! Lookup table to translate Sony A100 camera settings image size values to readable labels
constexpr TagDetails sonyImageSizeA100[] = {
    {0, N_("Standard")},
    {1, N_("Medium")},
    {2, N_("Small")},
};

//! Lookup table to translate Sony A100 camera settings instant playback setup values to readable labels
constexpr TagDetails sonyInstantPlaybackSetupA100[] = {
    {0, N_("Image and Information")},
    {1, N_("Image Only")},
    {3, N_("Image and Histogram")},
};

//! Lookup table to translate Sony A100 camera settings flash default setup values to readable labels
constexpr TagDetails sonyFlashDefaultA100[] = {
    {0, N_("Auto")},
    {1, N_("Fill Flash")},
};

//! Lookup table to translate Sony A100 camera settings auto bracket order values to readable labels
constexpr TagDetails sonyAutoBracketOrderA100[] = {
    {0, "0-+"},
    {1, "-0+"},
};

//! Lookup table to translate Sony A100 camera settings focus hold button values to readable labels
constexpr TagDetails sonyFocusHoldButtonA100[] = {
    {0, N_("Focus Hold")},
    {1, N_("DOF Preview")},
};

//! Lookup table to translate Sony A100 camera settings AEL button values to readable labels
constexpr TagDetails sonyAELButtonA100[] = {
    {0, N_("Hold")},
    {1, N_("Toggle")},
    {2, N_("Spot Hold")},
    {3, N_("Spot Toggle")},
};

//! Lookup table to translate Sony A100 camera settings control dial set values to readable labels
constexpr TagDetails sonyControlDialSetA100[] = {
    {0, N_("Shutter Speed")},
    {1, N_("Aperture")},
};

//! Lookup table to translate Sony A100 camera settings exposure compensation mode values to readable labels
constexpr TagDetails sonyExposureCompensationModeA100[] = {
    {0, N_("Ambient and Flash")},
    {1, N_("Ambient Only")},
};

//! Lookup table to translate Sony A100 camera settings sony AF area illumination values to readable labels
constexpr TagDetails sonyAFAreaIlluminationA100[] = {
    {0, N_("0.3 seconds")},
    {1, N_("0.6 seconds")},
    {2, N_("Off")},
};

//! Lookup table to translate Sony A100 camera settings monitor display off values to readable labels
constexpr TagDetails sonyMonitorDisplayOffA100[] = {
    {0, N_("Automatic")},
    {1, N_("Manual")},
};

//! Lookup table to translate Sony A100 camera settings record display values to readable labels
constexpr TagDetails sonyRecordDisplayA100[] = {
    {0, N_("Auto-rotate")},
    {1, N_("Horizontal")},
};

//! Lookup table to translate Sony A100 camera settings play display values to readable labels
constexpr TagDetails sonyPlayDisplayA100[] = {
    {0, N_("Auto-rotate")},
    {1, N_("Manual Rotate")},
};

//! Lookup table to translate Sony A100 camera settings metering off scale indicator values to readable labels
constexpr TagDetails sonyMeteringOffScaleIndicatorA100[] = {
    {0, N_("Within Range")},
    {1, N_("Under/Over Range")},
    {255, N_("Out of Range")},
};

//! Lookup table to translate Sony A100 camera settings exposure indicator values to readable labels
constexpr TagDetails sonyExposureIndicatorA100[] = {
    {0, N_("Not Indicated")},
    {1, N_("Under Scale")},
    {119, N_("Bottom of Scale")},
    {120, "-2.0"},
    {121, "-1.7"},
    {122, "-1.5"},
    {123, "-1.3"},
    {124, "-1.0"},
    {125, "-0.7"},
    {126, "-0.5"},
    {127, "-0.3"},
    {128, "-0.0"},
    {129, "+0.3"},
    {130, "+0.5"},
    {131, "+0.7"},
    {132, "+1.0"},
    {133, "+1.3"},
    {134, "+1.5"},
    {135, "+1.7"},
    {136, "+2.0"},
    {253, N_("Top of Scale")},
    {254, N_("Over Scale")},
};

//! Lookup table to translate Sony A100 camera settings focus mode switch values to readable labels
constexpr TagDetails sonyFocusModeSwitchA100[] = {
    {0, N_("AM")},
    {1, N_("MF")},
};

//! Lookup table to translate Sony A100 camera settings flash type switch values to readable labels
constexpr TagDetails sonyFlashTypeA100[] = {
    {0, N_("Off")},
    {1, N_("Built-in")},
    {2, N_("External")},
};

//! Lookup table to translate Sony A100 camera settings battery level switch values to readable labels
constexpr TagDetails sonyBatteryLevelA100[] = {
    {3, N_("Very Low")},
    {4, N_("Low")},
    {5, N_("Half Full")},
    {6, N_("Sufficient Power Remaining")},
};

// Sony A100 Camera Settings Tag Info
constexpr TagInfo MinoltaMakerNote::tagInfoCsA100_[] = {
    {0x0000, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaExposureMode5D)},
    {0x0001, "ExposureCompensationSetting", N_("Exposure Compensation Setting"), N_("Exposure compensation setting"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0005, "HighSpeedSync", N_("High Speed Sync"), N_("High speed sync"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x0006, "ManualExposureTime", N_("Manual Exposure Time"), N_("Manual exposure time"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0007, "ManualFNumber", N_("Manual FNumber"), N_("Manual FNumber"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0008, "ExposureTime", N_("Exposure Time"), N_("Exposure time"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0009, "FNumber", N_("FNumber"), N_("FNumber"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x000A, "DriveMode2", N_("Drive Mode 2"), N_("Drive mode 2"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyDriveMode2A100)},
    {0x000B, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaWhiteBalance5D)},
    {0x000C, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyFocusModeA100)},
    {0x000D, "LocalAFAreaPoint", N_("Local AF Area Point"), N_("Local AF Area Point"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint},
    {0x000E, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyAFAreaMode},
    {0x000F, "FlashMode", N_("FlashMode"), N_("FlashMode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyFlashModeA100)},
    {0x0010, "FlashExposureCompSetting", N_("Flash Exposure Comp Setting"), N_("Flash exposure compensation setting"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0012, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringModeA100)},
    {0x0013, "ISOSetting", N_("ISO Setting"), N_("ISO setting"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0014, "ZoneMatchingMode", N_("Zone Matching Mode"), N_("Zone Matching Mode"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyZoneMatchingModeA100)},
    {0x0015, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"), N_("Dynamic range optimizer mode"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode},
    {0x0016, "ColorMode", N_("Color Mode"), N_("Color mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyColorMode},
    {0x0017, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyColorSpaceA100)},
    {0x0018, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0019, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x001A, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x001C, "FlashMetering", N_("Flash Metering"), N_("Flash metering"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(minoltaFlashMeteringStd)},
    {0x001D, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"), N_("Priority Setup Shutter Release"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease},
    {0x001E, "DriveMode", N_("Drive Mode"), N_("Drive mode"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyDriveModeA100)},
    {0x001F, "SelfTimerTime", N_("Self Timer Time"), N_("Self timer time"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonySelfTimerTimeA100)},
    {0x0020, "ContinuousBracketing", N_("Continuous Bracketing"), N_("Continuous bracketing"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyContinuousBracketingA100)},
    {0x0021, "SingleFrameBracketing", N_("Single Frame Bracketing"), N_("Single frame bracketing"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonySingleFrameBracketingA100)},
    {0x0022, "WhiteBalanceBracketing", N_("White Balance Bracketing"), N_("White balance bracketing"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyWhiteBalanceBracketingA100)},
    {0x0023, "WhiteBalanceSetting", N_("White Balance Setting"), N_("White balance setting"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyWhiteBalanceSettingA100)},
    {0x0024, "PresetWhiteBalance", N_("Preset White Balance"), N_("Preset white balance"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyPresetWhiteBalanceA100)},
    {0x0025, "ColorTemperatureSetting", N_("Color Temperature Setting"), N_("Color temperature setting"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyColorTemperatureSettingA100)},
    {0x0026, "CustomWBSetting", N_("Custom WB Setting"), N_("Custom WB setting"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCustomWBSettingA100)},
    {0x0027, "DynamicRangeOptimizerSettings", N_("Dynamic Range Optimizer Settings"),
     N_("Dynamic Range Optimizer Settings"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyDynamicRangeOptimizerMode},
    {0x0032, "FreeMemoryCardImages", N_("Free Memory Card Images"), N_("Free memory card images"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0034, "CustomWBRedLevel", N_("Custom WB Red Level"), N_("Custom WB red level"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0035, "CustomWBGreenLevel", N_("Custom WB Green Level"), N_("Custom WB green level"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0036, "CustomWBBlueLevel", N_("Custom WB Blue Level"), N_("Custom WB blue level"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0037, "CustomWBError", N_("Custom WB Error"), N_("Custom WB Error"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCustomWBErrorA100)},
    {0x0038, "WhiteBalanceFineTune", N_("White Balance Fine Tune"), N_("White balance fine tune"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, signedShort, 1, printValue},
    {0x0039, "ColorTemperature", N_("Color Temperature"), N_("Color temperature"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x003A, "ColorCompensationFilter", N_("Color Compensation Filter"), N_("Color compensation filter"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, signedShort, 1, printValue},
    {0x003B, "SonyImageSize", N_("Sony Image Size"), N_("Sony Image Size"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSizeA100)},
    {0x003C, "Quality", N_("Quality"), N_("Quality"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyQualityCs},
    {0x003D, "InstantPlaybackTime", N_("Instant Playback Time"), N_("Instant playback time"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x003E, "InstantPlaybackSetup", N_("Instant Playback Setup"), N_("Instant playback setup"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyInstantPlaybackSetupA100)},
    {0x003F, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x0040, "EyeStartAF", N_("Eye Start AF"), N_("Eye start AF"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyBoolInverseValue},
    {0x0041, "RedEyeReduction", N_("Red Eye Reduction"), N_("Red eye reduction"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x0042, "FlashDefault", N_("Flash Default"), N_("Flash default"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyFlashDefaultA100)},
    {0x0043, "AutoBracketOrder", N_("Auto Bracket Order"), N_("Auto bracket order"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAutoBracketOrderA100)},
    {0x0044, "FocusHoldButton", N_("Focus Hold Button"), N_("Focus hold button"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFocusHoldButtonA100)},
    {0x0045, "AELButton", N_("AEL Button"), N_("AEL button"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyAELButtonA100)},
    {0x0046, "ControlDialSet", N_("Control Dial Set"), N_("Control dial set"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyControlDialSetA100)},
    {0x0047, "ExposureCompensationMode", N_("Exposure Compensation Mode"), N_("Exposure compensation mode"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureCompensationModeA100)},
    {0x0048, "AFAssist", N_("AF Assist"), N_("AF assist"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort,
     1, printMinoltaSonyBoolInverseValue},
    {0x0049, "CardShutterLock", N_("Card Shutter Lock"), N_("Card shutter lock"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue},
    {0x004A, "LensShutterLock", N_("Lens Shutter Lock"), N_("Lens shutter lock"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue},
    {0x004B, "AFAreaIllumination", N_("AF Area Illumination"), N_("AF area illumination"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAFAreaIlluminationA100)},
    {0x004C, "MonitorDisplayOff", N_("Monitor Display Off"), N_("Monitor display off"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMonitorDisplayOffA100)},
    {0x004D, "RecordDisplay", N_("Record Display"), N_("Record display"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyRecordDisplayA100)},
    {0x004E, "PlayDisplay", N_("Play Display"), N_("Play display"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyPlayDisplayA100)},
    {0x0050, "ExposureIndicator", N_("Exposure Indicator"), N_("Exposure indicator"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0051, "AELExposureIndicator", N_("AEL Exposure Indicator"),
     N_("AEL exposure indicator (also indicates exposure for next shot when bracketing)"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0052, "ExposureBracketingIndicatorLast", N_("Exposure Bracketing Indicator Last"),
     N_("Exposure bracketing indicator last (indicator for last shot when bracketing)"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0053, "MeteringOffScaleIndicator", N_("Metering Off Scale Indicator"),
     N_("Metering off scale indicator (two flashing triangles when under or over metering scale)"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringOffScaleIndicatorA100)},
    {0x0054, "FlashExposureIndicator", N_("Flash Exposure Indicator"), N_("Flash exposure indicator"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0055, "FlashExposureIndicatorNext", N_("Flash Exposure Indicator Next"),
     N_("Flash exposure indicator next (indicator for next shot when bracketing)"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0056, "FlashExposureIndicatorLast", N_("Flash Exposure Indicator Last"),
     N_("Flash exposure indicator last (indicator for last shot when bracketing)"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureIndicatorA100)},
    {0x0057, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x0058, "FocusModeSwitch", N_("Focus Mode Switch"), N_("Focus mode switch"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFocusModeSwitchA100)},
    {0x0059, "FlashType", N_("Flash Type"), N_("Flash type"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyFlashTypeA100)},
    {0x005A, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort,
     1, printMinoltaSonyRotation},
    {0x004B, "AELock", N_("AE Lock"), N_("AE lock"), IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyBoolValue},
    {0x005E, "ColorTemperature2", N_("Color Temperature 2"), N_("Color temperature 2"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x005F, "ColorCompensationFilter2", N_("Color Compensation Filter 2"),
     N_("Color compensation filter: negative is green, positive is magenta"), IfdId::sony1MltCsA100Id,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {0x0060, "BatteryLevel", N_("Battery Level"), N_("Battery level"), IfdId::sony1MltCsA100Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyBatteryLevelA100)},
    // End of list marker
    {0xffff, "(UnknownSonyCsA100Tag)", "(UnknownSonyCsA100Tag)", N_("Unknown Sony Camera Settings A100 tag"),
     IfdId::sony1MltCsA100Id, SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* MinoltaMakerNote::tagListCsA100() {
  return tagInfoCsA100_;
}

// TODO : Add camera settings tags info "New2"...

// -- Minolta and Sony MakerNote Common Values ---------------------------------------

//! Lookup table to translate Minolta/Sony Lens id values to readable labels
/* NOTE:
   - duplicate tags value are:
   0/25520, 4/25920, 13/25610, 19/25910, 22/26050/26070,
   25500/25501/26130, 25540/25541/25850, 25580/25581, 2564025641,
   25720/25721, 25790/25791, 25960/25961, 25980/25981, 26150/26151
   - No need to i18n these string.
*/
static constexpr TagDetails minoltaSonyLensID[] = {
    {0, "Minolta AF 28-85mm F3.5-4.5 New"},
    {1, "Minolta AF 80-200mm F2.8 HS-APO G"},
    {2, "Minolta AF 28-70mm F2.8 G"},
    {3, "Minolta AF 28-80mm F4-5.6"},
    {4, "Minolta AF 85mm F1.4G"},
    {5, "Minolta AF 35-70mm F3.5-4.5 [II]"},
    {6, "Minolta AF 24-85mm F3.5-4.5 [New]"},
    {7,
     "Minolta AF 100-300mm F4.5-5.6 (D) APO [New] | "
     "Minolta AF 100-400mm F4.5-6.7 (D) | "
     "Sigma AF 100-300mm F4 EX DG IF"},
    {8, "Minolta AF 70-210mm F4.5-5.6 [II]"},
    {9, "Minolta AF 50mm F3.5 Macro"},
    {10, "Minolta AF 28-105mm F3.5-4.5 [New]"},
    {11, "Minolta AF 300mm F4 HS-APO G"},
    {12, "Minolta AF 100mm F2.8 Soft Focus"},
    {13, "Minolta AF 75-300mm F4.5-5.6 (New or II)"},
    {14, "Minolta AF 100-400mm F4.5-6.7 APO"},
    {15, "Minolta AF 400mm F4.5 HS-APO G"},
    {16, "Minolta AF 17-35mm F3.5 G"},
    {17, "Minolta AF 20-35mm F3.5-4.5"},
    {18, "Minolta AF 28-80mm F3.5-5.6 II"},
    {19, "Minolta AF 35mm F1.4 G"},
    {20, "Minolta/Sony 135mm F2.8 [T4.5] STF"},
    {22, "Minolta AF 35-80mm F4-5.6 II"},
    {23, "Minolta AF 200mm F4 Macro APO G"},
    {24,
     "Minolta/Sony AF 24-105mm F3.5-4.5 (D) | "
     "Sigma 18-50mm F2.8 | "
     "Sigma 17-70mm F2.8-4.5 (D) | "
     "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
     "Sigma 18-200mm F3.5-6.3 DC | "
     "Sigma DC 18-125mm F4-5,6 D | "
     "Tamron SP AF 28-75mm F2.8 XR Di LD Aspherical [IF] Macro"},
    {25,
     "Minolta AF 100-300mm F4.5-5.6 APO (D) | "
     "Sigma 100-300mm F4 EX (APO (D) or D IF) | "
     "Sigma 70mm F2.8 EX DG Macro | "
     "Sigma 20mm F1.8 EX DG Aspherical RF | "
     "Sigma 30mm F1.4 DG EX | "
     "Sigma 24mm F1.8 EX DG ASP Macro"},
    {27, "Minolta AF 85mm F1.4 G (D)"},
    {0x1c,
     "Minolta/Sony AF 100mm F2.8 Macro (D) | "        // 1
     "Tamron SP AF 90mm F2.8 Di Macro | "             // 2
     "Tamron SP AF 180mm F3.5 SP Di LD [IF] Macro"},  // 3
    {29, "Minolta/Sony AF 75-300mm F4.5-5.6 (D) "},
    {30,
     "Minolta AF 28-80mm F3.5-5.6 (D) | "
     "Sigma AF 10-20mm F4-5.6 EX DC | "
     "Sigma AF 12-24mm F4.5-5.6 EX DG | "
     "Sigma 28-70mm EX DG F2.8 | "
     "Sigma 55-200mm F4-5.6 DC"},
    {31,
     "Minolta/Sony AF 50mm F2.8 Macro (D) | "
     "Minolta/Sony AF 50mm F3.5 Macro"},
    {32,
     "Minolta AF 100-400mm F4.5-6.7 (D) | "
     "Minolta/Sony AF 300mm F2.8G APO (D) SSM"},
    {33, "Minolta/Sony AF 70-200mm F2.8 APO G (D) SSM"},
    {35, "Minolta AF 85mm F1.4 G (D) Limited"},
    {36, "Minolta AF 28-100mm F3.5-5.6 (D)"},
    {38, "Minolta AF 17-35mm F2.8-4 (D)"},
    {39, "Minolta AF 28-75mm F2.8 (D)"},
    {40,
     "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | "
     "Sony AF DT 18-200mm F3.5-6.3"},
    {0x29,
     "Minolta/Sony AF DT 11-18mm F4.5-5.6 (D) | "              // 1
     "Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF"},  // 2
    {42, "Minolta/Sony AF DT 18-200mm F3.5-6.3 (D)"},
    {43, "Sony 35mm F1.4 G (SAL35F14G)"},
    {44, "Sony 50mm F1.4 (SAL50F14)"},
    {45, "Carl Zeiss Planar T* 85mm F1.4 ZA (SAL85F14Z)"},
    {46, "Carl Zeiss Vario-Sonnar T* DT 16-80mm F3.5-4.5 ZA (SAL1680Z)"},
    {47, "Carl Zeiss Sonnar T* 135mm F1.8 ZA (SAL135F18Z)"},
    {48,
     "Carl Zeiss Vario-Sonnar T* 24-70mm F2.8 ZA SSM (SAL2470Z) | "
     "Carl Zeiss Vario-Sonnar T* 24-70mm F2.8 ZA SSM II (SAL2470Z2)"},
    {49, "Sony AF DT 55-200mm F4-5.6 (SAL55200)"},
    {50, "Sony AF DT 18-250mm F3.5-6.3 (SAL18250)"},
    {51, "Sony AF DT 16-105mm F3.5-5.6 (SAL16105)"},
    //      { 51,    "Sony AF DT 55-200mm F4-5.5" }, // Anomaly? - unconfirmed.
    {0x34,
     "Sony 70-300mm F4.5-5.6 G SSM (SAL70300G) | "         // 1
     "Sony 70-300mm F4.5-5.6 G SSM II (SAL70300G2) | "     // 2
     "Tamron SP 70-300mm F4-5.6 Di USD | "                 // 3
     "Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical"},  // 4
    {53, "Sony AF 70-400mm F4.5-5.6 G SSM (SAL70400G)"},
    {54,
     "Carl Zeiss Vario-Sonnar T* 16-35mm F2.8 ZA SSM (SAL1635Z) | "
     "Carl Zeiss Vario-Sonnar T* 16-35mm F2.8 ZA SSM II (SAL1635Z2)"},
    {55,
     "Sony DT 18-55mm F3.5-5.6 SAM (SAL1855) | "
     "Sony DT 18-55mm F3.5-5.6 SAM II (SAL18552)"},
    {56, "Sony AF DT 55-200mm F4-5.6 SAM (SAL55200-2)"},
    {57,
     "Sony DT 50mm F1.8 SAM (SAL50F18) | "
     "Tamron SP AF 60mm F2 Di II LD [IF] Macro 1:1 | "
     "Tamron 18-270mm F3.5-6.3 Di II PZD"},
    {58, "Sony AF DT 30mm F2.8 SAM Macro (SAL30M28)"},
    {59, "Sony 28-75mm F2.8 SAM (SAL2875)"},
    {60, "Carl Zeiss Distagon T* 24mm F2 ZA SSM (SAL24F20Z)"},
    {61, "Sony 85mm F2.8 SAM (SAL85F28)"},
    {62, "Sony DT 35mm F1.8 SAM (SAL35F18)"},
    {63, "Sony DT 16-50mm F2.8 SSM (SAL1650)"},
    {64, "Sony 500mm F4.0 G SSM (SAL500F40G)"},
    {65, "Sony DT 18-135mm F3.5-5.6 SAM (SAL18135)"},
    {66, "Sony 300mm F2.8 G SSM II (SAL300F28G2)"},
    {67, "Sony 70-200mm F2.8 G SSM II (SAL70200G2)"},
    {68, "Sony DT 55-300mm F4.5-5.6 SAM (SAL55300)"},
    {69, "Sony 70-400mm F4-5.6 G SSM II (SAL70400G2)"},
    {70, "Sony Carl Zeiss Planar T* 50mm F1.4 ZA SSM (SALF0F14Z)"},
    {0x80,
     "Sigma 70-200mm F2.8 APO EX DG MACRO | "                            // 1
     "Tamron AF 18-200mm F3.5-6.3 XR Di II LD Aspherical [IF] Macro | "  // 2
     "Tamron AF 28-300mm F3.5-6.3 XR Di LD Aspherical [IF] Macro | "     // 3
     "Tamron 80-300mm F3.5-6.3 | "                                       // 4
     "Tamron AF 28-200mm F3.8-5.6 XR Di Aspherical [IF] MACRO | "        // 5
     "Tamron SP AF 17-35mm F2.8-4 Di LD Aspherical IF | "                // 6
     "Sigma AF 50-150mm F2.8 EX DC APO HSM II | "                        // 7
     "Sigma 10-20mm F3.5 EX DC HSM | "                                   // 8
     "Sigma 70-200mm F2.8 II EX DG APO MACRO HSM | "                     // 9
     "Sigma 10mm F2.8 EX DC HSM Fisheye | "                              // 10
     "Sigma 50mm F1.4 EX DG HSM | "                                      // 11
     "Sigma 85mm F1.4 EX DG HSM | "                                      // 12
     "Sigma 24-70mm F2.8 IF EX DG HSM | "                                // 13
     "Sigma 18-250mm F3.5-6.3 DC OS HSM | "                              // 14
     "Sigma 17-50mm F2.8 EX DC HSM | "                                   // 15
     "Sigma 17-70mm F2.8-4 DC Macro HSM | "                              // 16
     "Sigma 150mm F2.8 EX DG OS HSM APO Macro | "                        // 17
     "Sigma 150-500mm F5-6.3 APO DG OS HSM | "                           // 18
     "Tamron AF 28-105mm F4-5.6 [IF] | "                                 // 19
     "Sigma 35mm F1.4 DG HSM | "                                         // 20
     "Sigma 18-35mm F1.8 DC HSM | "                                      // 21
     "Sigma 50-500mm F4.5-6.3 APO DG OS HSM | "                          // 22
     "Sigma 24-105mm F4 DG HSM | Art 013"},                              // 23
    {129,
     "Tamron 200-400mm F5.6 LD | "
     "Tamron 70-300mm F4-5.6 LD"},
    {131, "Tamron 20-40mm F2.7-3.5 SP Aspherical IF"},
    {135, "Vivitar 28-210mm F3.5-5.6"},
    {136, "Tokina EMZ M100 AF 100mm F3.5"},
    {137, "Cosina 70-210mm F2.8-4 AF"},
    {138, "Soligor 19-35mm F3.5-4.5"},
    {139, "Tokina AF 28-300mm F4-6.3"},
    {142, "Voigtlander 70-300mm F4.5-5.6"},
    {146, "Voigtlander Macro APO-Lanthar 125mm F2.5 SL"},
    {193, "Minolta AF 1.4x APO II"},
    {194, "Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical [IF]"},
    {202, "Tamron SP AF 70-200mm F2.8 Di LD [IF] Macro"},
    {203, "Tamron SP 70-200mm F2.8 Di USD"},
    {204, "Tamron SP 24-70mm F2.8 Di USD"},
    {212, "Tamron 28-300mm F3.5-6.3 Di PZD"},
    {213, "Tamron 16-300mm F3.5-6.3 Di II PZD Macro"},
    {214, "Tamron Tamron SP 150-600mm F5-6.3 Di USD"},
    {215, "Tamron SP 15-30mm F2.8 Di USD"},
    {218, "Tamron SP 90mm F2.8 Di Macro 1:1 USD (F017)"},
    {224, "Tamron SP 90mm F2.8 Di Macro 1:1 USD (F004)"},
    {0xff,
     "Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical | "      // 1
     "Tamron AF 18-250mm F3.5-6.3 XR Di II LD | "               // 2
     "Tamron AF 55-200mm F4-5.6 Di II LD Macro | "              // 3
     "Tamron AF 70-300mm F4-5.6 Di LD Macro 1:2 | "             // 4
     "Tamron SP AF 200-500mm F5.0-6.3 Di LD IF | "              // 5
     "Tamron SP AF 10-24mm F3.5-4.5 Di II LD Aspherical IF | "  // 6
     "Tamron SP AF 70-200mm F2.8 Di LD IF Macro | "             // 7
     "Tamron SP AF 28-75mm F2.8 XR Di LD Aspherical IF | "      // 8
     "Tamron AF 90-300mm F4.5-5.6 Telemacro"},                  // 9
    {25500, "Minolta AF 50mm F1.7"},
    {25501, "Minolta AF 50mm F1.7"},
    {25510, "Minolta AF 35-70mm F4"},
    {25511,
     "Minolta AF 35-70mm F4 | "
     "Sigma UC AF 28-70mm F3.5-4.5 | "
     "Sigma AF 28-70mm F2.8 | "
     "Sigma M-AF 70-200mm F2.8 EX Aspherical | "
     "Quantaray M-AF 35-80mm F4-5.6 | "
     "Tokina 28-70mm F2.8-4.5 AF"},
    {25520, "Minolta AF 28-85mm F3.5-4.5"},
    {25521,
     "Minolta AF 28-85mm F3.5-4.5 | "
     "Tokina 19-35mm F3.5-4.5 | "
     "Tokina 28-70mm F2.8 AT-X | "
     "Tokina 80-400mm F4.5-5.6 AT-X AF II 840 | "
     "Tokina AF PRO 28-80mm F2.8 AT-X 280 | "
     "Tokina AT-X PRO [II] AF 28-70mm F2.6-2.8 270 | "
     "Tamron AF 19-35mm F3.5-4.5 | "
     "Angenieux AF 28-70mm F2.6 | "
     "Tokina AT-X 17 AF 17mm F3.5 | "
     "Tokina 20-35mm F3.5-4.5 II AF"},
    {25530, "Minolta AF 28-135mm F4-4.5"},
    {25531,
     "Minolta AF 28-135mm F4-4.5 | "
     "Sigma ZOOM-alpha 35-135mm F3.5-4.5 | "
     "Sigma 28-105mm F2.8-4 Aspherical | "
     "Sigma 28-105mm F4-5.6 UC"},
    {25540, "Minolta AF 35-105mm F3.5-4.5"},
    {25541, "Minolta AF 35-105mm F3.5-4.5"},
    {25550, "Minolta AF 70-210mm F4"},
    {25551,
     "Minolta AF 70-210mm F4 Macro | "
     "Sigma 70-210mm F4-5.6 APO | "
     "Sigma M-AF 70-200mm F2.8 EX APO | "
     "Sigma 75-200mm F2.8-3.5"},
    {25560, "Minolta AF 135mm F2.8"},
    {25561, "Minolta AF 135mm F2.8"},
    {25570, "Minolta AF 28mm F2.8"},
    {25571, "Minolta/Sony AF 28mm F2.8"},
    {25580, "Minolta AF 24-50mm F4"},
    {25581, "Minolta AF 24-50mm F4"},
    {25600, "Minolta AF 100-200mm F4.5"},
    {25601, "Minolta AF 100-200mm F4.5"},
    {25610, "Minolta AF 75-300mm F4.5-5.6"},
    {25611,
     "Minolta AF 75-300mm F4.5-5.6 | "
     "Sigma 70-300mm F4-5.6 DL Macro | "
     "Sigma 300mm F4 APO Macro | "
     "Sigma AF 500mm F4.5 APO | "
     "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
     "Tokina AT-X AF 300mm F4 | "
     "Tokina AT-X AF 400mm F5.6 SD | "
     "Tokina AF 730 II 75-300mm F4.5-5.6 | "
     "Sigma 800mm F5.6 APO | "
     "Sigma AF 400mm F5.6 APO Macro"},
    {25620, "Minolta AF 50mm F1.4"},
    {25621, "Minolta AF 50mm F1.4 [New]"},
    {25630, "Minolta AF 300mm F2.8G APO"},
    {25631,
     "Minolta AF 300mm F2.8 APO | "
     "Sigma AF 50-500mm F4-6.3 EX DG APO | "
     "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
     "Sigma AF 500mm F4.5 EX DG APO | "
     "Sigma 400mm F5.6 APO"},
    {25640, "Minolta AF 50mm F2.8 Macro"},
    {25641,
     "Minolta AF 50mm F2.8 Macro | "
     "Sigma 50mm F2.8 EX Macro"},
    {25650, "Minolta AF 600mm F4 APO"},
    {25651, "Minolta AF 600mm F4 APO"},
    {25660, "Minolta AF 24mm F2.8"},
    {25661,
     "Minolta AF 24mm F2.8 | "
     "Sigma 17-35mm F2.8-4.0 EX-D"},
    {25720, "Minolta AF 500mm F8 Reflex"},
    {25721, "Minolta/Sony AF 500mm F8 Reflex"},
    {25780, "Minolta/Sony AF 16mm F2.8 Fisheye"},
    {25781,
     "Minolta/Sony AF 16mm F2.8 Fisheye | "
     "Sigma 8mm F4 EX [DG] Fisheye | "
     "Sigma 14mm F3.5 | "
     "Sigma 15mm F2.8 Fisheye"},
    {25790, "Minolta AF 20mm F2.8"},
    {25791,
     "Minolta/Sony AF 20mm F2.8 | "
     "Tokina AT-X 116 PRO DX AF 11-16mm F2.8"},
    {25810, "Minolta AF 100mm F2.8 Macro"},
    {25811,
     "Minolta AF 100mm F2.8 Macro [New] | "
     "Sigma AF 90mm F2.8 Macro | "
     "Sigma AF 105mm F2.8 EX [DG] Macro | "
     "Sigma 180mm F5.6 Macro | "
     "Sigma 180mm F3.5 EX DG Macro | "
     "Tamron 90mm F2.8 Macro"},
    {25850, "Minolta AF 35-105mm F3.5-4.5"},
    {25851, "Beroflex 35-135mm F3.5-4.5"},
    {25858,
     "Minolta AF 35-105mm F3.5-4.5 New | "
     "Tamron 24-135mm F3.5-5.6"},
    {25880, "Minolta AF 70-210mm F3.5-4.5"},
    {25881, "Minolta AF 70-210mm F3.5-4.5"},
    {25890, "Minolta AF 80-200mm F2.8 APO"},
    {25891,
     "Minolta AF 80-200mm F2.8 APO | "
     "Tokina 80-200mm F2.8"},
    {25900, "Minolta AF 200mm F2.8 G APO + Minolta AF 1.4x APO"},
    {25901,
     "Minolta AF 200mm F2.8 G APO + Minolta AF 1.4x APO | "
     "Minolta AF 600mm F4 HS-APO G + Minolta AF 1.4x APO"},
    {25910, "Minolta AF 35mm F1.4G"},
    {25911, "Minolta AF 35mm F1.4"},
    {25920, "Minolta AF 85mm F1.4G"},
    {25921, "Minolta AF 85mm F1.4G (D)"},
    {25930, "Minolta AF 200mm F2.8 APO"},
    {25931, "Minolta AF 200mm F2.8 APO"},
    {25940, "Minolta AF 3X-1X F1.7-2.8 Macro"},
    {25941, "Minolta AF 3x-1x F1.7-2.8 Macro"},
    {25960, "Minolta AF 28mm F2"},
    {25961, "Minolta AF 28mm F2"},
    {25970, "Minolta AF 35mm F2"},
    {25971, "Minolta AF 35mm F2 [New]"},
    {25980, "Minolta AF 100mm F2"},
    {25981, "Minolta AF 100mm F2"},
    {26010, "Minolta AF 200mm F2.8 G APO + Minolta AF 2x APO"},
    {26011,
     "Minolta AF 200mm F2.8 G APO + Minolta AF 2x APO | "
     "Minolta AF 600mm F4 HS-APO G + Minolta AF 2x APO"},
    {26040, "Minolta AF 80-200mm F4.5-5.6"},
    {26041, "Minolta AF 80-200mm F4.5-5.6"},
    {26050, "Minolta AF 35-80mm F4-5.6"},
    {26051, "Minolta AF 35-80mm F4-5.6"},
    {26060, "Minolta AF 100-300mm F4.5-5.6"},
    {26061,
     "Minolta AF 100-300mm F4.5-5.6 (D) | "
     "Sigma 105mm F2.8 Macro EX DG"},
    {26070, "Minolta AF 35-80mm F4-5.6"},
    {26071, "Minolta AF 35-80mm F4-5.6"},
    {26080, "Minolta AF 300mm F2.8 G"},
    {26081, "Minolta AF 300mm F2.8 G APO High Speed"},
    {26090, "Minolta AF 600mm F4 G"},
    {26091, "Minolta AF 600mm F4 G APO High Speed"},
    {26120, "Minolta AF 200mm F2.8 G"},
    {26121, "Minolta AF 200mm F2.8 G APO High Speed"},
    {26130, "Minolta AF 50mm F1.7"},
    {26131, "Minolta AF 50mm F1.7 New"},
    {26150, "Minolta AF 28-105mm F3.5-4.5 Xi"},
    {26151, "Minolta AF 28-105mm F3.5-4.5 xi"},
    {26160, "Minolta AF 35-200mm F4.5-5.6 Xi"},
    {26161, "Minolta AF 35-200mm F4.5-5.6 Xi"},
    {26180, "Minolta AF 28-80mm F4-5.6 Xi"},
    {26181, "Minolta AF 28-80mm F4-5.6 xi"},
    {26190, "Minolta AF 80-200mm F4.5-5.6 Xi"},
    {26191, "Minolta AF 80-200mm F4.5-5.6 Xi"},
    {26201, "Minolta AF 28-70mm F2.8 G"},
    {26210, "Minolta AF 100-300mm F4.5-5.6 Xi"},
    {26211, "Minolta AF 100-300mm F4.5-5.6 xi"},
    {26240, "Minolta AF 35-80mm F4-5.6 Power Zoom"},
    {26241, "Minolta AF 35-80mm F4-5.6 Power Zoom"},
    {26281, "Minolta AF 80-200mm F2.8 HS-APO G"},
    {26291, "Minolta AF 85mm F1.4 New"},
    {26311, "Minolta/Sony AF 100-300mm F4.5-5.6 APO"},
    {26321, "Minolta AF 24-50mm F4 New"},
    {26381, "Minolta AF 50mm F2.8 Macro New"},
    {26391, "Minolta AF 100mm F2.8 Macro"},
    {26411, "Minolta/Sony AF 20mm F2.8 New"},
    {26421, "Minolta AF 24mm F2.8 New"},
    {26441, "Minolta AF 100-400mm F4.5-6.7 APO"},
    {26621, "Minolta AF 50mm F1.4 New"},
    {26671, "Minolta AF 35mm F2 New"},
    {26681, "Minolta AF 28mm F2 New"},
    {26721, "Minolta AF 24-105mm F3.5-4.5 (D)"},
    {45671, "Tokina 70-210mm F4-5.6"},
    {45711, "Vivitar 70-210mm F4.5-5.6"},
    {45741,
     "Minolta AF200mm F2.8G x2 | "
     "Tokina 300mm F2.8 x2 | "
     "Tokina RF 500mm F8.0 x2 | "
     "Tamron SP AF 90mm F2.5"},
    {45751, "1.4x Teleconverter "},
    {45851, "Tamron SP AF 300mm F2.8 LD IF"},
    {45861, "Tamron SP AF 35-105mm F2.8 LD Aspherical IF"},
    {45871, "Tamron AF 70-210mm F2.8 SP LD"},
    {65280, "Sigma 16mm F2.8 Filtermatic Fisheye"},
    {
        0xffff,
        "Manual lens | "             // 1
        "Sony E 50mm F1.8 OSS | "    // 2
        "E PZ 16-50mm F3.5-5.6 OSS"  // 3
    },
};

// ----------------------------------------------------------------------
// #1145 begin - respect lenses with shared LensID

#if 0
    // resolveLensTypeUsingExiftool has been debugged on the Mac
    // It's not in service because:
    // 1 we don't know the path to the file being processed
    // 2 can't work for a remote file as exiftool doesn't handle remote IO
    // 3 almost certainly throws an ugly ugly dos box on the screen in Windows
    // 4 I haven't asked Phil's permission to do this
    static std::ostream& resolveLensTypeUsingExiftool(std::ostream& os, const Value& value,
                                                 const ExifData* metadata)
    {
// #if ! defined(_WIN32) && ! defined(__CYGWIN__) && ! defined(__MINGW__)
#ifndef _MSC_VER
        FILE* f = ::popen("/bin/bash -c \"exiftool ~/temp/screen.jpg | grep 'Lens ID' | cut -d: -f 2 | sed -E -e 's/^ //g'\"","r");
        if ( f ) {
            char buffer[200];
            int  n=::fread(buffer,1,sizeof buffer-1,f);
            ::pclose(f);
            // just to be sure, add a null byte
            if ( 0 <= n && n < (int) sizeof(buffer) ) buffer[n] = 0 ;

            // and stop at any non-printing character such as line-feed
            for (int c = 0 ; c < 32 ; c++)
                if ( ::strchr(buffer,c) )
                    *::strchr(buffer,c)=0;
            return os << buffer;
        }
#endif
        return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
    }
#endif

static std::string getKeyString(const std::string& key, const ExifData* metadata) {
  std::string result;
  if (metadata->findKey(ExifKey(key)) != metadata->end()) {
    result = metadata->findKey(ExifKey(key))->toString();
  }
  return result;
}

static long getKeyLong(const std::string& key, const ExifData* metadata, int which = 0);
static long getKeyLong(const std::string& key, const ExifData* metadata, int which) {
  long result = -1;
  if (metadata->findKey(ExifKey(key)) != metadata->end()) {
    result = static_cast<long>(metadata->findKey(ExifKey(key))->toFloat(which));
  }
  return result;
}

/*! http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
    trim from left
*/
static std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v") {
  s.erase(0, s.find_first_not_of(t));
  return s;
}

//! trim from right
static std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v") {
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

//! trim from left & right
static std::string& trim(std::string& s, const char* t = " \t\n\r\f\v") {
  return ltrim(rtrim(s, t), t);
}

// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
static std::vector<std::string> split(const std::string& str, const std::string& delim) {
  std::vector<std::string> tokens;
  size_t prev = 0;
  size_t pos = 0;
  while (pos < str.length() && prev < str.length()) {
    pos = str.find(delim, prev);
    if (pos == std::string::npos)
      pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(std::move(token));
    prev = pos + delim.length();
  }
  return tokens;
}

static bool inRange(long value, long min, long max) {
  return min <= value && value <= max;
}

static std::ostream& resolvedLens(std::ostream& os, long lensID, long index) {
  auto td = Exiv2::find(minoltaSonyLensID, lensID);
  std::vector<std::string> tokens = split(td[0].label_, "|");
  return os << exvGettext(trim(tokens.at(index - 1)).c_str());
}

static std::ostream& resolveLens0x1c(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 0;

    std::string model = getKeyString("Exif.Image.Model", metadata);
    std::string lens = getKeyString("Exif.Photo.LensModel", metadata);

    if (model == "SLT-A77V" && lens == "100mm F2.8 Macro") {
      index = 2;
    }

    if (index > 0) {
      const long lensID = 0x1c;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

static std::ostream& resolveLens0x29(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 0;

    std::string model = getKeyString("Exif.Image.Model", metadata);
    std::string lens = getKeyString("Exif.Photo.LensModel", metadata);

    if (model == "SLT-A77V" && lens == "DT 11-18mm F4.5-5.6") {
      index = 2;
    }

    if (index > 0) {
      const long lensID = 0x29;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

static std::ostream& resolveLens0x34(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 0;

    std::string model = getKeyString("Exif.Image.Model", metadata);
    std::string maxAperture = getKeyString("Exif.Photo.MaxApertureValue", metadata);
    long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);

    // F2_8
    if (model == "SLT-A77V" && maxAperture == "760/256") {
      index = 4;
    }
    if (model == "SLT-A77V" && inRange(focalLength, 70, 300)) {
      index = 3;
    }

    if (index > 0) {
      const long lensID = 0x34;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

static std::ostream& resolveLens0x80(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 0;

    std::string model = getKeyString("Exif.Image.Model", metadata);
    std::string maxAperture = getKeyString("Exif.Photo.MaxApertureValue", metadata);
    long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);

    // F4
    if (model == "SLT-A77V" && maxAperture == "1024/256" && inRange(focalLength, 18, 200)) {
      index = 2;
    }

    if (index > 0) {
      const long lensID = 0x80;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

static std::ostream& resolveLens0xff(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 0;

    std::string model = getKeyString("Exif.Image.Model", metadata);
    long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);
    std::string maxAperture = getKeyString("Exif.Photo.MaxApertureValue", metadata);

    // F2_8
    if (model == "SLT-A77V" && maxAperture == "760/256" && inRange(focalLength, 17, 50)) {
      index = 1;
    }

    if (index > 0) {
      const long lensID = 0xff;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

static std::ostream& resolveLens0xffff(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    long index = 1;

    // #1153
    std::string model = getKeyString("Exif.Image.Model", metadata);
    std::string maxAperture = getKeyString("Exif.Photo.MaxApertureValue", metadata);

    std::string F1_8 = "434/256";
    static constexpr const char* maxApertures[] = {
        "926/256",   // F3.5
        "1024/256",  // F4
        "1110/256",  // F4.5
        "1188/256",  // F5
        "1272/256",  // F5.6
    };

    if (model == "ILCE-6000" && maxAperture == F1_8)
      try {
        long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);
        if (focalLength > 0) {
          long focalL35mm = getKeyLong("Exif.Photo.FocalLengthIn35mmFilm", metadata);
          long focalRatio = (focalL35mm * 100) / focalLength;
          if (inRange(focalRatio, 145, 155))
            index = 2;
        }
      } catch (...) {
      }

    if (model == "ILCE-6000" && Exiv2::find(maxApertures, maxAperture))
      try {
        long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);
        if (focalLength > 0) {
          long focalL35mm = getKeyLong("Exif.Photo.FocalLengthIn35mmFilm", metadata);
          long focalRatio = (focalL35mm * 100) / focalLength;
          if (inRange(focalRatio, 145, 155))
            index = 3;
        }
      } catch (...) {
      }

    if (index > 0) {
      const long lensID = 0xffff;
      return resolvedLens(os, lensID, index);
    }
  } catch (...) {
  }

  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

std::ostream& printMinoltaSonyLensID(std::ostream& os, const Value& value, const ExifData* metadata) {
  //! List of lens ids which require special treatment from printMinoltaSonyLensID
  static constexpr struct LensIdFct {
    uint32_t idx;
    PrintFct fct;

    bool operator==(uint32_t i) const {
      return i == idx;
    }
  } lensIdFct[] = {
      {0x001cu, &resolveLens0x1c}, {0x0029u, &resolveLens0x29}, {0x0034u, &resolveLens0x34},
      {0x0080u, &resolveLens0x80}, {0x00ffu, &resolveLens0xff}, {0xffffu, &resolveLens0xffff},
      //{0x00ffu, &resolveLensTypeUsingExiftool}, // was used for debugging
  };
  // #1145 end - respect lenses with shared LensID
  // ----------------------------------------------------------------------

  // #1034
  const std::string undefined("undefined");
  const std::string minolta("minolta");
  const std::string sony("sony");
  if (Internal::readExiv2Config(minolta, value.toString(), undefined) != undefined) {
    return os << Internal::readExiv2Config(minolta, value.toString(), undefined);
  }
  if (Internal::readExiv2Config(sony, value.toString(), undefined) != undefined) {
    return os << Internal::readExiv2Config(sony, value.toString(), undefined);
  }

  // #1145 - respect lenses with shared LensID
  uint32_t index = value.toUint32();
  if (metadata)
    if (auto f = Exiv2::find(lensIdFct, index))
      return f->fct(os, value, metadata);
  return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Minolta A100 and all other Sony Alpha camera color mode values to readable labels
constexpr TagDetails minoltaSonyColorMode[] = {
    {0, N_("Standard")}, {1, N_("Vivid Color")},         {2, N_("Portrait")},        {3, N_("Landscape")},
    {4, N_("Sunset")},   {5, N_("Night View/Portrait")}, {6, N_("Black & White")},   {7, N_("AdobeRGB")},
    {12, N_("Neutral")}, {100, N_("Neutral")},           {101, N_("Clear")},         {102, N_("Deep")},
    {103, N_("Light")},  {104, N_("Night View")},        {105, N_("Autumn Leaves")},
};

std::ostream& printMinoltaSonyColorMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyColorMode)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Minolta/Sony bool function values to readable labels
constexpr TagDetails minoltaSonyBoolFunction[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

std::ostream& printMinoltaSonyBoolValue(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyBoolFunction)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Minolta/Sony bool inverse function values to readable labels
constexpr TagDetails minoltaSonyBoolInverseFunction[] = {
    {0, N_("On")},
    {1, N_("Off")},
};

std::ostream& printMinoltaSonyBoolInverseValue(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyBoolInverseFunction)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings focus mode values to readable labels
constexpr TagDetails minoltaSonyAFAreaMode[] = {
    {0, N_("Wide")},
    {1, N_("Local")},
    {2, N_("Spot")},
};

std::ostream& printMinoltaSonyAFAreaMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyAFAreaMode)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings Local AF Area Point values to readable labels
constexpr TagDetails minoltaSonyLocalAFAreaPoint[] = {
    {1, N_("Center")},       {2, N_("Top")},        {3, N_("Top-Right")},   {4, N_("Right")},
    {5, N_("Bottom-Right")}, {6, N_("Bottom")},     {7, N_("Bottom-Left")}, {8, N_("Left")},
    {9, N_("Top-Left")},     {10, N_("Far-Right")}, {11, N_("Far-Left")},
};

std::ostream& printMinoltaSonyLocalAFAreaPoint(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyLocalAFAreaPoint)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings dynamic range optimizer mode values to readable labels
constexpr TagDetails minoltaSonyDynamicRangeOptimizerMode[] = {
    {0, N_("Off")}, {1, N_("Standard")}, {2, N_("Advanced Auto")}, {3, N_("Advanced Level")}, {4097, N_("Auto")},
};

std::ostream& printMinoltaSonyDynamicRangeOptimizerMode(std::ostream& os, const Value& value,
                                                        const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyDynamicRangeOptimizerMode)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings priority setup shutter release values to readable labels
constexpr TagDetails minoltaSonyPrioritySetupShutterRelease[] = {
    {0, N_("AF")},
    {1, N_("Release")},
};

std::ostream& printMinoltaSonyPrioritySetupShutterRelease(std::ostream& os, const Value& value,
                                                          const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyPrioritySetupShutterRelease)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings quality values to readable labels
constexpr TagDetails minoltaSonyQualityCs[] = {
    {0, N_("RAW")},       {2, N_("CRAW")},       {16, N_("Extra Fine")}, {32, N_("Fine")},
    {34, N_("RAW+JPEG")}, {35, N_("CRAW+JPEG")}, {48, N_("Standard")},
};

std::ostream& printMinoltaSonyQualityCs(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyQualityCs)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony camera settings rotation values to readable labels
constexpr TagDetails minoltaSonyRotation[] = {
    {0, N_("Horizontal (normal)")},
    {1, N_("Rotate 90 CW")},
    {2, N_("Rotate 270 CW")},
};

std::ostream& printMinoltaSonyRotation(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyRotation)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Minolta/Sony scene mode values to readable labels
constexpr TagDetails minoltaSonySceneMode[] = {
    {0, N_("Standard")},
    {1, N_("Portrait")},
    {2, N_("Text")},
    {3, N_("Night Scene")},
    {4, N_("Sunset")},
    {5, N_("Sports")},
    {6, N_("Landscape")},
    {7, N_("Night Portrait")},
    {8, N_("Macro")},
    {9, N_("Super Macro")},
    {16, N_("Auto")},
    {17, N_("Night View/Portrait")},
    {18, N_("Sweep Panorama")},
    {19, N_("Handheld Night Shot")},
    {20, N_("Anti Motion Blur")},
    {21, N_("Cont. Priority AE")},
    {22, N_("Auto+")},
    {23, N_("3D Sweep Panorama")},
    {24, N_("Superior Auto")},
    {25, N_("High Sensitivity")},
    {26, N_("Fireworks")},
    {27, N_("Food")},
    {28, N_("Pet")},
    {33, N_("HDR")},
    {0xffff, N_("n/a")},
};

std::ostream& printMinoltaSonySceneMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonySceneMode)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony/Minolta teleconverter model values to readable labels
constexpr TagDetails minoltaSonyTeleconverterModel[] = {
    {0x00, N_("None")},
    {0x04, N_("Minolta/Sony AF 1.4x APO (D) (0x04)")},
    {0x05, N_("Minolta/Sony AF 2x APO (D) (0x05)")},
    {0x48, N_("Minolta/Sony AF 2x APO (D)")},
    {0x50, N_("Minolta AF 2x APO II")},
    {0x60, N_("Minolta AF 2x APO")},
    {0x88, N_("Minolta/Sony AF 1.4x APO (D)")},
    {0x90, N_("Minolta AF 1.4x APO II")},
    {0xa0, N_("Minolta AF 1.4x APO")},
};

std::ostream& printMinoltaSonyTeleconverterModel(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyTeleconverterModel)(os, value, metadata);
}

// ----------------------------------------------------------------------------------------------------

//! Lookup table to translate Sony/Minolta zone matching values to readable labels
constexpr TagDetails minoltaSonyZoneMatching[] = {
    {0, N_("ISO Setting Used")},
    {1, N_("High Key")},
    {2, N_("Low Key")},
};

std::ostream& printMinoltaSonyZoneMatching(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_TAG(minoltaSonyZoneMatching)(os, value, metadata);
}

}  // namespace Exiv2::Internal
