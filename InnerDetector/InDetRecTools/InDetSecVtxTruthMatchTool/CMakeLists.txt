# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSecVtxTruthMatchTool )

# External(s):
find_package( ROOT COMPONENTS Core RIO Hist MathCore )

atlas_add_library( InDetSecVtxTruthMatchToolLib
   InDetSecVtxTruthMatchTool/*.h Root/*.h Root/*.cxx
   PUBLIC_HEADERS InDetSecVtxTruthMatchTool
   LINK_LIBRARIES PATCoreLib AsgAnalysisInterfaces TruthUtils xAODTracking xAODTruth InDetTrackSystematicsToolsLib)

if( NOT XAOD_STANDALONE )
   atlas_add_component( InDetSecVtxTruthMatchTool
      src/components/*.cxx
      LINK_LIBRARIES GaudiKernel AthenaBaseComps xAODTracking xAODTruth
      InDetSecVtxTruthMatchToolLib)
endif()

atlas_add_dictionary( InDetSecVtxTruthMatchToolDict
   InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchToolDict.h
   InDetSecVtxTruthMatchTool/selection.xml
   LINK_LIBRARIES InDetSecVtxTruthMatchToolLib )
