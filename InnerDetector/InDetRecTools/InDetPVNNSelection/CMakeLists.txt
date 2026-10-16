# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetPVNNSelection )

# Libraries in the package:
atlas_add_library( InDetPVNNSelectionLib
    Root/*.cxx
    PUBLIC_HEADERS InDetPVNNSelection
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
    PhotonVertexSelectionLib
    TrackVertexAssociationToolLib
    xAODParticleEvent
)


if( NOT XAOD_STANDALONE )
    atlas_add_component( InDetPVNNSelection
        src/*.h src/*.cxx src/components/*.cxx
        LINK_LIBRARIES InDetPVNNSelectionLib )
endif()

atlas_add_dictionary( InDetPVNNSelectionDict
    InDetPVNNSelection/InDetPVNNSelectionDict.h
    InDetPVNNSelection/selection.xml
    LINK_LIBRARIES InDetPVNNSelectionLib )


