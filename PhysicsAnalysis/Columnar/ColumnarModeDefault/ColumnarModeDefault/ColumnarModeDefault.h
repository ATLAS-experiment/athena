/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H
#define COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H

namespace columnar
{
  // This checks that COLUMNAR_DEFAULT_ACCESS_MODE is indeed defined, plus makes it
  // available for use with `if constexpr`.
  constexpr unsigned columnarAccessMode = COLUMNAR_DEFAULT_ACCESS_MODE;

  struct ColumnarModeXAOD;
  struct ColumnarModeArray;
  struct ColumnarModeXAODArray;

#if COLUMNAR_DEFAULT_ACCESS_MODE == 0
  using ColumnarModeDefault = ColumnarModeXAOD;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 2
  using ColumnarModeDefault = ColumnarModeArray;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 100
  using ColumnarModeDefault = ColumnarModeXAODArray;
#else
  #error "COLUMNAR_DEFAULT_ACCESS_MODE must be 0, 2, or 100"
#endif
}

using CMode = columnar::ColumnarModeDefault;

#endif // COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H
