/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMNAR_TOOL_H
#define COLUMNAR_CORE_COLUMNAR_TOOL_H

#include <ColumnarCore/ColumnarDef.h>

namespace columnar
{
  /// @brief the base class for all columnar components
  ///
  /// All components (and non-component classes) that want to declare columnar
  /// handles, etc. should inherit from this class.  It contains whatever is
  /// needed to register the data handles, etc.

  template<typename CM = ColumnarModeDefault> class ColumnarTool;
}

#include "ColumnarToolArray.icc"
#include "ColumnarToolXAOD.icc"

#endif
