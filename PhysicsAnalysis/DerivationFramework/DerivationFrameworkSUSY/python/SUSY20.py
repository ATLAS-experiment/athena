# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#========================================================================
# DAOD_SUSY20.py
# This defines DAOD_SUSY20, a skimmed DAOD format for Run-2 and Run-3.
# It requires the flag SUSY20 in Derivation_tf.py
#========================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def SUSY20KernelCfg(flags, name = "SUSY20Kernel", **kwargs):

	"""Configure the derivation framework driving algorithm (kernel) for SUSY20"""
	
	acc = ComponentAccumulator()

	#
	# SUSY20 augmentations
	#
	augmentationTools = []
	
	# V0Finder Tool
	# Details here: https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/python/V0ToolConfig.py
	from DerivationFrameworkBPhys.V0ToolConfig import BPHY_Reco_V0FinderCfg as Reco_V0Finder
	SUSY20_RecoV0_Finder = acc.popToolsAndMerge(Reco_V0Finder(
		flags,
		derivation             = "SUSY20",
		suffix                 = "DT",
		V0ContainerName        = "SUSY20RecoV0Candidates",
		KshortContainerName    = "SUSY20RecoKshortCandidates",
		LambdaContainerName    = "SUSY20RecoLambdaCandidates",
		LambdabarContainerName = "SUSY20RecoLambdabarCandidates",
		CheckVertexContainers  = ["PrimaryVertices"]
	))
	acc.addPublicTool(SUSY20_RecoV0_Finder)
	augmentationTools.append(SUSY20_RecoV0_Finder)
	
	# VrtSecInclusiveTool
	from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg
	acc.merge(VrtSecInclusiveCfg(
		flags,
		name                                   = "SUSY20_VrtSecInclusiveTool",
		AugmentingVersionString                = "_SUSY20",
		CutSctHits                             = 1,
		TrkA0ErrCut                            = 200000,
		TrkZErrCut                             = 200000,
		TrkPtCut                               = 300, 
		TrkChi2Cut                             = 5.0,
		SelTrkMaxCutoff                        = 2000,  
		a0TrkPVDstMinCut                       = 0.0,
		SelVrtChi2Cut                          = 4.5,
		CutSharedHits                          = 5,
		TruthTrkLen                            = 1,
		DoSAloneTRT                            = False,
		DoTruth                                = flags.Input.isMC,
	))
	
	# Track isolation
	from xAODPrimitives.xAODIso import xAODIso as isoPar
	deco_ptcones        = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20, isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20]
	deco_ptcones_suffix = ["ptcone40", "ptcone30", "ptcone20", "topoetcone40", "topoetcone30", "topoetcone20"]
	deco_prefix         = 'SUSY20_'

	from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg
	SUSY20TrackIsoTool = acc.popToolsAndMerge(TrackIsolationToolCfg(
		flags,
		name = "SUSY20TrackIsoTool",
	))
	SUSY20TrackIsoTool.TrackSelectionTool.maxZ0SinTheta = 1.5
	SUSY20TrackIsoTool.TrackSelectionTool.maxD0         = 1.5
	SUSY20TrackIsoTool.TrackSelectionTool.minPt         = 1000
	SUSY20TrackIsoTool.TrackSelectionTool.CutLevel      = "TightPrimary"
	acc.addPublicTool(SUSY20TrackIsoTool)

	from IsolationAlgs.IsoToolsConfig import CaloIsolationToolCfg
	from CaloIdentifier import SUBCALO
	SUSY20CaloIsoTool = acc.popToolsAndMerge(CaloIsolationToolCfg(
		flags,
		name                         = "SUSY20CaloIsoTool",
		EMCaloNums                   = [SUBCALO.LAREM],
		HadCaloNums                  = [SUBCALO.LARHEC, SUBCALO.TILE],
		UseEMScale                   = True,
		UseCaloExtensionCaching      = False,
		saveOnlyRequestedCorrections = True
	))
	acc.addPublicTool(SUSY20CaloIsoTool)
	
	from DerivationFrameworkInDet.InDetToolsConfig import IsolationTrackDecoratorCfg
	SUSY20IDTrackDecoratorTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(
		flags,
		name               = "SUSY20IDTrackDecoratorTool",
		TrackIsolationTool = SUSY20TrackIsoTool,
		CaloIsolationTool  = SUSY20CaloIsoTool,
		TargetContainer    = "InDetTrackParticles",
		SelectionString    = "InDetTrackParticles.pt > 0.5*GeV",
		iso                = deco_ptcones,
		isoSuffix          = deco_ptcones_suffix,
		Prefix             = deco_prefix,
	))
	acc.addPublicTool(SUSY20IDTrackDecoratorTool)
	augmentationTools.append(SUSY20IDTrackDecoratorTool)

	# Commented out calo isolation, not needed by current analyses but still included in r21 SUSY20_DAODs
	'''# Electron TrackParticles isolation 
	SUSY20ElectronDecorator = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(
		flags,
		name               = "SUSY20ElectronDecoratorTool",
		TrackIsolationTool = SUSY20TrackIsoTool,
		CaloIsolationTool  = SUSY20CaloIsoTool,
		TargetContainer    = "Electrons",
		iso                = [isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20],
		isoSuffix          = ["topoetcone40", "topoetcone30", "topoetcone20"],
		Prefix             = deco_prefix,
	))
	acc.addPublicTool(SUSY20ElectronDecorator)
	augmentationTools.append(SUSY20ElectronDecorator)

	# Muon TrackParticles isolation 
	SUSY20MuonDecorator = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(
		flags,
		name               = "SUSY20MuonDecoratorTool",
		TrackIsolationTool = SUSY20TrackIsoTool,
		CaloIsolationTool  = SUSY20CaloIsoTool,
		TargetContainer    = "Muons",
		iso                = [isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20],
		isoSuffix          = ["topoetcone40", "topoetcone30", "topoetcone20"],
		Prefix             = deco_prefix,
	))
	acc.addPublicTool(SUSY20MuonDecorator)
	augmentationTools.append(SUSY20MuonDecorator)

	# Photon TrackParticles isolation 
	SUSY20PhotonDecorator = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(
		flags,
		name               = "SUSY20PhotonDecoratorTool",
		TrackIsolationTool = SUSY20TrackIsoTool,
		CaloIsolationTool  = SUSY20CaloIsoTool,
		TargetContainer    = "Photons",
		iso                = [isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20],
		isoSuffix          = ["topoetcone40", "topoetcone30", "topoetcone20"],
		Prefix             = deco_prefix,
	))
	acc.addPublicTool(SUSY20PhotonDecorator)
	augmentationTools.append(SUSY20PhotonDecorator)'''

	# PHYS common augmentations
	from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
	acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))
	
	# Track selection augmentation need for InDetTrackParticles thinning
	from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
	SUSY20TrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
		flags,
		name           = "SUSY20TrackSelection",
		ContainerName  = "InDetTrackParticles",
		DecorationName = "DFLoose" 
	))
	SUSY20TrackSelection.TrackSelectionTool.CutLevel = "Loose"
	acc.addPublicTool(SUSY20TrackSelection)
	augmentationTools.append(SUSY20TrackSelection)

	#
	# Thinning tools
	#
	thinningTools = []
	thinningExpression = "InDetTrackParticles.DFLoose && (InDetTrackParticles.pt > 0.5*GeV) && (abs( DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta) ) < 3.0*mm)"

	# TrackParticles directly
	from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
	SUSY20TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
		flags,
		name                    = "SUSY20TrackParticleThinningTool",
		StreamName              = kwargs['StreamName'],
		SelectionString         = thinningExpression,
		InDetTrackParticlesKey  = "InDetTrackParticles"
	))
	acc.addPublicTool(SUSY20TrackParticleThinningTool)
	thinningTools.append(SUSY20TrackParticleThinningTool)

	# TrackParticles associated with Muons
	from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg
	SUSY20MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
		flags,
		name                    = "SUSY20MuonTPThinningTool",
		StreamName              = kwargs['StreamName'],
		MuonKey                 = "Muons",
		InDetTrackParticlesKey  = "InDetTrackParticles"
	))
	acc.addPublicTool(SUSY20MuonTPThinningTool)
	thinningTools.append(SUSY20MuonTPThinningTool)

	# TrackParticles associated with Electrons
	from DerivationFrameworkInDet.InDetToolsConfig import EgammaTrackParticleThinningCfg
	SUSY20ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
		flags,
		name                   = "SUSY20ElectronTPThinningTool",
		StreamName             = kwargs['StreamName'],
		SGKey                  = "Electrons",
		InDetTrackParticlesKey = "InDetTrackParticles"
	))
	acc.addPublicTool(SUSY20ElectronTPThinningTool)
	thinningTools.append(SUSY20ElectronTPThinningTool)

	# Photon thinning
	from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
	SUSY20PhotonThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(
		flags,
		name            = "SUSY20PhotonThinningTool",
		StreamName      = kwargs['StreamName'],
		ContainerName   = "Photons",
		SelectionString = "Photons.pt > 10*GeV"
	))
	acc.addPublicTool(SUSY20PhotonThinningTool)
	thinningTools.append(SUSY20PhotonThinningTool)

	# TrackParticles associated with photons
	SUSY20PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
		flags,
		name                     = "SUSY20PhotonTPThinningTool",
		StreamName               = kwargs['StreamName'],
		SGKey                    = "Photons",
		InDetTrackParticlesKey   = "InDetTrackParticles",
		GSFConversionVerticesKey = "GSFConversionVertices"
	))
	acc.addPublicTool(SUSY20PhotonTPThinningTool)
	thinningTools.append(SUSY20PhotonTPThinningTool)

	# Truth thinning
	if flags.Input.isMC:
		from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
		SUSY20TruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(
			flags,
			name                         = "SUSY20TruthThinningTool",
			WritePartons                 = False,
			WriteHadrons                 = True,
			WriteBHadrons                = True,
			WriteGeant                   = False,
			GeantPhotonPtThresh          = 20000,
			WriteTauHad                  = True,
			PartonPtThresh               = -1.0,
			WriteBSM                     = True, 
			WriteBosons                  = True,
			WriteBosonProducts           = False,
			WriteBSMProducts             = True,
			WriteTopAndDecays            = True,
			WriteEverything              = False,
			WriteAllLeptons              = True,
			WriteLeptonsNotFromHadrons   = False,
			WriteNotPhysical             = False,
			WriteFirstN                  = 10,
			PreserveAncestors            = True,
			PreserveGeneratorDescendants = True
		))
		acc.addPublicTool(SUSY20TruthThinningTool)
		thinningTools.append(SUSY20TruthThinningTool)

	#
	# Skimming
	#
	skimmingTools = []
	filterList = []
	
	# DT-specific trigger skimming
	if flags.Trigger.EDMVersion >= 0:
		from DerivationFrameworkSUSY.SUSYToolsConfig import (
			SUSY20DTTriggerSkimmingToolCfg)
		SUSY20TriggerSkimmingTool_DT = acc.getPrimaryAndMerge(
			SUSY20DTTriggerSkimmingToolCfg(flags, name = "SUSY20TriggerSkimmingTool"))
		filterList += [SUSY20TriggerSkimmingTool_DT]

	# DT-specific jet skimming
	jetRequirements_DT = "AntiKt4EMPFlowJets.pt > 200*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.8"
	jetSelection_DT    = "(count(" + jetRequirements_DT + ") >= 1)"
	
	from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
		xAODStringSkimmingToolCfg)
	SUSY20JetSkimmingTool_DT = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
		flags, name = "SUSY20JetSkimmingTool_DT", expression = jetSelection_DT))
	filterList += [SUSY20JetSkimmingTool_DT]

	# DT-specific final skim selection, with trigger selection AND jet selection
	from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationANDCfg
	SUSY20xAODSkimmingTool_DT = acc.getPrimaryAndMerge(FilterCombinationANDCfg(
		flags, name = "SUSY20SkimmingTool_DT", FilterList = filterList))
	skimmingTools.append(SUSY20xAODSkimmingTool_DT)

	#
	# The kernel algorithm itself
	#
	DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
	acc.addEventAlgo(DerivationKernel(
		name, 
		SkimmingTools     = skimmingTools, 
		ThinningTools     = thinningTools, 
		AugmentationTools = augmentationTools, 
		RunSkimmingFirst  = True
	))       
	
	return acc

