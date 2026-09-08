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

find_path( TRITON_BACKEND_INCLUDE_DIR
   NAMES triton/core/tritonbackend.h
   HINTS ENV TRITON_SERVER_SDK_INCLUDE_DIRECTORY )
find_library( TRITON_BACKEND_UTILS_LIBRARY
   NAMES triton-backend-utils tritonbackendutils
   HINTS ENV TRITON_SERVER_SDK_LIBRARY_DIRECTORY )
find_library( TRITON_SERVER_LIBRARY
   NAMES tritonserver
   HINTS ENV TRITON_SERVER_SDK_LIBRARY_DIRECTORY )
find_library( TRITON_ASYNC_WORK_QUEUE_LIBRARY
   NAMES tritonasyncworkqueue
   HINTS ENV TRITON_SERVER_SDK_LIBRARY_DIRECTORY )
find_library( TRITON_KERNEL_LIBRARY
   NAMES kernel_library_new
   HINTS ENV TRITON_SERVER_SDK_LIBRARY_DIRECTORY )

if ( NOT TRITON_BACKEND_INCLUDE_DIR OR NOT TRITON_BACKEND_UTILS_LIBRARY
      OR NOT TRITON_SERVER_LIBRARY OR NOT TRITON_ASYNC_WORK_QUEUE_LIBRARY
      OR NOT TRITON_KERNEL_LIBRARY )
   message( STATUS
      "TracccTritonBackend: Triton backend SDK not found "
      "(set TRITON_SERVER_SDK_INCLUDE_DIRECTORY / TRITON_SERVER_SDK_LIBRARY_DIRECTORY), skipping" )
   return()
endif()

find_package( vecmem )
find_package( traccc )
find_package( CUDAToolkit )
find_package( Python COMPONENTS Development )
get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python LOCATION )
if ( NOT TRACCC_PYTHON_SHARED_LIBRARY )
   get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python IMPORTED_LOCATION )
endif()
if ( NOT TRACCC_PYTHON_SHARED_LIBRARY )
   get_target_property( TRACCC_PYTHON_SHARED_LIBRARY Python::Python IMPORTED_LOCATION_RELEASE )
endif()

find_path( RAPIDJSON_INCLUDE_DIR
   NAMES rapidjson/document.h
   HINTS ${RAPIDJSON_LCGROOT}/include )

atlas_add_library( triton_traccc
   src/*.hpp src/*.h src/*.cpp src/*.cxx src/*.cc
   SHARED
   NO_PUBLIC_HEADERS
   DEFINITIONS
      -DTRITON_ENABLE_STATS
      -DTRITON_ENABLE_GPU
      -DTRACCC_TRITON_PYTHON_LIBRARY="${TRACCC_PYTHON_SHARED_LIBRARY}"
   INCLUDE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${TRITON_BACKEND_INCLUDE_DIR}
      ${RAPIDJSON_INCLUDE_DIR}
   LINK_LIBRARIES
      GaudiKernel Identifier AthenaBaseComps StoreGateLib CxxUtils PathResolver
      AthDeviceInterfacesLib AthCUDAInterfacesLib
      ActsGPUInterfacesLib ActsGPUEventLib
      vecmem::core vecmem::cuda traccc::core traccc::cuda
      CUDA::cudart ${TRITON_BACKEND_UTILS_LIBRARY} ${TRITON_SERVER_LIBRARY}
      ${TRITON_ASYNC_WORK_QUEUE_LIBRARY} ${TRITON_KERNEL_LIBRARY} Python::Python
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Install the Triton model repository for this backend.
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
