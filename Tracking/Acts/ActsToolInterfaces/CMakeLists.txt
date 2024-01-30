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
<<<<<<< HEAD
		     ActsCore
		     ActsEventLib
		     ActsGeometryLib
		     BeamSpotConditionsData
		     GaudiKernel
		     InDetIdentifier
		     InDetRawData
		     InDetReadoutGeometry
		     SiSpacePointFormationLib
		     TrkTrack
		     xAODInDetMeasurement
		     xAODMeasurementBase )
=======
                   GaudiKernel
                   BeamSpotConditionsData
                   ActsCore
                   ActsEventLib
		   ActsGeometryLib
                   SiSpacePointFormationLib
                   InDetReadoutGeometry
		   InDetIdentifier
                   xAODInDetMeasurement
                   xAODMeasurementBase
                   TrkFitterUtils
                   )
>>>>>>> ceee5aad679 (reverting accidental changes...)
