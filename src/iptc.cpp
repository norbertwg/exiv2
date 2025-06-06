// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "iptc.hpp"

#include "datasets.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "image_int.hpp"
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <algorithm>
#include <iostream>

// *****************************************************************************
namespace {
/*!
  @brief Read a single dataset payload and create a new metadata entry.

  @param iptcData IPTC metadata container to add the dataset to
  @param dataSet  DataSet number
  @param record   Record Id
  @param data     Pointer to the first byte of dataset payload
  @param sizeData Length in bytes of dataset payload
  @return 0 if successful.
 */
int readData(Exiv2::IptcData& iptcData, uint16_t dataSet, uint16_t record, const Exiv2::byte* data, uint32_t sizeData);

//! Unary predicate that matches an Iptcdatum with given record and dataset
class FindIptcdatum {
 public:
  //! Constructor, initializes the object with the record and dataset id
  FindIptcdatum(uint16_t dataset, uint16_t record) : dataset_(dataset), record_(record) {
  }
  /*!
    @brief Returns true if the record and dataset id of the argument
          Iptcdatum is equal to that of the object.
  */
  bool operator()(const Exiv2::Iptcdatum& iptcdatum) const {
    return dataset_ == iptcdatum.tag() && record_ == iptcdatum.record();
  }

 private:
  // DATA
  uint16_t dataset_;
  uint16_t record_;

};  // class FindIptcdatum
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
Iptcdatum::Iptcdatum(const IptcKey& key, const Value* pValue) : key_(key.clone()) {
  if (pValue)
    value_ = pValue->clone();
}

Iptcdatum::Iptcdatum(const Iptcdatum& rhs) {
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy
}

size_t Iptcdatum::copy(byte* buf, ByteOrder byteOrder) const {
  return value_ ? value_->copy(buf, byteOrder) : 0;
}

std::ostream& Iptcdatum::write(std::ostream& os, const ExifData*) const {
  return os << value();
}

std::string Iptcdatum::key() const {
  return key_ ? key_->key() : "";
}

std::string Iptcdatum::recordName() const {
  return key_ ? key_->recordName() : "";
}

uint16_t Iptcdatum::record() const {
  return key_ ? key_->record() : 0;
}

const char* Iptcdatum::familyName() const {
  return key_ ? key_->familyName() : "";
}

std::string Iptcdatum::groupName() const {
  return key_ ? key_->groupName() : "";
}

std::string Iptcdatum::tagName() const {
  return key_ ? key_->tagName() : "";
}

std::string Iptcdatum::tagLabel() const {
  return key_ ? key_->tagLabel() : "";
}

std::string Iptcdatum::tagDesc() const {
  return key_ ? key_->tagDesc() : "";
}

uint16_t Iptcdatum::tag() const {
  return key_ ? key_->tag() : 0;
}

TypeId Iptcdatum::typeId() const {
  return value_ ? value_->typeId() : invalidTypeId;
}

const char* Iptcdatum::typeName() const {
  return TypeInfo::typeName(typeId());
}

size_t Iptcdatum::typeSize() const {
  return TypeInfo::typeSize(typeId());
}

size_t Iptcdatum::count() const {
  return value_ ? value_->count() : 0;
}

size_t Iptcdatum::size() const {
  return value_ ? value_->size() : 0;
}

std::string Iptcdatum::toString() const {
  return value_ ? value_->toString() : "";
}

std::string Iptcdatum::toString(size_t n) const {
  return value_ ? value_->toString(n) : "";
}

int64_t Iptcdatum::toInt64(size_t n) const {
  return value_ ? value_->toInt64(n) : -1;
}

float Iptcdatum::toFloat(size_t n) const {
  return value_ ? value_->toFloat(n) : -1;
}

Rational Iptcdatum::toRational(size_t n) const {
  return value_ ? value_->toRational(n) : Rational(-1, 1);
}

Value::UniquePtr Iptcdatum::getValue() const {
  return value_ ? value_->clone() : nullptr;
}

const Value& Iptcdatum::value() const {
  if (!value_)
    throw Error(ErrorCode::kerValueNotSet, key());
  return *value_;
}

Iptcdatum& Iptcdatum::operator=(const Iptcdatum& rhs) {
  if (this == &rhs)
    return *this;

  key_.reset();
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy

  value_.reset();
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy

  return *this;
}  // Iptcdatum::operator=

Iptcdatum& Iptcdatum::operator=(const uint16_t& value) {
  auto v = std::make_unique<UShortValue>();
  v->value_.push_back(value);
  value_ = std::move(v);
  return *this;
}

Iptcdatum& Iptcdatum::operator=(const std::string& value) {
  setValue(value);
  return *this;
}

Iptcdatum& Iptcdatum::operator=(const Value& value) {
  setValue(&value);
  return *this;
}

void Iptcdatum::setValue(const Value* pValue) {
  value_.reset();
  if (pValue)
    value_ = pValue->clone();
}

int Iptcdatum::setValue(const std::string& value) {
  if (!value_) {
    TypeId type = IptcDataSets::dataSetType(tag(), record());
    value_ = Value::create(type);
  }
  return value_->read(value);
}

Iptcdatum& IptcData::operator[](const std::string& key) {
  IptcKey iptcKey(key);
  auto pos = findKey(iptcKey);
  if (pos == end()) {
    return iptcMetadata_.emplace_back(iptcKey);
  }
  return *pos;
}

size_t IptcData::size() const {
  size_t newSize = 0;
  for (auto&& iptc : iptcMetadata_) {
    // marker, record Id, dataset num, first 2 bytes of size
    newSize += 5;
    size_t dataSize = iptc.size();
    newSize += dataSize;
    if (dataSize > 32767) {
      // extended dataset (we always use 4 bytes)
      newSize += 4;
    }
  }
  return newSize;
}

int IptcData::add(const IptcKey& key, const Value* value) {
  return add(Iptcdatum(key, value));
}

int IptcData::add(const Iptcdatum& iptcDatum) {
  if (!IptcDataSets::dataSetRepeatable(iptcDatum.tag(), iptcDatum.record()) &&
      findId(iptcDatum.tag(), iptcDatum.record()) != end()) {
    return 6;
  }
  // allow duplicates
  iptcMetadata_.push_back(iptcDatum);
  return 0;
}

IptcData::const_iterator IptcData::findKey(const IptcKey& key) const {
  return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(), FindIptcdatum(key.tag(), key.record()));
}

