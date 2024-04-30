# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCalibTools )

# Component(s) in the package:
atlas_add_library( MuonCalibToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCalibTools
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel MuonCalibITools MuonIdHelpersLib )

atlas_add_component( MuonCalibTools
                     src/components/*.cxx
                     LINK_LIBRARIES MuonCalibToolsLib )
