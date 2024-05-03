# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimBanks )

# External dependencies:
find_package( Eigen )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Create a library for non-Gaudi components
atlas_add_library( FPGATrackSimBanksLib
   src/*.cxx
   PUBLIC_HEADERS FPGATrackSimBanks
   INCLUDE_DIRS   ${EIGEN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${EIGEN_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel FPGATrackSimMapsLib FPGATrackSimObjectsLib
   PRIVATE_LINK_LIBRARIES AsgMessagingLib )

# Declare Gaudi component(s) in the package
atlas_add_component( FPGATrackSimBanks
   src/components/*.cxx
   LINK_LIBRARIES FPGATrackSimBanksLib )  


#atlas_add_test(             FPGATrackSimFitConstantBank_test
#    SOURCES                 test/FPGATrackSimFitConstantBank_test.cxx
#    LINK_LIBRARIES          FPGATrackSimBanksLib FPGATrackSimObjectsLib FPGATrackSimMapsLib
#)
