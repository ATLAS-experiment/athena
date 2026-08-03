# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonEndcap )

atlas_add_component( L0MuonEndcap
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib
                                    xAODL0MuonCand xAODMuonViews xAODTrigL0Muon )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( L0MuonEndcapConfig
                SCRIPT python -m L0MuonEndcap.L0MuonEndcapConfig
                POST_EXEC_SCRIPT noerror.sh )
