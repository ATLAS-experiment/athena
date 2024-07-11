# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python

#====================================================================
# HION14.py
# author: Mariana Vivas <mariana.isabel.vivas.albornoz@cern.ch>
# Application: Open Data
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkMuons.MuonsCommon import *

# Configure stream
# The base name (DAOD_HION1 here) must match the string in
streamName = derivationFlags.WriteDAOD_HION1Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HION1Stream )
HION1Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HION1Stream.AcceptAlgs(["HION1Kernel"])

## Skimming 

skimmingTools = []
triggers=[
    "HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50",
    "HLT_noalg_mb_L1TE50"]

triggers='(' + ' || '.join(triggers) + ')'

print "******************************************************"
print triggers
print "******************************************************"

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
HION1SkimmingTriggerTool = DerivationFramework__xAODStringSkimmingTool( name = "HION1SkimmingTriggerTool",expression = triggers )

if not DerivationFrameworkHasTruth:
    ToolSvc+=HION1SkimmingTriggerTool
    skimmingTools.append(HION1SkimmingTriggerTool)

## Augmentation 

augmentationTools = []

from DerivationFrameworkHI.HIAugmentationTools import addHIGlobalAugmentationTool
HIGlobalAugmentationTool = addHIGlobalAugmentationTool("HION14",3,500)
augmentationTools+=[HIGlobalAugmentationTool]

### HITight
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__InDetTrackSelectionToolWrapper
from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool

HITightTrackSelector=InDet__InDetTrackSelectionTool("HION14TightInDetTrackSelectionTool") 
HITightTrackSelector.CutLevel = "HITight"
ToolSvc+=HITightTrackSelector

HITight_decorator = DerivationFramework__InDetTrackSelectionToolWrapper(name='HION14_HITight_Decorator', 
                                                                        TrackSelectionTool=HITightTrackSelector, 
                                                                        DecorationName='HITight', 
                                                                        ContainerName="InDetTrackParticles")

ToolSvc+=HITight_decorator
augmentationTools.append(HITight_decorator)

# Centrality
from DerivationFrameworkHI.DerivationFrameworkHIConf import DerivationFramework__HICentralityDecorationTool
HICentralityDecoratorTool = DerivationFramework__HICentralityDecorationTool(name="HICentralityTool")

# Register the tool with the ToolSvc
ToolSvc+=HICentralityDecoratorTool
augmentationTools.append(HICentralityDecoratorTool)

## Thinning 

thinningTools = []
# Track Loose
HILooseTrackSelector=InDet__InDetTrackSelectionTool("HION14LooseInDetTrackSelectionTool") 
HILooseTrackSelector.CutLevel = "HILoose"
ToolSvc+=HILooseTrackSelector

from DerivationFrameworkHI.DerivationFrameworkHIConf import DerivationFramework__HITrackParticleThinningTool
HION14LooseTrackThinningTool = DerivationFramework__HITrackParticleThinningTool(name = 'HION14LooseTrackThinningTool',
                                                                ThinningService = "HION14ThinningSvc",
                                                                InDetTrackParticlesKey = "InDetTrackParticles",
                                                                PrimaryVertexKey = "PrimaryVertices",
                                                                PrimaryVertexSelection = "sumPt2",
                                                                TrackSelectionTool = HILooseTrackSelector)


ToolSvc+=HION14LooseTrackThinningTool
thinningTools.append(HION14LooseTrackThinningTool)

# Stable particles and no spectators
from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
HION14ThinningTool = DerivationFramework__GenericTruthThinning(name = "HION14ThinningTool",
                                                            ThinningService  = "HION14ThinningSvc",
                                                            ParticleSelectionString  = "(TruthParticles.status == 1) && ( (TruthParticles.pdgId != 2112 && TruthParticles.pdgId != 2212) || TruthParticles.pt > 0.1 )")

if DerivationFrameworkHasTruth:
    ToolSvc+=HION14ThinningTool
    thinningTools.append(HION1ThinningTool)

# Include inner detector tracks associated with muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
HION1MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name = "HION1MuonTPThinningTool",
                                                                            ThinningService = "HION1ThinningSvc",
                                                                            MuonKey = "Muons",
                                                                            InDetTrackParticlesKey = "InDetTrackParticles",
                                                                            ApplyAnd = False)
ToolSvc += HION1MuonTPThinningTool
thinningTools.append(HION1MuonTPThinningTool)

## Special lines for thinning
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="HION1ThinningSvc", outStreams=[evtStream] )

## Kernel

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("HION1Kernel",
                                                                       AugmentationTools = augmentationTools,
                                                                       SkimmingTools = skimmingTools,
                                                                       ThinningTools = thinningTools
                                                                       )

## Slimming

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
allVariables = []
HION1SlimmingHelper = SlimmingHelper("HION1SlimmingHelper")
allVariables.append("CaloSums")
allVariables.append("PrimaryVertices")
HION1SlimmingHelper.ExtraVariables=["InDetTrackParticles.qOverP.theta.phi.d0.z0.TrackQuality.HITight", "TruthParticles.status",
                                    "Muons.pt.eta.phi.truthType.truthOrigin.author.muonType.quality.inDetTrackParticleLink.muonSpectrometerTrackParticleLink.combinedTrackParticleLink.InnerDetectorPt.MuonSpectrometerPt.DFCommonGoodMuon.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone20.topoetcone30.topoetcone40.truthParticleLink.charge.extrapolatedMuonSpectrometerTrackParticleLink.allAuthors.ptcone20_TightTTVA_pt1000.ptcone20_TightTTVA_pt500.ptvarcone30_TightTTVA_pt1000.ptvarcone30_TightTTVA_pt500.numberOfPrecisionLayers.combinedTrackOutBoundsPrecisionHits.numberOfPrecisionLayers.numberOfPrecisionHoleLayers.numberOfGoodPrecisionLayers.innerSmallHits.innerLargeHits.middleSmallHits.middleLargeHits.outerSmallHits.outerLargeHits.extendedSmallHits.extendedLargeHits.extendedSmallHoles.isSmallGoodSectors.cscUnspoiledEtaHits.EnergyLoss.energyLossType.momentumBalanceSignificance.scatteringCurvatureSignificance.scatteringNeighbourSignificance",
                                    "CombinedMuonTrackParticles.qOverP.d0.z0.vz.phi.theta.truthOrigin.truthType.definingParametersCovMatrix.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfTRTHits.numberOfTRTOutliers.chiSquared.numberDoF",
                                    "ExtrapolatedMuonTrackParticles.d0.z0.vz.definingParametersCovMatrix.truthOrigin.truthType.qOverP.theta.phi",
                                    "MuonSpectrometerTrackParticles.phi.d0.z0.vz.definingParametersCovMatrix.vertexLink.theta.qOverP.truthParticleLink",
]

if DerivationFrameworkHasTruth:
    HION1SlimmingHelper.ExtraVariables+=["TruthParticles.pdgId.barcode.m.e.py.px.pz"]
    allVariables.append("TruthEvents") 

HION1SlimmingHelper.AllVariables = allVariables
HION1SlimmingHelper.AppendContentToStream(HION1Stream) 
