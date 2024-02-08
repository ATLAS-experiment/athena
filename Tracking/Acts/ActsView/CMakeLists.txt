# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsView)

atlas_add_library( ActsViewLib
                   ActsView/*.h
                   INTERFACE
                   PUBLIC_HEADERS ActsView
                   LINK_LIBRARIES
		     AthLinks
		     GaudiKernel
		     TrigSteeringEvent )

atlas_add_component( ActsView
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
		       ActsViewLib
		       AthContainers
		       AthViews
		       AthenaBaseComps
		       BeamSpotConditionsData
		       StoreGateLib
		       TrigSteeringEvent
		       TrkCaloClusterROI )
