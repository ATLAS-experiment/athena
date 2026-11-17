# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1Gui )

# External dependencies:
find_package( Qt5 COMPONENTS Core OpenGL Gui Network PrintSupport Widgets )
find_package( Coin3D )
find_package( SoQt )

# Generate UI files automatically:
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )
# Generate resource files automatically:
set( CMAKE_AUTORCC TRUE )

# Component(s) in the package:
atlas_add_library( VP1Gui VP1Gui/*.h src/*.h src/*.cxx src/*.qrc
   PUBLIC_HEADERS VP1Gui
   PRIVATE_INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS} ${SOQT_INCLUDE_DIRS}
   ${CMAKE_CURRENT_BINARY_DIR}
   LINK_LIBRARIES VP1Base Qt5::Core Qt5::OpenGL Qt5::Gui
   Qt5::PrintSupport xAODBase
   PRIVATE_LINK_LIBRARIES ${COIN3D_LIBRARIES} ${SOQT_LIBRARIES} xAODCore
   xAODRootAccess xAODEventInfo xAODRootAccessInterfaces Qt5::Network VP1UtilsBase )

# Enable Qt support for cppcheck:
if( CMAKE_CXX_CPPCHECK )
   set_property( TARGET VP1Gui
      APPEND PROPERTY CXX_CPPCHECK "--library=qt" )
endif()
# Exercise the real rotation menu and startup notices using Qt's offscreen plugin.
atlas_add_test( RemoteMode
   SOURCES test/RemoteMode_test.cxx
   INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS} ${SOQT_INCLUDE_DIRS}
      ${CMAKE_CURRENT_BINARY_DIR}/VP1Gui_autogen/include
      ${CMAKE_CURRENT_SOURCE_DIR}/../VP1Base/src
   LINK_LIBRARIES VP1Gui VP1Base Qt5::Network ${COIN3D_LIBRARIES} ${SOQT_LIBRARIES}
   POST_EXEC_SCRIPT nopost.sh )
if( CMAKE_CXX_CPPCHECK )
   set_property( TARGET VP1Gui_RemoteMode
      APPEND PROPERTY CXX_CPPCHECK "--library=qt" )
endif()
