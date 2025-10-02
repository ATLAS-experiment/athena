/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarToolWrapper/ColumnVectorWrapper.h>

#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <CxxUtils/checker_macros.h>

#include <boost/core/demangle.hpp>

#include <stdexcept>

//
// method implementations
//

namespace columnar
{
  ColumnVectorHeader ::
  ColumnVectorHeader ()
  {
    m_elements.resize (numFixedColumns);

    // we always reserve the first column as having a null value
    m_elements.at(nullIndex).debugName = "<null>";
    m_elements.at(nullIndex).isOptional = true;

    // we always reserve the second column as the size column
    m_elements.at(sizeIndex).debugName = "<size>";
    m_elements.at(sizeIndex).type = &typeid(std::size_t);
    m_elements.at(sizeIndex).isOptional = false;
    m_elements.at(sizeIndex).readOnly = true;
  }



  [[nodiscard]] std::size_t ColumnVectorHeader ::
  addColumn (const ColumnInfo& columnInfo)
  {
    m_elements.emplace_back();
    m_elements.at(sizeIndex).arraySize = m_elements.size();

    auto& header = m_elements.back();
    header.debugName = columnInfo.name;
    header.type = columnInfo.type;

    switch (columnInfo.accessMode)
    {
    case ColumnAccessMode::input:
      break;
    case ColumnAccessMode::output:
      header.readOnly = false;
      break;
    case ColumnAccessMode::update:
      header.readOnly = false;
      break;
    }
    for (unsigned dim : columnInfo.fixedDimensions)
      header.arraySize *= dim;
    header.isOffset = columnInfo.isOffset;
    if (!columnInfo.isOptional)
      header.isOptional = false;
    if (!columnInfo.offsetName.empty())
      header.offsetIndex = unsetIndex;
    return m_elements.size() - 1;
  }



  void ColumnVectorHeader ::
  setOffsetColumn (std::size_t columnIndex, std::size_t offsetIndex)
  {
    if (columnIndex >= m_elements.size())
      throw std::logic_error ("column index out of range");
    if (offsetIndex >= m_elements.size())
      throw std::logic_error ("offset index out of range");
    auto& column = m_elements.at(columnIndex);
    auto& offset = m_elements.at(offsetIndex);
    if (!offset.isOffset)
      throw std::runtime_error ("trying to set " + offset.debugName + " as offset column for " + column.debugName + ", but it is not marked as offset column");
    if (column.offsetIndex != unsetIndex && column.offsetIndex != offsetIndex)
      throw std::runtime_error ("trying to set " + offset.debugName + " as offset column for " + column.debugName + ", but it is already set to " + m_elements.at(column.offsetIndex).debugName);
    column.offsetIndex = offsetIndex;
  }



  void ColumnVectorHeader ::
  checkSelf () const
  {
    if (m_elements.size() < numFixedColumns)
      throw std::logic_error ("header has too few m_elements, expected at least " + std::to_string(numFixedColumns) + " but got " + std::to_string(m_elements.size()));
    if (m_elements[sizeIndex].arraySize != m_elements.size())
      throw std::logic_error ("size column has wrong array size, expected " + std::to_string(m_elements.size()) + " but got " + std::to_string(m_elements[sizeIndex].arraySize));

    // skipping the first column, which is always the null column
    // and behaves in a funny manner
    for (std::size_t columnIndex = 1u; columnIndex != m_elements.size(); ++ columnIndex)
    {
      const auto& column = m_elements[columnIndex];
      if (column.debugName.empty())
        throw std::logic_error ("column " + std::to_string(columnIndex) + " has no name");
      if (column.type == nullptr)
        throw std::logic_error ("column " + column.debugName + " has no type");
      if (column.isOffset && *column.type != typeid(ColumnarOffsetType))
        throw std::logic_error ("column " + column.debugName + " is an offset column that is of type " + boost::core::demangle(column.type->name()) + " instead of ColumnarOffsetType");
    }

    for (std::size_t columnIndex = 1u; columnIndex != m_elements.size(); ++ columnIndex)
    {
      const auto& column = m_elements[columnIndex];
      if (column.offsetIndex != nullIndex)
      {
        if (column.offsetIndex == unsetIndex)
          throw std::logic_error ("column " + column.debugName + " has an offset index that was never set");
        if (column.offsetIndex >= m_elements.size())
          throw std::logic_error ("column " + column.debugName + " has invalid offset index " + std::to_string(column.offsetIndex) + " (max is " + std::to_string(m_elements.size()-1) + ")");
        const auto& offsetElement = m_elements[column.offsetIndex];
        if (!offsetElement.isOffset)
          throw std::logic_error ("column " + column.debugName + " has offset index that is not marked as offset");
      }
    }
  }



