/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_CORE_COLUMNAR_DEF_H
#define COLUMNAR_CORE_COLUMNAR_DEF_H

#include <cstdint>
#include <string_view>

#include <SGCore/sgkey_t.h>

// IMPORTANT: If this include is not found, then you need to add
// `ColumnarModeLib` to your (public) CMake dependencies. Do **not**
// change this package to publicly depend on `ColumnarModeLib`.
#include <ColumnarMode/ColumnarMode.h>

namespace columnar
{
  struct ColumnarModeXAOD
  {
    /// Whather this is a columnar mode
    static constexpr bool isColumnarMode = true;

    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = true;

    /// Whether this is an array mode
    static constexpr bool isArrayMode = false;

    /// Whether we use nested vectors for std::vector<...> columns
    static constexpr bool useNestedVectors = true;

    /// Whether we use offset maps for std::vector<...> and std::string
    /// columns, i.e. an offset column together with a flat data column
    static constexpr bool useOffsetMaps = false;

    /// Whether element links are represented as a key column and an
    /// index column, with the index being relative to the start of the
    /// event, not the column.
    static constexpr bool useKeyIndexAsLink = false;

    /// Whether this is the mode used for uproot/awkward-style inputs
    ///
    /// This is a catch-all flag for code that has no more specific
    /// flag to rely on.  It makes no promises beyond being the mode
    /// used with uproot.
    static constexpr bool isUprootMode = false;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = true;
  };



  /// @brief the array mode
  ///
  /// Links are read as the key and index columns found in the file.
  struct ColumnarModeArray
  {
    /// Whather this is a columnar mode
    static constexpr bool isColumnarMode = true;

    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = false;

    /// Whether this is an array mode
    static constexpr bool isArrayMode = true;

    /// Whether we use nested vectors for std::vector<...> columns
    static constexpr bool useNestedVectors = false;

    /// Whether we use offset maps for std::vector<...> and std::string
    /// columns, i.e. an offset column together with a flat data column
    static constexpr bool useOffsetMaps = true;

    /// Whether element links are represented as a key column and an
    /// index column, with the index being relative to the start of the
    /// event, not the column. This is the layout as found on disk for
    /// xAODs.
    static constexpr bool useKeyIndexAsLink = true;

    /// Whether this is the mode used for uproot/awkward-style inputs
    ///
    /// This is a catch-all flag for code that has no more specific
    /// flag to rely on.  It makes no promises beyond being the mode
    /// used with uproot.
    static constexpr bool isUprootMode = true;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = false;


    /// Element Link Definition
    /// =======================
    ///
    /// Links are represented by a key column and an index column, as
    /// found in the input file. The key is the StoreGate key of the
    /// target container, and the index is relative to the start of the
    /// event in the target container. To turn them into an object the
    /// link accessors also read the event-index column for the offset
    /// map of the link column and the offset maps of all the targets.

    /// The name of the column used for the event-index-per-entry column
    static constexpr std::string_view eventIndexColumnName = "eventIndex";

    /// The name/suffix of the column used for the link-key column
    static constexpr std::string_view linkKeyColumnName = "m_persKey";

    /// The name/suffix of the column used for the link-index column
    static constexpr std::string_view linkIndexColumnName = "m_persIndex";

    /// The type used for the link-key column
    using LinkKeyColumnType = SG::sgkey_t;

    /// The type used for the link-index column
    using LinkIndexColumnType = std::uint32_t;

    /// The key used for writing null links. On reading a link is null
    /// if either its key or its index matches the null value.
    static constexpr LinkKeyColumnType nullLinkKey = 0;

    /// The index used for writing null links. On reading a link is null
    /// if either its key or its index matches the null value.
    static constexpr LinkIndexColumnType nullLinkIndex = static_cast<LinkIndexColumnType>(-1);

    /// Whether to skip validation checks on link columns that are not
    /// strictly needed, e.g. whether the sgkey is valid for a single
    /// target link, or whether the index is less than the number of
    /// objects in the target container.
    static constexpr bool skipLinkValidation = false;
  };



  struct ColumnarModeXAODArray
  {
    /// Whather this is a columnar mode
    static constexpr bool isColumnarMode = true;

    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = false;

    /// Whether this is an array mode
    static constexpr bool isArrayMode = true;

    /// Whether we use nested vectors for std::vector<...> columns
    static constexpr bool useNestedVectors = true;

    /// Whether we use offset maps for std::vector<...> and std::string
    /// columns, i.e. an offset column together with a flat data column
    static constexpr bool useOffsetMaps = false;

    /// Whether element links are represented as a key column and an
    /// index column, with the index being relative to the start of the
    /// event, not the column.
    static constexpr bool useKeyIndexAsLink = false;

    /// Whether this is the mode used for uproot/awkward-style inputs
    ///
    /// This is a catch-all flag for code that has no more specific
    /// flag to rely on.  It makes no promises beyond being the mode
    /// used with uproot.
    static constexpr bool isUprootMode = false;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = true;
  };

  template<typename Mode>
  concept ColumnarModeConcept = Mode::isColumnarMode;
  template<typename Mode>
  concept ColumnarArrayModeConcept = ColumnarModeConcept<Mode> && Mode::isArrayMode;
}

#endif
