# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_Event )

# External dependencies:
find_package( CLHEP )
find_package( Eigen )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( ISF_Event
                   src/*.cxx
                   PUBLIC_HEADERS ISF_Event
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}  ${EIGEN_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib AtlasHepMCsearchLib ${EIGEN_LIBRARIES} TestTools AthenaBaseComps AtlasDetDescr BarcodeEventLib GeoPrimitives GeneratorObjects GaudiKernel CxxUtils TruthUtils )

atlas_add_test( ISFParticle_test
                   SOURCES test/ISFParticle_test.cxx
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}  ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib AtlasHepMCsearchLib ${EIGEN_LIBRARIES} TestTools AthenaBaseComps AtlasDetDescr GeoPrimitives GeneratorObjects GaudiKernel ISF_Event CxxUtils
                   POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( ISFTruthIncident_test
                  SOURCES test/ISFTruthIncident_test.cxx
                  INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                  LINK_LIBRARIES ${CLHEP_LIBRARIES}  AtlasHepMCLib AtlasHepMCsearchLib ${EIGEN_LIBRARIES} ${GTEST_LIBRARIES} CxxUtils GeneratorObjects TestTools AthenaBaseComps AtlasDetDescr GeoPrimitives GeneratorObjects GaudiKernel ISF_Event CxxUtils
                  POST_EXEC_SCRIPT nopost.sh )
