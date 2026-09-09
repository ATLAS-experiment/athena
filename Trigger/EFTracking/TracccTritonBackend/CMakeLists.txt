# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( TracccTritonBackend )

find_package( Acts )
if ( NOT TARGET Acts::PluginDetray )
   message( STATUS "TracccTritonBackend: Acts::PluginDetray not available, skipping" )
   return()
endif()

# Bail out if CUDA is not available.
if ( NOT CMAKE_CUDA_COMPILER )
   message( STATUS "TracccTritonBackend: no CUDA compiler, skipping" )
   return()
endif()

# Triton backend has to go after these configs
find_package( TritonCore )
find_package( TritonCommon )
find_package( CUDAToolkit )

# need to setup RapidJSON by hand for Triton (not exported by default)
if ( NOT TARGET TritonCommon::triton-common-json )
   find_package( RapidJSON QUIET HINTS ${RAPIDJSON_LCGROOT} )
   add_library( TritonCommon::triton-common-json INTERFACE IMPORTED )
   if ( RAPIDJSON_INCLUDE_DIRS )
      set_target_properties( TritonCommon::triton-common-json PROPERTIES
         INTERFACE_INCLUDE_DIRECTORIES "${RAPIDJSON_INCLUDE_DIRS}" )
   endif()
endif()

find_package( TritonBackend )

find_package( vecmem )
find_package( traccc )
find_package( Python COMPONENTS Development )
get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python LOCATION )
if ( NOT TRACCC_PYTHON_SHARED_LIBRARY )
   get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python IMPORTED_LOCATION )
endif()
if ( NOT TRACCC_PYTHON_SHARED_LIBRARY )
   get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python IMPORTED_LOCATION_RELEASE )
endif()

atlas_add_library( triton_traccc
   src/*.h src/*.cxx
   SHARED
   NO_PUBLIC_HEADERS
   DEFINITIONS
      -DTRACCC_TRITON_PYTHON_LIBRARY="${TRACCC_PYTHON_SHARED_LIBRARY}"
   INCLUDE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/src
   LINK_LIBRARIES
      GaudiKernel Identifier AthenaBaseComps StoreGateLib CxxUtils PathResolver
      AthDeviceInterfacesLib AthCUDAInterfacesLib
      ActsGPUInterfacesLib ActsGPUEventLib
      vecmem::core vecmem::cuda traccc::core traccc::cuda
      CUDA::cudart Python::Python
      TritonBackend::triton-backend-utils
      TritonCore::triton-core-serverstub
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Install the Triton model repository for this backend in ${Athena_DIR}/data/
set( _tracccModelDest
   "${CMAKE_INSTALL_DATADIR}/TracccTritonBackend/models/traccc-gpu" )

atlas_install_generic( models/traccc-gpu/config.pbtxt
   DESTINATION ${_tracccModelDest}
   TYPENAME TritonModel )

# The backend library goes next to config.pbtxt
set( _tracccModelBuildDir
   "${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/${_tracccModelDest}" )
add_custom_target( TracccTritonBackendModelRepo ALL
   COMMAND ${CMAKE_COMMAND} -E make_directory "${_tracccModelBuildDir}/1"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:triton_traccc>"
      "${_tracccModelBuildDir}/$<TARGET_FILE_NAME:triton_traccc>"
   COMMENT "Populating the traccc-gpu Triton model repository"
   VERBATIM )
add_dependencies( TracccTritonBackendModelRepo triton_traccc )
add_dependencies( Package_TracccTritonBackend TracccTritonBackendModelRepo )

install( PROGRAMS "$<TARGET_FILE:triton_traccc>"
   DESTINATION "${_tracccModelDest}" )
install( DIRECTORY DESTINATION "${_tracccModelDest}/1" )

unset( _tracccModelBuildDir )
unset( _tracccModelDest )
