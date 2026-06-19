# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(FPGATrackSimReporting)

atlas_add_component(FPGATrackSimReporting
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES  AthenaBaseComps xAODInDetMeasurement FPGATrackSimObjectsLib 
                    FPGATrackSimConfToolsLib FPGATrackSimSGInputLib FPGATrackSimInputLib 
                    AthenaKernel GaudiKernel ActsEventLib ActsCalibratorsLib)

atlas_install_python_modules( python/*.py )
