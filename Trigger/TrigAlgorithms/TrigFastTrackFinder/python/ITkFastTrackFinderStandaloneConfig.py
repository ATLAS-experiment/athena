# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkFastTrackFinderStandaloneCfg(flags, SiSPSeededTrackCollectionKey = None):
    acc = ComponentAccumulator()

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolNoHoleSearchCfg
    ITkTrackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolNoHoleSearchCfg(flags))
    acc.addPublicTool(ITkTrackSummaryTool)

    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags))
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )

    from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
    pixRegSelTool = acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg(flags) )
    sctRegSelTool = acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg(flags) )
    
    if flags.Trigger.InDetTracking.doGPU:
        inDetAccelSvc = CompFactory.TrigInDetAccelerationSvc("TrigInDetAccelerationSvc")
        inDetAccelSvc.useITkGeometry = True # Allows to read and export the ITk geometry
        inDetAccelSvc.MiddleSpacePointLayers = [81000, 82000,
            91005, 91004, 90014, 90013, 92000, 92001, 92002, 92003, 92004, 92005, 92006, 92007, 92008, 92009, 92010,
            92011, 92012, 92013, 92014, 92015, 92016, 92017, 92018, 92019, 92020, 92021, 92022,
            71005, 71004, 70014, 70013, 72000, 72001, 72002, 72003, 72004, 72005, 72006, 72007, 72008, 72009, 72010,
            72011, 72012, 72013, 72014, 72015, 72016, 72017, 72018, 72019, 72020, 72021, 72022
        ]
        acc.addService(inDetAccelSvc)

        acc.addPublicTool(CompFactory.TrigITkAccelerationTool(name = "TrigITkAccelerationTool_FTF"))

    acc.addPublicTool( CompFactory.TrigL2LayerNumberToolITk( name = "TrigL2LayerNumberTool_FTF",UseNewLayerScheme = True) )

    acc.addPublicTool( CompFactory.TrigSpacePointConversionTool( "TrigSpacePointConversionTool",
                                                                    DoPhiFiltering    = True,
                                                                    UseBeamTilt       = False,
                                                                    UseNewLayerScheme = True,
                                                                    RegSelTool_Pixel  = pixRegSelTool,
                                                                    RegSelTool_SCT    = sctRegSelTool,
                                                                    PixelSP_ContainerName = "ITkPixelSpacePoints",
                                                                    UseSctSpacePoints = False,
                                                                    layerNumberTool   = acc.getPublicTool("TrigL2LayerNumberTool_FTF") ) )

    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderMonitoringArg
    from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
    acc.merge(TriggerHistSvcConfig(flags))
    monTool = TrigFastTrackFinderMonitoringArg(flags, name = "FullScan", doResMon=False)
    
    ftf = CompFactory.TrigFastTrackFinder( name = "TrigFastTrackFinder_",
                                            LayerNumberTool          = acc.getPublicTool( "TrigL2LayerNumberTool_FTF" ),
                                            TrigAccelerationTool     = acc.getPublicTool( "TrigITkAccelerationTool_FTF" ) if flags.Trigger.InDetTracking.doGPU else None,
                                            TrigAccelerationSvc      = acc.getService("TrigInDetAccelerationSvc") if flags.Trigger.InDetTracking.doGPU else None,
                                            SpacePointProviderTool   = acc.getPublicTool( "TrigSpacePointConversionTool"),
                                            TrackSummaryTool         = ITkTrackSummaryTool,
                                            initialTrackMaker        = ITkSiTrackMakerTool,
                                            trigInDetTrackFitter     = acc.getPublicTool( "TrigInDetTrackFitter" ),
                                            trigZFinder              = CompFactory.TrigZFinder(),
                                            doZFinder                = False,
                                            SeedRadBinWidth          = 10,
                                            TrackInitialD0Max        = 20.0,
                                            TracksName               = SiSPSeededTrackCollectionKey,
                                            Triplet_D0Max            = 4,
                                            Triplet_MaxBufferLength  = 1,
                                            Triplet_MinPtFrac        = 0.8,
                                            UseTrigSeedML            = 1,
                                            doResMon                 = False,
                                            doSeedRedundancyCheck    = True,
                                            pTmin                    = flags.Tracking.ActiveConfig.minPT[0],
                                            useNewLayerNumberScheme  = True,
                                            MinHits                  = 3,
                                            ITkMode                  = True, # Allows ftf to use the new TrigTrackSeedGenerator for ITk
                                            useGPU                   = flags.Trigger.InDetTracking.doGPU,
                                            StandaloneMode           = True, # Allows ftf to be run as an offline algorithm with reco_tf
                                            doTrackRefit             = False,
                                            FreeClustersCut          = 1,
                                            MonTool                  = monTool,
                                            DoubletDR_Max            = 150.0)

    acc.addEventAlgo( ftf, primary=True )
    
    return acc


