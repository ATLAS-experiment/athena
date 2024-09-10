#====================================================================
# BPHY23.py
# Contact: xin.chen@cern.ch
# In the following, Meson (Onium) refers to charged (neutral) hadrons
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
BPHY23_VertexTools = BPHYVertexTools("BPHY23")

# mass bounds and constants used in the following
X_hi = 141000.0

Jpsi_lo = 2600.0
Jpsi_hi = 3500.0
Zc_lo = 3650.0
Zc_hi = 4150.0
Psi_lo = 3350.0
Psi_hi = 4200.0
B_lo = 4850.0
B_hi = 5700.0
Kstar_lo = 640.0
Kstar_hi = 1140.0
Bs0_lo = 4950.0
Bs0_hi = 5800.0
Phi_lo = 770.0
Phi_hi = 1270.0
Upsi_lo = 8900.0
Upsi_hi = 9900.0
Upsi2S_lo = 9550.0
Upsi2S_hi = 10500.0
Ds_lo = 1660.0
Ds_hi = 2230.0

Mumass = 105.658
Pimass = 139.570
Kmass = 493.677
Kstarmass = 895.55
Phimass = 1019.461
Dpmmass = 1869.66
Dspmmass = 1968.35
etacmass = 2983.9
chic0mass = 3414.71
chic1mass = 3510.67
Jpsimass = 3096.916
Psi2Smass = 3686.10
X3872mass = 3871.65
Zcmass = 3887.1
Bpmmass = 5279.34
B0mass = 5279.66
Bs0mass = 5366.92
Upsimass = 9460.30
Upsi2Smass = 10023.26

#--------------------------------------------------------------------
## 2/ Setup the vertex fitter tools (e.g. JpsiFinder, JpsiPlus1Track, etc).
##    These are general tools independent of DerivationFramework that do the 
##    actual vertex fitting and some pre-selection.

# invMass range covers phi, J/psi, psi(2S), Upsi(1S) and Upsi(2S)
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY23JpsiFinder = Analysis__JpsiFinder(
    name                        = "BPHY23JpsiFinder",
    OutputLevel                 = INFO,
    muAndMu                     = True,
    muAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt            = 2400.,
    invMassLower                = Jpsi_lo,
    invMassUpper                = Upsi_hi,
    Chi2Cut                     = 10.,
    oppChargesOnly	        = True,
    atLeastOneComb              = True,
    useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
    muonCollectionKey           = "Muons",
    TrackParticleCollection     = "InDetTrackParticles",
    V0VertexFitterTool          = BPHY23_VertexTools.TrkV0Fitter, # V0 vertex fitter
    useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool         = BPHY23_VertexTools.TrkVKalVrtFitter, # VKalVrt vertex fitter
    TrackSelectorTool           = BPHY23_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool  = BPHY23_VertexTools.InDetConversionHelper,
    VertexPointEstimator        = BPHY23_VertexTools.VtxPointEstimator,
    useMCPCuts                  = False )
ToolSvc += BPHY23JpsiFinder

#--------------------------------------------------------------------
## 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool if the JpsiFinder mass window is wide enough.

# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/Reco_mumu.cxx
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY23_Reco_mumu = DerivationFramework__Reco_mumu(
    name                   = "BPHY23_Reco_mumu",
    JpsiFinder             = BPHY23JpsiFinder,
    OutputVtxContainerName = "BPHY23OniaCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 1)
ToolSvc += BPHY23_Reco_mumu

## 4/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BPHY23VertexFit = Trk__TrkVKalVrtFitter(
    name                = "BPHY23VertexFit",
    Extrapolator        = BPHY23_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint  = False,  # use Perigee strategy
    MakeExtendedVertex  = True)
ToolSvc += BPHY23VertexFit

## 5/ setup the Jpsi+2 track finder
# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/JpsiUpsilonTools/src/JpsiPlus2Tracks.cxx
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus2Tracks

# X(3872), Psi(2S) -> J/psi + pi pi
BPHY23PsiX3872_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23PsiX3872_Jpsi2Trk",
    kaonkaonHypothesis		        = False,
    pionpionHypothesis                  = True,
    kaonpionHypothesis                  = False,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 480.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkQuadrupletMassLower              = Psi_lo,
    TrkQuadrupletMassUpper              = Psi_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23PsiX3872_Jpsi2Trk

## 6/ setup the combined augmentation/skimming tool
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrkTrk
BPHY23FourTrackReco_PsiX3872 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_PsiX3872",
    Jpsi2PlusTrackName       = BPHY23PsiX3872_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY23FourTrack_PsiX3872",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23FourTrackReco_PsiX3872

# revertex with mass constraints to reduce combinatorics
# Psi(2S) -> J/psi pi pi
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
BPHY23Rev_Psi4Body = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Psi4Body",
    InputVtxContainerName      = "BPHY23FourTrack_PsiX3872",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 25.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Psi4Body")
