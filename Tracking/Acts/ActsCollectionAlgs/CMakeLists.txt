# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsCollectionAlgs )

# Component(s) in the package:
atlas_add_component( ActsCollectionAlgs
                        src/*.h src/*.cxx
                        src/components/*.cxx
                     LINK_LIBRARIES
		        AthenaBaseComps
			AthLinks
		        GaudiKernel
			StoreGateLib
		        xAODInDetMeasurement
			xAODMeasurementBase
		    )
