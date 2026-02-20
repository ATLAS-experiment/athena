#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonR4SegmentCnvAlgCfg(flags, name="MuonR4SegmentCnvAlg", **kwargs):
    result = ComponentAccumulator()

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    if not flags.Detector.GeometryMDT: kwargs.setdefault("MdtKey" ,"")    
    if not flags.Detector.GeometryRPC: kwargs.setdefault("RpcKey" ,"")
    if not flags.Detector.GeometryTGC: kwargs.setdefault("TgcKey" ,"")
    if not flags.Detector.GeometryMM: kwargs.setdefault("MmKey" ,"")
    if not flags.Detector.GeometrysTGC: kwargs.setdefault("sTgcKey" ,"")

    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg, MdtDriftCircleOnTrackCreatorCfg, TriggerChamberClusterOnTrackCreatorCfg

    if flags.Detector.GeometryMDT:
        kwargs.setdefault("MdtRotCreator", result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags)))
    if flags.Detector.GeometryRPC or flags.Detector.GeometryTGC \
     or flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        kwargs.setdefault("ClusterRotCreator", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))
    if flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        kwargs.setdefault("CompetingRotCreator", result.getPrimaryAndMerge(TriggerChamberClusterOnTrackCreatorCfg(flags)))
    from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg
    kwargs.setdefault("printerTool", result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags)))
    the_alg = CompFactory.MuonR4.TrkSegmentCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def xAODSegmentCnvAlgCfg(flags, name="MuonR4xAODSegmentCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    the_alg = CompFactory.MuonR4.xAODSegmentCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
