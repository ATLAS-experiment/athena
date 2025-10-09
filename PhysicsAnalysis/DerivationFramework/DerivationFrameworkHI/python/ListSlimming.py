# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ListSlimming.py - List of collections for slimming from athena 21.2 HION4 and EGAM1ExtraContent.py

#################################################################################
#HION2

def HION2AllVariablesGeneral():
    
    variables  = []
    variables += ["CaloSums"]
    variables += ["PrimaryVertices"]
    
    return variables

def HION2ExtraVariablesGeneral():
    variables  = []
    variables += ["InDetTrackParticles.qOverP.theta.phi.TrackQuality"]
    
    return variables

def HION2AllVariablesPbPb_2015_5TeV():
    
    variables  = []
    variables += ["HIEventShape"]
    
    return variables

def HION2ExtraVariablesPbPb_2015_5TeV(): 
    
    variables  = []
    variables += ["HIClusters.calE.eta0.phi0"]
    
    return variables

def HION2AllVariablespPb_2016():
    
    variables  = []
    variables += ["ZdcModules",]
    variables += ["ZdcSums"]
    variables += ["ZdcTriggerTowers"]
    variables += ["MBTSForwardEventInfo"]
    variables += ["MBTSModules"]
    
    return variables

#################################################################################
#HION4

def HION4SmartCollections():
    variables  = []
    variables += ["EventInfo"]
    variables += ["Electrons"]
    variables += ["Photons"]
    variables += ["Muons"]
    variables += ["TauJets"]
    variables += ["AntiKt4EMTopoJets"]
    variables += ["InDetTrackParticles"]
    variables += ["PrimaryVertices"]
    
    return variables
    
def HION4AllVariablesGeneral():
    variables  =  []
    variables += ["SpacePoints"]
    variables += ["HLT_TCEventInfo_jet"]
    variables += ["HLT_SpacePointCounts"]
    variables += ["EventInfo"]
    variables += ["Trigger"]
    variables += ["Core"]
    variables += ["LVL1EnergySumRoI"]    
    variables += ["HLT_HIEventShapeEG"]  
    variables += ["CaloSums"]
    variables += ["ZdcModules"]
    variables += ["ZdcSums"]
    variables += ["ZdcTriggerTowers"]
    variables += ["MBTSForwardEventInfo"]
    variables += ["MBTSModules"]
    variables += ["HLT_xAOD__HIEventShapeContainer_HIFCAL"]
    variables += ["HLT_xAOD__HIEventShapeContainer_HIUE"]
    variables += ["HLT_xAOD__TrigT2MbtsBitsContainer_T2Mbts"]
    variables += ["HLT_xAOD__TrigTrackCountsContainer_trackcounts"]
    variables += ["HLT_xAOD__TrigSpacePointCountsContainer_spacepoints"]
    variables += ["LVL1MuonRoIs"]
    variables += ["HIEventShape"]
    variables += ["CaloCalTopoClusters"]
    variables += ["egammaClusters"]
    variables += ["egammaTopoSeededClusters"]
    variables += ["InDetPixelTrackParticles"]
    variables += ["Photons"]
    variables += ["Electrons"]
    variables += ["ForwardElectrons"]
    variables += ["AntiKt4HIJets"]
    
    return variables

def HION4ExtraContentTracks():
    variables=[]
    variables += ["InDetTrackParticles.eProbabilityHT"]
    variables += ["InDetTrackParticles.eProbabilityComb"]
    variables += ["InDetTrackParticles.deltaPoverP"] #cant find in data 
    variables += ["InDetTrackParticles.pixeldEdx"]
    variables += ["InDetTrackParticles.numberOfTRTHighThresholdHits"]
    return variables
    
def HION4ExtraContentMuons():
    variables  = []
    variables += ["Muons.DFCommonMuonsPreselection"]
    variables += ["Muons.ptcone20"]
    variables += ["Muons.ptcone30"]
    variables += ["Muons.ptcone40"]
    variables += ["Muons.etcone20"]
    variables += ["Muons.etcone30"]
    variables += ["Muons.etcone40"]
    
    return variables

def HION4ExtraMuonsTruth():
    variables  = []
    variables += ["MuonTruthParticles.truthOrigin"]
    variables += ["MuonTruthParticles.truthType"]

    return variables

def HION4ExtraContentPrimaryVertices():
    variables  = []
    variables += ["PrimaryVertices.sumPt2"]
    return variables

def HION4ExtraPhotonsTruth():
    variables  = []
    variables += ["Photons.truthOrigin"]
    variables += ["Photons.truthType"]
    variables += ["Photons.truthParticleLink"]
    
    return variables
    
