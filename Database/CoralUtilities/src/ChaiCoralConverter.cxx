/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CoralUtilities/ChaiCoralConverter.h"

#include <chai/Container.h>
#include <chai/VectorContainer.h>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"

#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace {
  // Releases a CORAL specification on scope exit, so a throw partway through
  // building a CondAttrListCollection or AthenaAttributeList doesn't leak it
  struct SpecReleaser {
    void operator()(coral::AttributeListSpecification* spec) const {
      if (spec) {
        spec->release();
      }
    }
  };
  using SpecHolder = std::unique_ptr<coral::AttributeListSpecification, SpecReleaser>;

  // CORAL C++ type -> chai::Type, the reverse of the table below
  // Done as a lookup table because it uses typeid
  chai::Type chaiTypeFor(const coral::AttributeSpecification& spec) {
    static const std::unordered_map<std::type_index, chai::Type> table{
      {std::type_index(typeid(bool)), chai::Bool},
      {std::type_index(typeid(char)), chai::Int8},
      {std::type_index(typeid(unsigned char)), chai::UInt8},
      {std::type_index(typeid(short)), chai::Int16},
      {std::type_index(typeid(unsigned short)), chai::UInt16},
      {std::type_index(typeid(int)), chai::Int32},
      {std::type_index(typeid(unsigned int)), chai::UInt32},
      {std::type_index(typeid(long long)), chai::Int64},
      {std::type_index(typeid(unsigned long long)), chai::UInt64},
      {std::type_index(typeid(float)), chai::Float},
      {std::type_index(typeid(double)), chai::Double},
      {std::type_index(typeid(std::string)), chai::String},
      {std::type_index(typeid(coral::Blob)), chai::Blob},
    };
    const auto it = table.find(std::type_index(spec.type()));
    if (it == table.end()) {
      throw std::runtime_error(
        "ChaiCoralConverter: unsupported CORAL type '" + spec.typeName() + "' for field '" + spec.name() + "'");
    }
    return it->second;
  }
}

//   chai::Type       Coral C++ type          CHAI fixed-width type
//
//   Bool             bool                    bool
//   Int8 / UInt8     char / unsigned char    int8_t / uint8_t
//   Int16 / UInt16   short / unsigned short  int16_t / uint16_t
//   Int32 / UInt32   int / unsigned int      int32_t / uint32_t
//   Int64 / UInt64   long long / ull         int64_t / uint64_t
//   Float / Double   float / double          float / double
//   String           std::string             std::string
//   Blob             coral::Blob (byte copy) chai::BlobData
coral::AttributeListSpecification* ChaiCoralConverter::toCoralSpec(const chai::FieldSpec& fields) {
  auto* pSpec = new coral::AttributeListSpecification();
  try {
    for (const auto& field : fields.fields()) {
      switch (field.type) {
        case chai::Bool:   pSpec->extend(field.name, typeid(bool)); break;
        case chai::Int8:   pSpec->extend(field.name, typeid(char)); break;
        case chai::UInt8:  pSpec->extend(field.name, typeid(unsigned char)); break;
        case chai::Int16:  pSpec->extend(field.name, typeid(short)); break;
        case chai::UInt16: pSpec->extend(field.name, typeid(unsigned short)); break;
        case chai::Int32:  pSpec->extend(field.name, typeid(int)); break;
        case chai::UInt32: pSpec->extend(field.name, typeid(unsigned int)); break;
        case chai::Int64:  pSpec->extend(field.name, typeid(long long)); break;
        case chai::UInt64: pSpec->extend(field.name, typeid(unsigned long long)); break;
        case chai::Float:  pSpec->extend(field.name, typeid(float)); break;
        case chai::Double: pSpec->extend(field.name, typeid(double)); break;
        case chai::String: pSpec->extend(field.name, typeid(std::string)); break;
        case chai::Blob:   pSpec->extend(field.name, typeid(coral::Blob)); break;
        case chai::Unknown: // Fall-through
        default:
          throw std::runtime_error(
            "ChaiCoralConverter::toCoralSpec: unrecognised CHAI type for field '" + field.name + "'");
      }
    }
  } catch (...) {
    pSpec->release();
    throw;
  }
  return pSpec;
}

