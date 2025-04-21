/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_STRING_COLUMN_H
#define COLUMNAR_CORE_STRING_COLUMN_H

#include <ColumnarCore/VectorColumn.h>

namespace columnar
{
  // in xAOD mode we can do a straightforward conversion from
  // std::string to std::string_view, as xAODs natively use std::string
  template<>
  struct ColumnTypeTraits<std::string,ColumnarModeXAOD> final
  {
    using CM = ColumnarModeXAOD;
    using ColumnType = NativeColumn<std::string>;
    using UserType = std::string_view;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info) {return info;}
    static std::string_view convertInput (const std::string& value) {return std::string_view (value);}
  };



  // in external mode we can do a straightforward conversion from
  // std::vector<char> to std::string_view, as uproot natively
  // represents it as a vector of char
  template<>
  struct ColumnTypeTraits<std::string,ColumnarModeArray> final
  {
    using CM = ColumnarModeArray;
    using ColumnType = std::vector<char>;
    using UserType = std::string_view;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info) {return info;}
    static std::string_view convertInput (std::span<const char> value) {return std::string_view (value.begin(), value.end());}
  };
}

#endif
