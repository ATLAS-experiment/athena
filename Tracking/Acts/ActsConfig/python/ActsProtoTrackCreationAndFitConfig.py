#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTruthGuidedProtoTrackCreatorToolCfg(flags,
                                            name: str = "ActsTruthGuidedProtoTrackCreatorTool",
                                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('PRD_MultiTruthCollections', ["PRD_MultiTruthITkPixel","PRD_MultiTruthITkStrip"])
    acc.setPrivateTools(CompFactory.ActsTrk.TruthGuidedProtoTrackCreator(name, **kwargs))
    return acc

def ActsProtoTackCreationAndFitAlgCfg(flags,
                                      name: str = "ActsProtoTrackCreationAndFitAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator() 
    from ActsConfig.ActsTrackFindingConfig import isdet  
    kwargs.setdefault("DetectorElementCollectionKeys", isdet(flags, ["ITkPixelDetectorElementCollection"], ["ITkStripDetectorElementCollection"]))

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault('PixelClusterContainer', 'ITkPixelClusters')
    kwargs.setdefault('StripClusterContainer', 'ITkStripClusters')
    kwargs.setdefault('ACTSTracksLocation', 'EFTestTracks')
    
    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )  # PrivateToolHandle
        
    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            "ExtrapolationTool",
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )  # PrivateToolHandle

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault(
            "ATLASConverterTool",
            acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
        )

    if 'ActsFitter' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
        kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(ActsFitterCfg(flags,
                                                                           ReverseFilteringPt=0,
                                                                           OutlierChi2Cut=30)))

    if 'PatternBuilder' not in kwargs:
        kwargs.setdefault('PatternBuilder', acc.popToolsAndMerge(ActsTruthGuidedProtoTrackCreatorToolCfg(flags)))
        
    acc.addEventAlgo(CompFactory.ActsTrk.ProtoTrackCreationAndFitAlg(name,**kwargs),
                     primary=True)
    return acc

def ActsProtoTrackReportingAlgCfg(flags,
                                  name: str = "ActsProtoTrackReportingAlg",
                                  **kwargs) -> ComponentAccumulator: 
    acc = ComponentAccumulator() 
    acc.addEventAlgo(CompFactory.ActsTrk.ProtoTrackReportingAlg(name,**kwargs),
                     primary=True)
    return acc

if __name__ == "__main__":
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
            
    def SetupHistSvc(flags, streamName, dataFile):
        acc = ComponentAccumulator()
        histSvc = CompFactory.THistSvc(Output= ["{streamName} DATAFILE='{data_file}', OPT='RECREATE'".format(streamName=streamName, data_file = dataFile )])
        acc.addService(histSvc, primary=True)
        return acc
    
    # Key names for the different track containers
    ACTSProtoTrackChainTrackKey = "ACTSProtoTrackChainTestTracks"
    FinalProtoTrackChainTracksKey="TrkProtoTrackChainTestTracks"
    FinalProtoTrackChainxAODTracksKey="xAODProtoTrackChainTestTracks"

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # this job specific flags
    flags.addFlag("outputNTupleFile", "refits.root")

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"]

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    flags.Acts.doRotCorrection = False
    # IDPVM flags
    flags.PhysVal.IDPVM.doExpertOutput   = True
    flags.PhysVal.IDPVM.doPhysValOutput  = False
    flags.PhysVal.IDPVM.doHitLevelPlots = True
    flags.PhysVal.IDPVM.runDecoration = True
    flags.PhysVal.IDPVM.validateExtraTrackCollections = [f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"]
    flags.PhysVal.IDPVM.doTechnicalEfficiency = True
    flags.PhysVal.OutputFileName = "IDPVM.root"
    flags.fillFromArgs()
    flags.lock()

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    #Truth
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    # Standard reco
    top_acc.merge(ITkTrackRecoCfg(flags))

    # ProtoTrackChain Track algo
    top_acc.merge(SetupHistSvc(flags,streamName="HmmRefits",dataFile=flags.outputNTupleFile))
    top_acc.merge(ActsProtoTackCreationAndFitAlgCfg(flags,"ActsProtoTackCreationAndFitAlg",ACTSTracksLocation=ACTSProtoTrackChainTrackKey   ))

    ## Convert ACTs container to Trk converter
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
    top_acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               ACTSTracksLocation=ACTSProtoTrackChainTrackKey,
                                               TracksLocation=FinalProtoTrackChainTracksKey))
    
    # Add truth to the container
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    top_acc.merge(ITkTrackTruthCfg(
        flags,
        Tracks=FinalProtoTrackChainTracksKey,
        DetailedTruth=FinalProtoTrackChainxAODTracksKey+"DetailedTruth",
        TracksTruth=FinalProtoTrackChainxAODTracksKey+"TruthCollection"))

    # Trk to xAOD converter
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    top_acc.merge(ITkTrackParticleCnvAlgCfg(flags,
           name = f"{FinalProtoTrackChainTracksKey}TrackParticleCnvAlg",
           TrackContainerName = FinalProtoTrackChainTracksKey,
           xAODTrackParticlesFromTracksContainerName = f"{FinalProtoTrackChainxAODTracksKey}TrackParticles",
           TrackTruthContainerName = f"{FinalProtoTrackChainxAODTracksKey}TruthCollection")) 

    # Printout for ProtoTrackChain Tracking
    top_acc.merge(ActsProtoTrackReportingAlgCfg(flags, TrackCollection=FinalProtoTrackChainTracksKey, xAODTrackCollection=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))

    # Add the truth decorators
    from InDetPhysValMonitoring.InDetPhysValDecorationConfig import AddDecoratorCfg
    top_acc.merge(AddDecoratorCfg(flags))

    # IDPVM running
    from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetPhysValMonitoringCfg
    top_acc.merge(InDetPhysValMonitoringCfg(flags))

    top_acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("ITkTrackRecoWithProtoTracks.pkl", "wb"))
    sc = top_acc.run(1)
