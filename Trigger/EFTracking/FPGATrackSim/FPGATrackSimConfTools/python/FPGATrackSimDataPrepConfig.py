# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def getBaseName(flags):
    if (not (flags.Trigger.FPGATrackSim.baseName == '')):
        return flags.Trigger.FPGATrackSim.baseName
    elif (flags.Trigger.FPGATrackSim.region == 0):
        return 'eta0103phi0305'
    elif (flags.Trigger.FPGATrackSim.region == 1):
        return 'eta0709phi0305'
    elif (flags.Trigger.FPGATrackSim.region == 2):
        return 'eta1214phi0305'
    elif (flags.Trigger.FPGATrackSim.region == 3):
        return 'eta2022phi0305'
    elif (flags.Trigger.FPGATrackSim.region == 4):
        return 'eta3234phi0305'
    elif (flags.Trigger.FPGATrackSim.region == 5):
        return 'eta0103phi1113'
    elif (flags.Trigger.FPGATrackSim.region == 6):
        return 'eta0103phi1921'
    elif (flags.Trigger.FPGATrackSim.region == 7):
        return 'eta0103phi3436'
    else:
        return 'default'

def FPGATrackSimRawLogicCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimRawLogic = CompFactory.FPGATrackSimRawToLogicalHitsTool()
    FPGATrackSimRawLogic.SaveOptional = 2
    if (flags.Trigger.FPGATrackSim.ActiveConfig.sampleType == 'skipTruth'):
        FPGATrackSimRawLogic.SaveOptional = 1
    FPGATrackSimRawLogic.TowersToMap = [0] # TODO TODO why is this hardcoded?
    FPGATrackSimRawLogic.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
    FPGATrackSimRawLogic.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    result.addPublicTool(FPGATrackSimRawLogic, primary=True)
    return result

def FPGATrackSimSpacePointsToolCfg(flags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.FPGATrackSimSpacePointsTool()
    SpacePointTool.Filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
    SpacePointTool.FilteringClosePoints = False
    SpacePointTool.PhiWindow = 0.004
    SpacePointTool.Duplication = True
    result.addPublicTool(SpacePointTool, primary=True)
    return result


def prepareFlagsForFPGATrackSimDataPrepAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags


def FPGATrackSimDataPrepOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutputDataPrep")
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    FPGATrackSimWriteOutput.OutputTreeName = "FPGATrackSimDataPrepTree"
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.addPublicTool(FPGATrackSimWriteOutput, primary=True)
    return result

def FPGAConversionAlgCfg(inputFlags, name = 'FPGAConversionAlg', stage = '', **kwargs):

    flags = prepareFlagsForFPGATrackSimDataPrepAlg(inputFlags)
   
    result=ComponentAccumulator()
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault("FPGATrackSimClusterKey", "FPGAClusters%s" %(stage))
    kwargs.setdefault("FPGATrackSimHitKey", "FPGAHits%s" %(stage))
    kwargs.setdefault("FPGATrackSimHitInRoadsKey", "FPGAHitsInRoads%s" %(stage))
    kwargs.setdefault("FPGATrackSimRoadKey", "FPGARoads%s" %(stage))
    kwargs.setdefault("FPGATrackSimTrackKey", "FPGATracks%s" %(stage))
    kwargs.setdefault("xAODPixelClusterFromFPGAClusterKey", "xAODPixelClusters%sFromFPGACluster" %(stage))
    kwargs.setdefault("xAODStripClusterFromFPGAClusterKey", "xAODStripClusters%sFromFPGACluster" %(stage))
    kwargs.setdefault("xAODStripSpacePointFromFPGAKey", "xAODStripSpacePoints%sFromFPGA" %(stage))
    kwargs.setdefault("xAODPixelSpacePointFromFPGAKey", "xAODPixelSpacePoints%sFromFPGA" %(stage))
    kwargs.setdefault("xAODPixelClusterFromFPGAHitKey", "xAODPixelClusters%sFromFPGAHit" %(stage))
    kwargs.setdefault("xAODStripClusterFromFPGAHitKey", "xAODStripClusters%sFromFPGAHit" %(stage))
    kwargs.setdefault("ActsProtoTrackFromFPGARoadKey", "ActsProtoTracks%sFromFPGARoad" %(stage))
    kwargs.setdefault("ActsProtoTrackFromFPGATrackKey", "ActsProtoTracks%sFromFPGATrack" %(stage))
    kwargs.setdefault("doHits", True)
    kwargs.setdefault("doClusters", True)
    kwargs.setdefault("doActsTrk", False)
    kwargs.setdefault("ClusterConverter", result.popToolsAndMerge(FPGAClusterConverterCfg(flags)))
    kwargs.setdefault("ActsTrkConverter", result.popToolsAndMerge(FPGAActsTrkConverterCfg(flags)))
    
    result.addEventAlgo(CompFactory.FPGAConversionAlgorithm(name, **kwargs))

    return result

def FPGAClusterConverterCfg(flags):
    result=ComponentAccumulator()
    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    FPGAClusterConverter = CompFactory.FPGAClusterConverter(LorentzAngleTool=result.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))
    result.setPrivateTools(FPGAClusterConverter)

    return result

