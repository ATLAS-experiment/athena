/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONNSW_MMTRIGGERTOOL_H
#define L1MUONNSW_MMTRIGGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include "IL1MMTriggerTool.h"
#include "MMT_Diamond.h"

#include <memory>

namespace L1Muon::L1MM {

  /**
   * @brief Run 4 emulation of the MM trigger processor.
   *
   * Ported from the Run 3 NSWL1::MMTriggerTool (Trigger/TrigT1/TrigT1NSWSimTools), which is
   * left untouched: hits are built with the Phase-II geometry (MMT_Hit), roads and coincidences
   * with the unchanged Diamond Road algorithm (MMT_Road, MMT_Diamond). The output is a list of
   * MMCandidate (physics values). The Run 3 RDO encoding is not ported, the Run 4 encoding is
   * done by the caller.
   */
  class MMTriggerTool : public extends<AthAlgTool, IL1MMTriggerTool> {
  public:
    using base_class::base_class;
    virtual ~MMTriggerTool() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode runTrigger(const EventContext& ctx, std::vector<MMCandidate>& candidates,
                                  MMDebugInfo* debug) const override;

  private:
    SG::ReadHandleKey<MmDigitContainer> m_keyMmDigit{this, "MMDigits", "MM_DIGITS",
                                                      "Input MM digit container"};
    /// Phase-II geometry context, carrying the alignment (same key as L1MuonMDT and L1MuonS1RPC)
    ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment",
                                                             "Geometry context with the alignment"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc",
                                                        "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // Parameters of the Diamond Road algorithm (same names and defaults as Run 3)
    Gaudi::Property<int>  m_diamRoadSize{this, "DiamondRoadSize", 8, "Number of strips to create a road"};
    Gaudi::Property<bool> m_uv{this, "DiamondUV", true, "Include Stereo planes for tracking"};
    Gaudi::Property<int>  m_diamXthreshold{this, "DiamondEtaThreshold", 3,
                                           "Number of Eta planes for coincidences"};
    Gaudi::Property<int>  m_diamUVthreshold{this, "DiamondStereoThreshold", 3,
                                            "Number of Stereo planes for coincidences"};
    Gaudi::Property<int>  m_diamOverlapEtaUp{this, "DiamondEtaUpOverlap", 4,
                                             "Number of Eta strips for upper road overlap"};
    Gaudi::Property<int>  m_diamOverlapEtaDown{this, "DiamondEtaDownOverlap", 0,
                                               "Number of Eta strips for lower road overlap"};
    Gaudi::Property<int>  m_diamOverlapStereoUp{this, "DiamondStereoUpOverlap", 4,
                                                "Number of Stereo strips for upper road overlap"};
    Gaudi::Property<int>  m_diamOverlapStereoDown{this, "DiamondStereoDownOverlap", 0,
                                                  "Number of Stereo strips for lower road overlap"};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    std::unique_ptr<MMT_Diamond> m_diamond;
  };

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_MMTRIGGERTOOL_H
