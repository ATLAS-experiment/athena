# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TruthUtils )

# Decide what to do about HepMC(3) / AtlasHepMC.
set( extra_libs )
if( NOT XAOD_STANDALONE )
   list( APPEND extra_libs AtlasHepMCLib )
   message( STATUS "Linking TruthUtils against AtlasHepMC" )
endif()

# Component(s) in the package:
atlas_add_library( TruthUtils
   INTERFACE
   PUBLIC_HEADERS TruthUtils
   LINK_LIBRARIES  ${extra_libs}
   )

# Install files from the package:
atlas_install_runtime( share/*.txt )

atlas_add_test( AtlasPIDTest
                SOURCES
                test/testPIDs.cxx 
                LINK_LIBRARIES TruthUtils
                ENVIRONMENT "REFERENCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/share/AtlasPIDTest.ref"
                POST_EXEC_SCRIPT nopost.sh )
atlas_add_executable( pdgInfo
                test/pdgInfo.cxx
                LINK_LIBRARIES TruthUtils )
