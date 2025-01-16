################################################################################
# Package: PowhegControl
################################################################################

# Declare the package name.
atlas_subdir( PowhegControl )

# Install files from the package.
atlas_install_python_modules( python/*.py
   python/algorithms python/decorators python/parameters
   python/processes python/utility 
   POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/common/*.py share/control/*.py )

# Set up the runtime environment for Powheg.
configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PowhegEnvironmentConfig.cmake.in
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PowhegEnvironmentConfig.cmake
   @ONLY )
set( PowhegEnvironment_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}
   CACHE PATH "Location of PowhegEnvironmentConfig.cmake" )

find_package( PowhegEnvironment )

if(NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
  find_package( ggvvamp )
  find_package( qqvvamp )
endif()
find_package( chaplin )
find_package( cln )
find_package( ginac )
find_package( nlox )
find_package( Lhapdf )



