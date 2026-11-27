# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AFP_GeoModel )

# External dependencies:
find_package( CLHEP )
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( AFP_GeoModel
                     src/*.cxx
                     src/components/*.cxx
		     LINK_LIBRARIES ${CLHEP_LIBRARIES} GeoModelCore::GeoModelKernel AFP_Geometry AthenaKernel 
			 				GeoModelUtilities AthenaPoolUtilities GaudiKernel GeoModelInterfaces 
							GeoPrimitives StoreGateLib )

