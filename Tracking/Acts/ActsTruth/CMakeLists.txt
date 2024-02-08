# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTruth )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_component( ActsTruth
                     src/*.h src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
                     LINK_LIBRARIES
                       ActsEventLib
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
		       ActsInteropLib
		       AthenaBaseComps
		       CxxUtils
		       GaudiKernel
		       GeneratorObjects
		       Identifier
		       InDetSimData
		       StoreGateLib
		       xAODInDetMeasurement
		       xAODMeasurementBase
		       xAODTruth )
