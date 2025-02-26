# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L0MuonS1TGC )

atlas_add_component( L0MuonS1TGC
        src/*.cxx src/components/*.cxx
        LINK_LIBRARIES AthenaKernel MuonDigitContainer xAODTrigger MuonTGC_CablingLib AthenaMonitoringKernelLib)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )

atlas_add_test( L0MuonS1TGCConfig
                SCRIPT python -m L0MuonS1TGC.L0MuonS1TGCConfig
                POST_EXEC_SCRIPT noerror.sh)
