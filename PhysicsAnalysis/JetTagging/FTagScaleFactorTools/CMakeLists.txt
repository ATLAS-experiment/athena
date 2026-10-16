# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FTagScaleFactorTools )

# External dependencies:
find_package( nlohmann_json )

atlas_add_library ( FTagScaleFactorToolsLib
  Root/*.cxx 
  src/*.cxx
  PUBLIC_HEADERS FTagScaleFactorTools
  PRIVATE_INCLUDE_DIRS src
  LINK_LIBRARIES
    AsgTools
    PathResolver
    FTagAnalysisInterfacesLib
  PRIVATE_LINK_LIBRARIES CxxUtils nlohmann_json::nlohmann_json
)

atlas_add_component( FTagScaleFactorTools
  src/components/*.cxx
  LINK_LIBRARIES FTagScaleFactorToolsLib
)

atlas_add_dictionary( FTagScaleFactorToolsDict
  FTagScaleFactorTools/FTagScaleFactorToolsDict.h
  FTagScaleFactorTools/selection.xml
  LINK_LIBRARIES FTagScaleFactorToolsLib
)

if( XAOD_STANDALONE )
  set(XAOD_ACCESS_LIB xAODRootAccess)
else()
  set(XAOD_ACCESS_LIB POOLRootAccessLib)
endif()
atlas_add_executable(
  ScaleFactorToolTester
  util/ScaleFactorToolTester.cxx
  LINK_LIBRARIES xAODRootAccess AsgTools FTagScaleFactorToolsLib ${XAOD_ACCESS_LIB}
)

