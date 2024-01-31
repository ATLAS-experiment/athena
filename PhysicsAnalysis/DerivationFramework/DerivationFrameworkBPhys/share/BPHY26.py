#====================================================================
# BPHY26.py (Based on BPHY8, BPHY16, and BPHY13)
# Contact: yue.xu@cern.ch
#====================================================================

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkHasTruth
isSimulation = DerivationFrameworkHasTruth

#====================================================================
# AUGMENTATION TOOLS 
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY26_VertexTools = BPHYVertexTools("BPHY26")

Phimass = 1019.461
Jpsimass = 3096.916
Psi2Smass = 3686.10
Upsimass = 9460.30
Upsi2Smass = 10023.26

#--------------------------------------------------------------------
## 2/ Setup the vertex fitter tools (e.g. JpsiFinder, JpsiPlus1Track, etc).
##    These are general tools independent of DerivationFramework that do the 
##    actual vertex fitting and some pre-selection.
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY26JpsiFinder = Analysis__JpsiFinder( # phi, psi, upsilon
    name                        = "BPHY26JpsiFinder",
    OutputLevel                 = INFO,
    muAndMu                     = True,
    muAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt            = 2500.,
    invMassUpper                = 10000.0,
    invMassLower                = 770.,
    Chi2Cut                     = 50.,
    oppChargesOnly	            = True,
    atLeastOneComb              = True,
    useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
    muonCollectionKey           = "Muons",
    TrackParticleCollection     = "InDetTrackParticles",
    V0VertexFitterTool          = BPHY26_VertexTools.TrkV0Fitter, # V0 vertex fitter
    useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool         = BPHY26_VertexTools.TrkVKalVrtFitter, # VKalVrt vertex fitter
    TrackSelectorTool           = BPHY26_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool  = BPHY26_VertexTools.InDetConversionHelper,
    VertexPointEstimator        = BPHY26_VertexTools.VtxPointEstimator,
    useMCPCuts                  = False )

ToolSvc += BPHY26JpsiFinder

#--------------------------------------------------------------------
## 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool if the JpsiFinder mass window is wide enough.

# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/Reco_mumu.cxx
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY26_Reco_mumu = DerivationFramework__Reco_mumu(
    name                   = "BPHY26_Reco_mumu",
    JpsiFinder             = BPHY26JpsiFinder,
    OutputVtxContainerName = "BPHY26OniaCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
#    RefPVContainerName     = "BPHY26RefittedPrimaryVertices",
#    RefitPV                = True,
#    MaxPVrefit             = 10000,
#https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/BPhysPVTools.cxx#L259
# bit pattern: doZ0BA|doZ0|doA0|doPt
    DoVertexType           = 1)
  
ToolSvc += BPHY26_Reco_mumu

## 4/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BPHY26VertexFit = Trk__TrkVKalVrtFitter(
    name                = "BPHY26VertexFit",
    Extrapolator        = BPHY26_VertexTools.InDetExtrapolator,
#    FirstMeasuredPoint  = True,
    FirstMeasuredPoint  = False,
    MakeExtendedVertex  = True)
ToolSvc += BPHY26VertexFit

## 5/ setup the Jpsi+2 track finder
# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/JpsiUpsilonTools/src/JpsiPlus1Track.cxx
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track
BPHY26Plus1Track = Analysis__JpsiPlus1Track(
    name                                = "BPHY26Plus1Track",
    pionHypothesis                      = True,
    kaonHypothesis                      = False,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = 770,
    JpsiMassUpper                       = 10000,
    TrkTrippletMassLower                = 40000., # 50000.
    TrkTrippletMassUpper                = 150000.,
    Chi2Cut                             = 30.0,
    JpsiContainerKey                    = "BPHY26OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY26VertexFit,
    TrackSelectorTool                   = BPHY26_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY26Plus1Track

## 6/ setup the combined augmentation/skimming tool
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrk
BPHY26ThreeTrackSelectAndWrite = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY26ThreeTrackSelectAndWrite",
    Jpsi1PlusTrackName       = BPHY26Plus1Track,
    OutputVtxContainerName   = "BPHY26ThreeTrack",
    PVContainerName          = "PrimaryVertices",
    RefPVContainerName       = "BPHY26RefittedPrimaryVertices",
    RefitPV                  = True,
    MaxPVrefit               = 20,
    DoVertexType             = 7)

