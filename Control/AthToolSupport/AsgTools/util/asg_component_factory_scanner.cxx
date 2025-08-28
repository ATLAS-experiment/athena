/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgTools/AsgComponentFactories.h>
#include <AsgTools/MessageCheckAsgTools.h>
#include <fstream>

//
// program implementation
//

using namespace asg;
using namespace asg::msgComponentConfig;

/// @file asg_component_factory_scanner.cxx
///
/// IMPORTANT: this file is only used in AnalysisBase, in any
/// Athena-based project the corresponding Gaudi mechanism is used
/// instead.
///
/// This program scans a component library and provides a map of all the
/// contained components. The files can then be merged in CMake by
/// simply concatenating the files. The component factory system knows
/// how to find and read those files, and can then use them to load the
/// corresponding component library on demand and create components from
/// them. By and large all of this is handled by AtlasCMake and users
/// shouldn't have to call it manually.
///
/// The way this works internally is that the program loads the
/// component library and then reports all known component factories as
/// belonging to that component library. Should the library internally
/// link another component library this would report components from
/// both libraries. However, that is generally not allowed in Athena
/// either, so not having a special handler in AnalysisBase ought not to
/// create a problem there either.

int main (int argc, char **argv)
{
  if (argc != 3)
  {
    ATH_MSG_ERROR ("usage: " << argv[0] << " <module path> <output file>");
    return 1;
  }

  const std::string modulePathName = argv[1];
  const std::string outputFile = argv[2];

  std::string modulePath, moduleName;
  {
    const auto pos = modulePathName.find_last_of ('/');
    if (pos != std::string::npos)
    {
      modulePath = modulePathName.substr (0, pos);
      moduleName = modulePathName.substr (pos + 1);
    }
    else
    {
      modulePath = ".";
      moduleName = modulePathName;
    }
  }

  if (auto types = getLoadedComponentFactoryTypes (); !types.empty())
  {
    ANA_MSG_ERROR ("already loaded component factory types:");
    for (const auto& type : types)
      ANA_MSG_ERROR ("  " << type);
    return 1;
  }

  ATH_MSG_DEBUG ("loading component factory preloader module from " << modulePath << " with name " << moduleName);
  loadComponentFactoryModule (moduleName, modulePath);

  std::ofstream outputStream (outputFile);
  for (auto& type : getLoadedComponentFactoryTypes ())
  {
    ANA_MSG_DEBUG ("module loaded type " << type);
    outputStream << moduleName << " " << type << "\n";
  }

  return 0;
}