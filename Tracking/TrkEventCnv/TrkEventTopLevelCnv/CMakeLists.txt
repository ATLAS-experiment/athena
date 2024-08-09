# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkEventTopLevelCnv )

# Component(s) in the package:
atlas_add_library( TrkEventTopLevelCnv
   TrkEventTopLevelCnv/*.h src/*.cxx
   PUBLIC_HEADERS TrkEventTopLevelCnv
   LINK_LIBRARIES
   InDetEventTPCnv MuonEventTPCnv TrkEventTPCnv RecTPCnv TPTools
   )

atlas_add_tpcnv_library( TrkEventTopLevelCnvFactories
   src/factories/*.cxx
   PUBLIC_HEADERS TrkEventTopLevelCnv
   LINK_LIBRARIES TrkEventTopLevelCnv
   )

atlas_add_dictionary( TrkEventTopLevelCnvDict
   TrkEventTopLevelCnv/TrkEventTopLevelCnvDict.h
   TrkEventTopLevelCnv/selection.xml
   LINK_LIBRARIES TrkEventTopLevelCnv
   )

atlas_add_test( GSFCnv_test
   SOURCES test/GSFCnv_test.cxx
   LINK_LIBRARIES GeoPrimitives GaudiKernel TrkSurfaces TrkEventPrimitives
   TrkMaterialOnTrack TrkMeasurementBase TrkPseudoMeasurementOnTrack
   TrkTrackSummary TestTools AthenaBaseComps CxxUtils TrkEventTPCnv TrkEventTopLevelCnv)
