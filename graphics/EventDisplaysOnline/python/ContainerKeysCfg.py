#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def getHIContainterKeys(cfg):
    cfg.getPublicTool("xAODJetRetriever").BTaggerNames = [""]
    cfg.getPublicTool("xAODJetRetriever").JetCollections = ["AntiKt2HIJets","AntiKt4HIJets","AntiKt4HITrackJets"]
    cfg.getPublicTool("xAODVertexRetriever").VertexCollections = ["PrimaryVertices","BTagging_AntiKt4HISecVtx","GSFConversionVertices","MSDisplacedVertex"]
    cfg.getPublicTool("xAODCaloClusterRetriever").ClusterCollections = ["egammaClusters","HIClusters"]
    cfg.getPublicTool("xAODMissingETRetriever").METCollections = [""]
    cfg.getPublicTool("xAODTauRetriever").TauJetCollections = [""]
    cfg.getPublicTool("xAODTrackParticleRetriever").TrackParticleCollections = ["InDetTrackParticles","CombinedMuonTrackParticles","GSFTrackParticles"]
    cfg.getPublicTool("TrackRetriever").TrackCollections = ["CombinedInDetTracks","CombinedMuonTracks","GSFTracks"]   
    return cfg

def getHIPContainterKeys(cfg):
    cfg.getPublicTool("xAODJetRetriever").BTaggerNames = [""]
    cfg.getPublicTool("xAODJetRetriever").JetCollections = ["AntiKt2HIJets","AntiKt4HIJets","AntiKt4HITrackJets","AntiKt4EMTopoJets","AntiKt4LCTopoJets","AntiKt10LCTopoJets","AntiKt4EMPFlowJets"]
    cfg.getPublicTool("xAODVertexRetriever").VertexCollections = ["PrimaryVertices","BTagging_AntiKt4HISecVtx","BTagging_AntiKt4EMTopoSecVtx","BTagging_AntiKt4EMPFlowSecVtx","GSFConversionVertices","MSDisplacedVertex"]
    cfg.getPublicTool("xAODCaloClusterRetriever").ClusterCollections = ["egammaClusters","HIClusters"]
    cfg.getPublicTool("xAODTrackParticleRetriever").TrackParticleCollections = ["InDetTrackParticles","CombinedMuonTrackParticles","GSFTrackParticles"]
    cfg.getPublicTool("TrackRetriever").TrackCollections = ["CombinedInDetTracks","CombinedMuonTracks","GSFTracks"]
