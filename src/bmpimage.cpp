// SPDX-License-Identifier: GPL-2.0-or-later
/*
  File:      bmpimage.cpp
  Author(s): Marco Piovanelli, Ovolab (marco)
  History:   05-Mar-2007, marco: created
 */

#include "bmpimage.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"

// + standard includes
#include <array>
#include <string>

#ifdef EXIV2_DEBUG_MESSAGES
#include <iostream>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {
BmpImage::BmpImage(BasicIo::UniquePtr io) : Image(ImageType::bmp, mdNone, std::move(io)) {
}

std::string BmpImage::mimeType() const {
  // "image/bmp" is a Generic Bitmap
  return "image/x-ms-bmp";  // Microsoft Bitmap
}

void BmpImage::setExifData(const ExifData& /*exifData*/) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "BMP"));
}

void BmpImage::setIptcData(const IptcData& /*iptcData*/) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "BMP"));
}

void BmpImage::setComment(const std::string&) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "BMP"));
}

void BmpImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::BmpImage::readMetadata: Reading Windows bitmap file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);

  // Ensure that this is the correct image type
  if (!isBmpType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "BMP");
  }
  clearMetadata();

  /*
    The Windows bitmap header goes as follows -- all numbers are in little-endian byte order:

    offset  length   name                   description
    ======  =======  =====================  =======
     0      2 bytes  signature              always 'BM'
     2      4 bytes  bitmap size
     6      4 bytes  reserved
    10      4 bytes  bitmap offset
    14      4 bytes  header size
    18      4 bytes  bitmap width
    22      4 bytes  bitmap height
    26      2 bytes  plane count
    28      2 bytes  depth
    30      4 bytes  compression            0 = none; 1 = RLE, 8 bits/pixel; 2 = RLE, 4 bits/pixel; 3 = bitfield;
    4 = JPEG; 5 = PNG 34      4 bytes  image size             size of the raw bitmap data, in bytes 38      4
    bytes  horizontal resolution  (in pixels per meter) 42      4 bytes  vertical resolution    (in pixels per
    meter) 46      4 bytes  color count 50      4 bytes  important colors       number of "important" colors
  */
  byte buf[26];
  if (io_->read(buf, sizeof(buf)) == sizeof(buf)) {
    pixelWidth_ = getULong(buf + 18, littleEndian);
    pixelHeight_ = getULong(buf + 22, littleEndian);
  }
}

void BmpImage::writeMetadata() {
  /// \todo implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "BMP"));
}

// *************************************************************************
// free functions
Image::UniquePtr newBmpInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<BmpImage>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isBmpType(BasicIo& iIo, bool advance) {
  const int32_t len = 2;
  const std::array<byte, len> BmpImageId{'B', 'M'};
  std::array<byte, len> buf;
  iIo.read(buf.data(), len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  bool matched = buf == BmpImageId;
  if (!advance || !matched) {
    iIo.seek(-len, BasicIo::cur);
  }
  return matched;
}
}  // namespace Exiv2
