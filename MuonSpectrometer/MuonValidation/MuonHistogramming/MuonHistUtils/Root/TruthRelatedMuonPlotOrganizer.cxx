/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHistUtils/TruthRelatedMuonPlotOrganizer.h"

#include "AthContainers/ConstAccessor.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

typedef ElementLink<xAOD::TruthParticleContainer> TruthLink;

namespace Muon {

TruthRelatedMuonPlotOrganizer::TruthRelatedMuonPlotOrganizer(
    PlotBase* pParent, const std::string& sDir, bool doBinnedResolutionPlots,
    std::vector<int> selPlots)
    : PlotBase(pParent, sDir)
// Truth related plots
{

    if (selPlots.empty()) {
        for (unsigned int i = 0; i < MAX_TRUTHRELATEDPLOTCLASS; i++) {
            m_selPlots.push_back(i);
        }
    } else {
        m_selPlots = std::move(selPlots);
    }

    for (auto p : m_selPlots) {
        switch (p) {
            case TRK_MATCHEDTRUE:
                m_oMatchedPlots = std::make_unique<Trk::ParamPlots>(
                    this, "/kinematics/", "Matched Muons");
                break;
            case TRK_MATCHEDRECO:
                m_oMatchedRecoPlots = std::make_unique<Trk::ParamPlots>(
                    this, "/kinematicsReco/", "Matched Muons");
                break;
            case TRK_MSHITDIFF:
                m_oMSHitDiffPlots =
                    std::make_unique<Trk::MSHitDiffPlots>(this, "/hits/");
                break;
            case MUON_HITDIFF:
                m_oMuonHitDiffSummaryPlots =
                    std::make_unique<Muon::MuonHitDiffSummaryPlots>(this,
                                                                    "/hits/");
                break;
            case MUON_TRUTHHIT:
                m_oMuonTruthHitPlots =
                    std::make_unique<Muon::MuonTruthHitPlots>(this,
                                                              "/truthHits/");
                break;
            case MUON_RESOL:
                m_oMuonResolutionPlots =
                    std::make_unique<Muon::MuonResolutionPlots>(
                        this, "/resolution/", "", doBinnedResolutionPlots);
                break;
            case TRK_DEFPARAMPULLS:
                m_oDefParamPullPlots = std::make_unique<Trk::DefParamPullPlots>(
                    this, "/pulls/", "");
                break;
            case MUON_PULLSTAIL:
                m_oMomentumTruthPullPlots_Tail =
                    std::make_unique<Muon::MomentumTruthPullPlots>(
                        this, "/momentumPulls/", "Tail");
                break;
            case MUON_PULLSNOTAIL:
                m_oMomentumTruthPullPlots_NoTail =
                    std::make_unique<Muon::MomentumTruthPullPlots>(
                        this, "/momentumPulls/", "NoTail");
                break;
            case MUON_PARAMELOSS:
                m_oMatchedRecoElossPlots =
                    std::make_unique<Muon::MuonParamElossPlots>(this,
                                                                "/Eloss/");
                break;
        }
    }
}
TruthRelatedMuonPlotOrganizer::~TruthRelatedMuonPlotOrganizer() = default;

void TruthRelatedMuonPlotOrganizer::fill(
    const xAOD::TruthParticle& truthMu, const xAOD::Muon& mu, float weight) {
    if (m_oMatchedPlots) {
        m_oMatchedPlots->fill(truthMu, weight);
    }
    if (m_oMuonHitDiffSummaryPlots) {
        m_oMuonHitDiffSummaryPlots->fill(mu, truthMu, weight);
    }
    if (m_oMuonTruthHitPlots) {
        m_oMuonTruthHitPlots->fill(mu, weight);
    }

    // for eloss
    if (m_oMatchedRecoElossPlots) {
        m_oMatchedRecoElossPlots->fill(truthMu, mu, weight);
    }

    // Tracking related plots
    const xAOD::TrackParticle* primaryTrk =
        mu.trackParticle(xAOD::Muon::Primary);
    // const xAOD::TrackParticle* meTrk =
    // mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);

    if (!primaryTrk) {
        return;
    }
    if (m_oMatchedRecoPlots) {
        m_oMatchedRecoPlots->fill(*primaryTrk, weight);
    }
    if (m_oMSHitDiffPlots) {
        m_oMSHitDiffPlots->fill(*primaryTrk, truthMu, weight);
    }
    if (m_oMuonResolutionPlots) {
        m_oMuonResolutionPlots->fill(*primaryTrk, truthMu, weight);
    }
    if (m_oDefParamPullPlots) {
        m_oDefParamPullPlots->fill(*primaryTrk, truthMu, weight);
    }

    if (m_oMomentumTruthPullPlots_NoTail || m_oMomentumTruthPullPlots_Tail) {
        // muon spectrometer track at MS entry (not extrapolated)
        const xAOD::TrackParticle* msTrk = mu.trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle);
 #ifndef XAOD_ANALYSIS
        float eloss = 0;
        if (mu.parameter(eloss, xAOD::Muon::ParamDef::EnergyLoss)) {
            if (mu.energyLossType() != xAOD::Muon::EnergyLossType::Tail) {  // to test MEASURED energy loss
                if (m_oMomentumTruthPullPlots_NoTail) {
                    m_oMomentumTruthPullPlots_NoTail->fill(mu, msTrk, truthMu,
                                                           weight);
                }
            } else {
                if (m_oMomentumTruthPullPlots_Tail) {
                    m_oMomentumTruthPullPlots_Tail->fill(
                        mu, msTrk, truthMu,
                        weight);  // to test PARAMETRIZED energy loss
                }
            }
        }
#endif  // not XAOD_ANALYSIS
    }
}

void TruthRelatedMuonPlotOrganizer::fill(const xAOD::TruthParticle& truthMu,
                                         const xAOD::TrackParticle& muTP,
                                         float weight) {
    // Tracking related plots
    if (m_oMatchedPlots) {
        m_oMatchedPlots->fill(truthMu, weight);
    }
    if (m_oMatchedRecoPlots) {
        m_oMatchedRecoPlots->fill(muTP, weight);
    }
    if (m_oDefParamPullPlots) {
        m_oDefParamPullPlots->fill(muTP, truthMu, weight);
    }
    if (m_oMuonResolutionPlots) {
        m_oMuonResolutionPlots->fill(muTP, truthMu, weight);
    }
    if (m_oMSHitDiffPlots) {
        m_oMSHitDiffPlots->fill(muTP, truthMu, weight);
    }
}

}  // namespace Muon
