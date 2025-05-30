// SPDX-License-Identifier: GPL-2.0-or-later

// *****************************************************************************
// included header files
#include "olympusmn_int.hpp"

#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "makernote_int.hpp"
#include "tags_int.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <array>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! OffOn, multiple tags
constexpr TagDetails olympusOffOn[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! NoYes, multiple tags
constexpr TagDetails olympusNoYes[] = {
    {0, N_("No")},
    {1, N_("Yes")},
};

//! Quality, tag 0x0201
constexpr TagDetails olympusQuality[] = {
    {1, N_("Standard Quality (SQ)")},
    {2, N_("High Quality (HQ)")},
    {3, N_("Super High Quality (SHQ)")},
    {6, N_("Raw")},
};

//! Macro, tag 0x0202
constexpr TagDetails olympusMacro[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("Super macro")},
};

//! OneTouchWB, tag 0x0302
constexpr TagDetails olympusOneTouchWb[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("On (preset)")},
};

//! SceneMode, tag 0x403 and CameraSettings tag 0x509
constexpr TagDetails olympusSceneMode[] = {
    {0, N_("Standard")},
    {6, N_("Auto")},
    {7, N_("Sport")},
    {8, N_("Portrait")},
    {9, N_("Landscape+Portrait")},
    {10, N_("Landscape")},
    {11, N_("Night Scene")},
    {12, N_("Self Portrait")},
    {13, N_("Panorama")},
    {14, N_("2 in 1")},
    {15, N_("Movie")},
    {16, N_("Landscape+Portrait")},
    {17, N_("Night+Portrait")},
    {18, N_("Indoor")},
    {19, N_("Fireworks")},
    {20, N_("Sunset")},
    {22, N_("Macro")},
    {23, N_("Super Macro")},
    {24, N_("Food")},
    {25, N_("Documents")},
    {26, N_("Museum")},
    {27, N_("Shoot & Select")},
    {28, N_("Beach & Snow")},
    {29, N_("Self Portrait+Timer")},
    {30, N_("Candle")},
    {31, N_("Available Light")},
    {32, N_("Behind Glass")},
    {33, N_("My Mode")},
    {34, N_("Pet")},
    {35, N_("Underwater Wide1")},
    {36, N_("Underwater Macro")},
    {37, N_("Shoot & Select1")},
    {38, N_("Shoot & Select2")},
    {39, N_("High Key")},
    {40, N_("Digital Image Stabilization")},
    {41, N_("Auction")},
    {42, N_("Beach")},
    {43, N_("Snow")},
    {44, N_("Underwater Wide2")},
    {45, N_("Low Key")},
    {46, N_("Children")},
    {47, N_("Vivid")},
    {48, N_("Nature Macro")},
    {49, N_("Underwater Snapshot")},
    {50, N_("Shooting Guide")},
};

//! FlashDevice, tag 0x1005
constexpr TagDetails olympusFlashDevice[] = {
    {0, N_("None")},
    {1, N_("Internal")},
    {4, N_("External")},
    {5, N_("Internal + External")},
};

//! FocusRange, tag 0x100a
constexpr TagDetails olympusFocusRange[] = {
    {0, N_("Normal")},
    {1, N_("Macro")},
};

//! FocusMode, tag 0x100b
constexpr TagDetails olympusFocusMode[] = {
    {0, N_("Auto")},
    {1, N_("Manual")},
};

//! Sharpness, tag 0x100f
constexpr TagDetails olympusSharpness[] = {
    {0, N_("Normal")},
    {1, N_("Hard")},
    {2, N_("Soft")},
};

//! Contrast, tag 0x1029
constexpr TagDetails olympusContrast[] = {
    {0, N_("High")},
    {1, N_("Normal")},
    {2, N_("Low")},
};

//! CCDScanMode, tag 0x1039
constexpr TagDetails olympusCCDScanMode[] = {
    {0, N_("Interlaced")},
    {1, N_("Progressive")},
};

