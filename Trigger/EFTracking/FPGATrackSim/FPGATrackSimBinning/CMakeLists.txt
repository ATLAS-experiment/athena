# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimBinning )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrackSimBinningLib
   src/*.cxx
   PUBLIC_HEADERS FPGATrackSimBinning
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel FourMomUtils FPGATrackSimMapsLib FPGATrackSimObjectsLib nlohmann_json::nlohmann_json
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} FPGATrackSimBanksLib FPGATrackSimConfToolsLib )

#atlas_add_component( FPGATrackSimBinning
#   src/components/*.cxx
#   LINK_LIBRARIES FPGATrackSimBanksLib FPGATrackSimBinningLib )

# Install files from the package:
#atlas_install_python_modules( python/*.py )
#atlas_install_scripts( scripts/Binning_plots.py )


