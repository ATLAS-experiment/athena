# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauRecTools )

# Component(s) in the package:
atlas_add_library( DiTauRecToolsLib
   DiTauRecTools/*.h Root/*.cxx 
   PUBLIC_HEADERS DiTauRecTools
   LINK_LIBRARIES ${EIGEN_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} tauRecToolsLib
   PRIVATE_LINK_LIBRARIES PathResolver AthContainers ) 

atlas_add_dictionary( DiTauRecToolsDict
   DiTauRecTools/DiTauRecToolsDict.h DiTauRecTools/selection.xml
   LINK_LIBRARIES DiTauRecToolsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( DiTauRecTools
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES tauRecToolsLib DiTauRecToolsLib
      ${extra_private_libs} )
endif()
