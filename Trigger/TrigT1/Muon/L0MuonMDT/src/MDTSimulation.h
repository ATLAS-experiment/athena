/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L0MuonMDT_MDTSIMULATION_H
#define L0MuonMDT_MDTSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "IRegionSelector/IRegSelTool.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h" 
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "L0MuonInterface/RPCCandData.h"
#include "L0MuonInterface/RPCCandDataContainer.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/ServiceHandle.h"




namespace L0Muon {

class MDTSimulation : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~MDTSimulation() = default;
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;


private:



// helper functions
  bool fitRPC(const L0Muon::RPCCandData& cand,
              float& m, float& b,
              std::vector<float>& z_positions,
              std::vector<float>& r_positions) const;

  StatusCode collectMDTHits(const EventContext& ctx, const ActsTrk::GeometryContext& gctx,float eta, float phi, 
                            std::vector<const xAOD::MdtDriftCircle*>& hits, float m, float b) const;

  float computeResidual(const Amg::Vector3D& gpos,float m, float b) const;

  StatusCode Calibration(const xAOD::MdtDriftCircle* mdt,
                         const EventContext& ctx) const;


  // Tools
  ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};
  ToolHandle<IRegSelTool> m_regionSelector{this, "RegSel_MDT", "RegSelTool/RegSelTool_MDT"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
  // Input

  SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtDriftCircleKey{
    this, "MdtDriftCircles", "xMdtDriftCircles"
  };
  
  SG::ReadHandleKey<L0Muon::RPCCandDataContainer> m_barrelCandidateKey{
    this, "L0MuonRPCCandKey", "L0MuonRPCCand"};
};

} // namespace L0Muon

#endif