chai::FieldSpec ChaiCoralConverter::toFieldSpec(const coral::AttributeListSpecification& spec) {
  std::vector<chai::Field> fields;
  fields.reserve(spec.size());
  const unsigned int s = spec.size();
  for (unsigned int i = 0; i != s; ++i) {
    const auto& attrSpec = spec[i];
    fields.push_back({attrSpec.name(), chaiTypeFor(attrSpec)});
  }
  return chai::FieldSpec(std::move(fields));
}

coral::AttributeList ChaiCoralConverter::toAttributeList(const coral::AttributeListSpecification& spec,
                                                         const chai::Values& values) {
  // The specification is built from a tag's first payload, so a later
  // payload with a different field count is caught here
  if (values.size() != spec.size()) {
    throw std::runtime_error("ChaiCoralConverter::toAttributeList: field count mismatch, payload "
                             + std::to_string(values.size()) + " vs specification " + std::to_string(spec.size()));
  }
  coral::AttributeList attr(spec, true);
  const unsigned int s = attr.size();
  for (unsigned int i = 0; i != s; ++i) {
    auto& att = attr[i];
    // Look up each value by name rather than position
    const std::string& fieldName = att.specification().name();
    const std::size_t idx = values.fieldIndex(fieldName);
    if (values.isNull(idx)) {
      att.setNull();
      continue;
    }
    switch (values.type(idx)) {
      case chai::Bool:
        att.setValue<bool>(values.get<bool>(idx));
        break;
      case chai::Int8:
        att.setValue<char>(values.get<int8_t>(idx));
        break;
      case chai::UInt8:
        att.setValue<unsigned char>(values.get<uint8_t>(idx));
        break;
      case chai::Int16:
        att.setValue<short>(values.get<int16_t>(idx));
        break;
      case chai::UInt16:
        att.setValue<unsigned short>(values.get<uint16_t>(idx));
        break;
      case chai::Int32:
        att.setValue<int>(values.get<int32_t>(idx));
        break;
      case chai::UInt32:
        att.setValue<unsigned int>(values.get<uint32_t>(idx));
        break;
      case chai::Int64:
        att.setValue<long long>(values.get<int64_t>(idx));
        break;
      case chai::UInt64:
        att.setValue<unsigned long long>(values.get<uint64_t>(idx));
        break;
      case chai::Float:
        att.setValue<float>(values.get<float>(idx));
        break;
      case chai::Double:
        att.setValue<double>(values.get<double>(idx));
        break;
      case chai::String:
        att.setValue<std::string>(values.get<std::string>(idx));
        break;
      case chai::Blob: {
        const auto& bytes = values.get<chai::BlobData>(idx).m_bytes;
        coral::Blob blob(bytes.size());
        // Skip the memcpy if the blob is empty
        if (!bytes.empty()) {
          std::memcpy(blob.startingAddress(), bytes.data(), bytes.size());
        }
        att.setValue<coral::Blob>(blob);
        break;
      }
      case chai::Unknown: // Fall-through
      default:
        throw std::runtime_error(
          "ChaiCoralConverter::toAttributeList: unrecognised CHAI type for field '" + fieldName + "'");
    }
  }
  return attr;
}

std::vector<coral::AttributeList> ChaiCoralConverter::toAttributeListVec(const coral::AttributeListSpecification& spec,
                                                                         const std::vector<chai::ValuesPtr>& rows) {
  std::vector<coral::AttributeList> result;
  result.reserve(rows.size());
  for (std::size_t i = 0; i != rows.size(); ++i) {
    if (!rows[i]) {
      throw std::invalid_argument(
        "ChaiCoralConverter::toAttributeListVec: null row at index " + std::to_string(i));
    }
    result.push_back(toAttributeList(spec, *rows[i]));
  }
  return result;
}

