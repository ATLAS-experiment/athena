# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTruth )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_library(ActsTruthLib
                  ActsTruth/*.h ActsTruth/*.icc
		  INTERFACE
                  PUBLIC_HEADERS ActsTruth
                  LINK_LIBRARIES ActsInteropLib )


atlas_add_component( ActsTruth
                     src/*.h src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
                     LINK_LIBRARIES
                       ActsEventLib
			   		   Acts::Core
		       ActsTruthLib
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
			   ActsCalibratorsLib
		       AthenaBaseComps
		       CxxUtils
		       GaudiKernel
		       GeneratorObjects
		       GeneratorModulesLib
		       Identifier
		       InDetSimData
		       MCTruthClassifierLib
		       StoreGateLib
		       TrkTruthTrackInterfaces
		       xAODInDetMeasurement
		       xAODMeasurementBase
		       xAODTruth
                       TruthUtils
               )
