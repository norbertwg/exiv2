// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#include "pentaxmn_int.hpp"

// included header files
#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "makernote_int.hpp"
#include "tags.hpp"
#include "types.hpp"
#include "value.hpp"

#include <iomanip>

namespace {
// Exception thrown by findLensInfo when the lens info can't be found.
class LensInfoNotFound : public std::exception {
  using std::exception::exception;
};
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! ShootingMode, tag 0x0001
constexpr TagDetails pentaxShootingMode[] = {
    {0, N_("Auto")},
    {1, N_("Night-Scene")},
    {2, N_("Manual")},
};

//! CameraModel, tag 0x0005
constexpr TagDetails pentaxModel[] = {
    {0x0000d, "Optio 330/430"},
    {0x12926, "Optio 230"},
    {0x12958, "Optio 330GS"},
    {0x12962, "Optio 450/550"},
    {0x1296c, "Optio S"},
    {0x12971, "Optio S V1.01"},
    {0x12994, "*ist D"},
    {0x129b2, "Optio 33L"},
    {0x129bc, "Optio 33LF"},
    {0x129c6, "Optio 33WR/43WR/555"},
    {0x129d5, "Optio S4"},
    {0x12a02, "Optio MX"},
    {0x12a0c, "Optio S40"},
    {0x12a16, "Optio S4i"},
    {0x12a34, "Optio 30"},
    {0x12a52, "Optio S30"},
    {0x12a66, "Optio 750Z"},
    {0x12a70, "Optio SV"},
    {0x12a75, "Optio SVi"},
    {0x12a7a, "Optio X"},
    {0x12a8e, "Optio S5i"},
    {0x12a98, "Optio S50"},
    {0x12aa2, "*ist DS"},
    {0x12ab6, "Optio MX4"},
    {0x12ac0, "Optio S5n"},
    {0x12aca, "Optio WP"},
    {0x12afc, "Optio S55"},
    {0x12b10, "Optio S5z"},
    {0x12b1a, "*ist DL"},
    {0x12b24, "Optio S60"},
    {0x12b2e, "Optio S45"},
    {0x12b38, "Optio S6"},
    {0x12b4c, "Optio WPi"},
    {0x12b56, "BenQ DC X600"},
    {0x12b60, "*ist DS2"},
    {0x12b62, "Samsung GX-1S"},
    {0x12b6a, "Optio A10"},
    {0x12b7e, "*ist DL2"},
    {0x12b80, "Samsung GX-1L"},
    {0x12b9c, "K100D"},
    {0x12b9d, "K110D"},
    {0x12ba2, "K100D Super"},
    {0x12bb0, "Optio T10/T20"},
    {0x12be2, "Optio W10"},
    {0x12bf6, "Optio M10"},
    {0x12c1e, "K10D"},
    {0x12c20, "Samsung GX10"},
    {0x12c28, "Optio S7"},
    {0x12c2d, "Optio L20"},
    {0x12c32, "Optio M20"},
    {0x12c3c, "Optio W20"},
    {0x12c46, "Optio A20"},
    {0x12c78, "Optio E30"},
    {0x12c7d, "Optio E35"},
    {0x12c82, "Optio T30"},
    {0x12c8c, "Optio M30"},
    {0x12c91, "Optio L30"},
    {0x12c96, "Optio W30"},
    {0x12ca0, "Optio A30"},
    {0x12cb4, "Optio E40"},
    {0x12cbe, "Optio M40"},
    {0x12cc3, "Optio L40"},
    {0x12cc5, "Optio L36"},
    {0x12cc8, "Optio Z10"},
    {0x12cd2, "K20D"},
    {0x12cd4, "Samsung GX20"},
    {0x12cdc, "Optio S10"},
    {0x12ce6, "Optio A40"},
    {0x12cf0, "Optio V10"},
    {0x12cfa, "K200D"},
    {0x12d04, "Optio S12"},
    {0x12d0e, "Optio E50"},
    {0x12d18, "Optio M50"},
    {0x12d22, "Optio L50"},
    {0x12d2c, "Optio V20"},
    {0x12d40, "Optio W60"},
    {0x12d4a, "Optio M60"},
    {0x12d68, "Optio E60/M90"},
    {0x12d72, "K2000"},
    {0x12d73, "K-m"},
    {0x12d86, "Optio P70"},
    {0x12d90, "Optio L70"},
    {0x12d9a, "Optio E70"},
    {0x12dae, "X70"},
    {0x12db8, "K-7"},
    {0x12dcc, "Optio W80"},
    {0x12dea, "Optio P80"},
    {0x12df4, "Optio WS80"},
    {0x12dfe, "K-x"},
    {0x12e08, "645D"},
    {0x12e12, "Optio E80"},
    {0x12e30, "Optio W90"},
    {0x12e3a, "Optio I-10"},
    {0x12e44, "Optio H90"},
    {0x12e4e, "Optio E90"},
    {0x12e58, "X90"},
    {0x12e6c, "K-r"},
    {0x12e76, "K-5"},
    {0x12e8a, "Optio RS1000/RS1500"},
    {0x12e94, "Optio RZ10"},
    {0x12e9e, "Optio LS1000"},
    {0x12ebc, "Optio WG-1 GPS"},
    {0x12ed0, "Optio S1"},
    {0x12ee4, "Q"},
    {0x12ef8, "K-01"},
    {0x12f0c, "Optio RZ18"},
    {0x12f16, "Optio VS20"},
    {0x12f2a, "Optio WG-2 GPS"},
    {0x12f48, "Optio LS465"},
    {0x12f52, "K-30"},
    {0x12f5c, "X-5"},
    {0x12f66, "Q10"},
    {0x12f70, "K-5 II"},
    {0x12f71, "K-5 II s"},
    {0x12f7a, "Q7"},
    {0x12f84, "MX-1"},
    {0x12f8e, "WG-3 GPS"},
    {0x12f98, "WG-3"},
    {0x12fa2, "WG-10"},
    {0x12fb6, "K-50"},
    {0x12fc0, "K-3"},
    {0x12fca, "K-500"},
    {0x12fde, "WG-4 GPS"},
    {0x12fe8, "WG-4"},
    {0x13006, "WG-20"},
    {0x13010, "645Z"},
    {0x1301a, "K-S1"},
    {0x13024, "K-S2"},
    {0x1302e, "Q-S1"},
    {0x13056, "WG-30"},
    {0x1307e, "WG-30W"},
    {0x13088, "WG-5 GPS"},
    {0x13092, "K-1"},
    {0x1309c, "K-3 II"},
    {0x131f0, "WG-M2"},
    {0x1320e, "GR III"},
    {0x13222, "K-70"},
    {0x1322c, "KP"},
    {0x13240, "K-1 Mark II"},
    {0x13254, "K-3 Mark III"},
    {0x13290, "WG-70"},
    {0x1329a, "GR IIIx"},
    {0x132b8, "KF"},
    {0x132d6, "K-3 Mark III Monochrome"},
};

//! Quality, tag 0x0008
constexpr TagDetails pentaxQuality[] = {
    {0, N_("Good")}, {1, N_("Better")},  {2, N_("Best")},    {3, N_("TIFF")},
    {4, N_("RAW")},  {5, N_("Premium")}, {65535, N_("n/a")},
};

//! Size, tag 0x0009
constexpr TagDetails pentaxSize[] = {
    {0, "640x480"},
    {1, N_("Full")},
    {2, "1024x768"},
    {3, "1280x960"},
    {4, "1600x1200"},
    {5, "2048x1536"},
    {8, N_("2560x1920 or 2304x1728")},
    {9, "3072x2304"},
    {10, "3264x2448"},
    {19, "320x240"},
    {20, "2288x1712"},
    {21, "2592x1944"},
    {22, N_("2304x1728 or 2592x1944")},
    {23, "3056x2296"},
    {25, N_("2816x2212 or 2816x2112")},
    {27, "3648x2736"},
    {29, "4000x3000"},
    {30, "4288x3216"},
    {31, "4608x3456"},
    {129, "1920x1080"},
    {135, "4608x2592"},
    {257, "3216x3216"},
    // not sure what to do with these values:
    //    '0 0' = 2304x1728
    //    '4 0' = 1600x1200
    //    '5 0' = 2048x1536
    //    '8 0' = 2560x1920
    //    '32 2' = 960x640
    //    '33 2' = 1152x768
    //    '34 2' = 1536x1024
    //    '35 1' = 2400x1600
    //    '36 0' = 3008x2008 or 3040x2024
    //    '37 0' = 3008x2000
};

//! Flash, tag 0x000c
constexpr TagDetails pentaxFlash[] = {
    {0x000, N_("Auto, Did not fire")},
    {0x001, N_("Off, Did not fire")},
    {0x002, N_("Off, Did not fire")},
    {0x003, N_("Auto, Did not fire, Red-eye reduction")},
    {0x005, N_("On. Did not fire. Wireless (Master)")},
    {0x100, N_("Auto, Fired")},
    {0x102, N_("On, Fired")},
    {0x103, N_("Auto, Fired, Red-eye reduction")},
    {0x104, N_("On, Red-eye reduction")},
    {0x105, N_("On, Wireless (Master)")},
    {0x106, N_("On, Wireless (Control)")},
    {0x108, N_("On, Soft")},
    {0x109, N_("On, Slow-sync")},
    {0x10a, N_("On, Slow-sync, Red-eye reduction")},
    {0x10b, N_("On, Trailing-curtain Sync")},
    // exiftool recognises 2 values, the values here correspond with Value 0
};

//! Focus, tag 0x000d
constexpr TagDetails pentaxFocus[] = {
    {0, N_("Normal")},
    {1, N_("Macro")},
    {2, N_("Infinity")},
    {3, N_("Manual")},
    {4, N_("Super Macro")},
    {5, N_("Pan Focus")},
    {16, N_("AF-S")},
    {17, N_("AF-C")},
    {18, N_("AF-A")},
    {32, N_("Contrast-detect")},
    {33, N_("Tracking Contrast-detect")},
    {288, N_("Face Detect")},
};

//! AFPoint, tag 0x000e
constexpr TagDetails pentaxAFPoint[] = {
    {0xffff, N_("Auto")},
    {0xfffe, N_("Fixed Center")},
    {0xfffd, N_("Automatic Tracking AF")},
    {0xfffc, N_("Face Recognition AF")},
    {0xfffb, N_("AF Select")},
    {0, N_("None")},
    {1, N_("Upper-left")},
    {2, N_("Top")},
    {3, N_("Upper-right")},
    {4, N_("Left")},
    {5, N_("Mid-left")},
    {6, N_("Center")},
    {7, N_("Mid-right")},
    {8, N_("Right")},
    {9, N_("Lower-left")},
    {10, N_("Bottom")},
    {11, N_("Lower-right")},
};

//! AFPointInFocus, tag 0x000f
constexpr TagDetails pentaxAFPointFocus[] = {
    {0xffff, N_("None")},    {0, N_("Fixed Center or multiple")},
    {1, N_("Top-left")},     {2, N_("Top-center")},
    {3, N_("Top-right")},    {4, N_("Left")},
    {5, N_("Center")},       {6, N_("Right")},
    {7, N_("Bottom-left")},  {8, N_("Bottom-center")},
    {9, N_("Bottom-right")},
};

//! ISO, tag 0x0014
constexpr TagDetails pentaxISO[] = {
    {3, "50"},
    {4, "64"},
    {5, "80"},
    {6, "100"},
    {7, "125"},
    {8, "160"},
    {9, "200"},
    {10, "250"},
    {11, "320"},
    {12, "400"},
    {13, "500"},
    {14, "640"},
    {15, "800"},
    {16, "1000"},
    {17, "1250"},
    {18, "1600"},
    {19, "2000"},
    {20, "2500"},
    {21, "3200"},
    {22, "4000"},
    {23, "5000"},
    {24, "6400"},
    {25, "8000"},
    {26, "10000"},
    {27, "12800"},
    {28, "16000"},
    {29, "20000"},
    {30, "25600"},
    {31, "32000"},
    {32, "40000"},
    {33, "51200"},
    {34, "64000"},
    {35, "80000"},
    {36, "102400"},
    {37, "128000"},
    {38, "160000"},
    {39, "204800"},
    {40, "256000"},
    {41, "320000"},
    {42, "409600"},
    {43, "512000"},
    {44, "640000"},
    {45, "819200"},
    {50, "50"},
    {100, "100"},
    {200, "200"},
    //        {    268, "200" },
    {400, "400"},
    {800, "800"},
    {1600, "1600"},
    {3200, "3200"},
    {258, "50"},
    {259, "70"},
    {260, "100"},
    {261, "140"},
    {262, "200"},
    {263, "280"},
    {264, "400"},
    {265, "560"},
    {266, "800"},
    {267, "1100"},
    {268, "1600"},
    {269, "2200"},
    {270, "3200"},
    {271, "4500"},
    {272, "6400"},
    {273, "9000"},
    {274, "12800"},
    {275, "18000"},
    {276, "25600"},
    {277, "36000"},
    {278, "51200"},
    {279, "72000"},
    {280, "102400"},
    {281, "144000"},
    {282, "204800"},
    {283, "288000"},
    {284, "409600"},
    {285, "576000"},
    {286, "819200"},
};

//! Generic for Off/On switches
constexpr TagDetails pentaxOffOn[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

//! Generic for Yes/No switches
constexpr TagDetails pentaxYesNo[] = {
    {0, N_("No")},
    {1, N_("Yes")},
};

//! MeteringMode, tag 0x0017
constexpr TagDetails pentaxMeteringMode[] = {
    {0, N_("Multi Segment")},
    {1, N_("Center Weighted")},
    {2, N_("Spot")},
};

//! WhiteBalance, tag 0x0019
constexpr TagDetails pentaxWhiteBalance[] = {
    {0, N_("Auto")},
    {1, N_("Daylight")},
    {2, N_("Shade")},
    {3, N_("Fluorescent")},
    {4, N_("Tungsten")},
    {5, N_("Manual")},
    {6, N_("DaylightFluorescent")},
    {7, N_("DaywhiteFluorescent")},
    {8, N_("WhiteFluorescent")},
    {9, N_("Flash")},
    {10, N_("Cloudy")},
    {15, N_("Color Temperature Enhancement")},
    {17, N_("Kelvin")},
    {65534, N_("Unknown")},
    {65535, N_("User Selected")},
};

//! WhiteBalance, tag 0x001a
constexpr TagDetails pentaxWhiteBalanceMode[] = {
    {1, N_("Auto (Daylight)")},
    {2, N_("Auto (Shade)")},
    {3, N_("Auto (Flash)")},
    {4, N_("Auto (Tungsten)")},
    {6, N_("Auto (DaylightFluorescent)")},
    {7, N_("Auto (DaywhiteFluorescent)")},
    {8, N_("Auto (WhiteFluorescent)")},
    {10, N_("Auto (Cloudy)")},
    {0xffff, N_("User-Selected")},
    {0xfffe, N_("Preset (Fireworks?)")},
};

//! Saturation, tag 0x001f
constexpr TagDetails pentaxSaturation[] = {
    {0, N_("Low")},      {1, N_("Normal")},    {2, N_("High")}, {3, N_("Med Low")}, {4, N_("Med High")},
    {5, N_("Very Low")}, {6, N_("Very High")}, {7, N_("-4")},   {8, N_("+4")},      {65535, N_("None")},
};

//! Contrast, tag 0x0020
constexpr TagDetails pentaxContrast[] = {
    {0, N_("Low")},      {1, N_("Normal")},    {2, N_("High")}, {3, N_("Med Low")}, {4, N_("Med High")},
    {5, N_("Very Low")}, {6, N_("Very High")}, {7, N_("-4")},   {8, N_("+4")},
};

//! Sharpness, tag 0x0021
constexpr TagDetails pentaxSharpness[] = {
    {0, N_("Soft")},      {1, N_("Normal")},    {2, N_("Hard")}, {3, N_("Med Soft")}, {4, N_("Med Hard")},
    {5, N_("Very Soft")}, {6, N_("Very Hard")}, {7, N_("-4")},   {8, N_("+4")},
};

//! Location, tag 0x0022
constexpr TagDetails pentaxLocation[] = {
    {0, N_("Home town")},
    {1, N_("Destination")},
};

//! City names, tags 0x0023 and 0x0024
constexpr TagDetails pentaxCities[] = {
    {0, N_("Pago Pago")},     {1, N_("Honolulu")},      {2, N_("Anchorage")},       {3, N_("Vancouver")},
    {4, N_("San Fransisco")}, {5, N_("Los Angeles")},   {6, N_("Calgary")},         {7, N_("Denver")},
    {8, N_("Mexico City")},   {9, N_("Chicago")},       {10, N_("Miami")},          {11, N_("Toronto")},
    {12, N_("New York")},     {13, N_("Santiago")},     {14, N_("Caracus")},        {15, N_("Halifax")},
    {16, N_("Buenos Aires")}, {17, N_("Sao Paulo")},    {18, N_("Rio de Janeiro")}, {19, N_("Madrid")},
    {20, N_("London")},       {21, N_("Paris")},        {22, N_("Milan")},          {23, N_("Rome")},
    {24, N_("Berlin")},       {25, N_("Johannesburg")}, {26, N_("Istanbul")},       {27, N_("Cairo")},
    {28, N_("Jerusalem")},    {29, N_("Moscow")},       {30, N_("Jeddah")},         {31, N_("Tehran")},
    {32, N_("Dubai")},        {33, N_("Karachi")},      {34, N_("Kabul")},          {35, N_("Male")},
    {36, N_("Delhi")},        {37, N_("Colombo")},      {38, N_("Kathmandu")},      {39, N_("Dacca")},
    {40, N_("Yangon")},       {41, N_("Bangkok")},      {42, N_("Kuala Lumpur")},   {43, N_("Vientiane")},
    {44, N_("Singapore")},    {45, N_("Phnom Penh")},   {46, N_("Ho Chi Minh")},    {47, N_("Jakarta")},
    {48, N_("Hong Kong")},    {49, N_("Perth")},        {50, N_("Beijing")},        {51, N_("Shanghai")},
    {52, N_("Manila")},       {53, N_("Taipei")},       {54, N_("Seoul")},          {55, N_("Adelaide")},
    {56, N_("Tokyo")},        {57, N_("Guam")},         {58, N_("Sydney")},         {59, N_("Noumea")},
    {60, N_("Wellington")},   {61, N_("Auckland")},     {62, N_("Lima")},           {63, N_("Dakar")},
    {64, N_("Algiers")},      {65, N_("Helsinki")},     {66, N_("Athens")},         {67, N_("Nairobi")},
    {68, N_("Amsterdam")},    {69, N_("Stockholm")},    {70, N_("Lisbon")},         {71, N_("Copenhagen")},
    {72, N_("Warsaw")},       {73, N_("Prague")},       {74, N_("Budapest")},
};

//! ImageProcessing, combi-tag 0x0032 (4 bytes)
constexpr TagDetails pentaxImageProcessing[] = {
    {0x00000000, N_("Unprocessed")},      {0x00000004, N_("Digital Filter")}, {0x01000000, N_("Resized")},
    {0x02000000, N_("Cropped")},          {0x04000000, N_("Color Filter")},   {0x06000000, N_("Digital Filter 6")},
    {0x10000000, N_("Frame Synthesis?")},
};

//! PictureMode, combi-tag 0x0033 (3 bytes)
constexpr TagDetails pentaxPictureMode[] = {
    {0x000000, N_("Program")},
    {0x000100, N_("Hi-speed Program")},
    {0x000200, N_("DOF Program")},
    {0x000300, N_("MTF Program")},
    {0x000400, N_("Standard")},
    {0x000500, N_("Portrait")},
    {0x000600, N_("Landscape")},
    {0x000700, N_("Macro")},
    {0x000800, N_("Sport")},
    {0x000900, N_("Night Scene Portrait")},
    {0x000a00, N_("No Flash")},
    /* SCN modes (menu-selected) */
    {0x000b00, N_("Night Scene")},
    {0x000c00, N_("Surf & Snow")},
    {0x000d00, N_("Text")},
    {0x000e00, N_("Sunset")},
    {0x000f00, N_("Kids")},
    {0x001000, N_("Pet")},
    {0x001100, N_("Candlelight")},
    {0x001200, N_("Museum")},
    {0x001300, N_("Food")},
    {0x001400, N_("Stage Lighting")},
    {0x001500, N_("Night Snap")},
    {0x001700, N_("Blue Sky")},
    {0x001800, N_("Sunset")},
    {0x001a00, N_("Night Scene HDR")},
    {0x001b00, N_("HDR")},
    {0x001c00, N_("Quick Macro")},
    {0x001d00, N_("Forest")},
    {0x001e00, N_("Backlight Silhouette")},
    /* AUTO PICT modes (auto-selected) */
    {0x010400, N_("Auto PICT (Standard)")},
    {0x010500, N_("Auto PICT (Portrait)")},
    {0x010600, N_("Auto PICT (Landscape)")},
    {0x010700, N_("Auto PICT (Macro)")},
    {0x010800, N_("Auto PICT (Sport)")},
    /* Manual dial modes */
    {0x020000, N_("Program AE")},
    {0x030000, N_("Green Mode")},
    {0x040000, N_("Shutter Speed Priority")},
    {0x050000, N_("Aperture Priority")},
    {0x080000, N_("Manual")},
    {0x090000, N_("Bulb")},
    /* *istD modes */
    {0x020001, N_("Program AE")},
    {0x020101, N_("Hi-speed Program")},
    {0x020201, N_("DOF Program")},
    {0x020301, N_("MTF Program")},
    {0x021601, N_("Shallow DOF")},
    {0x030001, N_("Green Mode")},
    {0x040001, N_("Shutter Speed Priority")},
    {0x050001, N_("Aperture Priority")},
    {0x060001, N_("Program Tv Shift")},
    {0x070001, N_("Program Av Shift")},
    {0x080001, N_("Manual")},
    {0x090001, N_("Bulb")},
    {0x0a0001, N_("Aperture Priority (Off-Auto-Aperture)")},
    {0x0b0001, N_("Manual (Off-Auto-Aperture)")},
    {0x0c0001, N_("Bulb (Off-Auto-Aperture)")},
    /* K10D modes */
    {0x060000, N_("Shutter Priority")},
    {0x0d0000, N_("Shutter & Aperture Priority AE")},
    {0x0d0001, N_("Shutter & Aperture Priority AE (1)")},
    {0x0f0000, N_("Sensitivity Priority AE")},
    {0x0f0001, N_("Sensitivity Priority AE (1)")},
    {0x100000, N_("Flash X-Sync Speed AE")},
    {0x100001, N_("Flash X-Sync Speed AE (1)")},
    {0x120001, N_("Auto Program (Normal)")},
    {0x120101, N_("Auto Program (Hi-Speed)")},
    {0x120201, N_("Auto Program (DOF)")},
    {0x120301, N_("Auto Program (MTF)")},
    {0x121601, N_("Auto Program (Shallow DOF)")},
    {0x141601, N_("Blur control")},
    /* other modes */
    {0x000001, N_("Program")},
    {0xfe0000, N_("Video (30 fps)")},
    {0xff0004, N_("Video (24 fps)")},
};

//! DriveMode, combi-tag 0x0034 (4 bytes)
constexpr TagDetails pentaxDriveMode[] = {
    {0x00000000, N_("Single-frame")},
    {0x01000000, N_("Continuous")},
    {0x02000000, N_("Continuous (Hi)")},
    {0x03000000, N_("Burst")},
    {0xff000000, N_("Video")},
    {0x00100000, N_("Single-frame")}, /* on 645D */
    {0x00010000, N_("Self-timer (12 sec)")},
    {0x00020000, N_("Self-timer (2 sec)")},
    {0x000f0000, N_("Video")},
    {0x00100000, N_("Mirror Lock-up")},
    {0x00000100, N_("Remote Control (3 sec)")},
    {0x00000200, N_("Remote Control")},
    {0x00000400, N_("Remote Continuous Shooting")},
    {0x00000001, N_("Multiple Exposure")},
    {0x00000010, N_("HDR")},
    {0x00000020, N_("HDR Strong 1")},
    {0x00000030, N_("HDR Strong 2")},
    {0x00000040, N_("HDR Strong 3")},
    {0x000000e0, N_("HDR Auto")},
    {0x000000ff, N_("Video")},
};

//! ColorSpace, tag 0x0037
constexpr TagDetails pentaxColorSpace[] = {
    {0, N_("sRGB")},
    {1, N_("Adobe RGB")},
};

//! LensType, combi-tag 0x003f (2 unsigned long)
constexpr TagDetails pentaxLensType[] = {
    {0x0000, N_("M-42 or No Lens")},
    {0x0100, N_("K or M Lens")},
    {0x0200, N_("A Series Lens")},
    {0x0300, "Sigma Lens"},
    {0x0311, "smc PENTAX-FA SOFT 85mm F2.8"},
    {0x0312, "smc PENTAX-F 1.7X AF ADAPTER"},
    {0x0313, "smc PENTAX-F 24-50mm F4"},
    {0x0314, "smc PENTAX-F 35-80mm F4-5.6"},
    {0x0315, "smc PENTAX-F 80-200mm F4.7-5.6"},
    {0x0316, "smc PENTAX-F FISH-EYE 17-28mm F3.5-4.5"},
    {0x0317, "smc PENTAX-F 100-300mm F4.5-5.6"},         // 0
    {0x0317, "Sigma AF 28-300mm F3.5-6.3 DG IF Macro"},  // 1
    {0x0317, "Tokina 80-200mm F2.8 ATX-Pro"},            // 2
    {0x0318, "smc PENTAX-F 35-135mm F3.5-4.5"},
    {0x0319, "smc PENTAX-F 35-105mm F4-5.6"},            // 0
    {0x0319, "Sigma AF 28-300mm F3.5-5.6 DL IF"},        // 1
    {0x0319, "Sigma 55-200mm F4-5.6 DC"},                // 2
    {0x0319, "Sigma AF 28-300mm F3.5-5.6 DL IF"},        // 3
    {0x0319, "Sigma AF 28-300mm F3.5-6.3 DG IF Macro"},  // 4
    {0x0319, "Tokina 80-200mm F2.8 ATX-Pro"},            // 5
    {0x0319, "Sigma Zoom 70-210mm F4-5.6 UC-II"},        // 6
    {0x031a, "smc PENTAX-F* 250-600mm F5.6 ED[IF]"},
    {0x031b, "smc PENTAX-F 28-80mm F3.5-4.5"},        // 0
    {0x031b, "Tokina AT-X Pro AF 28-70mm F2.6-2.8"},  // 1
    {0x031c, "smc PENTAX-F 35-70mm F3.5-4.5"},        // 0
    {0x031c, "Tokina 19-35mm F3.5-4.5 AF"},           // 1
    {0x031c, "Tokina AT-X AF 400mm F5.6"},            // 2
    {0x031d, "PENTAX-F 28-80mm F3.5-4.5"},            // 0
    {0x031d, "Sigma AF 18-125mm F3.5-5.6 DC"},        // 1
    {0x031d, "Tokina AT-X PRO 28-70mm F2.6-2.8"},     // 2
    {0x031e, "PENTAX-F 70-200mm F4-5.6"},
    {0x031f, "smc PENTAX-F 70-210mm F4-5.6"},     // 0
    {0x031f, "Tokina AF 730 75-300mm F4.5-5.6"},  // 1
    {0x031f, "Takumar-F 70-210mm F4-5.6"},        // 2
    {0x0320, "smc PENTAX-F 50mm F1.4"},
    {0x0321, "smc PENTAX-F 50mm F1.7"},
    {0x0322, "smc PENTAX-F 135mm F2.8 [IF]"},
    {0x0323, "smc PENTAX-F 28mm F2.8"},
    {0x0324, "Sigma 20mm F1.8 EX DG Aspherical RF"},
    {0x0326, "smc PENTAX-F* 300mm F4.5 ED[IF]"},
    {0x0327, "smc PENTAX-F* 600mm F4 ED[IF]"},
    {0x0328, "smc PENTAX-F Macro 100mm F2.8"},
    {0x0329, "smc PENTAX-F Macro 50mm F2.8"},  // 0
    {0x0329, "Sigma 50mm F2.8 Macro"},         // 1
    {0x032a, "Sigma 300mm F2.8 EX DG APO IF"},
    {0x032c, "Tamron 35-90mm F4 AF"},                          // 0
    {0x032c, "Sigma AF 10-20mm F4-5.6 EX DC"},                 // 1
    {0x032c, "Sigma 12-24mm F4.5-5.6 EX DG"},                  // 2
    {0x032c, "Sigma 17-70mm F2.8-4.5 DC Macro"},               // 3
    {0x032c, "Sigma 18-50mm F3.5-5.6 DC"},                     // 4
    {0x032c, "Sigma 17-35mm F2.8-4 EX DG"},                    // 5
    {0x032c, "Sigma AF 18-35mm F3.5-4.5 Aspherical"},          // 6
    {0x032e, "Sigma or Samsung Lens"},                         // 0
    {0x032e, "Sigma APO 70-200mm F2.8 EX"},                    // 1
    {0x032e, "Sigma EX APO 100-300mm F4 IF"},                  // 2
    {0x032e, "Samsung/Schneider D-XENON 50-200mm F4-5.6 ED"},  // 3
    {0x0332, "smc PENTAX-FA 28-70mm F4 AL"},
    {0x0333, "Sigma 28mm F1.8 EX DG Aspherical Macro"},
    {0x0334, "smc PENTAX-FA 28-200mm F3.8-5.6 AL[IF]"},                 // 0
    {0x0334, "Tamron AF LD 28-200mm F3.8-5.6 [IF] Aspherical (171D)"},  // 1
    {0x0335, "smc PENTAX-FA 28-80mm F3.5-5.6 AL"},
    {0x03f7, "smc PENTAX-DA FISH-EYE 10-17mm F3.5-4.5 ED[IF]"},
    {0x03f8, "smc PENTAX-DA 12-24mm F4 ED AL[IF]"},
    {0x03fa, "smc PENTAX-DA 50-200mm F4-5.6 ED"},
    {0x03fb, "smc PENTAX-DA 40mm F2.8 Limited"},
    {0x03fc, "smc PENTAX-DA 18-55mm F3.5-5.6 AL"},
    {0x03fd, "smc PENTAX-DA 14mm F2.8 ED[IF]"},
    {0x03fe, "smc PENTAX-DA 16-45mm F4 ED AL"},
    {0x03ff, "Sigma Lens"},                             // 0
    {0x03ff, "Sigma 18-200mm F3.5-6.3 DC"},             // 1
    {0x03ff, "Sigma DL-II 35-80mm F4-5.6"},             // 2
    {0x03ff, "Sigma DL Zoom 75-300mm F4-5.6"},          // 3
    {0x03ff, "Sigma DF EX Aspherical 28-70mm F2.8"},    // 4
    {0x03ff, "Sigma AF Tele 400mm F5.6 Multi-coated"},  // 5
    {0x03ff, "Sigma 24-60mm F2.8 EX DG"},               // 6
    {0x03ff, "Sigma 70-300mm F4-5.6 Macro"},            // 7
    {0x03ff, "Sigma 55-200mm F4-5.6 DC"},               // 8
    {0x03ff, "Sigma 18-50mm F2.8 EX DC"},               // 9
    {0x03ff, "Sigma 18-50mm F2.8 EX DC Macro"},         // 10
    {0x0401, "smc PENTAX-FA SOFT 28mm F2.8"},
    {0x0402, "smc PENTAX-FA 80-320mm F4.5-5.6"},
    {0x0403, "smc PENTAX-FA 43mm F1.9 Limited"},
    {0x0406, "smc PENTAX-FA 35-80mm F4-5.6"},
    {0x0407, "Irix 45mm F/1.4"},
    {0x0408, "Irix 150mm F/2.8 Macro"},
    {0x0409, "Irix 11mm F/4"},
    {0x040a, "Irix 15mm F/2.4"},
    {0x040c, "smc PENTAX-FA 50mm F1.4"},
    {0x040f, "smc PENTAX-FA 28-105mm F4-5.6 [IF]"},
    {0x0410, "Tamron AF 80-210mm F4-5.6 (178D)"},
    {0x0413, "Tamron SP AF 90mm F2.8 (172E)"},
    {0x0414, "smc PENTAX-FA 28-80mm F3.5-5.6"},
    {0x0415, "Cosina AF 100-300mm F5.6-6.7"},
    {0x0416, "Tokina 28-80mm F3.5-5.6"},
    {0x0417, "smc PENTAX-FA 20-35mm F4 AL"},
    {0x0418, "smc PENTAX-FA 77mm F1.8 Limited"},
    {0x0419, "Tamron SP AF 14mm F2.8"},
    {0x041a, "smc PENTAX-FA Macro 100mm F3.5"},  // 0
    {0x041a, "Cosina 100mm F3.5 Macro"},         // 1
    {0x041b, "Tamron AF28-300mm F/3.5-6.3 LD Aspherical[IF] Macro (185D/285D)"},
    {0x041c, "smc PENTAX-FA 35mm F2 AL"},
    {0x041d, "Tamron AF 28-200mm F/3.8-5.6 LD Super II Macro (371D)"},
    {0x0422, "smc PENTAX-FA 24-90mm F3.5-4.5 AL[IF]"},
    {0x0423, "smc PENTAX-FA 100-300mm F4.7-5.8"},
    {0x0424, "Tamron AF 70-300mm F/4-5.6 LD Macro 1:2 (572D/A17)"},
    {0x0425, "Tamron SP AF 24-135mm F3.5-5.6 AD AL (190D)"},
    {0x0426, "smc PENTAX-FA 28-105mm F3.2-4.5 AL[IF]"},
    {0x0427, "smc PENTAX-FA 31mm F1.8AL Limited"},
    {0x0429, "Tamron AF 28-200mm Super Zoom F3.8-5.6 Aspherical XR [IF] Macro (A03)"},
    {0x042b, "smc PENTAX-FA 28-90mm F3.5-5.6"},
    {0x042c, "smc PENTAX-FA J 75-300mm F4.5-5.8 AL"},
    {0x042d, "Tamron Lens"},                                                 // 0
    {0x042d, "Tamron 28-300mm F3.5-6.3 Ultra zoom XR"},                      // 1
    {0x042d, "Tamron AF 28-300mm F3.5-6.3 XR Di LD Aspherical [IF] Macro"},  // 2
    {0x042e, "smc PENTAX-FA J 28-80mm F3.5-5.6 AL"},
    {0x042f, "smc PENTAX-FA J 18-35mm F4-5.6 AL"},
    {0x0431, "Tamron SP AF 28-75mm F2.8 XR Di LD Aspherical [IF] Macro (A09)"},
    {0x0433, "smc PENTAX-D FA 50mm F2.8 Macro"},
    {0x0434, "smc PENTAX-D FA 100mm F2.8 Macro"},
    {0x0437, "Samsung/Schneider D-XENOGON 35mm F2"},
    {0x0438, "Samsung/Schneider D-XENON 100mm F2.8 Macro"},
    {0x044b, "Tamron SP AF 70-200mm F2.8 Di LD [IF] Macro (A001)"},
    {0x04d6, "smc PENTAX-DA 35mm F2.4 AL"},
    {0x04e5, "smc PENTAX-DA 18-55mm F3.5-5.6 AL II"},
    {0x04e6, "Tamron SP AF 17-50mm F2.8 XR Di II"},
    {0x04e7, "smc PENTAX-DA 18-250mm F3.5-6.3 ED AL [IF]"},
    {0x04ed, "Samsung/Schneider D-XENOGON 10-17mm F3.5-4.5"},
    {0x04ef, "Samsung/Schneider D-XENON 12-24mm F4 ED AL [IF]"},
    {0x04f2, "smc PENTAX-DA* 16-50mm F2.8 ED AL [IF] SDM (SDM unused)"},
    {0x04f3, "smc PENTAX-DA 70mm F2.4 Limited"},
    {0x04f4, "smc PENTAX-DA 21mm F3.2 AL Limited"},
    {0x04f5, "Samsung/Schneider D-XENON 50-200mm F4-5.6"},
    {0x04f6, "Samsung/Schneider D-XENON 18-55mm F3.5-5.6"},
    {0x04f7, "smc PENTAX-DA FISH-EYE 10-17mm F3.5-4.5 ED [IF]"},
    {0x04f8, "smc PENTAX-DA 12-24mm F4 ED AL [IF]"},
    {0x04f9, "Tamron XR DiII 18-200mm F3.5-6.3 (A14)"},
    {0x04fa, "smc PENTAX-DA 50-200mm F4-5.6 ED"},
    {0x04fb, "smc PENTAX-DA 40mm F2.8 Limited"},
    {0x04fc, "smc PENTAX-DA 18-55mm F3.5-5.6 AL"},
    {0x04fd, "smc PENTAX-DA 14mm F2.8 ED[IF]"},
    {0x04fe, "smc PENTAX-DA 16-45mm F4 ED AL"},
    {0x0501, "smc PENTAX-FA* 24mm F2 AL[IF]"},
    {0x0502, "smc PENTAX-FA 28mm F2.8 AL"},
    {0x0503, "smc PENTAX-FA 50mm F1.7"},
    {0x0504, "smc PENTAX-FA 50mm F1.4"},
    {0x0505, "smc PENTAX-FA* 600mm F4 ED[IF]"},
    {0x0506, "smc PENTAX-FA* 300mm F4.5 ED[IF]"},
    {0x0507, "smc PENTAX-FA 135mm F2.8 [IF]"},
    {0x0508, "smc PENTAX-FA Macro 50mm F2.8"},
    {0x0509, "smc PENTAX-FA Macro 100mm F2.8"},
    {0x050a, "smc PENTAX-FA* 85mm F1.4 [IF]"},
    {0x050b, "smc PENTAX-FA* 200mm F2.8 ED[IF]"},
    {0x050c, "smc PENTAX-FA 28-80mm F3.5-4.7"},
    {0x050d, "smc PENTAX-FA 70-200mm F4-5.6"},
    {0x050e, "smc PENTAX-FA* 250-600mm F5.6 ED[IF]"},
    {0x050f, "smc PENTAX-FA 28-105mm F4-5.6"},
    {0x0510, "smc PENTAX-FA 100-300mm F4.5-5.6"},
    {0x0562, "smc PENTAX-FA 100-300mm F4.5-5.6"},
    {0x0601, "smc PENTAX-FA* 85mm F1.4[IF]"},
    {0x0602, "smc PENTAX-FA* 200mm F2.8 ED[IF]"},
    {0x0603, "smc PENTAX-FA* 300mm F2.8 ED[IF]"},
    {0x0604, "smc PENTAX-FA* 28-70mm F2.8 AL"},
    {0x0605, "smc PENTAX-FA* 80-200mm F2.8 ED[IF]"},
    {0x0606, "smc PENTAX-FA* 28-70mm F2.8 AL"},
    {0x0607, "smc PENTAX-FA* 80-200mm F2.8 ED[IF]"},
    {0x0608, "smc PENTAX-FA 28-70mm F4AL"},
    {0x0609, "smc PENTAX-FA 20mm F2.8"},
    {0x060a, "smc PENTAX-FA* 400mm F5.6 ED[IF]"},
    {0x060d, "smc PENTAX-FA* 400mm F5.6 ED[IF]"},
    {0x060e, "smc PENTAX-FA* Macro 200mm F4 ED[IF]"},
    {0x0700, "smc PENTAX-DA 21mm F3.2 AL Limited"},
    {0x073a, "smc PENTAX-D FA Macro 100mm F2.8 WR"},
    {0x074b, "Tamron SP AF 70-200mm F2.8 Di LD [IF] Macro (A001)"},
    {0x07c9, "smc Pentax-DA L 50-200mm F4-5.6 ED WR"},
    {0x07ca, "smc PENTAX-DA L 18-55mm F3.5-5.6 AL WR"},
    {0x07cb, "HD PENTAX-DA 55-300mm F4-5.8 ED WR"},
    {0x07cc, "HD PENTAX-DA 15mm F4 ED AL Limited"},
    {0x07cd, "HD PENTAX-DA 35mm F2.8 Macro Limited"},
    {0x07ce, "HD PENTAX-DA 70mm F2.4 Limited"},
    {0x07cf, "HD PENTAX-DA 21mm F3.2 ED AL Limited"},
    {0x07d0, "HD PENTAX-DA 40mm F2.8 Limited"},
    {0x07d4, "smc PENTAX-DA 50mm F1.8"},
    {0x07d5, "smc PENTAX-DA 40mm F2.8 XS"},
    {0x07d6, "smc PENTAX-DA 35mm F2.4 AL"},
    {0x07d8, "smc PENTAX-DA L 55-300mm F4-5.8 ED"},
    {0x07d9, "smc PENTAX-DA 50-200mm F4-5.6 ED WR"},
    {0x07da, "smc PENTAX-DA 18-55mm F3.5-5.6 AL WR"},
    {0x07dc, "Tamron SP AF 10-24mm F3.5-4.5 Di II LD Aspherical [IF]"},
    {0x07dd, "smc PENTAX-DA L 50-200mm F4-5.6 ED"},
    {0x07de, "smc PENTAX-DA L 18-55mm F3.5-5.6"},
    {0x07df, "Samsung/Schneider D-XENON 18-55mm F3.5-5.6 II"},
    {0x07e0, "smc PENTAX-DA 15mm F4 ED AL Limited"},
    {0x07e1, "Samsung/Schneider D-XENON 18-250mm F3.5-6.3"},
    {0x07e2, "smc PENTAX-DA* 55mm F1.4 SDM (SDM unused)"},
    {0x07e3, "smc PENTAX-DA* 60-250mm F4 [IF] SDM (SDM unused)"},
    {0x07e4, "Samsung 16-45mm F4 ED"},
    {0x07e5, "smc PENTAX-DA 18-55mm F3.5-5.6 AL II"},
    {0x07e6, "Tamron AF 17-50mm F2.8 XR Di-II LD (Model A16)"},
    {0x07e7, "smc PENTAX-DA 18-250mm F3.5-6.3ED AL [IF]"},
    {0x07e9, "smc PENTAX-DA 35mm F2.8 Macro Limited"},
    {0x07ea, "smc PENTAX-DA* 300 mm F4ED [IF] SDM (SDM not used)"},
    {0x07eb, "smc PENTAX-DA* 200mm F2.8 ED [IF] SDM (SDM not used)"},
    {0x07ec, "smc PENTAX-DA 55-300mm F4-5.8 ED"},
    {0x07ee, "Tamron AF 18-250mm F3.5-6.3 Di II LD Aspherical [IF] Macro"},
    {0x07f1, "smc PENTAX-DA* 50-135mm F2.8 ED [IF] SDM (SDM not used)"},
    {0x07f2, "smc PENTAX-DA* 16-50mm F2.8 ED AL [IF] SDM (SDM not used)"},
    {0x07f3, "smc PENTAX-DA 70mm F2.4 Limited"},
    {0x07f4, "smc PENTAX-DA 21mm F3.2 AL Limited"},
    {0x0800, "Sigma 50-150mm F2.8 II APO EX DC HSM"},
    {0x0803, "Sigma AF 18-125mm F3.5-5.6 DC"},
    {0x0804, "Sigma 50mm F1.4 EX DG HSM"},
    {0x0807, "Sigma 24-70mm F2.8 IF EX DG HSM"},
    {0x0808, "Sigma 18-250mm F3.5-6.3 DC OS HSM"},
    {0x080b, "Sigma 10-20mm F3.5 EX DC HSM"},
    {0x080c, "Sigma 70-300mm F4-5.6 DG OS"},
    {0x080d, "Sigma 120-400mm F4.5-5.6 APO DG OS HSM"},
    {0x080e, "Sigma 17-70mm F2.8-4.0 DC Macro OS HSM"},
    {0x080f, "Sigma 150-500mm F5-6.3 APO DG OS HSM"},
    {0x0810, "Sigma 70-200mm F2.8 EX DG Macro HSM II"},
    {0x0811, "Sigma 50-500mm F4.5-6.3 DG OS HSM"},
    {0x0812, "Sigma 8-16mm F4.5-5.6 DC HSM"},
    {0x0815, "Sigma 17-50mm F2.8 EX DC OS HSM"},
    {0x0816, "Sigma 85mm F1.4 EX DG HSM"},
    {0x0817, "Sigma 70-200mm F2.8 APO EX DG OS HSM"},
    {0x0819, "Sigma 17-50mm F2.8 EX DC HSM"},
    {0x081b, "Sigma 18-200mm F3.5-6.3 II DC HSM"},
    {0x081c, "Sigma 18-250mm F3.5-6.3 DC Macro HSM"},
    {0x081d, "Sigma 35mm F1.4 DG HSM"},
    {0x081e, "Sigma 17-70mm F2.8-4 DC Macro HSM | C"},
    {0x081f, "Sigma 18-35mm F1.8 DC HSM"},
    {0x0820, "Sigma 30mm F1.4 DC HSM | A"},
    {0x0822, "Sigma 18-300mm F3.5-6.3 DC Macro HSM"},
    {0x083b, "HD PENTAX-D FA 150-450mm F4.5-5.6 ED DC AW"},
    {0x083c, "HD PENTAX-D FA* 70-200mm F2.8 ED DC AW"},
    {0x083d, "HD PENTAX-D FA 28-105mm F3.5-5.6 ED DC WR"},
    {0x083e, "HD PENTAX-D FA 24-70mm F2.8 ED SDM WR"},
    {0x083f, "HD PENTAX-D FA 15-30mm F2.8 ED SDM WR"},
    {0x0840, "HD PENTAX-D FA* 50mm F1.4 SDM AW"},
    {0x0841, "HD PENTAX-D FA 70-210mm F4 ED SDM WR"},
    {0x0842, "HD PENTAX-D FA* 85mm F1.4 SDM AW"},
    {0x0843, "HD PENTAX-D FA 21mm F2.4 ED Limited DC WR"},
    {0x08c3, "HD PENTAX DA* 16-50mm F2.8 ED PLM AW"},
    {0x08c4, "HD PENTAX-DA* 11-18mm F2.8 ED DC AW"},
    {0x08c5, "HD PENTAX-DA 55-300mm F4.5-6.3 ED PLM WR RE"},
    {0x08c6, "smc PENTAX-DA L 18-50mm F4-5.6 DC WR RE"},
    {0x08c7, "HD PENTAX-DA 18-50mm F4-5.6 DC WR RE"},
    {0x08c8, "HD PENTAX-DA 16-85mm F3.5-5.6 ED DC WR"},
    {0x08d1, "HD PENTAX-DA 20-40mm F2.8-4 ED Limited DC WR"},
    {0x08d2, "smc PENTAX-DA 18-270mm F3.5-6.3 ED SDM"},
    {0x08d3, "HD PENTAX-DA 560mm F5.6 ED AW"},
    {0x08d7, "smc PENTAX-DA 18-135mm F3.5-5.6 ED AL [IF] DC WR"},
    {0x08e2, "smc PENTAX-DA* 55mm F1.4 SDM"},
    {0x08e3, "smc PENTAX DA* 60-250mm F4 [IF] SDM"},
    {0x08e8, "smc PENTAX-DA 17-70mm F4 AL [IF] SDM"},
    {0x08ea, "smc PENTAX-DA* 300mm F4 ED [IF] SDM"},
    {0x08eb, "smc PENTAX-DA* 200mm F2.8 ED [IF] SDM"},
    {0x08f1, "smc PENTAX-DA* 50-135mm F2.8 ED [IF] SDM"},
    {0x08f2, "smc PENTAX-DA* 16-50mm F2.8 ED AL [IF] SDM"},
    {0x08ff, "Sigma Lens"},                                   // 0
    {0x08ff, "Sigma 70-200mm F2.8 EX DG Macro HSM II"},       // 1
    {0x08ff, "Sigma 150-500mm F5-6.3 DG APO [OS] HSM"},       // 2
    {0x08ff, "Sigma 50-150mm F2.8 II APO EX DC HSM"},         // 3
    {0x08ff, "Sigma 4.5mm F2.8 EX DC HSM Circular Fisheye"},  // 4
    {0x08ff, "Sigma 50-200mm F4-5.6 DC OS"},                  // 5
    {0x08ff, "Sigma 24-70mm F2.8 EX DG HSM"},                 // 6
    {0x08ff, "Sigma 18-50mm F2.8-4.5 HSM OS"},                // 7
    {0x0900, "645 Manual Lens"},
    {0x0a00, "645 A Series Lens"},
    {0x0b01, "smc PENTAX-FA 645 75mm F2.8"},
    {0x0b02, "smc PENTAX-FA 645 45mm F2.8"},
    {0x0b03, "smc PENTAX-FA* 645 300mm F4 ED [IF]"},
    {0x0b04, "smc PENTAX-FA 645 45mm-85mm F4.5"},
    {0x0b05, "smc PENTAX-FA 645 400mm F5.6 ED [IF]"},
    {0x0b07, "smc PENTAX-FA 645 Macro 120mm F4"},
    {0x0b08, "smc PENTAX-FA 645 80-160mm F4.5"},
    {0x0b09, "smc PENTAX-FA 645 200mm F4 [IF]"},
    {0x0b0a, "smc PENTAX-FA 645 150mm F2.8 [IF]"},
    {0x0b0b, "smc PENTAX-FA 645 35mm F3.5 AL [IF]"},
    {0x0b0c, "smc PENTAX-FA 645 300mm F5.6 ED [IF]"},
    {0x0b0e, "smc PENTAX-FA 645 55-110mm F5.6"},
    {0x0b10, "smc PENTAX-FA 645 33-55mm F4.5 AL"},
    {0x0b11, "smc PENTAX-FA 645 150-300mm F5.6 ED [IF]"},
    {0x0b15, "HD PENTAX-D FA 645 35mm F3.5 AL [IF]"},
    {0x0d12, "smc PENTAX-D FA 645 55mm F2.8 AL [IF] SDM AW"},
    {0x0d13, "smc PENTAX-D FA 645 25mm F4 AL [IF] SDM AW"},
    {0x0d14, "HD PENTAX-D FA 645 90mm F2.8 ED AW SR"},
    {0x0dfd, "HD PENTAX-DA 645 28-45mm F4.5 ED AW SR"},
    {0x1500, "Pentax Q Manual Lens"},
    {0x1501, "01 Standard Prime 8.5mm F1.9"},
    {0x1502, "02 Standard Zoom 5-15mm F2.8-4.5"},
    {0x1603, "03 Fish-eye 3.2mm F5.6"},
    {0x1604, "04 Toy Lens Wide 6.3mm F7.1"},
    {0x1605, "05 Toy Lens Telephoto 18mm F8"},
    {0x1506, "06 Telephoto Zoom 15-45mm F2.8"},
    {0x1507, "07 Mount Shield 11.5mm F9"},
    {0x1508, "08 Wide Zoom 3.8-5.9mm F3.7-4"},
    {0x15e9, "Adapter Q for K-mount Lens"},
};

//! ImageTone, tag 0x004f
constexpr TagDetails pentaxImageTone[] = {
    {0, N_("Natural")},    {1, N_("Bright")}, {2, N_("Portrait")},      {3, N_("Landscape")},     {4, N_("Vibrant")},
    {5, N_("Monochrome")}, {6, N_("Muted")},  {7, N_("Reversal film")}, {8, N_("Bleach bypass")}, {9, N_("Radiant")},
};

//! DynamicRangeExpansion, tag 0x0069
constexpr TagDetails pentaxDynamicRangeExpansion[] = {
    {0, N_("Off")},
    {0x1000000, N_("On")},
};

//! HighISONoiseReduction, tag 0x0071
constexpr TagDetails pentaxHighISONoiseReduction[] = {
    {0, N_("Off")}, {1, N_("Weakest")}, {2, N_("Weak")}, {3, N_("Strong")}, {4, N_("Custom")},
};

std::ostream& PentaxMakerNote::printVersion(std::ostream& os, const Value& value, const ExifData*) {
  std::string val = value.toString();
  std::replace(val.begin(), val.end(), ' ', '.');
  os << val;
  return os;
}

std::ostream& PentaxMakerNote::printResolution(std::ostream& os, const Value& value, const ExifData*) {
  std::string val = value.toString();
  std::replace(val.begin(), val.end(), ' ', 'x');
  os << val;
  return os;
}

std::ostream& PentaxMakerNote::printDate(std::ostream& os, const Value& value, const ExifData*) {
  /* I choose same format as is used inside EXIF itself */
  return os << stringFormat("{}:{:02}:{:02}", ((static_cast<uint16_t>(value.toInt64(0)) << 8) + value.toInt64(1)),
                            value.toInt64(2), value.toInt64(3));
}

std::ostream& PentaxMakerNote::printTime(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:02}:{:02}:{:02}", value.toInt64(0), value.toInt64(1), value.toInt64(2));
}

std::ostream& PentaxMakerNote::printExposure(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:g} ms", static_cast<float>(value.toInt64()) / 100);
}

