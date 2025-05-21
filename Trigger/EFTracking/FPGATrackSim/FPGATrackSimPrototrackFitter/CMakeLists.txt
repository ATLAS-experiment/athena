# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(FPGATrackSimPrototrackFitter)

# External dependencies:
find_package(Acts COMPONENTS Core)

atlas_add_component(FPGATrackSimPrototrackFitter
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES 
    ActsCore
    ActsToolInterfacesLib
    ActsCalibrationLib
    LINK_LIBRARIES ActsEventLib
)

atlas_install_python_modules( python/*.py )