ToolSvc += BPHY23Rev_Psi4Body


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY23Select_Jpsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Jpsi",
    HypothesisName             = "Jpsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Jpsi_lo,
    MassMax                    = Jpsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Jpsi

BPHY23Select_Psi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Psi",
    HypothesisName             = "Psi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Psi_lo,
    MassMax                    = Psi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Psi

BPHY23Select_Upsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Upsi",
    HypothesisName             = "Upsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi_lo,
    MassMax                    = Upsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Upsi

BPHY23Rev_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Jpsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    HypothesisNames            = [ "Jpsi" ],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 50.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Jpsi")
ToolSvc += BPHY23Rev_Jpsi

BPHY23Rev_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Psi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    HypothesisNames            = [ "Psi" ],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 50.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Psi")
ToolSvc += BPHY23Rev_Psi

BPHY23Rev_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Upsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    HypothesisNames            = [ "Upsi" ],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 50.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Upsi")
ToolSvc += BPHY23Rev_Upsi


########################
###  2 trks + 0 trk  ###
########################

list_2trk0trk_hypo = ["Psi2Jpsi0", "Psi2Psi0"]
list_2trk0trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body"]
list_2trk0trk_psi2Input = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi"]
list_2trk0trk_jpsi2lo = [Jpsi_lo, Psi_lo]
list_2trk0trk_jpsi2hi = [Jpsi_hi, Psi_hi]
list_2trk0trk_jpsi1mass = [Jpsimass, Jpsimass]
list_2trk0trk_psi1mass = [Psi2Smass, Psi2Smass]
list_2trk0trk_jpsi2mass = [Jpsimass, Psi2Smass]

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__PsiPlusPsiSingleVertex

list_2trk0trk_obj = []
for hypo in list_2trk0trk_hypo:
    list_2trk0trk_obj.append( DerivationFramework__PsiPlusPsiSingleVertex("BPHY23_"+hypo) )

ToolSvc += list_2trk0trk_obj

for i in range(len(list_2trk0trk_obj)):
    list_2trk0trk_obj[i].HypothesisName           = list_2trk0trk_hypo[i]
    list_2trk0trk_obj[i].Psi1Vertices             = list_2trk0trk_psi1Input[i]
    list_2trk0trk_obj[i].Psi2Vertices             = list_2trk0trk_psi2Input[i]
    list_2trk0trk_obj[i].MaxCandidates            = 15
    list_2trk0trk_obj[i].NumberOfPsi1Daughters    = 4
    list_2trk0trk_obj[i].NumberOfPsi2Daughters    = 2
    list_2trk0trk_obj[i].Jpsi1MassLowerCut        = Jpsi_lo
    list_2trk0trk_obj[i].Jpsi1MassUpperCut        = Jpsi_hi
    list_2trk0trk_obj[i].Psi1MassLowerCut         = Psi_lo
    list_2trk0trk_obj[i].Psi1MassUpperCut         = Psi_hi
    list_2trk0trk_obj[i].Jpsi2MassLowerCut        = list_2trk0trk_jpsi2lo[i]
    list_2trk0trk_obj[i].Jpsi2MassUpperCut        = list_2trk0trk_jpsi2hi[i]
    list_2trk0trk_obj[i].MassLowerCut             = 0.
    list_2trk0trk_obj[i].MassUpperCut             = X_hi
    list_2trk0trk_obj[i].Jpsi1Mass                = list_2trk0trk_jpsi1mass[i]
    list_2trk0trk_obj[i].Psi1Mass                 = list_2trk0trk_psi1mass[i]
    list_2trk0trk_obj[i].Jpsi2Mass                = list_2trk0trk_jpsi2mass[i]
    list_2trk0trk_obj[i].ApplyJpsi1MassConstraint = True
    list_2trk0trk_obj[i].ApplyPsi1MassConstraint  = True
    list_2trk0trk_obj[i].ApplyJpsi2MassConstraint = True
    list_2trk0trk_obj[i].Chi2Cut                  = 30.
    list_2trk0trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_2trk0trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk0trk_obj[i].OutputVertexCollections  = ["BPHY23_"+list_2trk0trk_hypo[i]+"_SubVtx1","BPHY23_"+list_2trk0trk_hypo[i]+"_SubVtx2","BPHY23_"+list_2trk0trk_hypo[i]+"_MainVtx"]
    list_2trk0trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk0trk_obj[i].RefPVContainerName       = "BPHY23_"+list_2trk0trk_hypo[i]+"_RefPrimaryVertices"
    list_2trk0trk_obj[i].RefitPV                  = True
    list_2trk0trk_obj[i].MaxnPV                   = 100

#########################
###  2 trks + 2 trks  ###
#########################

