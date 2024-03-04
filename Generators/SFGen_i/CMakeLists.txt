# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SFGen_i )

# External dependencies:

find_package( Apfel )
find_package( SFGen )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py share/common/*.py )


# Set SFGen specific environment variable(s).
set( SFGenEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of SFGenEnvironment.cmake" )
find_package( SFGenEnvironment )
