# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( TrkAnalysisInterfaces )

# Component(s) in the package:
atlas_add_library( TrkAnalysisInterfacesLib
   TrkAnalysisInterfaces/*.h
   INTERFACE
   PUBLIC_HEADERS TrkAnalysisInterfaces
   LINK_LIBRARIES AsgTools xAODEventInfo  PATCoreLib PATInterfaces  xAODTracking)

if (NOT XAOD_STANDALONE)
  target_link_libraries(TrkAnalysisInterfacesLib INTERFACE TrkTrack ) # Why did I need this?
endif()


# if( NOT XAOD_STANDALONE )
#    atlas_add_component( TrkAnalysisInterfaces
#       src/*.h src/*.cxx src/components/*.cxx
#       LINK_LIBRARIES AthenaBaseComps GaudiKernel xAODCore PathResolver xAODTracking
#       PixelToTPIDToolLib PATCoreLib TrkTrack TrkSurfaces TrkMeasurementBase TrkParameters TrkRIO_OnTrack TrkToolInterfaces TrkAnalysisInterfacesLib )
# endif()

atlas_add_dictionary( TrkAnalysisInterfacesDict
   TrkAnalysisInterfaces/TrkAnalysisInterfacesDict.h
   TrkAnalysisInterfaces/selection.xml
   LINK_LIBRARIES TrkAnalysisInterfacesLib )
