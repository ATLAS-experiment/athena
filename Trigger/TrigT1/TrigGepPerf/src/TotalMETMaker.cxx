/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigGepPerf/TotalMETMaker.h"

#include <algorithm>

namespace Gep {

  // ==================================================================
  // TotalMETConfig: derived constants and LUTs
  // ==================================================================
  void TotalMETConfig::computeDerived() {
    // Tower grid. Everything here is sized from the INDEX COUNTS (eta_range, phi_range),
    // never from the field widths: the field is only what an index travels in.
    eta_max         = eta_min + eta_range * eta_granularity;
    phi_granularity = (2.0 * M_PI) / static_cast<double>(phi_range);
    // First tower CENTER, so a tower digitizes to an integer index instead of landing on
    // a round-half tie halfway between two.
    phi_min         = -M_PI + phi_granularity / 2.0;
    et_granularity  = (et_max - et_min) / static_cast<double>(1u << et_bit_length);

    pi_digitized_in_phi      = phi_range / 2;
    half_pi_digitized_in_phi = pi_digitized_in_phi / 2;
    two_pi_digitized_in_phi  = phi_range - 1;

    deltaR2_granularity = eta_granularity * eta_granularity;
    digitized_delta_R2Cut =
        static_cast<unsigned int>((jetConeR * jetConeR) / deltaR2_granularity + 0.5);

    phi_half_range                = 2 * phi_range;
    half_pi_digitized_in_phi_half = 2 * half_pi_digitized_in_phi;
    two_pi_digitized_in_phi_half  = phi_half_range - 1;

    // MET output azimuth: derived FROM the field width, unlike the tower grid above.
    met_phi_range    = 1u << met_phi_bit_length;
    met_phi_quadrant = met_phi_range / 4;
    met_phi_octant   = met_phi_range / 8;

    buildSinLUTs();
    buildMetPhiThresholds();
    buildMetSqrtLUT();
  }

  // sin at every tower center, scaled by 1 << (sin_bit_length - 1) and rounded to
  // nearest integer.
  void TotalMETConfig::buildSinLUTs() {
    const double amplitude = static_cast<double>(1u << (sin_bit_length - 1));

    sinLUT.clear();
    sinLUT.reserve(phi_range);
    for (unsigned int iPhi = 0; iPhi < phi_range; ++iPhi) {
      const double phi = phi_min + iPhi * phi_granularity;
      sinLUT.push_back(static_cast<int>(std::lround(std::sin(phi) * amplitude)));
    }

    const double halfGranularity = phi_granularity / 2.0;
    sinLUTHalf.clear();
    sinLUTHalf.reserve(phi_half_range);
    for (unsigned int iHalf = 0; iHalf < phi_half_range; ++iHalf) {
      const double phi = phi_min + iHalf * halfGranularity;
      sinLUTHalf.push_back(static_cast<int>(std::lround(std::sin(phi) * amplitude)));
    }
  }

  // Comparator ladder thresholds, T[k] = round(tan((k + 0.5) * 2*pi / met_phi_range) *
  // 2^met_phi_tan_scale_bit_length): the tangent of the boundary between bin k and bin
  // k+1 within the first octant.
  void TotalMETConfig::buildMetPhiThresholds() {
    const double scale = static_cast<double>(1u << met_phi_tan_scale_bit_length);
    metPhiTanThresholds.clear();
    metPhiTanThresholds.reserve(met_phi_octant);
    for (unsigned int k = 0; k < met_phi_octant; ++k) {
      const double angle = (k + 0.5) * (2.0 * M_PI / static_cast<double>(met_phi_range));
      metPhiTanThresholds.push_back(
          static_cast<unsigned int>(std::lround(std::tan(angle) * scale)));
    }
  }

  // Normalized square-root ROM, Q1.13 coefficients.
  // Generated from the closed form
  //     coeff[addr] = round(sqrt((1 + (i + 0.5) / 2^indexBits) * 2^parity) * 2^fracBits)
  // which reproduces all 1024 entries of sqrt_rom in MET_LUT_SQRT.v exactly (verified
  // entry by entry). The + 0.5 puts the entry at the MIDPOINT of the range of f the index
  // stands for, rather than at its bottom edge.
  void TotalMETConfig::buildMetSqrtLUT() {
    const unsigned int lutIndexCount = 1u << sqrt_lut_index_bit_length;
    const unsigned int romSize       = 2u * lutIndexCount;
    const double       fracScale     = static_cast<double>(1u << sqrt_frac_bit_length);
    const unsigned int coeffMax      = maskN(sqrt_coeff_bit_length);

    metSqrtLUT.clear();
    metSqrtLUT.reserve(romSize);
    for (unsigned int addr = 0; addr < romSize; ++addr) {
      const unsigned int parity   = addr >> sqrt_lut_index_bit_length;
      const unsigned int lutIndex = addr & (lutIndexCount - 1);
      const double normalized = (1.0 + (lutIndex + 0.5) / static_cast<double>(lutIndexCount))
                              * (parity ? 2.0 : 1.0);
      unsigned int coeff =
          static_cast<unsigned int>(std::lround(std::sqrt(normalized) * fracScale));
      if (coeff > coeffMax) coeff = coeffMax;
      metSqrtLUT.push_back(coeff);
    }
  }

