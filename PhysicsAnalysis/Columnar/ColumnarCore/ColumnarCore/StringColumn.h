/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_STRING_COLUMN_H
#define COLUMNAR_CORE_STRING_COLUMN_H

#include <ColumnarCore/VectorColumn.h>

namespace columnar
{
  namespace detail
  {
    // in xAOD mode we can do a straightforward conversion from
    // std::string to std::string_view, as xAODs natively use std::string
    template<>
    class MemoryAccessor<std::string,ColumnarModeXAOD> final
    {
      /// Public Members
      /// ==============
    public:

      using CM = ColumnarModeXAOD;
      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using MemoryType = std::string;

      static void updateColumnInfo (ColumnInfo& /*info*/) {}

      [[nodiscard]] static auto makeViewer (void**)
      {
        return [] (const std::string& value) {return std::string_view (value);};
      }
    };



    // in Array mode we can do a straightforward conversion from
    // std::vector<char> to std::string_view, as uproot natively
    // represents it as a vector of char
    template<>
    class ContainerFreeAccessor<std::string,ColumnAccessMode::input,ColumnarModeArray> final
    {
      /// Public Members
      /// ==============
    public:

      static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
      using CM = ColumnarModeArray;
      using BaseAccessor = ContainerFreeAccessor<std::vector<char>,CAM,CM>;

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = BaseAccessor::internalOffsetColumns;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
        : m_accessor (columnarTool, std::move (options), std::move (optionsArray))
      {}

      auto operator () (void** dataArea, std::size_t index) const noexcept
      {
        std::span<const char> base = m_accessor(dataArea, index);
        return std::string_view (base.data(), base.size());
      }

      auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
      {
        return detail::VectorConvertView ([] (std::span<const char> base) noexcept {
            return std::string_view (base.data(), base.size());},
          m_accessor(dataArea, beginIndex, endIndex));
      }

      bool isAvailable (void** dataArea) const noexcept
      {
        return m_accessor.isAvailable (dataArea);
      }

      /// Private Members
      /// ===============
    private:

      BaseAccessor m_accessor;
    };
  }
}

#endif
