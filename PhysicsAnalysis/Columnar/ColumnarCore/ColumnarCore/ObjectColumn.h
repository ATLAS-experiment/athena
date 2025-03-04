/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OBJECT_COLUMN_H
#define COLUMNAR_CORE_OBJECT_COLUMN_H

#include <ColumnarCore/ColumnAccessor.h>

namespace columnar
{
  /// @brief a special type to use for columns accessing
  /// containers/offset maps
  ///
  /// These columns are a bit unusual in that they don't map from e.g.
  /// JetId to float, but from EventContextId to JetRange.  That means
  /// some of the behavior is different from "regular" columns, but it
  /// is still helpful to have this as a specialization.  If nothing
  /// else, it means that I have all the type aliases from regular
  /// columns available.
  struct ObjectColumn {};
}

#include "ObjectColumnArray.icc"
#include "ObjectColumnXAOD.icc"

#endif
