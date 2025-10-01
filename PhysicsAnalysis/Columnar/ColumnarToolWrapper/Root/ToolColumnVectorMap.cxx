/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarToolWrapper/ToolColumnVectorMap.h>

#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <algorithm>
#include <stdexcept>

//
// method implementations
//

namespace columnar
{
  ToolColumnVectorMap ::
  ToolColumnVectorMap (ColumnVectorHeader& val_columnHeader, IColumnarTool& val_tool)
    : m_tool (&val_tool), m_columnHeader (&val_columnHeader)
  {
    auto toolColumns = m_tool->getColumnInfo();
    for (auto& column : toolColumns)
    {
      const std::size_t columnIndex = m_columnHeader->addColumn (column);

      MyColumnInfo myinfo;
      myinfo.index = columnIndex;
      val_tool.setColumnIndex (column.name, myinfo.index);

      auto [iter, success] = m_columns.emplace (column.name, std::move (myinfo));
      if (!success)
        throw std::runtime_error ("column name already registered: " + column.name);
    }

    for (auto& column : toolColumns)
    {
      if (!column.offsetName.empty())
      {
        auto offsetIter = m_columns.find (column.offsetName);
        if (offsetIter == m_columns.end())
          throw std::runtime_error ("offset column name not found: " + column.offsetName);
        m_columnHeader->setOffsetColumn (m_columns.at (column.name).index, offsetIter->second.index);
      }
    }

    // final consistency check
    m_columnHeader->checkSelf ();
  }



  std::vector<std::string> ToolColumnVectorMap ::
  getColumnNames () const
  {
    std::vector<std::string> result;
    for (auto& column : m_columns)
      result.push_back (column.first);
    std::sort (result.begin(), result.end());
    return result;
  }



  [[nodiscard]] std::size_t ToolColumnVectorMap ::
  getColumnIndex (const std::string& name) const
  {
    auto column = m_columns.find (name);
    if (column == m_columns.end())
      throw std::runtime_error ("trying to access unknown column " + name + " on tool");
    return column->second.index;
  }
}
