/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarToolWrapper/ColumnarToolHelpers.h>

#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>

//
// method implementations
//

namespace columnar
{
  void renameContainers (IColumnarTool& tool, const std::vector<std::pair<std::string,std::string>>& renames)
  {
    if (!renames.empty())
    {
      auto columnInfo = tool.getColumnInfo ();
      for (auto& [from, to] : renames)
      {
        for (auto& column : columnInfo)
        {
          if (column.name.starts_with (from) && (column.name.size() == from.size() || column.name[from.size()] == '.'))
          {
            std::string newName = to + column.name.substr (from.size());
            tool.renameColumn (column.name, newName);
          }
        }
      }
    }
  }
}
