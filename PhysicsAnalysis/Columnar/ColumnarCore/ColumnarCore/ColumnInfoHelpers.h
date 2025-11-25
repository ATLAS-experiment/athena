/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMN_INFO_HELPERS_H
#define COLUMNAR_CORE_COLUMN_INFO_HELPERS_H

#include <ColumnarInterfaces/ColumnInfo.h>

namespace columnar
{
  void addColumnAccessMode (ColumnInfo& info, ColumnAccessMode accessMode);

  void mergeColumnInfo (ColumnInfo& target, const ColumnInfo& source);
}

#endif
