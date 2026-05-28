# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZWindowRoISeedTool )

# Component(s) in the package:
atlas_add_component( ZWindowRoISeedTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps BeamSpotConditionsData GaudiKernel InDetRecToolInterfaces ITrackToVertex xAODEventInfo)
