# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsRegionsOfInterest)

atlas_add_component( ActsRegionsOfInterest
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
		       GaudiKernel
		       ActsToolInterfacesLib
		       AthContainers
		       AthViews
		       AthenaBaseComps
		       BeamSpotConditionsData
		       StoreGateLib
		       TrigSteeringEvent
		       TrkCaloClusterROI )
