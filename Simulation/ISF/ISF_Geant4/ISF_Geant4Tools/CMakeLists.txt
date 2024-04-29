# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_Geant4Tools )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( TBB )

atlas_add_library( ISF_Geant4ToolsLib
                   ISF_Geant4Tools/*.h
                   INTERFACE
                   PUBLIC_HEADERS ISF_Geant4Tools
                   LINK_LIBRARIES CxxUtils GaudiKernel G4AtlasAlgLib )

# Component(s) in the package:
atlas_add_library( ISF_Geant4Tools
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AtlasHepMCLib ${GEANT4_LIBRARIES} ${TBB_LIBRARIES} AthenaKernel GaudiKernel G4AtlasInterfaces AthenaBaseComps StoreGateLib AtlasDetDescr GeneratorObjects G4AtlasToolsLib G4AtlasAlgLib MCTruth ISF_Event ISF_InterfacesLib ISF_Geant4Event ISF_Geant4ToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

