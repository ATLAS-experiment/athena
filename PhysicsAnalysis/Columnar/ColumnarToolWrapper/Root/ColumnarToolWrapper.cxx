/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarToolWrapper/ColumnarToolWrapper.h>

#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <CxxUtils/checker_macros.h>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <typeindex>

//
// method implementations
//

namespace columnar
{
  ColumnarToolWrapper ::
  ColumnarToolWrapper (IColumnarTool *val_tool)
    : m_tool (val_tool)
  {
    constexpr unsigned numpySigned = 0;
    constexpr unsigned numpyUnsigned = 1;
    constexpr unsigned numpyFloat = 2;
    std::unordered_map<std::type_index, std::pair<int,unsigned>> numpyTypes;
    numpyTypes[typeid (float)] = { numpyFloat, sizeof (float) * 8 };
    numpyTypes[typeid (char)] = { std::is_signed_v<char> ? numpySigned : numpyUnsigned, sizeof (char) * 8 };
    numpyTypes[typeid (int)] = { numpySigned, sizeof (int) * 8 };
    numpyTypes[typeid (std::uint8_t)] = { numpyUnsigned, sizeof (std::uint8_t) * 8 };
    numpyTypes[typeid (std::uint16_t)] = { numpyUnsigned, sizeof (std::uint16_t) * 8 };
    numpyTypes[typeid (std::uint32_t)] = { numpyUnsigned, sizeof (std::uint32_t) * 8 };
    numpyTypes[typeid (std::uint64_t)] = { numpyUnsigned, sizeof (std::uint64_t) * 8 };


    auto toolColumns = m_tool->getColumnInfo();
    unsigned nextIndex = 1u;
    for (auto& column : toolColumns)
    {
      MyColumnInfo myinfo;
      myinfo.index = nextIndex++;
      val_tool->setColumnIndex (column.name, myinfo.index);
      myinfo.type = column.type;
      switch (column.accessMode)
      {
      case ColumnAccessMode::input:
        myinfo.isConst = true;
        break;
      case ColumnAccessMode::output:
        myinfo.isConst = false;
        break;
      case ColumnAccessMode::update:
        myinfo.isConst = false;
        break;
      }
      for (unsigned dim : column.fixedDimensions)
        myinfo.fixedDimensions *= dim;
      myinfo.isOffset = column.isOffset;
      myinfo.isOptional = column.isOptional;

      if (auto iter = numpyTypes.find (*column.type); iter != numpyTypes.end())
      {
        myinfo.numpyType = iter->second.first;
        myinfo.numpyBits = iter->second.second;
      }
      const auto infoIdx = myinfo.index;
      auto [iter, success] = m_columns.emplace (column.name, std::move (myinfo));
      if (!success)
        throw std::runtime_error ("column name already registered: " + column.name);

      if (m_numColumns <= infoIdx)
        m_numColumns = infoIdx + 1;
   }

    for (auto& column : toolColumns)
    {
      if (!column.offsetName.empty())
      {
        auto offsetIter = m_columns.find (column.offsetName);
        if (offsetIter == m_columns.end())
          throw std::runtime_error ("offset column name not found: " + column.offsetName);
        if (*offsetIter->second.type != typeid (ColumnarOffsetType))
          throw std::runtime_error ("offset column has wrong type: " + column.offsetName);
        if (!offsetIter->second.isOffset)
          throw std::runtime_error ("offset column is not registered as offset: " + column.offsetName);
        m_columns.at (column.name).offsets = &*offsetIter;
      }
    }
  }



  ColumnarToolWrapper ::
  ColumnarToolWrapper (std::shared_ptr<IColumnarTool> val_tool)
    : ColumnarToolWrapper (val_tool.get())
  {
    m_toolOwn = std::move (val_tool);
  }



  ColumnarToolWrapperData ::
  ColumnarToolWrapperData (const ColumnarToolWrapper *val_wrapper) noexcept
    : m_wrapper (val_wrapper),
      m_data (m_wrapper->m_numColumns, nullptr),
      m_dataSize (m_wrapper->m_numColumns, 0u),
      m_columnIsChecked (m_wrapper->m_numColumns, false),
      m_columnIsFilled (m_wrapper->m_numColumns, false)
  {}



  void ColumnarToolWrapperData ::
  setColumnVoid (const std::string& name, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst)
  {
    auto column = m_wrapper->m_columns.find (name);
    if (column == m_wrapper->m_columns.end())
      throw std::runtime_error ("unknown column name: " + name);

    if (type != *column->second.type)
      throw std::runtime_error ("invalid type for column: " + name);
    if (isConst && !column->second.isConst)
      throw std::runtime_error ("assigning const vector to a non-const column: " + name);
    if (column->second.index == 0)
      throw std::runtime_error ("column has no index assigned: " + name);
    if (m_columnIsFilled[column->second.index])
      throw std::runtime_error ("column filled multiple times: " + name);
    m_columnIsFilled[column->second.index] = true;
    auto *castDataPtr ATLAS_THREAD_SAFE = const_cast<void*>(dataPtr);
    m_data[column->second.index] = castDataPtr;
    m_dataSize[column->second.index] = size;
  }



