# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L0MuonS1RPC )

find_package( CLHEP )

atlas_add_component (L0MuonS1RPC
        src/*.cxx src/components/*.cxx
        INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
	LINK_LIBRARIES AnaAlgorithmLib AthenaKernel xAODMuonRDO MuonDigitContainer xAODTrigger AthenaMonitoringKernelLib
	MuonCablingData xAODTrigger L0MuonInterface
        PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} )


atlas_add_test( RunTest
                SCRIPT python -m L0MuonS1RPC.L0MuonS1RPCConfig --nEvents 2 
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )


