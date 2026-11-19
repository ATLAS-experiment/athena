/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MlMuonDVtxTrkVetoHypoAlg.h"

#include "CxxUtils/phihelper.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace TrigCompositeUtils;
using SegCluster = MlMuonDVtxTrkVetoHypoTool::SegCluster;

StatusCode MlMuonDVtxTrkVetoHypoAlg::initialize()
{
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_trackKey.initialize());
  ATH_CHECK(m_vertexKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  ATH_CHECK(m_hypoTools.retrieve());
  return StatusCode::SUCCESS;
}

std::vector<SegCluster> MlMuonDVtxTrkVetoHypoAlg::clusterSegments(const xAOD::MuonSegmentContainer& segments) const {
    const std::size_t n = segments.size();
    std::vector<float> eta(n), phi(n);
    for (std::size_t i = 0; i < n; ++i) {
        const Amg::Vector3D pos = segments[i]->position();
        eta[i] = pos.eta();
        phi[i] = pos.phi();
    }
    auto close = [&](std::size_t i, std::size_t j) {
        return std::hypot(eta[i] - eta[j], CxxUtils::deltaPhi(phi[i], phi[j])) < m_clusterDR;
    };
    // Seeds in order of decreasing number of neighbours; each segment joins the first seed that reaches it
    std::vector<unsigned int> nNeighbours(n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            if (close(i, j)) { ++nNeighbours[i]; ++nNeighbours[j]; }
        }
    }
    std::vector<std::size_t> order(n);
    for (std::size_t i = 0; i < n; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return nNeighbours[a] > nNeighbours[b]; });

    std::vector<bool> used(n, false);
    std::vector<SegCluster> clusters;
    for (std::size_t seed : order) {
        if (used[seed]) continue;
        float sumEta {0.f}, sumCos {0.f}, sumSin {0.f};
        SegCluster cl;
        for (std::size_t j = 0; j < n; ++j) {
            if (used[j] || !close(seed, j)) continue;
            used[j] = true;
            sumEta += eta[j];
            sumCos += std::cos(phi[j]);
            sumSin += std::sin(phi[j]);
            ++cl.nSeg;
        }
        cl.eta = sumEta / cl.nSeg;
        cl.phi = std::atan2(sumSin, sumCos);
        clusters.push_back(cl);
    }
    std::stable_sort(clusters.begin(), clusters.end(), [](const SegCluster& a, const SegCluster& b) { return a.nSeg > b.nSeg; });
    return clusters;
}

void MlMuonDVtxTrkVetoHypoAlg::dumpEvent(const EventContext& ctx, const MlMuonDVtxTrkVetoHypoTool::Info& info,
                                         unsigned int nSegments) const {
    const unsigned long long evt = ctx.eventID().event_number();
    ATH_MSG_DEBUG("MLVTXTRK EVT event=" << evt << " nseg=" << nSegments << " nclus=" << info.clusters->size()
                  << " ntrk=" << info.tracks->size() << " haspv=" << info.hasPV << " zpv=" << info.zPV);
    for (std::size_t k = 0; k < std::min<std::size_t>(m_dumpMaxClusters, info.clusters->size()); ++k) {
        const SegCluster& cl = info.clusters->at(k);
        ATH_MSG_DEBUG("MLVTXTRK CL event=" << evt << " icl=" << k << " eta=" << cl.eta << " phi=" << cl.phi << " nseg=" << cl.nSeg);
        for (const xAOD::TrackParticle* trk : *info.tracks) {
            if (trk->pt() < m_dumpMinTrackPt) continue;
            const float dr = std::hypot(trk->eta() - cl.eta, CxxUtils::deltaPhi<float>(trk->phi(), cl.phi));
            if (dr > m_dumpMaxTrackDR) continue;
            const float dz = info.hasPV ? (trk->z0() + trk->vz() - info.zPV) * std::sin(trk->theta()) : std::nanf("");
            std::ostringstream os;
            os << std::setprecision(4) << "MLVTXTRK TRK event=" << evt << " icl=" << k << " pt=" << trk->pt() * 1e-3
               << " eta=" << trk->eta() << " dr=" << dr << " dz0sin=" << dz << " d0=" << trk->d0();
            ATH_MSG_DEBUG(os.str());
        }
    }
}

StatusCode MlMuonDVtxTrkVetoHypoAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG ( "Executing " << name() << "..." );

    // Obtain incoming Decision Objects (one per event, from the ML score step)
    auto previousDecisionsHandle = SG::makeHandle( decisionInput(), ctx );
    ATH_CHECK( previousDecisionsHandle.isValid() );
    ATH_MSG_DEBUG("Running with "<<previousDecisionsHandle->size()<<" previous decisions");

    // New output trigger decisions container
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), ctx );
    auto decisions = outputHandle.ptr();

    if (previousDecisionsHandle->size() == 0) {
        ATH_MSG_DEBUG( "No previous decision, nothing to do.");
        return StatusCode::SUCCESS;
    }

    const xAOD::MuonSegmentContainer* segments {nullptr};
    ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
    const xAOD::TrackParticleContainer* tracks {nullptr};
    ATH_CHECK(SG::get(tracks, m_trackKey, ctx));
    const xAOD::VertexContainer* vertices {nullptr};
    ATH_CHECK(SG::get(vertices, m_vertexKey, ctx));

    const std::vector<SegCluster> clusters = clusterSegments(*segments);

    // Hard-scatter vertex: the first primary vertex (vertices are sorted by sum pT^2)
    const auto pv = std::find_if(vertices->begin(), vertices->end(),
                                 [](const xAOD::Vertex* v) { return v->vertexType() == xAOD::VxType::PriVtx; });

    // Output object with the DV direction, linked as the feature of the decision
    SG::WriteHandle<xAOD::TrigCompositeContainer> outputTC{m_outputKey, ctx};
    ATH_CHECK(outputTC.record(std::make_unique<xAOD::TrigCompositeContainer>(),
                              std::make_unique<xAOD::TrigCompositeAuxContainer>()));
    xAOD::TrigComposite* dvDir = new xAOD::TrigComposite();
    outputTC->push_back(dvDir);
    dvDir->setDetail<unsigned int>("NClusters", clusters.size());
    dvDir->setDetail<float>("Eta", clusters.empty() ? 0.f : clusters.front().eta);
    dvDir->setDetail<float>("Phi", clusters.empty() ? 0.f : clusters.front().phi);
    dvDir->setDetail<unsigned int>("NSegments", clusters.empty() ? 0 : clusters.front().nSeg);

    DecisionIDContainer chainIDs;
    Decision* newDecision = newDecisionIn(decisions, hypoAlgNodeName());
    newDecision->setObjectLink(featureString(), ElementLink<xAOD::TrigCompositeContainer>(*outputTC, 0, ctx));
    for (const Decision* previousDecision : *previousDecisionsHandle) {
        linkToPrevious(newDecision, previousDecision, ctx);
        decisionIDs(previousDecision, chainIDs);
    }

    MlMuonDVtxTrkVetoHypoTool::Info info{newDecision, &clusters, tracks,
                                         pv != vertices->end(),
                                         pv != vertices->end() ? static_cast<float>((*pv)->z()) : 0.f,
                                         std::move(chainIDs)};

    if (msgLvl(MSG::DEBUG)) dumpEvent(ctx, info, segments->size());

    for (const auto& hypoTool : m_hypoTools) {
        ATH_CHECK(hypoTool->decide(info));
    }

    ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
    return StatusCode::SUCCESS;
}