def HION4ExtraContentGSFConversionVertices():
    variables  = []
    variables += ["GSFConversionVertices.x"]
    variables += ["GSFConversionVertices.y"]
    variables += ["GSFConversionVertices.z"]
    variables += ["GSFConversionVertices.px"]
    variables += ["GSFConversionVertices.py"]
    variables += ["GSFConversionVertices.pz"]
    variables += ["GSFConversionVertices.pt1"]
    variables += ["GSFConversionVertices.pt2"]
    variables += ["GSFConversionVertices.etaAtCalo"]
    variables += ["GSFConversionVertices.phiAtCalo"]
    variables += ["GSFConversionVertices.trackParticleLinks"]
    
    return variables
    

    
def HION4ExtraContentTrackJets():
    variables  = []
    variables += ["AntiKt4PV0TrackJets.pt.eta.phi.e.m.rapidity.btaggingLink.constituentLinks"]
    
    return variables

def HION4ExtraContentAll(): 
    variables  = []
    variables += HION4ExtraContentMuons()
    variables += HION4ExtraContentGSFConversionVertices()
    variables += HION4ExtraContentPrimaryVertices()   
    variables += HION4ExtraContentTrackJets()
    variables += HION4ExtraContentTracks()
    
    return variables

def HION4ExtraContentAllTruth():
    variables  = []
    variables += HION4ExtraMuonsTruth()
    variables += HION4ExtraPhotonsTruth()
    
    return variables
    
def HION4ExtraContainersTruth():
    variables  = []
    variables += ["TruthEvents"] 
    variables += ["TruthParticles"]
    variables += ["TruthVertices"]
    variables += ["AntiKt4TruthJets"]
    variables += ["egammaTruthParticles"]
    variables += ["MuonTruthParticles"]
    
    return variables

def HION4ExtraContainersElectrons():
    variables  = []
    variables += ["Electrons"]
    variables += ["GSFTrackParticles"]
    variables += ["egammaClusters"]
    variables += ["CaloCalTopoClusters"]
    variables += ["NewSwElectrons"]    # only if DoCellReweighting is ON
    variables += ["MaxVarSwElectrons"] # if variations are ON
    variables += ["MinVarSwElectrons"]  # if variations are ON

    return variables
    
def HION4ExtraContainersTrigger():    
    variables  = []
    variables += ["HLT_xAOD__ElectronContainer_egamma_Electrons"]
    variables += ["HLT_xAOD__ElectronContainer_egamma_ElectronsAux."]
    variables += ["HLT_xAOD__PhotonContainer_egamma_Photons"]
    variables += ["HLT_xAOD__PhotonContainer_egamma_PhotonsAux."]
    variables += ["HLT_xAOD__TrigRingerRingsContainer_TrigT2CaloEgamma"]
    variables += ["HLT_xAOD__TrigRingerRingsContainer_TrigT2CaloEgammaAux."]
    variables += ["HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgamma"]
    variables += ["HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgammaAux."]
    variables += ["HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFex"]
    variables += ["HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFexAux."]
    variables += ["HLT_xAOD__TrigRNNOutputContainer_TrigRingerNeuralFex"]
    variables += ["HLT_xAOD__TrigRNNOutputContainer_TrigRingerNeuralFexAux."]
    variables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_IDTrig"]
    variables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_IDTrigAux."]
    variables += ["HLT_xAOD__TrigPassBitsContainer_passbits"]
    variables += ["HLT_xAOD__TrigPassBitsContainer_passbitsAux."]
    variables += ["LVL1EmTauRoIs"]
    variables += ["LVL1EmTauRoIsAux."]
    variables += ["HLT_TrigRoiDescriptorCollection_initialRoI"]
    variables += ["HLT_TrigRoiDescriptorCollection_initialRoIAux."]
    variables += ["HLT_xAOD__RoiDescriptorStore_initialRoI"]
    variables += ["HLT_xAOD__RoiDescriptorStore_initialRoIAux."]
    variables += ["HLT_xAOD__TrigElectronContainer_L2ElectronFex"]
    variables += ["HLT_xAOD__TrigElectronContainer_L2ElectronFexAux."]

    return variables

def HION4ExtraVariablesEventShape():
    variables  = []
    for shape in ["TopoClusterIsoCentral", "TopoClusterIsoForward", "NeutralParticleFlowIsoCentral",
                  "NeutralParticleFlowIsoForward", "ParticleFlowIsoCentral", "ParticleFlowIsoForward"]:
        variables += [shape + "EventShape.DensitySigma.Density.DensityArea"]
    
    return variables

#################################################################################
#HION5

