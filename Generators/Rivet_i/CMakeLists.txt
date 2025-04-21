# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Rivet_i )

# External dependencies:
find_package( FastJet )
find_package( FastJetContrib COMPONENTS RecursiveTools )
find_package( ROOT COMPONENTS Core Hist )
find_package( Rivet )
find_package( YODA )
find_package( HighFive QUIET )   # Private dependency of Rivet 4.0.0
find_package( HDF5 )             # Private dependency of Rivet 4.0.0
find_package( Boost )

# Remove the --as-needed linker flags:
atlas_disable_as_needed()

# Component(s) in the package:
atlas_add_component( Rivet_i
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS} ${FASTJETCONTRIB_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
    ${RIVET_INCLUDE_DIRS} ${YODA_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${FASTJET_LIBRARIES} ${FASTJETCONTRIB_LIBRARIES} ${ROOT_LIBRARIES} AtlasHepMCLib TruthUtils
   ${RIVET_LIBRARIES} ${YODA_LIBRARIES} ${Boost_LIBRARIES} AthenaBaseComps CxxUtils GaudiKernel GenInterfacesLib
   AthenaKernel GeneratorObjects PathResolver xAODEventInfo )
if( HIGHFIVE_FOUND )
   target_include_directories( Rivet_i
      PRIVATE ${HIGHFIVE_INCLUDE_DIRS} )
endif()

# Install file(s) from the package.
atlas_install_scripts( share/setup* )

# Set up the runtime environment of Rivet.
set( RivetEnvironment_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   CACHE PATH "Location of RivetEnvironmentConfig.cmake" )
find_package( RivetEnvironment )
