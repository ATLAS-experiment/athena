/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TOOL_WRAPPER_COLUMNAR_TOOL_HELPERS_H
#define COLUMNAR_TOOL_WRAPPER_COLUMNAR_TOOL_HELPERS_H

#include <string>
#include <vector>

namespace columnar
{
  class IColumnarTool;

  /// rename containers in the columnar tool
  ///
  /// The interface itself only allows renaming individual columns, but
  /// sometimes it is nice to rename a whole container.  This happens
  /// strictly on the basis of the name, i.e. it being prefixed with
  /// "Container.".
  void renameContainers (IColumnarTool& tool, const std::vector<std::pair<std::string,std::string>>& renames);
}

#endif