# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

TauJetsLRTCPContent = [
    "InDetLargeD0TrackParticles",
    "InDetLargeD0TrackParticlesAux.phi.vertexLink.theta.qOverP.truthParticleLink.truthMatchProbability",
]

from DerivationFrameworkTau.TauJetsCPContent import TauJetsCPContent

TauJetsLRTCPContent = []
for item in TauJetsCPContent:
    lrtitem = item.replace("TauJets", "TauJetsLRT")\
                  .replace("TauTracks", "TauTracksLRT")\
                  .replace("TauSecondaryVertices", "TauSecondaryVerticesLRT")\
                  .replace("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjectsLRT")
    TauJetsLRTCPContent.append(lrtitem)