def HION5ExtraVariables():
    variables  = []
    variables += [
        ".".join(["InDetTrackParticles", field]) for field in [
            "truthMatchProbability.x.y.z.vx.vy.vz",
            "numberOfInnermostPixelLayerSplitHits",
            "numberOfNextToInnermostPixelLayerSplitHits",
            "numberOfNextToInnermostPixelLayerSharedHits",
            "numberOfPixelSplitHits",
            "numberOfInnermostPixelLayerSharedHits",
            "numberOfContribPixelLayers",
            "hitPattern.radiusOfFirstHit",
            "is_selected", "is_associated", "is_svtrk_final",
            "pt_wrtSV", "eta_wrtSV", "phi_wrtSV", "d0_wrtSV", "z0_wrtSV",
            "errP_wrtSV", "errd0_wrtSV", "errz0_wrtSV",
            "chi2_toSV",
            "eProbabilityHT", "eProbabilityComb", "deltaPoverP"
        ]
    ] 

    variables += [
        ".".join(["PrimaryVertices", field]) for field in [
            "neutralWeights", "numberDoF", "sumPt2", "chiSquared",
            "covariance", "trackWeights",
            "x.y.trackParticleLinks.vertexType.neutralParticleLinks"
        ]
    ]

    variables += ["ExtrapolatedMuonTrackParticles.vx.vy.vz"]
    variables += ["MuonSpectrometerTrackParticles.vx.vy.vz"]
    variables += ["CombinedMuonTrackParticles.vx.vy.vz"]

    variables += [
        ".".join(["Electrons", field]) for field in [
            "DFCommonElectronsHILHLoose", "DFCommonElectronsHILHMedium",
            "ptcone20", "ptcone30", "ptcone40",
            "ptvarcone20", "ptvarcone30", "ptvarcone40",
            "etcone20", "etcone30", "etcone40",
            "topoetcone20", "topoetcone30", "topoetcone40",
            "ptvarcone20_TightTTVA_pt500", "ptvarcone30_TightTTVA_pt500", "ptvarcone40_TightTTVA_pt500",
            "ptvarcone20_TightTTVA_pt1000", "ptvarcone30_TightTTVA_pt1000", "ptvarcone40_TightTTVA_pt1000",
            "ptvarcone20_TightTTVALooseCone_pt500", "ptvarcone30_TightTTVALooseCone_pt500", "ptvarcone40_TightTTVALooseCone_pt500",
            "ptvarcone20_TightTTVALooseCone_pt1000", "ptvarcone30_TightTTVALooseCone_pt1000", "ptvarcone40_TightTTVALooseCone_pt1000",
            "ptcone20_TightTTVA_pt500", "ptcone30_TightTTVA_pt500", "ptcone40_TightTTVA_pt500",
            "ptcone20_TightTTVA_pt1000", "ptcone30_TightTTVA_pt1000", "ptcone40_TightTTVA_pt1000",
            "ptcone20_TightTTVALooseCone_pt500", "ptcone30_TightTTVALooseCone_pt500", "ptcone40_TightTTVALooseCone_pt500",
            "ptcone20_TightTTVALooseCone_pt1000", "ptcone30_TightTTVALooseCone_pt1000", "ptcone40_TightTTVALooseCone_pt1000",
            "topoetcone20ptCorrection", "topoetcone30ptCorrection", "topoetcone40ptCorrection",
            "deltaPoverP"
        ]
    ]

    variables += [
        ".".join(["Muons", field]) for field in [
            "EnergyLoss.energyLossType",
            "ptcone20", "ptcone30", "ptcone40",
            "ptvarcone20", "ptvarcone30", "ptvarcone40",
            "etcone20", "etcone30", "etcone40",
            "topoetcone20", "topoetcone30", "topoetcone40",
            "ptcone20_TightTTVA_pt500", "ptcone30_TightTTVA_pt500", "ptcone40_TightTTVA_pt500",
            "ptcone20_TightTTVA_pt1000", "ptcone30_TightTTVA_pt1000", "ptcone40_TightTTVA_pt1000",
            "ptvarcone20_TightTTVA_pt500", "ptvarcone30_TightTTVA_pt500", "ptvarcone40_TightTTVA_pt500",
            "ptvarcone20_TightTTVA_pt1000", "ptvarcone30_TightTTVA_pt1000", "ptvarcone40_TightTTVA_pt1000",
            "ptcone20_TightTTVALooseCone_pt500", "ptcone30_TightTTVALooseCone_pt500", "ptcone40_TightTTVALooseCone_pt500",
            "ptcone20_TightTTVALooseCone_pt1000", "ptcone30_TightTTVALooseCone_pt1000", "ptcone40_TightTTVALooseCone_pt1000",
            "ptvarcone20_TightTTVALooseCone_pt500", "ptvarcone30_TightTTVALooseCone_pt500", "ptvarcone40_TightTTVALooseCone_pt500",
            "ptvarcone20_TightTTVALooseCone_pt1000", "ptvarcone30_TightTTVALooseCone_pt1000", "ptvarcone40_TightTTVALooseCone_pt1000"
        ]
    ]

    variables += [
        ".".join(["Photons", field]) for field in [
            "etcone20.etcone30.etcone40.Loose"
        ]
    ]
    

    return variables

