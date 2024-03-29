# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimAlgorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO )
find_package( lwtnn )

# Component(s) in the package:
atlas_add_component( FPGATrackSimAlgorithms
   src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS   ${LWTNN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${LWTNN_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib CxxUtils GaudiKernel FPGATrackSimBanksLib FPGATrackSimConfToolsLib FPGATrackSimHoughLib FPGATrackSimInputLib FPGATrackSimLRTLib FPGATrackSimMapsLib FPGATrackSimObjectsLib FPGATrackSimSGInputLib)

# Install files from the package:
atlas_install_python_modules( python/*.py )
