# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( MuonDigitizationR4 )


atlas_add_library( MuonDigitizationR4
                   src/*.cxx
                   PUBLIC_HEADERS MuonDigitizationR4
                   LINK_LIBRARIES MuonReadoutGeometryR4 PileUpToolsLib xAODMuonSimHit
                                  ActsGeometryInterfacesLib MuonIdHelpersLib HitManagement
                                  AthenaBaseComps StoreGateLib)

# Component(s) in the package:
atlas_add_component( MuonDigitizationR4Comp
                     src/MuonDigitizer.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel PileUpToolsLib)

