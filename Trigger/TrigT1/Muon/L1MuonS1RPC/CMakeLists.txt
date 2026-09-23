# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L1MuonS1RPC )

find_package( CLHEP )

atlas_add_component (L1MuonS1RPC
        src/*.cxx src/components/*.cxx
        INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
	LINK_LIBRARIES AnaAlgorithmLib AthenaKernel xAODMuonRDO MuonDigitContainer AthenaMonitoringKernelLib
	MuonCablingData xAODTrigger L1MuonInterface GeneratorObjects xAODMuonSimHit xAODMuon xAODL0MuonCand xAODTruth MuonTruthHelpers MuonReadoutGeometryR4
        PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} )


atlas_add_test( RunTest
                SCRIPT python -m L1MuonS1RPC.L0MuonS1RPCConfig --nEvents 2
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )


