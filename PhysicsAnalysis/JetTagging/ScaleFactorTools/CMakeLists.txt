# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ScaleFactorTools )

# External dependencies:
find_package( nlohmann_json )

atlas_add_library ( ScaleFactorToolsLib
  Root/*.cxx
  PUBLIC_HEADERS ScaleFactorTools
  LINK_LIBRARIES
    AsgTools
    PathResolver
    FTagAnalysisInterfacesLib
  PRIVATE_LINK_LIBRARIES CxxUtils nlohmann_json::nlohmann_json
)


atlas_add_component( ScaleFactorTools
   src/components/*.cxx
   LINK_LIBRARIES ScaleFactorToolsLib
)

atlas_add_executable(
  ScaleFactorToolTester
  util/ScaleFactorToolTester.cxx
  LINK_LIBRARIES xAODRootAccess AsgTools ScaleFactorToolsLib
)

