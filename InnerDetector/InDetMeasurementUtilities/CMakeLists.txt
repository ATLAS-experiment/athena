# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetMeasurementUtilities )

atlas_add_library( InDetMeasurementUtilitiesLib
		   src/ClusterConversionUtilities.cxx
		   src/SpacePointConversionUtilities.cxx
		   src/Helpers.cxx
		   PUBLIC_HEADERS
		      InDetMeasurementUtilities
   		   LINK_LIBRARIES
		      InDetPrepRawData
		      HGTD_PrepRawData
		      xAODInDetMeasurement
		      InDetIdentifier
		      HGTD_Identifier
		      InDetReadoutGeometry
		      PixelReadoutGeometryLib
		      HGTD_ReadoutGeometry
		      SCT_ReadoutGeometry
		      TrkSpacePoint
		      SiSpacePoint
		   )

# Component(s) in the package:
atlas_add_component( InDetMeasurementUtilities
                     src/*.cxx src/*.h
		     src/components/*.cxx
                     LINK_LIBRARIES
		 		        InDetMeasurementUtilitiesLib
						GaudiKernel
						AthenaBaseComps
						StoreGateLib
						BeamSpotConditionsData
                                                AthContainers
                                                AthAllocators
		     )

atlas_install_scripts( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_python_modules(python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
