/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSimulation.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "xAODMuonViews/ChamberViewer.h"
#include <cmath>


namespace L0Muon {

  StatusCode MDTSimulation::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_CHECK(m_mdtDriftCircleKey.initialize());
    ATH_CHECK(m_barrelCandidateKey.initialize());
    ATH_CHECK(m_regionSelector.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibrationTool.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());

  
    return StatusCode::SUCCESS;
  }

  StatusCode MDTSimulation::execute(const EventContext& ctx) const {

    const RPCCandDataContainer* barrelCandidates{nullptr};
    ATH_CHECK(SG::get(barrelCandidates, m_barrelCandidateKey, ctx));
  
    const xAOD::MdtDriftCircleContainer* driftCircles{nullptr};
    ATH_CHECK(SG::get(driftCircles, m_mdtDriftCircleKey, ctx));

    const ActsTrk::GeometryContext* geoCtx{nullptr};
    ATH_CHECK(SG::get(geoCtx, m_geoCtxKey, ctx));


    const ActsTrk::GeometryContext& gctx = *geoCtx;
    std::vector<float> z_positions, r_positions;

    for(const auto& cand : *barrelCandidates) {

      float m=0, b=0;
      z_positions.clear();
      r_positions.clear();
      if (!fitRPC(*cand, m, b, z_positions, r_positions)) {
        ATH_MSG_DEBUG("RPC fit failed for this candidate");
      continue;  // do NOT abort event
      }


      std::vector<const xAOD::MdtDriftCircle*> mdtHits;

      ATH_CHECK(collectMDTHits(ctx, gctx, cand->eta(), cand->phi(), mdtHits, m, b));

    }

    return StatusCode::SUCCESS;
  }


  bool MDTSimulation::fitRPC(const L0Muon::RPCCandData& cand,float& m, float& b, std::vector<float>& z_positions, std::vector<float>& r_positions) const {

    float theta = 2.f * std::atan(std::exp(-cand.eta()));
    float tanTheta = std::tan(theta);

    z_positions.clear();
    r_positions.clear();


    for (int i = 0; i < 4; ++i) {
      const uint16_t z_bits = cand.zPos(i);
      if (!z_bits) continue;
      const float z_norm = static_cast<float>(z_bits) / RPCCandData::s_zPosBitRange;
      const float z_mm = z_norm * RPCCandData::s_zPosRange;

      z_positions.push_back(z_mm);
      r_positions.push_back(z_mm * tanTheta);
    }
    
    size_t N = z_positions.size();
    if (N < 1) return false;  // nothing to fit

    float sumZ=0, sumR=0, sumZZ=0, sumZR=0;
    for (size_t i=0;i<N;++i) {
      sumZ  += z_positions[i];
      sumR  += r_positions[i];
      sumZZ += z_positions[i]*z_positions[i];
      sumZR += z_positions[i]*r_positions[i];
    }

    if (N>=2) {
      float den = N*sumZZ - sumZ*sumZ;
      if (den != 0) {
        m = (N*sumZR - sumZ*sumR) / den;
        b = (sumR - m*sumZ) / N;
      }
    } else {//if the RPC candidate has only one z position we provide the fit using IP
        m = tanTheta;
        b = 0.f;
    }

    return true;
  }

  StatusCode MDTSimulation::collectMDTHits(const EventContext& ctx,  const ActsTrk::GeometryContext& gctx, float eta, float phi, std::vector<const xAOD::MdtDriftCircle*>& hits, float m, float b) const {
   
    hits.clear();

    const float dEta = 0.01f, dPhi = 0.01f;
 

    RoiDescriptor roi(eta, eta-dEta, eta+dEta,
                      phi, phi-dPhi, phi+dPhi);
    TrigRoiDescriptor trigROI(roi);


    auto regSel = m_regionSelector->lookup(ctx);
    if (!regSel) return StatusCode::FAILURE;

    std::vector<IdentifierHash> hashList;
    regSel->HashIDList(trigROI, hashList);


    const xAOD::MdtDriftCircleContainer* driftCircles{};
    ATH_CHECK(SG::get(driftCircles, m_mdtDriftCircleKey, ctx));

    float windowSize = 5;

    ATH_MSG_DEBUG("found n chamber hashes " << hashList.size());
    xAOD::ChamberViewer viewer{*driftCircles, m_idHelperSvc.get(), xAOD::ChamberView::Mode::Chamber};
    IdContext modContext = m_idHelperSvc->mdtIdHelper().module_context();
    for (const IdentifierHash& h : hashList) {
      Identifier mId;
      m_idHelperSvc->mdtIdHelper().get_id(h, mId, &modContext);
      ATH_MSG_DEBUG("looking at chamber " << m_idHelperSvc->toString(mId));
      
      if(!viewer.loadView(mId)){
        continue;
      } 
      for(const xAOD::MdtDriftCircle* dc : viewer){
          const MuonGMR4::MdtReadoutElement* detEl = viewer.at(0)->readoutElement();
          const float pitch = detEl->tubePitch();
          const Amg::Transform3D& locToGlob= detEl->localToGlobalTransform(gctx,dc->measurementHash());
          const Amg::Vector3D gpos = locToGlob* dc->localMeasurementPos() ;
      
          float resZ = computeResidual(gpos, m, b);

            if (std::abs(resZ) > windowSize * pitch) continue;

            hits.push_back(dc);

      } 
    }
    return StatusCode::SUCCESS;
  }


 
  float MDTSimulation::computeResidual(const Amg::Vector3D& gpos, float m, float b) const {
    float z_pred = (m!=0.f ? (gpos.perp() - b)/m : 0.f);
    return gpos.z() - z_pred;
  }



} // namespace L0Muon
