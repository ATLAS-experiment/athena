/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_SELECTION_COLUMN_H
#define COLUMNAR_CORE_SELECTION_COLUMN_H

#include <ColumnarCore/ColumnAccessor.h>

namespace columnar
{
  /// @brief a column type wrapper for selection flag columns
  ///
  /// The standard Athena convention is to use a `char` decoration for
  /// selections (1: pass, 0: fail) which this emulates, while
  /// converting to a `bool` for the visible accessor type.
  ///
  /// There are three reasons for having a dedicated type here, instead
  /// of just using a `char` accessor/decorator:
  /// - it is immediately clear this is meant to be a selection flag
  /// - it can add extra flags to the column configuration, if needed
  /// - it would allow alternate representations in columnar code in the
  ///   future, e.g. using a single bit per object
  ///
  /// This comes with the promise that an output column will only ever
  /// be filled with the values 0 or 1, and that for an input column
  /// only whether the value is non-zero matters (see @ref
  /// ColumnTypeCategory::boolean). That allows to connect these
  /// columns to buffers of boolean type, e.g. a numpy `bool` array.
  struct SelectionColumn final {};


  /// @brief the standard name of the optional preselection input column
  ///
  /// Columnar tools that support a preselection should declare an
  /// optional input column with this name (and type @ref
  /// SelectionColumn). If the column is present, objects failing the
  /// preselection should not be processed by the tool and instead be
  /// given a default output value. If the column is not present, all
  /// objects are processed.
  inline constexpr const char *preselectionName = "preselection";


  namespace detail
  {
    template<ColumnarModeConcept CM>
    class MemoryAccessor<SelectionColumn,CM> final
    {
    public:
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = true;
      using MemoryType = char;

      /// @brief the promise that we only read whether the value is
      /// non-zero, and only write 0 or 1
      static constexpr bool isBooleanValued = true;

      [[nodiscard]] static auto makeViewer (void**) noexcept
      {
        return [] (char value) noexcept -> bool {return value != 0;};
      }

      [[nodiscard]] static auto makeSetter (void**) noexcept
      {
        return [] (char& memory, bool value) noexcept {memory = value ? 1 : 0;};
      }
    };
  }
}

#endif