// Olympus Tag Info
constexpr TagInfo OlympusMakerNote::tagInfo_[] = {

    /* TODO:
       add Minolta makenotes tags here (0x0000-0x0103). See Exiftool database.*/
    {0x0000, "0x0000", "0x0000", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0100, "ThumbnailImage", N_("Thumbnail Image"), N_("Thumbnail image"), IfdId::olympusId, SectionId::makerTags,
     undefined, -1, printValue},

    {0x0104, "BodyFirmwareVersion", N_("Body Firmware Version"), N_("Body firmware version"), IfdId::olympusId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0200, "SpecialMode", N_("Special Mode"), N_("Picture taking mode"), IfdId::olympusId, SectionId::makerTags,
     unsignedLong, -1, print0x0200},
    {0x0201, "Quality", N_("Quality"), N_("Image quality setting"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusQuality)},
    {0x0202, "Macro", N_("Macro"), N_("Macro mode"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusMacro)},
    {0x0203, "BWMode", N_("Black & White Mode"), N_("Black and white mode"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0204, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom ratio"), IfdId::olympusId, SectionId::makerTags,
     unsignedRational, -1, print0x0204},
    {0x0205, "FocalPlaneDiagonal", N_("Focal Plane Diagonal"), N_("Focal plane diagonal"), IfdId::olympusId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x0206, "LensDistortionParams", N_("Lens Distortion Parameters"), N_("Lens distortion parameters"),
     IfdId::olympusId, SectionId::makerTags, signedShort, -1, printValue},
    {0x0207, "CameraType", N_("Camera Type"), N_("Camera type"), IfdId::olympusId, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x0208, "PictureInfo", N_("Picture Info"), N_("ASCII format data such as [PictureInfo]"), IfdId::olympusId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0209, "CameraID", N_("Camera ID"), N_("Camera ID data"), IfdId::olympusId, SectionId::makerTags, asciiString, -1,
     print0x0209},
    {0x020b, "ImageWidth2", N_("Image Width 2"), N_("Image width 2"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x020c, "ImageHeight2", N_("Image Height 2"), N_("Image height 2"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x020d, "Software", N_("Software"), N_("Software"), IfdId::olympusId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0280, "PreviewImage", N_("Preview Image"), N_("Preview image"), IfdId::olympusId, SectionId::makerTags,
     unsignedByte, -1, printValue},
    {0x0300, "PreCaptureFrames", N_("Pre Capture Frames"), N_("Pre-capture frames"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0301, "WhiteBoard", N_("White Board"), N_("White board"), IfdId::olympusId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0302, "OneTouchWB", N_("One Touch WB"), N_("One touch white balance"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOneTouchWb)},
    {0x0303, "WhiteBalanceBracket", N_("White Balance Bracket"), N_("White balance bracket"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0304, "WhiteBalanceBias", N_("White Balance Bias"), N_("White balance bias"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0403, "SceneMode", N_("Scene Mode"), N_("Scene mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusSceneMode)},
    {0x0404, "Firmware", N_("Firmware"), N_("Firmware"), IfdId::olympusId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), IfdId::olympusId, SectionId::makerTags, undefined,
     -1, printValue},
    {0x0f00, "DataDump1", N_("Data Dump 1"), N_("Various camera settings 1"), IfdId::olympusId, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0f01, "DataDump2", N_("Data Dump 2"), N_("Various camera settings 2"), IfdId::olympusId, SectionId::makerTags,
     undefined, -1, printValue},
    {0x1000, "ShutterSpeed", N_("Shutter Speed"), N_("Shutter speed value"), IfdId::olympusId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x1001, "ISOSpeed", N_("ISO Speed"), N_("ISO speed value"), IfdId::olympusId, SectionId::makerTags, signedRational,
     -1, printValue},
    {0x1002, "ApertureValue", N_("Aperture Value"), N_("Aperture value"), IfdId::olympusId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x1003, "Brightness", N_("Brightness"), N_("Brightness value"), IfdId::olympusId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x1004, "FlashMode", N_("Flash Mode"), N_("Flash mode"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusOffOn)},
    {0x1005, "FlashDevice", N_("Flash Device"), N_("Flash device"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusFlashDevice)},
    {0x1006, "Bracket", N_("Bracket"), N_("Exposure compensation value"), IfdId::olympusId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x1007, "SensorTemperature", N_("Sensor Temperature"), N_("Sensor temperature"), IfdId::olympusId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x1008, "LensTemperature", N_("Lens Temperature"), N_("Lens temperature"), IfdId::olympusId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x1009, "LightCondition", N_("Light Condition"), N_("Light condition"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x100a, "FocusRange", N_("Focus Range"), N_("Focus range"), IfdId::olympusId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusFocusRange)},
    {0x100b, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusFocusMode)},
    {0x100c, "FocusDistance", N_("Focus Distance"), N_("Manual focus distance"), IfdId::olympusId, SectionId::makerTags,
     unsignedRational, -1, printValue},
    {0x100d, "Zoom", N_("Zoom"), N_("Zoom step count"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x100e, "MacroFocus", N_("Macro Focus"), N_("Macro focus step count"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x100f, "SharpnessFactor", N_("Sharpness Factor"), N_("Sharpness factor"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusSharpness)},
    {0x1010, "FlashChargeLevel", N_("Flash Charge Level"), N_("Flash charge level"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1011, "ColorMatrix", N_("Color Matrix"), N_("Color matrix"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1012, "BlackLevel", N_("BlackLevel"), N_("Black level"), IfdId::olympusId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x1013, "0x1013", "0x1013", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1014, "0x1014", "0x1014", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1015, "WhiteBalance", N_("White Balance"), N_("White balance mode"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, print0x1015},
    {0x1016, "0x1016", "0x1016", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1017, "RedBalance", N_("Red Balance"), N_("Red balance"), IfdId::olympusId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x1018, "BlueBalance", N_("Blue Balance"), N_("Blue balance"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1019, "ColorMatrixNumber", N_("Color Matrix Number"), N_("Color matrix number"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x101a, "SerialNumber2", N_("Serial Number 2"), N_("Serial number 2"), IfdId::olympusId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x101b, "0x101b", "0x101b", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x101c, "0x101c", "0x101c", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x101d, "0x101d", "0x101d", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x101e, "0x101e", "0x101e", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x101f, "0x101f", "0x101f", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1020, "0x1020", "0x1020", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1021, "0x1021", "0x1021", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1022, "0x1022", "0x1022", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1023, "FlashBias", N_("Flash Bias"), N_("Flash exposure compensation"), IfdId::olympusId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x1024, "0x1024", "0x1024", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1025, "0x1025", "0x1025", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, signedRational, -1, printValue},
    {0x1026, "ExternalFlashBounce", N_("External Flash Bounce"), N_("External flash bounce"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1027, "ExternalFlashZoom", N_("External Flash Zoom"), N_("External flash zoom"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1028, "ExternalFlashMode", N_("External Flash Mode"), N_("External flash mode"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1029, "Contrast", N_("Contrast"), N_("Contrast setting"), IfdId::olympusId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusContrast)},
    {0x102a, "SharpnessFactor2", N_("Sharpness Factor 2"), N_("Sharpness factor 2"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x102b, "ColorControl", N_("Color Control"), N_("Color control"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x102c, "ValidBits", N_("ValidBits"), N_("Valid bits"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x102d, "CoringFilter", N_("CoringFilter"), N_("Coring filter"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x102e, "ImageWidth", N_("Image Width"), N_("Image width"), IfdId::olympusId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x102f, "ImageHeight", N_("Image Height"), N_("Image height"), IfdId::olympusId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x1030, "0x1030", "0x1030", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1031, "0x1031", "0x1031", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1032, "0x1032", "0x1032", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x1033, "0x1033", "0x1033", N_("Unknown"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1034, "CompressionRatio", N_("Compression Ratio"), N_("Compression ratio"), IfdId::olympusId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x1035, "Thumbnail", N_("Thumbnail"), N_("Preview image embedded"), IfdId::olympusId, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1036, "ThumbnailOffset", N_("Thumbnail Offset"), N_("Offset of the preview image"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1037, "ThumbnailLength", N_("Thumbnail Length"), N_("Size of the preview image"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1039, "CCDScanMode", N_("CCD Scan Mode"), N_("CCD scan mode"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusCCDScanMode)},
    {0x103a, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x103b, "InfinityLensStep", N_("Infinity Lens Step"), N_("Infinity lens step"), IfdId::olympusId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x103c, "NearLensStep", N_("Near Lens Step"), N_("Near lens step"), IfdId::olympusId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x2010, "Equipment", N_("Equipment Info"), N_("Camera equipment sub-IFD"), IfdId::olympusId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x2020, "CameraSettings", N_("Camera Settings"), N_("Camera Settings sub-IFD"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x2030, "RawDevelopment", N_("Raw Development"), N_("Raw development sub-IFD"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x2031, "RawDevelopment2", N_("Raw Development 2"), N_("Raw development 2 sub-IFD"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x2040, "ImageProcessing", N_("Image Processing"), N_("Image processing sub-IFD"), IfdId::olympusId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x2050, "FocusInfo", N_("Focus Info"), N_("Focus sub-IFD"), IfdId::olympusId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x3000, "RawInfo", N_("Raw Info"), N_("Raw sub-IFD"), IfdId::olympusId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusMakerNoteTag)", "(UnknownOlympusMakerNoteTag)", N_("Unknown OlympusMakerNote tag"),
     IfdId::olympusId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagList() {
  return tagInfo_;
}

// Olympus CameraSettings Tags
//! ExposureMode, tag 0x0200
constexpr TagDetails olympusExposureMode[] = {
    {1, N_("Manual")},        {2, N_("Program")}, {3, N_("Aperture-priority AE")}, {4, N_("Shutter speed priority AE")},
    {5, N_("Program-shift")},
};

//! MeteringMode, tag 0x0202
constexpr TagDetails olympusMeteringMode[] = {
    {2, N_("Center-weighted average")},
    {3, N_("Spot")},
    {5, N_("ESP")},
    {261, N_("Pattern+AF")},
    {515, N_("Spot+Highlight control")},
    {1027, N_("Spot+Shadow control")},
};

//! MacroMode, tag 0x0300
constexpr TagDetails olympusMacroMode[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("Super Macro")},
};

//! FocusMode, tag 0x0301
[[maybe_unused]] constexpr TagDetails olympusCsFocusMode[] = {
    {0, N_("Single AF")}, {1, N_("Sequential shooting AF")}, {2, N_("Continuous AF")}, {3, N_("Multi AF")},
    {10, N_("MF")},
};

//! FocusProcess, tag 0x0302
constexpr TagDetails olympusFocusProcess[] = {
    {0, N_("AF Not Used")},
    {1, N_("AF Used")},
};

//! AFSearch, tag 0x0303
constexpr TagDetails olympusAFSearch[] = {
    {0, N_("Not Ready")},
    {1, N_("Ready")},
};

//! FlashMode, tag 0x0400
constexpr TagDetailsBitmask olympusFlashMode[] = {
    {0x0000, N_("Off")},       {0x0001, N_("On")},        {0x0002, N_("Fill-in")},     {0x0004, N_("Red-eye")},
    {0x0008, N_("Slow-sync")}, {0x0010, N_("Forced On")}, {0x0020, N_("2nd Curtain")},
};

//! FlashRemoteControl, tag 0x0403
constexpr TagDetails olympusFlashRemoteControl[] = {
    {0x0, N_("Off")},
    {0x1, N_("Channel 1, Low")},
    {0x2, N_("Channel 2, Low")},
    {0x3, N_("Channel 3, Low")},
    {0x4, N_("Channel 4, Low")},
    {0x9, N_("Channel 1, Mid")},
    {0xa, N_("Channel 2, Mid")},
    {0xb, N_("Channel 3, Mid")},
    {0xc, N_("Channel 4, Mid")},
    {0x11, N_("Channel 1, High")},
    {0x12, N_("Channel 2, High")},
    {0x13, N_("Channel 3, High")},
    {0x14, N_("Channel 4, High")},
};

//! FlashControlMode, tag 0x0404
constexpr TagDetails olympusFlashControlMode[] = {
    {0, N_("Off")},
    {3, N_("TTL")},
    {4, N_("Auto")},
    {5, N_("Manual")},
};

//! WhiteBalance, tag 0x0500
constexpr TagDetails olympusWhiteBalance[] = {
    {0, N_("Auto")},
    {1, N_("Auto (Keep Warm Color Off)")},
    {16, N_("7500K (Fine Weather with Shade)")},
    {17, N_("6000K (Cloudy)")},
    {18, N_("5300K (Fine Weather)")},
    {20, N_("3000K (Tungsten light)")},
    {21, N_("3600K (Tungsten light-like)")},
    {22, N_("Auto Setup")},
    {23, N_("5500K (Flash)")},
    {33, N_("6600K (Daylight fluorescent)")},
    {34, N_("4500K (Neutral white fluorescent)")},
    {35, N_("4000K (Cool white fluorescent)")},
    {36, N_("White Fluorescent")},
    {48, N_("3600K (Tungsten light-like)")},
    {67, N_("Underwater")},
    {256, N_("One Touch WB 1")},
    {257, N_("One Touch WB 2")},
    {258, N_("One Touch WB 3")},
    {259, N_("One Touch WB 4")},
    {512, N_("Custom WB 1")},
    {513, N_("Custom WB 2")},
    {514, N_("Custom WB 3")},
    {515, N_("Custom WB 4")},
};

//! ModifiedSaturation, tag 0x0504
constexpr TagDetails olympusModifiedSaturation[] = {
    {0, N_("Off")},
    {1, N_("CM1 (Red Enhance)")},
    {2, N_("CM2 (Green Enhance)")},
    {3, N_("CM3 (Blue Enhance)")},
    {4, N_("CM4 (Skin Tones)")},
};

//! ColorSpace, tag 0x0507
constexpr TagDetails olympusColorSpace[] = {
    {0, N_("sRGB")},
    {1, N_("Adobe RGB")},
    {2, N_("Pro Photo RGB")},
};

//! NoiseReduction, tag 0x050a
constexpr TagDetailsBitmask olympusNoiseReduction[] = {
    {0x0001, N_("Noise Reduction")},
    {0x0002, N_("Noise Filter")},
    {0x0004, N_("Noise Filter (ISO Boost)")},
    {0x0008, N_("Auto")},
};

//! PictureMode, tag 0x0520
constexpr TagDetails olympusPictureMode[] = {
    {1, N_("Vivid")},
    {2, N_("Natural")},
    {3, N_("Muted")},
    {4, N_("Portrait")},
    {5, N_("i-Enhance")},
    {6, N_("e-Portrait")},
    {7, N_("Color Creator")},
    {9, N_("Color Profile 1")},
    {10, N_("Color Profile 2")},
    {11, N_("Color Profile 3")},
    {12, N_("Monochrome Profile 1")},
    {13, N_("Monochrome Profile 2")},
    {14, N_("Monochrome Profile 3")},
    {256, N_("Monotone")},
    {512, N_("Sepia")},
};

//! PictureModeBWFilter, tag 0x0525
constexpr TagDetails olympusPictureModeBWFilter[] = {
    {0, N_("n/a")}, {1, N_("Neutral")}, {2, N_("Yellow")}, {3, N_("Orange")}, {4, N_("Red")}, {5, N_("Green")},
};

//! PictureModeTone, tag 0x0526
constexpr TagDetails olympusPictureModeTone[] = {
    {0, N_("n/a")}, {1, N_("Neutral")}, {2, N_("Sepia")}, {3, N_("Blue")}, {4, N_("Purple")}, {5, N_("Green")},
};

constexpr TagDetails artFilters[] = {
    {0, N_("Off")},
    {1, N_("Soft Focus")},
    {2, N_("Pop Art")},
    {3, N_("Pale & Light Color")},
    {4, N_("Light Tone")},
    {5, N_("Pin Hole")},
    {6, N_("Grainy Film")},
    {9, N_("Diorama")},
    {10, N_("Cross Process")},
    {12, N_("Fish Eye")},
    {13, N_("Drawing")},
    {14, N_("Gentle Sepia")},
    {15, N_("Pale & Light Color II")},
    {16, N_("Pop Art II")},
    {17, N_("Pin Hole II")},
    {18, N_("Pin Hole III")},
    {19, N_("Grainy Film II")},
    {20, N_("Dramatic Tone")},
    {21, N_("Punk")},
    {22, N_("Soft Focus 2")},
    {23, N_("Sparkle")},
    {24, N_("Watercolor")},
    {25, N_("Key Line")},
    {26, N_("Key Line II")},
    {27, N_("Miniature")},
    {28, N_("Reflection")},
    {29, N_("Fragmented")},
    {31, N_("Cross Process II")},
    {32, N_("Dramatic Tone II")},
    {33, N_("Watercolor I")},
    {34, N_("Watercolor II")},
    {35, N_("Diorama II")},
    {36, N_("Vintage")},
    {37, N_("Vintage II")},
    {38, N_("Vintage III")},
    {39, N_("Partial Color")},
    {40, N_("Partial Color II")},
    {41, N_("Partial Color III")},
};

//! OlympusCs Quality, tag 0x0603
constexpr TagDetails olympusCsQuality[] = {
    {1, N_("SQ")},
    {2, N_("HQ")},
    {3, N_("SHQ")},
    {4, N_("RAW")},
};

//! Olympus ImageStabilization, tag 0x0604
static constexpr TagDetails olympusImageStabilization[] = {
    {0, N_("Off")}, {1, N_("S-IS 1")}, {2, N_("S-IS 2")}, {3, N_("S-IS 3")}, {4, N_("S-IS AUTO")},
};

constexpr TagInfo OlympusMakerNote::tagInfoCs_[] = {
    {0x0000, "CameraSettingsVersion", N_("Camera Settings Version"), N_("Camera settings version"), IfdId::olympusCsId,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0100, "PreviewImageValid", N_("PreviewImage Valid"), N_("Preview image valid"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(olympusNoYes)},
    {0x0101, "PreviewImageStart", N_("PreviewImage Start"), N_("Preview image start"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0102, "PreviewImageLength", N_("PreviewImage Length"), N_("Preview image length"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0200, "ExposureMode", N_("Exposure Mode"), N_("Exposure mode"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusExposureMode)},
    {0x0201, "AELock", N_("AE Lock"), N_("Auto exposure lock"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0202, "MeteringMode", N_("Metering Mode"), N_("Metering mode"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusMeteringMode)},
    {0x0203, "ExposureShift", N_("Exposure Shift"), N_("Exposure shift"), IfdId::olympusCsId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x0300, "MacroMode", N_("Macro Mode"), N_("Macro mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusMacroMode)},
    {0x0301, "FocusMode", N_("Focus Mode"), N_("Focus mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, printCs0x0301},
    {0x0302, "FocusProcess", N_("Focus Process"), N_("Focus process"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusFocusProcess)},
    {0x0303, "AFSearch", N_("AF Search"), N_("AF search"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusAFSearch)},
    {0x0304, "AFAreas", N_("AF Areas"), N_("AF areas"), IfdId::olympusCsId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    {0x0305, "AFPointSelected", N_("AFPointSelected"), N_("AFPointSelected"), IfdId::olympusCsId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x0307, "AFFineTuneAdj", N_("AF Fine Tune Adjust"), N_("AF fine tune adjust"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0400, "FlashMode", N_("Flash Mode"), N_("Flash mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG_BITMASK(olympusFlashMode)},
    {0x0401, "FlashExposureComp", N_("Flash Exposure Compensation"), N_("Flash exposure compensation"),
     IfdId::olympusCsId, SectionId::makerTags, signedRational, -1, printValue},
    {0x0403, "FlashRemoteControl", N_("Flash Remote Control"), N_("Flash remote control"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusFlashRemoteControl)},
    {0x0404, "FlashControlMode", N_("Flash Control Mode"), N_("Flash control mode"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusFlashControlMode)},
    {0x0405, "FlashIntensity", N_("Flash Intensity"), N_("Flash intensity"), IfdId::olympusCsId, SectionId::makerTags,
     signedRational, -1, printValue},
    {0x0406, "ManualFlashStrength", N_("Manual Flash Strength"), N_("Manual flash strength"), IfdId::olympusCsId,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x0500, "WhiteBalance", N_("White Balance 2"), N_("White balance 2"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusWhiteBalance)},
    {0x0501, "WhiteBalanceTemperature", N_("White Balance Temperature"), N_("White balance temperature"),
     IfdId::olympusCsId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0502, "WhiteBalanceBracket", N_("White Balance Bracket"), N_("White balance bracket"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0503, "CustomSaturation", N_("Custom Saturation"), N_("Custom saturation"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0504, "ModifiedSaturation", N_("Modified Saturation"), N_("Modified saturation"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusModifiedSaturation)},
    {0x0505, "ContrastSetting", N_("Contrast Setting"), N_("Contrast setting"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0506, "SharpnessSetting", N_("Sharpness Setting"), N_("Sharpness setting"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0507, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusColorSpace)},
    {0x0509, "SceneMode", N_("Scene Mode"), N_("Scene mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusSceneMode)},
    {0x050a, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG_BITMASK(olympusNoiseReduction)},
    {0x050b, "DistortionCorrection", N_("Distortion Correction"), N_("Distortion correction"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x050c, "ShadingCompensation", N_("Shading Compensation"), N_("Shading compensation"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x050d, "CompressionFactor", N_("Compression Factor"), N_("Compression factor"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x050f, "Gradation", N_("Gradation"), N_("Gradation"), IfdId::olympusCsId, SectionId::makerTags, signedShort, -1,
     print0x050f},
    {0x0520, "PictureMode", N_("Picture Mode"), N_("Picture mode"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusPictureMode)},
    {0x0521, "PictureModeSaturation", N_("Picture Mode Saturation"), N_("Picture mode saturation"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0522, "PictureModeHue", N_("Picture Mode Hue"), N_("Picture mode hue"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0523, "PictureModeContrast", N_("Picture Mode Contrast"), N_("Picture mode contrast"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0524, "PictureModeSharpness", N_("Picture Mode Sharpness"), N_("Picture mode sharpness"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0525, "PictureModeBWFilter", N_("Picture Mode BW Filter"), N_("Picture mode BW filter"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(olympusPictureModeBWFilter)},
    {0x0526, "PictureModeTone", N_("Picture Mode Tone"), N_("Picture mode tone"), IfdId::olympusCsId,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(olympusPictureModeTone)},
    {0x0527, "NoiseFilter", N_("Noise Filter"), N_("Noise filter"), IfdId::olympusCsId, SectionId::makerTags,
     signedShort, -1, print0x0527},
    {0x0529, "ArtFilter", N_("Art Filter"), N_("Art filter"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, print0x0529},
    {0x052c, "MagicFilter", N_("Magic Filter"), N_("Magic filter"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, print0x0529},
    {0x0600, "DriveMode", N_("Drive Mode"), N_("Drive mode"), IfdId::olympusCsId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0601, "PanoramaMode", N_("Panorama Mode"), N_("Panorama mode"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0603, "Quality", N_("Image Quality 2"), N_("Image quality 2"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusCsQuality)},
    {0x0604, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(olympusImageStabilization)},
    {0x0900, "ManometerPressure", N_("Manometer Pressure"), N_("Manometer pressure"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0901, "ManometerReading", N_("Manometer Reading"), N_("Manometer reading"), IfdId::olympusCsId,
     SectionId::makerTags, signedLong, -1, printValue},
    {0x0902, "ExtendedWBDetect", N_("Extended WB Detect"), N_("Extended WB detect"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0903, "LevelGaugeRoll", N_("Level Gauge Roll"), N_("Level gauge roll"), IfdId::olympusCsId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0904, "LevelGaugePitch", N_("Level Gauge Pitch"), N_("Level gauge pitch"), IfdId::olympusCsId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    // End of list marker
    {0xffff, "(UnknownOlympusCsTag)", "(UnknownOlympusCsTag)", N_("Unknown OlympusCs tag"), IfdId::olympusCsId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListCs() {
  return tagInfoCs_;
}

//! OlympusEq FlashType, tag 0x1000
constexpr TagDetails olympusEqFlashType[] = {
    {0, N_("None")},
    {2, N_("Simple E-System")},
    {3, N_("E-System")},
};

//! OlympusEq FlashModel, tag 0x1001
constexpr TagDetails olympusEqFlashModel[] = {
    {0, N_("None")}, {1, "FL-20"},  {2, "FL-50"},  {3, "RF-11"}, {4, "TF-22"},
    {5, "FL-36"},    {6, "FL-50R"}, {7, "FL-36R"}, {9, "FL-14"}, {11, "FL-600R"},
};

constexpr TagInfo OlympusMakerNote::tagInfoEq_[] = {
    {0x0000, "EquipmentVersion", N_("Equipment Version"), N_("Equipment version"), IfdId::olympusEqId,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0100, "CameraType", N_("Camera Type"), N_("Camera type"), IfdId::olympusEqId, SectionId::makerTags, asciiString,
     -1, printValue},
    {0x0101, "SerialNumber", N_("Serial Number"), N_("Serial number"), IfdId::olympusEqId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0102, "InternalSerialNumber", N_("Internal Serial Number"), N_("Internal serial number"), IfdId::olympusEqId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0103, "FocalPlaneDiagonal", N_("Focal Plane Diagonal"), N_("Focal plane diagonal"), IfdId::olympusEqId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x0104, "BodyFirmwareVersion", N_("Body Firmware Version"), N_("Body firmware version"), IfdId::olympusEqId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0201, "LensType", N_("Lens Type"), N_("Lens type"), IfdId::olympusEqId, SectionId::makerTags, unsignedByte, -1,
     print0x0201},
    {0x0202, "LensSerialNumber", N_("Lens Serial Number"), N_("Lens serial number"), IfdId::olympusEqId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0203, "LensModel", N_("Lens Model"), N_("Lens model"), IfdId::olympusEqId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0204, "LensFirmwareVersion", N_("Lens Firmware Version"), N_("Lens firmware version"), IfdId::olympusEqId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0205, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal"),
     IfdId::olympusEqId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0206, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal"),
     IfdId::olympusEqId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0207, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), IfdId::olympusEqId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0208, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), IfdId::olympusEqId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x020a, "MaxApertureAtCurrentFocal", N_("Max Aperture At Current Focal"), N_("Max aperture at current focal"),
     IfdId::olympusEqId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x020b, "LensProperties", N_("Lens Properties"), N_("Lens properties"), IfdId::olympusEqId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0301, "Extender", N_("Extender"), N_("Extender"), IfdId::olympusEqId, SectionId::makerTags, unsignedByte, -1,
     printEq0x0301},
    {0x0302, "ExtenderSerialNumber", N_("Extender Serial Number"), N_("Extender serial number"), IfdId::olympusEqId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0303, "ExtenderModel", N_("Extender Model"), N_("Extender model"), IfdId::olympusEqId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0304, "ExtenderFirmwareVersion", N_("Extender Firmware Version"), N_("Extender firmware version"),
     IfdId::olympusEqId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0403, "ConversionLens", N_("Conversion Lens"), N_("Conversion lens"), IfdId::olympusEqId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x1000, "FlashType", N_("Flash Type"), N_("Flash type"), IfdId::olympusEqId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusEqFlashType)},
    {0x1001, "FlashModel", N_("Flash Model"), N_("Flash model"), IfdId::olympusEqId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusEqFlashModel)},
    {0x1002, "FlashFirmwareVersion", N_("Flash Firmware Version"), N_("Flash firmware version"), IfdId::olympusEqId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x1003, "FlashSerialNumber", N_("FlashSerialNumber"), N_("FlashSerialNumber"), IfdId::olympusEqId,
     SectionId::makerTags, asciiString, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusEqTag)", "(UnknownOlympusEqTag)", N_("Unknown OlympusEq tag"), IfdId::olympusEqId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListEq() {
  return tagInfoEq_;
}

//! OlympusRd ColorSpace, tag 0x0108
constexpr TagDetails olympusRdColorSpace[] = {
    {0, N_("sRGB")},
    {1, N_("Adobe RGB")},
    {2, N_("Pro Photo RGB")},
};

//! OlympusRd Engine, tag 0x0109
constexpr TagDetails olympusRdEngine[] = {
    {0, N_("High Speed")},
    {1, N_("High Function")},
    {2, N_("Advanced High Speed")},
    {3, N_("Advanced High Function")},
};

//! OlympusRd EditStatus, tag 0x010b
constexpr TagDetails olympusRdEditStatus[] = {
    {0, N_("Original")},
    {1, N_("Edited (Landscape)")},
    {6, N_("Edited (Portrait)")},
    {8, N_("Edited (Portrait)")},
};

//! OlympusRd Settings, tag 0x010c
constexpr TagDetailsBitmask olympusRdSettings[] = {
    {0x0001, N_("WB Color Temp")}, {0x0004, N_("WB Gray Point")},   {0x0008, N_("Saturation")},
    {0x0010, N_("Contrast")},      {0x0020, N_("Sharpness")},       {0x0040, N_("Color Space")},
    {0x0080, N_("High Function")}, {0x0100, N_("Noise Reduction")},
};

constexpr TagInfo OlympusMakerNote::tagInfoRd_[] = {
    {0x0000, "RawDevVersion", N_("Raw Development Version"), N_("Raw development version"), IfdId::olympusRdId,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0100, "ExposureBiasValue", N_("Exposure Bias Value"), N_("Exposure bias value"), IfdId::olympusRdId,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x0101, "WhiteBalanceValue", N_("White Balance Value"), N_("White balance value"), IfdId::olympusRdId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0102, "WBFineAdjustment", N_("WB Fine Adjustment"), N_("WB fine adjustment"), IfdId::olympusRdId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0103, "GrayPoint", N_("Gray Point"), N_("Gray point"), IfdId::olympusRdId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0104, "SaturationEmphasis", N_("Saturation Emphasis"), N_("Saturation emphasis"), IfdId::olympusRdId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0105, "MemoryColorEmphasis", N_("Memory Color Emphasis"), N_("Memory color emphasis"), IfdId::olympusRdId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0106, "ContrastValue", N_("Contrast Value"), N_("Contrast value"), IfdId::olympusRdId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0107, "SharpnessValue", N_("Sharpness Value"), N_("Sharpness value"), IfdId::olympusRdId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0108, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::olympusRdId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRdColorSpace)},
    {0x0109, "Engine", N_("Engine"), N_("Engine"), IfdId::olympusRdId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusRdEngine)},
    {0x010a, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::olympusRdId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG_BITMASK(olympusNoiseReduction)},
    {0x010b, "EditStatus", N_("Edit Status"), N_("Edit status"), IfdId::olympusRdId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRdEditStatus)},
    {0x010c, "Settings", N_("Settings"), N_("Settings"), IfdId::olympusRdId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG_BITMASK(olympusRdSettings)},
    // End of list marker
    {0xffff, "(UnknownOlympusRdTag)", "(UnknownOlympusRdTag)", N_("Unknown OlympusRd tag"), IfdId::olympusRdId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListRd() {
  return tagInfoRd_;
}

//! OlympusRd2 WhiteBalance, tag 0x0101
constexpr TagDetails olympusRd2WhiteBalance[] = {
    {1, N_("Color Temperature")},
    {2, N_("Gray Point")},
};

//! OlympusRd2 ColorSpace, tag 0x0109
constexpr TagDetails olympusRd2ColorSpace[] = {
    {0, N_("sRGB")},
    {1, N_("Adobe RGB")},
    {2, N_("Pro Photo RGB")},
};

//! OlympusRd2 Engine, tag 0x010b
constexpr TagDetails olympusRd2Engine[] = {
    {0, N_("High Speed")},
    {1, N_("High Function")},
};

//! OlympusRd2 PictureMode, tag 0x010c
constexpr TagDetails olympusRd2PictureMode[] = {
    {1, N_("Vivid")}, {2, N_("Natural")}, {3, N_("Muted")}, {256, N_("Monotone")}, {512, N_("Sepia")},
};

//! OlympusRd2 PM_BWFilter, tag 0x0110
constexpr TagDetails olympusRd2PM_BWFilter[] = {
    {1, N_("Neutral")}, {2, N_("Yellow")}, {3, N_("Orange")}, {4, N_("Red")}, {5, N_("Green")},
};

//! OlympusRd2 PMPictureTone, tag 0x0111
constexpr TagDetails olympusRd2PMPictureTone[] = {
    {1, N_("Neutral")}, {2, N_("Sepia")}, {3, N_("Blue")}, {4, N_("Purple")}, {5, N_("Green")},
};

constexpr TagInfo OlympusMakerNote::tagInfoRd2_[] = {
    {0x0000, "RawDev2Version", N_("Raw Development 2 Version"), N_("Raw development 2 version"), IfdId::olympusRd2Id,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0100, "ExposureBiasValue", N_("Exposure Bias Value"), N_("Exposure bias value"), IfdId::olympusRd2Id,
     SectionId::makerTags, signedRational, -1, printValue},
    {0x0101, "WhiteBalance", N_("White Balance"), N_("White balance"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRd2WhiteBalance)},
    {0x0102, "WhiteBalanceValue", N_("White Balance Value"), N_("White balance value"), IfdId::olympusRd2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0103, "WBFineAdjustment", N_("WB Fine Adjustment"), N_("White balance fine adjustment"), IfdId::olympusRd2Id,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0104, "GrayPoint", N_("Gray Point"), N_("Gray point"), IfdId::olympusRd2Id, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0105, "ContrastValue", N_("Contrast Value"), N_("Contrast value"), IfdId::olympusRd2Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0106, "SharpnessValue", N_("Sharpness Value"), N_("Sharpness value"), IfdId::olympusRd2Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0107, "SaturationEmphasis", N_("Saturation Emphasis"), N_("Saturation emphasis"), IfdId::olympusRd2Id,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0108, "MemoryColorEmphasis", N_("Memory Color Emphasis"), N_("Memory color emphasis"), IfdId::olympusRd2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0109, "ColorSpace", N_("Color Space"), N_("Color space"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRd2ColorSpace)},
    {0x010a, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG_BITMASK(olympusNoiseReduction)},
    {0x010b, "Engine", N_("Engine"), N_("Engine"), IfdId::olympusRd2Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(olympusRd2Engine)},
    {0x010c, "PictureMode", N_("Picture Mode"), N_("Picture mode"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRd2PictureMode)},
    {0x010d, "PMSaturation", N_("PM Saturation"), N_("Picture mode saturation"), IfdId::olympusRd2Id,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x010e, "PMContrast", N_("PM Contrast"), N_("Picture mode contrast"), IfdId::olympusRd2Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x010f, "PMSharpness", N_("PM Sharpness"), N_("Picture mode sharpness"), IfdId::olympusRd2Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0110, "PM_BWFilter", N_("PM BW Filter"), N_("PM BW filter"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRd2PM_BWFilter)},
    {0x0111, "PMPictureTone", N_("PM Picture Tone"), N_("PM picture tone"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRd2PMPictureTone)},
    {0x0112, "Gradation", N_("Gradation"), N_("Gradation"), IfdId::olympusRd2Id, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x0113, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::olympusRd2Id, SectionId::makerTags, signedShort,
     -1, printValue},
    {0x0119, "AutoGradation", N_("Auto Gradation"), N_("Auto gradation"), IfdId::olympusRd2Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0120, "PMNoiseFilter", N_("PM Noise Filter"), N_("Picture mode noise filter"), IfdId::olympusRd2Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusRd2Tag)", "(UnknownOlympusRd2Tag)", N_("Unknown OlympusRd2 tag"), IfdId::olympusRd2Id,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListRd2() {
  return tagInfoRd2_;
}

//! OlympusIp MultipleExposureMode, tag 0x101c
constexpr TagDetails olympusIpMultipleExposureMode[] = {
    {0, N_("Off")},
    {2, N_("On (2 frames)")},
    {3, N_("On (3 frames)")},
};

//! OlympusIp olympusIpAspectRatio, tag 0x101c
constexpr TagDetails olympusIpAspectRatio[] = {
    {1, "4:3"}, {2, "3:2"}, {3, "16:9"}, {4, "6:6"}, {5, "5:4"}, {6, "7:6"}, {7, "6:5"}, {8, "7:5"}, {9, "3:4"},
};

constexpr TagInfo OlympusMakerNote::tagInfoIp_[] = {
    {0x0000, "ImageProcessingVersion", N_("Image Processing Version"), N_("Image processing version"),
     IfdId::olympusIpId, SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0100, "WB_RBLevels", N_("WB RB Levels"), N_("WB RB levels"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0102, "WB_RBLevels3000K", N_("WB RB Levels 3000K"), N_("WB RB levels 3000K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0103, "WB_RBLevels3300K", N_("WB RB Levels 3300K"), N_("WB RB levels 3300K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0104, "WB_RBLevels3600K", N_("WB RB Levels 3600K"), N_("WB RB levels 3600K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0105, "WB_RBLevels3900K", N_("WB RB Levels 3900K"), N_("WB RB levels 3900K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0106, "WB_RBLevels4000K", N_("WB RB Levels 4000K"), N_("WB RB levels 4000K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0107, "WB_RBLevels4300K", N_("WB RB Levels 4300K"), N_("WB RB levels 4300K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0108, "WB_RBLevels4500K", N_("WB RB Levels 4500K"), N_("WB RB levels 4500K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0109, "WB_RBLevels4800K", N_("WB RB Levels 4800K"), N_("WB RB levels 4800K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010a, "WB_RBLevels5300K", N_("WB RB Levels 5300K"), N_("WB RB levels 5300K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010b, "WB_RBLevels6000K", N_("WB RB Levels 6000K"), N_("WB RB levels 6000K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010c, "WB_RBLevels6600K", N_("WB RB Levels 6600K"), N_("WB RB levels 6600K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010d, "WB_RBLevels7500K", N_("WB RB Levels 7500K"), N_("WB RB levels 7500K"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010e, "WB_RBLevelsCWB1", N_("WB RB Levels CWB1"), N_("WB RB levels CWB1"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x010f, "WB_RBLevelsCWB2", N_("WB RB Levels CWB2"), N_("WB RB levels CWB2"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0110, "WB_RBLevelsCWB3", N_("WB RB Levels CWB3"), N_("WB RB levels CWB3"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0111, "WB_RBLevelsCWB4", N_("WB RB Levels CWB4"), N_("WB RB levels CWB4"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0113, "WB_GLevel3000K", N_("WB G Level 3000K"), N_("WB G level 3000K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0114, "WB_GLevel3300K", N_("WB G Level 3300K"), N_("WB G level 3300K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0115, "WB_GLevel3600K", N_("WB G Level 3600K"), N_("WB G level 3600K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0116, "WB_GLevel3900K", N_("WB G Level 3900K"), N_("WB G level 3900K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0117, "WB_GLevel4000K", N_("WB G Level 4000K"), N_("WB G level 4000K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0118, "WB_GLevel4300K", N_("WB G Level 4300K"), N_("WB G level 4300K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0119, "WB_GLevel4500K", N_("WB G Level 4500K"), N_("WB G level 4500K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011a, "WB_GLevel4800K", N_("WB G Level 4800K"), N_("WB G level 4800K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011b, "WB_GLevel5300K", N_("WB G Level 5300K"), N_("WB G level 5300K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011c, "WB_GLevel6000K", N_("WB G Level 6000K"), N_("WB G level 6000K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011d, "WB_GLevel6600K", N_("WB G Level 6600K"), N_("WB G level 6600K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011e, "WB_GLevel7500K", N_("WB G Level 7500K"), N_("WB G level 7500K"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x011f, "WB_GLevel", N_("WB G Level"), N_("WB G level"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0200, "ColorMatrix", N_("Color Matrix"), N_("Color matrix"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0300, "Enhancer", N_("Enhancer"), N_("Enhancer"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0301, "EnhancerValues", N_("Enhancer Values"), N_("Enhancer values"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0310, "CoringFilter", N_("Coring Filter"), N_("Coring filter"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0311, "CoringValues", N_("Coring Values"), N_("Coring values"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0600, "BlackLevel", N_("Black Level"), N_("Black level"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0610, "GainBase", N_("Gain Base"), N_("Gain base"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0611, "ValidBits", N_("Valid Bits"), N_("Valid bits"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0612, "CropLeft", N_("Crop Left"), N_("Crop left"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0613, "CropTop", N_("Crop Top"), N_("Crop top"), IfdId::olympusIpId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0614, "CropWidth", N_("Crop Width"), N_("Crop width"), IfdId::olympusIpId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x0615, "CropHeight", N_("Crop Height"), N_("Crop height"), IfdId::olympusIpId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x1010, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG_BITMASK(olympusNoiseReduction)},
    {0x1011, "DistortionCorrection", N_("Distortion Correction"), N_("Distortion correction"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1012, "ShadingCompensation", N_("Shading Compensation"), N_("Shading compensation"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x101c, "MultipleExposureMode", N_("Multiple Exposure Mode"), N_("Multiple exposure mode"), IfdId::olympusIpId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusIpMultipleExposureMode)},
    {0x1112, "AspectRatio", N_("Aspect Ratio"), N_("Aspect ratio"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedByte, -1, EXV_PRINT_TAG(olympusIpAspectRatio)},
    {0x1113, "AspectFrame", N_("Aspect Frame"), N_("Aspect frame"), IfdId::olympusIpId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x1200, "FaceDetect", N_("Face Detect"), N_("Face detect"), IfdId::olympusIpId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1201, "FaceDetectArea", N_("Face Detect Area"), N_("Face detect area"), IfdId::olympusIpId, SectionId::makerTags,
     signedShort, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusIpTag)", "(UnknownOlympusIpTag)", N_("Unknown OlympusIp tag"), IfdId::olympusIpId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListIp() {
  return tagInfoIp_;
}

//! OlympusFi ExternalFlashBounce, tag 0x1204
constexpr TagDetails olympusFiExternalFlashBounce[] = {
    {0, N_("Bounce or Off")},
    {1, N_("Direct")},
};

constexpr TagInfo OlympusMakerNote::tagInfoFi_[] = {
    {0x0000, "FocusInfoVersion", N_("Focus Info Version"), N_("Focus info version"), IfdId::olympusFiId,
     SectionId::makerTags, undefined, -1, printExifVersion},
    {0x0209, "AutoFocus", N_("Auto Focus"), N_("Auto focus"), IfdId::olympusFiId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x0210, "SceneDetect", N_("Scene Detect"), N_("Scene detect"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0211, "SceneArea", N_("Scene Area"), N_("Scene area"), IfdId::olympusFiId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x0212, "SceneDetectData", N_("Scene Detect Data"), N_("Scene detect data"), IfdId::olympusFiId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0300, "ZoomStepCount", N_("Zoom Step Count"), N_("Zoom step count"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0301, "FocusStepCount", N_("Focus Step Count"), N_("Focus step count"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0303, "FocusStepInfinity", N_("Focus Step Infinity"), N_("Focus step infinity"), IfdId::olympusFiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0304, "FocusStepNear", N_("Focus Step Near"), N_("Focus step near"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0305, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedRational, -1, print0x0305},
    {0x0308, "AFPoint", N_("AF Point"), N_("AF point"), IfdId::olympusFiId, SectionId::makerTags, unsignedShort, -1,
     print0x0308},
    {0x1201, "ExternalFlash", N_("External Flash"), N_("External flash"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1203, "ExternalFlashGuideNumber", N_("External Flash Guide Number"), N_("External flash guide number"),
     IfdId::olympusFiId, SectionId::makerTags, signedRational, -1, printValue},
    {0x1204, "ExternalFlashBounce", N_("External Flash Bounce"), N_("External flash bounce"), IfdId::olympusFiId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(olympusFiExternalFlashBounce)},
    {0x1205, "ExternalFlashZoom", N_("External Flash Zoom"), N_("External flash zoom"), IfdId::olympusFiId,
     SectionId::makerTags, unsignedRational, -1, printValue},
    {0x1208, "InternalFlash", N_("Internal Flash"), N_("Internal flash"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusOffOn)},
    {0x1209, "ManualFlash", N_("Manual Flash"), N_("Manual flash"), IfdId::olympusFiId, SectionId::makerTags,
     unsignedShort, -1, print0x1209},
    {0x1500, "SensorTemperature", N_("Sensor Temperature"), N_("Sensor temperature"), IfdId::olympusFiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x1600, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::olympusFiId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusFiTag)", "(UnknownOlympusFiTag)", N_("Unknown OlympusFi tag"), IfdId::olympusFiId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListFi() {
  return tagInfoFi_;
}

constexpr TagInfo OlympusMakerNote::tagInfoFe_[] = {
    {0x0100, "BodyFirmwareVersion", N_("Body Firmware Version"), N_("Body firmware version"), IfdId::olympusFe1Id,
     SectionId::makerTags, asciiString, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusFeTag)", "(UnknownOlympusFeTag)", N_("Unknown OlympusFe tag"), IfdId::olympusFe1Id,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListFe() {
  return tagInfoFe_;
}

//! OlympusRi LightSource, tag 0x1000
constexpr TagDetails olympusRiLightSource[] = {
    {0, N_("Unknown")},
    {16, N_("Shade")},
    {17, N_("Cloudy")},
    {18, N_("Fine Weather")},
    {20, N_("Tungsten (incandescent)")},
    {22, N_("Evening Sunlight")},
    {33, N_("Daylight Fluorescent (D 5700 - 7100K)")},
    {34, N_("Day White Fluorescent (N 4600 - 5400K)")},
    {35, N_("Cool White Fluorescent (W 3900 - 4500K)")},
    {36, N_("White Fluorescent (WW 3200 - 3700K)")},
    {256, N_("One Touch White Balance")},
    {512, N_("Custom 1-4")},
};

constexpr TagInfo OlympusMakerNote::tagInfoRi_[] = {
    {0x0000, "RawInfoVersion", N_("Raw Info Version"), N_("Raw info version"), IfdId::olympusRiId, SectionId::makerTags,
     undefined, -1, printValue},
    {0x0100, "WB_RBLevelsUsed", N_("WB_RB Levels Used"), N_("WB_RB levels used"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0110, "WB_RBLevelsAuto", N_("WB_RB Levels Auto"), N_("WB_RB levels auto"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0120, "WB_RBLevelsShade", N_("WB_RB Levels Shade"), N_("WB_RB levels shade"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0121, "WB_RBLevelsCloudy", N_("WB_RB Levels Cloudy"), N_("WB_RB levels cloudy"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0122, "WB_RBLevelsFineWeather", N_("WB_RB Levels Fine Weather"), N_("WB_RB levels fine weather"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0123, "WB_RBLevelsTungsten", N_("WB_RB Levels Tungsten"), N_("WB_RB levels tungsten"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0124, "WB_RBLevelsEveningSunlight", N_("WB_RB Levels Evening Sunlight"), N_("WB_RB levels evening sunlight"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0130, "WB_RBLevelsDaylightFluor", N_("WB_RB Levels Daylight Fluor"), N_("WB_RB levels daylight fluor"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0131, "WB_RBLevelsDayWhiteFluor", N_("WB_RB Levels Day White Fluor"), N_("WB_RB levels day white fluor"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0132, "WB_RBLevelsCoolWhiteFluor", N_("WB_RB Levels Cool White Fluor"), N_("WB_RB levels cool white fluor"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0133, "WB_RBLevelsWhiteFluorescent", N_("WB_RB Levels White Fluorescent"), N_("WB_RB levels white fluorescent"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0200, "ColorMatrix2", N_("Color Matrix2"), N_("Color matrix 2"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0310, "CoringFilter", N_("Coring Filter"), N_("Coring filter"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0311, "CoringValues", N_("Coring Values"), N_("Coring values"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0600, "BlackLevel2", N_("Black Level 2"), N_("Black level 2"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0601, "YCbCrCoefficients", N_("YCbCr Coefficients"), N_("YCbCr coefficients"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0611, "ValidPixelDepth", N_("Valid Pixel Depth"), N_("Valid pixel depth"), IfdId::olympusRiId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0612, "CropLeft", N_("Crop Left"), N_("Crop left"), IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0613, "CropTop", N_("Crop Top"), N_("Crop top"), IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0614, "CropWidth", N_("Crop Width"), N_("Crop width"), IfdId::olympusRiId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x0615, "CropHeight", N_("Crop Height"), N_("Crop height"), IfdId::olympusRiId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x1000, "LightSource", N_("Light Source"), N_("Light source"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(olympusRiLightSource)},
    {0x1001, "WhiteBalanceComp", N_("White Balance Comp"), N_("White balance comp"), IfdId::olympusRiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x1010, "SaturationSetting", N_("Saturation Setting"), N_("Saturation setting"), IfdId::olympusRiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x1011, "HueSetting", N_("Hue Setting"), N_("Hue setting"), IfdId::olympusRiId, SectionId::makerTags, signedShort,
     -1, printValue},
    {0x1012, "ContrastSetting", N_("Contrast Setting"), N_("Contrast setting"), IfdId::olympusRiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x1013, "SharpnessSetting", N_("Sharpness Setting"), N_("Sharpness setting"), IfdId::olympusRiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x2000, "CMExposureCompensation", N_("CM Exposure Compensation"), N_("CM exposure compensation"),
     IfdId::olympusRiId, SectionId::makerTags, signedRational, -1, printValue},
    {0x2001, "CMWhiteBalance", N_("CM White Balance"), N_("CM white balance"), IfdId::olympusRiId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x2002, "CMWhiteBalanceComp", N_("CM White Balance Comp"), N_("CM white balance comp"), IfdId::olympusRiId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x2010, "CMWhiteBalanceGrayPoint", N_("CM White Balance Gray Point"), N_("CM white balance gray point"),
     IfdId::olympusRiId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x2020, "CMSaturation", N_("CM Saturation"), N_("CM saturation"), IfdId::olympusRiId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x2021, "CMHue", N_("CM Hue"), N_("CM hue"), IfdId::olympusRiId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x2022, "CMContrast", N_("CM Contrast"), N_("CM contrast"), IfdId::olympusRiId, SectionId::makerTags, signedShort,
     -1, printValue},
    {0x2023, "CMSharpness", N_("CM Sharpness"), N_("CM sharpness"), IfdId::olympusRiId, SectionId::makerTags,
     signedShort, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownOlympusRiTag)", "(UnknownOlympusRiTag)", N_("Unknown OlympusRi tag"), IfdId::olympusRiId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* OlympusMakerNote::tagListRi() {
  return tagInfoRi_;
}

// Gradation
std::ostream& OlympusMakerNote::print0x050f(std::ostream& os, const Value& value, const ExifData*) {
  if ((value.count() != 3 && value.count() != 4) || value.typeId() != signedShort) {
    return os << value;
  }

  if (value.toInt64(0) == -1 && value.toInt64(1) == -1 && value.toInt64(2) == 1)
    os << _("Low Key");
  else if (value.toInt64(0) == 0 && value.toInt64(1) == -1 && value.toInt64(2) == 1)
    os << _("Normal");
  else if (value.toInt64(0) == 1 && value.toInt64(1) == -1 && value.toInt64(2) == 1)
    os << _("High Key");
  else
    os << value.toInt64(0) << " " << value.toInt64(1) << " " << value.toInt64(2);

  if (value.count() == 4) {
    switch (value.toInt64(3)) {
      case 0:
        os << ", " << _("User-Selected");
        break;
      case 1:
        os << ", " << _("Auto-Override");
        break;
      default:
        os << value.toInt64(3);
        break;
    }
  }
  return os;
}

// Olympus CameraSettings tag 0x0527 NoiseFilter
std::ostream& OlympusMakerNote::print0x0527(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 3 || value.typeId() != signedShort || value.toInt64(1) != -2 || value.toInt64(2) != 1) {
    return os << value;
  }

  switch (value.toInt64(0)) {
    case -2:
      os << _("Off");
      break;
    case -1:
      os << _("Low");
      break;
    case 0:
      os << _("Standard");
      break;
    case 1:
      os << _("High");
      break;
    default:
      os << value.toInt64(0);
      break;
  }

  return os;
}

std::ostream& OlympusMakerNote::print0x0200(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 3 || value.typeId() != unsignedLong) {
    return os << value;
  }
  const auto l0 = value.toInt64(0);
  switch (l0) {
    case 0:
      os << _("Normal");
      break;
    case 2:
      os << _("Fast");
      break;
    case 3:
      os << _("Panorama");
      break;
    default:
      os << "(" << l0 << ")";
      break;
  }
  if (l0 != 0) {
    os << ", ";
    const auto l1 = value.toInt64(1);
    os << _("Sequence number") << " " << l1;
  }
  if (l0 != 0 && l0 != 2) {
    os << ", ";
    const auto l2 = value.toInt64(2);
    switch (l2) {
      case 1:
        os << _("Left to right");
        break;
      case 2:
        os << _("Right to left");
        break;
      case 3:
        os << _("Bottom to top");
        break;
      case 4:
        os << _("Top to bottom");
        break;
      default:
        os << "(" << l2 << ")";
        break;
    }
  }
  return os;
}  // OlympusMakerNote::print0x0200

std::ostream& OlympusMakerNote::print0x0204(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() == 0 || value.toRational().second == 0) {
    return os << "(" << value << ")";
  }
  float f = value.toFloat();
  if (f == 0.0F || f == 1.0F)
    return os << _("None");
  return os << stringFormat("{:.1f}x", f);
}  // OlympusMakerNote::print0x0204

std::ostream& OlympusMakerNote::print0x1015(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedShort) {
    return os << value;
  }
  if (value.count() == 1) {
    auto l0 = value.toInt64(0);
    if (l0 == 1) {
      os << _("Auto");
    } else {
      return os << value;
    }
  } else if (value.count() == 2) {
    auto l0 = value.toInt64(0);
    auto l1 = value.toInt64(1);
    if (l0 == 1) {
      if (l1 == 0)
        os << _("Auto");
      else
        os << _("Auto") << " (" << l1 << ")";
    } else if (l0 == 2) {
      switch (l1) {
        case 2:
          os << _("3000 Kelvin");
          break;
        case 3:
          os << _("3700 Kelvin");
          break;
        case 4:
          os << _("4000 Kelvin");
          break;
        case 5:
          os << _("4500 Kelvin");
          break;
        case 6:
          os << _("5500 Kelvin");
          break;
        case 7:
          os << _("6500 Kelvin");
          break;
        case 8:
          os << _("7500 Kelvin");
          break;
        default:
          os << value;
          break;
      }
    } else if (l0 == 3) {
      if (l1 == 0)
        os << _("One-touch");
      else
        os << value;
    } else
      return os << value;
  } else {
    return os << value;
  }
  return os;
}  // OlympusMakerNote::print0x1015

//! OlympusEq LensType, tag 0x201
std::ostream& OlympusMakerNote::print0x0201(std::ostream& os, const Value& value, const ExifData*) {
  // #1034
  const std::string undefined("undefined");
  const std::string section("olympus");
  if (Internal::readExiv2Config(section, value.toString(), undefined) != undefined) {
    return os << Internal::readExiv2Config(section, value.toString(), undefined);
  }

  // 6 numbers: 0. Make, 1. Unknown, 2. Model, 3. Sub-model, 4-5. Unknown.
  // Only the Make, Model and Sub-model are used to determine the lens model
  static constexpr struct {
    byte val[3];
    const char* label;
  } lensTypes[] = {
      {{0, 0, 0}, N_("None")},
      {{0, 1, 0}, "Olympus Zuiko Digital ED 50mm F2.0 Macro"},
      {{0, 1, 1}, "Olympus Zuiko Digital 40-150mm F3.5-4.5"},
      {{0, 1, 16}, "Olympus M.Zuiko Digital ED 14-42mm F3.5-5.6"},
      {{0, 2, 0}, "Olympus Zuiko Digital ED 150mm F2.0"},
      {{0, 2, 16}, "Olympus M.Zuiko Digital 17mm F2.8 Pancake"},
      {{0, 3, 0}, "Olympus Zuiko Digital ED 300mm F2.8"},
      {{0, 3, 16}, "Olympus M.Zuiko Digital ED 14-150mm F4.0-5.6"},
      {{0, 4, 16}, "Olympus M.Zuiko Digital ED 9-18mm F4.0-5.6"},
      {{0, 5, 0}, "Olympus Zuiko Digital 14-54mm F2.8-3.5"},
      {{0, 5, 1}, "Olympus Zuiko Digital Pro ED 90-250mm F2.8"},
      {{0, 5, 16}, "Olympus M.Zuiko Digital ED 14-42mm F3.5-5.6 L"},
      {{0, 6, 0}, "Olympus Zuiko Digital ED 50-200mm F2.8-3.5"},
      {{0, 6, 1}, "Olympus Zuiko Digital ED 8mm F3.5 Fisheye"},
      {{0, 6, 16}, "Olympus M.Zuiko Digital ED 40-150mm F4.0-5.6"},
      {{0, 7, 0}, "Olympus Zuiko Digital 11-22mm F2.8-3.5"},
      {{0, 7, 1}, "Olympus Zuiko Digital 18-180mm F3.5-6.3"},
      {{0, 7, 16}, "Olympus M.Zuiko Digital ED 12mm F2.0"},
      {{0, 8, 1}, "Olympus Zuiko Digital 70-300mm F4.0-5.6"},
      {{0, 8, 16}, "Olympus M.Zuiko Digital ED 75-300mm F4.8-6.7"},
      {{0, 9, 16}, "Olympus M.Zuiko Digital 14-42mm F3.5-5.6 II"},
      {{0, 16, 1}, "Kenko Tokina Reflex 300mm F6.3 MF Macro"},
      {{0, 16, 16}, "Olympus M.Zuiko Digital ED 12-50mm F3.5-6.3 EZ"},
      {{0, 17, 16}, "Olympus M.Zuiko Digital 45mm F1.8"},
      {{0, 18, 16}, "Olympus M.Zuiko Digital ED 60mm F2.8 Macro"},
      {{0, 19, 16}, "Olympus M.Zuiko Digital 14-42mm F3.5-5.6 II R"},
      {{0, 20, 16}, "Olympus M.Zuiko Digital ED 40-150mm F4.0-5.6 R"},
      {{0, 21, 0}, "Olympus Zuiko Digital ED 7-14mm F4.0"},
      {{0, 21, 16}, "Olympus M.Zuiko Digital ED 75mm F1.8"},
      {{0, 22, 16}, "Olympus M.Zuiko Digital 17mm F1.8"},
      {{0, 23, 0}, "Olympus Zuiko Digital Pro ED 35-100mm F2.0"},
      {{0, 24, 0}, "Olympus Zuiko Digital 14-45mm F3.5-5.6"},
      {{0, 24, 16}, "Olympus M.Zuiko Digital ED 75-300mm F4.8-6.7 II"},
      {{0, 25, 16}, "Olympus M.Zuiko Digital ED 12-40mm F2.8 Pro"},
      {{0, 32, 0}, "Olympus Zuiko Digital 35mm F3.5 Macro"},
      {{0, 32, 16}, "Olympus M.Zuiko Digital ED 40-150mm F2.8 Pro"},
      {{0, 33, 16}, "Olympus M.Zuiko Digital ED 14-42mm F3.5-5.6 EZ"},
      {{0, 34, 0}, "Olympus Zuiko Digital 17.5-45mm F3.5-5.6"},
      {{0, 34, 16}, "Olympus M.Zuiko Digital 25mm F1.8"},
      {{0, 35, 0}, "Olympus Zuiko Digital ED 14-42mm F3.5-5.6"},
      {{0, 35, 16}, "Olympus M.Zuiko Digital ED 7-14mm F2.8 Pro"},
      {{0, 36, 0}, "Olympus Zuiko Digital ED 40-150mm F4.0-5.6"},
      {{0, 36, 16}, "Olympus M.Zuiko Digital ED 300mm F4.0 IS Pro"},
      {{0, 37, 16}, "Olympus M.Zuiko Digital ED 8mm F1.8 Fisheye Pro"},
      {{0, 38, 16}, "Olympus M.Zuiko Digital ED 12-100mm F4.0 IS Pro"},
      {{0, 39, 16}, "Olympus M.Zuiko Digital ED 30mm F3.5 Macro"},
      {{0, 40, 16}, "Olympus M.Zuiko Digital ED 25mm F1.2 Pro"},
      {{0, 41, 16}, "Olympus M.Zuiko Digital ED 17mm F1.2 Pro"},
      {{0, 48, 0}, "Olympus Zuiko Digital ED 50-200mm F2.8-3.5 SWD"},
      {{0, 49, 0}, "Olympus Zuiko Digital ED 12-60mm F2.8-4.0 SWD"},
      {{0, 50, 0}, "Olympus Zuiko Digital ED 14-35mm F2.0 SWD"},
      {{0, 51, 0}, "Olympus Zuiko Digital 25mm F2.8"},
      {{0, 52, 0}, "Olympus Zuiko Digital ED 9-18mm F4.0-5.6"},
      {{0, 52, 16}, "Olympus M.Zuiko Digital ED 12-45mm F4.0 Pro"},
      {{0, 53, 0}, "Olympus Zuiko Digital 14-54mm F2.8-3.5 II"},
      {{1, 1, 0}, "Sigma 18-50mm F3.5-5.6 DC"},
      {{1, 1, 16}, "Sigma 30mm F2.8 EX DN"},
      {{1, 2, 0}, "Sigma 55-200mm F4.0-5.6 DC"},
      {{1, 2, 16}, "Sigma 19mm F2.8 EX DN"},
      {{1, 3, 0}, "Sigma 18-125mm F3.5-5.6 DC"},
      {{1, 3, 16}, "Sigma 30mm F2.8 DN | A"},
      {{1, 4, 0}, "Sigma 18-125mm F3.5-5.6"},
      {{1, 4, 16}, "Sigma 19mm F2.8 DN | A"},
      {{1, 5, 0}, "Sigma 30mm F1.4"},
      {{1, 5, 16}, "Sigma 60mm F2.8 DN | A"},
      {{1, 6, 0}, "Sigma 50-500mm F4.0-6.3 EX DG APO HSM RF"},
      {{1, 6, 16}, "Sigma 30mm F1.4 DC DN | C"},
      {{1, 7, 0}, "Sigma 105mm F2.8 DG"},
      {{1, 8, 0}, "Sigma 150mm F2.8 DG HSM"},
      {{1, 9, 0}, "Sigma 18-50mm F2.8 EX DC Macro"},
      {{1, 16, 0}, "Sigma 24mm F1.8 EX DG Aspherical Macro"},
      {{1, 17, 0}, "Sigma 135-400mm F4.5-5.6 DG ASP APO RF"},
      {{1, 18, 0}, "Sigma 300-800mm F5.6 EX DG APO"},
      {{1, 19, 0}, "Sigma 30mm F1.4 EX DC HSM"},
      {{1, 20, 0}, "Sigma 50-500mm F4.0-6.3 EX DG APO HSM RF"},
      {{1, 21, 0}, "Sigma 10-20mm F4.0-5.6 EX DC HSM"},
      {{1, 22, 0}, "Sigma 70-200mm F2.8 EX DG Macro HSM II"},
      {{1, 23, 0}, "Sigma 50mm F1.4 EX DG HSM"},
      {{2, 1, 0}, "Leica D Vario Elmarit 14-50mm F2.8-3.5 Asph."},
      {{2, 1, 16}, "Lumix G Vario 14-45mm F3.5-5.6 Asph. Mega OIS"},
      {{2, 2, 0}, "Leica D Summilux 25mm F1.4 Asph."},
      {{2, 2, 16}, "Lumix G Vario 45-200mm F4.0-5.6 Mega OIS"},
      {{2, 3, 0}, "Leica D Vario Elmar 14-50mm F3.8-5.6 Asph. Mega OIS"},
      {{2, 3, 1}, "Leica D Vario Elmar 14-50mm F3.8-5.6 Asph."},
      {{2, 3, 16}, "Lumix G Vario HD 14-140mm F4.0-5.8 Asph. Mega OIS"},
      {{2, 4, 0}, "Leica D Vario Elmar 14-150mm F3.5-5.6"},
      {{2, 4, 16}, "Lumix G Vario 7-14mm F4.0 Asph."},
      {{2, 5, 16}, "Lumix G 20mm F1.7 Asph."},
      {{2, 6, 16}, "Leica DG Macro-Elmarit 45mm F2.8 Asph. Mega OIS"},
      {{2, 7, 16}, "Lumix G Vario 14-42mm F3.5-5.6 Asph. Mega OIS"},
      {{2, 8, 16}, "Lumix G Fisheye 8mm F3.5"},
      {{2, 9, 16}, "Lumix G Vario 100-300mm F4.0-5.6 Mega OIS"},
      {{2, 16, 16}, "Lumix G 14mm F2.5 Asph."},
      {{2, 17, 16}, "Lumix G 3D 12.5mm F12"},
      {{2, 18, 16}, "Leica DG Summilux 25mm F1.4 Asph."},
      {{2, 19, 16}, "Lumix G X Vario PZ 45-175mm F4.0-5.6 Asph. Power OIS"},
      {{2, 20, 16}, "Lumix G X Vario PZ 14-42mm F3.5-5.6 Asph. Power OIS"},
      {{2, 21, 16}, "Lumix G X Vario 12-35mm F2.8 Asph. Power OIS"},
      {{2, 22, 16}, "Lumix G Vario 45-150mm F4.0-5.6 Asph. Mega OIS"},
      {{2, 23, 16}, "Lumix G X Vario 35-100mm F2.8 Power OIS"},
      {{2, 24, 16}, "Lumix G Vario 14-42mm F3.5-5.6 II Asph. Mega OIS"},
      {{2, 25, 16}, "Lumix G Vario 14-140mm F3.5-5.6 Asph. Power OIS"},
      {{2, 32, 16}, "Lumix G Vario 12-32mm F3.5-5.6 Asph. Mega OIS"},
      {{2, 33, 16}, "Leica DG Nocticron 42.5mm F1.2 Asph. Power OIS"},
      {{2, 34, 16}, "Leica DG Summilux 15mm F1.7 Asph."},
      {{2, 35, 16}, "Lumix G Vario 35-100mm F4.0-5.6 Asph. Mega OIS"},
      {{2, 36, 16}, "Lumix G Macro 30mm F2.8 Asph. Mega OIS"},
      {{2, 37, 16}, "Lumix G 42.5mm F1.7 Asph. Power OIS"},
      {{2, 38, 16}, "Lumix G 25mm F1.7 Asph."},
      {{2, 39, 16}, "Leica DG Vario-Elmar 100-400mm F4.0-6.3 Asph. Power OIS"},
      {{2, 40, 16}, "Lumix G Vario 12-60mm F3.5-5.6 Asph. Power OIS"},
      {{3, 1, 0}, "Leica D Vario Elmarit 14-50mm F2.8-3.5 Asph."},
      {{3, 2, 0}, "Leica D Summilux 25mm F1.4 Asph."},
      {{5, 1, 16}, "Tamron 14-150mm F3.5-5.8 Di III"},
  };

  if (value.count() != 6 || value.typeId() != unsignedByte) {
    return os << value;
  }

  auto v0 = static_cast<byte>(value.toInt64(0));
  auto v2 = static_cast<byte>(value.toInt64(2));
  auto v3 = static_cast<byte>(value.toInt64(3));

  for (const auto& type : lensTypes) {
    if (type.val[0] == v0 && type.val[1] == v2 && type.val[2] == v3) {
      return os << type.label;
    }
  }
  return os << value;
}  // OlympusMakerNote::print0x0201

// Olympus tag 0x0209 CameraID
std::ostream& OlympusMakerNote::print0x0209(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != asciiString && value.typeId() != undefined) {
    return os << value;
  }

  char ch;
  size_t size = value.size();
  for (size_t i = 0; i < size && ((ch = static_cast<char>(value.toInt64(i))) != '\0'); i++) {
    os << ch;
  }
  return os;
}  // OlympusMakerNote::print0x0209

//! OlympusEq Extender, tag 0x0301
std::ostream& OlympusMakerNote::printEq0x0301(std::ostream& os, const Value& value, const ExifData*) {
  // 6 numbers: 0. Make, 1. Unknown, 2. Model, 3. Sub-model, 4-5. Unknown.
  // Only the Make and Model are used to determine the extender model
  static constexpr struct {
    byte val[2];
    const char* label;
  } extenderModels[] = {
      {{0, 0}, N_("None")},
      {{0, 4}, "Olympus Zuiko Digital EC-14 1.4x Teleconverter"},
      {{0, 8}, "Olympus EX-25 Extension Tube"},
      {{0, 16}, "Olympus Zuiko Digital EC-20 2.0x Teleconverter"},
  };

  if (value.count() != 6 || value.typeId() != unsignedByte) {
    return os << value;
  }

  auto v0 = static_cast<byte>(value.toInt64(0));
  auto v2 = static_cast<byte>(value.toInt64(2));

  for (const auto& model : extenderModels) {
    if (model.val[0] == v0 && model.val[1] == v2) {
      return os << model.label;
    }
  }
  return os << value;
}  // OlympusMakerNote::printEq0x0301

//! OlympusCs FocusMode, tag 0x0301
// (1 or 2 values)
std::ostream& OlympusMakerNote::printCs0x0301(std::ostream& os, const Value& value, const ExifData*) {
  using mode = std::pair<uint16_t, const char*>;
  static constexpr mode focusModes0[] = {
      {0, N_("Single AF")},     {1, N_("Sequential shooting AF")},
      {2, N_("Continuous AF")}, {3, N_("Multi AF")},
      {4, N_("Face detect")},   {10, N_("MF")},
  };

  static constexpr mode focusModes1[] = {
      {0x0001, N_("S-AF")},        {0x0004, N_("C-AF")},      {0x0010, N_("MF")},
      {0x0020, N_("Face detect")}, {0x0040, N_("Imager AF")}, {0x0100, N_("AF sensor")},
  };

  if (value.count() < 1 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }
  auto v = static_cast<uint16_t>(value.toInt64(0));

  // If value 2 is present, it is used instead of value 1.
  if (value.count() > 1) {
    std::string p;  // Used to enable ',' separation

    v = static_cast<uint16_t>(value.toInt64(1));
    for (auto&& [val, label] : focusModes1) {
      if ((v & val) != 0) {
        if (!p.empty()) {
          os << ", ";
        }
        p = label;
        os << p;
      }
    }
  } else {
    for (auto&& [val, label] : focusModes0) {
      if (val == v) {
        os << label;
        break;
      }
    }
  }
  return os << v;

}  // OlympusMakerNote::printCs0x0301

//! OlympusCs ArtFilter, tag 0x0529, OlympusCs MagicFilter, tag 0x052c
std::ostream& OlympusMakerNote::print0x0529(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 4 || value.typeId() != unsignedShort) {
    return os << "(" << value << ")";
  }

  const auto v0 = value.toInt64(0);

  printTag<std::size(artFilters), artFilters>(os, v0, metadata);

  if (v0 == 39) {  // The "Partial color" option also has a color choice
    const auto v3 = value.toInt64(3);
    os << " (" << _("position") << " " << (v3 + 1) << ")";
    return os;
  }

  return os;
}  // OlympusMakerNote::print0x0529

// Olympus FocusInfo tag 0x1209 ManualFlash
std::ostream& OlympusMakerNote::print0x1209(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 2 || value.typeId() != unsignedShort) {
    return os << value;
  }

  switch (value.toInt64(0)) {
    case 0:
      os << _("Off");
      break;
    case 1:
      os << _("On");
      break;
    default:
      os << value.toInt64(0);
      break;
  }
  os << " ";
  os << value.toInt64(1);

  return os;
}  // OlympusMakerNote::print0x1209

// Olympus FocusDistance 0x0305
std::ostream& OlympusMakerNote::print0x0305(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedRational) {
    return os << value;
  }

  auto [r, s] = value.toRational();
  if (static_cast<uint32_t>(r) == 0xffffffff) {
    return os << _("Infinity");
  }
  return os << stringFormat("{:.2f} m", static_cast<float>(r) / 1000);
}

// Olympus FocusInfo tag 0x0308 AFPoint
std::ostream& OlympusMakerNote::print0x0308(std::ostream& os, const Value& value, const ExifData* metadata) {
  static constexpr std::pair<uint16_t, const char*> afPoints[] = {
      {0, N_("Left (or n/a)")}, {1, N_("Center (horizontal)")}, {2, N_("Right")}, {3, N_("Center (vertical)")},
      {255, N_("None")},
  };

  static constexpr std::pair<byte, const char*> afPointsE3[] = {
      {0x00, N_("None")},
      {0x01, N_("Top-left (horizontal)")},
      {0x02, N_("Top-center (horizontal)")},
      {0x03, N_("Top-right (horizontal)")},
      {0x04, N_("Left (horizontal)")},
      {0x05, N_("Mid-left (horizontal)")},
      {0x06, N_("Center (horizontal)")},
      {0x07, N_("Mid-right (horizontal)")},
      {0x08, N_("Right (horizontal)")},
      {0x09, N_("Bottom-left (horizontal)")},
      {0x0a, N_("Bottom-center (horizontal)")},
      {0x0b, N_("Bottom-right (horizontal)")},
      {0x0c, N_("Top-left (vertical)")},
      {0x0d, N_("Top-center (vertical)")},
      {0x0e, N_("Top-right (vertical)")},
      {0x0f, N_("Left (vertical)")},
      {0x10, N_("Mid-left (vertical)")},
      {0x11, N_("Center (vertical)")},
      {0x12, N_("Mid-right (vertical)")},
      {0x13, N_("Right (vertical)")},
      {0x14, N_("Bottom-left (vertical)")},
      {0x15, N_("Bottom-center (vertical)")},
      {0x16, N_("Bottom-right (vertical)")},
  };

  if (value.count() != 1 || value.typeId() != unsignedShort) {
    return os << value;
  }

  bool E3_E30model = false;

  if (metadata) {
    auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
    if (pos != metadata->end() && pos->count() != 0) {
      std::string model = pos->toString();
      if (Internal::contains(model, "E-3 ") || Internal::contains(model, "E-30 ")) {
        E3_E30model = true;
      }
    }
  }

  auto v = static_cast<uint16_t>(value.toInt64(0));

  if (!E3_E30model) {
    for (auto&& [val, label] : afPoints) {
      if (val == v) {
        return os << label;
      }
    }
  } else {
    // E-3 and E-30
    for (auto&& [val, label] : afPointsE3) {
      if (val == (v & 0x1f)) {
        os << label;
        os << ", ";
        if ((v & 0xe0) == 0)
          return os << N_("Single Target");
        if (v & 0x40)
          return os << N_("All Target");
        if (v & 0x80)
          return os << N_("Dynamic Single Target");
      }
    }
  }
  return os << v;
}  // OlympusMakerNote::print0x0308

}  // namespace Exiv2::Internal