def FPGAActsTrkConverterCfg(flags):
    result=ComponentAccumulator()
    FPGAActsTrkConverter = CompFactory.FPGAActsTrkConverter()
    result.setPrivateTools(FPGAActsTrkConverter)

    return result


def WriteToAOD(flags, stage = '',finalTrackParticles = ''): #  store xAOD containers in AOD file
    result = ComponentAccumulator()
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
    from AthenaConfiguration.Enums import MetadataCategory
    
    result.merge( SetupMetaDataForStreamCfg( flags,"AOD", 
                                            createMetadata=[
                                                MetadataCategory.ByteStreamMetaData,
                                                MetadataCategory.LumiBlockMetaData,
                                                MetadataCategory.TruthMetaData,
                                                MetadataCategory.IOVMetaData,],)
                )
    log.info("AOD ItemList: %s", result.getEventAlgo(outputStreamName("AOD")).ItemList)
    log.info("AOD MetadataItemList: %s", result.getEventAlgo(outputStreamName("AOD")).MetadataItemList)
    log.info("---------- Configured AOD writing")
    
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    toAOD = []
    toAOD += [f"xAOD::PixelClusterContainer#xAODPixelClusters{stage}FromFPGACluster",f"xAOD::PixelClusterAuxContainer#xAODPixelClusters{stage}FromFPGAClusterAux.",
              f"xAOD::StripClusterContainer#xAODStripClusters{stage}FromFPGACluster",f"xAOD::StripClusterAuxContainer#xAODStripClusters{stage}FromFPGAClusterAux.",
              f"xAOD::TrackParticleContainer#{finalTrackParticles}",f"xAOD::TrackParticleAuxContainer#{finalTrackParticles}Aux.",
              f"xAOD::SpacePointContainer#xAODPixelSpacePoints{stage}FromFPGA",f"xAOD::SpacePointAuxContainer#xAODPixelSpacePoints{stage}FromFPGAAux.-measurements",
              f"xAOD::SpacePointContainer#xAODStripSpacePoints{stage}FromFPGA",f"xAOD::SpacePointAuxContainer#xAODStripSpacePoints{stage}FromFPGAAux.-measurements.-sctSpacePointLink",
            ]
    
    result.merge(addToAOD(flags, toAOD))

    return result


def FPGATrackSimEventSelectionCfg(flags):
    result=ComponentAccumulator()
    eventSelector = CompFactory.FPGATrackSimEventSelectionSvc()
    eventSelector.regions = flags.Trigger.FPGATrackSim.slicesFile
    eventSelector.regionID = flags.Trigger.FPGATrackSim.region
    eventSelector.sampleType = flags.Trigger.FPGATrackSim.sampleType
    eventSelector.skipRegionCheck = flags.Trigger.FPGATrackSim.pipeline.startswith('F-1') # if set to True, it will essentially run for the whole detector
    eventSelector.withPU = False
    result.addService(eventSelector, create=True, primary=True)
    return result

