# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsGPUEventCnv)

find_package(vecmem)
find_package(traccc)
find_package(detray)

atlas_add_component( ActsGPUEventCnv
    src/*.h src/*.cxx
    src/components/*.cxx
    DEFINITIONS ALGEBRA_PLUGINS_INCLUDE_ARRAY
    LINK_LIBRARIES
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

atlas_install_python_modules(python/*.py)

if (TARGET traccc::cuda)
atlas_add_test( RDOtoTracccTest
   PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
   ENVIRONMENT "PATHRESOLVER_DEVAREARESPONSE=WARNING"
   SCRIPT RDOtoTracccTest.py Device.Backend=DeviceBackend.CUDA
   POST_EXEC_SCRIPT noerror.sh
   PROPERTIES SKIP_RETURN_CODE 2 )
endif()

