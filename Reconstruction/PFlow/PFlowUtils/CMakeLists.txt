# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PFlowUtils )

# Component(s) in the package:
atlas_add_library( PFlowUtilsLib
   PFlowUtils/*.h Root/*.cxx
   PUBLIC_HEADERS PFlowUtils
   LINK_LIBRARIES AsgTools xAODPFlow )

atlas_add_dictionary( PFlowUtilsDict
   PFlowUtils/PFlowUtilsDict.h
   PFlowUtils/selection.xml
   LINK_LIBRARIES PFlowUtilsLib )


if( NOT XAOD_ANALYSIS )
   atlas_add_component( PFlowUtils
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AthContainers AthenaBaseComps xAODJet xAODPFlow xAODTruth
      xAODCore PFlowUtilsLib CaloCalibHitRecLib CaloSimEvent StoreGateLib )
endif()

if(  XAOD_ANALYSIS AND NOT XAOD_STANDALONE )
   atlas_add_component( PFlowUtils
     src/components/*.cxx
     LINK_LIBRARIES AthContainers AthenaBaseComps xAODJet xAODPFlow xAODTruth
     xAODCore PFlowUtilsLib )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