IptcData::iterator IptcData::findKey(const IptcKey& key) {
  return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(), FindIptcdatum(key.tag(), key.record()));
}

IptcData::const_iterator IptcData::findId(uint16_t dataset, uint16_t record) const {
  return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(), FindIptcdatum(dataset, record));
}

IptcData::iterator IptcData::findId(uint16_t dataset, uint16_t record) {
  return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(), FindIptcdatum(dataset, record));
}

/// \todo not used internally. At least we should test it
void IptcData::sortByKey() {
  std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpMetadataByKey);
}

/// \todo not used internally. At least we should test it
void IptcData::sortByTag() {
  std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpMetadataByTag);
}

IptcData::iterator IptcData::erase(IptcData::iterator pos) {
  return iptcMetadata_.erase(pos);
}

void IptcData::printStructure(std::ostream& out, const Slice<byte*>& bytes, size_t depth) {
  if (bytes.size() < 3) {
    return;
  }
  size_t i = 0;
  while (i < bytes.size() - 3 && bytes.at(i) != 0x1c)
    i++;
  out << Internal::indent(++depth) << "Record | DataSet | Name                     | Length | Data" << '\n';
  while (i < bytes.size() - 3) {
    if (bytes.at(i) != 0x1c) {
      break;
    }
    uint16_t record = bytes.at(i + 1);
    uint16_t dataset = bytes.at(i + 2);
    Internal::enforce(bytes.size() - i >= 5, ErrorCode::kerCorruptedMetadata);
    uint16_t len = getUShort(bytes.subSlice(i + 3, bytes.size()), bigEndian);

    Internal::enforce(bytes.size() - i >= 5 + static_cast<size_t>(len), ErrorCode::kerCorruptedMetadata);
    out << stringFormat("  {:6} | {:7} | {:<24} | {:6} | ", record, dataset,
                        Exiv2::IptcDataSets::dataSetName(dataset, record), len);
    out << Internal::binaryToString(makeSlice(bytes, i + 5, i + 5 + std::min<uint16_t>(40, len)))
        << (len > 40 ? "...\n" : "\n");
    i += 5 + len;
  }
}

const char* IptcData::detectCharset() const {
  auto pos = findKey(IptcKey("Iptc.Envelope.CharacterSet"));
  if (pos != end()) {
    const std::string value = pos->toString();
    if (pos->value().ok() && value == "\033%G")
      return "UTF-8";
  }

  bool ascii = true;
  bool utf8 = true;

  for (const auto& key : *this) {
    std::string value = key.toString();
    if (key.value().ok()) {
      int seqCount = 0;
      for (auto c : value) {
        if (seqCount) {
          if ((c & 0xc0) != 0x80) {
            utf8 = false;
            break;
          }
          --seqCount;
        } else {
          if (c & 0x80)
            ascii = false;
          else
            continue;  // ascii character

          if ((c & 0xe0) == 0xc0)
            seqCount = 1;
          else if ((c & 0xf0) == 0xe0)
            seqCount = 2;
          else if ((c & 0xf8) == 0xf0)
            seqCount = 3;
          else if ((c & 0xfc) == 0xf8)
            seqCount = 4;
          else if ((c & 0xfe) == 0xfc)
            seqCount = 5;
          else {
            utf8 = false;
            break;
          }
        }
      }
      if (seqCount)
        utf8 = false;  // unterminated seq
      if (!utf8)
        break;
    }
  }

  if (ascii)
    return "ASCII";
  if (utf8)
    return "UTF-8";
  return nullptr;
}

