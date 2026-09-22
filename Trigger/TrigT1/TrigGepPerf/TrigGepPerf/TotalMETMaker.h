/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_TOTALMETMAKER_H
#define TRIGGEPPERF_TOTALMETMAKER_H

/*
  TotalMETMaker:
  -------------
  Bitwise GEP MET core. A port of the standalone metEmulation.cc into TrigGepPerf,
  structured the same way JetTaggerLRJMaker is: everything here is integer, takes
  already-digitized objects in and gives digitized results out, so the same code can be
  driven from Athena, from the standalone emulation, or from GlobalSim.

  Clients whose input is floating point should digitize with the helpers on
  TotalMETConfig first (TotalMETAlg does this); clients whose input is already on the
  grid -- GlobalSim, or a memory-print testbench -- should call makeMETDigitized()
  directly, since re-quantizing a value already on the grid can cost an LSB.

  Four MET flavours are produced in one pass over the same inputs:
    jet MET    -- from the jet collection
    tower MET  -- from the tower collection, optionally with jet overlap removal
    total MET  -- the scale-factor-weighted combination of the two
    GEP JwoJ   -- towers split into hard/soft terms by local block E_T, no jets involved

  The firmware this must agree with bit-for-bit lives in ~/METPhiCalculation
  (MET_Engine.v, MET_PHI_COMPARATOR.v, MET_SUM_SQUARE.v, MET_LUT_SQRT.v, trig.v).
*/

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace Gep {

  // ==================================================================
  // TotalMETConfig
  // ------------------------------------------------------------------
  // Full runtime description of the digitization scheme and algorithm
  // parameters. Base fields are set directly (TotalMETAlg copies them
  // from its Gaudi properties); the derived fields and the LUTs are
  // then filled by computeDerived().
  //
  // Mirrors metConstants/constants.h and emulationHelperFunctions_MET.h
  // in the standalone emulation.
  // ==================================================================
  class TotalMETConfig {
  public:
    // ---------------- algorithm enables ----------------------------
    // Total MET always computes the jet and tower terms internally, since it is their
    // weighted sum; these flags govern which results a client should read, and which
    // containers TotalMETAlg records.
    bool doJetMET     {false};
    bool doTowerMET   {false};
    bool doTotalMET   {true};
    bool doGEPJwoJMET {false};

    // ---------------- physics thresholds / flow --------------------
    double jetEtThresholdGeV       {0.0};   // jets at or below this are dropped
    double towerEtThresholdGeV     {0.0};   // towers at or below this are dropped
    bool   doJetTowerOverlapRemoval{false}; // drop towers within jetConeR of a surviving jet
    double towerScaleFactor        {1.0};   // weight on tower MET in the total
    double jetScaleFactor          {1.0};   // weight on jet MET in the total
    // The jet cone radius belongs to the UPSTREAM jet algorithm, not to MET: it is the
    // WTACone Jet_dR the jets were built with, and overlap removal has to use that same
    // radius to mean anything. No client exposes it as a settable parameter -- it lives
    // here so the two cannot disagree, and changes only if the jet algorithm changes.
    double jetConeR                {0.4};   // overlap-removal radius

    // ---------------- multiplicities -------------------------------
    // maxJetsConsidered is likewise fixed upstream: JET1 emits a fixed multiplicity, so
    // MET takes what it is given. Only the tower cap is a MET parameter.
    unsigned int maxTowersConsidered{4096};
    unsigned int maxJetsConsidered  {10};

    // ---------------- digitization: field widths -------------------
    unsigned int et_bit_length       {13};
    unsigned int signed_et_bit_length{13};  // sign-magnitude: 1 sign + 12 magnitude
    unsigned int eta_bit_length      {7};
    unsigned int phi_bit_length      {6};
    unsigned int sin_bit_length      {13};

    // ---------------- digitization: the tower grid -----------------
    // The grid is sized by its INDEX COUNTS, never by the field widths above: the field
    // is only what an index is transported in. 98 eta towers of 0.1 spanning |eta| < 4.9
    // and 64 phi towers of pi/32 covering 2*pi, matching Athena's
    // CaloTowerContainer::configureGrid(98, -4.9, 4.9, 64).
    unsigned int eta_range      {98};
    unsigned int phi_range      {64};
    double       eta_min        {-4.85}; // first tower center, so a tower lands on an index
    double       eta_granularity{0.1};

    // ---------------- digitization: E_T ----------------------------
    double et_min{0.0};
    double et_max{2048.0};  // GeV; with 13 bits this is the 0.25 GeV LSB

    // Input E_T unit conversion: incoming TLorentzVector Et is in MeV,
    // the emulation works in GeV.
    double inputEtToGeV{1.0e-3};

    // ---------------- MET output azimuth ---------------------------
    // OUTPUT phi is sized by its FIELD WIDTH, the opposite of the tower grid above, and
    // deliberately so. A tower phi indexes a physical grid, so a wider field buys no
    // resolution. The missing-energy azimuth is arctan(Ey, Ex), which is continuous, so
    // the field width is the only thing capping it -- widen the field and the comparator
    // ladder resolves more finely. This width is expected to change, so nothing may
    // hard-code 64 bins or the eight firmware threshold constants.
    unsigned int met_phi_bit_length{6};

    // Fixed-point scale of the tangent comparison, the firmware's Q10. MUST grow with
    // met_phi_bit_length: the thresholds are rounded at this scale, so finer bins need
    // more of it. Roughly met_phi_bit_length + 4 to stay within 0.501 bins of ideal
    // atan2 binning, + 6 to be exact. Keep at 10 while the firmware is at 10.
    unsigned int met_phi_tan_scale_bit_length{10};

    // ---------------- MET magnitude LUT ----------------------------
    // Normalized-LUT square root (MET_LUT_SQRT.v). NOT an exact integer root: it deviates
    // up to 3 output LSBs (0.75 GeV) from floor(sqrt(Ex^2 + Ey^2)). That is a property of
    // the firmware algorithm, not an error to correct -- correcting it would put this out
    // of agreement with the hardware, which is the one thing this class exists to prevent.
    //
    // The radicand is normalized to 2^exponent * f with f in [1, 2); the ROM tabulates
    // sqrt(f) and is addressed by {exponent parity, index}, where the INDEX is f's
    // leading fractional bits -- those sitting just below the radicand's leading one.
    unsigned int sqrt_lut_index_bit_length{9};  // index bits, i.e. the ROM's address width
    unsigned int sqrt_frac_bit_length     {13}; // Q1.13 coefficients
    unsigned int sqrt_coeff_bit_length    {14};
    unsigned int sqrt_radicand_bit_length {26};

    // ---------------- GEP JwoJ -------------------------------------
    // Tower blocks above the hard threshold form the hard term; the towers of every block
    // that did not pass form the soft term. No jets are involved -- that is what "jets
    // without jets" means.
    //
    // The hard/soft coefficients are SEPARATE from jetScaleFactor/towerScaleFactor here,
    // unlike the standalone emulation which reuses that pair. Keeping them apart is what
    // allows the default configuration to run total MET at unit weights while GEP JwoJ
    // uses a 0.3 soft coefficient.
    double       jwojHardEtThresholdGeV{7.5};
    unsigned int jwojBlockSize         {1};   // 1 or 2; must tile both grid axes exactly
    double       jwojHardCoeff         {1.0};
    double       jwojSoftCoeff         {0.3};

    // ---------------- derived (computeDerived) ---------------------
    double       eta_max                 {0.0};
    double       phi_granularity         {0.0};
    double       phi_min                 {0.0};
    double       et_granularity          {0.0};
    unsigned int pi_digitized_in_phi     {0};
    unsigned int half_pi_digitized_in_phi{0};
    unsigned int two_pi_digitized_in_phi {0};
    double       deltaR2_granularity     {0.0};
    unsigned int digitized_delta_R2Cut   {0};

    // Half-index phi grid, for even-sized GEP JwoJ blocks whose center falls on a tower
    // corner rather than a tower center.
    unsigned int phi_half_range               {0};
    unsigned int half_pi_digitized_in_phi_half{0};
    unsigned int two_pi_digitized_in_phi_half {0};

    // MET output azimuth, all derived from met_phi_bit_length.
    unsigned int met_phi_range   {0};
    unsigned int met_phi_quadrant{0};
    unsigned int met_phi_octant  {0};

    // LUTs, all generated rather than transcribed so they cannot drift from the
    // parameters above.
    std::vector<int>          sinLUT;              // sin at tower centers, scaled
    std::vector<int>          sinLUTHalf;          // sin at half-tower steps
    std::vector<unsigned int> metPhiTanThresholds; // comparator ladder
    std::vector<unsigned int> metSqrtLUT;          // Q1.13 normalized-root ROM

    // Fill all derived fields and rebuild the LUTs. Call once per configuration,
    // after the base fields are set.
    void computeDerived(); 

    // True when jwojBlockSize tiles BOTH axes of the tower grid exactly. Checked rather
    // than hard-coded to {1, 2} so the reason stays in the code: a size that leaves a
    // ragged block at the phi seam would push a phi slice systematically into the soft
    // term, i.e. give MET a preferred direction.
    bool isSupportedJwoJBlockSize(unsigned int blockSize) const {
      return blockSize >= 1 && (phi_range % blockSize) == 0 && (eta_range % blockSize) == 0;
    }

    // ---------------- digitization helpers -------------------------
    static inline uint32_t maskN(unsigned int n) {
      return (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1u);
    }

    unsigned int digitize(double value, unsigned int bit_length,
                          double min_val, double max_val,
                          unsigned int altRange = 0) const {
      const unsigned int range = (altRange == 0) ? (1u << bit_length) : altRange;
      const double scale = static_cast<double>(range) / (max_val - min_val);
      if (value < min_val) value = min_val;
      if (value >= max_val) return range - 1;
      return static_cast<unsigned int>(std::round((value - min_val) * scale));
    }

    unsigned int digitizeEt (double etGeV) const { return digitize(etGeV, et_bit_length, et_min, et_max); }
    unsigned int digitizeEta(double eta)   const { return digitize(eta, eta_bit_length, eta_min, eta_max, eta_range); }

    // Phi is periodic while eta and E_T are not: the generic digitize() saturates at
    // range - 1, which is wrong at both ends of the phi axis. A value in the top half
    // tower belongs in index 0, not one past the end of the range.
    unsigned int digitizePhi(double phi) const {
      const int nPhi = static_cast<int>(phi_range);
      const int index = static_cast<int>(std::lround((phi - phi_min) / phi_granularity));
      return static_cast<unsigned int>(((index % nPhi) + nPhi) % nPhi);
    }

    double undigitizeEt(unsigned int v) const { return (v & maskN(et_bit_length)) * et_granularity; }

    // Sign-magnitude, as the TOB carries it: MSB is the sign, the rest the magnitude.
    double undigitizeSignedEt(unsigned int v) const {
      const unsigned int magBits = signed_et_bit_length - 1;
      const int sign = ((v >> magBits) & 1u) ? -1 : 1;
      const int mag  = static_cast<int>(v & maskN(magBits));
      return sign * mag * et_granularity;
    }

    unsigned int wrapPhiUnsigned(unsigned int phi) const {
      return (phi > two_pi_digitized_in_phi) ? (phi - (two_pi_digitized_in_phi + 1)) : phi;
    }
    unsigned int wrapPhiHalfUnsigned(unsigned int phiHalf) const {
      return (phiHalf > two_pi_digitized_in_phi_half) ? (phiHalf - (two_pi_digitized_in_phi_half + 1)) : phiHalf;
    }

    // Digitized deltaR^2 with phi wrap, folded around pi_digitized_in_phi so indices on
    // opposite sides of the +-pi seam come out an index apart, not most of a turn apart.
    unsigned int digitizedDeltaR2(unsigned int eta1, unsigned int phi1,
                                  unsigned int eta2, unsigned int phi2) const {
      unsigned int dEta = static_cast<unsigned int>(std::abs(static_cast<int>(eta1) - static_cast<int>(eta2)));
      unsigned int dPhi = static_cast<unsigned int>(std::abs(static_cast<int>(phi1) - static_cast<int>(phi2)));
      if (dPhi >= pi_digitized_in_phi) dPhi = (2 * pi_digitized_in_phi) - dPhi;
      return dEta * dEta + dPhi * dPhi;
    }

    // ---------------- MET TOB encoders -----------------------------
    // Ports of the firmware modules; see the standalone
    // emulationHelperFunctions_MET.h for the long-form notes.

    // MET_PHI_COMPARATOR.v. Azimuth measured from +x over [0, 2*pi):
    // bin 0 = +x, met_phi_quadrant = +y, 2*quadrant = -x, 3*quadrant = -y.
    // This is NOT the tower phi grid -- different origin, and different bin count at any
    // field width but 6 bits. The zero vector maps to bin 0.
    unsigned int metPhiIndex(int ex, int ey) const;

    // Bin center in radians, folded into (-pi, pi].
    double undigitizeMetPhi(unsigned int phiIndex) const {
      const double phi = (phiIndex % met_phi_range) * (2.0 * M_PI / static_cast<double>(met_phi_range));
      return (phi > M_PI) ? (phi - 2.0 * M_PI) : phi;
    }

    // MET_LUT_SQRT.v: normalized-LUT approximation to floor(sqrt(radicand)).
    unsigned int metLutSqrt(unsigned long long radicand) const;

    // MET_SUM_SQUARE.v: 13-bit magnitude, saturating at its maximum. Two independent
    // overflow conditions, both from the firmware -- a single component of 2^13 or more,
    // and the sum of squares reaching 2^26.
    unsigned int metIntegerRoot(int ex, int ey, bool& overflow) const;

    // Sign-magnitude TOB field, SATURATING (the firmware saturates where the older
    // standalone pack_signed_et masked). The sign bit is forced low at zero magnitude, so
    // there is no negative zero -- a pattern the firmware can never emit.
    unsigned int metTobSignedEt(int value, bool& overflow) const {
      const unsigned int magBits = signed_et_bit_length - 1;
      const unsigned long long mag = static_cast<unsigned long long>(std::llabs(static_cast<long long>(value)));
      overflow = ((mag >> magBits) != 0ull);
      const unsigned int magOut = overflow ? maskN(magBits) : static_cast<unsigned int>(mag);
      const unsigned int sign = (value < 0 && magOut != 0u) ? 1u : 0u;
      return (sign << magBits) | magOut;
    }

    unsigned int metTobSumEt(unsigned long long sumEt, bool& overflow) const {
      overflow = ((sumEt >> et_bit_length) != 0ull);
      return overflow ? maskN(et_bit_length) : static_cast<unsigned int>(sumEt);
    }

    // An accepted input object arrived already clipped at the top of its E_T field, so
    // every sum built from it is a lower bound. Feeds TOB bit [62].
    bool inputEtSaturated(unsigned int digitizedEt) const {
      return digitizedEt >= maskN(et_bit_length);
    }

  private:
    void buildSinLUTs();
    void buildMetPhiThresholds();
    void buildMetSqrtLUT();
  };

  // ==================================================================
  // TotalMETMaker
  // ==================================================================
  class TotalMETMaker {
  public:
    TotalMETMaker() = default;
    virtual ~TotalMETMaker() = default;

    std::string toString() const { return "TotalMET"; }

    // Digitized (et, eta, phi) triplet -- the emulation's tower/jet input object.
    struct DigiObj { unsigned int et{0}; unsigned int eta{0}; unsigned int phi{0}; };

    // One MET term, fully digitized: everything a TOB word needs and nothing that
    // depends on where the inputs came from.
    struct DigiMETTerm {
      int                metX {0};  // full-width accumulator counts, MET = -(sum E_T)
      int                metY {0};
      unsigned int       met  {0};  // magnitude from the LUT root, saturated
      unsigned int       phi  {0};  // azimuth index, from +x (NOT the tower phi grid)
      unsigned long long sumEt{0};  // full-width scalar sum, saturated only at packing
      bool metOverflow   {false};   // the magnitude did not fit its field
      bool inputSaturated{false};   // an accepted input object had already clipped
    };

    // All flavours produced in one pass. Terms whose enable is off are left default.
    // The GEP JwoJ hard and soft terms are UNCOEFFICIENTED, so the coefficients can be
    // re-derived downstream without re-running the algorithm.
    struct DigiMETResult {
      DigiMETTerm jet;
      DigiMETTerm tower;
      DigiMETTerm total;
      DigiMETTerm jwoj;
      DigiMETTerm jwojHard;
      DigiMETTerm jwojSoft;
    };

    // The algorithm proper. This is the firmware-equivalent block.
    //
    // `towers` and `jets` must already be digitized onto the configured grid and already
    // cut to maxTowersConsidered / maxJetsConsidered. E_T thresholds and overlap removal
    // are applied here, not by the caller, because the firmware applies them here.
    DigiMETResult makeMETDigitized(const std::vector<DigiObj>& towers,
                                   const std::vector<DigiObj>& jets) const;

    // Configuration access. Set m_cfg (or its fields) then call
    // m_cfg.computeDerived() once before running the algorithm.
    TotalMETConfig m_cfg;

  protected:
    // ---- algorithm stages (mirror metEmulation.cc::eventLoop) ----
    // Accumulate one already-digitized collection into a MET term. `skip`, when
    // non-empty, drops the objects it marks -- that is how the tower pass applies
    // overlap removal without copying the collection.
    DigiMETTerm accumulate(const std::vector<DigiObj>& objects,
                           double etThresholdGeV,
                           const std::vector<bool>& skip) const;

    // Component-wise weighted combination of two terms, at full accumulator width.
    // The magnitude and azimuth are recomputed from the summed components -- |a + b| is
    // not a function of |a| and |b| -- and the upstream-overflow flag is OR-ed, which is
    // what MET_Engine.v specifies for that bit.
    DigiMETTerm combine(const DigiMETTerm& a, double aCoeff,
                        const DigiMETTerm& b, double bCoeff) const;

    // GEP JwoJ: split the towers into hard and soft terms by block E_T.
    void computeJwoJ(const std::vector<DigiObj>& towers,
                     DigiMETTerm& hard, DigiMETTerm& soft) const;
  };

}

#endif // TRIGGEPPERF_TOTALMETMAKER_H
