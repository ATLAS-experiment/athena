# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsEventCnv)

# External dependencies:
find_package(Acts COMPONENTS Core)


atlas_add_component(ActsEventCnv
    src/*.h src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES
    ActsCore
    ActsEventLib
    ActsGeometryLib
    ActsGeometryInterfacesLib
    ActsToolInterfacesLib
    ActsCalibrationLib
    ActsInteropLib
    AthenaBaseComps
    GaudiKernel
    InDetPrepRawData
    InDetReadoutGeometry
    InDetRIO_OnTrack
    MuonReadoutGeometry
    MuonReadoutGeometryR4
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
    xAODTracking)

atlas_add_test(ActsEventCnvTrackTest
    SCRIPT test/testTrackConversion.py
    PROPERTIES TIMEOUT 900
    POST_EXEC_SCRIPT noerror.sh)

atlas_add_test(ActsEventCnvMuonTrackTest
    SCRIPT test/testMuonTrackConversion.py
    PROPERTIES TIMEOUT 900
    LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from AtasTrackingGeometryConversion
    POST_EXEC_SCRIPT noerror.sh)
