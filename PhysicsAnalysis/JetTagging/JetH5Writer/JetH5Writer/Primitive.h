/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <string>

struct Primitive {
  enum class Type {
    PRECISION_CUSTOM,
    CUSTOM,
    UCHAR,
    CHAR,
    USHORT,
    SHORT,
    UINT,
    INT,
    ULONG,
    LONG,
    ULL,
    LL,
    HALF,
    FLOAT,
    DOUBLE,
    UINT2UCHAR,
    INT2CHAR,
    UINT2USHORT,
    INT2SHORT,
    UL2ULL,
  };
  Type type{};
  std::string source;
  std::string target;
};

#endif
