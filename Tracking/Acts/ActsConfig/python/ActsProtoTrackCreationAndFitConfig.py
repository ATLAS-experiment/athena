# 
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsProtoTackCreationAndFitCfg(flags, name="ActsProtoTrackCreationAndFitAlg", **kwargs):
    result = ComponentAccumulator() 
    from ActsConfig.ActsTrackFindingConfig import isdet  
    kwargs.setdefault("DetectorElementCollectionKeys", isdet(flags, ["ITkPixelDetectorElementCollection"], ["ITkStripDetectorElementCollection"]))
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg, ActsTrackingGeometryToolCfg 
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    
    result.merge(ITkPixelReadoutGeometryCfg(flags))
    result.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault(
        "TrackingGeometryTool",
        result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
    )  # PrivateToolHandle

    kwargs.setdefault(
        "ExtrapolationTool",
        result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    )  # PrivateToolHandle

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault(
        "ATLASConverterTool",
        result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    kwargs.setdefault("ActsFitter", result.popToolsAndMerge(ActsFitterCfg(flags,
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=30)))
    theAlg = CompFactory.ActsTrk.ProtoTrackCreationAndFitAlg(name,**kwargs)
    result.addEventAlgo(theAlg,primary=True)
    return result 

def ActsProtoTrackReportingAlgCfg(flags, name="ActsProtoTrackReportingAlg",**kwargs): 
    result = ComponentAccumulator() 
    theAlg = CompFactory.ActsTrk.ProtoTrackReportingAlg(name,**kwargs)
    result.addEventAlgo(theAlg,primary=True)
    return result 

if __name__ == "__main__":
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
            
    def SetupHistSvc(flags, streamName, dataFile):
        result = ComponentAccumulator()
        histSvc = CompFactory.THistSvc(Output= ["{streamName} DATAFILE='{data_file}', OPT='RECREATE'".format(streamName=streamName, data_file = dataFile )])
        result.addService(histSvc, primary=True)
        return result
    
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
    top_acc.merge(ActsProtoTackCreationAndFitCfg(flags,"ActsProtoTackCreationAndFitAlg",ACTSTracksLocation=ACTSProtoTrackChainTrackKey   ))

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
