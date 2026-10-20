# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L1MuonEndcap )

atlas_add_component( L1MuonEndcap
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps StoreGateLib
                                    TrigT1MuctpiBits
                                    xAODMuonViews xAODTrigL1Muon )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( L1MuonEndcapConfig
                SCRIPT python -m L1MuonEndcap.L1MuonEndcapConfig
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonEndcap_SectorLogicWordEncoder_test
                SOURCES test/SectorLogicWordEncoder_test.cxx
                        src/SectorLogicWordEncoder.cxx
                INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                LINK_LIBRARIES AthContainers TrigT1MuctpiBits
                               xAODTrigL1Muon
                POST_EXEC_SCRIPT noerror.sh )
