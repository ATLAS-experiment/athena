/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMN_INFO_HELPERS_H
#define COLUMNAR_CORE_COLUMN_INFO_HELPERS_H

#include <ColumnarInterfaces/ColumnInfo.h>
#include <SGCore/sgkey_t.h>

#include <cstdint>
#include <string>

namespace columnar
{
  void addColumnAccessMode (ColumnInfo& info, ColumnAccessMode accessMode);

  void mergeColumnInfo (ColumnInfo& target, const ColumnInfo& source);

  /// @brief compute the StoreGate hashed key for a container
  ///
  /// This reproduces SG::StringPool::stringToKey (SGTools), which is
  /// how Athena computes the keys stored as `m_persKey` for
  /// persistified element links: the crc64 of the container name,
  /// extended with the container CLID, masked to 30 bits. SGTools
  /// itself is not available in AnalysisBase-style releases, hence
  /// this local implementation on top of CxxUtils.
  ///
  /// Together with @ref ColumnInfo::soleLinkTargetClid this allows
  /// computing the expected `m_persKey` for a link column's target at
  /// runtime instead of hard-coding the values.
  [[nodiscard]] SG::sgkey_t computeSgKey (const std::string& name, std::uint32_t clid);
}

#endif
