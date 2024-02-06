#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

### Output data name ###
from TrigEDMConfig.TriggerEDM import recordable
from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg, TgcBytestreamDecodeCfg, MdtBytestreamDecodeCfg, CscBytestreamDecodeCfg, sTgcBytestreamDecodeCfg, MmBytestreamDecodeCfg
from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg, TgcRDODecodeCfg, MdtRDODecodeCfg, CscRDODecodeCfg, CscClusterBuildCfg, StgcRDODecodeCfg, MMRDODecodeCfg
from MuonConfig.MuonRdoDecodeConfig import MuonPrdCacheNames
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

CBTPname = recordable("HLT_CBCombinedMuon_RoITrackParticles")
CBTPnameFS = recordable("HLT_CBCombinedMuon_FSTrackParticles")
CBTPnameLRT = recordable("HLT_CBCombinedMuon_LRTTrackParticles")
ExtrpTPname = recordable("HLT_MSExtrapolatedMuons_RoITrackParticles")
ExtrpTPnameFS = recordable("HLT_MSExtrapolatedMuons_FSTrackParticles")
MSextrpTPname = recordable("HLT_MSOnlyExtrapolatedMuons_FSTrackParticles")


from AthenaConfiguration.Enums import BeamType

class muonNames(object):
  def __init__(self):
    #EFSA and EFCB containers have different names 
    #for RoI and FS running. Other containers are 
    #produced in RoIs only.

    self.L2SAName = recordable("HLT_MuonL2SAInfo")
    self.L2CBName = recordable("HLT_MuonL2CBInfo")
    self.EFSAName = "Muons"
    self.EFCBName = "MuonsCB"
    self.EFCBOutInName = "MuonsCBOutsideIn"
    self.EFCBInOutName = "HLT_MuonsCBInsideOut"
    self.L2IsoMuonName = recordable("HLT_MuonL2ISInfo")
    self.EFIsoMuonName = recordable("HLT_MuonsIso")
    self.L2forIDName   = "RoIs_fromL2SAViews"

  def getNames(self, name):

    if "FS" in name:
      self.EFSAName = recordable("HLT_Muons_FS")
      self.EFCBName = recordable("HLT_MuonsCB_FS")
      self.EFCBOutInName = "MuonsCBOutsideIn_FS"
    if "RoI" in name:
      self.EFSAName = recordable("HLT_Muons_RoI")
      self.EFCBName = recordable("HLT_MuonsCB_RoI")
    if "LRT" in name:
      self.L2CBName = recordable("HLT_MuonL2CBInfoLRT")
      self.EFSAName = recordable("HLT_Muons_RoI")
      self.EFCBName = recordable("HLT_MuonsCB_LRT")
    return self

muNames = muonNames().getNames('RoI')
muNamesFS = muonNames().getNames('FS')
muNamesLRT = muonNames().getNames('LRT')

def isCosmic(flags):
  #FIXME: this might not be ideal criteria to determine if this is cosmic chain but used to work in Run2 and will do for now, ATR-22758
  return (flags.Beam.Type == BeamType.Cosmics)

def isLRT(name):
  return "LRT" in name

#Returns relevant track collection name
def getIDTracks(flags, name=''):

  from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

  if isLRT(name):
    return getInDetTrigConfig("muonLRT").tracks_FTF()
  elif isCosmic(flags):
    return getInDetTrigConfig("cosmics" ).tracks_IDTrig()
  else:
    return getInDetTrigConfig("muon").tracks_FTF()


