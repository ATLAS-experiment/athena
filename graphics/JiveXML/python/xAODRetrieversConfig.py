# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def xAODElectronRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODElectronRetriever(
            name="xAODElectronRetriever",
            StoreGateKey="Electrons",
            OtherCollections=["Electrons"],
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODMissingETRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODMissingETRetriever(
            name="xAODMissingETRetriever",
            FavouriteMETCollection="MET_Reference_AntiKt4EMPFlow",
            OtherMETCollections=[
                "MET_Reference_AntiKt4EMTopo",
                "MET_Calo",
                "MET_LocHadTopo",
                "MET_Core_AntiKt4LCTopo",
            ],
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODMuonRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODMuonRetriever(
            name="xAODMuonRetriever", StoreGateKey="Muons", OtherCollections=["Muons"]
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODPhotonRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODPhotonRetriever(
            name="xAODPhotonRetriever",
            StoreGateKey="Photons",
            OtherCollections=["Photons"],
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODJetRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODJetRetriever(
            name="xAODJetRetriever",
            FavouriteJetCollection="AntiKt4EMPFlowJets",
            OtherJetCollections=[
                "AntiKt4EMTopoJets",
                "AntiKt4LCTopoJets",
                "AntiKt10LCTopoJets",
                "AntiKt10UFOCSSKJets",
            ],
            BTaggerNames=[
                "DL1dv01",
                "GN2v01",
            ],
            CDIPaths=[
                "xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root",
                "xAODBTaggingEfficiency/13p6TeV/2023-02_MC23_CDI_GN2v01-noSF.root",
            ]
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODTauRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODTauRetriever(
            name="xAODTauRetriever", StoreGateKey="TauJets"
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODTrackParticleRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODTrackParticleRetriever(
            name="xAODTrackParticleRetriever",
            StoreGateKey="InDetTrackParticles",
            OtherTrackCollections=[
                "InDetLargeD0TrackParticles",
                "CombinedMuonTrackParticles",
                "GSFTrackParticles",
            ],
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODVertexRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODVertexRetriever(
            name="xAODVertexRetriever",
            PrimaryVertexCollection="PrimaryVertices",
            SecondaryVertexCollection="BTagging_AntiKt2TrackSecVtx",
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODCaloClusterRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODCaloClusterRetriever(
            name="xAODCaloClusterRetriever",
            FavouriteClusterCollection="egammaClusters",
            OtherClusterCollections=["CaloCalTopoClusters"],
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODRetrieversCfg(flags):
    # Based on xAODJiveXML_DataTypes.py
    result = ComponentAccumulator()
    tools = []
    # It's not really necessary to configure these, since nothing depends on flags.
    # We could just make all this the default in cpp (if it is not already)
    tools += [result.getPrimaryAndMerge(xAODElectronRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODMissingETRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODMuonRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODPhotonRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODJetRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODTauRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODTrackParticleRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODVertexRetrieverCfg(flags))]#
    tools += [result.getPrimaryAndMerge(xAODCaloClusterRetrieverCfg(flags))]

    return result, tools
