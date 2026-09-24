/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H
#define COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H

namespace columnar
{
  struct ColumnarModeXAOD;
  struct ColumnarModeArray;
  struct ColumnarModeXAODArray;
}

#if COLUMNAR_DEFAULT_ACCESS_MODE == 0
  using CMode = columnar::ColumnarModeXAOD;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 2
  using CMode = columnar::ColumnarModeArray;
#elif COLUMNAR_DEFAULT_ACCESS_MODE == 100
  using CMode = columnar::ColumnarModeXAODArray;
#else
  #error "COLUMNAR_DEFAULT_ACCESS_MODE must be 0, 2, or 100"
#endif

#define ADD_CMODE_JOIN(name,mode) name##mode
#define ADD_CMODE_JOIN2(name,mode) ADD_CMODE_JOIN(name,mode)
#define ADD_CMODE(name) ADD_CMODE_JOIN2(name,COLUMNAR_MODE_SUFFIX)

#endif // COLUMNAR_MODE_DEFAULT_COLUMNAR_MODE_DEFAULT_H