def MuDataPrepViewDataVerifierCfg(flags):
    result = ComponentAccumulator()
    dataobjects=[( 'RpcPrepDataCollection_Cache' , 'StoreGateSvc+RpcPrdCache' ),
                 ( 'TgcRdo_Cache' , 'StoreGateSvc+TgcRdoCache' ),
                 ( 'MdtCsm_Cache' , 'StoreGateSvc+MdtCsmRdoCache' ),
                 ( 'RpcPad_Cache' , 'StoreGateSvc+RpcRdoCache' ),
                 ( 'RpcCoinDataCollection_Cache' , 'StoreGateSvc+RpcCoinCache' ),
                 ( 'TgcPrepDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCache + 'PriorBC' ),
                 ( 'TgcPrepDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCache + 'NextBC' ),
                 ( 'TgcPrepDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCache + 'AllBCs' ),
                 ( 'TgcPrepDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCache ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'PriorBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'NextBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache + 'NextNextBC' ),
                 ( 'TgcCoinDataCollection_Cache' , 'StoreGateSvc+' + MuonPrdCacheNames.TgcCoinCache )
               ]
    if flags.Input.isMC:
      dataobjects += [( 'MdtCsmContainer' , 'StoreGateSvc+MDTCSM' ),
                      ( 'RpcPadContainer' , 'StoreGateSvc+RPCPAD' ),
                      ('TgcRdoContainer' , 'StoreGateSvc+TGCRDO' )]
    if flags.Detector.GeometryCSC:
      dataobjects+=[( 'CscRawDataCollection_Cache' , 'StoreGateSvc+CscRdoCache' )]
      if flags.Input.isMC:
        dataobjects += [( 'CscRawDataContainer' , 'StoreGateSvc+CSCRDO' ),
                        ( 'CscRawDataCollection_Cache' , 'StoreGateSvc+CscRdoCache' )]
    if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM and flags.Input.isMC:
      dataobjects += [( 'Muon::STGC_RawDataContainer' , 'StoreGateSvc+sTGCRDO' ),
                      ( 'Muon::MM_RawDataContainer' , 'StoreGateSvc+MMRDO' )]
    if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM:
      dataobjects += [( 'MMPrepDataCollection_Cache'  , 'StoreGateSvc+' + MuonPrdCacheNames.MmCache)]
      dataobjects += [( 'sTgcPrepDataCollection_Cache'  , 'StoreGateSvc+' + MuonPrdCacheNames.sTgcCache)]
    alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuDataPrep",
                                                 DataObjects = dataobjects)
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

    return acc

def muFastVDVCfg(flags, RoIs, postFix, InsideOutMode, extraLoads):
  result=ComponentAccumulator()
  # In insideout mode, need to inherit muon decoding objects for TGC, RPC, MDT, CSC
  dataObjects=[]
  if InsideOutMode:
    dataObjects = [('Muon::TgcPrepDataContainer','StoreGateSvc+TGC_Measurements'),
                   ('TgcRdoContainer' , 'StoreGateSvc+TGCRDO'),
                   ('Muon::RpcPrepDataContainer','StoreGateSvc+RPC_Measurements'),
                   ('Muon::MdtPrepDataContainer','StoreGateSvc+MDT_DriftCircles'),
                   ( 'RpcPadContainer' , 'StoreGateSvc+RPCPAD' )]
    if flags.Detector.GeometryCSC:
      dataObjects += [('Muon::CscPrepDataContainer','StoreGateSvc+CSC_Clusters')]
    if flags.Detector.GeometrysTGC:
      dataObjects += [('Muon::sTgcPrepDataContainer','StoreGateSvc+STGC_Measurements')]
    if flags.Detector.GeometryMM:
      dataObjects += [('Muon::MMPrepDataContainer','StoreGateSvc+MM_Measurements')]
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
  ViewVerify = CompFactory.AthViews.ViewDataVerifier("muFastRecoVDV"+postFix, DataObjects = dataObjects)

  result.addEventAlgo(ViewVerify)
  return result

def muFastRecoSequenceCfg( flags, RoIs, doFullScanID = False, InsideOutMode=False, extraLoads=None, l2mtmode=False, calib=False ):

  acc = ComponentAccumulator()
  postFix = ""
  if InsideOutMode:
    postFix = "IOmode"
  elif l2mtmode:
    postFix = "l2mtmode"
  elif calib:
    postFix = "Calib"

  acc.merge(muFastVDVCfg(flags, RoIs, postFix, InsideOutMode, extraLoads))


  ### set up MuFastSteering ###
  from TrigL2MuonSA.TrigL2MuonSAConfig import l2MuFastAlgCfg
  acc.merge(l2MuFastAlgCfg(flags,
                           roisKey = RoIs,
                           setup = postFix,
                           FILL_FSIDRoI = doFullScanID,
                           MuonL2SAInfo = muNames.L2SAName+postFix,
                           L2IOCB = muNames.L2CBName+postFix,
                           forID = muNames.L2forIDName+postFix,
                           forMS = "forMS"+postFix,
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

def muonIDFastTrackingSequenceCfg( flags, RoIs, name, extraLoads=None, extraLoadsForl2mtmode=None, doLRT=False ):

  acc = ComponentAccumulator()
  from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
  acc.merge(trigInDetFastTrackingCfg( flags, roisKey=RoIs, signatureName=name ))

  acc.merge(muonIDtrackVDVCfg(flags, name, RoIs, extraLoads, extraLoadsForl2mtmode))

  return acc

def muonIDCosmicTrackingSequenceCfg( flags, RoIs, name, extraLoads=None, extraLoadsForl2mtmode=None ):

  acc = ComponentAccumulator()

  acc.merge(muonIDtrackVDVCfg(flags, 'cosmics', RoIs, extraLoads, extraLoadsForl2mtmode))

  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flagsWithTrk = getFlagsForActiveConfig(flags, "cosmics", log)

  from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
  seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                          rois = RoIs, inView = "muCombVDVcosmics")
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

def EFMuSADataPrepViewDataVerifierCfg(flags, RoIs, roiName):
  result=ComponentAccumulator()
  dataobjects=[( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
               ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuEFSA_"+roiName,
                                               DataObjects = dataobjects)
  result.addEventAlgo(alg)
  return result


def muEFSARecoSequenceCfg( flags, RoIs, name ):


  from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
  from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFinderAlgCfg, MuonLayerHoughAlgCfg, MuonSegmentFilterAlgCfg
  from MuonConfig.MuonTrackBuildingConfig import MuPatTrackBuilderCfg, EMEO_MuPatTrackBuilderCfg
  from xAODTrackingCnv.xAODTrackingCnvConfig import MuonStandaloneTrackParticleCnvAlgCfg
  from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCombinedMuonCandidateAlgCfg, MuonCreatorAlgCfg

  acc = ComponentAccumulator()

  acc.merge(EFMuSADataPrepViewDataVerifierCfg(flags, RoIs, name))

  
  acc.merge(MuonLayerHoughAlgCfg(flags, "TrigMuonLayerHoughAlg"))

  # if NSW is excluded from reconstruction (during commissioning)
  if flags.Muon.runCommissioningChain:
    acc.merge(MuonSegmentFinderAlgCfg(flags, name="TrigMuonSegmentMaker_"+name,SegmentCollectionName="TrackMuonSegments_withNSW"))
    acc.merge(MuonSegmentFilterAlgCfg(flags, name="TrigMuonSegmentFilter_"+name,SegmentCollectionName="TrackMuonSegments_withNSW",
                                                FilteredCollectionName="TrackMuonSegments", TrashUnFiltered=False, ThinStations=()))
  else:
    acc.merge(MuonSegmentFinderAlgCfg(flags, "TrigMuonSegmentMaker_"+name))

  from MuonSegmentTrackMaker.MuonTrackMakerAlgsMonitoring import MuPatTrackBuilderMonitoring

  if flags.Muon.runCommissioningChain:
    acc.merge(EMEO_MuPatTrackBuilderCfg(flags, name="TrigMuPatTrackBuilder_"+name ,MuonSegmentCollection = "TrackMuonSegments", MonTool = MuPatTrackBuilderMonitoring(flags, "MuPatTrackBuilderMonitoringSA_"+name), SpectrometerTrackOutputLocation="MuonSpectrometerTracks"))

  else:
    acc.merge(MuPatTrackBuilderCfg(flags, name="TrigMuPatTrackBuilder_"+name ,MuonSegmentCollection = "TrackMuonSegments", MonTool = MuPatTrackBuilderMonitoring(flags, "MuPatTrackBuilderMonitoringSA_"+name)))

  acc.merge(MuonStandaloneTrackParticleCnvAlgCfg(flags, name = "TrigMuonStandaloneTrackParticleCnvAlg_"+name))
  acc.merge(MuonCombinedMuonCandidateAlgCfg(flags, name="TrigMuonCandidateAlg_"+name))

  msMuonName = muNames.EFSAName
  if 'FS' in name:
    msMuonName = muNamesFS.EFSAName

  acc.merge(MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlg_"+name, CreateSAmuons=True, TagMaps=[], MuonContainerLocation=msMuonName,
                              ExtrapolatedLocation = "HLT_MSExtrapolatedMuons_"+name, MSOnlyExtrapolatedLocation = "HLT_MSOnlyExtrapolatedMuons_"+name,
                              MonTool = MuonCreatorAlgMonitoring(flags, "MuonCreatorAlgSA_"+name)))




  sequenceOut = msMuonName

  return acc, sequenceOut



def VDVEFMuCBCfg(flags, RoIs, name):
  acc = ComponentAccumulator()
  dataObjects = [( 'Muon::MdtPrepDataContainer' , 'StoreGateSvc+MDT_DriftCircles' ),  
                 ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_Measurements' ),
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

  alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuEFCB_"+name,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc

def VDVPrecMuTrkCfg(flags, name):
  acc = ComponentAccumulator()

  vdvName = "VDVMuTrkLRT" if "LRT" in name else "VDVMuTrk"
  trkname = "LRT" if "LRT" in name else ''
  dataObjects = [( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+'+getIDTracks(flags, trkname) ),
                 ( 'xAOD::IParticleContainer' , 'StoreGateSvc+'+ getIDTracks(flags, trkname) )]

  if not flags.Input.isMC:
    dataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
                    ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = vdvName,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc



def muEFCBRecoSequenceCfg( flags, RoIs, name ):


  from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
  from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg, MuonCombinedAlgCfg, MuonCombinedInDetCandidateAlgCfg
  acc = ComponentAccumulator()

  acc.merge(VDVEFMuCBCfg(flags, RoIs, name))




  if "FS" in name:
    #Need to run tracking for full scan chains
    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    acc.merge(trigInDetFastTrackingCfg( flags, roisKey=RoIs, signatureName="muonFS" ))

  else:
    acc.merge(VDVPrecMuTrkCfg(flags, name))


  #Precision Tracking
  from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
  #When run in a different view than FTF some data dependencies needs to be loaded through verifier
  #Pass verifier as an argument and it will automatically append necessary DataObjects
  #@NOTE: Don't provide any verifier if loaded in the same view as FTF
  if isCosmic(flags) and 'LRT' not in name:
    trackParticles = getIDTracks(flags)
  elif 'LRT' in name:
    muLrtFlags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.muonLRT")
    acc.merge(trigInDetPrecisionTrackingCfg(muLrtFlags, rois= RoIs, signatureName="muonLRT"))
    trackParticles = muLrtFlags.Tracking.ActiveConfig.tracks_IDTrig
  elif 'FS' in name:
    muFsFlags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.muonFS")
    acc.merge(trigInDetPrecisionTrackingCfg(muFsFlags, rois= RoIs, signatureName="muonFS", in_view=False))
    trackParticles = muFsFlags.Tracking.ActiveConfig.tracks_IDTrig
  else:
    muFlags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.muon")
    acc.merge(trigInDetPrecisionTrackingCfg(muFlags, rois= RoIs, signatureName="muon"))
    trackParticles = muFlags.Tracking.ActiveConfig.tracks_IDTrig


  #Make InDetCandidates
  acc.merge(MuonCombinedInDetCandidateAlgCfg(flags, name="TrigMuonCombinedInDetCandidateAlg_"+name,TrackParticleLocation = [trackParticles], InDetCandidateLocation="InDetCandidates_"+name))


  #MS ID combination
  candidatesName = "MuonCandidates"
  if 'FS' in name:
    candidatesName = "MuonCandidates_FS"

  acc.merge(MuonCombinedAlgCfg(flags,name="TrigMuonCombinedAlg_"+name, MuonCandidateLocation=candidatesName, InDetCandidateLocation="InDetCandidates_"+name))

  cbMuonName = muNames.EFCBOutInName
  if 'FS' in name:
    cbMuonName = muNamesFS.EFCBOutInName
  elif 'LRT' in name:
    cbMuonName = muNamesLRT.EFCBName


  acc.merge(MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgCB_"+name, MuonCandidateLocation=[candidatesName], TagMaps=["muidcoTagMap"], InDetCandidateLocation="InDetCandidates_"+name,
                                       MuonContainerLocation = cbMuonName, ExtrapolatedLocation = "CBExtrapolatedMuons",
                                       MSOnlyExtrapolatedLocation = "CBMSonlyExtrapolatedMuons", CombinedLocation = "HLT_CBCombinedMuon_"+name,
                                       MonTool = MuonCreatorAlgMonitoring(flags, "MuonCreatorAlgCB_"+name)))



  return acc


def VDVMuInsideOutCfg(flags, name, candidatesName):
  acc = ComponentAccumulator()
  dataObjects = [( 'Muon::RpcPrepDataContainer' , 'StoreGateSvc+RPC_Measurements' ),
                 ( 'Muon::TgcPrepDataContainer' , 'StoreGateSvc+TGC_Measurements' ),
                 ( 'MuonCandidateCollection' , 'StoreGateSvc+'+candidatesName )]
  if not isCosmic(flags): dataObjects += [( 'Muon::HoughDataPerSectorVec' , 'StoreGateSvc+HoughDataPerSectorVec')]
  if flags.Detector.GeometryCSC:
    dataObjects += [( 'Muon::CscPrepDataContainer' , 'StoreGateSvc+CSC_Clusters' )]
  if flags.Detector.GeometrysTGC and flags.Detector.GeometryMM:
    dataObjects += [( 'Muon::MMPrepDataContainer'       , 'StoreGateSvc+MM_Measurements'),
                    ( 'Muon::sTgcPrepDataContainer'     , 'StoreGateSvc+STGC_Measurements') ]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "VDVMuInsideOut_"+name,
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def muEFInsideOutRecoSequenceCfg(flags, RoIs, name):

  from MuonConfig.MuonSegmentFindingConfig import MuonSegmentFinderAlgCfg, MuonLayerHoughAlgCfg, MuonSegmentFilterAlgCfg
  from MuonCombinedAlgs.MuonCombinedAlgsMonitoring import MuonCreatorAlgMonitoring
  from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCreatorAlgCfg, MuGirlStauAlgCfg, StauCreatorAlgCfg, MuonInDetToMuonSystemExtensionAlgCfg, MuonInsideOutRecoAlgCfg, MuonCombinedInDetCandidateAlgCfg

  acc = ComponentAccumulator()
  
  candidatesName = "MuonCandidates"
  if 'FS' in name:
    candidatesName = "MuonCandidates_FS"

  if "Late" in name:

    #Need to run hough transform at start of late muon chain   
    acc.merge(MuonLayerHoughAlgCfg(flags, "TrigMuonLayerHoughAlg"))

    # if NSW is excluded from reconstruction (during commissioning)
    if flags.Muon.runCommissioningChain:
      acc.merge(MuonSegmentFinderAlgCfg(flags, name="TrigMuonSegmentMaker_"+name,SegmentCollectionName="TrackMuonSegments_withNSW"))
      acc.merge(MuonSegmentFilterAlgCfg(flags, name="TrigMuonSegmentFilter_"+name,SegmentCollectionName="TrackMuonSegments_withNSW",
                                                  FilteredCollectionName="TrackMuonSegments", TrashUnFiltered=False, ThinStations=())) 
    else:
      acc.merge(MuonSegmentFinderAlgCfg(flags, "TrigMuonSegmentMaker_"+name))


    # need to run precisions tracking for late muons, since we don't run it anywhere else
    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    muLateFlags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.muonLate")
    acc.merge(trigInDetPrecisionTrackingCfg(muLateFlags, rois= RoIs, signatureName="muonLate"))
    trackParticles = muLateFlags.Tracking.ActiveConfig.tracks_IDTrig

    #Make InDetCandidates
    acc.merge(MuonCombinedInDetCandidateAlgCfg(flags, name="TrigMuonCombinedInDetCandidateAlg_"+name,TrackParticleLocation = [trackParticles],ForwardParticleLocation=trackParticles, InDetCandidateLocation="InDetCandidates_"+name))

  else:
    # for non-latemu chains, the decoding/hough transform is run in an earlier step
    #Need PRD containers for inside-out reco
    acc.merge(VDVMuInsideOutCfg(flags, name, candidatesName))

  #Inside-out reconstruction

  cbMuonName = muNames.EFCBInOutName
  if 'Late' in name:
    cbMuonName = cbMuonName+"_Late"
    acc.merge(MuGirlStauAlgCfg(flags, name="TrigMuonLateInsideOutRecoAlg_"+name,InDetCandidateLocation="InDetCandidates_"+name))
    acc.merge(StauCreatorAlgCfg(flags, name="TrigLateMuonCreatorAlg_"+name, TagMaps=["stauTagMap"],InDetCandidateLocation="InDetCandidates_"+name,
                                         MuonContainerLocation = cbMuonName, MonTool = MuonCreatorAlgMonitoring(flags, "LateMuonCreatorAlg_"+name)))
  else:
    acc.merge(MuonInDetToMuonSystemExtensionAlgCfg(flags, name="TrigInDetMuonExtensionAlg_"+name, InputInDetCandidates="InDetCandidates_"+name,
                                                          WriteInDetCandidates="InDetCandidatesSystemExtended_"+name))
    acc.merge(MuonInsideOutRecoAlgCfg(flags, name="TrigMuonInsideOutRecoAlg_"+name,InDetCandidateLocation="InDetCandidatesSystemExtended_"+name))

    acc.merge(MuonCreatorAlgCfg(flags, name="TrigMuonCreatorAlgInsideOut_"+name,  MuonCandidateLocation=[candidatesName], TagMaps=["muGirlTagMap"],InDetCandidateLocation="InDetCandidates_"+name,
                                         MuonContainerLocation = cbMuonName, ExtrapolatedLocation = "InsideOutCBExtrapolatedMuons",
                                         MSOnlyExtrapolatedLocation = "InsideOutCBMSOnlyExtrapolatedMuons", CombinedLocation = "InsideOutCBCombinedMuon", MonTool = MuonCreatorAlgMonitoring(flags, "MuonCreatorAlgInsideOut_"+name)))



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
  muIsoFlags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.muonIso"+name)
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
  # TODO: Replace MuCTPI_RDO dependency with xAOD::MuonRoIContainer for BC+1, BC-1 candidates, ATR-25031
  dataObjects = [( 'MuCTPI_RDO' , 'StoreGateSvc+MUCTPI_RDO' )]

  alg = CompFactory.AthViews.ViewDataVerifier( name = "efLateMuRoIVDV",
                                               DataObjects = dataObjects)
  acc.addEventAlgo(alg)
  return acc


def efLateMuRoISequenceCfg(flags):

  acc = VDVLateMuCfg(flags)
  # Make sure the RDOs are still available at whole-event level
  loadFromSG= [( 'MuCTPI_RDO' , 'StoreGateSvc+MUCTPI_RDO' )]
  from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
  acc.merge(SGInputLoaderCfg(flags, Load=loadFromSG))

  from TrigmuRoI.TrigmuRoIConfig import TrigmuRoIConfig
  sequenceOut = "LateMuRoIs"
  acc.merge(TrigmuRoIConfig(flags, "TrigmuRoI", outputRoIs=sequenceOut))

  return acc, sequenceOut
