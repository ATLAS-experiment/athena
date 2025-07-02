# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkEventPrimitives )

find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( TrkEventPrimitives
                   src/*.cxx
                   PUBLIC_HEADERS TrkEventPrimitives
                   LINK_LIBRARIES GeoPrimitives EventPrimitives GaudiKernel TruthUtils)

atlas_add_dictionary( TrkEventPrimitivesDict
                      TrkEventPrimitives/TrkEventPrimitivesDict.h
                      TrkEventPrimitives/selection.xml
                      LINK_LIBRARIES TrkEventPrimitives )

atlas_add_test( 
  LocalParameters_test
  SOURCES test/LocalParameters_test.cxx
  LINK_LIBRARIES TrkEventPrimitives
  )

atlas_add_test( 
  SurfaceConsistencyCheck_test
  SOURCES test/SurfaceConsistencyCheck_test.cxx
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils TrkEventPrimitives
  POST_EXEC_SCRIPT nopost.sh
  )


