# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimActsInspectionToolCfg():
    acc  = ComponentAccumulator()
    actsTrackInspection = CompFactory.FPGATrackSim.ActsTrackInspectionTool()
    acc.setPrivateTools(actsTrackInspection)
    return acc

def FPGATrackSimReportingCfg(flags,name='FPGATrackSimReportingAlg',**kwargs):
    acc = ComponentAccumulator()
    
    xAODPixelClustersOfInterest=[]
    xAODStripClustersOfInterest=[]
    
    xAODPixelClustersOfInterest += ["ITkPixelClusters" ,"xAODPixelClusters_1stFromFPGACluster", "xAODPixelClusters_1stFromFPGAHit"]
    xAODStripClustersOfInterest += ["ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit","xAODSpacePoints_1stFromFPGASP"]
    
    kwargs.setdefault('perEventReports',True)
    kwargs.setdefault('xAODPixelClusterContainers',["ITkPixelClusters" ,"xAODPixelClusters_1stFromFPGACluster", "xAODPixelClusters_1stFromFPGAHit"])
    kwargs.setdefault('xAODStripClusterContainers',["ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit"])
    kwargs.setdefault('xAODSpacePointContainersFromFPGA',["xAODStripSpacePoints_1stFromFPGA","xAODPixelSpacePoints_1stFromFPGA"])
    kwargs.setdefault('FPGATrackSimTracks','FPGATracks_1st')
    kwargs.setdefault('FPGATrackSimRoads','FPGARoads_1st')
    kwargs.setdefault('FPGATrackSimProtoTracks',["ActsProtoTracks_1stFromFPGATrack"])
    kwargs.setdefault('FPGAActsTracks',["ACTSProtoTrackChainTestTracks","ExtendedFPGATracks"])
    
    reportinAlgorithm = CompFactory.FPGATrackSim.FPGATrackSimReportingAlg(name,**kwargs)
    reportinAlgorithm.ActsInspectionTool = acc.getPrimaryAndMerge(FPGATrackSimActsInspectionToolCfg())
    acc.addEventAlgo(CompFactory.FPGATrackSim.FPGATrackSimReportingAlg(name,**kwargs))
    return acc
