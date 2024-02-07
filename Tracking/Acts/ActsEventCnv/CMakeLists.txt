# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsEventCnv)

# External dependencies:
find_package(Acts COMPONENTS Core)

# Component(s) in the package:
atlas_add_library(ActsEventCnvLib
    ActsEventCnv/*.h
    PUBLIC_HEADERS ActsEventCnv
    INTERFACE
    LINK_LIBRARIES
    ActsCore
    ActsEventLib
    GaudiKernel
    StoreGateLib
    TrkParameters
    TrkTrack
    xAODMeasurementBase
    xAODTracking
)

atlas_add_component(ActsEventCnv
    src/*.h src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES
    ActsEventCnvLib
    ActsCore
    ActsEventLib
    ActsGeometryLib
    ActsGeometryInterfacesLib
    ActsInteropLib
    AthenaBaseComps
    GaudiKernel
    InDetPrepRawData
    InDetReadoutGeometry
    InDetRIO_OnTrack
    StoreGateLib
    TrkEventPrimitives
    TrkExUtils
    TrkMeasurementBase
    TrkParameters
    TrkSurfaces
    TrkToolInterfaces
    TrkTrack
    xAODInDetMeasurement
    xAODMeasurementBase
    xAODTracking
)

atlas_add_test(ActsEventCnvTest
    SCRIPT test/testConversion.py
    PROPERTIES TIMEOUT 900
    POST_EXEC_SCRIPT noerror.sh)
