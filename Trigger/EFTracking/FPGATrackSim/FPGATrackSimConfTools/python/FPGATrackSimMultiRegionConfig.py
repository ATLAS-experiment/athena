# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FPGATrackSimMultiRegionTrackingCfg(flags):
    acc = ComponentAccumulator()
    acc.merge(FPGATrackSimRunFirstStageOnManyRegions(flags))
        
    if flags.Trigger.FPGATrackSim.Hough.secondStage:
        acc.merge(FPGATrackSimRunSecondStageOnManyRegions(flags))    
    acc.merge(FPGATrackSimRegionMergeringAlgCfg(flags))
    
    return acc
    
    
def FPGATrackSimRunFirstStageOnManyRegions(flags):
    acc = ComponentAccumulator()
    for region in flags.Trigger.FPGATrackSim.regionList:
        flags1st = flags.clone()
        flags1st.Trigger.FPGATrackSim.region=region
        flags1st = flags1st.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=True)
        flags1st.lock()
        
        from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimLogicalHitsProcessAlgCfg
        acc.merge(FPGATrackSimLogicalHitsProcessAlgCfg(flags1st,
                  **{'FPGATrackSimHitKey_2nd': f"FPGAHits_2nd_reg{region}",
                     'FPGATrackSimHitFiltered1stKey': f"FPGAHitsFiltered_1st_reg{region}",
                     'FPGATrackSimHitInRoads1stKey': f"FPGAHitsInRoads_1st_reg{region}",
                     'FPGATrackSimRoad1stKey': f"FPGARoads_1st_reg{region}",
                     'FPGATrackSimTrack1stKey': f"FPGATracks_1st_reg{region}",
                     'FPGATrackSimSpacePoints1stKey': f"FPGASpacePoints_1st_reg{region}"}))
    return acc
    
def FPGATrackSimRunSecondStageOnManyRegions(flags):
    acc = ComponentAccumulator()
    for region in flags.Trigger.FPGATrackSim.regionList:
        flags2nd = flags.clone()
        flags2nd.Trigger.FPGATrackSim.region=region
        flags2nd = flags2nd.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=True)
        flags2nd.lock()
        
        from FPGATrackSimConfTools import FPGATrackSimSecondStageConfig
        acc.merge(FPGATrackSimSecondStageConfig.FPGATrackSimSecondStageAlgCfg(flags2nd,
                  **{'FPGATrackSimHitKey': f"FPGAHits_2nd_reg{region}",
                     'FPGATrackSimTrack1stKey': f"FPGATracks_1st_reg{region}",
                     'FPGATrackSimHitInRoads2ndKey': f"FPGAHitsInRoads_2nd_reg{region}",
                     'FPGATrackSimRoad2ndKey': f"FPGARoads_2nd_reg{region}",
                     'FPGATrackSimTrack2ndKey': f"FPGATracks_2nd_reg{region}"}))
    return acc

def FPGATrackSimRegionMergeringAlgCfg(flagsIn,name="FPGATrackSimRegionMergingAlg",**kwargs):
    acc = ComponentAccumulator()
    flags = flagsIn.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flagsIn.Trigger.FPGATrackSim.algoTag,keepOriginal=False)
    stage="2nd" if flags.Trigger.FPGATrackSim.ActiveConfig.secondStage else "1st"
    
    TrackCollections = [f"FPGATracks_{stage}_reg{region}" for region in flags.Trigger.FPGATrackSim.regionList]
    RoadCollections = [f"FPGARoads_{stage}_reg{region}" for region in flags.Trigger.FPGATrackSim.regionList]
    HitsInRoadsCollections = [f"FPGAHitsInRoads_{stage}_reg{region}" for region in flags.Trigger.FPGATrackSim.regionList]
    
    kwargs.setdefault('FPGATrackSimTrackCollections',TrackCollections)
    kwargs.setdefault('FPGATrackSimRoadCollections',RoadCollections)
    kwargs.setdefault('FPGATrackSimHitsInRoadsCollections',HitsInRoadsCollections)
    
    regionMerging = CompFactory.FPGATrackSim.FPGATrackSimRegionMergingAlg(name,**kwargs)
    regionMerging.doOverlapRemoval = flags.Trigger.FPGATrackSim.doOverlapRemovalBetweenRegions


    regionMerging.useRoads = False # not flags.Trigger.FPGATrackSim.tracking (maybe we'll never actually use roads at this point)

    if stage == "2nd":
        from FPGATrackSimConfTools.FPGATrackSimSecondStageConfig import FPGATrackSimOverlapRemovalToolCfg
        regionMerging.OverlapRemovalTool = acc.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags,name="OLRMerge"))
    elif stage == "1st":
        from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimOverlapRemovalToolCfg
        regionMerging.OverlapRemovalTool = acc.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags,name="OLRMerge"))
    
    ### disable chi2 cut for overlap removal tool
    regionMerging.OverlapRemovalTool.MinChi2 = 1e15
        
    acc.addEventAlgo(regionMerging)
    return acc
