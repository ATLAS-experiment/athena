/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGCANDIDATEBUILDERTOOL_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGCANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonS1TGCToolInterfaces/ICandBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "FloatingPtLut.h"
#include "GoodMagMap.h"

#include <memory>
#include <string>
#include <vector>

namespace L1Muon::L1TGC {

class FloatingCandidateBuilderTool final
    : public extends<AthAlgTool, ICandBuilderTool> {
 public:
  using base_class::base_class;

  StatusCode initialize() override;

  /// \copydoc ICandBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcSLCandidateContainer& candidates,
                   const EventContext& ctx) const override;

  /// \copydoc ICandBuilderTool::build
  StatusCode build(const TgcRdoContainer& rdos,
                   TgcSLCandidateContainer& candidates,
                   SegmentContainer* segments,
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
      "L1MuonS1TGC/Floating/eta30_phi20_rel95_physicaldtheta_v2_goodmag_eta20_phi18_slope0p090_v4/"
      "TgcL1PtCalibration.txt",
      "ASCII Floating-pT calibration path relative to the GroupData "
      "development directory"};
  std::unique_ptr<const FloatingPtLut> m_ptLut{};

  Gaudi::Property<std::string> m_goodMagMapFile{
      this, "GoodMagMapFile",
      "L1MuonS1TGC/Floating/eta30_phi20_rel95_physicaldtheta_v2_goodmag_eta20_phi18_slope0p090_v4/"
      "TgcL1PoorMagneticFieldMap.txt",
      "ASCII GoodMag map path relative to the GroupData development "
      "directory"};
  std::unique_ptr<const GoodMagMap> m_goodMagMap{};
};

}  // namespace L1Muon::L1TGC

#endif