std::ostream& PentaxMakerNote::printFValue(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("F{:.2g}", static_cast<float>(value.toInt64()) / 10);
}

std::ostream& PentaxMakerNote::printFocalLength(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:.1f} mm", static_cast<float>(value.toInt64()) / 100);
}

std::ostream& PentaxMakerNote::printCompensation(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:.2g} EV", (static_cast<float>(value.toInt64()) - 50) / 10);
}

std::ostream& PentaxMakerNote::printTemperature(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{} C", value.toInt64());
}

std::ostream& PentaxMakerNote::printFlashCompensation(std::ostream& os, const Value& value, const ExifData*) {
  return os << stringFormat("{:.2g} EV", static_cast<float>(value.toInt64()) / 256);
}

std::ostream& PentaxMakerNote::printBracketing(std::ostream& os, const Value& value, const ExifData*) {
  if (auto l0 = value.toUint32(0); l0 < 10) {
    os << std::setprecision(2) << static_cast<float>(l0) / 3 << " EV";
  } else {
    os << std::setprecision(2) << static_cast<float>(l0) - 9.5F << " EV";
  }

  if (value.count() == 2) {
    const auto l1 = value.toUint32(1);
    os << " (";
    if (l1 == 0) {
      os << _("No extended bracketing");
    } else {
      auto type = l1 >> 8;
      auto range = static_cast<byte>(l1);
      switch (type) {
        case 1:
          os << _("WB-BA");
          break;
        case 2:
          os << _("WB-GM");
          break;
        case 3:
          os << _("Saturation");
          break;
        case 4:
          os << _("Sharpness");
          break;
        case 5:
          os << _("Contrast");
          break;
        default:
          os << _("Unknown ") << type;
          break;
      }
      os << " " << +range;
    }
    os << ")";
  }
  return os;
}

