# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrkConverterInterface )

# External dependencies:

# Component(s) in the package:
atlas_add_library( FPGATrkConverterInterface
                   FPGATrkConverterInterface/*.h
                   INTERFACE
                   PUBLIC_HEADERS FPGATrkConverterInterface
                   LINK_LIBRARIES  GaudiKernel FPGATrackSimInputLib)

