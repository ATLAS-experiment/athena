# $Id: CMakeLists.txt 729385 2016-03-13 14:47:24Z krasznaa $
################################################################################
# Package: G4PhysicsLists
################################################################################

# Declare the package name:
atlas_subdir( G4PhysicsLists )

# External dependencies:
find_package( AdePT QUIET )
find_package( Celeritas QUIET )
find_package( CLHEP )
find_package( Geant4 )

# If AdePT is found, add its code
if( AdePT_FOUND )
  message( STATUS "Building G4PhysicsLists with AdePT support" )
  set( AdePT_SOURCES src/AdePT/*.cc src/G4HepEm/*.cc )
endif()

# If Celeritas is found, add its code
if( Celeritas_FOUND )
  message( STATUS "Building G4PhysicsLists with Celeritas support" )
  set( Celeritas_SOURCES src/Celeritas/*.cc )
endif()


# Component(s) in the package:
atlas_add_library( G4PhysicsLists
  src/*.cc src/MuBias/*.cc src/hadronicVariations/*.cc ${AdePT_SOURCES} ${Celeritas_SOURCES}
  OBJECT
  PUBLIC_HEADERS G4PhysicsLists
  INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
  PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MuBias
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hadronicVariations
  ${CMAKE_CURRENT_SOURCE_DIR}/src/AdePT
  ${CMAKE_CURRENT_SOURCE_DIR}/src/G4HepEm
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Celeritas
  LINK_LIBRARIES ${GEANT4_LIBRARIES}
  PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} GaudiKernel )
set_target_properties( G4PhysicsLists PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Link Celeritas with cuda_rdc to ensure correct resolution if CUDA enabled
if( Celeritas_FOUND )
  target_compile_definitions( G4PhysicsLists PRIVATE ATHSIMULATION_USE_CELER )
  cuda_rdc_target_link_libraries( G4PhysicsLists PUBLIC Celeritas::accel )
endif()

# Compile/Link for AdePT if needed
if( AdePT_FOUND )
  target_compile_definitions( G4PhysicsLists PRIVATE ATHSIMULATION_USE_ADEPT )
  cuda_rdc_target_link_libraries( G4PhysicsLists PUBLIC AdePT::G4Integration )
endif()

# Install files from the package:
atlas_install_runtime( share/biasValues.txt )
