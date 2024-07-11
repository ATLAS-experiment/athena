# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(FPGATrackSimReporting)

# External dependencies:
find_package(Boost)

atlas_add_component(FPGATrackSimReporting
    src/*.cxx
    src/components/*.cxx
    INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
    LINK_LIBRARIES ${Boost_LIBRARIES} AthenaBaseComps xAODInDetMeasurement FPGATrackSimObjectsLib FPGATrackSimConfToolsLib FPGATrackSimSGInputLib FPGATrackSimInputLib AthenaKernel GaudiKernel ActsEventLib
)

atlas_install_python_modules( python/*.py )