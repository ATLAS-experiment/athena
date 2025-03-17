# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L0MuonMDT )

atlas_add_component (L0MuonMDT
        src/*.cxx src/components/*.cxx
        LINK_LIBRARIES AthenaKernel xAODMuonRDO MuonRDO MuonDigitContainer xAODTrigger AthenaMonitoringKernelLib
        MuonCablingData xAODTrigger)


atlas_add_test( RunTest
        SCRIPT python -m L0MuonMDT.L0MuonMDTConfig --nEvents 2
                PRIVATE_WORKING_DIRECTORY
              	POST_EXEC_SCRIPT noerror.sh)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )


