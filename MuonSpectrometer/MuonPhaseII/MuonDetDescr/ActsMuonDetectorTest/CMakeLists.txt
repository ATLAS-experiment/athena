# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsMuonDetectorTest
################################################################################

atlas_subdir( ActsMuonDetectorTest )

atlas_add_component( ActsMuonDetectorTest
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib GeoModelUtilities MuonTesterTreeLib
                                    GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib ActsGeometryLib AthenaPoolUtilities
                                    xAODTruth xAODMuonSimHit CxxUtils TruthUtils)

atlas_add_test( testMuonDetectorNav
                SCRIPT python -m ActsMuonDetectorTest.testMuonDetector
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

