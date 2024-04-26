# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# ListSlimming.py - List of collections for slimming from athena 21.2 HION4 and EGAM1ExtraContent.py

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
