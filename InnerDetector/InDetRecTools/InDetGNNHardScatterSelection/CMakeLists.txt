# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetGNNHardScatterSelection )

# Libraries in the package:
atlas_add_library( InDetGNNHardScatterSelectionLib
    Root/*.cxx
    PUBLIC_HEADERS InDetGNNHardScatterSelection
    PRIVATE_LINK_LIBRARIES PathResolver CxxUtils
    LINK_LIBRARIES AthLinks 
    AsgDataHandlesLib 
    AsgTools 
    GaudiKernel
    InDetRecToolInterfaces 
    InDetTrackSelectionToolLib
    InDetTruthVertexValidationLib 
    xAODTracking 
    xAODJet 
    FlavorTagInference
    FlavorTagDiscriminants
    FourMomUtils 
    AthenaBaseComps
    AthContainers
    xAODBase
    xAODBTagging
    xAODEventInfo
    xAODEgamma
    xAODMuon
    xAODTau
    xAODTruth
    StoreGateLib
    SystematicsHandlesLib
    FourMomUtils
    TruthUtils
    InDetPhysValMonitoringLib
    TrackVertexAssociationToolLib
)


if( NOT XAOD_STANDALONE )
    atlas_add_component( InDetGNNHardScatterSelection
        src/*.h src/*.cxx src/components/*.cxx
        LINK_LIBRARIES InDetGNNHardScatterSelectionLib )
endif()

atlas_add_dictionary( InDetGNNHardScatterSelectionDict
    InDetGNNHardScatterSelection/InDetGNNHardScatterSelectionDict.h
    InDetGNNHardScatterSelection/selection.xml
    LINK_LIBRARIES InDetGNNHardScatterSelectionLib )