def HION5AllVariables():
    variables  = []
    variables += ["AntiKt4HITrackJets"]
    variables += ["AntiKt2HIJets"]
    variables += ["AntiKt4HIJets"]
    variables += ["HIEventShape"]
    variables += ["ForwardElectrons"]
    variables += ["ForwardElectronClusters"]
    variables += ["EventInfo"]
    variables += ["CaloSums"]
    variables += ["ZdcModules"]
    variables += ["ZdcSums"]
    variables += ["ZdcTriggerTowers"]
    variables += ["PeripheralCaloCalTopoClusters"]
    variables += ["MET_Track1000", "MET_Track2000", "MET_Track3000", "MET_Track4000", "MET_Track5000"]

    return variables

def HION5AllTruthVariables():
    variables  = []
    variables += ["TruthEvents"]
    variables += ["TruthParticles"]
    variables += ["TruthVertices"]
    variables += ["egammaTruthParticles"]
    variables += ["MuonTruthParticles"]
    variables += ["AntiKt2TruthJets"]
    variables += ["AntiKt4TruthJets"]
    variables += ["TruthElectrons"]
    variables += ["TruthMuons"]
    return variables

def HION5SmartCollections():
    variables  = []
    variables += ["InDetTrackParticles"]
    variables += ["PrimaryVertices"]
    variables += ["Electrons"]
    variables += ["Muons"]
    variables += ["Photons"]
    #variables += ["MET_Reference_AntiKt4EMTopo",]
    variables += ["AntiKt4EMTopoJets"]
    
    return variables

def HION5ExtraContainersTrigger():    
    variables  = ["HLT_MuonsCB_RoI",
                  "HLT_MuonsCB_RoIAux.",
                  "HLT_egamma_Electrons",
                  "HLT_egamma_ElectronsAux."]
    return variables    

#################################################################################
#HION7

def HION7SmartCollections():
    variables  = []
    variables += ["EventInfo"]
    variables += ["Electrons"]
    variables += ["Photons"]
    variables += ["Muons"]
    variables += ["InDetTrackParticles"]

    return variables

def HION7AllVarContent():
    variables  = []
    variables += ["CaloSums"]
    variables += ["ZdcModules"]
    variables += ["PrimaryVertices"]
    variables += ["EventInfo"]

    return variables

def HION7AllVarTruthContent():
    variables  = []
    variables += ["AntiKt2TruthJets"]
    variables += ["AntiKt4TruthJets"]
    variables += ["TruthEvents"]
    variables += ["TruthParticles"]
    variables += ["TruthVertices"]
    variables += ["TruthElectrons"]
    variables += ["TruthMuons"]
    variables += ["TruthPrimaryVertices"]

    return variables

def HION7AllVarFromFTAG1():

    variables = []
    variables += ["InDetLargeD0TrackParticles"]
    variables += ["AntiKt4EMPFlowJets"]
    variables += ["AntiKt4UFOCSSKJets"]
    variables += ["CaloCalFwdTopoTowers"]
    variables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"]
    variables += ["UFOCSSK"]
    variables += ["GlobalChargedParticleFlowObjects"]
    variables += ["GlobalNeutralParticleFlowObjects"]
    variables += ["CHSGChargedParticleFlowObjects"]
    variables += ["CHSGNeutralParticleFlowObjects"]
    variables += ["JetAssociatedPixelClusters"]
    variables += ["JetAssociatedSCTClusters"]
    variables += ["PixelClusters"]
    variables += ["SCT_Clusters"]

    return variables

