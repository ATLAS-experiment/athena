#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTruthGuidedProtoTrackCreatorToolCfg(flags,
                                            name: str = "ActsTruthGuidedProtoTrackCreatorTool",
                                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('PRD_MultiTruthCollections', ["PRD_MultiTruthITkPixel","PRD_MultiTruthITkStrip"])
    acc.setPrivateTools(CompFactory.ActsTrk.TruthGuidedProtoTrackCreatorTool(name, **kwargs))
    return acc

def ActsProtoTackCreationAndFitAlgCfg(flags,
                                      name: str = "ActsProtoTrackCreationAndFitAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

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
    kwargs.setdefault("ProtoTracksLocation", "ProtoTracksCollection")
    kwargs.setdefault("copyParametersFromFit", True)
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


    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    flags.Acts.doRotCorrection = False
    
    # IDTPM flags
    from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags
    flags = initializeIDTPMConfigFlags(flags)
    
    flags.PhysVal.IDTPM.plotsDefFileList = "InDetTrackPerfMon/PlotsDefFileList_default.txt" # default value - not needed
    flags.PhysVal.IDTPM.plotsCommonValuesFile = "InDetTrackPerfMon/PlotsDefCommonValues.json" # default value - not needed
    flags.PhysVal.OutputFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.HIST.root' # automatically set in IDTPM config - not needed
    flags.Output.doWriteAOD_IDTPM = True
    flags.PhysVal.IDTPM.trkAnaCfgFile = "InDetTrackPerfMon/EFTrkAnaConfig_example.json"
    flags = initializeIDTPMTrkAnaConfigFlags(flags)
    ## override respective configurations from trkAnaCfgFile
    flags.PhysVal.IDTPM.TrkAnaEF.TrigTrkKey = f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"
    flags.PhysVal.IDTPM.TrkAnaDoubleRatio.TrigTrkKey = f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"
    
    flags.PhysVal.doExample = False

    flags.fillFromArgs()
    if flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']:
        flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"]
        
    flags.Debug.DumpEvtStore = True
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
    
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
    
    ## Associate truth to the xAOD clusters and validate track finding
    from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
    top_acc.merge(ActsTruthAssociationAlgCfg(flags))
    top_acc.merge(ActsTruthParticleHitCountAlgCfg(flags))
    
    from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
    acts_tracks=f"{flags.Tracking.ActiveConfig.extension}Tracks" if not flags.Acts.doAmbiguityResolution else f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
    top_acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                    name=f"{acts_tracks}TrackToTruthAssociationAlg",
                                                    ACTSTracksLocation=ACTSProtoTrackChainTrackKey,
                                                    AssociationMapOut=acts_tracks+"ToTruthParticleAssociation"))

    top_acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                    name=f"{acts_tracks}TrackFindingValidationAlg",
                                                    TrackToTruthAssociationMap=acts_tracks+"ToTruthParticleAssociation"
                                                    ))

    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
    prefix = flags.Tracking.ActiveConfig.extension
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    top_acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, f"{prefix}ResolvedProtoTrackToAltTrackParticleCnvAlg",
                                                       ACTSTracksLocation=[ACTSProtoTrackChainTrackKey,],
                                                       TrackParticlesOutKey=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))
   
    from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
    top_acc.merge(ActsTrackParticleTruthDecorationAlgCfg(flags,
                                                         f"{prefix}ActsSandboxTrackParticleTruthDecorationAlg",
                                                         TrackToTruthAssociationMaps=[acts_tracks+"ToTruthParticleAssociation"],
                                                         TrackParticleContainerName=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))

    # Add the truth decorators
    from InDetPhysValMonitoring.InDetPhysValDecorationConfig import AddDecoratorCfg
    top_acc.merge(AddDecoratorCfg(flags))
    
    # IDTPM running
    from InDetTrackPerfMon.InDetTrackPerfMonConfig import InDetTrackPerfMonCfg
    top_acc.merge( InDetTrackPerfMonCfg(flags) )

    top_acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("ITkTrackRecoWithProtoTracks.pkl", "wb"))
    sc = top_acc.run(flags.Exec.MaxEvents)

    if sc.isFailure():
        import sys
        sys.exit(1)
