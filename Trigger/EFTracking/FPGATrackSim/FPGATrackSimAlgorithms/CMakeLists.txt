# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimAlgorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO )
find_package( lwtnn )
find_package( onnxruntime )
find_package( Eigen )

# Component(s) in the package:
atlas_add_component( FPGATrackSimAlgorithms
   src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS   ${LWTNN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
   LINK_LIBRARIES ${LWTNN_LIBRARIES} ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES}  ${EIGEN_LIBRARIES} AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib CxxUtils GaudiKernel FPGATrackSimBanksLib FPGATrackSimConfToolsLib FPGATrackSimHoughLib FPGATrackSimInputLib FPGATrackSimLRTLib FPGATrackSimMapsLib FPGATrackSimObjectsLib FPGATrackSimSGInputLib)

# Install files from the package:
atlas_install_python_modules( python/*.py )
