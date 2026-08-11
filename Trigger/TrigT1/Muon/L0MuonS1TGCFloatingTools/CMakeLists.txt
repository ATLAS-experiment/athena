# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonS1TGCFloatingTools )

atlas_add_component( L0MuonS1TGCFloatingTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils L0MuonS1TGCToolInterfaces
                                    FourMomUtils MuonIdHelpersLib MuonReadoutGeometry
                                    MuonRDO MuonTGC_CablingLib StoreGateLib )

atlas_add_test( TgcL0FloatingPtLut_test
  SOURCES test/TgcL0FloatingPtLut_test.cxx
          src/TgcL0FloatingPtLut.cxx
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
  LINK_LIBRARIES CxxUtils
  POST_EXEC_SCRIPT nopost.sh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
