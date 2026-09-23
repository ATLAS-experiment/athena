# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonS1TGCFloatingTools )

find_package( Boost COMPONENTS unit_test_framework )

atlas_add_component( L0MuonS1TGCFloatingTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils L1MuonS1TGCToolInterfaces
                                    FourMomUtils MuonIdHelpersLib MuonReadoutGeometry
                                    MuonRDO MuonTGC_CablingLib PathResolver StoreGateLib )

atlas_add_test( TgcL0FloatingPtLut_test
  SOURCES test/TgcL0FloatingPtLut_test.cxx
          src/TgcL0FloatingPtLut.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
  LINK_LIBRARIES CxxUtils
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TgcL0GoodMagMap_test
  SOURCES test/TgcL0GoodMagMap_test.cxx
          src/TgcL0GoodMagMap.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
  LINK_LIBRARIES CxxUtils
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TgcL0SegmentReconstruction_test
  SOURCES test/TgcL0SegmentReconstruction_test.cxx
          src/TgcL0FloatingData.cxx
          src/TgcL0SegmentReconstruction.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} FourMomUtils L1MuonS1TGCToolInterfaces
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( TgcL0TrackSelector_test
  SOURCES test/TgcL0TrackSelector_test.cxx
          src/TgcL0TrackSelector.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} AthContainers
                 L1MuonS1TGCToolInterfaces xAODL0MuonCand
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TgcL0InnerCoincidence_test
  SOURCES test/TgcL0InnerCoincidence_test.cxx
          src/TgcL0InnerCoincidence.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} L1MuonS1TGCToolInterfaces
  POST_EXEC_SCRIPT noerror.sh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
