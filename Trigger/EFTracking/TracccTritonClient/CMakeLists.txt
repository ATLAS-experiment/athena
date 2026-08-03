# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TracccTritonClient )

find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( TracccTritonClient
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES
        AthenaBaseComps GaudiKernel StoreGateLib CxxUtils PathResolver
        AthOnnxInterfaces
        TrkTrack TrkSpacePoint TrkEventPrimitives
        InDetIdentifier InDetPrepRawData InDetRawData InDetReadoutGeometry
        ReadoutGeometryBase GeoPrimitives PixelReadoutGeometryLib SCT_ReadoutGeometry
        xAODInDetMeasurement xAODTracking xAODTruth
        Acts::Core
        ActsEventLib ActsGeometryLib ActsGeometryInterfacesLib ActsCalibBase
)

atlas_install_python_modules( python/*.py )