ToolSvc += BPHY26ThreeTrackSelectAndWrite 


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu

BPHY26_Select_ThreeTrack      = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY26_Select_ThreeTrack",
    HypothesisName             = "ThreeTracks",
    InputVtxContainerName      = "BPHY26ThreeTrack",
    TrkMasses                  = [105.658, 105.658, 139.570],
    VtxMassHypo                = 80377., # for decay time
    MassMin                    = 40000., # 50000.
    MassMax                    = 150000.,
    Chi2Max                    = 30.)

ToolSvc += BPHY26_Select_ThreeTrack


#====================================================================
# Isolation
#====================================================================

#Track isolation for candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__VertexTrackIsolation
BPHY26TrackIsolationDecorator = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY26TrackIsolationDecorator",
  OutputLevel                     = INFO,
  TrackIsoTool                    = "xAOD::TrackIsolationTool",
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY26ThreeTrack",
  PassFlags                       = ["passed_ThreeTracks"],
  DoIsoPerTrk                     = True,
  RemoveDuplicate                 = 2
)

ToolSvc += BPHY26TrackIsolationDecorator


#====================================================================
# Revertex with mass constraint
#====================================================================

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
## can try "UseVertexFittingWithPV" to see if there is a difference on W resolution
BPHY26_Revertex_phipi          = DerivationFramework__ReVertex(
    name                       = "BPHY26_Revertex_phipi",
    InputVtxContainerName      = "BPHY26ThreeTrack",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ], # Track indices start from 1 (not 0)! (https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtCore/src/PrCFit.cxx#L170)
    RefitPV                    = True,
    RefPVContainerName         = "BPHY26RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = False,
    SubVertexMass              = 1019.461,
    MassInputParticles         = [105.658, 105.658, 139.570],
    TrkVertexFitterTool        = BPHY26VertexFit,
    OutputVtxContainerName     = "BPHY26Revtx_phipi")

ToolSvc += BPHY26_Revertex_phipi

BPHY26_Select_phipi          = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY26_Select_phipi",
    HypothesisName             = "phipi",
    InputVtxContainerName      = "BPHY26Revtx_phipi",
    TrkMasses                  = [105.658, 105.658, 139.570],
    VtxMassHypo                = 80377.,
    MassMin                    = 40000., # 50000.
    MassMax                    = 150000,
    Chi2Max                    = 30)

ToolSvc += BPHY26_Select_phipi


BPHY26_Revertex_Jpsipi          = DerivationFramework__ReVertex(
    name                       = "BPHY26_Revertex_Jpsipi",
    InputVtxContainerName      = "BPHY26ThreeTrack",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY26RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    SubVertexMass              = 3096.916,
    MassInputParticles         = [105.658, 105.658, 139.570],
    TrkVertexFitterTool        = BPHY26VertexFit,
    OutputVtxContainerName     = "BPHY26Revtx_Jpsipi")

ToolSvc += BPHY26_Revertex_Jpsipi

BPHY26_Select_Jpsipi          = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY26_Select_Jpsipi",
    HypothesisName             = "Jpsipi",
    InputVtxContainerName      = "BPHY26Revtx_Jpsipi",
    TrkMasses                  = [105.658, 105.658, 139.570],
    VtxMassHypo                = 80377.,
    MassMin                    = 40000., #50000.
    MassMax                    = 150000,
    Chi2Max                    = 30)

ToolSvc += BPHY26_Select_Jpsipi

BPHY26_Revertex_Upsipi          = DerivationFramework__ReVertex(
    name                       = "BPHY26_Revertex_Upsipi",
    InputVtxContainerName      = "BPHY26ThreeTrack",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY26RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    SubVertexMass              = 9460.30,
    MassInputParticles         = [105.658, 105.658, 139.570],
    TrkVertexFitterTool        = BPHY26VertexFit,
    OutputVtxContainerName     = "BPHY26Revtx_Upsipi")

