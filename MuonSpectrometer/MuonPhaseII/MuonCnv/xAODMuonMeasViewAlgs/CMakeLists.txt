# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonMeasViewAlgs )

# External dependencies:
find_package( xAODUtilities )

# Component(s) in the package:
atlas_add_component( xAODMuonMeasViewAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps xAODMuonPrepData StoreGateLib GaudiKernel)

atlas_install_python_modules( python/*.py)