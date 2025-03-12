# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from TrigFastTrackFinder.TrigFastTrackFinderConfig import ITkTrigL2LayerNumberToolCfg
  
def ITkTrigTrackSeedingToolStandaloneCfg(flags: AthConfigFlags, **kwargs) -> ComponentAccumulator:

  acc = ComponentAccumulator()

  if "layerNumberTool" not in kwargs:
      ntargs = {"UseNewLayerScheme" : True}
      kwargs.setdefault("layerNumberTool",acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags,**ntargs)))
  
  kwargs.setdefault("DoPhiFiltering", False) #no phi-filtering for full-scan tracking
  kwargs.setdefault("UseBeamTilt", False)
  kwargs.setdefault("PixelSP_ContainerName", "ITkPixelSpacePoints")
  kwargs.setdefault("SCT_SP_ContainerName", "ITkStripSpacePoints")

  isLRT=flags.Tracking.ActiveConfig.extension == "LargeD0"
  
  kwargs.setdefault("UsePixelSpacePoints", (not isLRT))
  kwargs.setdefault("UseSctSpacePoints", isLRT)
  kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT[0])
  kwargs.setdefault("MaxGraphEdges", 2500000 if flags.Trigger.InDetTracking.doGPU else 1500000)
  kwargs.setdefault("ConnectionFileName", "binTables_ITK_RUN4_LRT.txt" if isLRT else "binTables_ITK_RUN4.txt")

  kwargs.setdefault("UseGPU", flags.Trigger.InDetTracking.doGPU)

  if flags.Trigger.InDetTracking.doGPU:
    inDetAccelSvc = CompFactory.TrigInDetAccelerationSvc("TrigInDetAccelerationSvc")
    inDetAccelSvc.useITkGeometry = True # Allows to read and export the ITk geometry
    acc.addService(inDetAccelSvc)
  
  kwargs.setdefault("TrigAccelerationSvc", acc.getService("TrigInDetAccelerationSvc") if flags.Trigger.InDetTracking.doGPU else None)
  
  from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
  
  kwargs.setdefault("RegSelTool_Pixel", acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg( flags) ))

  kwargs.setdefault("RegSelTool_SCT", acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg( flags) ))
  
  acc.setPrivateTools(CompFactory.TrigInDetTrackSeedingTool(**kwargs))

  return acc

def ITkFastTrackFinderStandaloneCfg(flags, SiSPSeededTrackCollectionKey = None):
    acc = ComponentAccumulator()

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
    
    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags))
    
    ITkSiTrackMakerTool.CombinatorialTrackFinder.writeHolesFromPattern = False
    
    if flags.Tracking.ActiveConfig.useTrigTrackFollowing:
        acc.addPublicTool( CompFactory.TrigInDetTrackFollowingTool( name = "TrigTrackFollowingTool_FTF", LayerNumberTool = acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags))))

        ITkSiTrackMakerTool.useTrigTrackFollowingTool = True
        ITkSiTrackMakerTool.TrigTrackFollowingTool = acc.getPublicTool("TrigTrackFollowingTool_FTF")
    
    if flags.Tracking.ActiveConfig.useTrigRoadPredictor:
        acc.addPublicTool( CompFactory.TrigInDetRoadPredictorTool( name = "TrigRoadPredictorTool_FTF", LayerNumberTool = acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags))))

        ITkSiTrackMakerTool.useTrigInDetRoadPredictorTool = True
        ITkSiTrackMakerTool.TrigInDetRoadPredictorTool = acc.getPublicTool("TrigRoadPredictorTool_FTF")
    
    ITkSiTrackMakerTool.trackletPoints = flags.Trigger.InDetTracking.trackletPoints
    
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )
        
    isLRT=flags.Tracking.ActiveConfig.extension == "LargeD0"
    
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderMonitoringArg
    from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig

    acc.merge(TriggerHistSvcConfig(flags))

    monTool = TrigFastTrackFinderMonitoringArg(flags, name = "FullScanLRT" if isLRT else "FullScan", doResMon=False)
    
    ftf = CompFactory.TrigFastTrackFinder(  name = "TrigFastTrackFinder"+flags.Tracking.ActiveConfig.extension,
                                            LayerNumberTool          = acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags)),
                                            TrigAccelerationTool     = None,
                                            TrigAccelerationSvc      = None,
                                            SpacePointProviderTool   = None,
                                            TrackSummaryTool         = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(flags)),
                                            TrackSeedingTool         = acc.popToolsAndMerge(ITkTrigTrackSeedingToolStandaloneCfg(flags)),
                                            initialTrackMaker        = ITkSiTrackMakerTool,
                                            trigInDetTrackFitter     = CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ),
                                            trigZFinder              = CompFactory.TrigZFinder(),
                                            doZFinder                = False,
                                            SeedRadBinWidth          = 10,
                                            TrackInitialD0Max        = 300. if isLRT else 20.0,
                                            TracksName               = SiSPSeededTrackCollectionKey,
                                            TrackZ0Max               = 500. if isLRT else 300.,
                                            Triplet_D0Max            = 300. if isLRT else 4,
                                            Triplet_MaxBufferLength  = 3    if isLRT else 1,
                                            Triplet_MinPtFrac        = 0.8,
                                            UseTrigSeedML            = 1,
                                            doResMon                 = False,
                                            doSeedRedundancyCheck    = True,
                                            pTmin                    = flags.Tracking.ActiveConfig.minPT[0],
                                            useNewLayerNumberScheme  = True,
                                            MinHits                  = 3,
                                            ITkMode                  = True, # Allows ftf to use the new track seeding for ITk
                                            useGPU                   = False,# set to False as the GPU option is now included in TrigTrackSeedingTool
                                            StandaloneMode           = True, # Allows ftf to be run as an offline algorithm with reco_tf
                                            UseTracklets             = flags.Tracking.ActiveConfig.useTracklets,
                                            doTrackRefit             = False,
                                            FreeClustersCut          = 1,
                                            MonTool                  = monTool,
                                            DoubletDR_Max            = 150.0,
                                            LRT_Mode                 = isLRT,
                                            doDisappearingTrk        = False,
                                            dodEdxTrk                = False,
                                            ConnectionFileName       = "binTables_ITK_RUN4_LRT.txt" if isLRT else "binTables_ITK_RUN4.txt")

    acc.addEventAlgo( ftf, primary=True )
    
    return acc


