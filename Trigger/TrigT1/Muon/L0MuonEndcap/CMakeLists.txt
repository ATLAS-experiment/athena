# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonEndcap )

atlas_add_component( L0MuonEndcap
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib
                                    TrigT1MuctpiBits xAODL0MuonCand
                                    xAODMuonViews xAODTrigL0Muon )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( L0MuonEndcapConfig
                SCRIPT python -m L0MuonEndcap.L0MuonEndcapConfig
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TgcL0SectorLogicWordEncoder_test
                SOURCES test/TgcL0SectorLogicWordEncoder_test.cxx
                        src/TgcL0SectorLogicWordEncoder.cxx
                INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                LINK_LIBRARIES AthContainers TrigT1MuctpiBits
                               xAODL0MuonCand xAODTrigL0Muon
                POST_EXEC_SCRIPT noerror.sh )
