# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( D3PDMakerTest )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( D3PDMakerTestLib
                   src/*.cxx
                   PUBLIC_HEADERS D3PDMakerTest
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES AthContainers AthenaBaseComps AthenaKernel D3PDMakerUtils Navigation FourMom ParticleEvent
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} EventKernel NavFourMom MuonSimEvent TrackRecordLib )

atlas_add_component( D3PDMakerTest
                     src/components/*.cxx
                     LINK_LIBRARIES D3PDMakerTestLib )

atlas_add_dictionary( D3PDMakerTestDict
                      D3PDMakerTest/D3PDMakerTestDict.h
                      D3PDMakerTest/selection.xml
                      LINK_LIBRARIES D3PDMakerTestLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests
set( D3PDMAKERTEST_REFERENCE_TAG D3PDMakerTest/D3PDMakerTest-01-00-00 ) 

string( CONCAT ignore
   "^ *[^ ]+ +INFO |^Py:.* ::? +INFO"  # ignore INFO messages
   "|^.Entries :"
   "|no dictionary for class AttributeListLayout|leaving with code 0" )

atlas_add_test( D3PDTest1
  SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/D3PDTest1.py
  PROPERTIES TIMEOUT 600
  ENVIRONMENT "ATLAS_REFERENCE_TAG=${D3PDMAKERTEST_REFERENCE_TAG}"
  LOG_IGNORE_PATTERN ${ignore} )
