# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimActsInspectionToolCfg():
    acc  = ComponentAccumulator()
    actsTrackInspection = CompFactory.FPGATrackSim.ActsTrackInspectionTool()
    acc.setPrivateTools(actsTrackInspection)
    return acc

def FPGATrackSimReportingCfg(flags,name='FPGATrackSimReportingAlg',stage="",**kwargs):
    acc = ComponentAccumulator()
    
    xAODPixelClustersOfInterest=[]
    xAODStripClustersOfInterest=[]
    
    xAODPixelClustersOfInterest += ["ITkPixelClusters" ,"xAODPixelClustersFromFPGACluster", f"xAODPixelClusters{stage}FromFPGAHit"]
    xAODStripClustersOfInterest += ["ITkStripClusters" ,"xAODStripClustersFromFPGACluster", f"xAODStripClusters{stage}FromFPGAHit",f"xAODSpacePoints{stage}FromFPGASP"]
    
    kwargs.setdefault('perEventReports',True)
    kwargs.setdefault('xAODPixelClusterContainers',["ITkPixelClusters" ,"xAODPixelClustersFromFPGACluster", f"xAODPixelClusters{stage}FromFPGAHit"])
    kwargs.setdefault('xAODStripClusterContainers',["ITkStripClusters" ,"xAODStripClustersFromFPGACluster", f"xAODStripClusters{stage}FromFPGAHit"])
    kwargs.setdefault('xAODSpacePointContainersFromFPGA',[f"xAODStripSpacePoints{stage}FromFPGA",f"xAODPixelSpacePoints{stage}FromFPGA"])
    kwargs.setdefault('FPGATrackSimTracks','FPGATracks')
    kwargs.setdefault('FPGATrackSimRoads','FPGARoads')
    kwargs.setdefault('FPGATrackSimProtoTracks',[f"ActsProtoTracks{stage}FromFPGATrack",f"ActsProtoTracks{stage}FromFPGARoad"])
    kwargs.setdefault('FPGAActsTracks',["ACTSProtoTrackChainTestTracks","ExtendedFPGATracks"])
    
    reportinAlgorithm = CompFactory.FPGATrackSim.FPGATrackSimReportingAlg(name,**kwargs)
    reportinAlgorithm.ActsInspectionTool = acc.getPrimaryAndMerge(FPGATrackSimActsInspectionToolCfg())
    acc.addEventAlgo(reportinAlgorithm)
    return acc
