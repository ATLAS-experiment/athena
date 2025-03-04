/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


/// Includes
/// ========

#include <ColumnarCore/ColumnAccessorDataArray.h>

#include <ColumnarCore/ColumnarToolDataArray.h>


/// Function Implementations
/// ========================

namespace columnar
{
  ColumnAccessorDataArray ::
  ~ColumnAccessorDataArray () noexcept
  {
    if (dataRef)
      dataRef->removeAccessor (*this);
  }



  void moveAccessor (unsigned& dataIndex, std::unique_ptr<ColumnAccessorDataArray>& accessorData, unsigned& sourceIndex, std::unique_ptr<ColumnAccessorDataArray>& sourceData)
  {
    if (accessorData != nullptr)
      throw std::runtime_error ("data already set, overwriting not yet supported");

    dataIndex = sourceIndex;
    sourceIndex = 0;

    if (sourceData)
    {
      if (sourceData->selfPtr != &sourceData)
        throw std::logic_error ("selfPtr not set correctly");

      accessorData = std::move (sourceData);
      accessorData->selfPtr = &accessorData;
      accessorData->dataIndexPtr = &dataIndex;
    }
  }
}