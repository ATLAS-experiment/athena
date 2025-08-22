# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1AODPlugin )

# External dependencies:
find_package( Qt5 COMPONENTS Core )

# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )

# Build the library.
atlas_add_library( VP1AODPlugin
   VP1AODPlugin/*.h src/*.cxx
   PUBLIC_HEADERS VP1AODPlugin
   LINK_LIBRARIES Qt5::Core VP1Base
   PRIVATE_LINK_LIBRARIES VP1GuideLineSystems VP1AODSystems )

# Enable Qt support for cppcheck:
if( CMAKE_CXX_CPPCHECK )
   set_property( TARGET VP1AODPlugin
      APPEND PROPERTY CXX_CPPCHECK "--library=qt" )
endif()