int IptcParser::decode(IptcData& iptcData, const byte* pData, size_t size) {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "IptcParser::decode, size = " << size << "\n";
#endif
  auto pRead = pData;
  const auto pEnd = pData + size;
  iptcData.clear();

  uint16_t record = 0;
  uint16_t dataSet = 0;
  uint32_t sizeData = 0;
  byte extTest = 0;

  while (6 <= static_cast<size_t>(pEnd - pRead)) {
    // First byte should be a marker. If it isn't, scan forward and skip
    // the chunk bytes present in some images. This deviates from the
    // standard, which advises to treat such cases as errors.
    if (*pRead++ != marker_)
      continue;
    record = *pRead++;
    dataSet = *pRead++;

    extTest = *pRead;
    if (extTest & 0x80) {
      // extended dataset
      uint16_t sizeOfSize = (getUShort(pRead, bigEndian) & 0x7FFF);
      if (sizeOfSize > 4)
        return 5;
      pRead += 2;
      if (sizeOfSize > pEnd - pRead)
        return 6;
      sizeData = 0;
      for (; sizeOfSize > 0; --sizeOfSize) {
        sizeData |= *pRead++ << (8 * (sizeOfSize - 1));
      }
    } else {
      // standard dataset
      sizeData = getUShort(pRead, bigEndian);
      pRead += 2;
    }
    if (sizeData <= static_cast<size_t>(pEnd - pRead)) {
      int rc = readData(iptcData, dataSet, record, pRead, sizeData);
      if (rc != 0) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to read IPTC dataset " << IptcKey(dataSet, record) << " (rc = " << rc << "); skipped.\n";
#endif
      }
    } else {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "IPTC dataset " << IptcKey(dataSet, record) << " has invalid size " << sizeData << "; skipped.\n";
#endif
      return 7;
    }
    pRead += sizeData;
  }

  return 0;
}  // IptcParser::decode

DataBuf IptcParser::encode(const IptcData& iptcData) {
  DataBuf buf;
  if (iptcData.empty())
    return buf;

  buf = DataBuf(iptcData.size());
  byte* pWrite = buf.data();

  // Copy the iptc data sets and sort them by record but preserve the order of datasets
  IptcMetadata sortedIptcData(iptcData.begin(), iptcData.end());
  std::stable_sort(sortedIptcData.begin(), sortedIptcData.end(),
                   [](const auto& l, const auto& r) { return l.record() < r.record(); });

  for (const auto& iter : sortedIptcData) {
    // marker, record Id, dataset num
    *pWrite++ = marker_;
    *pWrite++ = static_cast<byte>(iter.record());
    *pWrite++ = static_cast<byte>(iter.tag());

    // extended or standard dataset?
    if (size_t dataSize = iter.size(); dataSize > 32767) {
      // always use 4 bytes for extended length
      uint16_t sizeOfSize = 4 | 0x8000;
      us2Data(pWrite, sizeOfSize, bigEndian);
      pWrite += 2;
      ul2Data(pWrite, static_cast<uint32_t>(dataSize), bigEndian);
      pWrite += 4;
    } else {
      us2Data(pWrite, static_cast<uint16_t>(dataSize), bigEndian);
      pWrite += 2;
    }
    pWrite += iter.value().copy(pWrite, bigEndian);
  }

  return buf;
}  // IptcParser::encode

}  // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {
int readData(Exiv2::IptcData& iptcData, uint16_t dataSet, uint16_t record, const Exiv2::byte* data, uint32_t sizeData) {
  Exiv2::TypeId type = Exiv2::IptcDataSets::dataSetType(dataSet, record);
  auto value = Exiv2::Value::create(type);
  int rc = value->read(data, sizeData, Exiv2::bigEndian);
  if (0 == rc) {
    Exiv2::IptcKey key(dataSet, record);
    iptcData.add(key, value.get());
  } else if (1 == rc) {
    // If the first attempt failed, try with a string value
    value = Exiv2::Value::create(Exiv2::string);
    rc = value->read(data, sizeData, Exiv2::bigEndian);
    if (0 == rc) {
      Exiv2::IptcKey key(dataSet, record);
      iptcData.add(key, value.get());
    }
  }
  return rc;
}

}  // namespace
