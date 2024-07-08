# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauRecTools )

# External dependencies:
find_package( Eigen )
find_package( ROOT COMPONENTS Core Tree Hist RIO )
find_package( FastJet COMPONENTS fastjetplugins fastjettools )
find_package( FastJetContrib COMPONENTS VariableR RecursiveTools )

atlas_add_component( DiTauRecTools
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AsgTools AsgMessagingLib xAODBase tauRecToolsLib
      xAODCaloEvent xAODJet xAODPFlow xAODTau FourMomUtils DiTauRecToolsLib
      ${extra_private_libs} )

# Component(s) in the package:
atlas_add_library( DiTauRecToolsLib
   DiTauRecTools/*.h Root/*.cxx 
   PUBLIC_HEADERS DiTauRecTools
   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} ${FASTJET_INCLUDE_DIRS} ${FASTJETCONTRIB_INCLUDE_DIRS}
   ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${EIGEN_LIBRARIES} ${FASTJET_LIBRARIES} ${FASTJETCONTRIB_LIBRARIES}
   ${ROOT_LIBRARIES}
   CxxUtils AthLinks AsgMessagingLib AsgDataHandlesLib AsgTools xAODCaloEvent
   xAODEventInfo xAODJet xAODParticleEvent xAODPFlow xAODTau xAODTracking xAODEventShape
   MVAUtils ${extra_public_libs}
   PRIVATE_LINK_LIBRARIES CaloGeoHelpers FourMomUtils PathResolver AthContainers )

atlas_add_dictionary( DiTauRecToolsDict
   DiTauRecTools/DiTauRecToolsDict.h DiTauRecTools/selection.xml
   LINK_LIBRARIES DiTauRecToolsLib )