def HION7ExtraVarForBtag(JetColl):

    variables  = [JetColl+"AntiKt4HIJets.HadronConeExclTruthLabelID.HadronGhostTruthLabelID.GhostBHadronsFinal.GhostCHadronsFinal.GhostTausFinal.ConeExclBHadronsFinal.ConeExclCHadronsFinal.ConeExclTausFinal"]
    FTAG_AUXDATA = [
    'VxTrackAtVertex',
    'btagIp_d0Uncertainty',
    'btagIp_z0SinThetaUncertainty',
    'btagIp_z0SinTheta',
    'btagIp_d0',
    'btagIp_trackMomentum',
    'btagIp_trackDisplacement',
    'btagIp_invalidIp',
    'numberOfNextToInnermostPixelLayerHits',
    'numberOfInnermostPixelLayerSharedHits',
    'numberOfInnermostPixelLayerSplitHits',
    'numberOfPixelSplitHits',
    'leptonID',
    'TTVA_AMVFVertices',
    'TTVA_AMVFWeights',
    'ftagTruthParentBarcode',
    'ftagTruthOriginLabel',
    'ftagTruthTypeLabel',
    'ftagTruthVertexIndex',
    'ftagTruthSourceLabel',
    ]
    variables += [".".join(["InDetTrackParticles"] + FTAG_AUXDATA)]
    variables += ["PrimaryVertices.covariance"]

    return variables

def makeHIJetBasicBranchList():
    state_vars=["pt","eta","phi","m"]
    c=list(state_vars)
    states=["JetUnsubtractedScaleMomentum","JetSubtractedScaleMomentum"]
    for s in states:
        for v in state_vars:
            c.append(s+'_'+v)
    c+=['ConstituentScale',
        'constituentLinks',
        'constituentWeights',
        ]
    return c

def makeHIJetRemovedBranchList():
    state_vars=["pt","eta","phi","m"]
    c=list(state_vars)
    states=["JetEMScaleMomentum", "JetConstitScaleMomentum", "JetEtaJESScaleMomentum", "JetPileupScaleMomentum", "JetInsituScaleMomentum", "JetSubtractedOriginCorrectedScaleMomentum"]
    for s in states:
        for v in state_vars:
            c.append(s+'_'+v)
    return c

def makeHIJetBranchList():
    state_vars=["pt","eta","phi","m"]
    c=list(state_vars)
    states=["JetUnsubtractedScaleMomentum","JetSubtractedScaleMomentum"]
    for s in states:
        for v in state_vars:
            c.append(s+'_'+v)
    c+=['ConstituentScale',
        'constituentLinks',
        'constituentWeights',
        'AverageLArQF',
        'EMFrac',
        'FracSamplingMax',
        'FracSamplingMaxIndex',
        'HECFrac',
        'HECQuality',
        'LArQuality',
        'N90Constituents',
        'NegativeE',
        'Timing',
        'BchCorrCell',
        'LArBadHVEnergyFrac',
        'LArBadHVNCell',
        'EnergyPerSampling',
        'GhostAntiKt4HITrackJets',
        'GhostAntiKt4HITrackJetsCount',
        'GhostAntiKt4HITrackJetsPt',
        'GhostMuonSegmentCount',
        'GhostTrack',
        'NumTrkPt4000',
        'SumPtTrkPt4000',
        'TrackWidthPt4000',
        'Width',
        'MaxConstituentET',
        'MaxOverMean']
    return c

def HION7BasicJetVars(JetColl):
    #Only basic kinematics for small jets
    ExtraVars = []
    HIJetBranches=makeHIJetBranchList()
    for collection in [JetColl + "AntiKt2HIJets",JetColl + "AntiKt4HIJets"]:
        for j in HIJetBranches:
            ExtraVars.append(collection+'.'+j)

    return ExtraVars

def HION7ExtraContainersTrigger():
    variables  = ["HLT_MuonsCB_RoI",
                  "HLT_MuonsCB_RoIAux."]
    return variables

#################################################################################
#HION12

def HION12SmartCollections():
    variables  = []
    variables += ["Electrons"]             # Smart collection for electrons
    variables += ["Muons"]                  # Smart collection for muons
    variables += ["Photons"]                # Smart collection for photons
    variables += ["InDetTrackParticles"]    # Smart collection for tracks

    return variables

