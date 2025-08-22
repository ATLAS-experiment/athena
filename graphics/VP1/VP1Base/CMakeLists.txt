# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Set the name of the package:
atlas_subdir( VP1Base )

# Get OS version
atlas_os_id( _os _isValid )
if( NOT _isValid )
   message( WARNING "Could not determine OS name!" )
   return()
endif()
message ( STATUS "  > Compiling on ${_os}" ) 

# Set up the build of Coin3D:
# Select which version to use:

# Find the external dependencies:
find_package( Qt5 COMPONENTS Core OpenGL Gui PrintSupport Widgets )
find_package( SoQt )
find_package( Coin3D )
find_package( GeoModel COMPONENTS GeoModelKernel )

# Generate UI files automatically:
# Note: add the "Widgets" component to "find_package( Qt5 ...)" if you have UI files, otherwise UIC, even if CMAKE_AUTOUIC is set to ON, is not run
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )
# Generate resource files automatically:
set( CMAKE_AUTORCC TRUE )

# Build the library. Remember that the auto-generated UI files are put under
# CMAKE_CURRENT_BINARY_DIR, so it needs to be added explicitly to the
# declaration of PRIVATE_INCLUDE_DIRS.
atlas_add_library( VP1Base VP1Base/*.h src/*.cxx src/*.qrc
   PUBLIC_HEADERS VP1Base
   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS} ${SOQT_INCLUDE_DIRS} ${COIN3D_INCLUDE_DIRS}
   PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
   LINK_LIBRARIES ${QT5_LIBRARIES} ${SOQT_LIBRARIES} ${COIN3D_LIBRARIES} ${GEOMODEL_LIBRARIES} GeoPrimitives xAODBase
   PRIVATE_LINK_LIBRARIES VP1HEPVis )

# Enable Qt support for cppcheck:
if( CMAKE_CXX_CPPCHECK )
  set_property( TARGET VP1Base
     APPEND PROPERTY CXX_CPPCHECK "--library=qt" )
endif()

