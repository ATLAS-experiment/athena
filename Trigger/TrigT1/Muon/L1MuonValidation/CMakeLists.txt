################################################################################
# Package: L1MuonValidation
################################################################################

# Declare the package name:
atlas_subdir( L1MuonValidation )

# Common validation infrastructure.  Keep this library free of dependencies on
# TGC/RPC/MDT/MUCTPI implementation packages; subsystem content is injected via
# IValidationBranchTool implementations.
atlas_add_library( L1MuonValidationLib
                   INTERFACE
                   PUBLIC_HEADERS L1MuonValidation
                   LINK_LIBRARIES GaudiKernel MuonTesterTreeLib xAODMuon )

atlas_add_component( L1MuonValidation
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AtlasHepMCLib GaudiKernel
                                    GeneratorObjects L1MuonValidationLib
                                    MuonTesterTreeLib StoreGateLib xAODMuon )

atlas_add_test( TruthMuonSelection_test
                SOURCES test/TruthMuonSelection_test.cxx
                LINK_LIBRARIES L1MuonValidationLib
                POST_EXEC_SCRIPT noerror.sh )

# Install python modules:
atlas_install_python_modules( python/*.py )

atlas_add_test( FullChainTrigger
                SCRIPT python -m L1MuonValidation.FullChainTrigger
                       --threads 1 --nEvents 1
                POST_EXEC_SCRIPT noerror.sh )

# Verify composition using real Athena component dictionaries, without I/O.
atlas_add_test( MuonValidationReferenceConfig_test
  SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/MuonValidationReferenceConfig_test.py
  POST_EXEC_SCRIPT noerror.sh )