  void ColumnVectorHeader ::
  checkData (std::span<const void* const> data) const
  {
    if (data.size() != m_elements.size())
      throw std::logic_error ("data vector has wrong size, expected " + std::to_string(m_elements.size()) + " but got " + std::to_string(data.size()));
    if (data[nullIndex] != nullptr)
      throw std::logic_error ("null column is not set to a nullptr value");
    const auto* sizeVector = static_cast<const std::size_t*>(data[sizeIndex]);
    for (std::size_t columnIndex = 0u; columnIndex != m_elements.size(); ++ columnIndex)
    {
      const auto& column = m_elements[columnIndex];
      if (data[columnIndex] == nullptr)
      {
        if (column.isOptional)
          continue;
        if (column.offsetIndex != nullIndex && data[column.offsetIndex] == nullptr)
          continue;
        throw std::logic_error ("column " + column.debugName + " was not set");
      }
      if (column.isOffset)
      {
        const auto size = sizeVector[columnIndex];
        if (size < 1)
          throw std::runtime_error ("offset column " + column.debugName + " has size " + std::to_string(size) + ", but needs at least 1 element");
        auto *offsets = static_cast<const ColumnarOffsetType*>(data[columnIndex]);
        if (offsets[0] != 0)
          throw std::runtime_error ("offset column doesn't start with 0: " + column.debugName);
        for (std::size_t i = 1u; i != size; ++ i)
        {
          if (offsets[i] < offsets[i-1])
            throw std::runtime_error ("offset column " + column.debugName + " is not monotonically increasing at index " + std::to_string(i) + ": " + std::to_string(offsets[i-1]) + " > " + std::to_string(offsets[i]));
        }
      }
    }
    for (std::size_t columnIndex = 0u; columnIndex != m_elements.size(); ++ columnIndex)
    {
      if (data[columnIndex] == nullptr)
        continue;
      const auto& column = m_elements[columnIndex];
      std::size_t expectedSize = 1u;
      if (column.offsetIndex != ColumnVectorHeader::nullIndex)
      {
        if (data[column.offsetIndex] == nullptr)
          throw std::runtime_error ("column " + column.debugName + " uses offset column " + m_elements[column.offsetIndex].debugName + " that is not set");
        const auto offsetIndex = column.offsetIndex;
        auto *offsetsPtr = static_cast<const ColumnarOffsetType*>(data[offsetIndex]);
        expectedSize = offsetsPtr[sizeVector[offsetIndex]-1];
      }
      expectedSize *= column.arraySize;

      if (column.isOffset)
        expectedSize += 1u;

      if (sizeVector[columnIndex] != expectedSize)
        throw std::runtime_error ("column size doesn't match expected size: " + column.debugName + ", found " + std::to_string (sizeVector[columnIndex]) + " vs exptected=" + std::to_string (expectedSize) + " isOffset=" + std::to_string (column.isOffset));
    }
  }



  ColumnVectorData ::
  ColumnVectorData (const ColumnVectorHeader *val_header)
    : m_header (val_header),
      m_data (val_header->numColumns(), nullptr),
      m_dataSize (val_header->numColumns(), 0u)
  {
    setColumn (ColumnVectorHeader::sizeIndex, m_dataSize.size(), m_dataSize.data());
  }



  void ColumnVectorData ::
  setColumnVoid (std::size_t columnIndex, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst)
  {
    if (columnIndex == ColumnVectorHeader::nullIndex)
      throw std::logic_error ("cannot set the null column");
    if (columnIndex >= m_header->numColumns())
      throw std::logic_error ("invalid column index: " + std::to_string(columnIndex) + " (max is " + std::to_string(m_header->numColumns()-1) + ")");
    auto& header = m_header->getColumn (columnIndex);

    // If dataPtr is null, we use a dummy value to avoid issues in which
    // we check whether a column exists by checking for a null pointer,
    // which would return false for columns that were set with a
    // nullptr.
    if (dataPtr == nullptr)
    {
      if (size != 0) [[unlikely]]
        throw std::logic_error ("dataPtr is null but size is not zero for column: " + header.debugName);
      static const unsigned dummyValue = 0;
      dataPtr = &dummyValue;
    }

    if (type != *header.type)
      throw std::runtime_error ("invalid type for column: " + header.debugName);
    if (isConst && !header.readOnly)
      throw std::runtime_error ("assigning const vector to a column that is not read-only: " + header.debugName);
    if (m_data[columnIndex] != nullptr)
      throw std::runtime_error ("column filled multiple times: " + header.debugName);
    auto *castDataPtr ATLAS_THREAD_SAFE = const_cast<void*>(dataPtr);
    m_data[columnIndex] = castDataPtr;
    m_dataSize[columnIndex] = size;
  }



  std::pair<std::size_t,const void*> ColumnVectorData ::
  getColumnVoid (std::size_t columnIndex, const std::type_info *type, bool isConst)
  {
    if (columnIndex >= m_header->numColumns())
      throw std::runtime_error ("invalid column index: " + std::to_string(columnIndex) + " (max is " + std::to_string(m_header->numColumns()-1) + ")");

    auto& header = m_header->getColumn (columnIndex);
    if (*type != *header.type)
      throw std::runtime_error ("invalid type for column: " + header.debugName);
    if (!isConst && header.readOnly)
      throw std::runtime_error ("retrieving non-const vector from a read-only column: " + header.debugName);
    if (m_data[columnIndex] != nullptr)
      return std::make_pair (m_dataSize[columnIndex], m_data[columnIndex]);
    else
      return std::make_pair (0u, nullptr);
  }



  void ColumnVectorData ::
  callNoCheck (const IColumnarTool& tool)
  {
    tool.callVoid (m_data.data());
  }
}