def FPGATrackSimMappingCfg(flags):
    result=ComponentAccumulator()

    mappingSvc = CompFactory.FPGATrackSimMappingSvc()
    mappingSvc.mappingType = "FILE"
    mappingSvc.rmap = flags.Trigger.FPGATrackSim.mapsDir+"/"+getBaseName(flags)+".rmap" # we need more configurability here i.e. file choice should depend on some flag
    mappingSvc.subrmap =  flags.Trigger.FPGATrackSim.mapsDir+"/"+getBaseName(flags)+".subrmap" # presumably also here we want to be able to change the slices definition file
    mappingSvc.pmap = flags.Trigger.FPGATrackSim.mapsDir+"/"+getBaseName(flags)+".pmap"
    mappingSvc.modulemap = flags.Trigger.FPGATrackSim.mapsDir+"/moduleidmap"
    mappingSvc.radiiFile = flags.Trigger.FPGATrackSim.mapsDir + "/"+getBaseName(flags)+"_radii.txt"
    mappingSvc.FakeNNonnx = flags.Trigger.FPGATrackSim.FakeNNonnxFile
    mappingSvc.ParamNNonnx = flags.Trigger.FPGATrackSim.ParamNNonnxFile
    mappingSvc.layerOverride = []
    result.addService(mappingSvc, create=True, primary=True)
    return result


def FPGATrackSimReadInputCfg(flags):
    result=ComponentAccumulator()
    InputTool = CompFactory.FPGATrackSimInputHeaderTool(name="FPGATrackSimReadInput",
                                               InFileName = flags.Trigger.FPGATrackSim.wrapperFileName)
    result.addPublicTool(InputTool, primary=True)
    return result

def FPGATrackSimReadInput2Cfg(flags):
    result=ComponentAccumulator()
    InputTool2 = CompFactory.FPGATrackSimReadRawRandomHitsTool(name="FPGATrackSimReadInput2", InFileName = flags.Trigger.FPGATrackSim.wrapperFileName2)
    result.addPublicTool(InputTool2, primary=True)
    return result

def FPGATrackSimHitFilteringToolCfg(flags):
    result=ComponentAccumulator()
    HitFilteringTool = CompFactory.FPGATrackSimHitFilteringTool()
    HitFilteringTool.barrelStubDphiCut = 3.0
    HitFilteringTool.doRandomRemoval = False
    HitFilteringTool.doStubs = False
    HitFilteringTool.endcapStubDphiCut = 1.5
    HitFilteringTool.pixelClusRmFrac = 0
    HitFilteringTool.pixelHitRmFrac = 0
    HitFilteringTool.stripClusRmFrac = 0
    HitFilteringTool.stripHitRmFrac = 0
    HitFilteringTool.useNstrips = False
    result.addPublicTool(HitFilteringTool, primary=True)
    return result



