# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsView)

atlas_add_library( ActsViewLib
                   ActsView/*.h
                   INTERFACE
                   PUBLIC_HEADERS ActsView
                   LINK_LIBRARIES
                   GaudiKernel
		   TrigSteeringEvent
		   AthViews
                   )

atlas_add_component( ActsView
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
		        ActsViewLib
			TrkCaloClusterROI
			BeamSpotConditionsData
		     )

