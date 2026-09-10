/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "JetTaggerLRJJetMaker.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

namespace Gep {

  // ==================================================================
  // JetTaggerLRJJetMaker: input loading (digitization of the float inputs)
  // ==================================================================
  std::vector<JetTaggerLRJJetMaker::DigiObj>
  JetTaggerLRJJetMaker::loadSeeds(const std::vector<Gep::Jet>& seeds) const {
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

  std::vector<JetTaggerLRJJetMaker::DigiObj>
  JetTaggerLRJJetMaker::loadConstituents(const std::vector<Gep::Cluster>& constituents,
                                      std::vector<int>& originalIndices) const {
    // Only feed the LRJ towers above constEtCutGeV (2 GeV by default), ordered
    // by E_T descending. With the raised, non-binding maxObjectsConsidered
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

    // Select constituents above the E_T cut (in E_T-descending order). The
    // threshold is derived from constEtCutGeV in computeDerived()
    std::vector<unsigned int> sel;
    sel.reserve(order.size());
    for (const auto idx : order){
      if (m_cfg.digitizeEt (constituents[idx].vec.Et() * m_cfg.inputEtToGeV) > m_cfg.constEtCutDigi){
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
  // Float-input wrapper: digitize, run the integer algorithm, convert back.
  // ==================================================================
  std::vector<Gep::LargeRJet>
  JetTaggerLRJJetMaker::makeLargeRJets(const std::vector<Gep::Jet>& seeds,
                                       const std::vector<Gep::Cluster>& constituents) const {
    const JetTaggerLRJConfig& cfg = m_cfg;

    // Stages 1 and 3: digitize seeds (leading-first, zero-padded to
    // nSeedsInput) and constituents (E_T-descending, above threshold).
    const std::vector<DigiObj> seedValues = loadSeeds(seeds);
    std::vector<int> constituentOrigIdx;   // maps a digitized slot -> source cluster index
    const std::vector<DigiObj> inputObjectValues =
        loadConstituents(constituents, constituentOrigIdx);

    // Stages 2, 4 and 5: the algorithm proper.
    const std::vector<DigiLRJ> digiJets =
        makeLargeRJetsDigitized(seedValues, inputObjectValues);

    // Convert back to physical units (Et in MeV, to match the other objects).
    std::vector<Gep::LargeRJet> outputs;
    outputs.reserve(digiJets.size());

    for (const auto& d : digiJets) {
      Gep::LargeRJet lrj;
      const double etGeV = cfg.undigitizeEt (d.et);
      const double etaU  = cfg.undigitizeEta(d.eta);
      const double phiU  = cfg.undigitizePhi(d.phi);

      // Jet mass computation (NOT FIRMWARE-LIKE):
      // invariant mass of the merged constituents, each taken as a MASSLESS 4-vector
      // built from its (Et, eta, phi):
      //   E += Et*cosh(eta); px += Et*cos(phi); py += Et*sin(phi); pz += Et*sinh(eta)
      //   m  = sqrt(max(0, E^2 - px^2 - py^2 - pz^2))
      // NOTE: unlike psi_R / tau_1 / tau_2 / massApprox / nSubjets -- which are
      // digitized, firmware-like calculations -- this mass uses
      // cosh/sinh/cos/sin on the undigitized constituents and is therefore NOT
      // a firmware-like computation. It is one of the reasons this wrapper
      // exists separately from makeLargeRJetsDigitized().
      // future firmware implementation would likely use LUT-based approach to compute
      // cosh, cos, sin, sinh, if further performance studies justify implementation
      double sumE = 0.0, sumPx = 0.0, sumPy = 0.0, sumPz = 0.0;
      for (auto idx : d.mergedIndices) {
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
        lrj.nSubjets   = static_cast<int>(d.numSubjets);
        lrj.psi_R      = cfg.undigitizePsiR(d.psi_R);
        lrj.tau_1      = cfg.undigitizeNSubjetiness(d.tau_1);
        lrj.tau_2      = cfg.undigitizeNSubjetiness(d.tau_2);
        lrj.tau_21     = (d.tau_1 != 0)
                           ? (static_cast<double>(d.tau_2) / static_cast<double>(d.tau_1))
                           : 0.0;
        lrj.massApprox = cfg.undigitizeMassApprox(d.massApprox);
      }

      if (cfg.writeSubjetKinematics) {
        for (const auto& sub : d.subjets) {
          lrj.subjet_et .push_back(cfg.undigitizeEt (sub.et) / cfg.inputEtToGeV);
          lrj.subjet_eta.push_back(cfg.undigitizeEta(sub.eta));
          lrj.subjet_phi.push_back(cfg.undigitizePhi(sub.phi));
        }
      }

      if (cfg.writeConstituentIndices) {
        lrj.nConstituents = static_cast<int>(d.mergedIndices.size());
        for (auto idx : d.mergedIndices) {
          lrj.mergedIndices.push_back(static_cast<int>(idx));
          // Map the (E_T-sorted) constituent slot back to its source-cluster
          // index so GepJetAlg can attach the actual cluster to the xAOD::Jet.
          lrj.constituentsIndices.push_back(constituentOrigIdx[idx]);
        }
      }

      outputs.push_back(std::move(lrj));
    }

    return outputs;
  }

}
