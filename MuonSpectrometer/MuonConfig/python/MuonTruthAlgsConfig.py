# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetailedTrackTruthMakerCfg(flags, name="MuonDetailedTrackTruthMaker", **kwargs):
    result = ComponentAccumulator()
    
    PRD_TruthNames = []
    if flags.Detector.EnableRPC:
        PRD_TruthNames+=["RPC_TruthMap"]
    if flags.Detector.EnableTGC:
        PRD_TruthNames+=["TGC_TruthMap"]
    if flags.Detector.EnableMDT:
        PRD_TruthNames+=["MDT_TruthMap"]
    if flags.Detector.EnableCSC:
        PRD_TruthNames += ["CSC_TruthMap"]
    if flags.Detector.EnableMM:
        PRD_TruthNames += ["MM_TruthMap"]
    if flags.Detector.EnablesTGC:
        PRD_TruthNames += ["STGC_TruthMap"]

    kwargs.setdefault("PRD_TruthNames", PRD_TruthNames)
    result.addEventAlgo(CompFactory.MuonDetailedTrackTruthMaker(name, **kwargs))
    return result

# The following 4 configuration fragments replace the ld MuonTruthDecorationAlg config
def TruthMuonMakerAlgCfg(flags, name="TruthMuonMakerAlg", **kwargs):
    result = ComponentAccumulator()

    from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
    kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(MCTruthClassifierCfg(flags)))
        
    result.addEventAlgo(CompFactory.Muon.TruthMuonMakerAlg(name, **kwargs))
    return result

def MuonTruthAddTrackRecordsAlgCfg(flags, name="MuonTruthAddTrackRecordsAlg", **kwargs):
    result = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
        
    trackRecords = [item for item in ["CaloEntryLayer", "MuonEntryLayer", "MuonExitLayer"] if item in flags.Input.Collections]
    kwargs.setdefault("TrackRecordCollectionNames", trackRecords)

    result.addEventAlgo(CompFactory.Muon.TruthTrackRecordsAlg(name, **kwargs))
    return result

def MuonTruthHitCountsAlgCfg(flags, name="MuonTruthHitSummaryAlg", **kwargs):
    result = ComponentAccumulator()

    PRD_TruthNames = []
    if flags.Detector.EnableRPC:
        PRD_TruthNames+=["RPC_TruthMap"]
    if flags.Detector.EnableTGC:
        PRD_TruthNames+=["TGC_TruthMap"]
    if flags.Detector.EnableMDT:
        PRD_TruthNames+=["MDT_TruthMap"]
    if flags.Detector.EnableCSC:
        PRD_TruthNames += ["CSC_TruthMap"]
    if flags.Detector.EnableMM:
        PRD_TruthNames += ["MM_TruthMap"]
    if flags.Detector.EnablesTGC:
        PRD_TruthNames += ["STGC_TruthMap"]
    
    kwargs.setdefault("PRD_TruthMaps", PRD_TruthNames)
  
    result.addEventAlgo(CompFactory.Muon.TruthHitSummaryAlg(name, **kwargs))
    return result

def MuonTruthSegmentCreationAlgCfg(flags, name="MuonTruthSegmentCreationAlg", **kwargs):
    result = ComponentAccumulator()

    SDOs = ["RPC_SDO","TGC_SDO","MDT_SDO"]
    CSCSDOs = "CSC_SDO"

    if flags.Detector.EnablesTGC and flags.Detector.EnableMM:
        SDOs += ["MM_SDO","sTGC_SDO"]
    if not flags.Detector.EnableCSC: 
        CSCSDOs = ""
    
    kwargs.setdefault("SDOs", SDOs)
    kwargs.setdefault("CSCSDOs", CSCSDOs)

    result.addEventAlgo(CompFactory.Muon.MuonTruthSegmentCreationAlg(name, **kwargs))
    return result


def MuonTruthAssociationAlgCfg(flags, name="MuonTruthAssociationAlg", **kwargs):
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.Muon.RecoToTruthAssociationAlg(name, **kwargs))
    return result

def MuonSegmentTruthAssociationAlgCfg(flags, name="MuonSegmentTruthAssociationAlg", **kwargs):
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.Muon.MuonSegmentTruthAssociationAlg(name, **kwargs))
    return result

def MuonTruthAlgsCfg(flags):
    result = ComponentAccumulator()
    if not flags.Input.isMC or flags.Muon.usePhaseIIGeoSetup:
        return result
    from MuonConfig.MuonRdoDecodeConfig import MuonPRD_MultiTruthMakerCfg
    result.merge(MuonPRD_MultiTruthMakerCfg(flags))
    from MuonConfig.MuonTruthAlgsConfig import TruthMuonMakerAlgCfg
    result.merge(TruthMuonMakerAlgCfg(flags))
    from MuonConfig.MuonTruthAlgsConfig import MuonTruthAddTrackRecordsAlgCfg
    result.merge(MuonTruthAddTrackRecordsAlgCfg(flags))
    from MuonConfig.MuonTruthAlgsConfig import MuonTruthHitCountsAlgCfg
    result.merge(MuonTruthHitCountsAlgCfg(flags))
    from MuonConfig.MuonTruthAlgsConfig import MuonTruthSegmentCreationAlgCfg
    result.merge(MuonTruthSegmentCreationAlgCfg(flags))
    return result