std::ostream& PentaxMakerNote::printShutterCount(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (!metadata)
    return os << "undefined";

  auto dateIt = metadata->findKey(ExifKey("Exif.PentaxDng.Date"));
  if (dateIt == metadata->end()) {
    dateIt = metadata->findKey(ExifKey("Exif.Pentax.Date"));
  }

  auto timeIt = metadata->findKey(ExifKey("Exif.PentaxDng.Time"));
  if (timeIt == metadata->end()) {
    timeIt = metadata->findKey(ExifKey("Exif.Pentax.Time"));
  }

  if (dateIt == metadata->end() || dateIt->size() != 4 || timeIt == metadata->end() || timeIt->size() != 3 ||
      value.size() != 4) {
    os << "undefined";
    return os;
  }
  const uint32_t date = (dateIt->toUint32(0) << 24) + (dateIt->toUint32(1) << 16) + (dateIt->toUint32(2) << 8) +
                        (dateIt->toUint32(3) << 0);
  const uint32_t time = (timeIt->toUint32(0) << 24) + (timeIt->toUint32(1) << 16) + (timeIt->toUint32(2) << 8);
  const uint32_t countEnc =
      (value.toUint32(0) << 24) + (value.toUint32(1) << 16) + (value.toUint32(2) << 8) + (value.toUint32(3) << 0);
  // The shutter count is encoded using date and time values stored
  // in Pentax-specific tags.  The prototype for the encoding/decoding
  // function is taken from Phil Harvey's ExifTool: Pentax.pm file,
  // CryptShutterCount() routine.
  const uint32_t count = countEnc ^ date ^ (~time);
  os << count;
  return os;
}

