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
#include "xAODL0MuonCand/RPCCandData.h"
#include "xAODL0MuonCand/RPCCandDataContainer.h"
#include "L0MuonMDTTools/IL0MDTSegmentFinderTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/ServiceHandle.h"
#include "L0MuonMDTTools/IPtEstimationTool.h"



namespace L0Muon {

class MDTSimulation : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~MDTSimulation() = default;
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;


private:



// helper functions
  bool fitRPC(const xAOD::RPCCandData& cand,
              float& m, float& b,
              std::vector<float>& z_positions,
              std::vector<float>& r_positions) const;



  /// Collect MDT hits within a RoI around (eta, phi), keeping only those
  /// whose residual from the RPC fit line r = m*z + b is within a window
  /// of a few tube pitches.
  /// @param ctx       Event context
  /// @param gctx      ACTS geometry context for alignment
  /// @param eta       Pseudorapidity of the RPC candidate
  /// @param phi       Azimuthal angle of the RPC candidate
  /// @param hits      Output vector of collected MDT drift circles
  /// @param m         Slope of the RPC fit line in the (z, r) plane
  /// @param b         Intercept of the RPC fit line in the (z, r) plane
  StatusCode collectMDTHits(const EventContext& ctx,
                          const ActsTrk::GeometryContext& gctx,
                          float eta,
                          float phi,
                          std::vector<const xAOD::MdtDriftCircle*>& hits,
                          float m,
                          float b) const;

  /// Compute the residual between a MDT hit global position and the fit line.
  /// @param gpos  Global position of the MDT hit
  /// @param m     Slope of the fit line 
  /// @param b     Intercept of the fit line 
  float computeResidual(const Amg::Vector3D& gpos,float m, float b) const;

  // Tools
  ToolHandle<IRegSelTool> m_regionSelector{this, "RegSel_MDT", "RegSelTool/RegSelTool_MDT"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  
  ToolHandle<L0MDT::IL0MDTSegmentFinderTool> m_csfSegmentFinder{this, "CSFSegmentFinder", "L0MDT::CompactSegmentFinderTool"};
  ToolHandle<L0MDT::IL0MDTSegmentFinderTool> m_legendreSegmentFinder{this, "LegendreSegmentFinder", "L0MDT::LegendreSegmentFinderTool"};
  ToolHandle<L0MDT::IPtEstimationTool> m_ptEstimationTool{this,"PtEstimationTool","L0MDT::PtEstimationTool/PtEstimationTool","Tool to estimate pT from CSF segments"};

  SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
  SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtDriftCircleKey{this, "MdtDriftCircles", "xMdtDriftCircles"};
  SG::ReadHandleKey<xAOD::RPCCandDataContainer> m_barrelCandidateKey{this, "RPCCandKey", "RPCCandData"};

  };
} // namespace L0Muon

#endif
