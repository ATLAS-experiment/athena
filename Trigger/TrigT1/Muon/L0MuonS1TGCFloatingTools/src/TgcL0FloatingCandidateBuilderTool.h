/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGCANDIDATEBUILDERTOOL_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0FLOATINGCANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0CandidateBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TgcL0FloatingPtLut.h"
#include "TgcL0GoodMagMap.h"

#include <memory>
#include <string>
#include <vector>

namespace L0Muon {

class TgcL0FloatingCandidateBuilderTool final
    : public extends<AthAlgTool, ITgcL0CandidateBuilderTool> {
 public:
  using base_class::base_class;

  StatusCode initialize() override;

  /// \copydoc ITgcL0CandidateBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcL0CandidateContainer& candidates,
                   const EventContext& ctx) const override;

  /// \copydoc ITgcL0CandidateBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcL0CandidateContainer& candidates,
                   TgcL0SegmentContainer* segments,
                   const EventContext& ctx) const override;

 private:
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{
      this, "CablingKey", "MuonTgc_CablingMap"};
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey{
      this, "DetectorManagerKey", "MuonDetectorManager"};

  Gaudi::Property<float> m_maxPivotWireStripDeltaEta{
      this, "MaxPivotWireStripDeltaEta", -1.F,
      "Maximum auxiliary eta-coordinate difference for same-station wire-strip association; negative disables the check"};
  Gaudi::Property<float> m_maxPivotWireStripDeltaPhi{
      this, "MaxPivotWireStripDeltaPhi", 0.35F,
      "Maximum auxiliary phi-coordinate difference for same-station wire-strip association; negative disables the check"};
  Gaudi::Property<unsigned int> m_maxSegmentCombinationsPerGroup{
      this, "MaxSegmentCombinationsPerGroup", 8U,
      "Old Floating projection/candidate working-set size per Trigger Sector "
      "and BC"};
  Gaudi::Property<unsigned int> m_maxCandidatesPerLocalBin{
      this, "MaxCandidatesPerLocalBin", 8U,
      "Maximum candidates retained per old Floating local eta-phi-pivot bin"};

  Gaudi::Property<std::string> m_ptCalibrationFile{
      this, "PtCalibrationFile",
      "L0MuonS1TGC/Floating/eta30_phi20_rel95_physicaldtheta_v2_goodmag_eta20_phi18_slope0p090_v4/"
      "TgcL0PtCalibration.txt",
      "ASCII Floating-pT calibration path relative to the GroupData "
      "development directory"};
  std::unique_ptr<const TgcL0FloatingPtLut> m_ptLut{};

  Gaudi::Property<std::string> m_goodMagMapFile{
      this, "GoodMagMapFile",
      "L0MuonS1TGC/Floating/eta30_phi20_rel95_physicaldtheta_v2_goodmag_eta20_phi18_slope0p090_v4/"
      "TgcL0PoorMagneticFieldMap.txt",
      "ASCII GoodMag map path relative to the GroupData development "
      "directory"};
  std::unique_ptr<const TgcL0GoodMagMap> m_goodMagMap{};
};

}  // namespace L0Muon

#endif
