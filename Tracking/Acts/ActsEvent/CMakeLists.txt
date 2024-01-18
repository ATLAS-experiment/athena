# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsEvent )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( Boost )
find_package( Eigen )

atlas_add_library( ActsEventLib
                   Root/*.cxx
                   PUBLIC_HEADERS ActsEvent
                   INCLUDE_DIRS ${Acts_INCLUDE_DIR}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${EIGEN_LIBRARIES} 
		   AthenaBaseComps GaudiKernel AtlasDetDescr CxxUtils
		   xAODCore ActsCore ActsFatras ActsGeometryLib BeamSpotConditionsData
		   MagFieldConditions MagFieldElements GeoPrimitives xAODTracking xAODInDetMeasurement
		   )

atlas_add_dictionary( ActsEventDict
		      ActsEvent/ActsEventDict.h
		      ActsEvent/selection.xml
		      LINK_LIBRARIES xAODCore ActsEventLib
		      DATA_LINKS 
		      ActsTrk::SeedContainer 
		      ActsTrk::BoundTrackParametersContainer
		      )

atlas_add_test( MultiTrajectoryBasic_test 
				SOURCES test/MultiTrajectoryBasic_test.cxx
				LINK_LIBRARIES TestTools ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( MultiTrajectorySE_test 
				SOURCES test/MultiTrajectorySE_test.cxx
				LINK_LIBRARIES TestTools ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)				

atlas_add_test( TrackStorageContainer_test
				SOURCES test/TrackStorageContainer_test.cxx
				LINK_LIBRARIES TestTools ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)
 