// #1144 begin
static std::string getKeyString(const std::string& key, const ExifData* metadata) {
  std::string result;
  if (metadata->findKey(ExifKey(key)) != metadata->end()) {
    result = metadata->findKey(ExifKey(key))->toString();
  }
  return result;
}

static long getKeyLong(const std::string& key, const ExifData* metadata) {
  long result = -1;
  if (metadata->findKey(ExifKey(key)) != metadata->end()) {
    result = static_cast<long>(metadata->findKey(ExifKey(key))->toFloat(0));
  }
  return result;
}

// Throws std::exception if the LensInfo can't be found.
static ExifData::const_iterator findLensInfo(const ExifData* metadata) {
  const auto dngLensInfo = metadata->findKey(ExifKey("Exif.PentaxDng.LensInfo"));
  if (dngLensInfo != metadata->end()) {
    return dngLensInfo;
  }
  const auto lensInfo = metadata->findKey(ExifKey("Exif.Pentax.LensInfo"));
  if (lensInfo != metadata->end()) {
    return lensInfo;
  }
  throw LensInfoNotFound();
}

//! resolveLens0x32c print lens in human format
static std::ostream& resolveLens0x32c(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    unsigned long index = 0;

    long focalLength = getKeyLong("Exif.Photo.FocalLength", metadata);
    bool bFL10_20 = 10 <= focalLength && focalLength <= 20;

    // std::cout << "model,focalLength = " << model << "," << focalLength << '\n';
    if (bFL10_20) {
      index = 1;
    }

    if (index > 0) {
      const unsigned long lensID = 0x32c;
      auto td = Exiv2::find(pentaxLensType, lensID);
      os << exvGettext(td[index].label_);
      return os;
    }
  } catch (...) {
  }
  return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
}
// #1144 end

