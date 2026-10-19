# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L1MuonS1TGCFloatingTools )

find_package( Boost COMPONENTS unit_test_framework )

atlas_add_component( L1MuonS1TGCFloatingTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils L1MuonS1TGCToolInterfaces
                                    FourMomUtils MuonIdHelpersLib MuonReadoutGeometry
                                    MuonRDO MuonTGC_CablingLib PathResolver StoreGateLib )

atlas_add_test( L1MuonS1TGC_FloatingPtLut_test
  SOURCES test/FloatingPtLut_test.cxx
          src/FloatingPtLut.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
  LINK_LIBRARIES CxxUtils
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonS1TGC_GoodMagMap_test
  SOURCES test/GoodMagMap_test.cxx
          src/GoodMagMap.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
  LINK_LIBRARIES CxxUtils
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonS1TGC_FloatingSegmentReconstruction_test
  SOURCES test/FloatingSegmentReconstruction_test.cxx
          src/FloatingData.cxx
          src/FloatingSegmentReconstruction.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} FourMomUtils L1MuonS1TGCToolInterfaces
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonS1TGC_FloatingTrackSelector_test
  SOURCES test/FloatingTrackSelector_test.cxx
          src/FloatingTrackSelector.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} AthContainers
                 L1MuonS1TGCToolInterfaces xAODTrigL1Muon
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1MuonS1TGC_FloatingInnerCoincidence_test
  SOURCES test/FloatingInnerCoincidence_test.cxx
          src/FloatingInnerCoincidence.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} L1MuonS1TGCToolInterfaces
  POST_EXEC_SCRIPT noerror.sh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
