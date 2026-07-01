# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


atlas_subdir( SystematicsHandles )

atlas_add_library( SystematicsHandlesLib
   SystematicsHandles/*.h SystematicsHandles/*.icc Root/*.cxx
   PUBLIC_HEADERS SystematicsHandles
   LINK_LIBRARIES AsgTools PATInterfaces AnaAlgorithmLib xAODBase xAODCore AthContainers xAODEventInfo xAODMissingET
   AsgServicesLib AsgDataHandlesLib CxxUtils
   PRIVATE_LINK_LIBRARIES xAODJet xAODMuon xAODEgamma xAODTau )

atlas_add_dictionary( SystematicsHandlesDict
   SystematicsHandles/SystematicsHandlesDict.h
   SystematicsHandles/selection.xml
   LINK_LIBRARIES SystematicsHandlesLib )

atlas_add_component( SystematicsHandles
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES SystematicsHandlesLib )

if( XAOD_STANDALONE )
   atlas_add_test( ut_CopyHelpers
      SOURCES test/ut_CopyHelpers.cxx
      LINK_LIBRARIES xAODRootAccess AsgTools xAODBase xAODJet
      SystematicsHandlesLib
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( gt_SystematicsHandles
      SOURCES test/gt_SystematicsHandles.cxx
      LINK_LIBRARIES xAODRootAccess AsgTools AsgServicesLib AsgTestingLib
      xAODJet SystematicsHandlesLib
      POST_EXEC_SCRIPT noerror.sh )
endif()
