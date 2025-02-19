# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetDefectsEmulation )

find_package( CLHEP )
find_package( ROOT COMPONENTS Hist )

# Component(s) in the package:
atlas_add_component( InDetDefectsEmulation
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AtlasDetDescr  GaudiKernel
                     Identifier InDetIdentifier InDetRawData InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry StoreGateLib
                     SCT_ConditionsToolsLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
