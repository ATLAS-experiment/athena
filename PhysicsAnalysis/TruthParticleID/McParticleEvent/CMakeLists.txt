# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( McParticleEvent )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( McParticleEvent
   McParticleEvent/*.h src/*.cxx
   PUBLIC_HEADERS McParticleEvent
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib TruthUtils  AthenaKernel AthContainers AthLinks Navigation
   EventKernel NavFourMom GeneratorObjects ParticleEvent VxVertex )

atlas_add_dictionary( McParticleEventDict
   McParticleEvent/McParticleEventDict.h McParticleEvent/selection.xml
   LINK_LIBRARIES McParticleEvent
   NAVIGABLES TruthParticleContainer 
   ELEMENT_LINKS McEventCollection TruthEtIsolationsContainer )

# Helper variable for the test:
set( _jobOPath "${CMAKE_CURRENT_SOURCE_DIR}/share" )
set( _jobOPath "${_jobOPath}:${CMAKE_JOBOPT_OUTPUT_DIRECTORY}" )
set( _jobOPath "${_jobOPath}:$ENV{JOBOPTSEARCHPATH}" )

# Test(s) in the package:
atlas_add_test( TruthParticle_test
   SOURCES test/TruthParticle_test.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} 
   LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib GeneratorObjects
   StoreGateLib TestTools GaudiKernel McParticleEvent
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )
