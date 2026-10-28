# $Id: CMakeLists.txt 729385 2016-03-13 14:47:24Z krasznaa $
################################################################################
# Package: G4PhysicsLists
################################################################################

# Declare the package name:
atlas_subdir( G4PhysicsLists )

# Optional e/gamma offload packages, with the compile definition and the
# target to link against for each of them:
set( _offloadPackages AdePT G4HepEm Celeritas )
set( _AdePT_define ATHSIMULATION_USE_ADEPT )
set( _AdePT_target AdePT::G4Integration )
set( _G4HepEm_define ATHSIMULATION_USE_G4HEPEM )
set( _G4HepEm_target G4HepEm::g4HepEm )
set( _Celeritas_define ATHSIMULATION_USE_CELER )
set( _Celeritas_target Celeritas::accel )
# Packages to link with cuda_rdc to ensure correct resolution if CUDA enabled:
set( _rdcPackages AdePT Celeritas )

# External dependencies:
foreach( _pkg ${_offloadPackages} )
  find_package( ${_pkg} QUIET )
endforeach()
find_package( CLHEP )
find_package( Geant4 )

# AdePT requires G4HepEm
if( AdePT_FOUND AND NOT G4HepEm_FOUND )
  message( FATAL_ERROR "AdePT was found but G4HepEm was not; AdePT requires G4HepEm" )
endif()

# If an offload package is found, add its code
set( _offloadSources )
set( _offloadIncludes )
foreach( _pkg ${_offloadPackages} )
  if( ${_pkg}_FOUND )
    message( STATUS "Building G4PhysicsLists with ${_pkg} support" )
    list( APPEND _offloadSources src/${_pkg}/*.cc )
    list( APPEND _offloadIncludes ${CMAKE_CURRENT_SOURCE_DIR}/src/${_pkg} )
  endif()
endforeach()

# Component(s) in the package:
atlas_add_library( G4PhysicsLists
  src/*.cc src/MuBias/*.cc src/hadronicVariations/*.cc ${_offloadSources}
  OBJECT
  PUBLIC_HEADERS G4PhysicsLists
  INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
  PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MuBias
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hadronicVariations
  ${_offloadIncludes}
  LINK_LIBRARIES ${GEANT4_LIBRARIES}
  PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} GaudiKernel )
set_target_properties( G4PhysicsLists PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Compile/Link for the offload packages if needed
foreach( _pkg ${_offloadPackages} )
  if( NOT ${_pkg}_FOUND )
    continue()
  endif()
  target_compile_definitions( G4PhysicsLists PRIVATE ${_${_pkg}_define} )
  if( _pkg IN_LIST _rdcPackages )
    cuda_rdc_target_link_libraries( G4PhysicsLists PUBLIC ${_${_pkg}_target} )
  else()
    target_link_libraries( G4PhysicsLists PUBLIC ${_${_pkg}_target} )
  endif()
endforeach()

# Install files from the package:
atlas_install_runtime( share/biasValues.txt )
