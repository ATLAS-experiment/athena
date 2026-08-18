#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

### Output data name ###
from TrigEDMConfig.TriggerEDM import recordable
from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg, NrpcBytestreamDecodeCfg, TgcBytestreamDecodeCfg, MdtBytestreamDecodeCfg, CscBytestreamDecodeCfg, sTgcBytestreamDecodeCfg, MmBytestreamDecodeCfg
from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg, TgcRDODecodeCfg, MdtRDODecodeCfg, CscRDODecodeCfg, CscClusterBuildCfg, StgcRDODecodeCfg, MMRDODecodeCfg
from MuonConfig.MuonRdoDecodeConfig import MuonPrdCacheNames
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigInDetConfig.utils import getFlagsForActiveConfig

from AthenaConfiguration.Enums import BeamType, LHCPeriod

from .TrigMuonKeys import muonNames
muNames = muonNames().getNames('RoI')
muNamesFS = muonNames().getNames('FS')
muNamesLRT = muonNames().getNames('LRT')

def isCosmic(flags):
  #FIXME: this might not be ideal criteria to determine if this is cosmic chain but used to work in Run2 and will do for now, ATR-22758
  return (flags.Beam.Type == BeamType.Cosmics)

def isLRT(name):
  return "LRT" in name

#Returns relevant track collection name
def getIDTracks(flags, name='', muonIDreuse=False, precision=False, suffix=''):

  if muonIDreuse or suffix != '':
    if isLRT(name):
      return 'HLT_IDTrack_MuonComb_FTF_LRT'
    elif isCosmic(flags):
      return 'HLT_IDTrack_MuonComb_FTF'
    else:
      return 'HLT_IDTrack_MuonComb_FTF'+suffix

  else:
    if precision:
      return flags.Tracking.ActiveConfig.tracks_IDTrig
    else:
      if isLRT(name):
        return flags.Trigger.InDetTracking.muonLRT.tracks_FTF
      elif isCosmic(flags):
        return flags.Trigger.InDetTracking.cosmics.tracks_IDTrig
      else:
        return flags.Trigger.InDetTracking.muon.tracks_FTF


def MuDataPrepViewDataVerifierCfg(flags):
    result = ComponentAccumulator()

    # Cache
    dataObjects=[( 'RpcPrepDataCollection_Cache' , 'StoreGateSvc+RpcPrdCache' ),
                 ( 'TgcRdo_Cache' , 'StoreGateSvc+TgcRdoCache' ),
                 ( 'MdtCsm_Cache' , 'StoreGateSvc+MdtCsmRdoCache' ),
                 ( 'RpcPad_Cache' , 'StoreGateSvc+RpcRdoCache' ),
                 ( 'RpcCoinDataCollection_Cache' , 'StoreGateSvc+RpcCoinCache' ),
                 ( 'TgcPrepDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCache + 'AllBCs' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'PriorBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'NextBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'NextNextBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache )
               ]
    
    if flags.Detector.GeometrysTGC:
      dataObjects += [( 'sTgcPrepDataCollection_Cache'  , 'StoreGateSvc+' + MuonPrdCacheNames.sTgcCache)]
      if flags.Input.isMC:
        dataObjects += [( 'Muon::STGC_RawDataContainer' , 'StoreGateSvc+sTGCRDO' )]
    
    if flags.Detector.GeometryMM:
      dataObjects += [( 'MMPrepDataCollection_Cache'  , 'StoreGateSvc+' + MuonPrdCacheNames.MmCache)]
      if flags.Input.isMC:
        dataObjects += [( 'Muon::MM_RawDataContainer' , 'StoreGateSvc+MMRDO' )]

    if flags.Detector.GeometryCSC:
      dataObjects+=[( 'CscRawDataCollection_Cache' , 'StoreGateSvc+CscRdoCache' )]
      if flags.Input.isMC:
        dataObjects += [( 'CscRawDataContainer' , 'StoreGateSvc+CSCRDO' ),
                        ( 'CscRawDataCollection_Cache' , 'StoreGateSvc+CscRdoCache' )]
    
    if flags.Input.isMC:
        dataObjects += [( 'MdtCsmContainer' , 'StoreGateSvc+MDTCSM' ),
                        *( [( 'RpcPadContainer' , 'StoreGateSvc+RPCPAD' )] if "RPCPAD" in flags.Input.Collections else [] ),
                        *( [( 'xAOD::NRPCRDOContainer' , 'StoreGateSvc+NRPCRDO' )] if flags.Muon.enableNRPC else [] ),
                        ('TgcRdoContainer' , 'StoreGateSvc+TGCRDO' )]
    
    if flags.Muon.usePhaseIIGeoSetup:
        dataObjects += [('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' )]
      
    alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuDataPrep",
                                                 DataObjects = dataObjects)
    result.addEventAlgo(alg)
    return result


