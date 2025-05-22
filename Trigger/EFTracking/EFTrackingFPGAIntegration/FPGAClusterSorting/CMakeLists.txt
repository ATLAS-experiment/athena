# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(FPGAClusterSorting)

atlas_add_component(FPGAClusterSorting
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES  AthenaBaseComps AthenaKernel GaudiKernel xAODInDetMeasurement StoreGateLib
)

atlas_install_python_modules( python/*.py )
