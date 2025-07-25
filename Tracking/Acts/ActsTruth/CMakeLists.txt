# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTruth )

# for pdg_id -> name
find_package( HepPDT )

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
                     PRIVATE_INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS}
                     LINK_LIBRARIES
                       ActsEventLib
		       ActsTruthLib
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
		       AthenaBaseComps
		       CxxUtils
		       GaudiKernel
		       GeneratorObjects
		       Identifier
		       InDetSimData
		       StoreGateLib
		       TrkTruthTrackInterfaces
		       xAODInDetMeasurement
		       xAODMeasurementBase
		       xAODTruth
                       TruthUtils
                       PRIVATE_LINK_LIBRARIES
                       ${HEPPDT_LIBRARIES} )
