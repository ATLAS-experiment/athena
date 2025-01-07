# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimBankGen )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Matrix )

# Declare Gaudi component(s) in the package
atlas_add_component( FPGATrackSimBankGen
   src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} FPGATrackSimAlgorithmsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py )


