# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsMuonDetectorTest
################################################################################

atlas_subdir( ActsMuonDetectorTest )

atlas_add_component( ActsMuonDetectorTest
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib  GaudiKernel MuonTesterTreeLib
                                     ActsGeometryLib MuonReadoutGeometryR4  MuonTruthHelpers
                                     xAODTruth xAODMuonSimHit CxxUtils ActsInteropLib)


atlas_add_test( testMuonTrackingGeometry
                SCRIPT python -m ActsMuonDetectorTest.testMuonTrackingGeometry --gen3
                PROPERTIES TIMEOUT 1000
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