chai::ValuesPtr ChaiCoralConverter::toValues(chai::FieldSpecPtr fields, const coral::AttributeList& attrList) {
  if (!fields) {
    throw std::invalid_argument("ChaiCoralConverter::toValues: null field specification");
  }
  if (fields->size() != attrList.size()) {
    throw std::runtime_error("ChaiCoralConverter::toValues: field count mismatch, specification "
                             + std::to_string(fields->size()) + " vs payload " + std::to_string(attrList.size()));
  }
  auto values = std::make_shared<chai::Values>(fields);
  const std::size_t n = fields->size();
  for (std::size_t i = 0; i != n; ++i) {
    const chai::Field& field = (*fields)[i];
    const int idx = attrList.specification().index(field.name);
    if (idx < 0) {
      throw std::runtime_error(
        "ChaiCoralConverter::toValues: field '" + field.name + "' not found in payload");
    }
    const auto& att = attrList[static_cast<unsigned int>(idx)];
    const chai::Type payloadType = chaiTypeFor(att.specification());
    if (payloadType != field.type) {
      throw std::runtime_error(
        "ChaiCoralConverter::toValues: type mismatch for field '" + field.name + "': specification "
        + field.typeString() + " vs payload " + chai::typeToString(payloadType));
    }
    if (att.isNull()) {
      values->push();
      continue;
    }
    // Have to cast to the exact fixed-width types for integers since push is templated
    switch (field.type) {
      case chai::Bool:
        values->push(att.data<bool>());
        break;
      case chai::Int8:
        values->push(static_cast<int8_t>(att.data<char>()));
        break;
      case chai::UInt8:
        values->push(static_cast<uint8_t>(att.data<unsigned char>()));
        break;
      case chai::Int16:
        values->push(static_cast<int16_t>(att.data<short>()));
        break;
      case chai::UInt16:
        values->push(static_cast<uint16_t>(att.data<unsigned short>()));
        break;
      case chai::Int32:
        values->push(static_cast<int32_t>(att.data<int>()));
        break;
      case chai::UInt32:
        values->push(static_cast<uint32_t>(att.data<unsigned int>()));
        break;
      case chai::Int64:
        values->push(static_cast<int64_t>(att.data<long long>()));
        break;
      case chai::UInt64:
        values->push(static_cast<uint64_t>(att.data<unsigned long long>()));
        break;
      case chai::Float:
        values->push(att.data<float>());
        break;
      case chai::Double:
        values->push(att.data<double>());
        break;
      case chai::String:
        values->push(att.data<std::string>());
        break;
      case chai::Blob: {
        const coral::Blob& blob = att.data<coral::Blob>();
        std::vector<uint8_t> bytes(blob.size());
        // Skip the memcpy if the blob is empty
        if (blob.size() != 0) {
          std::memcpy(bytes.data(), blob.startingAddress(), blob.size());
        }
        values->push(chai::BlobData(std::move(bytes)));
        break;
      }
      case chai::Unknown: // Fall-through
      default:
        throw std::runtime_error(
          "ChaiCoralConverter::toValues: unrecognised CHAI type for field '" + field.name + "'");
    }
  }
  return values;
}

std::vector<chai::ValuesPtr> ChaiCoralConverter::toValuesVec(chai::FieldSpecPtr fields,
                                                             const std::vector<coral::AttributeList>& rows) {
  if (!fields) {
    throw std::invalid_argument("ChaiCoralConverter::toValuesVec: null field specification");
  }
  std::vector<chai::ValuesPtr> result;
  result.reserve(rows.size());
  for (std::size_t i = 0; i != rows.size(); ++i) {
    result.push_back(toValues(fields, rows[i]));
  }
  return result;
}

std::unique_ptr<AthenaAttributeList> ChaiCoralConverter::toAthenaAttributeList(const chai::Container& container) {
  const auto ids = container.channelIds();
  if (ids.size() != 1) {
    throw std::runtime_error(
      "ChaiCoralConverter::toAthenaAttributeList: expected exactly one populated channel, found "
      + std::to_string(ids.size()));
  }
  const SpecHolder spec(toCoralSpec(container.fieldSpec()));
  return std::make_unique<AthenaAttributeList>(toAttributeList(*spec, container[ids[0]]));
}

chai::Container ChaiCoralConverter::toContainer(const AthenaAttributeList& attrList, uint64_t channelId,
                                                const std::string& channelName) {
  const coral::AttributeListSpecification& coralSpec = attrList.specification();
  if (coralSpec.size() == 0) {
    throw std::runtime_error("ChaiCoralConverter::toContainer: zero-attribute specification");
  }
  const auto fields = std::make_shared<const chai::FieldSpec>(toFieldSpec(coralSpec));
  chai::Container container(chai::PayloadSpec(*fields, chai::ChannelSpec({{channelId, channelName}})));
  container.add(channelId, toValues(fields, attrList));
  return container;
}

