/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MuonMDT_MDTSIMULATION_H
#define L1MuonMDT_MDTSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "IRegionSelector/IRegSelTool.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h" 
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "xAODTrigL1Muon/L1RPCCandData.h"
#include "xAODTrigL1Muon/L1RPCCandDataContainer.h"
#include "L1MuonMDTTools/ISegmentFinderTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/ServiceHandle.h"
#include "L1MuonMDTTools/IPtEstimationTool.h"
#include "L1MuonMDTTools/CandidateInfo.h"
#include "StoreGate/WriteHandleKey.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

#include <array>



namespace L1Muon {

class MDTSimulation : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~MDTSimulation() = default;
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;


private:



// helper functions
  /// Fit the line R = m * z + b through the RPC hits of the candidate. The candidate provides the z
  /// of the hits of each RPC layer (BI, BM doublet 1, BM doublet 2, BO) at a fixed radius per layer,
  /// which depends only on whether the sector (from phi) is large or small (L1MDT::rpcLayerRadius).
  /// With a single layer the line points to the IP.
  /// @param z_positions, r_positions  output: hit position of each layer (0 if the layer has no hit)
  /// @return false if the candidate has no RPC hit
  bool fitRPC(const xAOD::L1RPCCandData& cand,
              float eta, float phi,
              float& m, float& b,
              std::array<float, 4>& z_positions,
              std::array<float, 4>& r_positions) const;




  /// Hit extraction: in each station, the chamber of the sector of the candidate (from phi) crossed by
  /// the RPC fit line r = m*z + b, among those of the RoI around (eta, phi), and its neighbours along
  /// eta; keep their hits within m_heHalfWidth tubes of the line in every tube layer (the window can
  /// continue into the neighbouring chamber) and, if the calibration is available, with a drift time
  /// in the time window.
  /// @param ctx       Event context
  /// @param gctx      ACTS geometry context for alignment
  /// @param eta       Pseudorapidity of the RPC candidate
  /// @param phi       Azimuthal angle of the RPC candidate
  /// @param hits      Output vector of collected MDT drift circles
  /// @param m         Slope of the RPC fit line in the (z, r) plane
  /// @param b         Intercept of the RPC fit line in the (z, r) plane
  /// @param regSelChambers Output chambers (module identifiers) returned by the region selector
  /// @param stationChambers Output chambers used in each station slot (crossed chamber first)
  StatusCode collectMDTHits(const EventContext& ctx,
                          const ActsTrk::GeometryContext& gctx,
                          float eta,
                          float phi,
                          std::vector<const xAOD::MdtDriftCircle*>& hits,
                          float m,
                          float b,
                          std::vector<Identifier>& regSelChambers,
                          std::array<std::vector<Identifier>, L1MDT::nStationSlots>& stationChambers) const;

  Gaudi::Property<int> m_heHalfWidth{this, "HEHalfWidth", 5,
      "Hit extraction window: tubes on each side of the candidate line, in every tube layer"};
  /// Drift time (t0, time of flight) and time window of the hit extraction; no time window if empty
  ToolHandle<IMdtCalibrationTool> m_calibTool{this, "CalibrationTool", "",
      "MDT calibration tool for the hit extraction time window (none if empty)"};

  // Tools
  ToolHandle<IRegSelTool> m_regionSelector{this, "RegSel_MDT", "RegSelTool/RegSelTool_MDT"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  
  ToolHandle<L1MDT::ISegmentFinderTool> m_csfSegmentFinder{this, "CSFSegmentFinder", "L1Muon::L1MDT::CompactSegmentFinderTool"};
  ToolHandle<L1MDT::ISegmentFinderTool> m_legendreSegmentFinder{this, "LegendreSegmentFinder", "L1Muon::L1MDT::LegendreSegmentFinderTool"};
  ToolHandle<L1MDT::IPtEstimationTool> m_ptEstimationTool{this,"PtEstimationTool","L1Muon::L1MDT::PtEstimationTool/PtEstimationTool","Tool to estimate pT from CSF segments"};

  const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
  /// Sector of the candidate from phi, with the actual large / small sector sizes
  Muon::MuonSectorMapping m_sectorMapping{};

  SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
  SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_mdtDriftCircleKey{this, "MdtDriftCircles", "xMdtDriftCircles"};
  SG::ReadHandleKey<xAOD::L1RPCCandDataContainer> m_barrelCandidateKey{this, "RPCCandKey", "L1RPCCandData"};
  /// Optional transient output with the per-candidate intermediate results (hits, segments, pT).
  /// Nothing is written if the key is empty.
  SG::WriteHandleKey<L1MDT::CandidateInfoContainer> m_candInfoKey{this, "CandidateInfoKey", "",
      "Transient output with the hits, segments and pT estimate of each processed candidate"};

  };
} // namespace L1Muon

#endif
