/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_COLUMN_INFO_HELPERS_H
#define COLUMNAR_INTERFACES_COLUMN_INFO_HELPERS_H

#include <ColumnarInterfaces/ColumnInfo.h>

/// @file
/// @brief helper functions for working with the type information in
/// @ref columnar::ColumnInfo
///
/// These are the helpers that are needed by code that only links
/// against the columnar interfaces (e.g. the tool wrapper), which is
/// why they are here and not in `ColumnarCore/ColumnInfoHelpers.h`.
/// The helpers needed for defining accessors are in `ColumnInfo.h`
/// itself.

namespace columnar
{
  /// @brief whether the category is one of the integer categories
  /// (including boolean)
  [[nodiscard]] constexpr bool isIntegerTypeCategory (ColumnTypeCategory category) noexcept
  {
    return category == ColumnTypeCategory::signedInteger ||
      category == ColumnTypeCategory::unsignedInteger ||
      category == ColumnTypeCategory::signAgnosticInteger ||
      category == ColumnTypeCategory::boolean;
  }


  /// @brief merge the type categories of two users of the same column
  /// (with the same type), the strictest one wins
  ///
  /// The order (from least to most strict) is `boolean`,
  /// `signAgnosticInteger`, and then the signed/unsigned categories.
  [[nodiscard]] constexpr ColumnTypeCategory
  mergeColumnTypeCategory (ColumnTypeCategory a, ColumnTypeCategory b) noexcept
  {
    if (a == b || b == ColumnTypeCategory::boolean)
      return a;
    if (a == ColumnTypeCategory::boolean)
      return b;
    if (b == ColumnTypeCategory::signAgnosticInteger)
      return a;
    return b;
  }


  /// @brief whether data of the given category and size can be used
  /// for a column with the given category and size
  ///
  /// This doesn't consider exact type matches, i.e. for `other`
  /// columns this always returns false, and the caller needs to check
  /// for an exact type match separately. The data is reinterpreted,
  /// not converted, so the size always has to match.
  ///
  /// Boolean data can be used for any integer column that is only
  /// read, but for writing the column has to be boolean (i.e. the tool
  /// promises to only write 0 or 1).
  [[nodiscard]] constexpr bool
  isColumnTypeCompatible (ColumnTypeCategory columnCategory, unsigned columnSize, ColumnTypeCategory dataCategory, unsigned dataSize, bool isConst) noexcept
  {
    if (columnSize != dataSize || columnSize == 0u)
      return false;
    switch (columnCategory)
    {
    case ColumnTypeCategory::floatingPoint:
    case ColumnTypeCategory::signedInteger:
    case ColumnTypeCategory::unsignedInteger:
      return dataCategory == columnCategory ||
        (isConst && dataCategory == ColumnTypeCategory::boolean && columnCategory != ColumnTypeCategory::floatingPoint);
    case ColumnTypeCategory::signAgnosticInteger:
      return isIntegerTypeCategory (dataCategory) &&
        (isConst || dataCategory != ColumnTypeCategory::boolean);
    case ColumnTypeCategory::boolean:
      return isIntegerTypeCategory (dataCategory);
    case ColumnTypeCategory::other:
      return false;
    }
    return false;
  }


  /// @brief a human readable name for the category (for messages)
  [[nodiscard]] constexpr const char *
  columnTypeCategoryName (ColumnTypeCategory category) noexcept
  {
    switch (category)
    {
    case ColumnTypeCategory::floatingPoint: return "floatingPoint";
    case ColumnTypeCategory::signedInteger: return "signedInteger";
    case ColumnTypeCategory::unsignedInteger: return "unsignedInteger";
    case ColumnTypeCategory::signAgnosticInteger: return "signAgnosticInteger";
    case ColumnTypeCategory::boolean: return "boolean";
    case ColumnTypeCategory::other: return "other";
    }
    return "unknown";
  }
}

#endif