def HION12AllVarContent():
    variables  = []
    variables += ["AntiKt4EMPFlowJets"]                          # Include R = 0.4 anti-kt EM Particle Flow jets
    variables += ["AntiKt4EMTopoJets"]                           # Include R = 0.4 anti-kt EM topo-jets
    variables += ["AntiKt4LCTopoJets"]                           # Include R = 0.4 anti-kt local calibration topo-jets
    variables += ["CaloCalTopoClusters"]                         # Include topocluster information
    variables += ["PrimaryVertices"]                             # Include a list of all primary vertices
    variables += ["NCB_MuonSegments"]                            # Include the non-collision background muons to handle punch-throughs.
    variables += ["JetETMissChargedParticleFlowObjects"]         # Include the charged particle flow objects from the Jet/ET Miss group
    variables += ["JetETMissNeutralParticleFlowObjects"]         # Include the neutral particle flow objects from the Jet/ET Miss group
    variables += ["TauChargedParticleFlowObjects"]               # Include the charged particle flow objects used for Tau reconstruction
    variables += ["TauNeutralParticleFlowObjects"]               # Include the neutral particle flow objects used for Tau reconstruction
    variables += ["TauShotParticleFlowObjects"]                  # Include the["shot" particle flow objects used for Tau reconstruction
    variables += ["Kt4EMPFlowEventShape"]                        # The event shape specifically for R=0.4 EM PFlow jets
    variables += ["Kt4EMTopoOriginEventShape"]                   # The event shape specifically for R=0.4 EM Topo jets
    variables += ["Kt4LCTopoOriginEventShape"]                   # The event shape specifically for R=0.4 LC Topo jets
    variables += ["TopoClusterIsoCentralEventShape"]             # Part of the event shape for topo-jets
    variables += ["TopoClusterIsoVeryForwardEventShape"]         # Part of the event shape for topo-jets
    variables += ["TopoClusterIsoForwardEventShape"]             # Part of the event shape for topo-jets
    variables += ["NeutralParticleFlowIsoCentralEventShape"]     # Part of the event shape for PFlow jets
    variables += ["ParticleFlowIsoCentralEventShape"]            # Part of the event shape for PFlow jets
    variables += ["NeutralParticleFlowIsoForwardEventShape"]     # Part of the event shape for PFlow jets
    variables += ["ParticleFlowIsoForwardEventShape"]            # Part of the event shape for PFlow jets
    variables += ["HLT_xAOD__JetContainer_a4tcemsubjesISFS"]     # Include the HLT R = 0.4 EM Topo trigger jets (2018 HI Run)
    variables += ["HLT_xAOD__JetContainer_a4ionemsubjesISFS"]    # Include the HLT R = 0.4 heavy ion trigger jets (2015 HI Run)
    variables += ["HLT_xAOD__JetContainer_a10tclcwsubjesFS"]     # Include the HLT R = 1.0 LCW Topo trigger jets (2018 HI Run)

    return variables

def HION12HIJetBranches():
    state_vars  = []
    state_vars += ["pt"]
    state_vars += ["eta"]
    state_vars += ["phi"]
    state_vars += ["m"]

    states  = []
    states += ["JetUnsubtractedScaleMomentum"]
    states += ["JetSubtractedScaleMomentum"]

    HIJetBranches = []
    for v in state_vars:
        HIJetBranches += [v]
        for s in states:
            HIJetBranches += [s+'.'+v]

    HIJetBranches += ["ConstituentScale"]
    HIJetBranches += ["constituentLinks"]
    HIJetBranches += ["constituentWeights"]
    HIJetBranches += ["AverageLArQF"]
    HIJetBranches += ["EMFrac"]
    HIJetBranches += ["FracSamplingMax"]
    HIJetBranches += ["FracSamplingMaxIndex"]
    HIJetBranches += ["HECFrac"]
    HIJetBranches += ["HECQuality"]
    HIJetBranches += ["LArQuality"]
    HIJetBranches += ["N90Constituents"]
    HIJetBranches += ["NegativeE"]
    HIJetBranches += ["Timing"]
    HIJetBranches += ["BchCorrCell"]
    HIJetBranches += ["LArBadHVEnergyFrac"]
    HIJetBranches += ["LArBadHVNCell"]
    HIJetBranches += ["EnergyPerSampling"]
    HIJetBranches += ["GhostAntiKt4HITrackJets"]
    HIJetBranches += ["GhostAntiKt4HITrackJetsCount"]
    HIJetBranches += ["GhostAntiKt4HITrackJetsPt"]
    HIJetBranches += ["GhostMuonSegmentCount"]
    HIJetBranches += ["GhostTrack"]
    HIJetBranches += ["NumTrkPt4000"]
    HIJetBranches += ["SumPtTrkPt4000"]
    HIJetBranches += ["TrackWidthPt4000"]
    HIJetBranches += ["Width"]
    HIJetBranches += ["MaxConstituentET"]
    HIJetBranches += ["MaxOverMean"]
    
    return HIJetBranches

def HION12Extra():
    ExtraJets  = []
    ExtraJets += ["AntiKt4HITrackJets"]          # Include R = 0.4 Heavy Ion anti-kt track jets
    ExtraJets += ["AntiKt4HIJets"]               # Include R = 0.4 Heavy Ion anti-kt tower jets
    ExtraJets += ["AntiKt10HIJets"]              # Include R = 1.0 Heavy Ion anti-kt tower jets
    
    HIJetBranches = HION12HIJetBranches()
    
    variables  = []
    for collection in ExtraJets:
        for branch in HIJetBranches:
            variables += [collection+'.'+branch]

    variables += ['Muons.MuonSpectrometerPt'] 
    
    return variables

