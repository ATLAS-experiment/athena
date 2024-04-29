# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FatrasToolsG4 )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( ISF_FatrasToolsG4
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}  ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES}  ${CLHEP_LIBRARIES} AtlasHepMCLib ${GEANT4_LIBRARIES} AthenaBaseComps AthenaKernel BarcodeEventLib CxxUtils GaudiKernel ISF_Event ISF_FatrasInterfaces TrkEventPrimitives StoreGateLib ISF_InterfacesLib ISF_Geant4ToolsLib TrkGeometry TruthUtils )
