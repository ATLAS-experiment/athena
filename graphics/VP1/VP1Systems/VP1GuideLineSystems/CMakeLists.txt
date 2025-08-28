# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1GuideLineSystems )

# External dependencies:
find_package( Coin3D )
find_package( Qt5 COMPONENTS Core Gui Widgets )

# Generate UI files automatically:
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )
# Generate resource files automatically:
set( CMAKE_AUTORCC TRUE )

if(NOT BUILDVP1LIGHT)
# Component(s) in the package:
atlas_add_library( VP1GuideLineSystems VP1GuideLineSystems/*.h src/*.cxx src/*.qrc
   PUBLIC_HEADERS VP1GuideLineSystems
   INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS}
   LINK_LIBRARIES ${COIN3D_LIBRARIES} GeoPrimitives VP1Base VP1Utils
   Qt5::Core Qt5::Gui
   PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
   PRIVATE_LINK_LIBRARIES GaudiKernel VP1HEPVis )
endif()

# Enable Qt support for cppcheck:
if( CMAKE_CXX_CPPCHECK )
   set_property( TARGET VP1GuideLineSystems
      APPEND PROPERTY CXX_CPPCHECK "--library=qt" )
endif()

if(BUILDVP1LIGHT)
# Component(s) in the package:
atlas_add_library( VP1GuideLineSystems VP1GuideLineSystems/*.h src/*.cxx src/*.qrc
   PUBLIC_HEADERS VP1GuideLineSystems
   INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS}
   LINK_LIBRARIES ${COIN3D_LIBRARIES} GeoPrimitives VP1Base VP1Utils
   Qt5::Core Qt5::Gui
   PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
   PRIVATE_LINK_LIBRARIES VP1HEPVis )
endif()

