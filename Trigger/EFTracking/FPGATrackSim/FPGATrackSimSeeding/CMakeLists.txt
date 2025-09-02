# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(FPGATrackSimSeeding)

atlas_add_component(FPGATrackSimSeeding
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES  AthenaBaseComps FPGATrackSimObjectsLib FPGATrackSimConfToolsLib FPGATrackSimSGInputLib FPGATrackSimInputLib ActsEventLib AthenaKernel GaudiKernel xAODInDetMeasurement
)

atlas_install_python_modules( python/*.py )
