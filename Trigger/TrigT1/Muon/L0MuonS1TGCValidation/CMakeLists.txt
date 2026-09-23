# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonS1TGCValidation )

atlas_add_library( L0MuonS1TGCValidationLib
                   INTERFACE
                   PUBLIC_HEADERS L0MuonS1TGCValidation
                   LINK_LIBRARIES AthenaKernel )

atlas_add_component( L0MuonS1TGCValidation
  src/*.cxx
  src/components/*.cxx
  LINK_LIBRARIES
    AthenaBaseComps
    AtlasHepMCLib
    EventPrimitives
    FourMomUtils
    GaudiKernel
    GeneratorObjects
    L1MuonS1TGCToolInterfaces
    L0MuonS1TGCValidationLib
    MuonTesterTreeLib
    StoreGateLib
    TrkEventPrimitives
    TrkExInterfaces
    TrkParameters
    TrkSurfaces
    xAODL0MuonCand
    xAODTrigL0Muon )

# Protect the ROOT-independent validation-data contract with a lightweight
# unit test.  This test has no ROOT, extrapolator, or reconstruction dependency.
atlas_add_test( TgcL0ValidationEventCheck_test
  SOURCES test/TgcL0ValidationEventCheck_test.cxx
  LINK_LIBRARIES L0MuonS1TGCValidationLib
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L0MuonS1TGCValidationConfig
  SCRIPT python -m L0MuonS1TGCValidation.L0MuonS1TGCValidationConfig --nEvents 1
  POST_EXEC_SCRIPT noerror.sh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
