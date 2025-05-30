/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_VECTOR_VECTOR_COLUMN_H
#define COLUMNAR_CORE_VECTOR_VECTOR_COLUMN_H

#include <ColumnarCore/VectorColumn.h>

namespace columnar
{
  // the column accessor for std::vector of std::vector in ColumnarModeArray
  //
  // This is one of the more tricky accessors, as it need to connect to
  // three underlying columns.  The first two columns are the offset
  // columns, the third is the data column.
  template<ContainerId CI,typename CT>
    requires (ColumnTypeTraits<CT,ColumnarModeArray>::isNativeType)
  class AccessorTemplate<CI,std::vector<std::vector<CT>>,ColumnAccessMode::input,ColumnarModeArray> final
  {
    /// Public Members
    /// ==============
  public:

    static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
    using CM = ColumnarModeArray;
    using ElementType = typename ColumnTypeTraits<CT,ColumnarModeArray>::ColumnType;

    AccessorTemplate () = default;

    AccessorTemplate (ColumnarTool<CM>& columnBase, const std::string& name, ColumnInfo&& info = {})
    {
      auto myinfoOuter = info;
      myinfoOuter.offsetName = columnBase.objectName (CI);
      myinfoOuter.isOffset = true;
      auto myinfoInner = info;
      myinfoInner.offsetName = columnBase.objectName (CI) + "." + name + ".outerOffset";
      myinfoInner.isOffset = true;
      info.offsetName = columnBase.objectName (CI) + "." + name + ".innerOffset";
      m_outerOffsetData = std::make_unique<ColumnAccessorDataArray> (&m_outerOffsetIndex, &m_outerOffsetData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
      columnBase.addColumn (myinfoInner.offsetName, m_outerOffsetData.get(), std::move (myinfoOuter));
      m_innerOffsetData = std::make_unique<ColumnAccessorDataArray> (&m_innerOffsetIndex, &m_innerOffsetData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
      columnBase.addColumn (info.offsetName, m_innerOffsetData.get(), std::move (myinfoInner));
      m_dataData = std::make_unique<ColumnAccessorDataArray> (&m_dataIndex, &m_dataData, &typeid (ElementType), ColumnAccessMode::input);
      columnBase.addColumn (columnBase.objectName(CI) + "." + name + ".data", m_dataData.get(), std::move (info));
    }

    AccessorTemplate (AccessorTemplate&& that)
    {
      moveAccessor (m_outerOffsetIndex, m_outerOffsetData, that.m_outerOffsetIndex, that.m_outerOffsetData);
      moveAccessor (m_innerOffsetIndex, m_innerOffsetData, that.m_innerOffsetIndex, that.m_innerOffsetData);
      moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
    }

    AccessorTemplate& operator = (AccessorTemplate&& that)
    {
      if (this != &that)
      {
        moveAccessor (m_outerOffsetIndex, m_outerOffsetData, that.m_outerOffsetIndex, that.m_outerOffsetData);
        moveAccessor (m_innerOffsetIndex, m_innerOffsetData, that.m_innerOffsetIndex, that.m_innerOffsetData);
        moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
      }
      return *this;
    }

    auto operator () (ObjectId<CI,CM> id) const noexcept
    {
      auto *outerOffset = static_cast<const ColumnarOffsetType*>(id.getData()[m_outerOffsetIndex]);
      auto *innerOffset = static_cast<const ColumnarOffsetType*>(id.getData()[m_innerOffsetIndex]);
      auto *data = static_cast<const ElementType*>(id.getData()[m_dataIndex]);
      return detail::VectorConvertView ([data] (const ColumnarOffsetType& index) noexcept {
          const ColumnarOffsetType& endIndex = (&index)[1];
          return std::span<const ElementType> (data + index, data + endIndex);},
        std::span<const ColumnarOffsetType> (innerOffset + outerOffset[id.getIndex()], innerOffset + outerOffset[id.getIndex()+1]));
    }

    bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      auto *data = static_cast<const ElementType*>(id.getData()[m_dataIndex]);
      return data != nullptr;
    }

    /// Private Members
    /// ===============
  private:

    unsigned m_outerOffsetIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_outerOffsetData;
    unsigned m_innerOffsetIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_innerOffsetData;
    unsigned m_dataIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_dataData;
  };
}

#endif
