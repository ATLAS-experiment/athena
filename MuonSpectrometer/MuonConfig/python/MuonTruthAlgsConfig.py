# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetailedTrackTruthMakerCfg(flags, name="MuonDetailedTrackTruthMaker", **kwargs):
    result = ComponentAccumulator()
    
    PRD_TruthNames = ["RPC_TruthMap", "TGC_TruthMap", "MDT_TruthMap"]
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
def MuonTruthClassificationAlgCfg(flags, name="MuonTruthClassificationAlg", **kwargs):
    result = ComponentAccumulator()

    from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
    kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(MCTruthClassifierCfg(flags)))
        
    result.addEventAlgo(CompFactory.Muon.MuonTruthClassificationAlg(name, **kwargs))
    return result

def MuonTruthAddTrackRecordsAlgCfg(flags, name="MuonTruthAddTrackRecordsAlg", **kwargs):
    result = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
        
    trackRecords = [item for item in ["CaloEntryLayer", "MuonEntryLayer", "MuonExitLayer"] if item in flags.Input.Collections]
    kwargs.setdefault("TrackRecordCollectionNames", trackRecords)

    result.addEventAlgo(CompFactory.Muon.MuonTruthAddTrackRecordsAlg(name, **kwargs))
    return result

def MuonTruthHitCountsAlgCfg(flags, name="MuonTruthHitCountsAlg", **kwargs):
    result = ComponentAccumulator()

    PRD_TruthMaps = ["RPC_TruthMap","TGC_TruthMap","MDT_TruthMap"]

    if flags.Detector.EnablesTGC and flags.Detector.EnableMM:
        PRD_TruthMaps += ["MM_TruthMap", "STGC_TruthMap"]
    if flags.Detector.EnableCSC: 
        PRD_TruthMaps += ["CSC_TruthMap"]
    
    kwargs.setdefault("PRD_TruthMaps", PRD_TruthMaps)
  
    result.addEventAlgo(CompFactory.Muon.MuonTruthHitCountsAlg(name, **kwargs))
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
    result.addEventAlgo(CompFactory.MuonTruthAssociationAlg(name, **kwargs))
    return result

def MuonSegmentTruthAssociationAlgCfg(flags, name="MuonSegmentTruthAssociationAlg", **kwargs):
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.Muon.MuonSegmentTruthAssociationAlg(name, **kwargs))
    return result
