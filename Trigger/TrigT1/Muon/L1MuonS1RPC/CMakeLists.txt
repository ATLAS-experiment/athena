# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L1MuonS1RPC )

find_package( CLHEP )

atlas_add_library( L1MuonS1RPCLib
    L1MuonS1RPC/*.h L1MuonS1RPC/*.cxx
    src/*.h
    src/*.cxx
    INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
    PUBLIC_HEADERS L1MuonS1RPC
    LINK_LIBRARIES AthenaBaseComps GaudiKernel AnaAlgorithmLib AthenaKernel StoreGateLib xAODMuonRDO MuonDigitContainer AthenaMonitoringKernelLib
	MuonCablingData xAODTrigger xAODTrigL1Muon L1MuonInterface GeneratorObjects xAODMuonSimHit xAODMuon xAODTruth MuonTruthHelpers MuonReadoutGeometryR4
        PathResolver
        PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} 
)

atlas_add_component (L1MuonS1RPC
        src/components/*.cxx
        INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
	LINK_LIBRARIES AthenaBaseComps GaudiKernel AnaAlgorithmLib AthenaKernel StoreGateLib xAODMuonRDO MuonDigitContainer AthenaMonitoringKernelLib
	MuonCablingData xAODTrigger xAODTrigL1Muon L1MuonInterface GeneratorObjects xAODMuonSimHit xAODMuon xAODTruth MuonTruthHelpers MuonReadoutGeometryR4
        L1MuonS1RPCLib
        PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} )

# C++ Unit test for RPCSLStripConverter
atlas_add_test( RPCSLStripConverterTest
    SOURCES test/test_RPCSLStripConverter.cxx
    LINK_LIBRARIES L1MuonS1RPCLib AthenaKernel GaudiKernel MuonStationIndexLib
    POST_EXEC_SCRIPT noerror.sh 
)

# Python test for algorithms 
atlas_add_test( RunTest
                SCRIPT python -m L1MuonS1RPC.S1RPCConfig --nEvents 2 
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

# atlas_add_test( RPCPatternMatchingTest
#                SCRIPT python -m L1MuonS1RPC.RPCPatternMatchingConfig --nEvents 2
#                PROPERTIES TIMEOUT 600
#                PRIVATE_WORKING_DIRECTORY
#                POST_EXEC_SCRIPT noerror.sh)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )


