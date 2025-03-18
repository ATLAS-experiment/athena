/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMN_ACCESSOR_DATA_ARRAY_H
#define COLUMNAR_CORE_COLUMN_ACCESSOR_DATA_ARRAY_H

#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <memory>

namespace columnar
{
  template<typename CM> class ColumnarTool;
  struct ColumnDataArray;

  /// @brief all the data about a column accessor that the ColumnarTool
  /// needs to know about
  struct ColumnAccessorDataArray final
  {
    /// @brief a pointer to the index in the data vector
    ///
    /// This is needed to allow updating the index as needed, i.e. when
    /// the user sets an index or when tool gets assigned as a subtool
    /// to another tool.
    unsigned *dataIndexPtr = nullptr;

    /// @brief a pointer to the smart pointer that holds this object
    ///
    /// This is to allow clearing out all the configuration data once
    /// the configuration is fixed, to release memory and other
    /// potential overheads.
    std::unique_ptr<ColumnAccessorDataArray> *selfPtr = nullptr;

    /// @brief the @ref ColumnDataArray object that holds us
    ColumnDataArray *dataRef = nullptr;

    /// @brief the type of the column elements
    const std::type_info *type = nullptr;

    /// @brief the access mode for the column
    ColumnAccessMode accessMode = {};

    /// @brief initializing constructor
    ColumnAccessorDataArray (unsigned *val_dataIndexPtr, std::unique_ptr<ColumnAccessorDataArray> *val_selfPtr, const std::type_info *val_type, ColumnAccessMode val_accessMode) noexcept
      : dataIndexPtr (val_dataIndexPtr), selfPtr (val_selfPtr), type (val_type), accessMode (val_accessMode) {}

    /// @brief standard destructor
    ~ColumnAccessorDataArray () noexcept;
  };

  void moveAccessor (unsigned& dataIndex, std::unique_ptr<ColumnAccessorDataArray>& accessorData, unsigned& sourceIndex, std::unique_ptr<ColumnAccessorDataArray>& sourceData);
}

#endif
