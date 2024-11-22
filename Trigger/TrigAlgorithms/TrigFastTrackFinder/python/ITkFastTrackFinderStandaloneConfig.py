# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
  kwargs.setdefault("MaxGraphEdges", 1500000)
  kwargs.setdefault("ConnectionFileName", "binTables_ITK_RUN4_LRT.txt" if isLRT else "binTables_ITK_RUN4.txt")

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
    
    if flags.Trigger.InDetTracking.doGPU:
        inDetAccelSvc = CompFactory.TrigInDetAccelerationSvc("TrigInDetAccelerationSvc")
        inDetAccelSvc.useITkGeometry = True # Allows to read and export the ITk geometry
        inDetAccelSvc.MiddleSpacePointLayers = [81000, 82000,
            90011, 90012, 90013, 90014, 91002, 91003, 91004, 91005, 
            92000, 92001, 92002, 92003, 92004, 92005, 92006, 92007, 92008, 92009, 92010,
            92011, 92012, 92013, 92014, 92015, 92016, 92017, 92018, 92019, 92020, 92021, 92022,
            70011, 70012, 70013, 70014, 71002, 71003, 71004, 71005,
            72000, 72001, 72002, 72003, 72004, 72005, 72006, 72007, 72008, 72009, 72010,
            72011, 72012, 72013, 72014, 72015, 72016, 72017, 72018, 72019, 72020, 72021, 72022
        ]
        acc.addService(inDetAccelSvc)
    
    isLRT=flags.Tracking.ActiveConfig.extension == "LargeD0"
    
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderMonitoringArg
    from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig

    acc.merge(TriggerHistSvcConfig(flags))

    monTool = TrigFastTrackFinderMonitoringArg(flags, name = "FullScanLRT" if isLRT else "FullScan", doResMon=False)
    
    ftf = CompFactory.TrigFastTrackFinder(  name = "TrigFastTrackFinder"+flags.Tracking.ActiveConfig.extension,
                                            LayerNumberTool          = acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags)),
                                            TrigAccelerationTool     = CompFactory.TrigITkAccelerationTool(name = "TrigITkAccelerationTool_FTF") if flags.Trigger.InDetTracking.doGPU else None,
                                            TrigAccelerationSvc      = acc.getService("TrigInDetAccelerationSvc") if flags.Trigger.InDetTracking.doGPU else None,
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
                                            useGPU                   = flags.Trigger.InDetTracking.doGPU,
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


