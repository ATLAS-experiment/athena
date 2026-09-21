/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MuonMDTTools_IPTESTIMATIONTOOL_H
#define L1MuonMDTTools_IPTESTIMATIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "L1MuonMDTTools/L0MDTSegment.h"
#include <optional>

namespace L0MDT {

/// Output struct holding the result of a pT estimation.
/// All fields are meaningful only when the optional returned by estimatePt is not empty.
struct PtEstimate {
  unsigned int nStations{0};  ///< Number of stations used in the estimate

  float pt{0.f};         ///< pT proxy value (not a calibrated momentum)
  float deltaBeta{0.f};  ///< Angular deflection between stations (2-station mode)
  float sagitta{0.f};    ///< Sagitta of the track (3-station mode)
  float leverArm{0.f};   ///< Distance between inner and outer station (3-station mode)
};

/**
 * @class IPtEstimationTool
 * @brief Interface for pT estimation tools in the L0Muon MDT chain.
 *
 * Concrete implementations may use different strategies (geometric proxy,
 * magnetic field lookup, calibration tables, etc.).
 */
class IPtEstimationTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IPtEstimationTool, 1, 0);
  virtual ~IPtEstimationTool() = default;

  /// Estimate the transverse momentum from up to three MDT segment references.
  /// @param biSeg  Pointer to the BI segment, or nullptr if not available
  /// @param bmSeg  Pointer to the BM segment, or nullptr if not available
  /// @param boSeg  Pointer to the BO segment, or nullptr if not available
  /// @return A PtEstimate if at least two stations are available, std::nullopt otherwise
  virtual std::optional<PtEstimate> estimatePt(const Segment* biSeg,
                                               const Segment* bmSeg,
                                               const Segment* boSeg) const = 0;
};

}  // namespace L0MDT

#endif
