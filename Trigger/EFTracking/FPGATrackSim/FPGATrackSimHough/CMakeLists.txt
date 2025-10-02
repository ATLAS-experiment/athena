# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimHough )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrackSimHoughLib
   src/*.cxx
   PUBLIC_HEADERS FPGATrackSimHough
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel AthenaMonitoringKernelLib FourMomUtils FPGATrackSimMapsLib FPGATrackSimObjectsLib FPGATrackSimBinningLib nlohmann_json::nlohmann_json
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} FPGATrackSimBanksLib FPGATrackSimConfToolsLib )

atlas_add_component( FPGATrackSimHough
   src/components/*.cxx
   LINK_LIBRARIES FPGATrackSimBanksLib FPGATrackSimHoughLib  FPGATrackSimBinningLib)

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/Hough_plots.py )


