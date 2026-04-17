/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETTOOLS_FTAGLARGERJETLABELENUM_H
#define PARTICLEJETTOOLS_FTAGLARGERJETLABELENUM_H

/// Simplified FTAG large-R jet truth labelling scheme.
///
/// Factorized as Origin x Decay x Containment, stored as a single flat label.
/// Origin is the ghost-associated parent (H/top/Z/W/QCD) read from the reco jet.
/// Decay is determined by navigating the truth decay chain of that parent.
/// Containment is checked via ghost B/C hadron counts on the reco jet.
///
/// No kinematic cuts (mass, split12/23, jet-truth dR) are applied — the label
/// depends only on ghost associations and truth-record topology.

namespace FtagLargeRLabel {
  enum TypeEnum {
    UNKNOWN = 0,

    // --- Higgs ---
    Hbb       = 1,   ///< H -> bb, nB >= 2
    Hb        = 2,   ///< H -> bb, nB = 1 (partial containment)
    Hcc       = 3,   ///< H -> cc, nC >= 2
    Hc        = 4,   ///< H -> cc, nC = 1 (partial containment)
    Htautau   = 5,   ///< H -> tautau, both taus hadronic
    Htau      = 6,   ///< H -> tautau, nTau = 1 (partial containment)
    HtautauEl = 7,   ///< H -> tautau, one tau -> e
    HtautauMu = 8,   ///< H -> tautau, one tau -> mu
    Hother    = 9,   ///< H ghost-matched, decay products not captured
                     ///  (10-59 below are non-Higgs; Higgs sequence continues at 60)

    // --- Top ---
    TopBqq  = 10,  ///< t -> W(->qq)b, nB >= 1
    TopBcs  = 11,  ///< t -> W(->cs)b, nB >= 1, nC >= 1
    TopBx   = 12,  ///< t -> W(->cs)b, nB >= 1, nC = 0 (b contained, charm not)
    TopBlv  = 13,  ///< t -> W(->lv)b, nB >= 1

    // --- W (standalone, or from top with nB = 0) ---
    Wqq     = 20,  ///< W -> qq (ud, us)
    Wcs     = 21,  ///< W -> cs, nC >= 1
    Wother  = 22,  ///< W leptonic, uncontained, or unresolved

    // --- Z ---
    Zbb       = 30,  ///< Z -> bb, nB >= 2
    Zb        = 31,  ///< Z -> bb, nB = 1 (partial containment)
    Zcc       = 32,  ///< Z -> cc, nC >= 2
    Zc        = 33,  ///< Z -> cc, nC = 1 (partial containment)
    Zss       = 34,  ///< Z -> ss
    Zqq       = 35,  ///< Z -> uu, dd
    Ztautau   = 36,  ///< Z -> tautau, both taus hadronic
    Ztau      = 37,  ///< Z -> tautau, nTau = 1 (partial containment)
    ZtautauEl = 38,  ///< Z -> tautau, one tau -> e
    ZtautauMu = 39,  ///< Z -> tautau, one tau -> mu
    Zother    = 40,  ///< Z leptonic or uncontained

    // --- QCD (from ghost hadron counting) ---
    QCDbb   = 50,  ///< >= 2 ghost B-hadrons
    QCDbc   = 51,  ///< 1B + >= 1C (after B->C dedup)
    QCDcc   = 52,  ///< >= 2C, no B
    QCDbq   = 53,  ///< 1B, 0C
    QCDcq   = 54,  ///< 0B, 1C
    QCDqq   = 55,  ///< no heavy flavour

    // --- Higgs (continued: VV decays) ---
    HWWhad  = 60,  ///< H -> WW, both W -> qq (fully hadronic)
    HWWlep  = 61,  ///< H -> WW, one W -> qq, other W -> lv or tau nu (semileptonic)
    HZZhad  = 62   ///< H -> ZZ, both Z -> qq (fully hadronic)
  };
}

#endif
