/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_CORE_COLUMNAR_DEF_H
#define COLUMNAR_CORE_COLUMNAR_DEF_H

namespace columnar
{
  // This checks that COLUMNAR_DEFAULT_ACCESS_MODE is indeed defined, plus makes it
  // available for use with `if constexpr`.
  constexpr unsigned columnarAccessMode = COLUMNAR_DEFAULT_ACCESS_MODE;

  struct ColumnarModeXAOD
  {
    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = true;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = true;
  };



  struct ColumnarModeArray
  {
    /// Whether this is the xAOD mode.
    static constexpr bool isXAOD = false;

    /// Whether for this columnar mode decorators that replace the
    /// original column will also refer to the input column.
    ///
    /// This is very obscure, but can be queried if it avoids copying over
    /// the input column in the tool first.
    static constexpr bool inPlaceReplace = false;
  };



#if COLUMNAR_DEFAULT_ACCESS_MODE == 0
  using ColumnarModeDefault = ColumnarModeXAOD;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 2
  using ColumnarModeDefault = ColumnarModeArray;
#else
  #error "COLUMNAR_DEFAULT_ACCESS_MODE must be 0 or 2"
#endif
}

#endif
