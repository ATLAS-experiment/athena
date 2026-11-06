/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_TOWERLUTS_H
#define GLOBALSIM_TOWERLUTS_H

/*
  LUTs of the GEP algorithms built from a tower BitSpec's eta/phi fields rather than from
  hard-coded grid constants: every entry is evaluated at the values the field decodes its codes to.
  The tables are laid out exactly as TotalMETConfig / JetTaggerLRJConfig build them in
  computeDerived(), and are handed over through their setSinLUTs / setDeltaRLut.
*/

#include <cmath>
#include <utility>
#include <vector>

namespace GlobalSim {

  // The tower eta/phi fields carry their grid indices in the low bits; the top bits are zero padding.
  // Hard-coded until the BitSpec can state the code range it uses.
  constexpr unsigned int unusedEtaPhiBits = 3;

  // Grid codes of a tower eta/phi field: 2^(field width - padding).
  template<class Field>
  constexpr unsigned int nGridCodes(const Field&) { return 1u << (Field::width - unusedEtaPhiBits); }

  // sin, scaled by 1 << (sinBitLength - 1), at every tower centre (first) and at every half-tower
  // step (second): entry 2k is the centre of tower k, entry 2k + 1 its upper edge.
  template<class PhiField>
  std::pair<std::vector<int>, std::vector<int>> makeSinLUTs(const PhiField& phi, unsigned int sinBitLength) {
    const double amplitude = static_cast<double>(1u << (sinBitLength - 1));
    const unsigned int nCodes = nGridCodes(phi);

    std::vector<int> sinLUT, sinLUTHalf;
    sinLUT.reserve(nCodes);
    sinLUTHalf.reserve(2 * nCodes);
    for (unsigned int code = 0; code < nCodes; ++code) {
      const int centre = static_cast<int>(std::lround(std::sin(static_cast<double>(phi.decode(code))) * amplitude));
      const int upper  = static_cast<int>(std::lround(std::sin(static_cast<double>(phi.range(code).second)) * amplitude));
      sinLUT.push_back(centre);
      sinLUTHalf.push_back(centre);
      sinLUTHalf.push_back(upper);
    }
    return {std::move(sinLUT), std::move(sinLUTHalf)};
  }

  // deltaR LUT of the large-R jet tagger, indexed dEta * (phi codes / 2) + dPhi, digitized at
  // 2 * rCut / 2^psiRBitLength and truncated after the last entry below 2 * rCut.
  // The eta/phi steps are the fields' code widths, averaged over the grid so the float rounding of a
  // single decoded value does not bias them.
  template<class EtaField, class PhiField>
  std::vector<unsigned int> makeDeltaRLut(const EtaField& eta, const PhiField& phi,
                                          double rCut, unsigned int psiRBitLength) {
    const unsigned int nEta     = nGridCodes(eta);
    const unsigned int nPhi     = nGridCodes(phi);
    const unsigned int nPhiHalf = nPhi / 2;
    const double etaStep = (static_cast<double>(eta.decode(nEta - 1)) - static_cast<double>(eta.decode(0))) / (nEta - 1);
    const double phiStep = (static_cast<double>(phi.decode(nPhi - 1)) - static_cast<double>(phi.decode(0))) / (nPhi - 1);

    const double cut            = 2 * rCut;
    const double lutGranularity = cut / static_cast<double>(1u << psiRBitLength);
    const unsigned int clampMax = (1u << psiRBitLength) - 1;

    std::vector<unsigned int> lut;
    lut.reserve(nEta * nPhiHalf);
    unsigned int lastBelowCut = 0;
    for (unsigned int etaIt = 0; etaIt < nEta; ++etaIt) {
      for (unsigned int phiIt = 0; phiIt < nPhiHalf; ++phiIt) {
        const double dEta = etaIt * etaStep;
        const double dPhi = phiIt * phiStep;
        const double dR   = std::sqrt(dEta * dEta + dPhi * dPhi);
        unsigned int digi = static_cast<unsigned int>(dR / lutGranularity + 0.5);
        if (digi > clampMax) digi = clampMax;
        if (dR < cut) lastBelowCut = lut.size();
        lut.push_back(digi);
      }
    }
    lut.resize(lastBelowCut + 1);
    return lut;
  }

}

#endif
