/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_MODE_COLUMNAR_MODE_H
#define COLUMNAR_MODE_COLUMNAR_MODE_H

#include <string>

#ifndef COLUMNAR_MODE_NAME
#error "COLUMNAR_MODE_NAME was not defined in the build configuration"
#endif
#ifndef COLUMNAR_MODE_SUFFIX
#error "COLUMNAR_MODE_SUFFIX was not defined in the build configuration"
#endif

namespace columnar
{
  struct COLUMNAR_MODE_NAME;
  using ColumnarMode = COLUMNAR_MODE_NAME;

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

  /// add the columnar mode suffix to a given string
  ///
  /// WARNING: Do **not** try to move the suffix into the body and
  /// eliminate the parameter. We need it to be evaluated differently in
  /// different translation units. This is already skirting the limits
  /// of what the standard allows as is.
  inline std::string add_cmode (const std::string& name, const char *suffix = STR(COLUMNAR_MODE_SUFFIX))
  {
    return name + suffix;
  }

#undef STR_IMPL
#undef STR
}

#define ADD_CMODE_JOIN(name,mode) name##mode
#define ADD_CMODE_JOIN2(name,mode) ADD_CMODE_JOIN(name,mode)
#define ADD_CMODE(name) ADD_CMODE_JOIN2(name,COLUMNAR_MODE_SUFFIX)

#endif // COLUMNAR_MODE_COLUMNAR_MODE_H
