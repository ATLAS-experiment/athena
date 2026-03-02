/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  Test algorithm: prints variables exactly as extracted in AsgForwardElectronSelectorTool::getInputs()
*/

#include "ElectronPhotonSelectorTools/ForwardElectronToolsTestAlg.h"
#include "StoreGate/ReadHandle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include <cmath>

ForwardElectronToolsTestAlg::ForwardElectronToolsTestAlg(
    const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{}

StatusCode ForwardElectronToolsTestAlg::initialize()
{
    ATH_CHECK(m_electronKey.initialize());
    ATH_CHECK(m_truthKey.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_looseTool.retrieve());
    ATH_CHECK(m_mediumTool.retrieve());
    ATH_CHECK(m_tightTool.retrieve());
    ATH_MSG_INFO("ForwardElectronToolsTestAlg initialised");
    return StatusCode::SUCCESS;
}

static double deltaR(double eta1, double phi1, double eta2, double phi2) {
    double deta = eta1 - eta2;
    double dphi = phi1 - phi2;
    while (dphi >  M_PI) dphi -= 2*M_PI;
    while (dphi < -M_PI) dphi += 2*M_PI;
    return std::sqrt(deta*deta + dphi*dphi);
}

StatusCode ForwardElectronToolsTestAlg::execute()
{
    ++m_nEvents;
    const EventContext& ctx = getContext();

    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electronKey, ctx);
    if (!electrons.isValid()) return StatusCode::SUCCESS;

    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_truthKey, ctx);

    // Skip events with no forward electrons
    int nFwd = 0;
    for (const xAOD::Electron* el : *electrons) {
        const xAOD::CaloCluster* cl = el->caloCluster();
        if (cl && std::abs(cl->eta()) > 2.5 && std::abs(cl->eta()) <= 4.0) ++nFwd;
    }
    if (nFwd == 0) return StatusCode::SUCCESS;

    for (const xAOD::Electron* el : *electrons) {
        const xAOD::CaloCluster* cluster = el->caloCluster();
        if (!cluster) continue;
        const double absEta = std::abs(cluster->eta());
        if (absEta <= 2.5 || absEta > 4.0) continue;

        const xAOD::TrackParticle* track = el->trackParticle();
        if (!track) continue;

        ++m_nElectrons;

        // --- Tool outputs ---
        const double calibPt  = m_calibTool->calibrate(ctx, el);
        const double score    = m_mediumTool->calculate(ctx, el);
        const bool   isLoose  = static_cast<bool>(m_looseTool->accept(ctx, el));
        const bool   isMedium = static_cast<bool>(m_mediumTool->accept(ctx, el));
        const bool   isTight  = static_cast<bool>(m_tightTool->accept(ctx, el));
        if (isLoose)  ++m_nLoose;
        if (isMedium) ++m_nMedium;
        if (isTight)  ++m_nTight;

        // --- Truth match ---
        double truthPt = -1.;
        double bestDR  = 0.3;
        if (truthParticles.isValid()) {
            for (const xAOD::TruthParticle* tp : *truthParticles) {
                if (!tp || std::abs(tp->pdgId()) != 11 || tp->status() != 1) continue;
                double dr = deltaR(cluster->eta(), cluster->phi(), tp->eta(), tp->phi());
                if (dr < bestDR) { bestDR = dr; truthPt = tp->pt() / 1000.; }
            }
        }

        // =====================================================================
        // Extract variables exactly as in AsgForwardElectronSelectorTool::getInputs()
        // =====================================================================

        // x1, x2 = calo eta and phi
        double calo_eta = cluster->eta();
        double calo_phi = cluster->phi();

        // x3, x4 = track eta and phi
        double track_eta = track->eta();
        double track_phi = track->phi();

        // x5 = HGTD time
        double hgtd_time = track->time();

        // x6, x7 = ITk hit counts
        double pixels = el->trackParticleSummaryIntValue(xAOD::numberOfPixelHits);
        double strips = el->trackParticleSummaryIntValue(xAOD::numberOfSCTHits);

        // x8 to x14 = 7 calorimeter shower-shape moments
        auto getMom = [&](xAOD::CaloCluster::MomentType type) -> double {
            double val = 0.;
            cluster->retrieveMoment(type, val);
            return val;
        };
        double ENG_FRAC_MAX    = getMom(xAOD::CaloCluster::ENG_FRAC_MAX);
        double LONGITUDINAL    = getMom(xAOD::CaloCluster::LONGITUDINAL);
        double SECOND_LAMBDA   = getMom(xAOD::CaloCluster::SECOND_LAMBDA);
        double LATERAL         = getMom(xAOD::CaloCluster::LATERAL);
        double SECOND_R        = getMom(xAOD::CaloCluster::SECOND_R);
        double CENTER_LAMBDA   = getMom(xAOD::CaloCluster::CENTER_LAMBDA);
        double SECOND_ENG_DENS = getMom(xAOD::CaloCluster::SECOND_ENG_DENS);

        // x15 to x18 = track-calo matching
        auto getMatch = [&](xAOD::EgammaParameters::TrackCaloMatchType type) -> float {
            float val = 0.f;
            el->trackCaloMatchValue(val, type);
            return val;
        };
        double delta_eta2           = getMatch(xAOD::EgammaParameters::deltaEta2);
        double delta_phi2           = getMatch(xAOD::EgammaParameters::deltaPhi2);
        double delta_phi_rescaled2  = getMatch(xAOD::EgammaParameters::deltaPhiRescaled2);
        double delta_phi_last       = getMatch(xAOD::EgammaParameters::deltaPhiFromLastMeasurement);

        // x19-x25 - energy fractions (exactly as in getInputs())
        const double caloE = cluster->e();
        const double inv_E = (caloE != 0.) ? 1. / caloE : 0.;
        using CS = CaloSampling::CaloSample;

        double frac_EM_1  = cluster->energyBE(1) * inv_E;
        double frac_EM_2  = cluster->energyBE(2) * inv_E;
        double frac_EM_3  = cluster->energyBE(3) * inv_E;
        double frac_HAD_0 = cluster->eSample(static_cast<CS>(CaloSampling::HEC0)) * inv_E;
        double frac_HAD_1 = (cluster->eSample(static_cast<CS>(CaloSampling::HEC1)) +
                             cluster->eSample(static_cast<CS>(CaloSampling::FCAL0))) * inv_E;
        double frac_HAD_2 = (cluster->eSample(static_cast<CS>(CaloSampling::HEC2)) +
                             cluster->eSample(static_cast<CS>(CaloSampling::FCAL1))) * inv_E;
        double frac_HAD_3 = (cluster->eSample(static_cast<CS>(CaloSampling::HEC3)) +
                             cluster->eSample(static_cast<CS>(CaloSampling::FCAL2))) * inv_E;

        double calo_pt = el->pt();

        // =====================================================================
        // Print in parseable format
        // =====================================================================
        ATH_MSG_INFO("FWDEL_START");
        ATH_MSG_INFO("  raw_pT=" << calo_pt/1000.
            << " calib_pT=" << calibPt/1000.
            << " truth_pT=" << truthPt
            << " score_cpp=" << score
            << " L=" << isLoose << " M=" << isMedium << " T=" << isTight);
        ATH_MSG_INFO("  calo_eta=" << calo_eta
            << " calo_phi=" << calo_phi
            << " track_eta=" << track_eta
            << " track_phi=" << track_phi
            << " time=" << hgtd_time
            << " pixels=" << pixels
            << " strips=" << strips);
        ATH_MSG_INFO("  ENG_FRAC_MAX=" << ENG_FRAC_MAX
            << " LONGITUDINAL=" << LONGITUDINAL
            << " SECOND_LAMBDA=" << SECOND_LAMBDA
            << " LATERAL=" << LATERAL
            << " SECOND_R=" << SECOND_R
            << " CENTER_LAMBDA=" << CENTER_LAMBDA
            << " SECOND_ENG_DENS=" << SECOND_ENG_DENS);
        ATH_MSG_INFO("  delta_eta2=" << delta_eta2
            << " delta_phi2=" << delta_phi2
            << " delta_phi_rescaled2=" << delta_phi_rescaled2
            << " delta_phi_last=" << delta_phi_last);
        ATH_MSG_INFO("  calo_frac_EM_1=" << frac_EM_1
            << " calo_frac_EM_2=" << frac_EM_2
            << " calo_frac_EM_3=" << frac_EM_3
            << " calo_frac_HAD_0=" << frac_HAD_0
            << " calo_frac_HAD_1=" << frac_HAD_1
            << " calo_frac_HAD_2=" << frac_HAD_2
            << " calo_frac_HAD_3=" << frac_HAD_3);
        ATH_MSG_INFO("  calo_pt=" << calo_pt);
        ATH_MSG_INFO("FWDEL_END");
    }
    return StatusCode::SUCCESS;
}

StatusCode ForwardElectronToolsTestAlg::finalize()
{
    const double n = m_nElectrons > 0 ? static_cast<double>(m_nElectrons) : 1.;
    ATH_MSG_INFO("========================================");
    ATH_MSG_INFO("Events processed  : " << m_nEvents);
    ATH_MSG_INFO("Forward electrons : " << m_nElectrons);
    ATH_MSG_INFO("Pass Loose  (90%) : " << m_nLoose  << " (" << 100.*m_nLoose/n  << "%)");
    ATH_MSG_INFO("Pass Medium (80%) : " << m_nMedium << " (" << 100.*m_nMedium/n << "%)");
    ATH_MSG_INFO("Pass Tight  (70%) : " << m_nTight  << " (" << 100.*m_nTight/n  << "%)");
    ATH_MSG_INFO("========================================");
    return StatusCode::SUCCESS;
}

DECLARE_COMPONENT(ForwardElectronToolsTestAlg)