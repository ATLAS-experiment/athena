# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the name of the package:
atlas_subdir( TruthRivetTools )

# External dependencies:
find_package( Rivet )
find_package( YODA )
find_package( FastJet )
find_package( FastJetContrib )
find_package( ROOT COMPONENTS Core Physics )
find_package( GSL )
find_package( HighFive QUIET )   #  Public dependency of Rivet 4.0.0
find_package( HDF5 )             # Private dependency of Rivet 4.0.0

# Remove the --as-needed linker flags:
atlas_disable_as_needed()

# Component(s) in the package:
atlas_add_library( TruthRivetToolsLib
	Root/*.cxx TruthRivetTools/*.h
   SHARED
   PUBLIC_HEADERS TruthRivetTools
   INCLUDE_DIRS ${RIVET_INCLUDE_DIRS} ${YODA_INCLUDE_DIRS}
   ${FASTJET_INCLUDE_DIRS} ${FASTJETCONTRIB_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS}
	LINK_LIBRARIES ${RIVET_LIBRARIES} ${YODA_LIBRARIES}
   ${FASTJET_LIBRARIES} ${FASTJETCONTRIB_LIBRARIES} ${ROOT_LIBRARIES} ${GSL_LIBRARIES} ${HDF5_LIBRARIES} AsgTools AtlasHepMCLib AtlasHepMCsearchLib
   CxxUtils GenInterfacesLib )
if( HIGHFIVE_FOUND )
   target_include_directories( TruthRivetToolsLib
      PUBLIC ${HIGHFIVE_INCLUDE_DIRS} )
endif()

atlas_add_component( TruthRivetTools
   src/components/*.cxx
   LINK_LIBRARIES TruthRivetToolsLib GaudiKernel )

atlas_add_dictionary( TruthRivetToolsDict
   TruthRivetTools/TruthRivetToolsDict.h TruthRivetTools/selection.xml
   LINK_LIBRARIES TruthRivetToolsLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