  // ------------------------------------------------------------------
  // Port/Emulation of MET_PHI_COMPARATOR.v
  // ------------------------------------------------------------------
  unsigned int TotalMETConfig::metPhiIndex(int ex, int ey) const {
    const bool signX = (ex < 0);
    const bool signY = (ey < 0);

    // 64-bit throughout: the accumulator is 27 bits wide in firmware and the ladder
    // scales the minor component by 2^met_phi_tan_scale_bit_length, so the comparison
    // needs 27 + 10 = 37 bits at the present settings and more if either grows.
    const unsigned long long absX =
        static_cast<unsigned long long>(std::llabs(static_cast<long long>(ex)));
    const unsigned long long absY =
        static_cast<unsigned long long>(std::llabs(static_cast<long long>(ey)));

    const bool swap = (absY > absX);
    const unsigned long long majorVal = swap ? absY : absX;
    const unsigned long long minorVal = swap ? absX : absY;
    if (majorVal == 0ull) return 0u; // zero vector -> bin 0

    // Thermometer code: the thresholds ascend, so the number of them passed IS the
    // in-octant bin index (the firmware's popcount over the thermometer bits).
    const unsigned long long minorScaled = minorVal << met_phi_tan_scale_bit_length;
    unsigned int base = 0;
    for (unsigned int k = 0; k < metPhiTanThresholds.size(); ++k)
      if (minorScaled >= majorVal * metPhiTanThresholds[k]) ++base;

    // Fold the octant back out, then the quadrant. The wrap at the end turns a full turn
    // back into bin 0; it is a wrap around the azimuth range, which the firmware gets for
    // free by truncating to the field width because that range is a power of two.
    const unsigned int phiAbs = swap ? (met_phi_quadrant - base) : base;
    unsigned int phi;
    if      (!signX && !signY) phi = phiAbs;                         // quadrant I
    else if ( signX && !signY) phi = 2 * met_phi_quadrant - phiAbs;  // quadrant II
    else if ( signX &&  signY) phi = 2 * met_phi_quadrant + phiAbs;  // quadrant III
    else                       phi = 4 * met_phi_quadrant - phiAbs;  // quadrant IV
    return phi % met_phi_range;
  }

  // ------------------------------------------------------------------
  // Port/emulation of MET_LUT_SQRT.v
  // ------------------------------------------------------------------
  unsigned int TotalMETConfig::metLutSqrt(unsigned long long radicand) const {
    if (radicand == 0ull) return 0u;

    // Leading-one position, the RTL's leading_one_pos().
    unsigned int exponent = 0;
    while ((radicand >> (exponent + 1)) != 0ull) ++exponent;

    // The LUT index is the bits immediately below the leading one -- that is, the leading
    // fractional bits of the radicand once it has been normalized into [1, 2). They are
    // zero-padded on the right when the radicand is too small to supply them all.
    // Exponent 0 yields 0, matching the RTL's unlisted case falling through to its default.
    const unsigned int lutIndexCount = 1u << sqrt_lut_index_bit_length;
    unsigned int lutIndex;
    if (exponent >= sqrt_lut_index_bit_length)
      lutIndex = static_cast<unsigned int>((radicand >> (exponent - sqrt_lut_index_bit_length))
                                           & (lutIndexCount - 1));
    else
      lutIndex = static_cast<unsigned int>((radicand << (sqrt_lut_index_bit_length - exponent))
                                           & (lutIndexCount - 1));

    const unsigned int romAddr    = ((exponent & 1u) << sqrt_lut_index_bit_length) | lutIndex;
    const unsigned int scaleShift = exponent >> 1;

    // The RTL's scaled_coeff is sqrt_radicand_bit_length wide, so the shift TRUNCATES
    // there. Masking is not cosmetic: without it a large coefficient and shift would
    // carry bits the firmware silently drops.
    const unsigned long long scaledMask = (1ull << sqrt_radicand_bit_length) - 1ull;
    const unsigned long long scaled =
        (static_cast<unsigned long long>(metSqrtLUT[romAddr]) << scaleShift) & scaledMask;

    return static_cast<unsigned int>((scaled >> sqrt_frac_bit_length) & maskN(et_bit_length));
  }

