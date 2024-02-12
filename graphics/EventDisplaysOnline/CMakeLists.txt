# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventDisplaysOnline )

find_package( Python COMPONENTS Development )

atlas_add_library(EventDisplaysOnlineLib
  INTERFACE
  EventDisplaysOnline/*.h
  src/*.cxx
  PUBLIC_HEADERS EventDisplaysOnline
  INCLUDE_DIRS ${Python_INCLUDE_DIRS}
  LINK_LIBRARIES ${Python_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel xAODEventInfo RootUtils)

atlas_add_component( EventDisplaysOnline
  EventDisplaysOnline/*.h
  src/*.cxx
  src/components/*.cxx
  LINK_LIBRARIES EventDisplaysOnlineLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
atlas_install_scripts( scripts/*.sh )
