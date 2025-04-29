# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimAlgorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO )
find_package( lwtnn )
find_package( onnxruntime )
find_package( Eigen )


# Component(s) in the package:
atlas_add_library( FPGATrackSimAlgorithmsLib
   src/*.cxx
   PUBLIC_HEADERS FPGATrackSimAlgorithms
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}  ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaMonitoringKernelLib GaudiKernel FPGATrackSimBanksLib FPGATrackSimBinningLib FPGATrackSimConfToolsLib FPGATrackSimHoughLib FPGATrackSimInputLib FPGATrackSimLRTLib FPGATrackSimMapsLib FPGATrackSimObjectsLib FPGATrackSimSGInputLib  ${ONNXRUNTIME_LIBRARIES}
   PRIVATE_LINK_LIBRARIES FPGATrackSimBanksLib FPGATrackSimConfToolsLib )
 
# Component(s) in the package:
atlas_add_component( FPGATrackSimAlgorithms
   src/components/*.cxx
   LINK_LIBRARIES FPGATrackSimAlgorithmsLib)

 
# Install files from the package:
atlas_install_python_modules( python/*.py )