def FPGATrackSimDataPrepAlgCfg(inputFlags):

    flags = prepareFlagsForFPGATrackSimDataPrepAlg(inputFlags)

    result=ComponentAccumulator()

    theFPGATrackSimDataPrepAlg=CompFactory.FPGATrackSimDataPrepAlg()
    theFPGATrackSimDataPrepAlg.HitFiltering = flags.Trigger.FPGATrackSim.ActiveConfig.hitFiltering
    theFPGATrackSimDataPrepAlg.writeOutputData = flags.Trigger.FPGATrackSim.ActiveConfig.writeOutputData
    theFPGATrackSimDataPrepAlg.Clustering = flags.Trigger.FPGATrackSim.clustering
    theFPGATrackSimDataPrepAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
    theFPGATrackSimDataPrepAlg.runOnRDO = not flags.Trigger.FPGATrackSim.wrapperFileName
    
    FPGATrackSimMaping = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    theFPGATrackSimDataPrepAlg.FPGATrackSimMapping = FPGATrackSimMaping

    theFPGATrackSimDataPrepAlg.RawToLogicalHitsTool = result.getPrimaryAndMerge(FPGATrackSimRawLogicCfg(flags))

    if flags.Trigger.FPGATrackSim.wrapperFileName and flags.Trigger.FPGATrackSim.wrapperFileName is not None:
        theFPGATrackSimDataPrepAlg.InputTool = result.getPrimaryAndMerge(FPGATrackSimReadInputCfg(flags))
        if flags.Trigger.FPGATrackSim.wrapperFileName2 and flags.Trigger.FPGATrackSim.wrapperFileName2 is not None:
            theFPGATrackSimDataPrepAlg.InputTool2 = result.getPrimaryAndMerge(FPGATrackSimReadInput2Cfg(flags))
            theFPGATrackSimDataPrepAlg.SecondInputToolN = flags.Trigger.FPGATrackSim.secondInputToolN
        theFPGATrackSimDataPrepAlg.SGInputTool = ""
    else:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
        theFPGATrackSimDataPrepAlg.InputTool = ""
        theFPGATrackSimDataPrepAlg.InputTool2 = ""
        from FPGATrackSimSGInput.FPGATrackSimSGInputConfig import FPGATrackSimSGInputToolCfg
        theFPGATrackSimDataPrepAlg.SGInputTool = result.getPrimaryAndMerge(FPGATrackSimSGInputToolCfg(flags))

    theFPGATrackSimDataPrepAlg.SpacePointTool = result.getPrimaryAndMerge(FPGATrackSimSpacePointsToolCfg(flags))

    theFPGATrackSimDataPrepAlg.HitFilteringTool = result.getPrimaryAndMerge(FPGATrackSimHitFilteringToolCfg(flags))

    theFPGATrackSimDataPrepAlg.ClusteringTool = CompFactory.FPGATrackSimClusteringTool()
    theFPGATrackSimDataPrepAlg.OutputTool = result.getPrimaryAndMerge(FPGATrackSimDataPrepOutputCfg(flags))

    # Create SPRoadFilterTool if spacepoints are turned on. TODO: make things configurable?
    if flags.Trigger.FPGATrackSim.spacePoints:
        theFPGATrackSimDataPrepAlg.Spacepoints = True

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimLogicalHitsProcessAlgMonitoringCfg
    theFPGATrackSimDataPrepAlg.MonTool = result.getPrimaryAndMerge(FPGATrackSimLogicalHitsProcessAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimDataPrepAlg)

    return result


log = AthenaLogger(__name__)

