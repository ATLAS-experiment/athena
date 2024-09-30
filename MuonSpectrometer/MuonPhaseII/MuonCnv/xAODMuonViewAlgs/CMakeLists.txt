# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonViewAlgs )

# Component(s) in the package:
atlas_add_component( xAODMuonViewAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps xAODMuonPrepData StoreGateLib GaudiKernel xAODMuon)

atlas_install_python_modules( python/*.py)