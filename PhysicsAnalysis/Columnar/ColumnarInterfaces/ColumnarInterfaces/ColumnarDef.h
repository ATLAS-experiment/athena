/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_COLUMNAR_DEF_H
#define COLUMNAR_INTERFACES_COLUMNAR_DEF_H

#include <string_view>

namespace columnar
{
  /// @brief the type used for the size and offsets in the columnar data
  ///
  /// @todo This type still needs to be adjusted to match whatever uproot
  /// uses for its offset maps.
  using ColumnarOffsetType = std::size_t;

  /// @brief the value for an invalid element index
  ///
  /// This is mostly used for invalid element links
  inline constexpr ColumnarOffsetType invalidObjectIndex = static_cast<ColumnarOffsetType>(-1);


  /// @brief the name for the event context container id
  inline constexpr std::string_view eventContextCIName = "eventContext";

  /// @brief the default name for the column containing the event range
  ///
  /// This is used in quite a few places, so I'm defining it here to
  /// make it visible everywhere. I'm not sure what the best name for it
  /// is.  I'm currently using "EventInfo", because technically it is
  /// the number of `EventInfo` objects the data has. I'm pretty sure
  /// that this will be confusing to many people.
  inline const std::string eventRangeColumnName = "EventInfo";
}

#endif