  void ColumnarToolWrapperData ::
  setColumnNumpy (const std::string& name, std::size_t size, const void *dataPtr, int type, unsigned bits, bool isConst)
  {
    auto column = m_wrapper->m_columns.find (name);
    if (column == m_wrapper->m_columns.end())
      throw std::runtime_error ("unknown column name: " + name);

    if (type != column->second.numpyType || bits != column->second.numpyBits)
      throw std::runtime_error ("invalid type for column: " + name + " (expected " + std::to_string (column->second.numpyType) + "/" + std::to_string (column->second.numpyBits) + " but got " + std::to_string (type) + "/" + std::to_string (bits) + ")");
    if (isConst && !column->second.isConst)
      throw std::runtime_error ("assigning const vector to a non-const column: " + name);
    if (column->second.index == 0)
      throw std::runtime_error ("column has no index assigned: " + name);
    if (m_columnIsFilled[column->second.index])
      throw std::runtime_error ("column filled multiple times: " + name);
    m_columnIsFilled[column->second.index] = true;
    auto *castDataPtr ATLAS_THREAD_SAFE = const_cast<void*>(dataPtr);
    m_data[column->second.index] = castDataPtr;
    m_dataSize[column->second.index] = size;
  }



  std::pair<std::size_t,const void*> ColumnarToolWrapperData ::
  getColumnVoid (const std::string& name, const std::type_info *type, bool isConst)
  {
    auto column = m_wrapper->m_columns.find (name);
    if (column == m_wrapper->m_columns.end())
      throw std::runtime_error ("unknown column name: " + name);

    if (*type != *column->second.type)
      throw std::runtime_error ("invalid type for column: " + name);
    if (!isConst && column->second.isConst)
      throw std::runtime_error ("retrieving non-const vector from a const column: " + name);
    if (m_data[column->second.index] != nullptr)
      return std::make_pair (m_dataSize[column->second.index],
                              m_data[column->second.index]);
    else
      return std::make_pair (0u, nullptr);
  }



  void ColumnarToolWrapperData ::
  checkColumnsValid ()
  {
    for (auto& column : m_wrapper->m_columns)
      checkColumn (column);
  }



  void ColumnarToolWrapperData ::
  call ()
  {
    checkColumnsValid ();
    m_wrapper->m_tool->callVoid (m_data.data());
  }



  void ColumnarToolWrapperData ::
  checkColumn (const std::pair<const std::string,ColumnarToolWrapper::MyColumnInfo>& column)
  {
    if (m_columnIsChecked.at(column.second.index))
      return;
    if (!m_columnIsFilled.at(column.second.index))
    {
      if (!column.second.isOptional)
        throw std::runtime_error ("column not filled: " + column.first);
      m_columnIsChecked[column.second.index] = true;
      return;
    }

    ColumnarOffsetType expectedSize = 1u;
    if (column.second.offsets)
    {
      checkColumn (*column.second.offsets);
      if (m_data[column.second.offsets->second.index] == nullptr)
        throw std::runtime_error ("offset column not filled: " + column.second.offsets->first);
      const auto offsetIndex = column.second.offsets->second.index;
      auto *offsetsPtr = static_cast<const ColumnarOffsetType*>(m_data[offsetIndex]);
      expectedSize = offsetsPtr[m_dataSize[offsetIndex]-1];
    }
    expectedSize *= column.second.fixedDimensions;

    if (column.second.isOffset)
      expectedSize += 1u;

    if (m_dataSize[column.second.index] != expectedSize)
      throw std::runtime_error ("column size doesn't match expected size: " + column.first + ", found " + std::to_string (m_dataSize[column.second.index]) + " vs " + std::to_string (expectedSize));

    if (column.second.isOffset)
    {
      auto *dataPtr = static_cast<const ColumnarOffsetType*>(m_data[column.second.index]);
      if (dataPtr[0] != 0)
        throw std::runtime_error ("offset column doesn't start with 0: " + column.first);
    }

    m_columnIsChecked[column.second.index] = true;
  }



  std::vector<std::string> ColumnarToolWrapper ::
  getColumnNames () const
  {
    std::vector<std::string> result;
    for (auto& column : m_columns)
      result.push_back (column.first);
    std::sort (result.begin(), result.end());
    return result;
  }



  [[nodiscard]] std::vector<ColumnInfo> ColumnarToolWrapper ::
  getColumnInfo () const
  {
    return m_tool->getColumnInfo();
  }
}
