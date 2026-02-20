# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigAnalysisHelpers )

atlas_add_library( TrigAnalysisHelpersLib
   TrigAnalysisHelpers/*.h Root/*.cxx
   PUBLIC_HEADERS TrigAnalysisHelpers
   LINK_LIBRARIES AsgMessagingLib TrigDecisionInterface TrigCompositeUtilsLib )

# Unit test(s).
if( NOT XAOD_STANDALONE )
  atlas_add_test( TrigTraversal_test
    SOURCES test/TrigTraversal_test.cxx
    LINK_LIBRARIES TestTools StoreGateLib AthenaKernel GaudiKernel SGTools TrigAnalysisHelpersLib
    CxxUtils xAODTrigger xAODEgamma xAODMuon xAODBase TrigCompositeUtilsLib )
endif()
