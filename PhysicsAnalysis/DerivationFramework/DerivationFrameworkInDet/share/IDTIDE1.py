#====================================================================
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# IDTIDE1.py
# Contact: atlas-cp-tracking-denseenvironments@cern.ch
# 
# Caveat: This is still under development.
#       - storing strategy: so far use AddItem() method. But in future,
#         should move to SlimmingHelper() method.
#====================================================================
 
# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

from DerivationFrameworkInDet.InDetCommon import *
from InDetPrepRawDataToxAOD.InDetDxAODJobProperties import InDetDxAODFlags
from InDetRecExample import TrackingCommon
from AthenaCommon.CFElements import seqAND, parOR

from AthenaCommon.Logging import logging
msg = logging.getLogger( "IDTIDE1" )
_info = msg.info

#Steering options

idDxAOD_doPix=True
idDxAOD_doSct=True
idDxAOD_doTrt=False

select_aux_items=True
# should be true in production 
skimEvents=True

from AthenaCommon.JobProperties import jobproperties
from InDetPrepRawDataToxAOD.InDetDxAODJobProperties import InDetDxAODFlags
pix_from_InDetDxAOD = InDetDxAODFlags.DumpPixelInfo() and jobproperties.PrimaryDPDFlags.WriteDAOD_IDTRKVALIDStream.get_Value() is True
sct_from_InDetDxAOD = InDetDxAODFlags.DumpSctInfo()   and jobproperties.PrimaryDPDFlags.WriteDAOD_IDTRKVALIDStream.get_Value() is True
trt_from_InDetDxAOD = InDetDxAODFlags.DumpTrtInfo()   and jobproperties.PrimaryDPDFlags.WriteDAOD_IDTRKVALIDStream.get_Value() is True
need_pix_ToTList = idDxAOD_doPix and ( InDetDxAODFlags.DumpPixelRdoInfo() or InDetDxAODFlags.DumpPixelNNInfo() )

# IsMonteCarlo=(globalflags.DataSource == 'geant4')

if 'DerivationFrameworkIsMonteCarlo' not in dir() :
  DerivationFrameworkIsMonteCarlo=( globalflags.DataSource=='geant4' )

IsMonteCarlo=DerivationFrameworkIsMonteCarlo

#====================================================================
# Re-run jet reconstruction needed for preselection
#====================================================================
from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo,AntiKt4EMPFlow
jetList = [AntiKt4EMTopo,AntiKt4EMPFlow]
from DerivationFrameworkJetEtMiss.JetCommon import addDAODJets
addDAODJets(jetList,DerivationFrameworkJob)

#====================================================================
# SET UP STREAM  
#====================================================================
from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
from PrimaryDPDMaker.PrimaryDPDHelpers import buildFileName
from PrimaryDPDMaker.PrimaryDPDFlags import primDPD
streamName = primDPD.WriteDAOD_IDTIDEStream.StreamName
fileName   = buildFileName( primDPD.WriteDAOD_IDTIDEStream )
IDTIDE1Stream = MSMgr.NewPoolRootStream( streamName, fileName )

#idtideSeq = CfgMgr.AthSequencer("IDTIDE1Sequence")
#DerivationFrameworkJob += idtideSeq
#addTrackSumMoments("AntiKt4EMTopo")
#addDefaultTrimmedJets(idtideSeq,"IDTIDE1")

augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

#====================================================================
# CP GROUP TOOLS
#====================================================================
IDTIDE1IPETool = TrackingCommon.getTrackToVertexIPEstimator(name = "IDTIDE1IPETool")
_info(IDTIDE1IPETool)

#Setup tools
if idDxAOD_doTrt:
  from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawNeighbourSvc
  TRTStrawNeighbourSvc=TRT_StrawNeighbourSvc()
  ServiceMgr += TRTStrawNeighbourSvc
  from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
  TRTCalibDBTool=TRT_CalDbTool(name="TRT_CalDbTool")


#====================================================================
# AUGMENTATION TOOLS
#====================================================================
augmentationTools = []
tsos_augmentationTools=[]
# Add unbiased track parameters to track particles
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackToVertexWrapper
IDTIDE1TrackToVertexWrapper= DerivationFramework__TrackToVertexWrapper(name = "IDTIDE1TrackToVertexWrapper",
                                                                       TrackToVertexIPEstimator = IDTIDE1IPETool,
                                                                       DecorationPrefix = "IDTIDE1",
                                                                       ContainerName = "InDetTrackParticles")
ToolSvc += IDTIDE1TrackToVertexWrapper 
augmentationTools.append(IDTIDE1TrackToVertexWrapper)
_info(IDTIDE1TrackToVertexWrapper)

from InDetUsedInFitTrackDecoratorTool.InDetUsedInFitTrackDecoratorToolConf import InDet__InDetUsedInFitTrackDecoratorTool
IDTIDE1UsedInFitDecoratorTool = InDet__InDetUsedInFitTrackDecoratorTool(name                 = "IDTIDE1UsedInFitDecoratorTool",
                                                                          AMVFVerticesDecoName = "TTVA_AMVFVertices",
                                                                          AMVFWeightsDecoName  = "TTVA_AMVFWeights",
                                                                          TrackContainer       = "InDetTrackParticles",
                                                                          VertexContainer      = "PrimaryVertices" )
ToolSvc += IDTIDE1UsedInFitDecoratorTool

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__UsedInVertexFitTrackDecorator
IDTIDE1UsedInFitDecorator = DerivationFramework__UsedInVertexFitTrackDecorator(name                   = "IDTIDE1UsedInFitDecorator",
                                                                                UsedInFitDecoratorTool = ToolSvc.IDTIDE1UsedInFitDecoratorTool )
ToolSvc += IDTIDE1UsedInFitDecorator
augmentationTools.append(IDTIDE1UsedInFitDecorator)

# Turned off by defult for the LRT tracks as they are not used. Added this option to turn it on for future studies.
if InDetDxAODFlags.DecoLRTTTVA() and InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():

#====================================================================
# DECORATE THE LRT TRACKS WITH USED-IN-FIT TTVA VARIABLES
#====================================================================
  IDTIDE1UsedInFitDecoratorToolLRT = InDet__InDetUsedInFitTrackDecoratorTool(name                 = "IDTIDE1UsedInFitDecoratorToolLRT",
                                                                          AMVFVerticesDecoName = "TTVA_AMVFVertices",
                                                                          AMVFWeightsDecoName  = "TTVA_AMVFWeights",
                                                                          TrackContainer       = "InDetLargeD0TrackParticles",
                                                                          VertexContainer      = "PrimaryVertices" )
  ToolSvc += IDTIDE1UsedInFitDecoratorToolLRT

  IDTIDE1UsedInFitDecoratorLRT = DerivationFramework__UsedInVertexFitTrackDecorator(name                   = "IDTIDE1UsedInFitDecoratorLRT",
                                                                                UsedInFitDecoratorTool = ToolSvc.IDTIDE1UsedInFitDecoratorToolLRT )
  ToolSvc += IDTIDE1UsedInFitDecoratorLRT
  augmentationTools.append(IDTIDE1UsedInFitDecoratorLRT)

# @TODO eventually computed for other extra outputs. Possible to come  up with a solution to use a common Z0AtPV if there is more than one client ?
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParametersAtPV
DFCommonZ0AtPV = DerivationFramework__TrackParametersAtPV(name                       = "DFCommonZ0AtPV",
                                                          TrackParticleContainerName = "InDetTrackParticles",
                                                          VertexContainerName        = "PrimaryVertices",
                                                          Z0SGEntryName              = "IDTIDEInDetTrackZ0AtPV" )
ToolSvc += DFCommonZ0AtPV
augmentationTools.append(DFCommonZ0AtPV)


from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackStateOnSurfaceDecorator
DFTSOS = DerivationFramework__TrackStateOnSurfaceDecorator(name = "DFTrackStateOnSurfaceDecorator",
                                                          ContainerName = "InDetTrackParticles",
                                                          IsSimulation = False,
                                                          DecorationPrefix = "",
                                                          StoreTRT   = idDxAOD_doTrt,
                                                          AddExtraEventInfo = False,  # never decorate EventInfo with TRTPhase, doubt this is useful for IDTIDE
                                                          TRT_ToT_dEdx = TrackingCommon.getInDetTRT_dEdxTool() if idDxAOD_doTrt else "",
                                                          PRDtoTrackMap= "PRDtoTrackMap" + InDetKeys.UnslimmedTracks() if  jobproperties.PrimaryDPDFlags.WriteDAOD_IDTRKVALIDStream.get_Value() else "",
                                                          StoreSCT   = idDxAOD_doSct,
                                                          StorePixel = idDxAOD_doPix,
                                                          OutputLevel =INFO)
ToolSvc += DFTSOS
tsos_augmentationTools.append(DFTSOS)
_info(DFTSOS)

# Sequence for skimming kernel (if running on data) -> PrepDataToxAOD -> ID TIDE kernel
# sequence to be used for algorithm which should run before the IDTIDEPresel
IDTIDESequencePre   = DerivationFrameworkJob
IDTIDESequence = seqAND("IDTIDESequence")

# Add decoration with truth parameters if running on simulation
if IsMonteCarlo:
  # add track parameter decorations to truth particles but only if the decorations have not been applied already
  import InDetPhysValMonitoring.InDetPhysValDecoration
  meta_data = InDetPhysValMonitoring.InDetPhysValDecoration.getMetaData()
  from AthenaCommon.Logging import logging
  logger = logging.getLogger( "DerivationFramework" )
  if len(meta_data) == 0 :
    truth_track_param_decor_alg = InDetPhysValMonitoring.InDetPhysValDecoration.getInDetPhysValTruthDecoratorAlg()
    if  InDetPhysValMonitoring.InDetPhysValDecoration.findAlg([truth_track_param_decor_alg.getName()]) == None :
      IDTIDESequencePre += truth_track_param_decor_alg
    else :
      logger.info('Decorator %s already present not adding again.' % (truth_track_param_decor_alg.getName() ))
  else :
    logger.info('IDPVM decorations to track particles already applied to input file not adding again.')


#====================================================================
# SKIMMING TOOLS 
#====================================================================
skimmingTools = []
if not IsMonteCarlo and skimEvents:

  sel_jet600 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 600.*GeV'
  sel_jet800 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 800.*GeV'
  sel_jet1000 = 'AntiKt4EMPFlowJets.JetConstitScaleMomentum_pt >= 1000.*GeV'

  desd_jetA = '( HLT_j110_pf_ftf_preselj80_L1J30 || HLT_j175_pf_ftf_preselj140_L1J50 || HLT_j260_pf_ftf_preselj200_L1J75 )'
  desd_jetC = '( HLT_j360_pf_ftf_preselj225_L1J100 )'
  desd_jetD = '( HLT_j420_pf_ftf_preselj225_L1J100 && !HLT_j460_pf_ftf_preselj225_L1J100 )'
  desd_jetE = '( HLT_j460_pf_ftf_preselj225_L1J100 )'
  desd_jetF = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet600+')>0 && count('+sel_jet800+')==0 )'
  desd_jetG = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet800+')>0 && count('+sel_jet1000+')==0 )'
  desd_jetH = '( HLT_j460_pf_ftf_preselj225_L1J100 && count('+sel_jet1000+')>0 )'

  prescaleA = 20
  prescaleC = 40
  prescaleD = 30
  prescaleE = 20
  prescaleF = 10
  prescaleG = 5
  prescaleH = 1
  
  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__PrescaleTool


  IDTIDE_SkimmingToolA = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolA", expression = desd_jetA)
  ToolSvc += IDTIDE_SkimmingToolA
  IDTIDE_PrescaleToolA = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolA",Prescale=prescaleA)
  ToolSvc += IDTIDE_PrescaleToolA
  IDTIDE_ANDToolA = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolA",FilterList=[IDTIDE_SkimmingToolA,IDTIDE_PrescaleToolA] )
  ToolSvc += IDTIDE_ANDToolA

  IDTIDE_SkimmingToolC = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolC", expression = desd_jetC)
  ToolSvc += IDTIDE_SkimmingToolC
  IDTIDE_PrescaleToolC = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolC",Prescale=prescaleC)
  ToolSvc += IDTIDE_PrescaleToolC
  IDTIDE_ANDToolC = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolC",FilterList=[IDTIDE_SkimmingToolC,IDTIDE_PrescaleToolC] )
  ToolSvc += IDTIDE_ANDToolC

  IDTIDE_SkimmingToolD = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolD", expression = desd_jetD)
  ToolSvc += IDTIDE_SkimmingToolD
  IDTIDE_PrescaleToolD = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolD",Prescale=prescaleD)
  ToolSvc += IDTIDE_PrescaleToolD
  IDTIDE_ANDToolD = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolD",FilterList=[IDTIDE_SkimmingToolD,IDTIDE_PrescaleToolD] )
  ToolSvc += IDTIDE_ANDToolD

  IDTIDE_SkimmingToolE = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolE", expression = desd_jetE)
  ToolSvc += IDTIDE_SkimmingToolE
  IDTIDE_PrescaleToolE = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolE",Prescale=prescaleE)
  ToolSvc += IDTIDE_PrescaleToolE
  IDTIDE_ANDToolE = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolE",FilterList=[IDTIDE_SkimmingToolE,IDTIDE_PrescaleToolE] )
  ToolSvc += IDTIDE_ANDToolE

  IDTIDE_SkimmingToolF = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolF", expression = desd_jetF)
  ToolSvc += IDTIDE_SkimmingToolF
  IDTIDE_PrescaleToolF = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolF",Prescale=prescaleF)
  ToolSvc += IDTIDE_PrescaleToolF
  IDTIDE_ANDToolF = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolF",FilterList=[IDTIDE_SkimmingToolF,IDTIDE_PrescaleToolF] )
  ToolSvc += IDTIDE_ANDToolF

  IDTIDE_SkimmingToolG = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolG", expression = desd_jetG)
  ToolSvc += IDTIDE_SkimmingToolG
  IDTIDE_PrescaleToolG = DerivationFramework__PrescaleTool(name="IDTIDE_PrescaleToolG",Prescale=prescaleG)
  ToolSvc += IDTIDE_PrescaleToolG
  IDTIDE_ANDToolG = DerivationFramework__FilterCombinationAND(name="IDTIDE_ANDToolG",FilterList=[IDTIDE_SkimmingToolG,IDTIDE_PrescaleToolG] )
  ToolSvc += IDTIDE_ANDToolG

  IDTIDE_SkimmingToolH = DerivationFramework__xAODStringSkimmingTool(name = "IDTIDE_SkimmingToolH", expression = desd_jetH)
  ToolSvc += IDTIDE_SkimmingToolH

  IDTIDE_ORTool = DerivationFramework__FilterCombinationOR(name="myLogicalCombination", FilterList=[IDTIDE_ANDToolA,IDTIDE_ANDToolC,IDTIDE_ANDToolD,IDTIDE_ANDToolE,IDTIDE_ANDToolF,IDTIDE_ANDToolG,IDTIDE_SkimmingToolH] )
  ToolSvc += IDTIDE_ORTool
  
  skimmingTools.append(IDTIDE_ORTool)
  _info( "IDTIDE1.py IDTIDE_ORTool: %s", IDTIDE_ORTool)
  
  # Add the skimming kernel to the sequence
  from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
  IDTIDESequence += CfgMgr.DerivationFramework__DerivationKernel("IDTIDE1KernelPresel",
                                                                 SkimmingTools = skimmingTools)

# sequence for algorithms which should run after the preselection bu which can run in parallel
IDTIDESeqAfterPresel = parOR("IDTIDESeqAfterPresel")
IDTIDESequence += IDTIDESeqAfterPresel

#Setup decorators tools
if idDxAOD_doTrt and not trt_from_InDetDxAOD:
  from InDetPrepRawDataToxAOD.InDetPrepRawDataToxAODConf import TRT_PrepDataToxAOD
  xAOD_TRT_PrepDataToxAOD = TRT_PrepDataToxAOD( name = "xAOD_TRT_PrepDataToxAOD")
  xAOD_TRT_PrepDataToxAOD.OutputLevel=INFO
  xAOD_TRT_PrepDataToxAOD.UseTruthInfo=IsMonteCarlo
  _info( "Add TRT xAOD TrackMeasurementValidation: %s" , xAOD_TRT_PrepDataToxAOD)
  IDTIDESeqAfterPresel += xAOD_TRT_PrepDataToxAOD

if idDxAOD_doSct and not sct_from_InDetDxAOD:
  from InDetPrepRawDataToxAOD.InDetPrepRawDataToxAODConf import SCT_PrepDataToxAOD
  xAOD_SCT_PrepDataToxAOD = SCT_PrepDataToxAOD( name = "xAOD_SCT_PrepDataToxAOD")
  xAOD_SCT_PrepDataToxAOD.OutputLevel=INFO
  xAOD_SCT_PrepDataToxAOD.UseTruthInfo=IsMonteCarlo
  _info("Add SCT xAOD TrackMeasurementValidation: %s", xAOD_SCT_PrepDataToxAOD)
  IDTIDESeqAfterPresel += xAOD_SCT_PrepDataToxAOD

if idDxAOD_doPix and not pix_from_InDetDxAOD:
  if need_pix_ToTList :
    from PixelCalibAlgs.PixelCalibAlgsConf import PixelChargeToTConversion 
    PixelChargeToTConversionSetter = PixelChargeToTConversion(name = "PixelChargeToTConversionSetter",
                                                              ExtraOutputs = {'PixelClusters_ToTList'})
    # IDTIDESeqAfterPresel += PixelChargeToTConversionSetter 
    topSequence += PixelChargeToTConversionSetter
    _info("Add Pixel xAOD ToTConversionSetter: %s Properties: %s", PixelChargeToTConversionSetter, PixelChargeToTConversionSetter.properties())
  from InDetPrepRawDataToxAOD.InDetDxAODUtils import getPixelPrepDataToxAOD
  xAOD_PixelPrepDataToxAOD = getPixelPrepDataToxAOD(
                                                    # OutputLevel=INFO
                                                    )
  _info( "Add Pixel xAOD TrackMeasurementValidation: %s", xAOD_PixelPrepDataToxAOD)
  IDTIDESeqAfterPresel += xAOD_PixelPrepDataToxAOD


#====================================================================
# THINNING TOOLS 
#====================================================================
thinningTools = []

# TrackParticles directly
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
kw = {}
if not idDxAOD_doPix:
  kw['InDetTrackStatesPixKey'] = ''
  kw['InDetTrackMeasurementsPixKey'] = ''
if not idDxAOD_doSct:
  kw['InDetTrackStatesSctKey'] = ''
  kw['InDetTrackMeasurementsSctKey'] = ''
if not idDxAOD_doTrt:
  kw['InDetTrackStatesTrtKey'] = ''
  kw['InDetTrackMeasurementsTrtKey'] = ''
IDTIDE1ThinningTool = DerivationFramework__TrackParticleThinning(name = "IDTIDE1ThinningTool",
                                                                 StreamName              = streamName,
                                                                 SelectionString         = "abs(IDTIDEInDetTrackZ0AtPV) < 5.0",
                                                                 InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                 ThinHitsOnTrack =  InDetDxAODFlags.ThinHitsOnTrack(),
                                                                 **kw)
ToolSvc += IDTIDE1ThinningTool
thinningTools.append(IDTIDE1ThinningTool)

#====================================================================
# TRUTH THINNING
#====================================================================
if IsMonteCarlo:
  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
  IDTIDE1TruthThinningTool = DerivationFramework__MenuTruthThinning(name = "IDTIDE1TruthThinningTool",
      StreamName                 = streamName,
      WritePartons               = True,
      WriteHadrons               = True,
      WriteBHadrons              = True, 
      WriteGeant                 = True,
      GeantPhotonPtThresh        = 20000,
      WriteTauHad                = True,
      PartonPtThresh             = -1.0,
      WriteBSM                   = True,
      WriteBosons                = True,
      WriteBosonProducts         = True, 
      WriteBSMProducts           = True,
      WriteTopAndDecays          = True, 
      WriteEverything            = True,
      WriteAllLeptons            = True,
      WriteLeptonsNotFromHadrons = True,
      WriteStatus3               = True,
      WriteFirstN                = -1,
      PreserveAncestors          = True, 
      PreserveGeneratorDescendants = True)
  ToolSvc += IDTIDE1TruthThinningTool
  thinningTools.append(IDTIDE1TruthThinningTool)

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
IDTIDESkimmingSequence = seqAND("IDTIDESkimmingSequence")
IDTIDESeqAfterPresel += IDTIDESkimmingSequence
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
idtide_kernel = CfgMgr.DerivationFramework__DerivationKernel("IDTIDE1Kernel",
                                                             AugmentationTools = augmentationTools,
                                                             SkimmingTools = skimmingTools,
                                                             ThinningTools = [],
                                                             RunSkimmingFirst = True,
                                                             OutputLevel =INFO)

IDTIDEPostProcSequence = parOR("IDTIDEPostProcSequence")

# shared between IDTIDE and IDTRKVALID
dftsos_kernel = CfgMgr.DerivationFramework__DerivationKernel("DFTSOSKernel",
                                                             AugmentationTools = tsos_augmentationTools,
                                                             ThinningTools = [],
                                                             OutputLevel =INFO)
idtidethinning_kernel = CfgMgr.DerivationFramework__DerivationKernel("IDTIDEThinningKernel",
                                                             AugmentationTools = [],
                                                             ThinningTools = thinningTools,
                                                             OutputLevel =INFO)

IDTIDESkimmingSequence += idtide_kernel
IDTIDESkimmingSequence += IDTIDEPostProcSequence

IDTIDEPostProcSequence += dftsos_kernel
IDTIDEPostProcSequence += idtidethinning_kernel

DerivationFrameworkJob += IDTIDESequence
accept_algs=[ idtide_kernel.name() ]

# Set the accept algs for the stream
IDTIDE1Stream.AcceptAlgs( accept_algs )

#====================================================================
# CONTENT LIST  
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
IDTIDE1SlimmingHelper = SlimmingHelper("IDTIDE1SlimmingHelper")

AllVariables = []
StaticContent = []
SmartCollections = []
ExtraVariables = []

IDTIDE1SlimmingHelper.AppendToDictionary.update ({"Muons":"xAOD::MuonContainer", "MuonsAux":"xAOD::MuonAuxContainer", 
    "Electrons":"xAOD::ElectronContainer", "ElectronsAux":"xAOD::ElectronAuxContainer",
    "Photons":"xAOD::PhotonContainer", "PhotonsAux":"xAOD::PhotonAuxContainer",
    "JetETMissNeutralParticleFlowObjects":"xAOD::FlowElementContainer", "JetETMissNeutralParticleFlowObjectsAux":"xAOD::FlowElementAuxContainer",
    "JetETMissChargedParticleFlowObjects":"xAOD::FlowElementContainer", "JetETMissChargedParticleFlowObjectsAux":"xAOD::FlowElementAuxContainer",
    "TauJets":"xAOD::TauJetContainer", "TauJetsAux":"xAOD::TauJetAuxContainer",
    "InDetTrackParticles":"xAOD::TrackParticleContainer", "InDetTrackParticlesAux":"xAOD::TrackParticleAuxContainer",
    "InDetLargeD0TrackParticles":"xAOD::TrackParticleContainer", "InDetLargeD0TrackParticlesAux":"xAOD::TrackParticleAuxContainer", 
    "PixelClusters":"xAOD::TrackMeasurementValidationContainer", "PixelClustersAux":"xAOD::TrackMeasurementValidationAuxContainer", 
    "SCT_Clusters":"xAOD::TrackMeasurementValidationContainer", "SCT_ClustersAux":"xAOD::TrackMeasurementValidationAuxContainer", 
    "Kt4EMTopoOriginEventShape":"xAOD::EventShape", "Kt4EMTopoOriginEventShapeAux":"xAOD::EventShapeAuxInfo",
    "Kt4LCTopoOriginEventShape":"xAOD::EventShape", "Kt4LCTopoOriginEventShapeAux":"xAOD::EventShapeAuxInfo",
    "NeutralParticleFlowIsoCentralEventShape":"xAOD::EventShape", "NeutralParticleFlowIsoCentralEventShapeAux":"xAOD::EventShapeAuxInfo",
    "NeutralParticleFlowIsoForwardEventShape":"xAOD::EventShape", "NeutralParticleFlowIsoForwardEventShapeAux":"xAOD::EventShapeAuxInfo",
    "TopoClusterIsoCentralEventShape":"xAOD::EventShape", "TopoClusterIsoCentralEventShapeAux":"xAOD::EventShapeAuxInfo",
    "TopoClusterIsoForwardEventShape":"xAOD::EventShape", "TopoClusterIsoForwardEventShapeAux":"xAOD::EventShapeAuxInfo"}
)

SmartCollections += ["Muons", "Electrons", "Photons"]

AllVariables += ["EventInfo", 
    "JetETMissNeutralParticleFlowObjects", 
    "JetETMissChargedParticleFlowObjects",
    "InDetTrackParticles", 
    "InDetLargeD0TrackParticles", 
    "PixelClusters", 
    "SCT_Clusters", 
    "Kt4EMTopoOriginEventShape", 
    "Kt4LCTopoOriginEventShape", 
    "NeutralParticleFlowIsoCentralEventShape", 
    "NeutralParticleFlowIsoForwardEventShape",
    "TopoClusterIsoCentralEventShape", 
    "TopoClusterIsoForwardEventShape", 
]

IDTIDE1SlimmingHelper.AppendToDictionary.update({"TauJets":"xAOD::TauJetContainer", "TauJetsAux": "xAOD::TauJetAuxContainer",
    "Kt4EMPFlowEventShape":"xAOD::EventShape", "Kt4EMPFlowEventShapeAux":"xAOD::EventShapeAuxInfo",
    "PrimaryVertices":"xAOD::VertexContainer", "PrimaryVerticesAux":"xAOD::VertexAuxContainer",
    "InDetTrackParticlesClusterAssociations":"xAOD::TrackParticleClusterAssociationContainer", "InDetTrackParticlesClusterAssociationsAux":"xAOD::TrackParticleClusterAssociationAuxContainer",
    "AntiKt4EMTopoJets":"xAOD::JetContainer", "AntiKt4EMTopoJetsAux":"xAOD::JetAuxContainer",
    "AntiKt4EMPFlowJets":"xAOD::JetContainer", "AntiKt4EMPFlowJetsAux":"xAOD::JetAuxContainer",
    "BTagging_AntiKt4EMTopo":"xAOD::BTaggingContainer", "BTagging_AntiKt4EMTopoAux":"xAOD::BTaggingAuxContainer",
    "BTagging_AntiKt4EMPFlow":"xAOD::BTaggingContainer", "BTagging_AntiKt4EMPFlowAux":"xAOD::BTaggingAuxContainer"}
)

ExtraVariables += ["TauJets.ABS_ETA_LEAD_TRACK.ClusterTotalEnergy.ClustersMeanCenterLambda.ClustersMeanEMProbability.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.ClustersMeanSecondLambda.EMFRACTIONATEMSCALE_MOVEE3.EMFracFixed.GhostMuonSegmentCount.LeadClusterFrac.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.NNDecayModeProb_3pXn.PFOEngRelDiff.PanTau_DecayModeExtended.TAU_ABSDELTAETA.TAU_ABSDELTAPHI.TAU_SEEDTRK_SECMAXSTRIPETOVERPT.UpsilonCluster.absipSigLeadTrk.chargedFELinks.etHotShotDR1.etHotShotDR1OverPtLeadTrk.etHotShotWin.etHotShotWinOverPtLeadTrk.etaCombined.hadLeakFracFixed.leadTrackProbHT.mCombined.mu.nConversionTracks.nFakeTracks.nModifiedIsolationTracks.nVtxPU.neutralFELinks.passThinning.phiCombined.ptCombined.ptIntermediateAxisEM.rho"]
ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z"]

AllVariables += ["Kt4EMPFlowEventShape", "InDetTrackParticlesClusterAssociations", 
                  "AntiKt4EMTopoJets", "AntiKt4EMPFlowJets",
                  "BTagging_AntiKt4EMTopo", "BTagging_AntiKt4EMPFlow"]

if idDxAOD_doPix:     
    IDTIDE1SlimmingHelper.AppendToDictionary.update( {'PixelMSOSs':'xAOD::TrackStateValidationContainer','PixelMSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
    AllVariables += ["PixelMSOSs"]

if idDxAOD_doSct:
    IDTIDE1SlimmingHelper.AppendToDictionary.update( {'SCT_MSOSs':'xAOD::TrackStateValidationContainer','SCT_MSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
    AllVariables += ["SCT_MSOSs"]

if idDxAOD_doTrt:
    IDTIDE1SlimmingHelper.AppendToDictionary.update( {'TRT_MSOSs':'xAOD::TrackStateValidationContainer','TRT_MSOSsAux':'xAOD::TrackStateValidationAuxContainer'} )
    AllVariables += ["TRT_MSOSs"]

if IsMonteCarlo:

    IDTIDE1SlimmingHelper.AppendToDictionary.update({"AntiKt4TruthJets":"xAOD::JetContainer", "AntiKt4TruthJetsAux":"xAOD::JetAuxContainer",
        "JetInputTruthParticles":"xAOD::TruthParticleContainer",
        "JetInputTruthParticlesNoWZ":"xAOD::TruthParticleContainer",
        "TruthEvents":"xAOD::TruthEventContainer", "TruthEventsAux":"xAOD::TruthEventAuxContainer",
        "TruthParticles":"xAOD::TruthParticleContainer", "TruthParticlesAux":"xAOD::TruthParticleAuxContainer"}
    )

    AllVariables += ["AntiKt4TruthJets", 
                      "JetInputTruthParticles",
                      "JetInputTruthParticlesNoWZ",
                      "TruthEvents", 
                      "TruthParticles",
                      "egammaTruthParticles",
                      "MuonTruthParticles",
                      "LRTegammaTruthParticles",
                      "TruthVertices" 
                    ]

    list_aux = ["BHadronsFinal", "BHadronsInitial", "BQuarksFinal",  
                "CHadronsFinal", "CHadronsInitial", "CQuarksFinal",
                "HBosons", "Partons", "TQuarksFinal", "TausFinal", "WBosons", "ZBosons"] 
    for item in list_aux:
        label = "TruthLabel"+item
        labelAux = label+"Aux"
        IDTIDE1SlimmingHelper.AppendToDictionary.update( { label : "xAOD::TruthParticleContainer", labelAux : "xAOD::TruthParticleAuxContainer"} )
        AllVariables += [label]
# End of isMC block      

IDTIDE1SlimmingHelper.IncludeTriggerNavigation = True   # Trigger info is actually stored only when running on data...
IDTIDE1SlimmingHelper.IncludeAdditionalTriggerContent = True 

IDTIDE1SlimmingHelper.AllVariables = AllVariables
IDTIDE1SlimmingHelper.StaticContent = StaticContent
IDTIDE1SlimmingHelper.SmartCollections = SmartCollections
IDTIDE1SlimmingHelper.ExtraVariables = ExtraVariables

IDTIDE1SlimmingHelper.AppendContentToStream(IDTIDE1Stream)
