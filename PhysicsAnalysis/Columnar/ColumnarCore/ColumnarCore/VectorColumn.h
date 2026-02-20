/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_VECTOR_COLUMN_H
#define COLUMNAR_CORE_VECTOR_COLUMN_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/VectorConvertView.h>
#include <span>
#include <vector>

namespace columnar
{
  /// @file VectorColumn.h
  ///
  /// `std::vector` specialization for `AccessorTemplate`
  /// ===================================================
  ///
  /// Per-object vectors get handled differently depending on the
  /// framework, so there are different accessor specializations in
  /// different environments.  So far (11 Dec 24) only input accessors
  /// are supported, and they will generally return an object that
  /// behaves like `std::span<const CT>`



  namespace detail
  {
    template<typename CT,ColumnarMode CM>
      requires (CM::useNestedVectors)
    class MemoryAccessor<std::vector<CT>,CM> final
    {
      /// Public Members
      /// ==============
    public:

      using BaseAccessor = MemoryAccessor<CT,CM>;
      static_assert (BaseAccessor::isDefined, "Element type for std::vector has no accessor defined. Please check includes and whether the type is supported.");

      static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;

      static constexpr bool isDefined = true;
      static constexpr bool viewIsReference = false;
      static constexpr bool hasSetter = false;
      using ElementType = typename BaseAccessor::MemoryType;
      using MemoryType = std::vector<ElementType>;

      static void updateColumnInfo (ColumnInfo& /*info*/)
      {}

      [[nodiscard]] static auto makeViewer (void** dataArea)
      {
        if constexpr (BaseAccessor::viewIsReference)
          return [] (const MemoryType& value) {return std::span<const ElementType> (value);};
        else
        {
          return [viewer = BaseAccessor::makeViewer(dataArea)] (const MemoryType& value) {return VectorConvertView (viewer, std::span<const ElementType> (value));};
        }
      }
    };




    // the column accessor for std::vector in ColumnarModeArray
    //
    // This is one of the more tricky accessors, as it need to connect to
    // two underlying columns.  The first column is the offset column, the
    // second is the data column.
    template<typename CT>
      requires (ContainerFreeAccessor<CT,ColumnAccessMode::input,ColumnarModeArray>::isDefined)
    class ContainerFreeAccessor<std::vector<CT>,ColumnAccessMode::input,ColumnarModeArray> final
    {
      /// Public Members
      /// ==============
    public:

      static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
      using CM = ColumnarModeArray;

      static constexpr bool isDefined = true;
      static constexpr unsigned internalOffsetColumns = ContainerFreeAccessor<CT,ColumnAccessMode::input,ColumnarModeArray>::internalOffsetColumns + 1;

      ContainerFreeAccessor () = default;

      ContainerFreeAccessor (ColumnarTool<CM>& columnarTool, ColumnAccessorOptions&& options, ColumnAccessorOptionsArray&& optionsArray)
      {
        std::string offsetColumn;
        if (optionsArray.numOffsets + internalOffsetColumns == 1u)
          offsetColumn = optionsArray.baseName + ".offset";
        else if (optionsArray.numOffsets + internalOffsetColumns == 2u)
        {
          if (optionsArray.numOffsets == 0u)
            offsetColumn = optionsArray.baseName + ".outerOffset";
          else
            offsetColumn = optionsArray.baseName + ".innerOffset";
        } else
        {
          offsetColumn = optionsArray.baseName + ".offset" + std::to_string(optionsArray.numOffsets);
        }

        auto offsetInfo = options.makeColumnInfo();
        offsetInfo.offsetName = optionsArray.offsetName;
        offsetInfo.isOffset = true;
        m_offsetData = std::make_unique<ColumnAccessorDataArray> (&m_offsetIndex, &m_offsetData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
        columnarTool.addColumn (offsetColumn, m_offsetData.get(), std::move (offsetInfo));
        auto dataOptions = optionsArray;
        dataOptions.offsetName = offsetColumn;
        dataOptions.dataSuffix = ".data";
        dataOptions.numOffsets += 1u;
        m_dataAccessor = ContainerFreeAccessor<CT,ColumnAccessMode::input,CM> (columnarTool, std::move (options), std::move (dataOptions));
      }

      ContainerFreeAccessor (ContainerFreeAccessor&& that)
        : m_dataAccessor (std::move (that.m_dataAccessor))
      {
        moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
      }

      ContainerFreeAccessor& operator = (ContainerFreeAccessor&& that)
      {
        if (this != &that)
        {
          moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
          m_dataAccessor = std::move (that.m_dataAccessor);
        }
        return *this;
      }

      auto operator () (void** dataArea, std::size_t index) const noexcept
      {
        auto *offset = static_cast<const ColumnarOffsetType*>(dataArea[m_offsetIndex]);
        return m_dataAccessor(dataArea, offset[index], offset[index+1]);
      }

      auto operator () (void** dataArea, std::size_t beginIndex, std::size_t endIndex) const noexcept
      {
        auto *offset = static_cast<const ColumnarOffsetType*>(dataArea[m_offsetIndex]);
        return detail::VectorConvertView ([&dataAccessor = m_dataAccessor, dataArea] (const auto& internalIndex) noexcept {
            const ColumnarOffsetType& thisEndIndex = (&internalIndex)[1];
            return dataAccessor (dataArea, internalIndex, thisEndIndex);},
          std::span<const ColumnarOffsetType> (offset + beginIndex, offset + endIndex));
      }

      bool isAvailable (void** dataArea) const noexcept
      {
        return m_dataAccessor.isAvailable (dataArea);
      }

      /// Private Members
      /// ===============
    private:

      unsigned m_offsetIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> m_offsetData;
      ContainerFreeAccessor<CT,ColumnAccessMode::input,ColumnarModeArray> m_dataAccessor;
    };
  }
}

#endif
