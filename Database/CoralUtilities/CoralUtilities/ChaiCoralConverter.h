/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ChaiCoralConverter.h
 *
 * @brief Converts between CHAI payload containers and CORAL objects
 */

#ifndef CORALUTILITIES_CHAICORALCONVERTER_H
#define CORALUTILITIES_CHAICORALCONVERTER_H

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include <chai/ContainerBase.h>

#include <memory>
#include <string>
#include <vector>

namespace coral {
  class AttributeList;
  class AttributeListSpecification;
}

namespace chai {
  class Container;
  class FieldSpec;
  class VectorContainer;
}

/**
 * @class ChaiCoralConverter
 *
 * @brief Converts between CHAI payload containers and CORAL objects: a
 *        specification plus per-channel rows, both directions.
 *
 * IOV is never read or written, the caller owns that. Container-level
 * conversions only see populated channels (a Values with at least one
 * value pushed). Zero-attribute CORAL specifications are rejected.
 */
class ChaiCoralConverter {
public:
  /// Build a coral::AttributeListSpecification from a CHAI FieldSpec, one
  /// attribute per field in field order. See the type table above
  /// toCoralSpec()'s definition for the chai::Type <-> CORAL type mapping.
  /// @param fields - the field specification to convert
  /// @return A new specification. CORAL refcounts specifications so the
  ///         caller must release it
  /// @throws std::runtime_error on an unrecognised chai::Type.
  static coral::AttributeListSpecification* toCoralSpec(const chai::FieldSpec& fields);

  /// Build a chai::FieldSpec from a coral::AttributeListSpecification, one
  /// field per attribute in spec order. See the type table above
  /// toCoralSpec()'s definition for the chai::Type <-> CORAL type mapping.
  /// @param spec - the CORAL specification to convert
  /// @return An equivalent CHAI field specification
  /// @throws std::runtime_error if an attribute's CORAL type has no CHAI
  ///         equivalent
  static chai::FieldSpec toFieldSpec(const coral::AttributeListSpecification& spec);

  /// Convert one channel's CHAI Values to a coral::AttributeList, one
  /// attribute per field in spec order. Null fields map to a null
  /// coral::Attribute. Fields are resolved by name, so a payload whose
  /// fields are ordered differently than the specification still maps
  /// correctly.
  /// @param spec - the CORAL specification the values are converted against
  /// @param values - the CHAI row to convert
  /// @return A CORAL attribute list built from the given specification.
  /// @throws std::runtime_error if the field counts of spec and values
  ///         differ, if a field name in the specification is not present
  ///         in values, or on an unrecognised chai::Type.
  static coral::AttributeList toAttributeList(const coral::AttributeListSpecification& spec,
                                              const chai::Values& values);

  /// Convert one channel's CHAI vector-payload rows to a coral::AttributeList
  /// per row, in row order, via toAttributeList().
  /// @param spec - the CORAL specification every row is converted against
  /// @param rows - the CHAI rows to convert
  /// @return One CORAL attribute list per row, in row order.
  /// @throws std::invalid_argument if a row is null, naming its index.
  /// @throws std::runtime_error under the same conditions as toAttributeList().
  static std::vector<coral::AttributeList> toAttributeListVec(const coral::AttributeListSpecification& spec,
                                                              const std::vector<chai::ValuesPtr>& rows);

  /// Convert one coral::AttributeList to CHAI Values, one value per field in
  /// FieldSpec order. Null attributes map to a null CHAI value. Fields are
  /// resolved by name, so an attribute list whose fields are ordered
  /// differently than the FieldSpec still maps correctly.
  /// @param fields - the field specification the row shares with its siblings
  /// @param attrList - the CORAL row to convert
  /// @return A CHAI row built against the given field specification.
  /// @throws std::invalid_argument if fields is null.
  /// @throws std::runtime_error if the field counts of fields and attrList
  ///         differ, if a field name in fields is not present in attrList,
  ///         if a field's CORAL type doesn't match its chai::Type, or on an
  ///         unrecognised CORAL type.
  static chai::ValuesPtr toValues(chai::FieldSpecPtr fields, const coral::AttributeList& attrList);

  /// Convert a coral::AttributeList per row, in row order, to CHAI Values
  /// sharing one FieldSpecPtr, via toValues().
  /// @param fields - the field specification every row is converted against
  /// @param rows - the CORAL rows to convert
  /// @return One CHAI row per input row, in row order.
  /// @throws std::invalid_argument if fields is null.
  /// @throws std::runtime_error under the same conditions as toValues().
  static std::vector<chai::ValuesPtr> toValuesVec(chai::FieldSpecPtr fields,
                                                  const std::vector<coral::AttributeList>& rows);

  /// Convert a chai::Container's single populated channel to an AthenaAttributeList.
  /// @param container - the container to convert, with only one populated channel
  /// @return A new AthenaAttributeList built from that channel's row.
  /// @throws std::runtime_error if the container does not have exactly one
  ///         populated channel, or under the same conditions as
  ///         toCoralSpec() or toAttributeList() for that channel's row.
  static std::unique_ptr<AthenaAttributeList> toAthenaAttributeList(const chai::Container& container);

  /// Convert an AthenaAttributeList to a single-channel chai::Container.
  /// @param attrList - the row to convert
  /// @param channelId - the channel id to assign the row to
  /// @param channelName - the channel name to assign
  /// @return A new container with one populated channel.
  /// @throws std::runtime_error if attrList's specification has zero
  ///         attributes, or under the same conditions as toFieldSpec()
  ///         or toValues().
  static chai::Container toContainer(const AthenaAttributeList& attrList, uint64_t channelId = 0,
                                     const std::string& channelName = "");

  /// Convert a chai::Container to a CondAttrListCollection, one row per
  /// populated channel. Named channels get their name added.
  /// @param container - the container to convert
  /// @param hasRunLumiBlockTime - passed through to CondAttrListCollection's constructor
  /// @return A new CondAttrListCollection.
  /// @throws std::runtime_error if a populated channel id exceeds
  ///         CondAttrListCollection::ChanNum's range, or under the same
  ///         conditions as toCoralSpec() or toAttributeList() for any
  ///         channel's row.
  static std::unique_ptr<CondAttrListCollection> toCondAttrListCollection(const chai::Container& container,
                                                                          bool hasRunLumiBlockTime);

  /// Convert a CondAttrListCollection to a chai::Container, one channel per
  /// entry with an attribute list. Named channels keep their name.
  /// @param collection - the collection to convert
  /// @return A new Container.
  /// @throws std::runtime_error if the collection is empty, if its shared
  ///         specification has zero attributes, or under the same
  ///         conditions as toFieldSpec() or toValues() for any channel.
  static chai::Container toContainer(const CondAttrListCollection& collection);

  /// Total byte size of one channel's field values, using the same type
  /// mapping as toAttributeList(). Fixed-width for scalar types; the actual
  /// length for String/Blob. Null fields count as zero bytes.
  static unsigned int valuesSize(const chai::Values& values);

  /// Total byte size of every populated channel in a scalar payload.
  static unsigned int payloadSize(const chai::Container& container);

  /// Total byte size of every row of every populated channel in a vector payload.
  /// @throws std::invalid_argument if a row is null, naming its channel and index.
  static unsigned int payloadSize(const chai::VectorContainer& container);
};

#endif  // CORALUTILITIES_CHAICORALCONVERTER_H
