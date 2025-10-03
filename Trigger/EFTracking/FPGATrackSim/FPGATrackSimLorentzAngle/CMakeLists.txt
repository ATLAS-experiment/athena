# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimLorentzAngle )

atlas_add_library( FPGATrackSimLorentzAngleLib
                   src/*.cxx
                   PUBLIC_HEADERS FPGATrackSimLorentzAngle
                   LINK_LIBRARIES  AthenaBaseComps FPGATrackSimObjectsLib PixelReadoutGeometryLib SCT_ReadoutGeometry AthenaKernel GaudiKernel
)

atlas_add_component(FPGATrackSimLorentzAngle
    src/components/*.cxx
    LINK_LIBRARIES  FPGATrackSimLorentzAngleLib
)
