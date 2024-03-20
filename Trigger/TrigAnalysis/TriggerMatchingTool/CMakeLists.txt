# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TriggerMatchingTool )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Hist RIO )

# Component(s) in the package:
atlas_add_library( TriggerMatchingToolLib
   TriggerMatchingTool/*.h Root/*.cxx
   PUBLIC_HEADERS TriggerMatchingTool
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES AsgTools xAODBase TrigNavStructure TrigDecisionToolLib AnaAlgorithmLib CxxUtils
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} FourMomUtils xAODTrigger xAODEgamma)

if( NOT XAOD_STANDALONE )
   atlas_add_component( TriggerMatchingTool
      src/*.h src/*.cxx src/components/*.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel
      AthAnalysisBaseCompsLib TriggerMatchingToolLib )
endif()

atlas_add_dictionary( TriggerMatchingToolDict
   TriggerMatchingTool/TriggerMatchingToolDict.h
   TriggerMatchingTool/selection.xml
   LINK_LIBRARIES TriggerMatchingToolLib )