def FPGATrackSimDataPrepConnectToFastTracking(flags,FinalTracks="FPGADataPrep"):
    result = ComponentAccumulator()
    ACTSTracks="FPGADataPrepActsTracks"
    
    # ACTS Seeding
    from ActsConfig.ActsSeedingConfig import ActsStripSeedingAlgCfg, ActsPixelSeedingAlgCfg
    result.merge(ActsStripSeedingAlgCfg(flags, name="FPGADataPrepActsStripSeedingAlg",
                                    InputSpacePoints=['xAODStripSpacePoints_1stFromFPGA'],
                                    OutputSeeds="FPGADataPrepActsStripSeeds"))
    
    result.merge(ActsPixelSeedingAlgCfg(flags, name="FPGADataPrepActsPixelSeedingAlg",
                                    InputSpacePoints=['xAODPixelSpacePoints_1stFromFPGA'],
                                    OutputSeeds="FPGADataPrepActsPixelSeeds"))
    
    # ACTS Tracking
    from ActsConfig.ActsTrackFindingConfig import ActsMainTrackFindingAlgCfg
    result.merge(ActsMainTrackFindingAlgCfg(flags, name="FPGADataPrepActsTrackFindingAlg",
                                SeedContainerKeys=['FPGADataPrepActsPixelSeeds','FPGADataPrepActsStripSeeds'],
                                UncalibratedMeasurementContainerKeys=["xAODPixelClusters_1stFromFPGACluster","xAODStripClusters_1stFromFPGACluster"],
                                ACTSTracksLocation=ACTSTracks))
    
    # Track to Truth association and validation
    from ActsConfig.ActsTruthConfig import ActsTruthParticleHitCountAlgCfg, ActsPixelClusterToTruthAssociationAlgCfg,ActsStripClusterToTruthAssociationAlgCfg
    result.merge(ActsPixelClusterToTruthAssociationAlgCfg(flags,
                                                       name="FPGADataPrepActsPixelClusterToTruthAssociationAlg",
                                                       InputTruthParticleLinks="xAODFPGATruthLinks",
                                                       AssociationMapOut="ITkFPGAPixelClustersToTruthParticles",
                                                       Measurements="xAODPixelClusters_1stFromFPGACluster")) 
    
    result.merge(ActsStripClusterToTruthAssociationAlgCfg(flags,
                                                       name="FPGADataPrepActsStripClusterToTruthAssociationAlg",
                                                       InputTruthParticleLinks="xAODFPGATruthLinks",
                                                       AssociationMapOut="ITkFPGAStripClustersToTruthParticles",
                                                       Measurements="xAODStripClusters_1stFromFPGACluster"))
    
    result.merge(ActsTruthParticleHitCountAlgCfg(flags,
                                              name="FPGADataPrepActsTruthParticleHitCountAlg",
                                              PixelClustersToTruthAssociationMap="ITkFPGAPixelClustersToTruthParticles",
                                              StripClustersToTruthAssociationMap="ITkFPGAStripClustersToTruthParticles",
                                              TruthParticleHitCountsOut="FPGATruthParticleHitCounts"))
    
    from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
    acts_tracks=f"{flags.Tracking.ActiveConfig.extension}Tracks" if not flags.Acts.doAmbiguityResolution else f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
    result.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                name=f"{acts_tracks}FPGADataPrepTrackToTruthAssociationAlg",
                                                PixelClustersToTruthAssociationMap="ITkFPGAPixelClustersToTruthParticles",
                                                StripClustersToTruthAssociationMap="ITkFPGAStripClustersToTruthParticles",
                                                ACTSTracksLocation=ACTSTracks,
                                                AssociationMapOut=acts_tracks+"FPGAToTruthParticleAssociation"))
    
    
    result.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                name=f"{acts_tracks}FPGADataPrepTrackFindingValidationAlg",
                                                TrackToTruthAssociationMap=acts_tracks+"FPGAToTruthParticleAssociation",
                                                TruthParticleHitCounts="FPGATruthParticleHitCounts"
                                                ))
    
    ################################################################################
    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
    prefix = flags.Tracking.ActiveConfig.extension
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    result.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, name=f"{prefix}FPGADataPrepActsTrackToTrackParticleCnvAlg",
                                                ACTSTracksLocation=[ACTSTracks,],
                                                TrackParticlesOutKey=f"{FinalTracks}TrackParticles"))
   
    from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
    result.merge(ActsTrackParticleTruthDecorationAlgCfg(flags, name=f"{prefix}FPGADataPrepActsTrackParticleTruthDecorationAlg",
                                                    TrackToTruthAssociationMaps=[acts_tracks+"FPGAToTruthParticleAssociation"],
                                                    TrackParticleContainerName=f"{FinalTracks}TrackParticles",
                                                    TruthParticleHitCounts="FPGATruthParticleHitCounts",
                                                    ComputeTrackRecoEfficiency=True))
    
    return result


