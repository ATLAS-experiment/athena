# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUGeometry )

find_package(traccc)
find_package(vecmem)

atlas_add_component( ActsGPUGeometry
  src/*.h src/*.cxx
  src/components/*.cxx

  LINK_LIBRARIES
    vecmem::core
    traccc::core
    traccc::io
    GaudiKernel
    AthenaBaseComps
    AthDeviceInterfacesLib
    PathResolver
    InDetIdentifier
    ActsGeometryInterfacesLib
    ActsGeometryLib
    ActsGPUEventLib
    ActsGPUInterfacesLib
    SiSpacePointFormationLib
    PixelReadoutGeometryLib
    SCT_ReadoutGeometry
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if (TARGET traccc::cuda)
atlas_add_test( ActsDeviceGeometryTest
   PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
   ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
   SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsDeviceGeometryTest.py Device.Backend=DeviceBackend.CUDA
   POST_EXEC_SCRIPT noerror.sh
   PROPERTIES SKIP_RETURN_CODE 2 )
endif()