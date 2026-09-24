/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_JETTAGGERLRJMAKER_H
#define TRIGGEPPERF_JETTAGGERLRJMAKER_H

/*
  JetTaggerLRJMaker:
  -----------------
  GEP modified-seeded-cone large-R jet algorithm with substructure quantities
  (psi_R, tau_1, tau_2, mass approx, n subjets). A port of the existing
  standalone emulation into TrigGepPerf. The digitization scheme is
  configurable so that the input-output behaviour matches the HLS algorithm.

  This is the bitwise core: integer arithmetic throughout, with no dependency
  on ROOT or on the Gep float object model, in the same way WTAConeMaker is the
  core of the WTA cone jet algorithm. Clients whose input is already digitized
  - GlobalSimulation, whose TOBs carry integer codes - use it directly.

  Gep::JetTaggerLRJJetMaker (TrigGepPerf/src) is the floating point adapter
  around it, used by GepJetAlg: it digitizes xAOD jets/clusters on the way in
  and converts back to physical units on the way out, exactly as
  Gep::WTAConeJetMaker adapts WTAConeMaker.
*/

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace Gep {

  // ------------------------------------------------------------------
  // Which input objects feed the constituents.
  // ------------------------------------------------------------------
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
    double midpointSearchDistance  {2.0};   // seed-position-optimization search distance

    // Digitization: field bit lengths
    unsigned int et_bit_length            {13};
    unsigned int eta_bit_length           {7};
    unsigned int phi_bit_length           {6};
    unsigned int num_subjets_length       {2};  // substruct 0
    unsigned int N_subjetiness_bit_length {8};  // substruct 1 == substruct 2 (tau_1/tau_2)
    unsigned int mass_approx_bit_length   {8};  // substruct 3
    unsigned int psi_R_bit_length         {8};  // substruct 4
    unsigned int deltaR_lut_length        {8};

    // Number of phi codes on the GEP tower grid. This is a property of the
    // grid, not of the output format, so it is the same for every algorithm
    // version and is not configurable. The eta axis has always had its own
    // code count (eta_range, derived below); phi did not, and its granularity
    // was taken from the *field width* (1 << phi_bit_length) instead. That is
    // only correct when the codes happen to fill the field, which they do not
    // for the basic (v2) format: there the 64-code grid sits in a 9b TOB field.
    static constexpr unsigned int phi_range {64};

    // Digitization: physical ranges.
    // eta_min/phi_min are the *centre* of the first tower, one half-tower
    // inside the edge of the covered range, and *_max is one granularity past
    // the last centre. Matching CaloTowerContainer::configureGrid(98, -4.9,
    // 4.9, 64) and GlobalCellTowerAlgTool's floor(x*10) binning.
    double phi_min {-3.15};
    double phi_max {3.25};
    double eta_min {-4.85};
    double eta_max {4.95};
    double et_min  {0.0};
    double et_max  {2048.0};      // GeV; over the 13b field this is the 0.25 GeV TOB LSB
    double massApprox_max {512.0}; // GeV

    // Physics thresholds / flow toggles (undigitized, GeV)
    double subjetEtThresholdGeV       {25.0};
    double minEtSeedPosOptCutGeV      {25.0};
    // Minimum constituent E_T. Applied by loadConstituents() on the float path
    // only: callers of makeLargeRJetsDigitized() have already selected their
    // input (the basic algorithm applies no input energy cut at all)
    double constEtCutGeV              {2.0};
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
    // constEtCutGeV in digitized units. Derived rather than hard-coded so the
    // cut stays at the intended energy if et_max (and so et_granularity) moves.
    unsigned int constEtCutDigi           {0};

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
    unsigned int digitizePhi(double phi)   const { return digitize(phi,   phi_bit_length, phi_min, phi_max, phi_range); }

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
    // The row stride is the number of phi codes below pi, which is
    // pi_digitized_in_phi == phi_range/2. When the codes fill the field this is
    // 1 << (phi_bit_length - 1), the value used before phi_range existed.
    unsigned int calcLutIndex(unsigned int dEta, unsigned int dPhi) const {
      // A wrapped |dPhi| runs over [0, phi_range/2] == [0, 32]. That is 33
      // distinct values, while the LUT only has rows for [0, 31]: dPhi == 32
      if (pi_digitized_in_phi > 0 && dPhi >= pi_digitized_in_phi) {
        dPhi = pi_digitized_in_phi - 1;
      }
      return dEta * pi_digitized_in_phi + dPhi;
    }

    // deltaR LUT lookup (digitized deltaR), bounds-guarded.
    unsigned int lutR(unsigned int dEta, unsigned int dPhi) const {
      const unsigned int idx = calcLutIndex(dEta, dPhi);
      if (idx < lutR_8b.size()) return lutR_8b[idx];
      return (1u << psi_R_bit_length) - 1; // clamp: beyond LUT range == max deltaR
    }

  private:
    // Ports of the standalone LUT-sizing / building routines.
    // Sized over the code grid actually used (etaRange x phiHalfRange), which
    // is also how buildDeltaRLut() fills and calcLutIndex() addresses it. It
    // previously iterated the full field widths with a different row stride,
    // so the bound it returned did not correspond to the entries being written.
    static unsigned int calculateLutMaxSize(double cut,
                                             unsigned int etaRange,
                                             unsigned int phiHalfRange,
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
  // ------------------------------------------------------------------
  // The algorithm on digitized input. Integer arithmetic only.
  // ==================================================================
  class JetTaggerLRJMaker {
  public:
    JetTaggerLRJMaker() = default;
    virtual ~JetTaggerLRJMaker() = default;

    std::string toString() const { return "JetTaggerLRJ"; }

    // Digitized (et, eta, phi) triplet - the emulation's inputObject/outputJet.
    struct DigiObj { unsigned int et{0}; unsigned int eta{0}; unsigned int phi{0}; };

    // Digitized result of the algorithm: exactly the integer quantities the
    // firmware produces, before any conversion back to physical units.
    struct DigiLRJ {
      unsigned int et {0};
      unsigned int eta{0};
      unsigned int phi{0};
      unsigned int numSubjets{0};
      unsigned int psi_R     {0};
      unsigned int tau_1     {0};
      unsigned int tau_2     {0};
      unsigned int massApprox{0};
      std::vector<DigiObj>      subjets;      // first numSubjets entries are valid
      std::vector<unsigned int> mergedIndices;// slots in the constituent vector
    };

    // The algorithm proper. This is the firmware-equivalent block.
    //
    // `seeds` must be ordered leading-first; a short event is zero-padded up to
    // cfg.nSeedsInput internally (the jet maker upstream already bounds how
    // many there can be). `constituents` must be E_T-descending and already
    // cut to cfg.maxObjectsConsidered: the association loop stops at the first
    // zero-E_T entry.
    std::vector<DigiLRJ>
    makeLargeRJetsDigitized(const std::vector<DigiObj>& seeds,
                            const std::vector<DigiObj>& constituents) const;

    // Configuration access. Set m_cfg (or its fields) then call
    // m_cfg.computeDerived() once before running the algorithm.
    JetTaggerLRJConfig m_cfg;

    void SetConstSource(JetTaggerConstSource c) { m_constSource = c; }
    JetTaggerConstSource GetConstSource() const { return m_constSource; }

  protected:
    JetTaggerConstSource m_constSource {JetTaggerConstSource::Towers};

    // ---- algorithm stages (mirror jetTaggerEmulation.cc::eventLoop) ----
    void overlapRemoval(std::vector<DigiObj>& seeds) const;
    void seedPositionOptimization(std::vector<DigiObj>& seeds) const;
  };

}

#endif // TRIGGEPPERF_JETTAGGERLRJMAKER_H