// #816 begin
//! resolveLens0x3ff print lens in human format
static std::ostream& resolveLens0x3ff(std::ostream& os, const Value& value, const ExifData* metadata)
// ----------------------------------------------------------------------
{
  try {
    unsigned long index = 0;

    // http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Pentax.html#LensData
    const auto lensInfo = findLensInfo(metadata);
    if (lensInfo->count() < 5)
      return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);

    if (value.count() == 2) {
      // http://dev.exiv2.org/attachments/download/326/IMGP4432.PEF
      // exiv2 -pv --grep Lens ~/Downloads/IMGP4432.PEF
      // 0x003f Pentax       LensType  Byte        2 3 255
      // 0x0207 Pentax       LensInfo  Undefined  36 3 255 0 0 40 148 71 152 80 6 241 65 237 153 88 36 1 76
      // 107 251 255 255 255 0 0 80 6 241 0 0 0 0 0 0 0 0
      unsigned long base = 1;

      unsigned int autoAperture = lensInfo->toUint32(base + 1) & 0x01;
      unsigned int minAperture = lensInfo->toUint32(base + 2) & 0x06;
      unsigned int minFocusDistance = lensInfo->toUint32(base + 3) & 0xf8;

      if (autoAperture == 0x0 && minAperture == 0x0 && minFocusDistance == 0x28 && lensInfo->toUint32(base + 4) == 148)
        index = 8;
      if (autoAperture == 0x0 && minAperture == 0x0 && minFocusDistance == 0x28 && lensInfo->toUint32(base + 5) == 110)
        index = 7;
      if (autoAperture == 0x0 && minAperture == 0x0 && minFocusDistance == 0x28 && lensInfo->toUint32(base + 4) == 110)
        index = 7;

    } else if (value.count() == 3) {
      // http://dev.exiv2.org/attachments/download/858/_IGP9032.DNG
      // $ exiv2 -pv --grep Lens ~/Downloads/_IGP9032.DNG
      // 0x003f PentaxDng    LensType  Byte        3    3 255   0
      // 0x0207 PentaxDng    LensInfo  Undefined  69  131   0   0 255 0  40 148  68 244 ...
      //                                                0   1   2   3 4   5   6
      if (lensInfo->toUint32(4) == 0 && lensInfo->toUint32(5) == 40 && lensInfo->toUint32(6) == 148)
        index = 8;

    } else if (value.count() == 4) {
      // http://dev.exiv2.org/attachments/download/868/IMGP2221.JPG
      // 0x0207 PentaxDng    LensInfo  Undefined 128    0 131 128   0 0 255   1 184   0 0 0 0 0
      //                                                0   1   2   3 4   5   6
      if (lensInfo->count() == 128 && lensInfo->toUint32(1) == 131 && lensInfo->toUint32(2) == 128)
        index = 8;
      // #1155
      if (lensInfo->toUint32(6) == 5)
        index = 7;
    }

    if (index > 0) {
      const unsigned long lensID = 0x3ff;
      auto td = Exiv2::find(pentaxLensType, lensID);
      os << exvGettext(td[index].label_);
      return os;
    }
  } catch (...) {
  }
  return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
}

