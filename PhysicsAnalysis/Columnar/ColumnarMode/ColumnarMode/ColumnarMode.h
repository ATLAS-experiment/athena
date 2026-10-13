/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_MODE_COLUMNAR_MODE_H
#define COLUMNAR_MODE_COLUMNAR_MODE_H

namespace columnar
{
  struct COLUMNAR_MODE_NAME;
  using ColumnarMode = COLUMNAR_MODE_NAME;
}

#define ADD_CMODE_JOIN(name,mode) name##mode
#define ADD_CMODE_JOIN2(name,mode) ADD_CMODE_JOIN(name,mode)
#define ADD_CMODE(name) ADD_CMODE_JOIN2(name,COLUMNAR_MODE_SUFFIX)

#endif // COLUMNAR_MODE_COLUMNAR_MODE_H
