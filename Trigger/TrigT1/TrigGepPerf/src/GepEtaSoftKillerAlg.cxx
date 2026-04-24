/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "GepEtaSoftKillerAlg.h"

#include "xAODCaloEvent/CaloClusterAuxContainer.h"

#include <vector>
#include <algorithm>
#include <cmath>

GepEtaSoftKillerAlg::GepEtaSoftKillerAlg(const std::string& name,
                                           ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode GepEtaSoftKillerAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << "...");
    ATH_CHECK(m_inputClustersKey.initialize());
    ATH_CHECK(m_outputClustersKey.initialize());
    return StatusCode::SUCCESS;
}


StatusCode GepEtaSoftKillerAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    auto h_input = SG::makeHandle(m_inputClustersKey, ctx);
    ATH_CHECK(h_input.isValid());
    const auto& inputClusters = *h_input;

    ATH_MSG_DEBUG("Read in " << inputClusters.size() << " input clusters/towers");

    // Build the eta-phi grid
    const double minEta = -m_etaMax;
    const double maxEta =  m_etaMax;
    const double twopi  = 2.0 * M_PI;

    int neta = std::max(static_cast<int>((maxEta - minEta) / m_gridEtaSize + 0.5), 1);
    double deta = (maxEta - minEta) / neta;

    int nphi = static_cast<int>(twopi / m_gridPhiSize + 0.5);
    double dphi = twopi / nphi;

    std::vector<double> eta_bins(neta + 1);
    const double minPhi = -M_PI;
    std::vector<double> phi_bins(nphi + 1);

    for (int i = 0; i <= neta; ++i) {
        eta_bins[i] = minEta + i * deta;
    }
    for (int i = 0; i <= nphi; ++i) {
        phi_bins[i] = minPhi + i * dphi;
    }

    // Grid: each cell holds pointers to its clusters
    std::vector<std::vector<std::vector<const xAOD::CaloCluster*>>>
        grid(neta, std::vector<std::vector<const xAOD::CaloCluster*>>(nphi));

    for (const auto* cluster : inputClusters) {
        double eta = cluster->eta();
        double phi = cluster->phi();

        int eta_bin = static_cast<int>(
            std::distance(eta_bins.begin(),
                          std::upper_bound(eta_bins.begin(), eta_bins.end(), eta))) - 1;
        int phi_bin = static_cast<int>(
            std::distance(phi_bins.begin(),
                          std::upper_bound(phi_bins.begin(), phi_bins.end(), phi))) - 1;

        if (eta_bin >= 0 && eta_bin < neta && phi_bin >= 0 && phi_bin < nphi) {
            grid[eta_bin][phi_bin].push_back(cluster);
        }
    }

    // Compute the median of max-pT values per eta-band
    // Empty grid cells contribute 0.0 (consistent with FastJet SoftKiller)
    const int bandWidth = m_etaBandWidth;
    int nBands = (neta + bandWidth - 1) / bandWidth;
    std::vector<double> medianPerBand(nBands, 0.0);

    for (int band = 0; band < nBands; ++band) {
        std::vector<double> maxPtValues;
        int etaStart = band * bandWidth;
        int etaEnd   = std::min(etaStart + bandWidth, neta);

        for (int j = 0; j < nphi; ++j) {
            for (int i = etaStart; i < etaEnd; ++i) {
                const auto& cell = grid[i][j];
                if (!cell.empty()) {
                    double maxPt = (*std::max_element(
                        cell.begin(), cell.end(),
                        [](const xAOD::CaloCluster* a,
                           const xAOD::CaloCluster* b) {
                            return a->pt() < b->pt();
                        }))->pt();
                    maxPtValues.push_back(maxPt);
                } else {
                    maxPtValues.push_back(0.0);
                }
            }
        }

        if (!maxPtValues.empty()) {
            std::sort(maxPtValues.begin(), maxPtValues.end());
            size_t mid = maxPtValues.size() / 2;
            if (maxPtValues.size() % 2 == 0) {
                medianPerBand[band] =
                    (maxPtValues[mid - 1] + maxPtValues[mid]) / 2.0;
            } else {
                medianPerBand[band] = maxPtValues[mid];
            }
        }

        ATH_MSG_DEBUG("Eta band " << band
                      << " [" << eta_bins[etaStart] << ", "
                      << eta_bins[etaEnd] << ")"
                      << ": median pT threshold = " << medianPerBand[band]);
    }

    // Create output container
    SG::WriteHandle<xAOD::CaloClusterContainer> h_output =
        SG::makeHandle(m_outputClustersKey, ctx);
    ATH_CHECK(h_output.record(
        std::make_unique<xAOD::CaloClusterContainer>(),
        std::make_unique<xAOD::CaloClusterAuxContainer>()));

    // Apply eta-dependent pT threshold:
    //   weight = 1.0 if cluster pT > band median, else 0.0
    for (int i = 0; i < neta; ++i) {
        int band = i / bandWidth;
        double threshold = medianPerBand[band];

        for (int j = 0; j < nphi; ++j) {
            for (const auto* cluster : grid[i][j]) {
                double weight = (cluster->pt() > threshold) ? 1.0 : 0.0;

                auto* out = h_output->push_back(
                    std::make_unique<xAOD::CaloCluster>());
                out->setE(cluster->e() * weight);
                out->setEta(cluster->eta());
                out->setPhi(cluster->phi());
                out->setM(cluster->m());
            }
        }
    }

    ATH_MSG_DEBUG("Output " << h_output->size()
                  << " clusters after EtaSoftKiller");

    return StatusCode::SUCCESS;
}