def SUSY20Cfg(flags):

	stream_name = 'StreamDAOD_SUSY20'
	acc = ComponentAccumulator()

	# Get the lists of triggers needed for trigger matching.
	# This is needed at this scope (for the slimming) and further down in the config chain
	# for actually configuring the matching, so we create it here and pass it down
	from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
	SUSY20TriggerListsHelper = TriggerListsHelper(flags)

	# SUSY20 augmentations
	acc.merge(SUSY20KernelCfg(flags, name = "SUSY20Kernel", StreamName = stream_name, TriggerListsHelper = SUSY20TriggerListsHelper))

	## CloseByIsolation correction augmentation
	## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
	from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
	acc.merge(IsoCloseByAlgsCfg(flags, suff = "_SUSY20", isPhysLite = False, containerNames = ["Muons", "Electrons", "Photons"], useSelTools = True, stream_name = stream_name))

	# =============================
	# Define contents of the format
	# =============================
	from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
	from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
	from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

	SUSY20SlimmingHelper = SlimmingHelper("SUSY20SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
	
	# Containers that we slim
	SUSY20SlimmingHelper.SmartCollections = [
		"EventInfo",
		"Electrons", 
		"Photons", 
		"Muons",
		"InDetTrackParticles",
		"PrimaryVertices",
		"TauJets",
		"AntiKt4EMPFlowJets",
		
		"MET_Baseline_AntiKt4EMPFlow",
	]
	
	# Extra content
	# Commented out variables not needed by current analyses but still included in r21 SUSY20_DAODs
	SUSY20SlimmingHelper.ExtraVariables += [
		"EventInfo.DFCommonJets_eventClean_TightBad.DFCommonJets_jetClean_TightBad",
		"AntiKt4EMPFlowJets.DFCommonJets_jetClean_LooseBad.DFCommonJets_jetClean_TightBad",
		"Electrons.DFCommonElectronsLHVeryLoose.DFCommonElectronsLHLoose.DFCommonElectronsLHLooseBL.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsLHVeryLooseIsEMValue.DFCommonElectronsLHLooseIsEMValue.DFCommonElectronsLHLooseBLIsEMValue.DFCommonElectronsLHMediumIsEMValue.DFCommonElectronsLHTightIsEMValue",
		#"Electrons.SUSY20_topoetcone20.SUSY20_topoetcone30.SUSY20_topoetcone40.SUSY20_topoetcone20NonCoreCone.SUSY20_topoetcone30NonCoreCone.SUSY20_topoetcone40NonCoreCone",
		"Muons.ptcone40.ptcone30.ptcone20.charge.quality.InnerDetectorPt.MuonSpectrometerPt.CaloLRLikelihood.CaloMuonIDTag.eta_sampl.phi_sampl",
		#"Muons.SUSY20_topoetcone20.SUSY20_topoetcone30.SUSY20_topoetcone40.SUSY20_topoetcone20NonCoreCone.SUSY20_topoetcone30NonCoreCone.SUSY20_topoetcone40NonCoreCone",
		"Photons.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMTight",
		#"Photons.SUSY20_topoetcone20.SUSY20_topoetcone30.SUSY20_topoetcone40.SUSY20_topoetcone20NonCoreCone.SUSY20_topoetcone30NonCoreCone.SUSY20_topoetcone40NonCoreCone",
		"InDetTrackParticles.TRTdEdx.TRTdEdxUsedHits.hitPattern.numberOfContribPixelLayers.numberOfGangedFlaggedFakes.numberOfIBLOverflowsdEdx.numberOfPixelOutliers.numberOfPixelSplitHits.numberOfPixelSpoiltHits.numberOfSCTOutliers.numberOfSCTSpoiltHits.numberOfTRTDeadStraws.numberOfTRTHits.numberOfTRTHoles.numberOfTRTOutliers.numberOfTRTSharedHits.numberOfUsedHitsdEdx.pixeldEdx",
		"InDetTrackParticles.chiSquared.d0.definingParametersCovMatrix.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberDoF.numberOfInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerHits.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfPixelSharedHits.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfSCTSharedHits.phi.qOverP.theta.z0",
		"InDetTrackParticles.numberOfInnermostPixelLayerOutliers.numberOfInnermostPixelLayerSharedHits.numberOfInnermostPixelLayerSplitHits.numberOfNextToInnermostPixelLayerOutliers.numberOfNextToInnermostPixelLayerSharedHits.numberOfNextToInnermostPixelLayerSplitHits.numberOfGangedPixels.numberOfSCTDoubleHoles.numberOfTRTHighThresholdHits.numberOfTRTHighThresholdHitsTotal.numberOfTRTHighThresholdOutliers.numberOfTRTTubeHits.numberOfTRTXenonHits.numberOfOutliersOnTrack.standardDeviationOfChi2OS.eProbabilityHT.eProbabilityComb.TRTTrackOccupancy",
		"InDetTrackParticles.SUSY20_ptcone20.SUSY20_ptcone30.SUSY20_ptcone40",
		#"InDetTrackParticles.SUSY20_ptvarcone20.SUSY20_ptvarcone30.SUSY20_ptvarcone40",
		"InDetTrackParticles.SUSY20_topoetcone20.SUSY20_topoetcone30.SUSY20_topoetcone40.SUSY20_topoetcone20NonCoreCone.SUSY20_topoetcone30NonCoreCone.SUSY20_topoetcone40NonCoreCone",
		"InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights",
		"InDetTrackParticles.vertexLink.vz",
		#"InDetTrackParticles.trackCaloClusEta.trackCaloClusPhi.trackCaloSampleE.trackCaloSampleNumber",
		#"InDetTrackParticles.SUSY20_trackCaloClusEta_ele.SUSY20_trackCaloClusPhi_ele.SUSY20_trackCaloClusE_ele.SUSY20_trackCaloSampleE_ele.SUSY20_trackCaloSampleNumber_ele",
	]

	# Truth content
	if flags.Input.isMC:

		from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
		acc.merge(AddTauAndDownstreamParticlesCfg(flags, generations = 2))

		SUSY20SlimmingHelper.AllVariables += [
			'MET_Truth',
			'TruthEvents',
			'TruthParticles',
			'TruthVertices',
			'TruthElectrons',
			'TruthMuons',
			'TruthPhotons',
			'TruthTaus',
			'TruthNeutrinos',
			'TruthBSM',
			'TruthBosonsWithDecayParticles',
			'TruthBosonsWithDecayVertices',
			'TruthBSMWithDecayParticles',
			'TruthBSMWithDecayVertices',
			'TruthTausWithDecayParticles',
			'TruthTausWithDecayVertices',
			'AntiKt4TruthJets',
			'AntiKt4TruthDressedWZJets'
		]
		SUSY20SlimmingHelper.ExtraVariables += [
			"Electrons.TruthLink",
			"Muons.TruthLink",
			"Photons.TruthLink",
			"TruthEvents.Q.XF1.XF2.PDGID1.PDGID2.PDFID1.PDFID2.X1.X2.crossSection",
			"InDetTrackParticles.TruthLink.truthMatchProbability.truthOrigin.truthType.truthParticleLink"
		]

	StaticContent = []
	
	# V0 content
	for containerName in ["SUSY20RecoV0Candidates","SUSY20RecoKshortCandidates","SUSY20RecoLambdaCandidates","SUSY20RecoLambdabarCandidates"]:
		StaticContent += [
			"xAOD::VertexContainer#%s"%containerName,
			"xAOD::VertexAuxContainer#%sAux"%containerName
			+ ".-vxTrackAtVertex"  
			+ ".-vertexType"
			+ ".-neutralParticleLinks"
			+ ".-neutralWeights"
			+ ".-KshortLink"
			+ ".-LambdaLink"
			+ ".-LambdabarLink"
			+ ".-gamma_fit"
			+ ".-gamma_mass"
			+ ".-gamma_massError"
			+ ".-gamma_probability",
		]

	# VSI content
	VSITrackVars = ["is_selected", "is_associated", "is_svtrk_final", "pt_wrtSV", "eta_wrtSV", "phi_wrtSV", "d0_wrtSV", "z0_wrtSV", "errP_wrtSV", "errd0_wrtSV", "errz0_wrtSV", "chi2_toSV"]
	SUSY20SlimmingHelper.ExtraVariables += ["InDetTrackParticles." + ".".join( var + "_SUSY20" for var in VSITrackVars )]

	StaticContent += [
		'xAOD::VertexContainer#VrtSecInclusive_SecondaryVertices_SUSY20',
		'xAOD::VertexAuxContainer#VrtSecInclusive_SecondaryVertices_SUSY20Aux'
		+ '.-vxTrackAtVertex'
		+ '.-vertexType'
		+ '.-neutralParticleLinks'
		+ '.-trackParticleLinks'
		+ '.-neutralWeights'
	]

	SUSY20SlimmingHelper.StaticContent = StaticContent

	## CloseByIsolation content
	from IsolationSelection.IsolationSelectionConfig import setupIsoCloseBySlimmingVariables
	setupIsoCloseBySlimmingVariables(SUSY20SlimmingHelper)

	# Trigger content
	SUSY20SlimmingHelper.IncludeTriggerNavigation     = False
	SUSY20SlimmingHelper.IncludeMuonTriggerContent    = False
	SUSY20SlimmingHelper.IncludeEGammaTriggerContent  = False
	SUSY20SlimmingHelper.IncludeEtMissTriggerContent  = False
	SUSY20SlimmingHelper.IncludeJetTriggerContent     = False
	SUSY20SlimmingHelper.IncludeBJetTriggerContent    = False
	SUSY20SlimmingHelper.IncludeBPhysTriggerContent   = False
	SUSY20SlimmingHelper.IncludeTauTriggerContent     = False
	SUSY20SlimmingHelper.IncludeMinBiasTriggerContent = False

	# Trigger matching
	# Run 2
	if flags.Trigger.EDMVersion == 2:
		from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
		AddRun2TriggerMatchingToSlimmingHelper(
			SlimmingHelper        = SUSY20SlimmingHelper, 
			OutputContainerPrefix = "TrigMatch_", 
			TriggerList           = SUSY20TriggerListsHelper.Run2TriggerNamesTau
		)
		AddRun2TriggerMatchingToSlimmingHelper(
			SlimmingHelper        = SUSY20SlimmingHelper, 
			OutputContainerPrefix = "TrigMatch_",
			TriggerList           = SUSY20TriggerListsHelper.Run2TriggerNamesNoTau
		)
	# Run 3, or Run 2 with navigation conversion
	if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
		from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
		AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(SUSY20SlimmingHelper)

	# Output stream
	SUSY20ItemList = SUSY20SlimmingHelper.GetItemList()
	acc.merge(OutputStreamCfg(flags, "DAOD_SUSY20", ItemList = SUSY20ItemList, AcceptAlgs = ["SUSY20Kernel"]))
	acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_SUSY20", AcceptAlgs = ["SUSY20Kernel"], createMetadata = [MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

	return acc
