# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauRecTools )

# External dependencies:
find_package( Eigen )
find_package( ROOT COMPONENTS Core Tree Hist RIO )
find_package( onnxruntime )

# Component(s) in the package:
atlas_add_library( DiTauRecToolsLib
   DiTauRecTools/*.h Root/*.cxx 
   PUBLIC_HEADERS DiTauRecTools
   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
   ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${EIGEN_LIBRARIES} ${ONNXRUNTIME_LIBRARIES}
   ${ROOT_LIBRARIES}
   CxxUtils AthLinks AsgMessagingLib AsgDataHandlesLib AsgTools xAODCaloEvent
   xAODJet xAODParticleEvent xAODPFlow xAODTracking xAODEventShape
   MVAUtils ${extra_public_libs}
   tauRecToolsLib
   PRIVATE_LINK_LIBRARIES CaloGeoHelpers FourMomUtils PathResolver AthContainers AthOnnxInterfaces )

atlas_add_dictionary( DiTauRecToolsDict
   DiTauRecTools/DiTauRecToolsDict.h DiTauRecTools/selection.xml
   LINK_LIBRARIES DiTauRecToolsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( DiTauRecTools
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AsgTools AsgMessagingLib xAODBase tauRecToolsLib
      xAODCaloEvent xAODJet xAODPFlow FourMomUtils DiTauRecToolsLib
      ${extra_private_libs} )
endif()