  // ------------------------------------------------------------------
  // Port/emulation MET_SUM_SQUARE.v
  // ------------------------------------------------------------------
  unsigned int TotalMETConfig::metIntegerRoot(int ex, int ey, bool& overflow) const {
    const unsigned long long absX =
        static_cast<unsigned long long>(std::llabs(static_cast<long long>(ex)));
    const unsigned long long absY =
        static_cast<unsigned long long>(std::llabs(static_cast<long long>(ey)));

    // Two independent overflow conditions, both from the firmware and both needed: a
    // single component of 2^et_bit_length or more already guarantees the magnitude
    // overflows, and independently the sum of squares reaching 2^26 does (that carry bit
    // IS the threshold, since sqrt(2^26) = 8192).
    const bool componentOverflow = ((absX >> et_bit_length) != 0ull) ||
                                   ((absY >> et_bit_length) != 0ull);

    // The firmware squares only the low et_bit_length bits, which is harmless precisely
    // because the component test has already fired in that case.
    const unsigned long long exLow = absX & maskN(et_bit_length);
    const unsigned long long eyLow = absY & maskN(et_bit_length);
    const unsigned long long squareSum = exLow * exLow + eyLow * eyLow;

    overflow = componentOverflow || ((squareSum >> sqrt_radicand_bit_length) != 0ull);
    if (overflow) return maskN(et_bit_length);

    return metLutSqrt(squareSum);
  }

  // ==================================================================
  // TotalMETMaker: algorithm stages
  // ==================================================================

  // Accumulate one already-digitized collection into a MET term.
  //
  // The E_T threshold is compared in DIGITIZED units, not in GeV. 
  TotalMETMaker::DigiMETTerm
  TotalMETMaker::accumulate(const std::vector<DigiObj>& objects,
                            double etThresholdGeV,
                            const std::vector<bool>& skip) const {
    DigiMETTerm term;

    const unsigned int etThreshold = m_cfg.digitizeEt(etThresholdGeV);
    const int sinScale = 1 << (m_cfg.sin_bit_length - 1);

    int etXSum = 0;
    int etYSum = 0;

    for (unsigned int iObject = 0; iObject < objects.size(); ++iObject) {
      if (!skip.empty() && skip[iObject]) continue;

      const DigiObj& object = objects[iObject];
      if (object.et <= etThreshold) continue;

      // Cosine is the sine table read a quarter turn along, which is exact here because
      // pi/2 is a whole number of phi indices on this grid.
      const int cosPhi = m_cfg.sinLUT[m_cfg.wrapPhiUnsigned(object.phi + m_cfg.half_pi_digitized_in_phi)];
      const int sinPhi = m_cfg.sinLUT[object.phi];

      etXSum += (static_cast<int>(object.et) * cosPhi) / sinScale;
      etYSum += (static_cast<int>(object.et) * sinPhi) / sinScale;

      term.sumEt += object.et;
      if (m_cfg.inputEtSaturated(object.et)) term.inputSaturated = true;
    }

    // MET is the negative of the vector E_T sum, as everywhere else in this chain.
    term.metX = -etXSum;
    term.metY = -etYSum;
    term.met  = m_cfg.metIntegerRoot(term.metX, term.metY, term.metOverflow);
    term.phi  = m_cfg.metPhiIndex(term.metX, term.metY);
    return term;
  }

  // Weighted component-wise combination, at full accumulator width.
  TotalMETMaker::DigiMETTerm
  TotalMETMaker::combine(const DigiMETTerm& a, double aCoeff,
                         const DigiMETTerm& b, double bCoeff) const {
    DigiMETTerm out;
    out.metX = static_cast<int>(std::lround(aCoeff * a.metX + bCoeff * b.metX));
    out.metY = static_cast<int>(std::lround(aCoeff * a.metY + bCoeff * b.metY));

    // The scalar sum is NOT weighted: SumET is the total energy seen, which the
    // coefficients on the vector terms have no bearing on.
    out.sumEt = a.sumEt + b.sumEt;

    // TOB bit [62] is an OR, specifying if either set of input TOBs has a saturated TOB
    out.inputSaturated = a.inputSaturated || b.inputSaturated;

    out.met = m_cfg.metIntegerRoot(out.metX, out.metY, out.metOverflow);
    out.phi = m_cfg.metPhiIndex(out.metX, out.metY);
    return out;
  }

