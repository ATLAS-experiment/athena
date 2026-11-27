# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PFlowUtils )

# The ACTS calo parameters checker needs Athena/ACTS, so it is only built in Athena (not AnalysisBase)
file( GLOB pflowutils_srcs Root/*.cxx )
set( pflowutils_extra_libs )
if( XAOD_ANALYSIS )
   list( REMOVE_ITEM pflowutils_srcs ${CMAKE_CURRENT_SOURCE_DIR}/Root/ActsCaloParametersChecker.cxx )
else()
   set( pflowutils_extra_libs GaudiKernel ActsGeometryInterfacesLib CaloGeoHelpers )
endif()

# Component(s) in the package:
atlas_add_library( PFlowUtilsLib
   PFlowUtils/*.h ${pflowutils_srcs}
   PUBLIC_HEADERS PFlowUtils
   LINK_LIBRARIES AsgTools xAODEgamma xAODMuon xAODPFlow ${pflowutils_extra_libs} )

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

