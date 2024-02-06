# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TgcOverlay )

#External dependencies:
find_package( GTest )

atlas_add_test( TgcOverlay_test
                SOURCES src/*.cxx test/TgcOverlay_test.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES CxxUtils GaudiKernel TestTools MuonDigitContainer MuonOverlayBase AthenaKernel StoreGateLib MuonIdHelpersLib ${GTEST_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh )

# Component(s) in the package:
atlas_add_component( TgcOverlay
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel MuonDigitContainer MuonOverlayBase AthenaKernel StoreGateLib MuonIdHelpersLib )
