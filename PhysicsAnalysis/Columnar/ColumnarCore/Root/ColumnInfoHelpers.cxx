/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarCore/ColumnInfoHelpers.h>

#include <CxxUtils/crc64.h>

#include <boost/core/demangle.hpp>

#include <stdexcept>

//
// method implementations
//

namespace columnar
{
  void addColumnAccessMode (ColumnInfo& info, ColumnAccessMode accessMode)
  {
    if (info.accessMode == ColumnAccessMode::input)
      info.accessMode = accessMode;
    else if (info.accessMode != accessMode)
      throw std::runtime_error ("conflicting access modes for column " + info.name);
  }



  void mergeColumnInfo (ColumnInfo& target, const ColumnInfo& source)
  {
    if (target.name != source.name)
      throw std::runtime_error ("mismatched column names in mergeColumnInfo: " + target.name + " and " + source.name);

    if (target.index != source.index)
      throw std::runtime_error ("mismatched column index in mergeColumnInfo: " + std::to_string(target.index) + " and " + std::to_string(source.index));

    if (target.type == nullptr || source.type == nullptr)
      throw std::runtime_error ("missing type information in mergeColumnInfo for column: " + target.name);
    if (*target.type != *source.type)
      throw std::runtime_error ("mismatched column types in mergeColumnInfo for column: " + target.name + ": " + boost::core::demangle(target.type->name()) + " and " + boost::core::demangle(source.type->name()));
    
    addColumnAccessMode (target, source.accessMode);

    if (target.offsetName != source.offsetName)
      throw std::runtime_error ("mismatched offset names in mergeColumnInfo for column: " + target.name + ": " + target.offsetName + " and " + source.offsetName);

    if (target.fixedDimensions != source.fixedDimensions)
      throw std::runtime_error ("mismatched fixed dimensions in mergeColumnInfo for column: " + target.name);
    
    if (target.isOffset != source.isOffset)
      throw std::runtime_error ("mismatched isOffset in mergeColumnInfo for column: " + target.name);
    
    if (target.replacesColumn.empty())
      target.replacesColumn = source.replacesColumn;
    else if (!source.replacesColumn.empty() && target.replacesColumn != source.replacesColumn)
      throw std::runtime_error ("mismatched replacesColumn in mergeColumnInfo for column: " + target.name + ": " + target.replacesColumn + " and " + source.replacesColumn);

    if (!source.isOptional)
      target.isOptional = false;

    if (target.soleLinkTargetName != source.soleLinkTargetName)
      throw std::runtime_error ("mismatched soleLinkTargetName in mergeColumnInfo for column: " + target.name);

    if (target.soleLinkTargetClid != source.soleLinkTargetClid)
      throw std::runtime_error ("mismatched soleLinkTargetClid in mergeColumnInfo for column: " + target.name);

    if (target.isVariantLink != source.isVariantLink)
      throw std::runtime_error ("mismatched isVariantLink in mergeColumnInfo for column: " + target.name);

    if (target.variantLinkTargetNames != source.variantLinkTargetNames)
      throw std::runtime_error ("mismatched variantLinkTargetNames in mergeColumnInfo for column: " + target.name);

    if (target.keyColumnForVariantLink != source.keyColumnForVariantLink)
      throw std::runtime_error ("mismatched keyColumnForVariantLink in mergeColumnInfo for column: " + target.name + ": " + target.keyColumnForVariantLink + " and " + source.keyColumnForVariantLink);
  }


  SG::sgkey_t computeSgKey (const std::string& name, std::uint32_t clid)
  {
    std::uint64_t crc = CxxUtils::crc64 (name);
    if (clid != 0)
      crc = CxxUtils::crc64addint (crc, clid);
    // mask to SG::StringPool::sgkey_t_nbits (30) bits
    return static_cast<SG::sgkey_t> (crc & ((static_cast<std::uint64_t> (1) << 30) - 1));
  }
}
