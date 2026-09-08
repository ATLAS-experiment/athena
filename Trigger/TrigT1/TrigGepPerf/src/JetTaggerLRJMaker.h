/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_JETTAGGERLRJMAKER_H
#define TRIGGEPPERF_JETTAGGERLRJMAKER_H

/*
  JetTaggerLRJMaker:
  -----------------
  GEP modified-seeded-cone large-R jet algorithm with substructure quantitiess
  (psi_R, tau_1, tau_2, mass approx, n subjets). A port of existing standalone emulation
  into TrigGepPerf. The algorithm runs with configurable digitization so it input-output matches the HLS algorithm.
*/

#include "Jet.h"
#include "Cluster.h"
#include "JetTaggerLargeRJet.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace Gep {

  // ------------------------------------------------------------------
  // Source enums - which input objects feed seeds / constituents.
  // ------------------------------------------------------------------
  enum class JetTaggerSeedSource {
    WTACone   = 0, // WTA-cone small-R jets (TrigGepPerf output)
    jFexSRJ   = 1, // (re-simulated) jFEX small-R jets
    gFexSRJ   = 2  // (re-simulated) gFEX small-R jets
  };

  enum class JetTaggerConstSource {
    Towers   = 0, // GEP cell towers / clusters (constituent objects)
    WTACone  = 1  // WTA-cone small-R jets used as constituents (subjet-aware)
  };

  // ==================================================================
  // JetTaggerLRJConfig
  // ------------------------------------------------------------------
  // Full runtime description of the digitization scheme and algorithm
  // parameters. Base fields are set directly (or via a preset); the
  // derived fields and the deltaR LUT are then filled by computeDerived().
  // Mirrors write_constants_header() in emulationHelperFunctions.h.
  // ==================================================================
  class JetTaggerLRJConfig {
  public:
    // ---------------- base (user-controlled) parameters ------------
    // Algorithm flow
    unsigned int algoVersion         {3};   // 2 = basic, 3 = advanced (OR + seed-pos-opt)
    unsigned int nSeedsInput         {10};  // seeds read/considered per event
    unsigned int nProtoSeeds         {6};   // proto-seeds used in seed-pos-opt
    unsigned int nSeedsOutput        {2};   // output LRJs (leading, subleading)
    unsigned int maxObjectsConsidered{128}; // constituents loaded per event

    // Geometry (undigitized)
    double r2Cut      {1.21};  // jet radius^2 (rCut = sqrt(r2Cut))
    double rMergeCut  {2.0};   // seed-position-optimization search distance

    // Digitization: field bit lengths
    unsigned int et_bit_length            {13};
    unsigned int eta_bit_length           {7};
    unsigned int phi_bit_length           {6};
    unsigned int num_subjets_length       {2};  // substruct 0
    unsigned int N_subjetiness_bit_length {8};  // substruct 1 == substruct 2 (tau_1/tau_2)
    unsigned int mass_approx_bit_length   {8};  // substruct 3
    unsigned int psi_R_bit_length         {8};  // substruct 4
    unsigned int deltaR_lut_length        {8};

    // Digitization: physical ranges
    double phi_min {-3.2};
    double phi_max {3.2};
    double eta_min {-4.85};
    double eta_max {4.95};
    double et_min  {0.0};
    double et_max  {1024.0};      // GeV
    double massApprox_max {512.0}; // GeV

    // Physics thresholds / flow toggles (undigitized, GeV)
    double subjetEtThresholdGeV       {25.0};
    double minEtSeedPosOptCutGeV      {25.0};
    bool   enableOverlapRemoval       {true};
    bool   enableEtWeightedMidpoint   {true};
    bool   minEtSeedPosOptimization   {true};

    // Output toggles
    bool writeSubstructure       {true};
    bool writeSubjetKinematics   {true};
    bool writeConstituentIndices {true};

    // Input Et unit conversion: incoming TLorentzVector Et is in MeV,
    // the emulation works in GeV. (1 MeV = 1e-3 GeV.)
    double inputEtToGeV {1.0e-3};

    // ---------------- derived parameters (computeDerived) ----------
    double       rCut                     {0.0};
    double       phi_granularity          {0.0};
    unsigned int pi_digitized_in_phi      {0};
    int          PI_D                     {0};
    int          TWO_PI_D                 {0};
    unsigned int eta_range                {0};
    double       eta_granularity          {0.0};
    double       et_granularity           {0.0};
    double       deltaR_granularity       {0.0};
    double       psi_R_granularity        {0.0};
    double       mass_approx_granularity  {0.0};
    double       N_subjetiness_granularity{0.0};
    double       deltaR2_granularity      {0.0};
    unsigned int digitized_delta_R2Cut    {0};
    unsigned int digitized_d_search_squared{0};
    unsigned int massApproxDivisor        {1};

    // On-the-fly deltaR LUT (lutR_8b_ in the emulation), size = max_R_8b_lut_size.
    std::vector<unsigned int> lutR_8b;

    // Fill all derived fields and (re)build the deltaR LUT. Call once per
    // configuration, after the base fields are set (see GepJetTaggerLRJAlgCfg,
    // which is the single source of truth for the BasicV2 / AdvancedV3 presets).
    void computeDerived();

    // ---------------- digitization helpers -------------------------
    static inline uint32_t maskN(unsigned int n) {
      return (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1u);
    }

    // Digitize a physical value into an unsigned integer field.
    unsigned int digitize(double value, unsigned int bit_length,
                          double min_val, double max_val,
                          unsigned int altRange = 0) const {
      const unsigned int range = (altRange == 0) ? (1u << bit_length) : altRange;
      const double scale = static_cast<double>(range) / (max_val - min_val);
      if (value < min_val) value = min_val;
      if (value >= max_val) return range - 1;
      return static_cast<unsigned int>(std::round((value - min_val) * scale));
    }

    unsigned int digitizeEt (double etGeV) const { return digitize(etGeV, et_bit_length,  et_min,  et_max); }
    unsigned int digitizeEta(double eta)   const { return digitize(eta,   eta_bit_length, eta_min, eta_max, eta_range); }
    unsigned int digitizePhi(double phi)   const { return digitize(phi,   phi_bit_length, phi_min, phi_max); }

    double undigitizeEt (unsigned int v) const { return (v & maskN(et_bit_length))  * et_granularity; }
    double undigitizeEta(unsigned int v) const { return eta_min + (v & maskN(eta_bit_length)) * eta_granularity; }
    double undigitizePhi(unsigned int v) const { return phi_min + (v & maskN(phi_bit_length)) * phi_granularity; }
    double undigitizePsiR(unsigned int v)        const { return (v & maskN(psi_R_bit_length))         * psi_R_granularity; }
    double undigitizeMassApprox(unsigned int v)  const { return (v & maskN(mass_approx_bit_length))   * mass_approx_granularity; }
    double undigitizeNSubjetiness(unsigned int v)const { return (v & maskN(N_subjetiness_bit_length)) * N_subjetiness_granularity; }

    // Symmetric phi wrap in the digitized integer domain.
    int wrapSym(int phi) const {
      if (phi >  PI_D) return phi - TWO_PI_D;
      if (phi < -PI_D) return phi + TWO_PI_D;
      return phi;
    }

    // Digitized deltaR^2 between two (eta, phi) integer coordinates (phi wrapped).
    unsigned int digitizedDeltaR2(unsigned int eta1, unsigned int phi1,
                                  unsigned int eta2, unsigned int phi2) const {
      unsigned int dEta = static_cast<unsigned int>(std::abs(static_cast<int>(eta1) - static_cast<int>(eta2)));
      unsigned int dPhi = static_cast<unsigned int>(std::abs(static_cast<int>(phi1) - static_cast<int>(phi2)));
      if (dPhi >= pi_digitized_in_phi) dPhi = (2 * pi_digitized_in_phi) - dPhi;
      return dEta * dEta + dPhi * dPhi;
    }

    // LUT index from wrapped absolute (deltaEta, deltaPhi) integer components.
    unsigned int calcLutIndex(unsigned int dEta, unsigned int dPhi) const {
      return dEta * (1u << (phi_bit_length - 1)) + dPhi;
    }

    // deltaR LUT lookup (digitized deltaR), bounds-guarded.
    unsigned int lutR(unsigned int dEta, unsigned int dPhi) const {
      const unsigned int idx = calcLutIndex(dEta, dPhi);
      if (idx < lutR_8b.size()) return lutR_8b[idx];
      return (1u << psi_R_bit_length) - 1; // clamp: beyond LUT range == max deltaR
    }

  private:
    // Ports of the standalone LUT-sizing / building routines.
    static unsigned int calculateLutMaxSize(double cut,
                                             unsigned int etaBitLength,
                                             unsigned int phiBitLength,
                                             double etaGranularity,
                                             double phiGranularity,
                                             bool deltaR2orDeltaR);
    void buildDeltaRLut();

    static double wrapSymDbl(double dphi) {
      if (dphi >  M_PI) return dphi - 2 * M_PI;
      if (dphi < -M_PI) return dphi + 2 * M_PI;
      return dphi;
    }
  };

  // ==================================================================
  // JetTaggerLRJMaker
  // ==================================================================
  class JetTaggerLRJMaker {
  public:
    JetTaggerLRJMaker() = default;

    std::string toString() const { return "JetTaggerLRJ"; }

    // Seeds: undigitized Gep::Jet (WTACone, or jFEX/gFEX SRJ converted to
    // Gep::Jet inside GepJetAlg). Constituents: undigitized Gep::Cluster.
    // Et in both is taken from the TLorentzVector in MeV and converted to
    // GeV via m_cfg.inputEtToGeV.
    std::vector<Gep::LargeRJet>
    makeLargeRJets(const std::vector<Gep::Jet>& seeds,
                   const std::vector<Gep::Cluster>& constituents) const;

    // Configuration access. Set m_cfg (or its fields) then call
    // m_cfg.computeDerived() once before makeLargeRJets.
    JetTaggerLRJConfig m_cfg;

    void SetSeedSource(JetTaggerSeedSource s)   { m_seedSource = s; }
    JetTaggerSeedSource GetSeedSource() const   { return m_seedSource; }
    void SetConstSource(JetTaggerConstSource c) { m_constSource = c; }
    JetTaggerConstSource GetConstSource() const { return m_constSource; }

  private:
    JetTaggerSeedSource  m_seedSource  {JetTaggerSeedSource::WTACone};
    JetTaggerConstSource m_constSource {JetTaggerConstSource::Towers};

    // Digitized (et, eta, phi) triplet - the emulation's inputObject/outputJet.
    struct DigiObj { unsigned int et{0}; unsigned int eta{0}; unsigned int phi{0}; };

    // ---- algorithm stages (mirror jetTaggerEmulation.cc::eventLoop) ----
    std::vector<DigiObj> loadSeeds(const std::vector<Gep::Jet>& seeds) const;
    // Returns the digitized constituents (E_T-descending, > 2 GeV); fills
    // originalIndices with each one's index into `constituents`, so a digitized
    // slot can be mapped back to its source cluster.
    std::vector<DigiObj> loadConstituents(const std::vector<Gep::Cluster>& constituents,
                                          std::vector<int>& originalIndices) const;

    void overlapRemoval(std::vector<DigiObj>& seeds) const;
    void seedPositionOptimization(std::vector<DigiObj>& seeds) const;
  };

}

#endif // TRIGGEPPERF_JETTAGGERLRJMAKER_H