def muonDecodeCfg(flags, RoIs):
    acc = ComponentAccumulator()

    doSeededDecoding =True
    if 'FSRoI' in RoIs:
      doSeededDecoding = False
    acc.merge(MuDataPrepViewDataVerifierCfg(flags))
        
    # Get RPC BS decoder
    if not flags.Input.isMC:
      rpcAcc = RpcBytestreamDecodeCfg( flags, name = "RpcRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
      acc.merge( rpcAcc )
      if flags.Muon.enableNRPC:
        acc.merge(NrpcBytestreamDecodeCfg( flags, name="NRpcRawDataProvider_"+RoIs ))
    # Get RPC RDO convertor
    rpcAcc = RpcRDODecodeCfg( flags, name= "RpcRdoToRpcPrepData_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
    acc.merge( rpcAcc )
    # Get TGC BS decoder
    if not flags.Input.isMC:
        tgcAcc = TgcBytestreamDecodeCfg( flags, name="TgcRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
        acc.merge( tgcAcc )
    # Get TGC RDO convertor
    tgcAcc = TgcRDODecodeCfg( flags, name="TgcRdoToTgcPrepData_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
    acc.merge( tgcAcc )
    # Get MDT BS decoder
    if not flags.Input.isMC:
        mdtAcc = MdtBytestreamDecodeCfg( flags, name="MdtRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
        acc.merge( mdtAcc )
    # Get MDT RDO convertor
    mdtAcc = MdtRDODecodeCfg( flags, name="MdtRdoToMdtPrepData_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
    acc.merge( mdtAcc )
    # Get CSC BS decoder
    if flags.Detector.GeometryCSC:
        if not flags.Input.isMC:
            cscAcc = CscBytestreamDecodeCfg( flags, name="CscRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
            acc.merge( cscAcc )
        # Get CSC RDO convertor
        cscAcc = CscRDODecodeCfg( flags, name="CscRdoToCscPrepData_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
        acc.merge( cscAcc )
        # Get CSC cluster builder
        cscAcc = CscClusterBuildCfg( flags, name="CscThresholdClusterBuilder_"+RoIs )
        acc.merge( cscAcc )
    #sTGC and MM BS decoder
    if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM:
      if not flags.Input.isMC:
        stgcAcc = sTgcBytestreamDecodeCfg(flags, name="sTgcRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding)
        acc.merge( stgcAcc )
        mmAcc = MmBytestreamDecodeCfg(flags, name="MMRawDataProvider_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding)
        acc.merge( mmAcc )
      #sTGC and MM RDO converter
      stgcAcc = StgcRDODecodeCfg( flags, name="StgcRdoToStgcPrepData_"+RoIs, RoIs = RoIs, DoSeededDecoding = doSeededDecoding )
      acc.merge( stgcAcc )

      mmAcc = MMRDODecodeCfg( flags, name="MMRdoToMMPrepData_"+RoIs, RoIs =  RoIs, DoSeededDecoding = doSeededDecoding)
      acc.merge( mmAcc )

    # SpacePoint formation
    if flags.Muon.usePhaseIIGeoSetup and flags.Muon.scheduleActsReco:
      from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
      acc.merge( MuonSpacePointFormationCfg( flags, suffix =f'_{RoIs}' ) )
      
    return acc

def muFastVDVCfg(flags, RoIs, suffix, InsideOutMode, extraLoads):
  result=ComponentAccumulator()
  # In insideout mode, need to inherit muon decoding objects for TGC, RPC, MDT, CSC
  dataObjects=[]
  if InsideOutMode:
    dataObjects = [('Muon::TgcPrepDataContainer','StoreGateSvc+TGC_MeasurementsAllBCs'),
                   ('TgcRdoContainer' , 'StoreGateSvc+TGCRDO'),
                   ('Muon::RpcPrepDataContainer','StoreGateSvc+RPC_Measurements'),
                   ('Muon::MdtPrepDataContainer','StoreGateSvc+MDT_DriftCircles'),
                   *( [( 'RpcPadContainer' , 'StoreGateSvc+RPCPAD' )] if "RPCPAD" in flags.Input.Collections else [] )]
    if flags.Detector.GeometryCSC:
      dataObjects += [('Muon::CscPrepDataContainer','StoreGateSvc+CSC_Clusters')]
    if flags.Detector.GeometrysTGC:
      dataObjects += [('Muon::sTgcPrepDataContainer','StoreGateSvc+STGC_Measurements')]
    if flags.Detector.GeometryMM:
      dataObjects += [('Muon::MMPrepDataContainer','StoreGateSvc+MM_Measurements')]
    
    if flags.Muon.usePhaseIIGeoSetup:
      dataObjects += [('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' )]
        
  else:
    dataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]
  dataObjects += [( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]
  if flags.Trigger.enableL1MuonPhase1:
    dataObjects += [( 'xAOD::MuonRoIContainer' , 'StoreGateSvc+LVL1MuonRoIs' )]
  else:
    dataObjects += [( 'DataVector< LVL1::RecMuonRoI >' , 'StoreGateSvc+HLT_RecMURoIs' )]
  
  #For L2 multi-track SA mode
  if extraLoads:
    dataObjects += extraLoads
  ViewVerify = CompFactory.AthViews.ViewDataVerifier("muFastRecoVDV"+suffix, DataObjects = dataObjects)

  result.addEventAlgo(ViewVerify)
  return result

def muFastRecoSequenceCfg( flags, RoIs, suffix="", doFullScanID = False, InsideOutMode=False, extraLoads=None, l2mtmode=False, calib=False, useNewFast=False ):

    acc = ComponentAccumulator()

    if useNewFast:
        acc.merge(muFastVDVCfg(flags, RoIs=RoIs, suffix=suffix, InsideOutMode=False, extraLoads=None))

        from MuonFastRecoAlgs.MuonFastReconstructionConfig import MuonFastReconstructionAlgCfg
        acc.merge(MuonFastReconstructionAlgCfg(flags, name=f"MuonFastReconstructionAlg{suffix}",
                                                      OutMuons=muNames.L2SAPhIIName))
    else:

        suffix = ""
        if InsideOutMode:
            suffix="IOmode"
        elif l2mtmode:
            suffix="l2mtmode"
        elif calib:
            suffix="Calib"

        acc.merge(muFastVDVCfg(flags, RoIs=RoIs, suffix=suffix, InsideOutMode=InsideOutMode, extraLoads=extraLoads))

        ### set up MuFastSteering ###
        from TrigL2MuonSA.TrigL2MuonSAConfig import l2MuFastAlgCfg
        acc.merge(l2MuFastAlgCfg(flags,
                                roisKey = RoIs,
                                setup = suffix,
                                FILL_FSIDRoI = doFullScanID,
                                MuonL2SAInfo = muNames.L2SAName+suffix,
                                L2IOCB = muNames.L2CBName+suffix,
                                forID = muNames.L2forIDName+suffix,
                                forMS = "forMS"+suffix,
                                TrackParticlesContainerName = getIDTracks(flags)))
    return acc

def muonIDtrackVDVCfg( flags, name, RoIs, extraLoads=None, extraLoadsForl2mtmode=None ):
  result=ComponentAccumulator()
  dataObjects=[( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]
  if extraLoads:
    dataObjects += extraLoads
  if extraLoadsForl2mtmode:
    dataObjects += extraLoadsForl2mtmode
  ViewVerify = CompFactory.AthViews.ViewDataVerifier("muCombVDV"+name, DataObjects = dataObjects)

  result.addEventAlgo(ViewVerify)
  return result

def muonIDFastTrackingSequenceCfg( flags, RoIs, name, extraLoads=None, extraLoadsForl2mtmode=None, doLRT=False, trackingMode="FTF" ):

  acc = ComponentAccumulator()
  from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
  acc.merge(trigInDetFastTrackingCfg( flags, roisKey=RoIs, signatureName=name, patternMode=trackingMode ))

  acc.merge(muonIDtrackVDVCfg(flags, name, RoIs, extraLoads, extraLoadsForl2mtmode))

  return acc

def muonIDCosmicTrackingSequenceCfg( flags, RoIs, name, extraLoads=None, extraLoadsForl2mtmode=None ):

  acc = ComponentAccumulator()

  acc.merge(muonIDtrackVDVCfg(flags, 'cosmics', RoIs, extraLoads, extraLoadsForl2mtmode))

  flagsWithTrk = getFlagsForActiveConfig(flags, "cosmics", log)

  from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
  seq = InnerTrackingTrigSequence.create(flagsWithTrk, 
                                         flagsWithTrk.Tracking.ActiveConfig.input_name, 
                                         rois = RoIs, 
                                         inView = "muCombVDVcosmics")
  acc.merge(seq.sequence("Offline"))

  
  return acc

def muCombVDVCfg( flags, postFix):
  result=ComponentAccumulator()
  dataObjects=[('xAOD::L2StandAloneMuonContainer','StoreGateSvc+%s' % muNames.L2SAName+postFix)]
  ViewVerify = CompFactory.AthViews.ViewDataVerifier("muCombAlgVDV"+postFix, DataObjects = dataObjects)
  result.addEventAlgo(ViewVerify)
  return result


def muCombRecoSequenceCfg( flags, RoIs, name, l2mtmode=False, l2CBname="" ):

  acc = ComponentAccumulator()
  postFix = ""
  if l2mtmode:
    postFix = "l2mtmode"

  acc.merge(muCombVDVCfg(flags, postFix))
  from TrigmuComb.TrigmuCombConfig import muCombCfg
  l2trackname = getIDTracks(flags) if l2mtmode else getIDTracks(flags, name)
  acc.merge(muCombCfg(flags, f'{postFix}_{name}', useBackExtrp=True,
                      L2StandAloneMuonContainerName = muNames.L2SAName+postFix,
                      L2CombinedMuonContainerName = l2CBname, TrackParticleContainerName = l2trackname ))

  return acc

def EFMuSADataPrepViewDataVerifierCfg(flags, RoIs, suffix=""):
    result=ComponentAccumulator()
    dataObjects=[( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]

    if flags.Muon.usePhaseIIGeoSetup:
        dataObjects += [( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' )]

    if flags.Muon.setupTruthAlgorithms:
        dataObjects += [( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+MDT_SDO' ),
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+MM_SDO' ),
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+RPC_SDO' ),
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+TGC_SDO' ),
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+sTGC_SDO' ),
                        ( 'McEventCollection' , 'StoreGateSvc+TruthEvent' ),
                        # Truth muons containers and decorations
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles' ), 
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles.truthClassification' ), 
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles.truthOrigin' ), 
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles.truthParticleLink' ), 
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles.truthType' ),
                        # Truth segments and SDO-to-segment links
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments' ), 
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments.charge' ), 
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments.localSegPars' ), 
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments.pt' ), 
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments.simHitLinks' ), 
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+MDT_SDO.truthSegmentLink' ), 
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+MM_SDO.truthSegmentLink' ), 
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+RPC_SDO.truthSegmentLink' ), 
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+TGC_SDO.truthSegmentLink' ),
                        ( 'xAOD::MuonSimHitContainer' , 'StoreGateSvc+sTGC_SDO.truthSegmentLink' ), 
                        # Truth segment to truth particle links
                        ( 'xAOD::MuonSegmentContainer' , 'StoreGateSvc+MuonTruthSegments.truthParticleLink' ), 
                        ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+MuonTruthParticles.truthSegmentLinks' )]

    alg = CompFactory.AthViews.ViewDataVerifier( name = f"VDVMuEFSA_{suffix}",
                                                DataObjects = dataObjects)
    result.addEventAlgo(alg)
    return result


def muEFSARecoSequenceCfg(flags, RoIs, suffix="", useBucketFilter=False):

    acc = ComponentAccumulator()

    acc.merge(EFMuSADataPrepViewDataVerifierCfg(flags, RoIs=RoIs, suffix=suffix))

    nameGroups = muNamesFS if 'FS' in suffix else muNames

    if flags.Muon.usePhaseIIGeoSetup and flags.Muon.scheduleActsReco:

        msMuonName = nameGroups.EFSAPhIIName
        if "newFast" in suffix:
            msMuonName = nameGroups.EFSAPhIINewFastName
        elif useBucketFilter:
            msMuonName = nameGroups.EFSAPhIIMlbktName

        # Schedule reco-to-truth object association
        if flags.Muon.setupTruthAlgorithms:
            from MuonObjectMarker.ObjectMarkerConfig import TruthMeasMarkerAlgCfg
            acc.merge(TruthMeasMarkerAlgCfg(flags, name = f"TruthMeasMarkerAlg{suffix}"))
            from MuonTruthAlgsR4.MuonTruthAlgsConfig import TruthHitAssociationCfg, RecoSegmentTruthAssocCfg, MuonToTruthAssocAlgCfg
            acc.merge(TruthHitAssociationCfg(flags, useSDO=True, suffix=suffix))
            acc.merge(RecoSegmentTruthAssocCfg(flags, name=f"MuonSegmentsFromR4TruthMatching{suffix}",
                                                      SegmentKey="MuonSegmentsFromR4"))
            acc.merge(MuonToTruthAssocAlgCfg(flags, name=f'MuonToTruthMatchingAlg{suffix}'))

        # ML bucket filter
        if useBucketFilter:
            from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
            bucketTool = acc.popToolsAndMerge(GraphBucketFilterToolCfg(flags, name=f"GraphBucketFilterTool{suffix}", 
                                                                              WriteSpacePointKey="FilteredMlBuckets"))
            acc.merge(GraphInferenceAlgCfg(flags, name = f"GraphInferenceAlg{suffix}",
                                                  InferenceTools=[bucketTool]))

        # Pattern recognition & segment fitting
        from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonEtaHoughTransformAlgCfg, MuonNSWSegmentFinderAlgCfg, MuonPhiHoughTransformAlgCfg, MuonSegmentFittingAlgCfg
        segmentContainers = []
        if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
            segmentContainers+=["MuonNswSegments"]
            acc.merge(MuonEtaHoughTransformAlgCfg(flags, name=f"MuonNswEtaHoughTransformAlg{suffix}", 
                                                         EtaHoughMaxContainer = "MuonHoughNswMaxima", 
                                                         SpacePointContainer = "NswSpacePoints"))
            acc.merge(MuonNSWSegmentFinderAlgCfg(flags, name=f"MuonNswSegmentFinderAlg{suffix}", 
                                                        MuonNswSegmentWriteKey = segmentContainers[-1], 
                                                        MuonNswSegmentSeedWriteKey = "MuonNswSegmentSeeds",
                                                        CombinatorialReadKey = "MuonHoughNswMaxima"))
        
        if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
            if useBucketFilter:
                acc.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}",
                                                             SpacePointContainer = "FilteredMlBuckets"))
            else:
                acc.merge(MuonEtaHoughTransformAlgCfg(flags, name = f"MuonEtaHoughTransformAlg{suffix}"))
                
            acc.merge(MuonPhiHoughTransformAlgCfg(flags, name = f"MuonPhiHoughTransformAlg{suffix}"))
            
            segmentContainers+=["R4MuonSegments"]
            acc.merge(MuonSegmentFittingAlgCfg(flags, name = f"MuonSegmentFittingAlg{suffix}",
                                                      OutSegmentContainer=segmentContainers[-1]))
        
        from MuonSegmentCnv.MuonSegmentCnvConfig import xAODSegmentCnvAlgCfg
        acc.merge(xAODSegmentCnvAlgCfg(flags, name = f"MuonR4xAODSegmentCnvAlg{suffix}", 
                                              InSegmentKeys = segmentContainers))
        
        from MuonSegmentCnv.MuonSegmentCnvConfig import MuonR4SegmentCnvAlgCfg
        acc.merge(MuonR4SegmentCnvAlgCfg(flags, name=f"MuonR4SegmentCnvAlg{suffix}",
                                                ReadSegments = segmentContainers,
                                                WriteKey="TrackMuonSegments"))
        
        # Track building
        from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFinderAlgCfg, StandaloneTrackPartCnvCfg, MuidSaTagMakerAlgCfg, MuonCreatorAlgCfg
        acc.merge(MSTrackFinderAlgCfg(flags, name=f"MSTrackFinderAlg{suffix}"))
        trackContainer = f"HLT_MuonMSTrackParticles_{suffix}"
        acc.merge(StandaloneTrackPartCnvCfg(flags, name=f"MuonMsTrackParticleCnvR4{suffix}",
                                                   TrackParticlesOutKey = trackContainer))
        acc.merge(MuidSaTagMakerAlgCfg(flags, name=f"MuidSaTagMakerAlg{suffix}", 
                                              MsTracks = trackContainer))
        acc.merge(MuonCreatorAlgCfg(flags, name = f"MuonActsCreatorAlg{suffix}",
                                           TagKeys = ["MuonTagsSA"],
                                           MuonKey = msMuonName))
        # This alg needs to be removed once the Phase-2 combined steps is ready
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCombinedMuonCandidateAlgCfg
        acc.merge(MuonCombinedMuonCandidateAlgCfg(flags, name=f"MuonCombinedMuonCandidateAlg{suffix}",
                                                         MuonSpectrometerTrackParticleLocation = trackContainer))
        
    else:
        msMuonName = nameGroups.EFSAName

        from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFinderAlgCfg, MuonLayerHoughAlgCfg
        acc.merge(MuonLayerHoughAlgCfg(flags, f"TrigMuonLayerHoughAlg{suffix}"))
        acc.merge(MuonSegmentFinderAlgCfg(flags, f"TrigMuonSegmentMaker{suffix}"))

        from MuonSegmentTrackMaker.MuonTrackMakerAlgsMonitoring import MuPatTrackBuilderMonitoring
        from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg
        from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
        acc.merge(MuPatTrackBuilderCfg(flags, name=f"TrigMuPatTrackBuilder{suffix}",
                                              MuonSegmentCollection = "TrackMuonSegments", 
                                              MonTool = MuPatTrackBuilderMonitoring(flags, f"MuPatTrackBuilderMonitoringSA{suffix}")))
        
        acc.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags, name = f"TrigMuonStandaloneTrackParticleCnvAlg{suffix}"))
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCombinedMuonCandidateAlgCfg, MuonCreatorAlgCfg
        from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
        acc.merge(MuonCombinedMuonCandidateAlgCfg(flags, name=f"TrigMuonCandidateAlg{suffix}"))
        acc.merge(MuonCreatorAlgCfg(flags, name=f"TrigMuonCreatorAlg_{suffix}", 
                                           CreateSAmuons=True, 
                                           TagMaps=[], 
                                           MuonContainerLocation=msMuonName,
                                           ExtrapolatedLocation = f"HLT_MSExtrapolatedMuons_{suffix}", 
                                           MSOnlyExtrapolatedLocation = f"HLT_MSOnlyExtrapolatedMuons_{suffix}",
                                           MonTool = MuonCreatorAlgMonitoring(flags, f"MuonCreatorAlgSA_{suffix}")))

    sequenceOut = msMuonName

    return acc, sequenceOut



def VDVEFMuCBCfg(flags, RoIs, name, suffix):
  acc = ComponentAccumulator()
  dataObjects = [( 'Muon::MdtPrepDataContainer' , 'StoreGateSvc+MDT_DriftCircles' ),  
                 ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_MeasurementsAllBCs' ),
                 ( 'Muon::RpcPrepDataContainer' , 'StoreGateSvc+RPC_Measurements' ),
                 ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                 ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                 ]
  if "FS" in name:
    dataObjects +=[( 'MuonCandidateCollection' , 'StoreGateSvc+MuonCandidates_FS' )]
  else:
    dataObjects +=[( 'MuonCandidateCollection' , 'StoreGateSvc+MuonCandidates')]

  if flags.Detector.GeometryCSC:
    dataObjects += [( 'Muon::CscStripPrepDataContainer' , 'StoreGateSvc+CSC_Measurements' )]
  if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM: 
    dataObjects += [( 'Muon::MMPrepDataContainer' , 'StoreGateSvc+MM_Measurements'),
                    ( 'Muon::sTgcPrepDataContainer' , 'StoreGateSvc+STGC_Measurements') ]
  if flags.Muon.usePhaseIIGeoSetup:
      dataObjects += [( 'MuonR4::SpacePointContainer' , 'StoreGateSvc+MuonSpacePoints' )]
      if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        dataObjects += [( 'MuonR4::SpacePointContainer' , 'StoreGateSvc+NswSpacePoints' )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuEFCB_"+name+suffix,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc



def VDVPrecMuTrkCfg(flags, name, suffix):
  acc = ComponentAccumulator()

  vdvName = "VDVMuTrkLRT" if "LRT" in name else "VDVMuTrk"
  trkname = "LRT" if "LRT" in name else ''
  dataObjects = [( 'xAOD::IParticleContainer' , 'StoreGateSvc+'+ getIDTracks(flags, trkname) )]
  
  if not flags.Muon.enableTrigIDtrackReuse and suffix == '':
    dataObjects += [( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+getIDTracks(flags, trkname, muonIDreuse=flags.Muon.enableTrigIDtrackReuse) )]
  else:
    if suffix != 'idReuse':
      MuonL2CBContainer = muNames.L2CBName+suffix
    else:
      MuonL2CBContainer = muNames.L2CBName
    dataObjects += [( 'xAOD::L2CombinedMuonContainer', 'StoreGateSvc+'+MuonL2CBContainer)]

  if not flags.Input.isMC:
    dataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
                    ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = vdvName+suffix,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def VDVidReuseITkCfg(flags, suffix):
  acc = ComponentAccumulator()

  vdvName = "VDVidReuseITk"
  dataObjects = []

  from TrigInDetConfig.TrigInDetConfig import InDetExtraDataObjectsFromDataPrep
  InDetExtraDataObjectsFromDataPrep(flags, dataObjects)

  alg = CompFactory.AthViews.ViewDataVerifier( name = vdvName+suffix,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def muEFCBRecoSequenceCfg( flags, RoIs, name, suffix ):


  from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
  from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg, MuonCombinedAlgCfg, MuonCombinedInDetCandidateAlgCfg
  acc = ComponentAccumulator()

  acc.merge(VDVEFMuCBCfg(flags, RoIs, name, suffix))

  if "FS" in name:
    #Need to run tracking for full scan chains
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    acc.merge(trigInDetFastTrackingCfg( flags, roisKey=RoIs, signatureName="muonFS" ))

  else:
    acc.merge(VDVPrecMuTrkCfg(flags, name, suffix))


  #Precision Tracking
  from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
  #When run in a different view than FTF some data dependencies needs to be loaded through verifier
  #Pass verifier as an argument and it will automatically append necessary DataObjects
  #@NOTE: Don't provide any verifier if loaded in the same view as FTF
  if isCosmic(flags) and 'LRT' not in name:
    trackParticles=getIDTracks(flags, name, muonIDreuse=flags.Muon.enableTrigIDtrackReuse)
  elif 'LRT' in name:
     muLrtFlags = getFlagsForActiveConfig(flags, "muonLRT", log)
     acc.merge(trigInDetPrecisionTrackingCfg(muLrtFlags, rois= RoIs, signatureName="muonLRT"))
     trackParticles = getIDTracks(muLrtFlags, name, precision=True)
  elif 'FS' in name:
     muFsFlags = getFlagsForActiveConfig(flags, "muonFS", log)
     acc.merge(trigInDetPrecisionTrackingCfg(muFsFlags, rois= RoIs, signatureName="muonFS", in_view=False))
     trackParticles = getIDTracks(muFsFlags, precision=True)
  else:
     muFlags = getFlagsForActiveConfig(flags, "muon", log)
     if not flags.Muon.enableTrigIDtrackReuse and suffix == '':
        acc.merge(trigInDetPrecisionTrackingCfg(muFlags, rois= RoIs, signatureName="muon"))
     trackParticles=getIDTracks(muFlags, name, muonIDreuse=flags.Muon.enableTrigIDtrackReuse, precision=True, suffix=suffix)

  if flags.Muon.enableTrigIDtrackReuse or suffix != '':
     if 'LRT' not in name or 'FS' not in name:
        if flags.GeoModel.Run > LHCPeriod.Run3:
           acc.merge(VDVidReuseITkCfg(flags, suffix))
        if suffix != 'idReuse':
           MuonL2CBInputContainer = muNames.L2CBName+suffix
        else:
           MuonL2CBInputContainer = muNames.L2CBName
        from TrigMuonEF.TrigMuonEFConfig import GetL2CBmuonInDetTracksAlgCfg
        acc.merge(GetL2CBmuonInDetTracksAlgCfg(flags, name="GetL2CBInDetTracks"+suffix,
                                         MuonL2CBContainerLocation=MuonL2CBInputContainer, 
                                         IDtrackOutputLocation="HLT_IDTrack_MuonComb_FTF"+suffix))

  #Make InDetCandidates
  acc.merge(MuonCombinedInDetCandidateAlgCfg(flags, name="TrigMuonCombinedInDetCandidateAlg_"+name+suffix,TrackParticleLocation = [trackParticles], InDetCandidateLocation="InDetCandidates_"+name+suffix))


  #MS ID combination
  candidatesName = "MuonCandidates"
  if 'FS' in name:
    candidatesName = "MuonCandidates_FS"

  acc.merge(MuonCombinedAlgCfg(flags,name="TrigMuonCombinedAlg_"+name+suffix, MuonCandidateLocation=candidatesName, InDetCandidateLocation="InDetCandidates_"+name+suffix))

  cbMuonName = muNames.EFCBOutInName
  if 'FS' in name:
    cbMuonName = muNamesFS.EFCBOutInName
  elif 'LRT' in name:
    cbMuonName = muNamesLRT.EFCBName


  acc.merge(MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgCB_"+name+suffix, MuonCandidateLocation=[candidatesName], TagMaps=["muidcoTagMap"], InDetCandidateLocation="InDetCandidates_"+name+suffix,
                                       MuonContainerLocation = cbMuonName+suffix, ExtrapolatedLocation = "CBExtrapolatedMuons"+suffix,
                                       MSOnlyExtrapolatedLocation = "CBMSonlyExtrapolatedMuons"+suffix, CombinedLocation = "HLT_CBCombinedMuon_"+name+suffix,
                                       MonTool = MuonCreatorAlgMonitoring(flags, "MuonCreatorAlgCB_"+name+suffix)))



  return acc


def VDVMuInsideOutCfg(flags, name, candidatesName, suffix):
  acc = ComponentAccumulator()
  dataObjects = [( 'Muon::RpcPrepDataContainer' , 'StoreGateSvc+RPC_Measurements' ),
                 ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_MeasurementsAllBCs' ),
                 ( 'MuonCandidateCollection' , 'StoreGateSvc+'+candidatesName ),
                 ('Trk::SegmentCollection' , 'StoreGateSvc+TrackMuonSegments')]
  if not isCosmic(flags): dataObjects += [( 'Muon::HoughDataPerSectorVec' , 'StoreGateSvc+HoughDataPerSectorVec')]
  if flags.Detector.GeometryCSC:
    dataObjects += [( 'Muon::CscPrepDataContainer' , 'StoreGateSvc+CSC_Clusters' )]
  if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM:
    dataObjects += [( 'Muon::MMPrepDataContainer'       , 'StoreGateSvc+MM_Measurements'),
                    ( 'Muon::sTgcPrepDataContainer'     , 'StoreGateSvc+STGC_Measurements') ]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuInsideOut_"+name+suffix,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def muEFInsideOutRecoSequenceCfg(flags, RoIs, name, suffix ):

  from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFinderAlgCfg, MuonLayerHoughAlgCfg
  from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
  from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg, MuGirlStauAlgCfg, StauCreatorAlgCfg, MuonInDetToMuonSystemExtensionAlgCfg, MuonInsideOutRecoAlgCfg, MuonCombinedInDetCandidateAlgCfg
  from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonInsideOutRecoToolCfg

  acc = ComponentAccumulator()
  
  candidatesName = "MuonCandidates"
  if 'FS' in name:
    candidatesName = "MuonCandidates_FS"

  if "Late" in name:

    #Need to run hough transform at start of late muon chain   
    acc.merge(MuonLayerHoughAlgCfg(flags, "TrigMuonLayerHoughAlg_"+name,MuonPatternCombinationCollection="MuonLayerHoughCombis_"+name,
                                   Key_MuonLayerHoughToolHoughDataPerSectorVec="HoughDataPerSectorVec_"+name))

    acc.merge(MuonSegmentFinderAlgCfg(flags, "TrigMuonSegmentMaker_"+name,MuonLayerHoughCombisKey="MuonLayerHoughCombis_"+name))


    # need to run precisions tracking for late muons, since we don't run it anywhere else
    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    muLateFlags = getFlagsForActiveConfig(flags, "muonLate", log)
    acc.merge(trigInDetPrecisionTrackingCfg(muLateFlags, rois= RoIs, signatureName="muonLate", in_view=False))
    trackParticles = muLateFlags.Tracking.ActiveConfig.tracks_IDTrig

    #Make InDetCandidates
    acc.merge(MuonCombinedInDetCandidateAlgCfg(flags, name="TrigMuonCombinedInDetCandidateAlg_"+name,TrackParticleLocation=[trackParticles],ForwardParticleLocation=trackParticles,InDetCandidateLocation="InDetCandidates_"+name,ExtendBulk=True))

  else:
    # for non-latemu chains, the decoding/hough transform is run in an earlier step
    #Need PRD containers for inside-out reco
    acc.merge(VDVMuInsideOutCfg(flags, name, candidatesName, suffix))

  #Inside-out reconstruction

  cbMuonName = muNames.EFCBInOutName+suffix
  if 'Late' in name:
    cbMuonName = recordable(cbMuonName+"_Late")
    acc.merge(MuGirlStauAlgCfg(flags, name="TrigMuonLateInsideOutRecoAlg_"+name,InDetCandidateLocation="InDetCandidates_"+name))
    acc.merge(StauCreatorAlgCfg(flags, name="TrigLateMuonCreatorAlg_"+name, TagMaps=["stauTagMap"], SegmentContainerName="", InDetCandidateLocation="InDetCandidates_"+name,
                                         MuonContainerLocation=cbMuonName, MonTool=MuonCreatorAlgMonitoring(flags, "LateMuonCreatorAlg_"+name)))
  else:
    acc.merge(MuonInDetToMuonSystemExtensionAlgCfg(flags, name="TrigInDetMuonExtensionAlg_"+name+suffix, InputInDetCandidates="InDetCandidates_"+name+suffix,
                                                          WriteInDetCandidates="InDetCandidatesSystemExtended_"+name+suffix))
    if 'RoI' in name:
      ioTool = MuonInsideOutRecoToolCfg(flags, MuonLayerSegmentFinderTool="", InputSegments="TrackMuonSegments")
    else:
      ioTool = MuonInsideOutRecoToolCfg(flags)
    InsideOutRecoTool = acc.popToolsAndMerge(ioTool)

    acc.merge(MuonInsideOutRecoAlgCfg(flags, name="TrigMuonInsideOutRecoAlg_"+name+suffix,InDetCandidateLocation="InDetCandidatesSystemExtended_"+name+suffix, MuonCombinedInDetExtensionTool=InsideOutRecoTool))

    acc.merge(MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgInsideOut_"+name+suffix,  MuonCandidateLocation=[candidatesName], TagMaps=["muGirlTagMap"],InDetCandidateLocation="InDetCandidates_"+name+suffix,
                                         MuonContainerLocation = cbMuonName, ExtrapolatedLocation = "InsideOutCBExtrapolatedMuons"+suffix,
                                         MSOnlyExtrapolatedLocation = "InsideOutCBMSOnlyExtrapolatedMuons"+suffix, CombinedLocation = "InsideOutCBCombinedMuon"+suffix, MonTool = MuonCreatorAlgMonitoring(flags, "MuonCreatorAlgInsideOut_"+name+suffix)))

  return acc


def VDVMuIsoCfg(flags, name, RoIs):
  acc = ComponentAccumulator()
  dataObjects = [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+'+RoIs ),
                             ( 'xAOD::MuonContainer' , 'StoreGateSvc+IsoViewMuons'+name )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "efMuIsoVDV"+name,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def efmuisoRecoSequenceCfg( flags, RoIs, Muons, doMSiso=False ):

  name = ""
  if doMSiso:
    name = "MS"
  acc = VDVMuIsoCfg(flags, name, RoIs)

  acc.merge(muonIDFastTrackingSequenceCfg(flags, RoIs, "muonIso"+name ))

  from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
  muIsoFlags = getFlagsForActiveConfig(flags, "muonIso"+name, log)
  acc.merge(trigInDetPrecisionTrackingCfg(muIsoFlags, rois= RoIs, signatureName="muonIso"+name, in_view=False))
  trackParticles = muIsoFlags.Tracking.ActiveConfig.tracks_IDTrig

  # Isolation alg
  from TrigMuonEF.TrigMuonEFConfig import TrigMuonEFTrackIsolationAlgCfg
  acc.merge(TrigMuonEFTrackIsolationAlgCfg(flags,name="TrigEFMuIso"+name, requireCombinedMuon = not doMSiso, 
                                           MuonEFContainer = Muons,IdTrackParticles = trackParticles, MuonContName = muNames.EFIsoMuonName+name,
                                           ptcone02Name = muNames.EFIsoMuonName+name + ".ptcone02",
                                           ptcone03Name = muNames.EFIsoMuonName+name + ".ptcone03"))

  return acc

def VDVLateMuCfg(flags):
  acc = ComponentAccumulator()
  dataObjects = [( 'xAOD::MuonRoIContainer', 'StoreGateSvc+LVL1MuonRoIsBCm2' ),
                 ( 'xAOD::MuonRoIContainer', 'StoreGateSvc+LVL1MuonRoIsBCm1' ),
                 ( 'xAOD::MuonRoIContainer', 'StoreGateSvc+LVL1MuonRoIsBCp1' ),
                 ( 'xAOD::MuonRoIContainer', 'StoreGateSvc+LVL1MuonRoIsBCp2' )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "efLateMuRoIVDV",
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def efLateMuRoISequenceCfg(flags):

  acc = VDVLateMuCfg(flags)

  from TrigmuRoI.TrigmuRoIConfig import TrigmuRoIConfig
  sequenceOut = "LateMuRoIs"
  acc.merge(TrigmuRoIConfig(flags, "TrigmuRoI", outputRoIs=sequenceOut))

  return acc, sequenceOut
