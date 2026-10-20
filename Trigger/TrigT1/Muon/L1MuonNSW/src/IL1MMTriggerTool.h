/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONNSW_IL1MMTRIGGERTOOL_H
#define L1MUONNSW_IL1MMTRIGGERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/StatusCode.h"

#include "L1MMDefinitions.h"

#include <vector>

namespace L1Muon::L1MM {

  /**
   * @brief One MM trigger candidate (one road that passed the coincidence), before any
   *        quantisation into the Run 4 NSW-TP -> TGC-SL segment word.
   *
   * Physics values only. Encoding into L1NSWCandData (eta/phi/dtheta indices, quality,
   * board/fiber framing) is done by the caller (NSWSimulation), so that MM and sTGC
   * segments of the same frame can be counted together.
   * The fields below "Validation ntuple" are the rest of the Run 3 slope_t, carried only so
   * that the validation ntuple can have the same MM_diamond_* branches as the Run 3 one.
   */
  struct MMCandidate {
    int sectorPhi{-1};          ///< Trigger sector 1-16 (Athena MM sector convention)
    SideType side{SideType::Undefined};  ///< A (z>0) or C (z<0)
    int BC{-1};                 ///< Bunch crossing of the candidate (mode of the hit BCs, as Run 3)
    double eta{999.};           ///< Global eta of the segment
    double phi{999.};           ///< Local phi (relative to the sector centre)
    double phiShf{999.};        ///< Global phi as computed in Run 3 (MMT_Diamond::phiShift), for comparison
    double phiGlobal{999.};     ///< Global phi in [0, 2pi): sector centre phi from the geometry -/+ local phi
    double dtheta{999.};        ///< Angle w.r.t. the infinite-momentum track [rad]
    bool lowRes{false};         ///< Not all 8 planes in the road (Run 3 flag, not part of the Run 4 word)
    int iX{-1};                 ///< X road index
    int iU{-1};                 ///< U road index
    int iV{-1};                 ///< V road index
    unsigned int nHits{0};      ///< Hits in the road (Run 3 slope.totalCount)
    // Validation ntuple (Run 3 slope_t fields, same meaning)
    char sector{'-'};           ///< 'L' or 'S'
    unsigned int xCount{0};     ///< X hits in the road
    unsigned int uCount{0};     ///< U hits in the road
    int age{-1};                ///< BC of the candidate relative to the start of the processing window
    double mx{999.};            ///< phi-sensitive slope, from the U and V slopes
    double my{999.};            ///< Average R/Z slope of the X hits
    double mxl{999.};           ///< Local slope, R vs Z fit of the X hits
    double uavg{999.};          ///< Average R/Z slope of the U hits
    double vavg{999.};          ///< Average R/Z slope of the V hits
    double theta{999.};         ///< Polar angle of the candidate [rad]
    double secCentrePhi{999.};  ///< Sector centre phi from the Phase-II geometry
  };

  /// One MM trigger hit, for the validation ntuple only (MM_* hit branches)
  struct MMHitInfo {
    int stationEta{0};      ///< Station eta (-2, -1, 1, 2)
    int stationPhi{0};      ///< Station phi (1-8)
    int sectorPhi{0};       ///< Sector 1-16
    char sector{'-'};       ///< 'L' or 'S'
    int plane{-1};          ///< Plane 0-7 of the octuplet
    int strip{-1};          ///< Strip channel
    int VMM{-1};            ///< VMM index on the layer (0-127)
    int ART{-1};            ///< ART ASIC index (0-3)
    int BC{-1};             ///< Bunch crossing from the strip time
    float time{-1.f};       ///< Strip time [ns]
    double R{-1.};          ///< Global radius of the strip centre [mm]
    double Z{-1.};          ///< Global z of the strip centre [mm]
    double RZslope{-1.};    ///< R/Z
    double shift{-1.};      ///< Slope origin of the road window (MMT_Hit::getShift)
  };

  /// Extra information returned by runTrigger on request (used by the validation algorithm)
  struct MMDebugInfo {
    std::vector<MMHitInfo> hits;      ///< every accepted hit of every digit collection
    std::vector<unsigned int> nTrig;  ///< number of coincidences of each collection with at least one
  };

  /**
   * @brief Interface of the Run 4 MM trigger processor emulation.
   *
   * Named IL1MMTriggerTool (not IMMTriggerTool) on purpose: Gaudi derives the interface ID
   * from the class name, so reusing the Run 3 NSWL1::IMMTriggerTool name would give both
   * interfaces the same ID.
   */
  class IL1MMTriggerTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IL1MMTriggerTool, 1, 0);
    virtual ~IL1MMTriggerTool() = default;

    /// Run the MM trigger on the MM digits of the event and return the candidates found.
    /// debug: if not null, also filled with the hits and the per-collection counts.
    virtual StatusCode runTrigger(const EventContext& ctx, std::vector<MMCandidate>& candidates,
                                  MMDebugInfo* debug) const = 0;
  };

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_IL1MMTRIGGERTOOL_H
