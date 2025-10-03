/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ToolWrapper.h>

#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <CxxUtils/checker_macros.h>

#include <boost/core/demangle.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <typeindex>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    ToolWrapperData ::
    ToolWrapperData (ColumnVectorData *val_columnData, const ToolColumnVectorMap *val_wrapper) noexcept
      : m_wrapper (val_wrapper), m_columnData (val_columnData)
    {
    }
  }
}
