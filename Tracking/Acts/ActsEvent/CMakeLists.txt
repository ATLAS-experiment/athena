# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsEvent )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( Boost )

atlas_add_library( ActsEventLib
   ActsEvent/*.h Root/*.cxx
   PUBLIC_HEADERS ActsEvent
   INCLUDE_DIRS
   ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES
   ${Boost_LIBRARIES}
   ActsCore
   ActsGeometryLib
   ActsGeometryInterfacesLib
   AthContainers
   AthContainersInterfaces
   AthenaKernel
   AthLinks
   CxxUtils
   GaudiKernel
   xAODCore
   xAODInDetMeasurement
   xAODMeasurementBase
   xAODTracking
   xAODTruth
   xAODAuxillaryMeasurement
   PRIVATE_LINK_LIBRARIES
      ActsGeoUtils
)

atlas_add_dictionary( ActsEventDict
		      ActsEvent/ActsEventDict.h
		      ActsEvent/selection.xml
		      LINK_LIBRARIES AthLinks ActsEventLib)

atlas_add_test( MultiTrajectoryBasic_test
				SOURCES test/MultiTrajectoryBasic_test.cxx
				LINK_LIBRARIES ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MultiTrajectorySE_test
				SOURCES test/MultiTrajectorySE_test.cxx
				LINK_LIBRARIES ActsEventLib ActsGeoUtils
				POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MultiTrajectoryACTS_test
				SOURCES test/MultiTrajectoryACTS_test.cxx
				LINK_LIBRARIES ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TrackSummaryContainer_test
				SOURCES test/TrackSummaryContainer_test.cxx
				LINK_LIBRARIES ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)
