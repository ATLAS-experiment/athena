# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetDefectsEmulation )

find_package( CLHEP )
find_package( ROOT COMPONENTS Hist ROOTNTuple)
find_package( Acts COMPONENTS Core )
find_package( nlohmann_json )
find_package( VDT )

# Component(s) in the package:
atlas_add_component( InDetDefectsEmulation
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}  ${VDT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AtlasDetDescr  GaudiKernel
                     Identifier InDetIdentifier InDetRawData InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry StoreGateLib
                     SCT_ConditionsToolsLib Acts::Core ActsGeometryLib ActsGeometryInterfacesLib nlohmann_json::nlohmann_json )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( EmulatedDefectsBuilder_test
                SOURCES test/EmulatedDefectsBuilder_test.cxx
                INCLUDE_DIRS
                LINK_LIBRARIES InDetReadoutGeometry PixelReadoutGeometryLib
                POST_EXEC_SCRIPT nopost.sh )
