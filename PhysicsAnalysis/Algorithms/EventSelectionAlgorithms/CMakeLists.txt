# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina

atlas_subdir( EventSelectionAlgorithms )

find_package( ROOT COMPONENTS Core )

atlas_add_library( EventSelectionAlgorithmsLib
   EventSelectionAlgorithms/*.h EventSelectionAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS EventSelectionAlgorithms
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AnaAlgorithmLib FourMomUtils SystematicsHandlesLib SelectionHelpersLib xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData  )

atlas_add_dictionary( EventSelectionAlgorithmsDict
   EventSelectionAlgorithms/EventSelectionAlgorithmsDict.h
   EventSelectionAlgorithms/selection.xml
   LINK_LIBRARIES EventSelectionAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( EventSelectionAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES EventSelectionAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )