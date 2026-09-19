# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( TrkAnalysisInterfaces )

# Component(s) in the package:
atlas_add_library( TrkAnalysisInterfacesLib
   TrkAnalysisInterfaces/*.h
   INTERFACE
   PUBLIC_HEADERS TrkAnalysisInterfaces
   LINK_LIBRARIES AsgTools xAODEventInfo  PATCoreLib PATInterfaces  xAODTracking)