// #1155
//! resolveLens0x8ff print lens in human format
static std::ostream& resolveLens0x8ff(std::ostream& os, const Value& value, const ExifData* metadata)
// ----------------------------------------------------------------------
{
  try {
    unsigned long index = 0;

    const auto lensInfo = findLensInfo(metadata);
    if (value.count() == 4) {
      std::string model = getKeyString("Exif.Image.Model", metadata);
      if (model.starts_with("PENTAX K-3") && lensInfo->count() == 128 && lensInfo->toUint32(1) == 168 &&
          lensInfo->toUint32(2) == 144)
        index = 7;
    }

    if (index > 0) {
      const unsigned long lensID = 0x8ff;
      auto td = Exiv2::find(pentaxLensType, lensID);
      os << exvGettext(td[index].label_);
      return os;
    }
  } catch (...) {
  }
  return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
}

// #1155
//! resolveLens0x319 print lens in human format
static std::ostream& resolveLens0x319(std::ostream& os, const Value& value, const ExifData* metadata)
// ----------------------------------------------------------------------
{
  try {
    unsigned long index = 0;

    const auto lensInfo = findLensInfo(metadata);
    if (value.count() == 4) {
      std::string model = getKeyString("Exif.Image.Model", metadata);
      if (model.starts_with("PENTAX K-3") && lensInfo->count() == 128 && lensInfo->toUint32(1) == 131 &&
          lensInfo->toUint32(2) == 128)
        index = 6;
    }
    if (value.count() == 2) {
      std::string model = getKeyString("Exif.Image.Model", metadata);
      if (model.starts_with("PENTAX K100D") && lensInfo->count() == 44)
        index = 6;
      if (model.starts_with("PENTAX *ist DL") && lensInfo->count() == 36)
        index = 6;
    }

    if (index > 0) {
      const unsigned long lensID = 0x319;
      auto td = Exiv2::find(pentaxLensType, lensID);
      os << exvGettext(td[index].label_);
      return os;
    }
  } catch (...) {
  }
  return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
}