  // ------------------------------------------------------------------
  // GEP JwoJ
  // ------------------------------------------------------------------
  // The same towers and the same tower E_T threshold as the tower term, but split into a
  // hard and a soft term at a per-block threshold instead of being summed as one. The
  // hard term stands in for the jet term -- it is the high-E_T part of the event
  void TotalMETMaker::computeJwoJ(const std::vector<DigiObj>& towers,
                                  DigiMETTerm& hard, DigiMETTerm& soft) const {
    hard = DigiMETTerm();
    soft = DigiMETTerm();

    const unsigned int blockSize = m_cfg.jwojBlockSize;
    const unsigned int nEtaBlocks = m_cfg.eta_range / blockSize;
    const unsigned int nPhiBlocks = m_cfg.phi_range / blockSize;
    const unsigned int nBlocks    = nEtaBlocks * nPhiBlocks;

    const unsigned int etThreshold = m_cfg.digitizeEt(m_cfg.towerEtThresholdGeV);
    const int sinScale = 1 << (m_cfg.sin_bit_length - 1);

    // Per block: the summed digitized E_T that carries the vector sum, and a flag once
    // the block has been promoted to the hard term. Only blocks an event actually
    // touched are reset afterwards, so the per-event cost is O(occupied blocks) rather
    // than O(grid).
    std::vector<unsigned int>  blockEt(nBlocks, 0u);
    std::vector<unsigned char> blockSeen(nBlocks, 0u);
    std::vector<unsigned char> blockIsHard(nBlocks, 0u);
    std::vector<unsigned int>  touchedBlocks;
    touchedBlocks.reserve(towers.size());

    // Each surviving tower's digitized values and the block it landed in, kept from the
    // first pass so the soft-term pass does not re-derive them.
    struct JwoJTower { unsigned int et, phi, eta, blockIdx; };
    std::vector<JwoJTower> jwojTowers;
    jwojTowers.reserve(towers.size());

    // ---- pass 1: bin the surviving towers into blocks ----------------------------
    // A tower's block is integer division of its indices. blockSize tiles both axes
    // exactly, so every block is full and no tower falls outside the grid.
    for (const DigiObj& tower : towers) {
      if (tower.et <= etThreshold) continue;

      const unsigned int blockEta = tower.eta / blockSize;
      const unsigned int blockPhi = tower.phi / blockSize;
      const unsigned int blockIdx = blockEta * nPhiBlocks + blockPhi;
      if (blockIdx >= nBlocks) continue; // cannot happen on a tiled grid; cheap guard

      if (!blockSeen[blockIdx]) {
        blockSeen[blockIdx] = 1;
        touchedBlocks.push_back(blockIdx);
      }
      blockEt[blockIdx] += tower.et;
      jwojTowers.push_back({tower.et, tower.phi, tower.eta, blockIdx});
    }

    // ---- pass 2: hard term, one contribution per block above threshold -----------
    // The block sits at its geometric center. In HALF-indices that center is
    // firstIndex + lastIndex on each axis, which is an integer whatever the block size,
    // and whose parity says whether the center lands on a tower center (odd blockSize)
    // or on a tower corner (even). sinLUTHalf covers both.
    //
    // One multiply per block rather than one per tower is the point of blocking, and it
    // is also a real numerical difference from summing per-tower products: the rounding
    // of the E_T x sin/cos product now happens once, on the block sum.
    // TODO study whether to - after finding hard/soft term split - use tower OR block centers
    // for hard term MET computation
    const unsigned int hardThreshold = m_cfg.digitizeEt(m_cfg.jwojHardEtThresholdGeV);
    int hardETxSum = 0, hardETySum = 0;
    for (unsigned int iBlock = 0; iBlock < touchedBlocks.size(); ++iBlock) {
      const unsigned int blockIdx = touchedBlocks[iBlock];
      if (blockEt[blockIdx] <= hardThreshold) continue;
      blockIsHard[blockIdx] = 1;

      const unsigned int blockPhi = blockIdx % nPhiBlocks;
      const unsigned int firstPhi = blockPhi * blockSize;
      const unsigned int lastPhi  = firstPhi + blockSize - 1;
      const unsigned int blockPhiHalf = firstPhi + lastPhi;

      const int cosPhi = m_cfg.sinLUTHalf[m_cfg.wrapPhiHalfUnsigned(blockPhiHalf + m_cfg.half_pi_digitized_in_phi_half)];
      const int sinPhi = m_cfg.sinLUTHalf[blockPhiHalf];

      const int blockEtValue = static_cast<int>(blockEt[blockIdx]);
      hardETxSum += (blockEtValue * cosPhi) / sinScale;
      hardETySum += (blockEtValue * sinPhi) / sinScale;
      hard.sumEt += blockEt[blockIdx];
      if (m_cfg.inputEtSaturated(blockEt[blockIdx])) hard.inputSaturated = true;
    }

    // ---- pass 3: soft term, the towers of every block that did not pass ----------
    // Per tower and at the tower's own center
    int softETxSum = 0, softETySum = 0;
    for (const JwoJTower& tower : jwojTowers) {
      if (blockIsHard[tower.blockIdx]) continue;

      const int cosPhi = m_cfg.sinLUT[m_cfg.wrapPhiUnsigned(tower.phi + m_cfg.half_pi_digitized_in_phi)];
      const int sinPhi = m_cfg.sinLUT[tower.phi];

      const int towerEt = static_cast<int>(tower.et);
      softETxSum += (towerEt * cosPhi) / sinScale;
      softETySum += (towerEt * sinPhi) / sinScale;
      soft.sumEt += tower.et;
      if (m_cfg.inputEtSaturated(tower.et)) soft.inputSaturated = true;
    }

    hard.metX = -hardETxSum;
    hard.metY = -hardETySum;
    hard.met  = m_cfg.metIntegerRoot(hard.metX, hard.metY, hard.metOverflow);
    hard.phi  = m_cfg.metPhiIndex(hard.metX, hard.metY);

    soft.metX = -softETxSum;
    soft.metY = -softETySum;
    soft.met  = m_cfg.metIntegerRoot(soft.metX, soft.metY, soft.metOverflow);
    soft.phi  = m_cfg.metPhiIndex(soft.metX, soft.metY);
  }

