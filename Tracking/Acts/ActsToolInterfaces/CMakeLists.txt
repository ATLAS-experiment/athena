# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsToolInterfaces )

# External dependencies:
find_package( Acts COMPONENTS Core )

# External dependencies:
# Component(s) in the package:
atlas_add_library( ActsToolInterfacesLib
                   INTERFACE
                   PUBLIC_HEADERS ActsToolInterfaces
                   LINK_LIBRARIES
                   GaudiKernel
                   BeamSpotConditionsData
                   ActsCore
                   ActsEventLib
		   ActsGeometryLib
                   SiSpacePointFormationLib
                   InDetReadoutGeometry
		   InDetIdentifier
                   xAODInDetMeasurement
                   TrkFitterUtils
                   )