//! resolveLensType print lens in human format
static std::ostream& resolveLensType(std::ostream& os, const Value& value, const ExifData* metadata) {
  return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
}

//! A lens id and a pretty-print function for special treatment of the id.
static std::ostream& printLensType(std::ostream& os, const Value& value, const ExifData* metadata) {
  //! List of lens ids which require special treatment using resolveLensType
  static constexpr struct LensIdFct {
    uint32_t id_;   //!< Lens id
    PrintFct fct_;  //!< Pretty-print function
    //! Comparison operator for find template
    bool operator==(uint32_t id) const {
      return id_ == id;
    }
  } lensIdFct[] = {
      {0x0317, resolveLensType}, {0x0319, resolveLens0x319}, {0x031b, resolveLensType},  {0x031c, resolveLensType},
      {0x031d, resolveLensType}, {0x031f, resolveLensType},  {0x0329, resolveLensType},  {0x032c, resolveLens0x32c},
      {0x032e, resolveLensType}, {0x0334, resolveLensType},  {0x03ff, resolveLens0x3ff}, {0x041a, resolveLensType},
      {0x042d, resolveLensType}, {0x08ff, resolveLens0x8ff},
  };
  // #1034
  const std::string undefined("undefined");
  const std::string section("pentax");
  if (Internal::readExiv2Config(section, value.toString(), undefined) != undefined) {
    return os << Internal::readExiv2Config(section, value.toString(), undefined);
  }

  const auto index = (value.toUint32(0) * 256) + value.toUint32(1);

  // std::cout << '\n' << "printLensType value =" << value.toLong() << " index = " << index << '\n';
  auto lif = Exiv2::find(lensIdFct, index);
  if (!lif)
    return EXV_PRINT_COMBITAG_MULTI(pentaxLensType, 2, 1, 2)(os, value, metadata);
  if (metadata && lif->fct_)
    return lif->fct_(os, value, metadata);
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << "(" << value << ")";
  return os << value;
}

// #816 end
// ----------------------------------------------------------------------

