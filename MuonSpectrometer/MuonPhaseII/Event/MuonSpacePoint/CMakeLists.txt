# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePoint
################################################################################

# Declare the package name:
atlas_subdir( MuonSpacePoint )


atlas_add_library( MuonSpacePoint
                   src/*.cxx
                   PUBLIC_HEADERS MuonSpacePoint
                   LINK_LIBRARIES xAODMuonPrepData AthenaKernel GeoPrimitives GaudiKernel 
                                  Identifier ActsGeometryInterfacesLib MuonIdHelpersLib MuonReadoutGeometryR4)
