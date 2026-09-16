# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonS1TGC )

find_package( Boost COMPONENTS unit_test_framework )

atlas_add_component( L0MuonS1TGC
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringKernelLib
                                    L0MuonS1TGCToolInterfaces MuonRDO StoreGateLib
                                    xAODL0MuonCand xAODMuonViews )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( TgcL0MdtCandidateSelector_test
  SOURCES test/TgcL0MdtCandidateSelector_test.cxx
          src/TgcL0MdtCandidateSelector.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} AthContainers xAODL0MuonCand
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( L0MuonS1TGCConfig
                SCRIPT python -m L0MuonS1TGC.L0MuonS1TGCConfig
                POST_EXEC_SCRIPT nopost.sh )
