# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from DerivationFrameworkTau.TauJetsCPContent import TauJetsCPContent

TauJets_EleRMCPContent = []
for item in TauJetsCPContent:
    ermitem = item  .replace("TauJets", "TauJets_EleRM")\
                    .replace("TauTracks", "TauTracks_EleRM")\
                    .replace("TauSecondaryVertices", "TauSecondaryVertices_EleRM")\
                    .replace("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects_EleRM")
    TauJets_EleRMCPContent.append(ermitem)


TauJets_MuonRMCPContent = []
for item in TauJetsCPContent:
    murmitem = item .replace("TauJets", "TauJets_MuonRM")\
                    .replace("TauTracks", "TauTracks_MuonRM")\
                    .replace("TauSecondaryVertices", "TauSecondaryVertices_MuonRM")\
                    .replace("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects_MuonRM")
    if murmitem.startswith('TauJets_MuonRMAux') :
        murmitem += '.originalTauJet.removedMuons'
    TauJets_MuonRMCPContent.append(murmitem)

