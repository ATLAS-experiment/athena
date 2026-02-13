# Copyright (C) 2002-2026 by CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPrdSelector )

atlas_add_library( MuonPrdSelectorLib
                   MuonPrdSelector/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonPrdSelector
                   LINK_LIBRARIES GaudiKernel MuonCalibToolsLib MuonCalibIdentifier Identifier AthenaBaseComps MuonCalibToolsLib MuonIdHelpersLib MuonPrepRawData )

# Component(s) in the package:
atlas_add_component( MuonPrdSelector
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES CxxUtils MuonCalibIdentifier MuonCalibToolsLib MuonPrdSelectorLib )
