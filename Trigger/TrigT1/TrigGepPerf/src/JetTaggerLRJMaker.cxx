/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "JetTaggerLRJMaker.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

namespace Gep {

  // ==================================================================
  // JetTaggerLRJConfig: compute derived constants used for digitization of algorithm
  // ==================================================================
  void JetTaggerLRJConfig::computeDerived() {
    rCut               = std::sqrt(r2Cut);
    phi_granularity    = (phi_max - phi_min) / static_cast<double>(1u << phi_bit_length);
    pi_digitized_in_phi= static_cast<unsigned int>(M_PI / phi_granularity);
    PI_D               = static_cast<int>(pi_digitized_in_phi);
    TWO_PI_D           = 2 * static_cast<int>(pi_digitized_in_phi) + 1;

    // eta granularity is forced to match phi granularity,
    // which fixes eta_range for the given ranges/phi bit length.
    // Round (not truncate): eta/phi come in as Gaudi::Property<float>, so the
    // span/phi_granularity ratio can land at e.g. 97.9999 and floor to the wrong
    // bin count. The intended value is the nearest integer (98 for AdvancedV3).
    eta_range          = static_cast<unsigned int>(std::round((eta_max - eta_min) / phi_granularity));
    eta_granularity    = (eta_max - eta_min) / static_cast<double>(eta_range);
    et_granularity     = (et_max - et_min) / static_cast<double>(1u << et_bit_length);
    deltaR_granularity = (2 * rCut) / static_cast<double>(1u << deltaR_lut_length);
    psi_R_granularity  = (2 * rCut) / static_cast<double>(1u << deltaR_lut_length);
    mass_approx_granularity   = massApprox_max / static_cast<double>(1u << mass_approx_bit_length);
    N_subjetiness_granularity = 1.0 / static_cast<double>(1u << N_subjetiness_bit_length);
    deltaR2_granularity= eta_granularity * eta_granularity;

    digitized_delta_R2Cut      = static_cast<unsigned int>(r2Cut / deltaR2_granularity + 0.5);
    digitized_d_search_squared = static_cast<unsigned int>((rMergeCut * rMergeCut) / deltaR2_granularity + 0.5);

    const double massApproxRawLSB = et_granularity * deltaR_granularity;
    const double massApproxNewLSB = massApprox_max / static_cast<double>(1u << mass_approx_bit_length);
    const double massApproxScaleFactor = (massApproxRawLSB > 0.0) ? (massApproxNewLSB / massApproxRawLSB) : 1.0;
    massApproxDivisor = static_cast<unsigned int>(massApproxScaleFactor + 0.5);
    if (massApproxDivisor == 0) massApproxDivisor = 1;

    buildDeltaRLut();
  }

  // LUTs only stored until last relevant value to save size, this function finds that maximum value
  unsigned int JetTaggerLRJConfig::calculateLutMaxSize(double cut,
                                                       unsigned int etaBitLength,
                                                       unsigned int phiBitLength,
                                                       double etaGranularity,
                                                       double phiGranularity,
                                                       bool deltaR2orDeltaR) {
    unsigned int last_one_index = 0;
    unsigned int idx = 0;
    for (unsigned int etaIt = 0; etaIt < (1u << etaBitLength); ++etaIt) {
      for (unsigned int phiIt = 0; phiIt < (1u << phiBitLength); ++phiIt) {
        const double deltaPhiWrapped = wrapSymDbl(phiIt * phiGranularity);
        const double etaSquared = std::pow(etaIt * etaGranularity, 2);
        const double phiSquared = std::pow(deltaPhiWrapped, 2);
        const double deltaR2orR = deltaR2orDeltaR ? (etaSquared + phiSquared)
                                                  : std::sqrt(etaSquared + phiSquared);
        if (deltaR2orR < cut) last_one_index = idx;
        ++idx;
      }
    }
    return last_one_index + 1;
  }

