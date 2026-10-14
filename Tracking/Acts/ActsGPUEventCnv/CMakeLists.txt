# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsGPUEventCnv)

find_package(vecmem)
find_package(traccc)

set( _sources
  src/*.h src/*.cxx
  src/components/*.cxx
)

set( _libraries
  vecmem::core
  traccc::core
  traccc::io
  detray::io
  detray::core
  traccc::device_common
  GaudiKernel
  AthenaBaseComps
  ActsGeometryInterfacesLib
  ActsGPUInterfacesLib
  ActsGPUEventLib
  ActsEventLib
  ActsCalibratorsLib
  InDetRawData
  InDetPrepRawData
  InDetIdentifier
  xAODInDetMeasurement
  AthDeviceInterfacesLib
  PixelReadoutGeometryLib
  SCT_ReadoutGeometry
)

set( _definitions
  ALGEBRA_PLUGINS_INCLUDE_ARRAY
)

# Check if CUDA compilation is available.
if( CMAKE_CUDA_COMPILER )
  list( APPEND _sources
    src/cuda/*.h
    src/cuda/*.cxx
    src/cuda/*.cu
  )
  list( APPEND _libraries
    AthCUDAInterfacesLib
    AthCUDAKernelLib
  )
  list( APPEND _definitions
    ACTSGPUEVENTCNV_WITH_CUDA_COMPONENTS
  )
endif()

atlas_add_component( ActsGPUEventCnv
  ${_sources}
  DEFINITIONS
    ${_definitions}
  LINK_LIBRARIES
    ${_libraries}
)

atlas_install_python_modules(python/*.py)

if (TARGET traccc::cuda)
atlas_add_test( RDOtoTracccTest
  PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
  ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
  SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/RDOtoTracccTest.py Device.Backend=DeviceBackend.CUDA
  POST_EXEC_SCRIPT noerror.sh
  PROPERTIES SKIP_RETURN_CODE 2 )

atlas_add_test( DeviceSeedingFromHostSP
  PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
  ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
  SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsDeviceSeedingFromHostSpacePointsTest.py Device.Backend=DeviceBackend.CUDA
  POST_EXEC_SCRIPT noerror.sh
  PROPERTIES SKIP_RETURN_CODE 2 )  

atlas_add_test( TracccMeasurementConversion
  PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
  ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
  SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/TracccMeasurementConversionTest.py Device.Backend=DeviceBackend.CUDA
  POST_EXEC_SCRIPT noerror.sh
  PROPERTIES SKIP_RETURN_CODE 2 )
endif()
