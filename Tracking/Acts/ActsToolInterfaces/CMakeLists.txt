# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
		     Acts::Core
		     ActsEventLib
		     ActsGeometryLib
		     BeamSpotConditionsData
		     GaudiKernel
		     HGTD_RawData
		     InDetIdentifier
		     InDetRawData
		     InDetReadoutGeometry
		     SiSpacePointFormationLib
		     TrkTrack
		     xAODInDetMeasurement
		     xAODMeasurementBase )