  // LUT used for sqrt(deltaR^2) calculation for substructure variables that require deltaR, rather than deltaR^2 (which can be computed without a LUT)
  void JetTaggerLRJConfig::buildDeltaRLut() {
    lutR_8b.clear();

    // LUT digitizes deltaR with granularity 2*rCut / (1 << psi_R_bit_length)
    // and clamps to psi_R_bit_length bits (== substruct 4 in the emulation).
    const unsigned int psiBits = psi_R_bit_length;
    const double lutGranularity = (2 * rCut) / static_cast<double>(1u << psiBits);
    const unsigned int clampMax = (1u << psiBits) - 1;

    const unsigned int maxSize = calculateLutMaxSize(2 * rCut, eta_bit_length, phi_bit_length,
                                                     eta_granularity, phi_granularity, false);
    lutR_8b.reserve(maxSize);

    unsigned int iR = 0;
    for (unsigned int etaIt = 0; etaIt < eta_range; ++etaIt) {
      for (unsigned int phiIt = 0; phiIt < (1u << (phi_bit_length - 1)); ++phiIt) {
        if (iR >= maxSize) break;
        const double dEta = etaIt * eta_granularity;
        const double dPhi = phiIt * phi_granularity;
        const double dR   = std::sqrt(dEta * dEta + dPhi * dPhi);
        unsigned int digi = static_cast<unsigned int>(dR / lutGranularity + 0.5);
        if (digi > clampMax) digi = clampMax;
        lutR_8b.push_back(digi);
        ++iR;
      }
      if (iR >= maxSize) break;
    }
  }

  // ==================================================================
  // JetTaggerLRJMaker: input loading
  // ==================================================================
  std::vector<JetTaggerLRJMaker::DigiObj>
  JetTaggerLRJMaker::loadSeeds(const std::vector<Gep::Jet>& seeds) const {
    // Seeds are sorted leading-first by their transverse momentum
    std::vector<const Gep::Jet*> sorted;
    sorted.reserve(seeds.size());
    for (const auto& s : seeds) sorted.push_back(&s);
    std::sort(sorted.begin(), sorted.end(),
                     [](const Gep::Jet* a, const Gep::Jet* b) {
                       return a->vec.Pt() > b->vec.Pt();
                     });

    std::vector<DigiObj> out(m_cfg.nSeedsInput);   // zero-padded
    const unsigned int n = std::min<unsigned int>(m_cfg.nSeedsInput, sorted.size());
    for (unsigned int i = 0; i < n; ++i) {
      const double etGeV = sorted[i]->vec.Pt() * m_cfg.inputEtToGeV;
      out[i].et  = m_cfg.digitizeEt(etGeV);
      // Digitize the raw eta/phi (etaInput/phiInput), float32-truncated
      out[i].eta = m_cfg.digitizeEta(static_cast<float>(sorted[i]->etaInput));
      out[i].phi = m_cfg.digitizePhi(static_cast<float>(sorted[i]->phiInput));
    }
    return out;
  }

  std::vector<JetTaggerLRJMaker::DigiObj>
  JetTaggerLRJMaker::loadConstituents(const std::vector<Gep::Cluster>& constituents,
                                      std::vector<int>& originalIndices) const {
    // Only feed the LRJ towers with E_T > 2 GeV, ordered by E_T descending
    // With the raised, non-binding maxObjectsConsidered
    // (512) the input order only affects subjet selection among >=25 GeV candidates,
    // which are well separated, so an E_T-only order suffices and matches the
    // emulation reader (jetTaggerEmulation.cc, gepCellsTowers, same >2 GeV cut).

    // Sort an index list by E_T descending: the input is const, so we neither mutate
    // the caller's collection nor copy Cluster objects nor juggle pointers.
    std::vector<unsigned int> order(constituents.size());
    for (unsigned int k = 0; k < order.size(); ++k) order[k] = k;
    std::sort(order.begin(), order.end(),
                     [&constituents](unsigned int a, unsigned int b) {
                       return constituents[a].vec.Et() > constituents[b].vec.Et();
                     });

    // Select E_T > 2 GeV constituents (in E_T-descending order).
    std::vector<unsigned int> sel;
    sel.reserve(order.size());
    for (const auto idx : order){
      if (m_cfg.digitizeEt (constituents[idx].vec.Et() * m_cfg.inputEtToGeV) > 16){
        sel.push_back(idx);
      }
    }

    // Digitize the leading maxObjectsConsidered selected constituents.
    const unsigned int nMax = std::min<unsigned int>(m_cfg.maxObjectsConsidered, sel.size());
    std::vector<DigiObj> out(nMax);
    originalIndices.resize(nMax);
    for (unsigned int i = 0; i < nMax; ++i) {
      const Gep::Cluster& c = constituents[sel[i]];
      out[i].et  = m_cfg.digitizeEt (c.vec.Et() * m_cfg.inputEtToGeV);
      out[i].eta = m_cfg.digitizeEta(static_cast<float>(c.vec.Eta()));
      out[i].phi = m_cfg.digitizePhi(static_cast<float>(c.vec.Phi()));
      originalIndices[i] = static_cast<int>(sel[i]);
    }
    return out;
  }

