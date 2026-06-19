# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HanConfigGenerator )

# External dependencies:
find_package( ROOT COMPONENTS Graf Gpad Core Tree MathCore Hist RIO )

# Component(s) in the package.
atlas_add_executable( hcg
                      src/hcg.cxx src/addnode.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils )
