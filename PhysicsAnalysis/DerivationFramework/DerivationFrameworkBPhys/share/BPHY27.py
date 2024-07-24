#====================================================================
# BPHY27.py (Mostly derived from BPHY22)

# (B -> mu D*+)^2
# It requires the reductionConf flag BPHY27 in Reco_tf.py
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

# data or simulation?
isSimulation = False
if globalflags.DataSource()=='geant4':
    isSimulation = True

#svcMgr.MessageSvc.enableSuppression = False  
#svcMgr.MessageSvc.debugLimit = 500000000000

#====================================================================
# AUGMENTATION TOOLS
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY27_VertexTools = BPHYVertexTools("BPHY27")


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__AugOriginalCounts
BPHY27_AugOriginalCounts = DerivationFramework__AugOriginalCounts(
   name = "BPHY27_AugOriginalCounts",
   VertexContainer = "PrimaryVertices",
   TrackContainer = "InDetTrackParticles" )
ToolSvc += BPHY27_AugOriginalCounts

#===============================================================================================
#--------------------------------------------------------------------
# 1/ Select  mu pi
#--------------------------------------------------------------------
## a/ setup JpsiFinder tool
##    These are general tools independent of DerivationFramework that do the
##    actual vertex fitting and some pre-selection.
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY27MuPiFinder = Analysis__JpsiFinder(
    name                       = "BPHY27MuPiFinder",
    OutputLevel                = INFO, #DEBUG
    muAndMu                    = False,
    muAndTrack                 = True,  #need doTagAndProbe flag
    TrackAndTrack              = False,
    assumeDiMuons              = False,
    muonThresholdPt            = 2700,
    trackThresholdPt           = 250.0, # MeV
    invMassUpper               = 8200.0,
    invMassLower               = 200.0,
    Chi2Cut                    = 10.,
    oppChargesOnly             = False,
    allChargeCombinations      = True,
   # combOnly                   = False,
    atLeastOneComb             = False, # True by default
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY27_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY27_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY27_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY27_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY27_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    doTagAndProbe              = True, #won't work with all/same charges combs
    forceTagAndProbe           = True) #force T&P to work with any charges combs

ToolSvc += BPHY27MuPiFinder
print      BPHY27MuPiFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool is the JpsiFinder mass window is wide enough.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY27MuPiSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY27MuPiSelectAndWrite",
#    OutputLevel            = DEBUG,
    JpsiFinder             = BPHY27MuPiFinder,
    OutputVtxContainerName = "BPHY27MuPiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED")

ToolSvc +=  BPHY27MuPiSelectAndWrite
print       BPHY27MuPiSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select mu pi candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY27_Select_MuPi = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY27_Select_MuPi",
    HypothesisName        = "MuPi",
    InputVtxContainerName = "BPHY27MuPiCandidates",
 #   TrkMasses             = [105.658, 139.571],
    VtxMassHypo           = 5279.64,
    MassMin               = 200.0,
    MassMax               = 8200.0,
    Chi2Max               = 200)
    #LxyMin                = -100, #default lowest of Double

ToolSvc += BPHY27_Select_MuPi
print      BPHY27_Select_MuPi
##
#===============================================================================================


#===============================================================================================
##
#--------------------------------------------------------------------
# 2/ Select K+K-, pi+K- and K+pi-      for D0, Ds, Dm
#--------------------------------------------------------------------
## a/ Setup the vertex fitter tools
BPHY27DiTrkFinder = Analysis__JpsiFinder(
    name                       = "BPHY27DiTrkFinder",
    OutputLevel                = INFO, #DEBUG,
    muAndMu                    = False,
    muAndTrack                 = False,
    TrackAndTrack              = True,
    assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt           = 500, #900,
    invMassUpper               = 2100.0,
    invMassLower               = 275,
    Chi2Cut                    = 20., #chi2
    oppChargesOnly             = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY27_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY27_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY27_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY27_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY27_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
    track2Mass                 = 139.571,
    maxNTracksInEvent          = 500
)

ToolSvc += BPHY27DiTrkFinder
print      BPHY27DiTrkFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s).
BPHY27DiTrkSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY27DiTrkSelectAndWrite",
    JpsiFinder             = BPHY27DiTrkFinder,
    OutputVtxContainerName = "BPHY27DiTrkCandidates",
    PVContainerName        = "PrimaryVertices",
    #RefPVContainerName     = "BPHY27RefittedPrimaryVertices",#"SHOULDNOTBEUSED",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    #RefitPV                = True,
    #MaxPVrefit             = 100000,
    CheckCollections       = True,
    CheckVertexContainers  = ['BPHY27MuPiCandidates'])
  
ToolSvc += BPHY27DiTrkSelectAndWrite
print      BPHY27DiTrkSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select D0 candidates
BPHY27_Select_D0 = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY27_Select_D0",
    HypothesisName        = "D0",
    InputVtxContainerName = "BPHY27DiTrkCandidates",
    TrkMasses             = [139.571, 493.677],
    VtxMassHypo           = 1864.83,
    MassMin               = 1864.83-200,
    MassMax               = 1864.83+200,
    #LxyMin                = 0.0,#0.15,
    Chi2Max               = 50)

ToolSvc += BPHY27_Select_D0
print      BPHY27_Select_D0
##
#--------------------------------------------------------------------
## d/ augment and select D0bar candidates
BPHY27_Select_D0b = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY27_Select_D0b",
    HypothesisName        = "D0b",
    InputVtxContainerName = "BPHY27DiTrkCandidates",
    TrkMasses             = [493.677, 139.571],
    VtxMassHypo           = 1864.83,
    MassMin               = 1864.83-200,
    MassMax               = 1864.83+200,
    #LxyMin                = 0.0,
    Chi2Max               = 50)

ToolSvc += BPHY27_Select_D0b
print      BPHY27_Select_D0b
#==============================================================================================

#===============================================================================================

#--------------------------------------------------------------------
# 3/ select B -> mu pi D*
#--------------------------------------------------------------------
## a/ setup the cascade vertexing tool
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BMuDstVertexFit = Trk__TrkVKalVrtFitter(
    name                 = "BMuDstVertexFit",
    Extrapolator         = BPHY27_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += BMuDstVertexFit
print      BMuDstVertexFit

#--------------------------------------------------------------------
## b/ setup Jpsi D*+ finder
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__MuPlusDpstCascade
BPHY27MuDpst = DerivationFramework__MuPlusDpstCascade(
    name                     = "BPHY27MuDpst",
    HypothesisName           = "B",
    TrkVertexFitterTool      = BMuDstVertexFit,
    TrkVertexFitterToolAdd     = BPHY27_VertexTools.TrkVKalVrtFitter,

    DxHypothesis             = 421, # MC PID for D0
    ApplyD0MassConstraint    = True,
    MuPiMassLowerCut         = 200.,
    MuPiMassUpperCut         = 8200.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    DstMassUpperCutAft       = 2010.26 + 55., #mass cut after cascade fit old 25.
    MassLowerCut             = 0.,
    MassUpperCut             = 12500.,
    PtLowerCut               = 0.,
    Chi2Cut                  = 5, #chi2/ndf
    RefitPV                  = True,
    RefPVContainerName       = "BPHY27RefittedPrimaryVertices",
    MuPiVertices             = "BPHY27MuPiCandidates",
    CascadeVertexCollections = ["BMuDpstCascadeSV2", "BMuDpstCascadeSV1"],
    AdditionalCascadeVertexCollections = ["BMuDpstTrkCascadeSV2", "BMuDpstTrkCascadeSV1"],
    D0Vertices               = "BPHY27DiTrkCandidates",
    DoVertexType             = 15 )

ToolSvc += BPHY27MuDpst
print      BPHY27MuDpst
#===============================================================================================


#--------------------------------------------------------------------

CascadeCollections = []
CascadeCollections += BPHY27MuDpst.CascadeVertexCollections
CascadeCollections += BPHY27MuDpst.AdditionalCascadeVertexCollections

#--------------------------------------------------------------------
## 6/ select the event. We only want to keep events that have two muons (or passing the trigger / vertex requirements)

# Di-muon skimming
#muonsRequirements = '(Muons.pt >= 3.0*GeV) && (abs(Muons.eta) < 2.7) && (Muons.DFCommonMuonsPreselection)'
muonsRequirements = '(Muons.pt >= 3.0*GeV) && (abs(Muons.eta) < 2.7)'
objectSelection = 'count('+muonsRequirements+') >= 2'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY27_SelectDiMuonEvent = DerivationFramework__xAODStringSkimmingTool( name = "BPHY27_SelectDiMuonEvent",
                                                                 expression = objectSelection)
ToolSvc += BPHY27_SelectDiMuonEvent
print BPHY27_SelectDiMuonEvent

# (Optional) Trigger skimming
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
BPHY27_SelectMuonTriggerEvent = DerivationFramework__TriggerSkimmingTool( name = "BPHY27_SelectMuonTriggerEvent",
                                                                                   TriggerListOR = ['HLT_2mu4'])
ToolSvc += BPHY27_SelectMuonTriggerEvent
print BPHY27_SelectMuonTriggerEvent

# (Optional) B-vertex skimming
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY27_SelectBMuDxEvent = DerivationFramework__xAODStringSkimmingTool(
    name = "BPHY27_SelectBMuDxEvent",
    expression = "(count(BMuDpstCascadeSV1.x > -999)) > 0")

ToolSvc += BPHY27_SelectBMuDxEvent
print      BPHY27_SelectBMuDxEvent

#--------------------------------------------------------------------
##7/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which hasn't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept if it
##    satisfies any of the listed selections.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
BPHY27_thinningTool_Tracks = DerivationFramework__Thin_vtxTrk(
    name                       = "BPHY27_thinningTool_Tracks",
    ThinningService            = "BPHY27ThinningSvc",
    TrackParticleContainerName = "InDetTrackParticles",
    VertexContainerNames       = ["BMuDpstCascadeSV1", "BMuDpstCascadeSV2"],
    PassFlags                  = ["passed_B"])

ToolSvc += BPHY27_thinningTool_Tracks
print      BPHY27_thinningTool_Tracks

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BPhysPVThinningTool
BPHY27_thinningTool_PV = DerivationFramework__BPhysPVThinningTool(
    name                 = "BPHY27_thinningTool_PV",
    ThinningService      = "BPHY27ThinningSvc",
    CandidateCollections = ["BMuDpstCascadeSV1", "BMuDpstCascadeSV2"],
    KeepPVTracks         = True)

ToolSvc += BPHY27_thinningTool_PV
print      BPHY27_thinningTool_PV

## b) thinning out tracks that are not attached to muons. The final thinning decision is based on the OR operation
##    between decision from this and the previous tools.
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
BPHY27MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(
    name                   = "BPHY27MuonTPThinningTool",
    ThinningService        = "BPHY27ThinningSvc",
    MuonKey                = "Muons",
    InDetTrackParticlesKey = "InDetTrackParticles")

ToolSvc += BPHY27MuonTPThinningTool
print      BPHY27MuonTPThinningTool



#====================================================================
# Thinning collections
#====================================================================

thiningCollection = []
print thiningCollection


#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
#====================================================================
SeqBPHY27 = CfgMgr.AthSequencer("SeqBPHY27")
DerivationFrameworkJob += SeqBPHY27

# The name of the kernel (BPHY27Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel

# Run CPU-intensive algorithms afterwards to restrict those to skimmed events
SeqBPHY27 += CfgMgr.DerivationFramework__DerivationKernel(
  "BPHY27KernelSkim",
    SkimmingTools     = [BPHY27_SelectDiMuonEvent]
)
SeqBPHY27 += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY27KernelAug",
#    SkimmingTools     = [BPHY27_SelectBMuDxEvent],   #activate here if you want the vertex skimming
    AugmentationTools = [BPHY27MuPiSelectAndWrite,
                         BPHY27DiTrkSelectAndWrite,
                         BPHY27MuDpst,
                         BPHY27_AugOriginalCounts],
    ThinningTools     = thiningCollection
)

#====================================================================
# SET UP STREAM
#====================================================================
streamName   = derivationFlags.WriteDAOD_BPHY27Stream.StreamName
fileName     = buildFileName( derivationFlags.WriteDAOD_BPHY27Stream )
BPHY27Stream  = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY27Stream.AcceptAlgs(["BPHY27KernelSkim"])

# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

BPHY27ThinningSvc = createThinningSvc( svcName="BPHY27ThinningSvc", outStreams=[evtStream] )
svcMgr += BPHY27ThinningSvc

'''
#====================================================================
# SET UP TRUTH COLLECTION
#====================================================================
# Copied from PHYS.py (via SUSY20.py) to ensure having consistent standard Truth containers
if DerivationFrameworkHasTruth:
   from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents,addMiniTruthCollectionLinks,addHFAndDownstreamParticles,addPVCollection,addTausAndDownstreamParticles
   import DerivationFrameworkHiggs.TruthCategories
   # Add charm quark collection
   from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthCollectionMaker
   BPHY27TruthCharmTool = DerivationFramework__TruthCollectionMaker(name                  = "BPHY27TruthCharmTool",
                                                                  NewCollectionName       = "TruthCharm",
                                                                  KeepNavigationInfo      = False,
                                                                  ParticleSelectionString = "(abs(TruthParticles.pdgId) == 4)",
                                                                  Do_Compress             = True)
   ToolSvc += BPHY27TruthCharmTool
   from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__CommonAugmentation
   SeqBPHY27 += CfgMgr.DerivationFramework__CommonAugmentation("BPHY27TruthCharmKernel",AugmentationTools=[BPHY27TruthCharmTool])
   # Add HF particles
   addHFAndDownstreamParticles(SeqBPHY27)
   #Add custom tau collection with 2 generation below (To save photon information)
   addTausAndDownstreamParticles(SeqBPHY27, generations=2)
   # Add standard truth
   addStandardTruthContents(SeqBPHY27,prefix='')

   # Update to include charm quarks and HF particles - require a separate instance to be train safe
   from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthNavigationDecorator
   BPHY27TruthNavigationDecorator = DerivationFramework__TruthNavigationDecorator( name="BPHY27TruthNavigationDecorator",
                                                                                   InputCollections=["TruthElectrons", "TruthMuons", "TruthPhotons", "TruthTaus","TruthNeutrinos", "TruthBSM", "TruthBottom", "TruthTop", "TruthBoson","TruthCharm","TruthHFWithDecayParticles","TruthTauWithDecayParticles"])
   ToolSvc += BPHY27TruthNavigationDecorator
   SeqBPHY27.MCTruthNavigationDecoratorKernel.AugmentationTools = [BPHY27TruthNavigationDecorator]
   # Re-point links on reco objects
   addMiniTruthCollectionLinks(SeqBPHY27)
   addPVCollection(SeqBPHY27)
   # Set appropriate truth jet collection for tau truth matching
   ToolSvc.DFCommonTauTruthMatchingTool.TruthJetContainerName = "AntiKt4TruthDressedWZJets"
   # Add sumOfWeights metadata for LHE3 multiweights =======
   from DerivationFrameworkCore.LHE3WeightMetadata import *
'''

#====================================================================
# Slimming
#====================================================================
# Added by ASC
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY27SlimmingHelper = SlimmingHelper("BPHY27SlimmingHelper")
AllVariables  = []
StaticContent = []

# Needed for trigger objects
BPHY27SlimmingHelper.IncludeMuonTriggerContent  = TRUE
BPHY27SlimmingHelper.IncludeBPhysTriggerContent = TRUE

## primary vertices
AllVariables  += ["PrimaryVertices"]
StaticContent += ["xAOD::VertexContainer#BPHY27RefittedPrimaryVertices"]
StaticContent += ["xAOD::VertexAuxContainer#BPHY27RefittedPrimaryVerticesAux."]

## ID track particles
AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are store in InDetTrackParticles collection)
AllVariables += ["CombinedMuonTrackParticles"]
AllVariables += ["ExtrapolatedMuonTrackParticles"]

## muon container
AllVariables += ["Muons"]

## Jpsi candidates
StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY27MuPiSelectAndWrite.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY27MuPiSelectAndWrite.OutputVtxContainerName]


## K+K-, Kpi, D0/D0bar candidates
StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY27DiTrkSelectAndWrite.OutputVtxContainerName]
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY27DiTrkSelectAndWrite.OutputVtxContainerName]


## B+>mu D_(s)+/-, mu D*+/- and mu Lambda_c+/- candidates
for cascades in CascadeCollections:
   StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]

# Tagging information (in addition to that already requested by usual algorithms)
AllVariables += ["MuonSpectrometerTrackParticles" ]

# Added by ASC
# Truth information for MC only
if isSimulation:
    AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

AllVariables = list(set(AllVariables)) # remove duplicates

# MET
AllVariables += ["MET_Truth" ]

BPHY27SlimmingHelper.AllVariables = AllVariables
BPHY27SlimmingHelper.StaticContent = StaticContent
BPHY27SlimmingHelper.SmartCollections = ["MET_Reference_AntiKt4EMPFlow"]

# same Truth-related content as in PHYS.py to ensure having consistent standard Truth containers
if DerivationFrameworkHasTruth:

  BPHY27SlimmingHelper.AppendToDictionary = {'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                            'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                            'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
                                            'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTauWithDecayParticles':'xAOD::TruthParticleContainer','TruthTauWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTauWithDecayVertices':'xAOD::TruthVertexContainer','TruthTauWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'HardScatterVertices':'xAOD::TruthVertexContainer','HardScatterVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthHFWithDecayParticles':'xAOD::TruthParticleContainer','TruthHFWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthHFWithDecayVertices':'xAOD::TruthVertexContainer','TruthHFWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthCharm':'xAOD::TruthParticleContainer','TruthCharmAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
                                            'AntiKt10TruthSoftDropBeta100Zcut10Jets':'xAOD::JetContainer', 'AntiKt10TruthSoftDropBeta100Zcut10JetsAux':'xAOD::JetAuxContainer'
                                           }

  from DerivationFrameworkMCTruth.MCTruthCommon import addTruth3ContentToSlimmerTool
  addTruth3ContentToSlimmerTool(BPHY27SlimmingHelper)
  BPHY27SlimmingHelper.AllVariables += ['TruthTauWithDecayParticles','TruthTauWithDecayVertices','TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm']

BPHY27SlimmingHelper.AppendContentToStream(BPHY27Stream)


#====================================================================
# END OF BPHY27.py
#====================================================================
