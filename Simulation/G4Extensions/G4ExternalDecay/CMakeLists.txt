# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( G4ExternalDecay )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( Lhapdf )
find_package( Pythia8 )
find_package( XercesC )

set( extra_libs )
if( HEPMC_FOUND )
   list( APPEND extra_includes  )
   list( APPEND extra_libs AtlasHepMCLib )
endif()

# Component(s) in the package:
atlas_add_library( G4ExternalDecay
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS G4ExternalDecay
                   INCLUDE_DIRS ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${LHAPDF_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${PYTHIA8_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS 
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} ${PYTHIA8_LIBRARIES} ${LHAPDF_LIBRARIES} ${GEANT4_LIBRARIES} Pythia8_iLib ${extra_libs}
                   PRIVATE_LINK_LIBRARIES AtlasHepMCLib )