#################################################################################
#HION14
def HION14SmartCollections():
    variables  =  []
    variables += ["PrimaryVertices"]

    return variables

def HION14AllVariablesGeneral():
    variables  =  []
    variables += ["CaloSums"]
    variables += ["EventInfo"]

    return variables

def HION14ContentTracks():
    variables = []
    variables += ["InDetTrackParticles.qOverP"]
    variables += ["InDetTrackParticles.theta"]
    variables += ["InDetTrackParticles.phi"]
    variables += ["InDetTrackParticles.d0"]
    variables += ["InDetTrackParticles.z0"]
    variables += ["InDetTrackParticles.TrackQuality"]
    variables += ["InDetTrackParticles.HITight"]

    return variables

def HION14ContentMuons():
    variables = []
    variables += ["Muons.pt"]
    variables += ["Muons.eta"]
    variables += ["Muons.phi"]
    variables += ["Muons.truthType"]
    variables += ["Muons.truthOrigin"]
    variables += ["Muons.author"]
    variables += ["Muons.muonType"]
    variables += ["Muons.quality"]
    variables += ["Muons.inDetTrackParticleLink"]
    variables += ["Muons.muonSpectrometerTrackParticleLink"]
    variables += ["Muons.combinedTrackParticleLink"]
    variables += ["Muons.InnerDetectorPt"]
    variables += ["Muons.MuonSpectrometerPt"]
    variables += ["Muons.DFCommonGoodMuon"]
    variables += ["Muons.ptcone20"]
    variables += ["Muons.ptcone30"]
    variables += ["Muons.ptcone40"]
    variables += ["Muons.ptvarcone20"]
    variables += ["Muons.ptvarcone30"]
    variables += ["Muons.ptvarcone40"]
    variables += ["Muons.topoetcone20"]
    variables += ["Muons.topoetcone30"]
    variables += ["Muons.topoetcone40"]
    variables += ["Muons.truthParticleLink"]
    variables += ["Muons.charge"]
    variables += ["Muons.extrapolatedMuonSpectrometerTrackParticleLink"]
    variables += ["Muons.allAuthors"]
    variables += ["Muons.ptcone20_TightTTVA_pt1000"]
    variables += ["Muons.ptcone20_TightTTVA_pt500"]
    variables += ["Muons.ptvarcone30_TightTTVA_pt1000"]
    variables += ["Muons.ptvarcone30_TightTTVA_pt500"]
    variables += ["Muons.numberOfPrecisionLayers"]
    variables += ["Muons.combinedTrackOutBoundsPrecisionHits"]
    variables += ["Muons.numberOfPrecisionLayers"]
    variables += ["Muons.numberOfPrecisionHoleLayers"]
    variables += ["Muons.numberOfGoodPrecisionLayers"]
    variables += ["Muons.innerSmallHits"]
    variables += ["Muons.innerLargeHits"]
    variables += ["Muons.middleSmallHits"]
    variables += ["Muons.middleLargeHits"]
    variables += ["Muons.outerSmallHits"]
    variables += ["Muons.outerLargeHits"]
    variables += ["Muons.extendedSmallHits"]
    variables += ["Muons.extendedLargeHits"]
    variables += ["Muons.extendedSmallHoles"]
    variables += ["Muons.isSmallGoodSectors"]
    variables += ["Muons.cscUnspoiledEtaHits"]
    variables += ["Muons.EnergyLoss"]
    variables += ["Muons.energyLossType"]
    variables += ["Muons.momentumBalanceSignificance"]
    variables += ["Muons.scatteringCurvatureSignificance"]
    variables += ["Muons.scatteringNeighbourSignificance"]

    return variables

def HION14ContentCombinedMuonTrackParticles():
    variables = []
    variables += ["CombinedMuonTrackParticles.qOverP"]
    variables += ["CombinedMuonTrackParticles.d0"]
    variables += ["CombinedMuonTrackParticles.z0"]
    variables += ["CombinedMuonTrackParticles.vz"]
    variables += ["CombinedMuonTrackParticles.phi"]
    variables += ["CombinedMuonTrackParticles.theta"]
    variables += ["CombinedMuonTrackParticles.definingParametersCovMatrix"]
    variables += ["CombinedMuonTrackParticles.numberOfPixelDeadSensors"]
    variables += ["CombinedMuonTrackParticles.numberOfPixelHits"]
    variables += ["CombinedMuonTrackParticles.numberOfPixelHoles"]
    variables += ["CombinedMuonTrackParticles.numberOfSCTDeadSensors"]
    variables += ["CombinedMuonTrackParticles.numberOfSCTHits"]
    variables += ["CombinedMuonTrackParticles.numberOfSCTHoles"]
    variables += ["CombinedMuonTrackParticles.numberOfTRTHits"]
    variables += ["CombinedMuonTrackParticles.numberOfTRTOutliers"]
    variables += ["CombinedMuonTrackParticles.chiSquared"]
    variables += ["CombinedMuonTrackParticles.numberDoF"]

    return variables

