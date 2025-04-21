# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AtlasCLHEPDict )

# External dependencies:
find_package( CLHEP )

# Build the "main" dictionary.
atlas_add_dictionary( AtlasCLHEPDict
                      AtlasCLHEPDict/AtlasCLHEPDict.h
                      AtlasCLHEPDict/selection.xml
                      INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                      LINK_LIBRARIES ${CLHEP_LIBRARIES} )

# Select the name of the selection file for the CLHEP::Hep3Vector dictionary.
set( hep3VectorSelectionFile "AtlasCLHEPDict/selectionHep3Vector.xml" )
if( "${CLHEP_VERSION}" VERSION_GREATER_EQUAL "2.4.2" )
   set( hep3VectorSelectionFile "AtlasCLHEPDict/selectionHep3VectorCompat.xml" )
endif()

# Build the dictionary for CLHEP::Hep3Vector.
atlas_add_dictionary( AtlasCLHEPHep3VectorDict
   "AtlasCLHEPDict/AtlasCLHEPDict.h"
   "${hep3VectorSelectionFile}"
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} )