def runDataPrepChain():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg


    FinalDataPrepTrackChainxAODTracksKeyPrefix="FPGADataPrep"
    
    flags = initConfigFlags()
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    
    ############################################
    # Flags used in the prototrack chain
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    flags.Acts.doRotCorrection = False

    ###########################################
    # ITk Acts flags    
    flags.Detector.EnableITkPixel=True
    flags.Detector.EnableITkStrip=True
    
    flags.Tracking.ITkMainPass.doActsSeed=True
    flags.Tracking.ITkMainPass.doActsTrack = False # when set to True it seems to be causing issues related to TrackToTruthAssociation. To be investigated...
    flags.Tracking.doITkFastTracking=False # turn to True to enable Fast Tracking chain
    
    ###########################################
    # IDTPM flags
    from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags
    flags = initializeIDTPMConfigFlags(flags)
    
    flags.PhysVal.IDTPM.outputFilePrefix = "myIDTPM_CA"
    flags.PhysVal.IDTPM.plotsDefFileList = "InDetTrackPerfMon/PlotsDefFileList_default.txt" # default value - not needed
    flags.PhysVal.IDTPM.plotsCommonValuesFile = "InDetTrackPerfMon/PlotsDefCommonValues.json" # default value - not needed
    flags.PhysVal.OutputFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.HIST.root' # automatically set in IDTPM config - not needed
    flags.Output.doWriteAOD_IDTPM = True
    flags.Output.AOD_IDTPMFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.AOD_IDTPM.pool.root' # automatically set in IDTPM config - not needed
    flags.PhysVal.IDTPM.trkAnaCfgFile = "InDetTrackPerfMon/EFTrkAnaConfig_example.json"
    
    flags = initializeIDTPMTrkAnaConfigFlags(flags)
    ## override respective configurations from trkAnaCfgFile (in case something changes in the config file)
    flags.PhysVal.IDTPM.TrkAnaEF.TrigTrkKey = f"{FinalDataPrepTrackChainxAODTracksKeyPrefix}TrackParticles"
    flags.PhysVal.IDTPM.TrkAnaDoubleRatio.TrigTrkKey = f"{FinalDataPrepTrackChainxAODTracksKeyPrefix}TrackParticles"

    flags.PhysVal.doExample = False
    ############################################
    flags.Concurrency.NumThreads=1
    flags.Scheduler.ShowDataDeps=True
    flags.Debug.DumpEvtStore=False # Set to Truth to enable Event Store printouts
    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()
    if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
        log.info("wrapperFile is string, converting to list")
        flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
        flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]
    
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.MainPass", keepOriginal=True)
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)
    flags.dump()
    
    acc=MainServicesCfg(flags)
    acc.addService(CompFactory.THistSvc(Output = [f"EXPERT DATAFILE='{flags.Trigger.FPGATrackSim.outputMonitorFile}', OPT='RECREATE'"]))
    acc.addService(CompFactory.THistSvc(Output = ["FPGATRACKSIMOUTPUT DATAFILE='dataprep.root', OPT='RECREATE'"]))


    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    
        if flags.Input.isMC:
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            acc.merge(GEN_AOD2xAODCfg(flags))

            from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
            acc.merge(addTruthPileupJetsToOutputCfg(flags))
        
        if flags.Detector.EnableCalo:
            from CaloRec.CaloRecoConfig import CaloRecoCfg
            acc.merge(CaloRecoCfg(flags))

        if not flags.Reco.EnableTrackOverlay:
            from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
            acc.merge(InDetTrackRecoCfg(flags))
        
        if flags.Trigger.FPGATrackSim.connectToToITkTracking: # no point in running this if the seeding/tracking for FPGA SPs is off
            from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
            acc.merge(ITkTrackRecoCfg(flags))

    # Use the imported configuration function for the data prep algorithm.
    acc.merge(FPGATrackSimDataPrepAlgCfg(flags))

    if flags.Trigger.FPGATrackSim.doEDMConversion:
        acc.merge(FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlg_1st', stage = '_1st', doActsTrk=False, doSP = flags.Trigger.FPGATrackSim.spacePoints))
        if flags.Trigger.FPGATrackSim.convertUnmappedHits: acc.merge(FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlgUnmapped_1st', stage = 'Unmapped_1st', doClusters = False))
        
        if flags.Trigger.FPGATrackSim.connectToToITkTracking:
            acc.merge(FPGATrackSimDataPrepConnectToFastTracking(flags, FinalTracks=FinalDataPrepTrackChainxAODTracksKeyPrefix))
        
        if flags.Trigger.FPGATrackSim.writeToAOD:
            acc.merge(WriteToAOD(flags, stage = '_1st',))
            if flags.Trigger.FPGATrackSim.spacePoints : acc.merge(WriteToAOD(flags,
                                                                             stage = '_1st',
                                                                             finalTrackParticles=f"{FinalDataPrepTrackChainxAODTracksKeyPrefix}TrackParticles"))
            
        # Printout for various FPGA-related objects
        from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
        acc.merge(FPGATrackSimReportingCfg(flags,
                                           perEventReports = (flags.Trigger.FPGATrackSim.sampleType != 'skipTruth'),
                                           isDataPrep=True))
        
        # IDTPM running
        from InDetTrackPerfMon.InDetTrackPerfMonConfig import InDetTrackPerfMonCfg
        acc.merge( InDetTrackPerfMonCfg(flags) )
    
    acc.store(open('AnalysisConfig.pkl','wb'))

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess() is True, "Application execution did not succeed"


if __name__ == "__main__":
    runDataPrepChain()