  // ==================================================================
  // TotalMETMaker: the algorithm
  // ==================================================================
  TotalMETMaker::DigiMETResult
  TotalMETMaker::makeMETDigitized(const std::vector<DigiObj>& towers,
                                  const std::vector<DigiObj>& jets) const {
    DigiMETResult result;

    // Clamp to the configured multiplicities. Done here rather than trusted from the
    // caller because it is a property of the firmware's input buffers, not of whoever
    // happened to fill the vectors.
    const unsigned int nJets   = std::min<unsigned int>(m_cfg.maxJetsConsidered,   jets.size());
    const unsigned int nTowers = std::min<unsigned int>(m_cfg.maxTowersConsidered, towers.size());
    const std::vector<DigiObj> jetsUsed  (jets.begin(),   jets.begin()   + nJets);
    const std::vector<DigiObj> towersUsed(towers.begin(), towers.begin() + nTowers);

    // Jet term. Always computed: total MET is its weighted sum with the tower term, so
    // the enables govern what is published, not what is calculated.
    result.jet = accumulate(jetsUsed, m_cfg.jetEtThresholdGeV, {});

    // Jet/tower overlap removal. Built against the jets that SURVIVED the jet E_T threshold
    std::vector<bool> towerSkip;
    if (m_cfg.doJetTowerOverlapRemoval) {
      const unsigned int jetEtThreshold = m_cfg.digitizeEt(m_cfg.jetEtThresholdGeV);
      towerSkip.assign(nTowers, false);
      for (unsigned int iTower = 0; iTower < nTowers; ++iTower) {
        for (unsigned int iJet = 0; iJet < nJets; ++iJet) {
          if (jetsUsed[iJet].et <= jetEtThreshold) continue;
          const unsigned int dR2 = m_cfg.digitizedDeltaR2(jetsUsed[iJet].eta, jetsUsed[iJet].phi,
                                                          towersUsed[iTower].eta, towersUsed[iTower].phi);
          if (dR2 <= m_cfg.digitized_delta_R2Cut) { towerSkip[iTower] = true; break; }
        }
      }
    }

    result.tower = accumulate(towersUsed, m_cfg.towerEtThresholdGeV, towerSkip);

    // Total MET: the scale-factor-weighted combination.
    result.total = combine(result.tower, m_cfg.towerScaleFactor,
                           result.jet,   m_cfg.jetScaleFactor);

    if (m_cfg.doGEPJwoJMET) {
      computeJwoJ(towersUsed, result.jwojHard, result.jwojSoft);
      // Hard and soft are outputted UNCOEFFICIENTED so the coefficients can be re-derived
      // downstream; only the combination applies them.
      result.jwoj = combine(result.jwojHard, m_cfg.jwojHardCoeff,
                            result.jwojSoft, m_cfg.jwojSoftCoeff);
    }

    return result;
  }

}
