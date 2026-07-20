# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L0MuonNSW )

atlas_add_component( L0MuonNSW
        src/*.cxx src/components/*.cxx
        LINK_LIBRARIES AthenaKernel 
                       MuonDigitContainer 
                       xAODL0MuonCand            
                       AthenaMonitoringKernelLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )

atlas_add_test( L0MuonNSWConfig
                SCRIPT python -m L0MuonNSW.L0MuonNSWConfig
                POST_EXEC_SCRIPT noerror.sh )