def HION14ExtraCombinedMuonTrackParticlesTruth():
    variables = []
    variables += ["CombinedMuonTrackParticles.truthOrigin"]
    variables += ["CombinedMuonTrackParticles.truthType"]

    return variables

def HION14ContentExtrapolatedMuonTrackParticles():
    variables = []
    variables += ["ExtrapolatedMuonTrackParticles.d0"]
    variables += ["ExtrapolatedMuonTrackParticles.z0"]
    variables += ["ExtrapolatedMuonTrackParticles.vz"]
    variables += ["ExtrapolatedMuonTrackParticles.definingParametersCovMatrix"]
    variables += ["ExtrapolatedMuonTrackParticles.truthOrigin"]
    variables += ["ExtrapolatedMuonTrackParticles.truthType"]
    variables += ["ExtrapolatedMuonTrackParticles.qOverP"]
    variables += ["ExtrapolatedMuonTrackParticles.theta"]
    variables += ["ExtrapolatedMuonTrackParticles.phi"]

    return variables

def HION14ExtraExtrapolatedMuonTrackParticlesTruth():
    variables = []
    variables += ["ExtrapolatedMuonTrackParticles.truthOrigin"]
    variables += ["ExtrapolatedMuonTrackParticles.truthType"]

    return variables

def HION14ContentMuonSpectrometerTrackParticles():
    variables = []
    variables += ["MuonSpectrometerTrackParticles.phi"]
    variables += ["MuonSpectrometerTrackParticles.d0"]
    variables += ["MuonSpectrometerTrackParticles.z0"]
    variables += ["MuonSpectrometerTrackParticles.vz"]
    variables += ["MuonSpectrometerTrackParticles.definingParametersCovMatrix"]
    variables += ["MuonSpectrometerTrackParticles.vertexLink"]
    variables += ["MuonSpectrometerTrackParticles.theta"]
    variables += ["MuonSpectrometerTrackParticles.qOverP"]
    variables += ["MuonSpectrometerTrackParticles.truthParticleLink"]

    return variables

def HION14ExtraMuonSpectrometerTrackParticlesTruth():
    variables = []
    variables += ["MuonSpectrometerTrackParticles.truthParticleLink"]
    
    return variables

def HION14TruthVariablesGeneral():
    variables = []
    variables += ["TruthEvents"]

    return variables

def HION14ContentTruthParticles():
    variables = []
    variables += ["TruthParticles.pdgId"]
    variables += ["TruthParticles.uid"]
    variables += ["TruthParticles.status"]
    variables += ["TruthParticles.m"]
    variables += ["TruthParticles.e"]
    variables += ["TruthParticles.py"]
    variables += ["TruthParticles.px"]
    variables += ["TruthParticles.pz"]

    return variables

def HION14ExtraContentAll():
    variables  = []
    variables += HION14ContentTracks()
    variables += HION14ContentMuons()
    variables += HION14ContentCombinedMuonTrackParticles()
    variables += HION14ContentExtrapolatedMuonTrackParticles()
    variables += HION14ContentMuonSpectrometerTrackParticles()
    variables += HION14ContentTracks()

    return variables

def HION14ExtraContentAllTruth():
    variables  = []
    variables += HION4ExtraMuonsTruth()
    variables += HION14ExtraCombinedMuonTrackParticlesTruth()
    variables += HION14ExtraExtrapolatedMuonTrackParticlesTruth()
    variables += HION14ExtraMuonSpectrometerTrackParticlesTruth()
    variables += HION14ContentTruthParticles()

    return variables

#################################################################################
#HIONHPOD

def HIONHPODSmartCollections():
    variables  = []
    variables += ["Electrons"]
    variables += ["Photons"]
    variables += ["Muons"]

    return variables

#################################################################################
#HION15

def HION15SmartCollections():
    return HION7SmartCollections()

def HION15AllVarContent():
    return HION7AllVarContent()

def HION15AllVarTruthContent():
    return HION7AllVarTruthContent()

def HION15AllVarFromFTAG1():
    return HION7AllVarFromFTAG1()

def HION15ExtraVarForBtag(JetColl):
    return HION7ExtraVarForBtag(JetColl)

def HION15BasicJetVars(JetColl):
    return HION7BasicJetVars(JetColl)

def HION15ExtraContainersTrigger():
    return HION7ExtraContainersTrigger()
