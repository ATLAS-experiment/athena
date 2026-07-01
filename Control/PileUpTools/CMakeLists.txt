# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PileUpTools )

# External dependencies:
find_package( GTest )
find_package( UUID )

# Component(s) in the package:
atlas_add_library( PileUpToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS PileUpTools
                   PRIVATE_LINK_LIBRARIES xAODCore
                   LINK_LIBRARIES ${UUID_LIBRARIES} AthenaBaseComps AthenaKernel
                   AthLinks TruthUtils SGTools
                   EventInfo xAODEventInfo xAODCnvInterfaces
                   GaudiKernel StoreGateLib CxxUtils )

atlas_add_component( PileUpTools
                     src/components/*.cxx
                     LINK_LIBRARIES PileUpToolsLib )


atlas_add_test( PileUpHashHelper_test
                SOURCES test/PileUpHashHelper_test.cxx src/PileUpHashHelper.cxx
                INCLUDE_DIRS src ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} CxxUtils PileUpToolsLib
                POST_EXEC_SCRIPT noerror.sh )