ToolSvc += BPHY26_Revertex_Upsipi

BPHY26_Select_Upsipi          = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY26_Select_Upsipi",
    HypothesisName             = "Upsipi",
    InputVtxContainerName      = "BPHY26Revtx_Upsipi",
    TrkMasses                  = [105.658, 105.658, 139.570],
    VtxMassHypo                = 80377.,
    MassMin                    = 40000., # 50000.
    MassMax                    = 150000,
    Chi2Max                    = 30)

ToolSvc += BPHY26_Select_Upsipi



#--------------------------------------------------------------------
## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
##
##       "ContainerName.passed_HypoName > count"
##
##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
##    tool and "count" is the number of candidates passing the selection you want to keep. 

expression = "( count(BPHY26ThreeTrack.passed_ThreeTracks) > 0 && (count(BPHY26Revtx_phipi.passed_phipi) + count(BPHY26Revtx_Jpsipi.passed_Jpsipi) + count(BPHY26Revtx_Upsipi.passed_Upsipi) ) > 0)"


from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY26_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name = "BPHY26_SelectEvent", expression = expression)

ToolSvc += BPHY26_SelectEvent

#--------------------------------------------------------------------
## 8/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which hasn't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the 
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with 
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept is it 
##    satisfy any of the listed selections.


#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
## 9/ IMPORTANT bit. Don't forget to pass the tools to the DerivationKernel! If you don't do that, they will not be 
##    be executed!


# The name of the kernel (BPHY26Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentation_tools = [BPHY26_Reco_mumu, BPHY26ThreeTrackSelectAndWrite, BPHY26_Select_ThreeTrack]
augmentation_tools += [BPHY26TrackIsolationDecorator, BPHY26_Revertex_phipi, BPHY26_Select_phipi, BPHY26_Revertex_Jpsipi, BPHY26_Select_Jpsipi, BPHY26_Revertex_Upsipi, BPHY26_Select_Upsipi]

DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY26Kernel",
    AugmentationTools = augmentation_tools,
    SkimmingTools     = [BPHY26_SelectEvent]
)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_BPHY26Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_BPHY26Stream )
BPHY26Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY26Stream.AcceptAlgs(["BPHY26Kernel"])
# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="BPHY26ThinningSvc", outStreams=[evtStream] )


#====================================================================
# Slimming 
#====================================================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY26SlimmingHelper = SlimmingHelper("BPHY26SlimmingHelper")
BPHY26_AllVariables = []
BPHY26_StaticContent = []

# Needed for trigger objects
BPHY26SlimmingHelper.IncludeMuonTriggerContent = True
BPHY26SlimmingHelper.IncludeBPhysTriggerContent = True

## primary vertices
BPHY26_AllVariables += ["PrimaryVertices"]
#BPHY26_StaticContent += ["xAOD::VertexContainer#BPHY26RefittedPrimaryVertices"]
#BPHY26_StaticContent += ["xAOD::VertexAuxContainer#BPHY26RefittedPrimaryVerticesAux."]

## ID track particles
BPHY26_AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are store in InDetTrackParticles collection)
BPHY26_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

## muon container
BPHY26_AllVariables += ["Muons", "MuonSegments"]

BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26ThreeTrackSelectAndWrite.OutputVtxContainerName]
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY26ThreeTrackSelectAndWrite.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26ThreeTrackSelectAndWrite.OutputVtxContainerName]

BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_phipi.OutputVtxContainerName]
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY26_Revertex_phipi.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_phipi.OutputVtxContainerName]

BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_Jpsipi.OutputVtxContainerName]
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY26_Revertex_Jpsipi.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_Jpsipi.OutputVtxContainerName]

BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_Upsipi.OutputVtxContainerName]
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY26_Revertex_Upsipi.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_Upsipi.OutputVtxContainerName]


# Truth information for MC only
if isSimulation:
    BPHY26_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

BPHY26SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
BPHY26SlimmingHelper.AllVariables = BPHY26_AllVariables
BPHY26SlimmingHelper.StaticContent = BPHY26_StaticContent
BPHY26SlimmingHelper.AppendContentToStream(BPHY26Stream)
