# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPTracker )

# External dependencies:
find_package( Boost COMPONENTS program_options )

# this line failed automatic conversion in cmt2cmake :
# macro_prepend fpTrackerMainlinkopts " $(Boost_linkopts_program_options) "

# Component(s) in the package:
atlas_add_library( FPTracker
                   FPTracker/*.h src/*.cxx
                   PUBLIC_HEADERS FPTracker
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils )

atlas_add_executable( fpTrackerMain
                      test/fpTrackerMain.cxx
                      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES ${Boost_LIBRARIES} FPTracker )
