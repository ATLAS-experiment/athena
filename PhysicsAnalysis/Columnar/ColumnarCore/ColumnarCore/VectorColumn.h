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



  // in xAOD mode we can do a straightforward conversion from
  // std::vector to std::span, as xAODs natively use std::vector
  template<typename CT>
    requires (ColumnTypeTraits<CT,ColumnarModeXAOD>::isNativeType)
  struct ColumnTypeTraits<std::vector<CT>,ColumnarModeXAOD> final
  {
    using CM = ColumnarModeXAOD;
    using ElementType = typename ColumnTypeTraits<CT,CM>::ColumnType;
    using ColumnType = NativeColumn<std::vector<ElementType>>;
    using UserType = std::span<const ElementType>;
    static constexpr bool isNativeType = false;
    static constexpr bool useConvertInput = true;
    static constexpr bool useConvertWithDataInput = false;
    static ColumnInfo& updateColumnInfo (ColumnarTool<CM>& /*columnarTool*/, ColumnInfo& info) {return info;}
    static std::span<const ElementType> convertInput (const std::vector<ElementType>& value) {return std::span<const ElementType> (value);}
  };




  // the column accessor for std::vector in ColumnarModeArray
  //
  // This is one of the more tricky accessors, as it need to connect to
  // two underlying columns.  The first column is the offset column, the
  // second is the data column.
  template<ContainerIdConcept CI,typename CT>
    requires (ColumnTypeTraits<CT,ColumnarModeArray>::isNativeType)
  class AccessorTemplate<CI,std::vector<CT>,ColumnAccessMode::input,ColumnarModeArray> final
  {
    /// Public Members
    /// ==============
  public:

    static constexpr ColumnAccessMode CAM = ColumnAccessMode::input;
    using CM = ColumnarModeArray;
    using ElementType = typename ColumnTypeTraits<CT,CM>::ColumnType;

    AccessorTemplate () = default;

    AccessorTemplate (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
    {
      auto myinfo = info;
      myinfo.offsetName = columnarTool.containerStoreName (CI::idName);
      myinfo.isOffset = true;
      info.offsetName = columnarTool.containerStoreName (CI::idName) + "." + name + ".offset";
      m_offsetData = std::make_unique<ColumnAccessorDataArray> (&m_offsetIndex, &m_offsetData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
      columnarTool.addColumn (info.offsetName, m_offsetData.get(), std::move (myinfo));
      m_dataData = std::make_unique<ColumnAccessorDataArray> (&m_dataIndex, &m_dataData, &typeid (ElementType), ColumnAccessMode::input);
      columnarTool.addColumn (columnarTool.containerStoreName(CI::idName) + "." + name + ".data", m_dataData.get(), std::move (info));
    }

    AccessorTemplate (AccessorTemplate&& that)
    {
      moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
      moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
    }

    AccessorTemplate& operator = (AccessorTemplate&& that)
    {
      if (this != &that)
      {
        moveAccessor (m_offsetIndex, m_offsetData, that.m_offsetIndex, that.m_offsetData);
        moveAccessor (m_dataIndex, m_dataData, that.m_dataIndex, that.m_dataData);
      }
      return *this;
    }

    std::span<const ElementType> operator () (ObjectId<CI,CM> id) const noexcept
    {
      auto *offset = static_cast<const ColumnarOffsetType*>(id.getData()[m_offsetIndex]);
      auto *data = static_cast<const ElementType*>(id.getData()[m_dataIndex]);
      return std::span<const ElementType> (&data[offset[id.getIndex()]], offset[id.getIndex()+1]-offset[id.getIndex()]);
    }

    bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      auto *data = static_cast<const ElementType*>(id.getData()[m_dataIndex]);
      return data != nullptr;
    }

    /// Private Members
    /// ===============
  private:

    unsigned m_offsetIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_offsetData;
    unsigned m_dataIndex = 0u;
    std::unique_ptr<ColumnAccessorDataArray> m_dataData;
  };


  /// a @ref std::vector accessor for types that can be implemented via
  /// conversions
  ///
  /// This is a bit more involved in that it needs to wrap the
  /// underlying view, and apply the conversion on access.  That means I
  /// need to have a custom view and iterator object to handle it.
  /// Furthermore, since some conversions need a data pointer and others
  /// don't, I need to have two different implementations of the view
  /// and iterator.
  template<ContainerIdConcept CI,typename CT,typename CM>
    requires (ColumnTypeTraits<CT,CM>::useConvertInput || ColumnTypeTraits<CT,CM>::useConvertWithDataInput)
  class AccessorTemplate<CI,std::vector<CT>,ColumnAccessMode::input,CM> final
  {
    /// Public Members
    /// ==============
  public:

    AccessorTemplate () = default;

    AccessorTemplate (ColumnarTool<CM>& columnarTool, const std::string& name, ColumnInfo&& info = {})
      : m_accessor(columnarTool,name,std::move(ColumnTypeTraits<CT,CM>::updateColumnInfo(columnarTool, info)))
    {
    }

    auto operator () (ObjectId<CI,CM> id) const
    {
      if constexpr (ColumnTypeTraits<CT,CM>::useConvertWithDataInput)
        return detail::VectorConvertView ([data = id.getData()](const auto& value) {return ColumnTypeTraits<CT,CM>::convertInput (data, value);}, m_accessor(id));
      else
        return detail::VectorConvertView ([](const auto& value) {return ColumnTypeTraits<CT,CM>::convertInput(value);}, m_accessor(id));
    }

    [[nodiscard]] bool isAvailable (ObjectId<CI,CM> id) const noexcept
    {
      return m_accessor.isAvailable (id);
    }

    /// Private Members
    /// ===============

  private:

    ColumnAccessor<CI,std::vector<typename ColumnTypeTraits<CT,CM>::ColumnType>,CM> m_accessor;
  };
}

#endif