std::unique_ptr<CondAttrListCollection> ChaiCoralConverter::toCondAttrListCollection(
    const chai::Container& container, bool hasRunLumiBlockTime) {
  const SpecHolder spec(toCoralSpec(container.fieldSpec()));
  auto collection = std::make_unique<CondAttrListCollection>(hasRunLumiBlockTime);
  const auto names = container.channelSpec().channelNames();
  for (const auto id : container.channelIds()) {
    // Catch IDs too wide for ChanNum before the cast truncates them
    if (id > std::numeric_limits<CondAttrListCollection::ChanNum>::max()) {
      throw std::runtime_error(
        "ChaiCoralConverter::toCondAttrListCollection: channel id " + std::to_string(id)
        + " exceeds ChanNum range");
    }
    const auto chan = static_cast<CondAttrListCollection::ChanNum>(id);
    collection->addShared(chan, toAttributeList(*spec, container[id]));
    const auto nameIt = names.find(id);
    if (nameIt != names.end() && !nameIt->second.empty()) {
      collection->add(chan, nameIt->second);
    }
  }
  return collection;
}

chai::Container ChaiCoralConverter::toContainer(const CondAttrListCollection& collection) {
  if (collection.size() == 0) {
    throw std::runtime_error("ChaiCoralConverter::toContainer: empty collection");
  }
  const coral::AttributeListSpecification& coralSpec = collection.begin()->second.specification();
  if (coralSpec.size() == 0) {
    throw std::runtime_error("ChaiCoralConverter::toContainer: zero-attribute specification");
  }
  // Container fixes its channel set at construction, so names are collected before building it
  std::map<uint64_t, std::string> channelNames;
  for (const auto& pair : collection) {
    channelNames[static_cast<uint64_t>(pair.first)] = collection.chanName(pair.first);
  }
  const auto fields = std::make_shared<const chai::FieldSpec>(toFieldSpec(coralSpec));
  chai::Container container(chai::PayloadSpec(*fields, chai::ChannelSpec(channelNames)));
  for (const auto& [chan, attrList] : collection) {
    const uint64_t id = chan;
    container.add(id, toValues(fields, attrList));
  }
  return container;
}

unsigned int ChaiCoralConverter::valuesSize(const chai::Values& values) {
  unsigned int total = 0;
  const std::size_t n = values.size();
  for (std::size_t i = 0; i != n; ++i) {
    if (values.isNull(i)) continue;
    switch (values.type(i)) {
      case chai::Bool:   total += sizeof(bool);     break;
      case chai::Int8:   total += sizeof(int8_t);   break;
      case chai::UInt8:  total += sizeof(uint8_t);  break;
      case chai::Int16:  total += sizeof(int16_t);  break;
      case chai::UInt16: total += sizeof(uint16_t); break;
      case chai::Int32:  total += sizeof(int32_t);  break;
      case chai::UInt32: total += sizeof(uint32_t); break;
      case chai::Int64:  total += sizeof(int64_t);  break;
      case chai::UInt64: total += sizeof(uint64_t); break;
      case chai::Float:  total += sizeof(float);    break;
      case chai::Double: total += sizeof(double);   break;
      case chai::String:
        total += values.get<std::string>(i).size();
        break;
      case chai::Blob:
        total += values.get<chai::BlobData>(i).m_bytes.size();
        break;
      case chai::Unknown: // Fall-through
      default:
        throw std::runtime_error(
          "ChaiCoralConverter::valuesSize: unrecognised CHAI type at field index " + std::to_string(i));
    }
  }
  return total;
}

unsigned int ChaiCoralConverter::payloadSize(const chai::Container& container) {
  unsigned int total = 0;
  for (const auto channelId : container.channelIds()) {
    total += valuesSize(container[channelId]);
  }
  return total;
}

unsigned int ChaiCoralConverter::payloadSize(const chai::VectorContainer& container) {
  unsigned int total = 0;
  for (const auto channelId : container.channelIds()) {
    const auto& rows = container.rows(channelId);
    for (std::size_t i = 0; i != rows.size(); ++i) {
      if (!rows[i]) {
        throw std::invalid_argument(
          "ChaiCoralConverter::payloadSize: null row at channel " + std::to_string(channelId)
          + " index " + std::to_string(i));
      }
      total += valuesSize(*rows[i]);
    }
  }
  return total;
}