  // ==================================================================
  // Stage: overlap removal between the leading two seeds
  // ==================================================================
  void JetTaggerLRJMaker::overlapRemoval(std::vector<DigiObj>& seeds) const {
    if (seeds.size() < 2) return;
    const unsigned int cut = 2 * 2 * m_cfg.digitized_delta_R2Cut; // (2 * jet radius)^2

    const unsigned int dR2 = m_cfg.digitizedDeltaR2(seeds[0].eta, seeds[0].phi,
                                                    seeds[1].eta, seeds[1].phi);
    if (dR2 > cut) return;

    for (unsigned int i = m_cfg.nSeedsOutput; i < m_cfg.nSeedsInput && i < seeds.size(); ++i) {
      if (seeds[i].et == 0 && seeds[i].eta == 0 && seeds[i].phi == 0) continue;
      const unsigned int dR2b = m_cfg.digitizedDeltaR2(seeds[0].eta, seeds[0].phi,
                                                       seeds[i].eta, seeds[i].phi);
      if (dR2b > cut) {
        std::swap(seeds[1], seeds[i]);
        break;
      }
    }
  }

  // ==================================================================
  // Stage: seed position optimization toward Et-weighted midpoint
  // ==================================================================
  void JetTaggerLRJMaker::seedPositionOptimization(std::vector<DigiObj>& seeds) const {
    const unsigned int nOut = m_cfg.nSeedsOutput;
    if (m_cfg.nProtoSeeds <= nOut) return;
    const unsigned int nPre = m_cfg.nProtoSeeds - nOut;

    std::vector<unsigned int> counter(nOut, 0);
    std::vector<unsigned int> indices(nOut, std::numeric_limits<unsigned int>::max());
    std::vector<std::vector<char>> inProto(nOut, std::vector<char>(nPre, 0));

    const unsigned int minEtDigi =
        (m_cfg.et_granularity > 0.0)
            ? static_cast<unsigned int>(m_cfg.minEtSeedPosOptCutGeV / m_cfg.et_granularity)
            : 0u;

    for (unsigned int iSeed = 0; iSeed < nOut; ++iSeed) {
      for (unsigned int iPre = 0; iPre < nPre; ++iPre) {
        const unsigned int idxSeed = iPre + nOut;
        if (idxSeed >= seeds.size()) continue;
        if (seeds[idxSeed].et == 0) continue;
        if (m_cfg.minEtSeedPosOptimization && seeds[idxSeed].et <= minEtDigi) continue;

        const unsigned int dR2 = m_cfg.digitizedDeltaR2(seeds[iSeed].eta, seeds[iSeed].phi,
                                                        seeds[idxSeed].eta, seeds[idxSeed].phi);
        if (dR2 <= m_cfg.digitized_d_search_squared) {
          counter[iSeed]++;
          inProto[iSeed][iPre] = 1;
          indices[iSeed] = iPre;
        }
      }
    }

    // If several proto-seeds are in range, shift toward the highest-Et one.
    for (unsigned int iSeed = 0; iSeed < nOut; ++iSeed) {
      if (counter[iSeed] <= 1) continue;
      unsigned int maxEt = 0;
      for (unsigned int iPre = 0; iPre < nPre; ++iPre) {
        if (!inProto[iSeed][iPre]) continue;
        const unsigned int et = seeds[iPre + nOut].et;
        if (et > maxEt) { maxEt = et; indices[iSeed] = iPre; }
      }
    }

    // Prevent both seeds shifting toward the same proto-seed.
    bool skipSecondSeed = false;
    if (nOut >= 2 && indices[0] == indices[1] &&
        indices[0] != std::numeric_limits<unsigned int>::max()) skipSecondSeed = true;

    const int etaHalf = 1 << (m_cfg.eta_bit_length - 1);
    const int phiHalf = 1 << (m_cfg.phi_bit_length - 1);

    for (unsigned int iSeed = 0; iSeed < nOut; ++iSeed) {
      if (skipSecondSeed && iSeed == 1) continue;
      if (indices[iSeed] == std::numeric_limits<unsigned int>::max()) continue;

      const unsigned int idxSeed = indices[iSeed] + nOut;
      const int et1 = static_cast<int>(seeds[iSeed].et);
      const int et2 = static_cast<int>(seeds[idxSeed].et);
      const int etSum = et1 + et2;

      const int eta1 = static_cast<int>(seeds[iSeed].eta)  - etaHalf;
      const int eta2 = static_cast<int>(seeds[idxSeed].eta) - etaHalf;
      const int phi1s = static_cast<int>(seeds[iSeed].phi)  - phiHalf;
      const int phi2s = static_cast<int>(seeds[idxSeed].phi) - phiHalf;

      const int dphi = m_cfg.wrapSym(phi2s - phi1s);

      int eta_mid;
      int phi_mid;
      if (!m_cfg.enableEtWeightedMidpoint || etSum == 0) {
        eta_mid = (eta1 + eta2) >> 1;
        phi_mid = m_cfg.wrapSym(phi1s + (dphi >> 1));
      } else {
        eta_mid = (et2 * eta2 + et1 * eta1) / etSum;
        phi_mid = m_cfg.wrapSym(phi1s + (et2 * dphi) / etSum);
      }

      seeds[iSeed].eta = static_cast<unsigned int>(eta_mid + etaHalf);
      seeds[iSeed].phi = static_cast<unsigned int>(phi_mid + phiHalf);
    }
  }

