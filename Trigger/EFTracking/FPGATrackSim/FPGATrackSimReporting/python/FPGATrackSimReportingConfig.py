# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FPGATrackSimReportingCfg(flags,name='FPGATrackSimReportingAlg',**kwargs):
    acc = ComponentAccumulator()
    
    xAODPixelClustersOfInterest=[]
    xAODStripClustersOfInterest=[]
    
    xAODPixelClustersOfInterest += ["ITkPixelClusters" ,"xAODPixelClusters_1stFromFPGACluster", "xAODPixelClusters_1stFromFPGAHit"]
    xAODStripClustersOfInterest += ["ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit"]
    
    kwargs.setdefault('perEventReports',True)
    kwargs.setdefault('xAODPixelClusterContainersFromFPGA',["ITkPixelClusters" ,"xAODPixelClusters_1stFromFPGACluster", "xAODPixelClusters_1stFromFPGAHit"])
    kwargs.setdefault('xAODStripClusterContainersFromFPGA',["ITkStripClusters" ,"xAODStripClusters_1stFromFPGACluster", "xAODStripClusters_1stFromFPGAHit","xAODStripClustersFromFPGASP"])
    kwargs.setdefault('FPGATrackSimTracks','FPGATracks_1st')
    kwargs.setdefault('FPGATrackSimRoads','FPGARoads_1st')
    kwargs.setdefault('FPGATrackSimProtoTracks','ActsProtoTracks_1stFromFPGATrack')
    
    acc.addEventAlgo(CompFactory.FPGATrackSim.FPGATrackSimReportingAlg(name,**kwargs))
    return acc
