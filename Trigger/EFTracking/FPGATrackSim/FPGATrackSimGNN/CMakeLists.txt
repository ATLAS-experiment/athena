# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimGNN )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrackSimGNNLib
   src/*.cxx
   NO_PUBLIC_HEADERS
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} AthenaBaseComps AthOnnxInterfaces AthOnnxUtilsLib GaudiKernel FPGATrackSimObjectsLib FPGATrackSimHoughLib FPGATrackSimConfToolsLib )
   
atlas_add_component( FPGATrackSimGNN
   src/components/*.cxx
   LINK_LIBRARIES FPGATrackSimGNNLib )