// Pentax MakerNote Tag Info
constexpr TagInfo PentaxMakerNote::tagInfo_[] = {
    {0x0000, "Version", N_("Version"), N_("Pentax Makernote version"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, printVersion},
    {0x0001, "Mode", N_("Shooting mode"), N_("Camera shooting mode"), IfdId::pentaxId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(pentaxShootingMode)},
    {0x0002, "PreviewResolution", N_("Resolution of a preview image"), N_("Resolution of a preview image"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printResolution},
    {0x0003, "PreviewLength", N_("Length of a preview image"), N_("Size of an IFD containing a preview image"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printValue},
    {0x0004, "PreviewOffset", N_("Pointer to a preview image"), N_("Offset to an IFD containing a preview image"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printValue},
    {0x0005, "ModelID", N_("Model identification"), N_("Pentax model identification"), IfdId::pentaxId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(pentaxModel)},
    {0x0006, "Date", N_("Date"), N_("Date"), IfdId::pentaxId, SectionId::makerTags, undefined, -1, printDate},
    {0x0007, "Time", N_("Time"), N_("Time"), IfdId::pentaxId, SectionId::makerTags, undefined, -1, printTime},
    {0x0008, "Quality", N_("Image quality"), N_("Image quality settings"), IfdId::pentaxId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(pentaxQuality)},
    {0x0009, "Size", N_("Image size"), N_("Image size settings"), IfdId::pentaxId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(pentaxSize)},
    /* Some missing ! */
    {0x000c, "Flash", N_("Flash mode"), N_("Flash mode settings"), IfdId::pentaxId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(pentaxFlash)},
    {0x000d, "Focus", N_("Focus mode"), N_("Focus mode settings"), IfdId::pentaxId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(pentaxFocus)},
    {0x000e, "AFPoint", N_("AF point"), N_("Selected AF point"), IfdId::pentaxId, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(pentaxAFPoint)},
    {0x000F, "AFPointInFocus", N_("AF point in focus"), N_("AF point in focus"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(pentaxAFPointFocus)},
    /* Some missing ! */
    {0x0012, "ExposureTime", N_("Exposure time"), N_("Exposure time"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, printExposure},
    {0x0013, "FNumber", N_("F-Number"), N_("F-Number"), IfdId::pentaxId, SectionId::makerTags, unsignedLong, -1,
     printFValue},
    {0x0014, "ISO", N_("ISO sensitivity"), N_("ISO sensitivity settings"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(pentaxISO)},
    /* Some missing ! */
    {0x0016, "ExposureCompensation", N_("Exposure compensation"), N_("Exposure compensation"), IfdId::pentaxId,
     SectionId::makerTags, unsignedLong, -1, printCompensation},
    /* Some missing ! */
    {0x0017, "MeteringMode", N_("MeteringMode"), N_("MeteringMode"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, EXV_PRINT_TAG(pentaxMeteringMode)},
    {0x0018, "AutoBracketing", N_("AutoBracketing"), N_("AutoBracketing"), IfdId::pentaxId, SectionId::makerTags,
     undefined, -1, printBracketing},
    {0x0019, "WhiteBalance", N_("White balance"), N_("White balance"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, EXV_PRINT_TAG(pentaxWhiteBalance)},
    {0x001a, "WhiteBalanceMode", N_("White balance mode"), N_("White balance mode"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(pentaxWhiteBalanceMode)},
    {0x001b, "BlueBalance", N_("Blue balance"), N_("Blue color balance"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x001c, "RedBalance", N_("Red balance"), N_("Red color balance"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x001d, "FocalLength", N_("FocalLength"), N_("FocalLength"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printFocalLength},
    {0x001e, "DigitalZoom", N_("Digital zoom"), N_("Digital zoom"), IfdId::pentaxId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x001f, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxSaturation)},
    {0x0020, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxContrast)},
    {0x0021, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxSharpness)},
    {0x0022, "Location", N_("Location"), N_("Location"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxLocation)},
    {0x0023, "Hometown", N_("Hometown"), N_("Home town"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxCities)},
    {0x0024, "Destination", N_("Destination"), N_("Destination"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_TAG(pentaxCities)},
    {0x0025, "HometownDST", N_("Hometown DST"), N_("Whether day saving time is active in home town"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(pentaxYesNo)},
    {0x0026, "DestinationDST", N_("Destination DST"), N_("Whether day saving time is active in destination"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, EXV_PRINT_TAG(pentaxYesNo)},
    {0x0027, "DSPFirmwareVersion", N_("DSPFirmwareVersion"), N_("DSPFirmwareVersion"), IfdId::pentaxId,
     SectionId::makerTags, unsignedByte, -1, printValue}, /* TODO: Decoding missing */
    {0x0028, "CPUFirmwareVersion", N_("CPUFirmwareVersion"), N_("CPUFirmwareVersion"), IfdId::pentaxId,
     SectionId::makerTags, unsignedByte, -1, printValue}, /* TODO: Decoding missing */
    {0x0029, "FrameNumber", N_("Frame number"), N_("Frame number"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, printValue},
    /* Some missing ! */
    {0x002d, "EffectiveLV", N_("Light value"), N_("Camera calculated light value, includes exposure compensation"),
     IfdId::pentaxId, SectionId::makerTags, unsignedShort, -1, printValue},
    /* Some missing ! */
    {0x0032, "ImageProcessing", N_("Image processing"), N_("Image processing"), IfdId::pentaxId, SectionId::makerTags,
     undefined, -1, EXV_PRINT_COMBITAG(pentaxImageProcessing, 4, 0)},
    {0x0033, "PictureMode", N_("Picture mode"), N_("Picture mode"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, EXV_PRINT_COMBITAG(pentaxPictureMode, 3, 0)},
    {0x0034, "DriveMode", N_("Drive mode"), N_("Drive mode"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     EXV_PRINT_COMBITAG(pentaxDriveMode, 4, 0)},
    /* Some missing ! */
    {0x0037, "ColorSpace", N_("Color space"), N_("Color space"), IfdId::pentaxId, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(pentaxColorSpace)},
    {0x0038, "ImageAreaOffset", N_("Image area offset"), N_("Image area offset"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x0039, "RawImageSize", N_("Raw image size"), N_("Raw image size"), IfdId::pentaxId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    /* Some missing ! */
    {0x003e, "PreviewImageBorders", N_("Preview image borders"), N_("Preview image borders"), IfdId::pentaxId,
     SectionId::makerTags, unsignedByte, -1, printValue},
    {0x003f, "LensType", N_("Lens type"), N_("Lens type"), IfdId::pentaxId, SectionId::makerTags, unsignedByte, -1,
     printLensType},  // #816
    {0x0040, "SensitivityAdjust", N_("Sensitivity adjust"), N_("Sensitivity adjust"), IfdId::pentaxId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0041, "DigitalFilter", N_("Digital filter"), N_("Digital filter"), IfdId::pentaxId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(pentaxOffOn)},
    /* Some missing ! */
    {0x0047, "Temperature", N_("Temperature"), N_("Camera temperature"), IfdId::pentaxId, SectionId::makerTags,
     signedByte, -1, printTemperature},
    {0x0048, "AELock", N_("AE lock"), N_("AE lock"), IfdId::pentaxId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(pentaxOffOn)},
    {0x0049, "NoiseReduction", N_("Noise reduction"), N_("Noise reduction"), IfdId::pentaxId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(pentaxOffOn)},
    /* Some missing ! */
    {0x004d, "FlashExposureCompensation", N_("Flash exposure compensation"), N_("Flash exposure compensation"),
     IfdId::pentaxId, SectionId::makerTags, signedLong, -1, printFlashCompensation},
    /* Some missing ! */
    {0x004f, "ImageTone", N_("Image tone"), N_("Image tone"), IfdId::pentaxId, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(pentaxImageTone)},
    {0x0050, "ColorTemperature", N_("Color temperature"), N_("Color temperature"), IfdId::pentaxId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    /* Some missing ! */
    {0x005c, "ShakeReduction", N_("Shake reduction"), N_("Shake reduction information"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue},
    {0x005d, "ShutterCount", N_("Shutter count"), N_("Shutter count"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, printShutterCount},
    {0x0069, "DynamicRangeExpansion", N_("Dynamic range expansion"), N_("Dynamic range expansion"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, EXV_PRINT_COMBITAG(pentaxDynamicRangeExpansion, 4, 0)},
    {0x0071, "HighISONoiseReduction", N_("High ISO noise reduction"), N_("High ISO noise reduction"), IfdId::pentaxId,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(pentaxHighISONoiseReduction)},
    {0x0072, "AFAdjustment", N_("AF Adjustment"), N_("AF Adjustment"), IfdId::pentaxId, SectionId::makerTags, undefined,
     -1, printValue},
    /* Many missing ! */
    {0x0200, "BlackPoint", N_("Black point"), N_("Black point"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue},
    {0x0201, "WhitePoint", N_("White point"), N_("White point"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue},
    /* Some missing ! */
    {0x0205, "ShotInfo", N_("ShotInfo"), N_("ShotInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0206, "AEInfo", N_("AEInfo"), N_("AEInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0207, "LensInfo", N_("LensInfo"), N_("LensInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0208, "FlashInfo", N_("FlashInfo"), N_("FlashInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0209, "AEMeteringSegments", N_("AEMeteringSegments"), N_("AEMeteringSegments"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x020a, "FlashADump", N_("FlashADump"), N_("FlashADump"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x020b, "FlashBDump", N_("FlashBDump"), N_("FlashBDump"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    /* Some missing ! */
    {0x020d, "WB_RGGBLevelsDaylight", N_("WB_RGGBLevelsDaylight"), N_("WB_RGGBLevelsDaylight"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x020e, "WB_RGGBLevelsShade", N_("WB_RGGBLevelsShade"), N_("WB_RGGBLevelsShade"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x020f, "WB_RGGBLevelsCloudy", N_("WB_RGGBLevelsCloudy"), N_("WB_RGGBLevelsCloudy"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0210, "WB_RGGBLevelsTungsten", N_("WB_RGGBLevelsTungsten"), N_("WB_RGGBLevelsTungsten"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0211, "WB_RGGBLevelsFluorescentD", N_("WB_RGGBLevelsFluorescentD"), N_("WB_RGGBLevelsFluorescentD"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0212, "WB_RGGBLevelsFluorescentN", N_("WB_RGGBLevelsFluorescentN"), N_("WB_RGGBLevelsFluorescentN"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0213, "WB_RGGBLevelsFluorescentW", N_("WB_RGGBLevelsFluorescentW"), N_("WB_RGGBLevelsFluorescentW"),
     IfdId::pentaxId, SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0214, "WB_RGGBLevelsFlash", N_("WB_RGGBLevelsFlash"), N_("WB_RGGBLevelsFlash"), IfdId::pentaxId,
     SectionId::makerTags, undefined, -1, printValue}, /* TODO: Decoding missing */
    {0x0215, "CameraInfo", N_("CameraInfo"), N_("CameraInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0216, "BatteryInfo", N_("BatteryInfo"), N_("BatteryInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x021f, "AFInfo", N_("AFInfo"), N_("AFInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0222, "ColorInfo", N_("ColorInfo"), N_("ColorInfo"), IfdId::pentaxId, SectionId::makerTags, undefined, -1,
     printValue}, /* TODO: Decoding missing */
    {0x0229, "SerialNumber", N_("Serial Number"), N_("Serial Number"), IfdId::pentaxId, SectionId::makerTags,
     asciiString, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownPentaxMakerNoteTag)", "(UnknownPentaxMakerNoteTag)", N_("Unknown PentaxMakerNote tag"),
     IfdId::pentaxId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* PentaxMakerNote::tagList() {
  return tagInfo_;
}

}  // namespace Exiv2::Internal