  std::vector<Gep::LargeRJet>
  JetTaggerLRJMaker::makeLargeRJets(const std::vector<Gep::Jet>& seeds,
                                    const std::vector<Gep::Cluster>& constituents) const {
    const JetTaggerLRJConfig& cfg = m_cfg;
    const bool jetInput = (m_constSource == JetTaggerConstSource::WTACone);

    // Stage 1: digitize seeds (leading-first, zero-padded to nSeedsInput).
    std::vector<DigiObj> seedValuesOriginal = loadSeeds(seeds);
    std::vector<DigiObj> seedValues = seedValuesOriginal; // pre-optimization copy kept in *Original

    // Stage 2: overlap removal (advanced only).
    if (cfg.algoVersion != 2 && cfg.enableOverlapRemoval) {
      overlapRemoval(seedValues);
    }

    // Stage 3: digitize constituents.
    std::vector<int> constituentOrigIdx;   // maps a digitized slot -> source cluster index
    std::vector<DigiObj> inputObjectValues = loadConstituents(constituents, constituentOrigIdx);
    const unsigned int objectsProcessed = inputObjectValues.size();

    // Stage 4: seed position optimization (advanced algo only). The
    // minEtSeedPosOptimization flag only gates the min-Et proto-seed cut
    // *inside* this stage (handled in seedPositionOptimization), matching
    // the standalone emulation where the stage always runs for v3.
    if (cfg.algoVersion != 2) {
      seedPositionOptimization(seedValues);
    }

    // Substructure bookkeeping.
    const unsigned int subjetCap = (cfg.num_subjets_length == 0)
                                     ? 0u : ((1u << cfg.num_subjets_length) - 1u);
    const unsigned int subjetEtDigi =
        (cfg.et_granularity > 0.0)
            ? static_cast<unsigned int>(cfg.subjetEtThresholdGeV / cfg.et_granularity)
            : 0u;
    const int etClampMax = static_cast<int>(cfg.et_max / cfg.et_granularity - 1);
    const unsigned int nSubjetinessScale = (1u << cfg.N_subjetiness_bit_length) - 1u;
    const unsigned int rCutInDeltaRUnits =
        (cfg.deltaR_granularity > 0.0)
            ? static_cast<unsigned int>(cfg.rCut / cfg.deltaR_granularity + 0.5)
            : 1u;

    std::vector<Gep::LargeRJet> outputs;
    outputs.reserve(cfg.nSeedsOutput);

    // Stage 5: per output seed - cluster constituents and compute substructure.
    for (unsigned int iSeed = 0; iSeed < cfg.nSeedsOutput && iSeed < seedValues.size(); ++iSeed) {
      std::vector<unsigned int> mergedInputObjectIndices;
      std::vector<DigiObj> subjets(subjetCap);
      unsigned int numSubjets  = 0;
      unsigned int outputJetEt = jetInput ? seedValues[iSeed].et : 0u;
      unsigned int jet_psi_R   = 0;
      unsigned int massApprox  = 0;
      unsigned int tau_1       = 0;
      unsigned int tau_2       = 0;

      if (seedValues[iSeed].et != 0) {
        // ---- constituent association ----
        for (unsigned int iInput = 0; iInput < objectsProcessed; ++iInput) {
          if (inputObjectValues[iInput].et == 0) break; // Et-sorted: rest are empty

          unsigned int dEta = static_cast<unsigned int>(
              std::abs(static_cast<int>(seedValues[iSeed].eta) - static_cast<int>(inputObjectValues[iInput].eta)));
          unsigned int dPhi = static_cast<unsigned int>(
              std::abs(static_cast<int>(seedValues[iSeed].phi) - static_cast<int>(inputObjectValues[iInput].phi)));
          if (dPhi >= cfg.pi_digitized_in_phi) dPhi = (2 * cfg.pi_digitized_in_phi) - dPhi;

          // If constituent falls within jet cone
          if (dEta * dEta + dPhi * dPhi <= cfg.digitized_delta_R2Cut) {
            if (outputJetEt + inputObjectValues[iInput].et >= static_cast<unsigned int>(etClampMax)) // Accumulate output jet E_T
              outputJetEt = static_cast<unsigned int>(etClampMax);
            else
              outputJetEt += inputObjectValues[iInput].et;

            mergedInputObjectIndices.push_back(iInput);
            jet_psi_R += inputObjectValues[iInput].et * cfg.lutR(dEta, dPhi);

            // For jet inputs, a merged jet above threshold is itself a subjet
            if (jetInput && inputObjectValues[iInput].et > subjetEtDigi) {
              if (numSubjets < subjetCap) {
                subjets[numSubjets] = inputObjectValues[iInput];
                numSubjets++;
              }
            }
          }
        }

        // ---- add seed jets that fall inside R as subjets ----
        if (jetInput) {
          for (unsigned int iSub = 0; iSub < cfg.nSeedsOutput && iSub < seedValuesOriginal.size(); ++iSub) {
            if (seedValuesOriginal[iSub].et > subjetEtDigi &&
                m_cfg.digitizedDeltaR2(seedValues[iSeed].eta, seedValues[iSeed].phi,
                                       seedValuesOriginal[iSub].eta, seedValuesOriginal[iSub].phi)
                    <= cfg.digitized_delta_R2Cut) {
              if (numSubjets < subjetCap) { subjets[numSubjets] = seedValuesOriginal[iSub]; numSubjets++; }
            }
          }
        } else {
          for (unsigned int iSub = 0; iSub < cfg.nSeedsInput && iSub < seedValuesOriginal.size(); ++iSub) {
            if (seedValuesOriginal[iSub].et > subjetEtDigi &&
                m_cfg.digitizedDeltaR2(seedValues[iSeed].eta, seedValues[iSeed].phi,
                                       seedValuesOriginal[iSub].eta, seedValuesOriginal[iSub].phi)
                    <= cfg.digitized_delta_R2Cut) {
              if (numSubjets < subjetCap) { subjets[numSubjets] = seedValuesOriginal[iSub]; numSubjets++; }
            }
          }
        }

        // ---- psi_R normalization ----
        jet_psi_R = (outputJetEt > 0) ? (jet_psi_R / outputJetEt) : 0u;

        // ---- N-subjettiness / mass approximation ----
        if (numSubjets == 1) {
          for (auto iInput : mergedInputObjectIndices) {
            unsigned int dEta = static_cast<unsigned int>(
                std::abs(static_cast<int>(subjets[0].eta) - static_cast<int>(inputObjectValues[iInput].eta)));
            unsigned int dPhi = static_cast<unsigned int>(
                std::abs(static_cast<int>(subjets[0].phi) - static_cast<int>(inputObjectValues[iInput].phi)));
            if (dPhi >= cfg.pi_digitized_in_phi) dPhi = (2 * cfg.pi_digitized_in_phi) - dPhi;
            tau_1 += cfg.lutR(dEta, dPhi) * inputObjectValues[iInput].et;
          }
        } else if (numSubjets >= 2) {
          const unsigned int subjetTotalEt = subjets[0].et + subjets[1].et;

          unsigned int dEtaSub = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[0].eta) - static_cast<int>(subjets[1].eta)));
          unsigned int dPhiSub = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[0].phi) - static_cast<int>(subjets[1].phi)));
          if (dPhiSub >= cfg.pi_digitized_in_phi) dPhiSub = (2 * cfg.pi_digitized_in_phi) - dPhiSub;

          const unsigned int massApproxRaw = subjetTotalEt * cfg.lutR(dEtaSub, dPhiSub);
          const unsigned int massApproxTmp = massApproxRaw / cfg.massApproxDivisor;
          massApprox = (massApproxTmp > static_cast<unsigned int>(cfg.massApprox_max))
                         ? static_cast<unsigned int>(cfg.massApprox_max) : massApproxTmp;

          for (auto iInput : mergedInputObjectIndices) {
            unsigned int dEta0 = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[0].eta) - static_cast<int>(inputObjectValues[iInput].eta)));
            unsigned int dPhi0 = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[0].phi) - static_cast<int>(inputObjectValues[iInput].phi)));
            if (dPhi0 >= cfg.pi_digitized_in_phi) dPhi0 = (2 * cfg.pi_digitized_in_phi) - dPhi0;
            const unsigned int dR0 = cfg.lutR(dEta0, dPhi0);
            tau_1 += dR0 * inputObjectValues[iInput].et;

            unsigned int dEta1 = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[1].eta) - static_cast<int>(inputObjectValues[iInput].eta)));
            unsigned int dPhi1 = static_cast<unsigned int>(std::abs(static_cast<int>(subjets[1].phi) - static_cast<int>(inputObjectValues[iInput].phi)));
            if (dPhi1 >= cfg.pi_digitized_in_phi) dPhi1 = (2 * cfg.pi_digitized_in_phi) - dPhi1;
            const unsigned int dR1 = cfg.lutR(dEta1, dPhi1);

            tau_2 += std::min(dR0, dR1) * inputObjectValues[iInput].et;
          }
        }

        // N-subjetiness normalization
        if (outputJetEt > 0) {
          tau_1 = (tau_1 * nSubjetinessScale) / (outputJetEt * rCutInDeltaRUnits);
          tau_2 = (tau_2 * nSubjetinessScale) / (outputJetEt * rCutInDeltaRUnits);
        } else {
          tau_1 = 0;
          tau_2 = 0;
        }
      } // non-zero seed

      double tau_21 = (tau_1 != 0) ? (static_cast<double>(tau_2) / static_cast<double>(tau_1)) : 0.0;

      // Mask to field widths (as the emulation does via bitsets).
      const unsigned int et_value  = outputJetEt        & JetTaggerLRJConfig::maskN(cfg.et_bit_length);
      const unsigned int eta_value = seedValues[iSeed].eta & JetTaggerLRJConfig::maskN(cfg.eta_bit_length);
      const unsigned int phi_value = seedValues[iSeed].phi & JetTaggerLRJConfig::maskN(cfg.phi_bit_length);

      // ---- build output LargeRJet (units restored to MeV to match other objects) ----
      Gep::LargeRJet lrj;
      const double etGeV = cfg.undigitizeEt(et_value);
      const double etaU  = cfg.undigitizeEta(eta_value);
      const double phiU  = cfg.undigitizePhi(phi_value);

      // Jet mass computation (NOT FIRMWARE-LIKE): 
      // invariant mass of the merged constituents, each taken as a MASSLESS 4-vector
      // built from its (Et, eta, phi):
      //   E += Et*cosh(eta); px += Et*cos(phi); py += Et*sin(phi); pz += Et*sinh(eta)
      //   m  = sqrt(max(0, E^2 - px^2 - py^2 - pz^2))
      // NOTE: unlike psi_R / tau_1 / tau_2 / massApprox / nSubjets -- which are
      // digitized, firmware-like calculations -- this mass uses
      // cosh/sinh/cos/sin on the undigitized constituents and is therefore NOT
      // a firmware-like computation.
      // future firmware implementation would likely use LUT-based approach to compute
      // cosh, cos, sin, sinh, if further performance studies justify implementation
      double sumE = 0.0, sumPx = 0.0, sumPy = 0.0, sumPz = 0.0;
      for (auto idx : mergedInputObjectIndices) {
        const double cEt  = cfg.undigitizeEt (inputObjectValues[idx].et);
        const double cEta = cfg.undigitizeEta(inputObjectValues[idx].eta);
        const double cPhi = cfg.undigitizePhi(inputObjectValues[idx].phi);
        sumE  += cEt * std::cosh(cEta);
        sumPx += cEt * std::cos(cPhi);
        sumPy += cEt * std::sin(cPhi);
        sumPz += cEt * std::sinh(cEta);
      }
      const double m2Jet   = sumE * sumE - sumPx * sumPx - sumPy * sumPy - sumPz * sumPz;
      const double massGeV = (m2Jet > 0.0) ? std::sqrt(m2Jet) : 0.0;

      lrj.vec.SetPtEtaPhiM(etGeV / cfg.inputEtToGeV, etaU, phiU, massGeV / cfg.inputEtToGeV);
      lrj.seedEt  = etGeV / cfg.inputEtToGeV;
      lrj.seedEta = etaU;
      lrj.seedPhi = phiU;
      lrj.radius  = cfg.rCut;

      if (cfg.writeSubstructure) {
        lrj.nSubjets   = static_cast<int>(numSubjets);
        lrj.psi_R      = cfg.undigitizePsiR(jet_psi_R);
        lrj.tau_1      = cfg.undigitizeNSubjetiness(tau_1);
        lrj.tau_2      = cfg.undigitizeNSubjetiness(tau_2);
        lrj.tau_21     = tau_21;
        lrj.massApprox = cfg.undigitizeMassApprox(massApprox);
      }

      if (cfg.writeSubjetKinematics) {
        for (unsigned int iSub = 0; iSub < numSubjets; ++iSub) {
          lrj.subjet_et .push_back(cfg.undigitizeEt (subjets[iSub].et)  / cfg.inputEtToGeV);
          lrj.subjet_eta.push_back(cfg.undigitizeEta(subjets[iSub].eta));
          lrj.subjet_phi.push_back(cfg.undigitizePhi(subjets[iSub].phi));
        }
      }

      if (cfg.writeConstituentIndices) {
        lrj.nConstituents = static_cast<int>(mergedInputObjectIndices.size());
        for (auto idx : mergedInputObjectIndices) {
          lrj.mergedIndices.push_back(static_cast<int>(idx));
          // Map the (E_T-sorted) constituent slot back to its source-cluster
          // index so GepJetAlg can attach the actual cluster to the xAOD::Jet.
          lrj.constituentsIndices.push_back(constituentOrigIdx[idx]);
        }
      }

      outputs.push_back(std::move(lrj));
    } // per seed

    // Emit leading-first (sorted by Et) to match the emulation's leading/subleading trees.
    std::sort(outputs.begin(), outputs.end(),
                     [](const Gep::LargeRJet& a, const Gep::LargeRJet& b) {
                       return a.vec.Pt() > b.vec.Pt();
                     });

    return outputs;
  }

}
