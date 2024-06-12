# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def xAODRetrieversCfg(flags):
    # Based on xAODJiveXML_DataTypes.py
    result = ComponentAccumulator()
    # It's not really necessary to configure these, since nothing depends on flags.
    # We could just make all this the default in cpp (if it is not already)
    result.addPublicTool(
        CompFactory.JiveXML.xAODElectronRetriever(
            name="xAODElectronRetriever",
            StoreGateKey="Electrons",
            OtherCollections=["Electrons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMissingETRetriever(
            name="xAODMissingETRetriever",
            FavouriteMETCollection="MET_Reference_AntiKt4EMPFlow",
            OtherMETCollections=[
                "MET_Reference_AntiKt4EMTopo",
                "MET_Calo",
                "MET_LocHadTopo",
                "MET_Core_AntiKt4LCTopo",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMuonRetriever(
            name="xAODMuonRetriever", StoreGateKey="Muons", OtherCollections=["Muons"]
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODPhotonRetriever(
            name="xAODPhotonRetriever",
            StoreGateKey="Photons",
            OtherCollections=["Photons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODJetRetriever(
            name="xAODJetRetriever",
            FavouriteJetCollection="AntiKt4EMPFlowJets",
            OtherJetCollections=[
                "AntiKt4EMTopoJets",
                "AntiKt4LCTopoJets",
                "AntiKt10LCTopoJets",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTauRetriever(
            name="xAODTauRetriever", StoreGateKey="TauJets"
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTrackParticleRetriever(
            name="xAODTrackParticleRetriever",
            StoreGateKey="InDetTrackParticles",
            OtherTrackCollections=[
                "InDetLargeD0TrackParticles",
                "CombinedMuonTrackParticles",
                "GSFTrackParticles",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODVertexRetriever(
            name="xAODVertexRetriever",
            PrimaryVertexCollection="PrimaryVertices",
            SecondaryVertexCollection="BTagging_AntiKt2TrackSecVtx",
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODCaloClusterRetriever(
            name="xAODCaloClusterRetriever",
            FavouriteClusterCollection="egammaClusters",
            OtherClusterCollections=["CaloCalTopoClusters"],
        )
    )
    return result
