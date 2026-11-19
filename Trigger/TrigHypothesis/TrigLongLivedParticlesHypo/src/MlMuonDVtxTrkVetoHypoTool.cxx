/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MlMuonDVtxTrkVetoHypoTool.h"

#include "CxxUtils/phihelper.h"

#include <algorithm>
#include <cmath>

using namespace TrigCompositeUtils;

MlMuonDVtxTrkVetoHypoTool::MlMuonDVtxTrkVetoHypoTool(const std::string& type,
                                                     const std::string& name,
                                                     const IInterface* parent )
    : AthAlgTool( type, name, parent ),
    m_decisionId( HLT::Identifier::fromToolName( name ) ) {}

unsigned int MlMuonDVtxTrkVetoHypoTool::nPromptTracks(const SegCluster& cl, const Info& info) const {
    unsigned int n {0};
    for (const xAOD::TrackParticle* trk : *info.tracks) {
        if (trk->pt() < m_minTrackPt) continue;
        const float dEta = trk->eta() - cl.eta;
        const float dPhi = CxxUtils::deltaPhi<float>(trk->phi(), cl.phi);
        if (std::hypot(dEta, dPhi) > m_maxTrackDR) continue;
        // Without a primary vertex the z requirement cannot be applied: every track counts
        if (m_maxDz0SinTheta >= 0 && info.hasPV &&
            std::abs((trk->z0() + trk->vz() - info.zPV) * std::sin(trk->theta())) > m_maxDz0SinTheta) continue;
        ++n;
    }
    return n;
}

StatusCode MlMuonDVtxTrkVetoHypoTool::decide(Info& info)  const {

    if (info.activeChainIDs.count(m_decisionId.numeric()) == 0) {
        ATH_MSG_DEBUG("Chain ID " << m_decisionId.numeric() << " not active for this event, skipping");
        return StatusCode::SUCCESS;
    }

    bool pass = m_acceptAll;
    const std::size_t nTry = m_onlyLeadingCluster ? 1 : m_maxClusters.value();
    for (std::size_t k = 0; k < std::min(nTry, info.clusters->size()) && !pass; ++k) {
        const SegCluster& cl = info.clusters->at(k);
        if (cl.nSeg < m_minClusterSegments) break;  // sorted: the next ones have even fewer segments
        const unsigned int nTrk = nPromptTracks(cl, info);
        ATH_MSG_DEBUG("Cluster eta=" << cl.eta << " phi=" << cl.phi << " nSeg=" << cl.nSeg << ": " << nTrk << " prompt tracks");
        pass = (nTrk == 0);
    }

    if (!pass) {
        ATH_MSG_DEBUG("Failing prompt-track veto");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Passing prompt-track veto");
    addDecisionID(m_decisionId.numeric(), info.outDecision);

    return StatusCode::SUCCESS;
}
