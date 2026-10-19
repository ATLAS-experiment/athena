# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L1MuonS1TGC )

find_package( Boost COMPONENTS unit_test_framework )

atlas_add_component( L1MuonS1TGC
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaMonitoringKernelLib
                                    L1MuonS1TGCToolInterfaces MuonRDO StoreGateLib
                                    xAODTrigL1Muon xAODMuonViews )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( L1MuonS1TGC_MdtCandSelector_test
  SOURCES test/MdtCandSelector_test.cxx
          src/MdtCandSelector.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} AthContainers xAODTrigL1Muon
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonS1TGCConfig
                SCRIPT python -m L1MuonS1TGC.L1MuonS1TGCConfig
                POST_EXEC_SCRIPT noerror.sh )
