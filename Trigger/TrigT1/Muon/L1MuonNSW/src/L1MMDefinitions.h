/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONNSW_L1MMDEFINITIONS_H
#define L1MUONNSW_L1MMDEFINITIONS_H

#include <ostream>

/**
 * Definitions shared by the classes of the MM trigger processor emulation.
 *
 * The electronics constants describe the nominal readout of one MM sector layer (all eight planes are
 * read out the same way). They are not part of the readout geometry, which only knows the active
 * strips of each module (firstStrip, numStrips): a few nominal strips at the module edges are not
 * mounted, but the electronics numbering still counts them.
 */
namespace L1Muon::L1MM {

  /// Detector side. The value is the sign of z, so that it can be used as a sign (e.g. Acts::copySign).
  enum class SideType : int { A = 1, C = -1, Undefined = 0 };

  inline std::ostream& operator<<(std::ostream& os, const SideType& side) {
    return os << (side == SideType::A ? 'A' : (side == SideType::C ? 'C' : '-'));
  }

  /// Type of an MM layer: X (eta strips), U (stereo +1.5 deg) or V (stereo -1.5 deg)
  enum class LayerType : int { X, U, V, Undefined };

  /// Planes of the MM octuplet (2 multilayers x 4 gas gaps)
  constexpr int kPlanes = 8;
  /// Strips read out by one VMM
  constexpr int kStripsPerVMM = 64;
  /// VMMs on one MMFE8 board
  constexpr int kVMMsPerMMFE8 = 8;
  /// Strips read out by one MMFE8 board
  constexpr int kStripsPerMMFE8 = kStripsPerVMM * kVMMsPerMMFE8;
  /// MMFE8 boards of one layer of the |stationEta| = 1 module (5 PCBs x 2)
  constexpr int kMMFE8sEta1 = 10;
  /// MMFE8 boards of one sector layer (8 PCBs x 2: 10 for |stationEta| = 1, 6 for |stationEta| = 2)
  constexpr int kMMFE8sPerLayer = 16;
  /// PCBs of one sector layer (2 MMFE8 boards per PCB)
  constexpr int kPCBsPerLayer = kMMFE8sPerLayer / 2;
  /// MMFE8 boards in each radial half of the layer (inner: 0-7, outer: 8-15)
  constexpr int kMMFE8sPerHalfLayer = kMMFE8sPerLayer / 2;
  /// Nominal strips of one layer of the |stationEta| = 1 module (5120)
  constexpr int kStripsEta1 = kStripsPerMMFE8 * kMMFE8sEta1;
  /// Nominal strips of one sector layer, both modules (8192)
  constexpr int kStripsPerLayer = kStripsPerMMFE8 * kMMFE8sPerLayer;
  /// VMMs of one sector layer (128)
  constexpr int kVMMsPerLayer = kStripsPerLayer / kStripsPerVMM;
  /// ART ASICs of one sector layer (2 ADDC cards x 2 ART ASICs)
  constexpr int kARTsPerLayer = 4;
  /// Hits that one ART ASIC can forward per bunch crossing
  constexpr int kMaxHitsPerART = 8;

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_L1MMDEFINITIONS_H
