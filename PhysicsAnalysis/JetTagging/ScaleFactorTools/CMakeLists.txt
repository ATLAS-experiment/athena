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

if( NOT XAOD_STANDALONE )
  atlas_add_component( ScaleFactorTools
    src/*.h
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES ScaleFactorToolsLib
  )
else()
  atlas_add_component( ScaleFactorTools
    src/components/*.cxx
    LINK_LIBRARIES ScaleFactorToolsLib
  )
endif()

atlas_add_dictionary( ScaleFactorToolsDict
  ScaleFactorTools/ScaleFactorToolsDict.h
  ScaleFactorTools/selection.xml
  LINK_LIBRARIES ScaleFactorToolsLib
)

atlas_add_executable(
  ScaleFactorToolTester
  util/ScaleFactorToolTester.cxx
  LINK_LIBRARIES xAODRootAccess AsgTools ScaleFactorToolsLib
)

