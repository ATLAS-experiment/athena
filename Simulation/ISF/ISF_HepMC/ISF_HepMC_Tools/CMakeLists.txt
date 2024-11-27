# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_HepMC_Tools )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( GTest )
find_package( GMock )

# Component(s) in the package:
atlas_add_component( ISF_HepMC_Tools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AtlasHepMCLib GaudiKernel AtlasDetDescr AthenaBaseComps TruthUtils ISF_Event ISF_HepMC_Interfaces ISF_InterfacesLib PathResolver )

# Tests
atlas_add_test( GenParticleGenericFilter_test
                SOURCES test/GenParticleGenericFilter_test.cxx src/GenParticleGenericFilter.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS} 
                LINK_LIBRARIES ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} AtlasHepMCLib AthenaBaseComps GoogleTestTools ISF_HepMC_Interfaces CxxUtils
                POST_EXEC_SCRIPT nopost.sh )

# Tests
atlas_add_test( GenParticleInteractingFilter_test
                SOURCES test/GenParticleInteractingFilter_test.cxx src/GenParticleInteractingFilter.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS} 
                LINK_LIBRARIES ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} AtlasHepMCLib AthenaBaseComps GoogleTestTools TruthUtils ISF_HepMC_Interfaces CxxUtils
                POST_EXEC_SCRIPT nopost.sh )
  # Needed for the dynamic_cast to work properly with clang.
set_target_properties( ISF_HepMC_Tools_GenParticleInteractingFilter_test PROPERTIES ENABLE_EXPORTS True )

# Tests
atlas_add_test( GenParticleLifetimeFilter_test
                SOURCES test/GenParticleLifetimeFilter_test.cxx src/GenParticleLifetimeFilter.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS} 
                LINK_LIBRARIES ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} AtlasHepMCLib AthenaBaseComps GoogleTestTools TruthUtils ISF_HepMC_Interfaces CxxUtils
                POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

