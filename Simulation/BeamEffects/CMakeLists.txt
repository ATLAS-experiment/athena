# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BeamEffects )

# External dependencies:
find_package( CLHEP )
find_package( GTest )
find_package( Eigen )
#find_package( GMock )

set( _sources
  src/BeamEffectsAlg.cxx src/BeamEffectsAlg.h
  src/BeamSpotFixerAlg.cxx src/BeamSpotFixerAlg.h
  src/BeamSpotReweightingAlg.cxx src/BeamSpotReweightingAlg.h
  src/CrabKissingVertexPositioner.cxx src/CrabKissingVertexPositioner.h
  src/GenEventBeamEffectBooster.cxx src/GenEventBeamEffectBooster.h
  src/GenEventRotator.cxx src/GenEventRotator.h
  src/GenEventValidityChecker.cxx src/GenEventValidityChecker.h
  src/GenEventVertexPositioner.cxx src/GenEventVertexPositioner.h
  src/LongBeamspotVertexPositioner.cxx src/LongBeamspotVertexPositioner.h
  src/VertexBeamCondPositioner.cxx src/VertexBeamCondPositioner.h
  src/LRAVertexPositioner.cxx src/LRAVertexPositioner.h
  src/ZeroLifetimePositioner.cxx src/ZeroLifetimePositioner.h
)
set( _extra_libs )

if( NOT SIMULATIONBASE )
  set( _extra_libs xAODTracking )
  set( _sources ${_sources}
    src/MatchingBkgVertexPositioner.cxx src/MatchingBkgVertexPositioner.h
  )
endif()

# Needed to suppress warnings from eigen when compiling the test.
include_directories(SYSTEM ${EIGEN_INCLUDE_DIRS})

atlas_add_test( BeamEffectsAlg_test
                SOURCES ${_sources} test/BeamEffectsAlg_test.cxx
                INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib GaudiKernel TestTools AthenaBaseComps
                  StoreGateLib xAODEventInfo GeneratorObjects HepMC_InterfacesLib
                  BeamSpotConditionsData CxxUtils PathResolver ${GTEST_LIBRARIES} ${_extra_libs} )

#Added test of new component accumulator syntax
atlas_add_test( BeamEffectsAlgConfig_test
                SCRIPT python/BeamEffectsAlgConfig.py
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_component( BeamEffects
                     ${_sources}
                     src/components/*.cxx
                     INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib CxxUtils GaudiKernel AthenaBaseComps
                      HepMC_InterfacesLib StoreGateLib xAODEventInfo GeneratorObjects
                      BeamSpotConditionsData PathResolver ${_extra_libs} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