list_2trk2trk_hypo = ["Psi2Psi2"]
list_2trk2trk_psi1Input = ["BPHY23Revtx_Psi4Body"]
list_2trk2trk_psi2Input = ["BPHY23Revtx_Psi4Body"]
list_2trk2trk_jpsi1mass = [Jpsimass]
list_2trk2trk_psi1mass = [Psi2Smass]
list_2trk2trk_jpsi2mass = [Jpsimass]
list_2trk2trk_psi2mass = [Psi2Smass]

list_2trk2trk_obj = []
for hypo in list_2trk2trk_hypo:
   list_2trk2trk_obj.append( DerivationFramework__PsiPlusPsiSingleVertex("BPHY23_"+hypo) )

ToolSvc += list_2trk2trk_obj

for i in range(len(list_2trk2trk_obj)):
    list_2trk2trk_obj[i].HypothesisName           = list_2trk2trk_hypo[i]
    list_2trk2trk_obj[i].Psi1Vertices             = list_2trk2trk_psi1Input[i]
    list_2trk2trk_obj[i].Psi2Vertices             = list_2trk2trk_psi2Input[i]
    list_2trk2trk_obj[i].MaxCandidates            = 15
    list_2trk2trk_obj[i].NumberOfPsi1Daughters    = 4
    list_2trk2trk_obj[i].NumberOfPsi2Daughters    = 4
    list_2trk2trk_obj[i].Jpsi1MassLowerCut        = Jpsi_lo
    list_2trk2trk_obj[i].Jpsi1MassUpperCut        = Jpsi_hi
    list_2trk2trk_obj[i].Psi1MassLowerCut         = Psi_lo
    list_2trk2trk_obj[i].Psi1MassUpperCut         = Psi_hi
    list_2trk2trk_obj[i].Jpsi2MassLowerCut        = Jpsi_lo
    list_2trk2trk_obj[i].Jpsi2MassUpperCut        = Jpsi_hi
    list_2trk2trk_obj[i].Psi2MassLowerCut         = Psi_lo
    list_2trk2trk_obj[i].Psi2MassUpperCut         = Psi_hi
    list_2trk2trk_obj[i].MassLowerCut             = 0.
    list_2trk2trk_obj[i].MassUpperCut             = X_hi
    list_2trk2trk_obj[i].Jpsi1Mass                = list_2trk2trk_jpsi1mass[i]
    list_2trk2trk_obj[i].Psi1Mass                 = list_2trk2trk_psi1mass[i]
    list_2trk2trk_obj[i].Jpsi2Mass                = list_2trk2trk_jpsi2mass[i]
    list_2trk2trk_obj[i].Psi2Mass                 = list_2trk2trk_psi2mass[i]
    list_2trk2trk_obj[i].ApplyJpsi1MassConstraint = True
    list_2trk2trk_obj[i].ApplyPsi1MassConstraint  = True
    list_2trk2trk_obj[i].ApplyJpsi2MassConstraint = True
    list_2trk2trk_obj[i].ApplyPsi2MassConstraint  = True
    list_2trk2trk_obj[i].Chi2Cut                  = 30.
    list_2trk2trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_2trk2trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk2trk_obj[i].OutputVertexCollections  = ["BPHY23_"+list_2trk2trk_hypo[i]+"_SubVtx1","BPHY23_"+list_2trk2trk_hypo[i]+"_SubVtx2","BPHY23_"+list_2trk2trk_hypo[i]+"_MainVtx"]
    list_2trk2trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk2trk_obj[i].RefPVContainerName       = "BPHY23_"+list_2trk2trk_hypo[i]+"_RefPrimaryVertices"
    list_2trk2trk_obj[i].RefitPV                  = True
    list_2trk2trk_obj[i].MaxnPV                   = 100

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusEtacSingleVertex
BPHY23_JpsiEtac4T = DerivationFramework__JpsiPlusEtacSingleVertex(
    name                     = "BPHY23_JpsiEtac4T",
    HypothesisName           = "JpsiEtac4T",
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = [ "Jpsi" ],
    JpsiMassLowerCut         = 2600.0,
    JpsiMassUpperCut         = 3500.0,
    NumberOfEtacDaughters    = 4,
    TrackMinPtTrk1           = 1850.,
    TrackMinPtTrk2           = 1550.,
    TrackMinPtTrk3           = 1250.,
    TrackMinPtTrk4           = 1050.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    Vtx1Daug2MassHypo        = Pimass,
    Vtx2Daug1MassHypo        = Pimass,
    Vtx2Daug2MassHypo        = Pimass,
    Rho1MassLowerCut         = 0.,
    Rho1MassUpperCut         = 3300.,
    Rho2MassLowerCut         = 0.,
    Rho2MassUpperCut         = 3300.,
    EtacMassLowerCut         = 2300.,
    EtacMassUpperCut         = 4400.,
    MassLowerCut             = 0.0,
    MassUpperCut             = 31000.0,
    MaxDR                    = 0.6,
    MaxRhoCandidates         = 200,
    MaxEtacCandidates        = 1000,
    JpsiMass                 = 3096.916,
    ApplyJpsiMassConstraint  = True,
    Chi2CutJpsi              = 4.,
    Chi2CutRho               = 3.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiEtac4TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiEtac4T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiEtac4T

BPHY23_JpsiEtac6T = DerivationFramework__JpsiPlusEtacSingleVertex(
    name                     = "BPHY23_JpsiEtac6T",
    HypothesisName           = "JpsiEtac6T",
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = [ "Jpsi" ],
    JpsiMassLowerCut         = 2600.0,
    JpsiMassUpperCut         = 3500.0,
    NumberOfEtacDaughters    = 6,
    TrackMinPtTrk1           = 1750.,
    TrackMinPtTrk2           = 1550.,
    TrackMinPtTrk3           = 1350.,
    TrackMinPtTrk4           = 1150.,
    TrackMinPtTrk5           = 950.,
    TrackMinPtTrk6           = 850.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    Vtx1Daug2MassHypo        = Pimass,
    Vtx2Daug1MassHypo        = Pimass,
    Vtx2Daug2MassHypo        = Pimass,
    Vtx3Daug1MassHypo        = Pimass,
    Vtx3Daug2MassHypo        = Pimass,
    Rho1MassLowerCut         = 0.,
    Rho1MassUpperCut         = 3000.,
    Rho2MassLowerCut         = 0.,
    Rho2MassUpperCut         = 3000.,
    Rho3MassLowerCut         = 0.,
    Rho3MassUpperCut         = 3000.,
    EtacMassLowerCut         = 2300.,
    EtacMassUpperCut         = 4400.,
    MassLowerCut             = 0.0,
    MassUpperCut             = 31000.0,
    MaxDR                    = 0.6,
    MaxRhoCandidates         = 100,
    MaxEtacCandidates        = 1000,
    JpsiMass                 = 3096.916,
    ApplyJpsiMassConstraint  = True,
    Chi2CutJpsi              = 4.,
    Chi2CutRho               = 3.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiEtac6TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiEtac6T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiEtac6T

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__DiJpsiPlusTracksSingleVertex
BPHY23_JpsiJpsi1T = DerivationFramework__DiJpsiPlusTracksSingleVertex(
    name                     = "BPHY23_JpsiJpsi1T",
    HypothesisName           = "JpsiJpsi1T",
    Jpsi1Vertices            = "BPHY23OniaCandidates",
    Jpsi2Vertices            = "BPHY23OniaCandidates",
    Jpsi1VtxHypoNames        = [ "Jpsi" ],
    Jpsi2VtxHypoNames        = [ "Jpsi" ],
    Jpsi1MassLowerCut        = 2600.0,
    Jpsi1MassUpperCut        = 3500.0,
    Jpsi2MassLowerCut        = 2600.0,
    Jpsi2MassUpperCut        = 3500.0,
    NumberOfTracks           = 1,
    TrackMinPtTrk1           = 480.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx0Daug3MassHypo        = Mumass,
    Vtx0Daug4MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    MassLowerCut             = 0.0,
    MassUpperCut             = 31000.0,
    MaxDR                    = 0.6,
    MaxEtacCandidates        = 1000,
    Jpsi1Mass                = 3096.916,
    Jpsi2Mass                = 3096.916,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Chi2CutJpsi1             = 4.,
    Chi2CutJpsi2             = 4.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiJpsi1TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiJpsi1T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiJpsi1T

BPHY23_JpsiJpsi2T = DerivationFramework__DiJpsiPlusTracksSingleVertex(
    name                     = "BPHY23_JpsiJpsi2T",
    HypothesisName           = "JpsiJpsi2T",
    Jpsi1Vertices            = "BPHY23OniaCandidates",
    Jpsi2Vertices            = "BPHY23OniaCandidates",
    Jpsi1VtxHypoNames        = [ "Jpsi" ],
    Jpsi2VtxHypoNames        = [ "Jpsi" ],
    Jpsi1MassLowerCut        = 2600.0,
    Jpsi1MassUpperCut        = 3500.0,
    Jpsi2MassLowerCut        = 2600.0,
    Jpsi2MassUpperCut        = 3500.0,
    NumberOfTracks           = 2,
    TrackMinPtTrk1           = 550.,
    TrackMinPtTrk2           = 480.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx0Daug3MassHypo        = Mumass,
    Vtx0Daug4MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    Vtx1Daug2MassHypo        = Pimass,
    Rho1MassLowerCut         = 0.,
    Rho1MassUpperCut         = 4000.,
    MassLowerCut             = 0.0,
    MassUpperCut             = 31000.0,
    MaxDR                    = 0.6,
    MaxRhoCandidates         = 5000,
    MaxEtacCandidates        = 1000,
    Jpsi1Mass                = 3096.916,
    Jpsi2Mass                = 3096.916,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Chi2CutJpsi1             = 4.,
    Chi2CutJpsi2             = 4.,
    Chi2CutRho               = 3.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiJpsi2TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiJpsi2T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiJpsi2T

BPHY23_JpsiJpsi4T = DerivationFramework__DiJpsiPlusTracksSingleVertex(
    name                     = "BPHY23_JpsiJpsi4T",
    HypothesisName           = "JpsiJpsi4T",
    Jpsi1Vertices            = "BPHY23OniaCandidates",
    Jpsi2Vertices            = "BPHY23OniaCandidates",
    Jpsi1VtxHypoNames        = [ "Jpsi" ],
    Jpsi2VtxHypoNames        = [ "Jpsi" ],
    Jpsi1MassLowerCut        = 2600.0,
    Jpsi1MassUpperCut        = 3500.0,
    Jpsi2MassLowerCut        = 2600.0,
    Jpsi2MassUpperCut        = 3500.0,
    NumberOfTracks           = 4,
    TrackMinPtTrk1           = 950.,
    TrackMinPtTrk2           = 800.,
    TrackMinPtTrk3           = 650.,
    TrackMinPtTrk4           = 480.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx0Daug3MassHypo        = Mumass,
    Vtx0Daug4MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    Vtx1Daug2MassHypo        = Pimass,
    Vtx2Daug1MassHypo        = Pimass,
    Vtx2Daug2MassHypo        = Pimass,
    Rho1MassLowerCut         = 0.,
    Rho1MassUpperCut         = 3300.,
    Rho2MassLowerCut         = 0.,
    Rho2MassUpperCut         = 3300.,
    EtacMassLowerCut         = 2300.,
    EtacMassUpperCut         = 4400.,
    MassLowerCut             = 0.0,
    MassUpperCut             = 99000.0,
    MaxDR                    = 1.0,
    MaxRhoCandidates         = 200,
    MaxEtacCandidates        = 1000,
    Jpsi1Mass                = 3096.916,
    Jpsi2Mass                = 3096.916,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Chi2CutJpsi1             = 4.,
    Chi2CutJpsi2             = 4.,
    Chi2CutRho               = 3.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiJpsi4TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiJpsi4T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiJpsi4T

BPHY23_JpsiJpsi6T = DerivationFramework__DiJpsiPlusTracksSingleVertex(
    name                     = "BPHY23_JpsiJpsi6T",
    HypothesisName           = "JpsiJpsi6T",
    Jpsi1Vertices            = "BPHY23OniaCandidates",
    Jpsi2Vertices            = "BPHY23OniaCandidates",
    Jpsi1VtxHypoNames        = [ "Jpsi" ],
    Jpsi2VtxHypoNames        = [ "Jpsi" ],
    Jpsi1MassLowerCut        = 2600.0,
    Jpsi1MassUpperCut        = 3500.0,
    Jpsi2MassLowerCut        = 2600.0,
    Jpsi2MassUpperCut        = 3500.0,
    NumberOfTracks           = 6,
    TrackMinPtTrk1           = 1250.,
    TrackMinPtTrk2           = 1100.,
    TrackMinPtTrk3           = 950.,
    TrackMinPtTrk4           = 800.,
    TrackMinPtTrk5           = 650.,
    TrackMinPtTrk6           = 480.,
    Vtx0Daug1MassHypo        = Mumass,
    Vtx0Daug2MassHypo        = Mumass,
    Vtx0Daug3MassHypo        = Mumass,
    Vtx0Daug4MassHypo        = Mumass,
    Vtx1Daug1MassHypo        = Pimass,
    Vtx1Daug2MassHypo        = Pimass,
    Vtx2Daug1MassHypo        = Pimass,
    Vtx2Daug2MassHypo        = Pimass,
    Vtx3Daug1MassHypo        = Pimass,
    Vtx3Daug2MassHypo        = Pimass,
    Rho1MassLowerCut         = 0.,
    Rho1MassUpperCut         = 3000.,
    Rho2MassLowerCut         = 0.,
    Rho2MassUpperCut         = 3000.,
    Rho3MassLowerCut         = 0.,
    Rho3MassUpperCut         = 3000.,
    EtacMassLowerCut         = 2300.,
    EtacMassUpperCut         = 4400.,
    MassLowerCut             = 0.0,
    MassUpperCut             = 99000.0,
    MaxDR                    = 1.0,
    MaxRhoCandidates         = 100,
    MaxEtacCandidates        = 1000,
    Jpsi1Mass                = 3096.916,
    Jpsi2Mass                = 3096.916,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Chi2CutJpsi1             = 4.,
    Chi2CutJpsi2             = 4.,
    Chi2CutRho               = 3.,
    Chi2Cut                  = 4.,
    TrkVertexFitterTool      = BPHY23_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool        = BPHY23_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator     = BPHY23_VertexTools.VtxPointEstimator,
    OutputVertexCollection   = "BPHY23_JpsiJpsi6TVertices",
    TrackContainerName       = "InDetTrackParticles",
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23_JpsiJpsi6T_RefPrimaryVertices",
    RefitPV                  = True,
    MaxnPV                   = 100,
    DoVertexType             = 7)
ToolSvc += BPHY23_JpsiJpsi6T


BPHY23Rev_JpsiEtac4T_etac = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac4T_etac",
    InputVtxContainerName      = "BPHY23_JpsiEtac4TVertices",
    TrackIndices               = [ 2, 3, 4, 5 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = etacmass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac4T_etac",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac4T_etac

BPHY23Rev_JpsiEtac4T_jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac4T_jpsi",
    InputVtxContainerName      = "BPHY23_JpsiEtac4TVertices",
    TrackIndices               = [ 2, 3, 4, 5 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac4T_jpsi",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac4T_jpsi

BPHY23Rev_JpsiEtac4T_chic0 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac4T_chic0",
    InputVtxContainerName      = "BPHY23_JpsiEtac4TVertices",
    TrackIndices               = [ 2, 3, 4, 5 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic0mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac4T_chic0",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac4T_chic0

BPHY23Rev_JpsiEtac4T_chic1 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac4T_chic1",
    InputVtxContainerName      = "BPHY23_JpsiEtac4TVertices",
    TrackIndices               = [ 2, 3, 4, 5 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic1mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac4T_chic1",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac4T_chic1


BPHY23Rev_JpsiEtac6T_etac = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac6T_etac",
    InputVtxContainerName      = "BPHY23_JpsiEtac6TVertices",
    TrackIndices               = [ 2, 3, 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = etacmass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac6T_etac",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac6T_etac

BPHY23Rev_JpsiEtac6T_jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac6T_jpsi",
    InputVtxContainerName      = "BPHY23_JpsiEtac6TVertices",
    TrackIndices               = [ 2, 3, 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac6T_jpsi",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac6T_jpsi

BPHY23Rev_JpsiEtac6T_chic0 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac6T_chic0",
    InputVtxContainerName      = "BPHY23_JpsiEtac6TVertices",
    TrackIndices               = [ 2, 3, 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic0mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac6T_chic0",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac6T_chic0

BPHY23Rev_JpsiEtac6T_chic1 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiEtac6T_chic1",
    InputVtxContainerName      = "BPHY23_JpsiEtac6TVertices",
    TrackIndices               = [ 2, 3, 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiEtac6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic1mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiEtac6T_chic1",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiEtac6T_chic1


BPHY23Rev_JpsiJpsi4T_etac = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi4T_etac",
    InputVtxContainerName      = "BPHY23_JpsiJpsi4TVertices",
    TrackIndices               = [ 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = etacmass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi4T_etac",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi4T_etac

BPHY23Rev_JpsiJpsi4T_jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi4T_jpsi",
    InputVtxContainerName      = "BPHY23_JpsiJpsi4TVertices",
    TrackIndices               = [ 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi4T_jpsi",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi4T_jpsi

BPHY23Rev_JpsiJpsi4T_chic0 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi4T_chic0",
    InputVtxContainerName      = "BPHY23_JpsiJpsi4TVertices",
    TrackIndices               = [ 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic0mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi4T_chic0",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi4T_chic0

BPHY23Rev_JpsiJpsi4T_chic1 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi4T_chic1",
    InputVtxContainerName      = "BPHY23_JpsiJpsi4TVertices",
    TrackIndices               = [ 4, 5, 6, 7 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi4T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic1mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi4T_chic1",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi4T_chic1


BPHY23Rev_JpsiJpsi6T_etac = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi6T_etac",
    InputVtxContainerName      = "BPHY23_JpsiJpsi6TVertices",
    TrackIndices               = [ 4, 5, 6, 7, 8, 9 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = etacmass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi6T_etac",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi6T_etac

BPHY23Rev_JpsiJpsi6T_jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi6T_jpsi",
    InputVtxContainerName      = "BPHY23_JpsiJpsi6TVertices",
    TrackIndices               = [ 4, 5, 6, 7, 8, 9 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi6T_jpsi",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi6T_jpsi

BPHY23Rev_JpsiJpsi6T_chic0 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi6T_chic0",
    InputVtxContainerName      = "BPHY23_JpsiJpsi6TVertices",
    TrackIndices               = [ 4, 5, 6, 7, 8, 9 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic0mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi6T_chic0",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi6T_chic0

BPHY23Rev_JpsiJpsi6T_chic1 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_JpsiJpsi6T_chic1",
    InputVtxContainerName      = "BPHY23_JpsiJpsi6TVertices",
    TrackIndices               = [ 4, 5, 6, 7, 8, 9 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23_JpsiJpsi6T_RefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = chic1mass,
    MassInputParticles         = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass],
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_JpsiJpsi6T_chic1",
    MaxPVrefit                 = 100,
    DoVertexType               = 7)
ToolSvc += BPHY23Rev_JpsiJpsi6T_chic1


#Track isolation for vertices
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__VertexTrackIsolation
BPHY23_JpsiEtac4T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiEtac4T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiEtac4TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiEtac4T_VtxTrkIso

BPHY23_JpsiEtac6T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiEtac6T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiEtac6TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiEtac6T_VtxTrkIso

BPHY23_JpsiJpsi1T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiJpsi1T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiJpsi1TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiJpsi1T_VtxTrkIso

BPHY23_JpsiJpsi2T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiJpsi2T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiJpsi2TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiJpsi2T_VtxTrkIso

BPHY23_JpsiJpsi4T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiJpsi4T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiJpsi4TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiJpsi4T_VtxTrkIso

BPHY23_JpsiJpsi6T_VtxTrkIso = DerivationFramework__VertexTrackIsolation(
  name                  = "BPHY23_JpsiJpsi6T_VtxTrkIso",
  TrackContainer        = "InDetTrackParticles",
  InputVertexContainer  = "BPHY23_JpsiJpsi6TVertices",
  DoVertexTypes         = 7)
ToolSvc += BPHY23_JpsiJpsi6T_VtxTrkIso


list_all_obj = list_2trk0trk_obj + list_2trk2trk_obj

OutputCollections = []
RefPVContainers = []
RefPVAuxContainers = []
expression = "("

for obj in list_all_obj:
    OutputCollections += obj.OutputVertexCollections
    RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + obj.HypothesisName + "_RefPrimaryVertices"]
    RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
    expression += "count(BPHY23_" + obj.HypothesisName + "_MainVtx.passed_" + obj.HypothesisName + ") + "

OutputCollections += ["BPHY23_JpsiEtac4TVertices", "BPHY23_JpsiEtac6TVertices",
                      "BPHY23_JpsiJpsi1TVertices", "BPHY23_JpsiJpsi2TVertices",
                      "BPHY23_JpsiJpsi4TVertices", "BPHY23_JpsiJpsi6TVertices",
                      "BPHY23Revtx_JpsiEtac4T_etac", "BPHY23Revtx_JpsiEtac4T_jpsi",
                      "BPHY23Revtx_JpsiEtac4T_chic0", "BPHY23Revtx_JpsiEtac4T_chic1",
                      "BPHY23Revtx_JpsiEtac6T_etac", "BPHY23Revtx_JpsiEtac6T_jpsi",
                      "BPHY23Revtx_JpsiEtac6T_chic0", "BPHY23Revtx_JpsiEtac6T_chic1",
                      "BPHY23Revtx_JpsiJpsi4T_etac", "BPHY23Revtx_JpsiJpsi4T_jpsi",
                      "BPHY23Revtx_JpsiJpsi4T_chic0", "BPHY23Revtx_JpsiJpsi4T_chic1",
                      "BPHY23Revtx_JpsiJpsi6T_etac", "BPHY23Revtx_JpsiJpsi6T_jpsi",
                      "BPHY23Revtx_JpsiJpsi6T_chic0", "BPHY23Revtx_JpsiJpsi6T_chic1"]
RefPVContainers += ["xAOD::VertexContainer#BPHY23_JpsiEtac4T_RefPrimaryVertices",
                    "xAOD::VertexContainer#BPHY23_JpsiEtac6T_RefPrimaryVertices",
                    "xAOD::VertexContainer#BPHY23_JpsiJpsi1T_RefPrimaryVertices",
                    "xAOD::VertexContainer#BPHY23_JpsiJpsi2T_RefPrimaryVertices",
                    "xAOD::VertexContainer#BPHY23_JpsiJpsi4T_RefPrimaryVertices",
                    "xAOD::VertexContainer#BPHY23_JpsiJpsi6T_RefPrimaryVertices"]
RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_JpsiEtac4T_RefPrimaryVerticesAux.",
                       "xAOD::VertexAuxContainer#BPHY23_JpsiEtac6T_RefPrimaryVerticesAux.",
                       "xAOD::VertexAuxContainer#BPHY23_JpsiJpsi1T_RefPrimaryVerticesAux.",
                       "xAOD::VertexAuxContainer#BPHY23_JpsiJpsi2T_RefPrimaryVerticesAux.",
                       "xAOD::VertexAuxContainer#BPHY23_JpsiJpsi4T_RefPrimaryVerticesAux.",
                       "xAOD::VertexAuxContainer#BPHY23_JpsiJpsi6T_RefPrimaryVerticesAux."]
expression += "count(BPHY23_JpsiEtac4TVertices.passed_JpsiEtac4T) + count(BPHY23_JpsiEtac6TVertices.passed_JpsiEtac6T) + count(BPHY23_JpsiJpsi1TVertices.passed_JpsiJpsi1T) + count(BPHY23_JpsiJpsi2TVertices.passed_JpsiJpsi2T) + count(BPHY23_JpsiJpsi4TVertices.passed_JpsiJpsi4T) + count(BPHY23_JpsiJpsi6TVertices.passed_JpsiJpsi6T)"

expression += ") > 0"

#--------------------------------------------------------------------
## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
##
##       "ContainerName.passed_HypoName > count"
##
##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
##    tool and "count" is the number of candidates passing the selection you want to keep. 

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY23_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name = "BPHY23_SelectEvent", expression = expression)

ToolSvc += BPHY23_SelectEvent

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
## IMPORTANT bit. Don't forget to pass the tools to the DerivationKernel! If you don't do that, they will not be 
## be executed!

# The name of the kernel (BPHY23Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentation_tools = [BPHY23_Reco_mumu, BPHY23FourTrackReco_PsiX3872, BPHY23Rev_Psi4Body, BPHY23Select_Jpsi, BPHY23Select_Psi, BPHY23Rev_Jpsi, BPHY23Rev_Psi] + list_all_obj + [ BPHY23_JpsiEtac4T, BPHY23_JpsiEtac6T, BPHY23_JpsiJpsi1T, BPHY23_JpsiJpsi2T, BPHY23_JpsiJpsi4T, BPHY23_JpsiJpsi6T, BPHY23_JpsiEtac4T_VtxTrkIso, BPHY23_JpsiEtac6T_VtxTrkIso, BPHY23_JpsiJpsi1T_VtxTrkIso, BPHY23_JpsiJpsi2T_VtxTrkIso, BPHY23_JpsiJpsi4T_VtxTrkIso, BPHY23_JpsiJpsi6T_VtxTrkIso, BPHY23Rev_JpsiEtac4T_etac, BPHY23Rev_JpsiEtac4T_jpsi, BPHY23Rev_JpsiEtac4T_chic0, BPHY23Rev_JpsiEtac4T_chic1, BPHY23Rev_JpsiEtac6T_etac, BPHY23Rev_JpsiEtac6T_jpsi, BPHY23Rev_JpsiEtac6T_chic0, BPHY23Rev_JpsiEtac6T_chic1, BPHY23Rev_JpsiJpsi4T_etac, BPHY23Rev_JpsiJpsi4T_jpsi, BPHY23Rev_JpsiJpsi4T_chic0, BPHY23Rev_JpsiJpsi4T_chic1, BPHY23Rev_JpsiJpsi6T_etac, BPHY23Rev_JpsiJpsi6T_jpsi, BPHY23Rev_JpsiJpsi6T_chic0, BPHY23Rev_JpsiJpsi6T_chic1]

DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY23Kernel",
    AugmentationTools = augmentation_tools,
    SkimmingTools     = [BPHY23_SelectEvent]
)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_BPHY23Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_BPHY23Stream )
BPHY23Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY23Stream.AcceptAlgs(["BPHY23Kernel"])
# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="BPHY23ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Slimming 
#====================================================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY23SlimmingHelper = SlimmingHelper("BPHY23SlimmingHelper")
BPHY23_AllVariables = []
BPHY23_StaticContent = []

# Needed for trigger objects
BPHY23SlimmingHelper.IncludeMuonTriggerContent = True
BPHY23SlimmingHelper.IncludeBPhysTriggerContent = True

## primary vertices
BPHY23_AllVariables += ["PrimaryVertices"]
BPHY23_StaticContent += RefPVContainers
BPHY23_StaticContent += RefPVAuxContainers

## ID track particles
BPHY23_AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are stored in InDetTrackParticles collection)
BPHY23_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

## muon container
BPHY23_AllVariables += ["Muons", "MuonSegments"]

## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
for output in OutputCollections:
    BPHY23_StaticContent += ["xAOD::VertexContainer#%s" % output]
    BPHY23_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % output]

# Truth information for MC only
if isSimulation:
    BPHY23_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

BPHY23SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
BPHY23SlimmingHelper.AllVariables = BPHY23_AllVariables
BPHY23SlimmingHelper.StaticContent = BPHY23_StaticContent
BPHY23SlimmingHelper.AppendContentToStream(BPHY23Stream)
