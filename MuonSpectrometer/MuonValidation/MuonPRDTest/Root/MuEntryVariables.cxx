/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/MuEntryVariables.h"


#include "TrackRecord/TrackRecordCollection.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"

namespace MuonPRDTest{
  MuEntryVariables::MuEntryVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl):
        PrdTesterModule(tree, "MuEntryVariables", true, msglvl), m_key{container_name} {}
  bool MuEntryVariables::declare_keys() { return declare_dependency(m_key); }

  bool MuEntryVariables::fill(const EventContext& ctx) {
      ATH_MSG_DEBUG("do fillNSWMuEntryVariables()");
      const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
      if (!MuonDetMgr) { return false; }
      SG::ReadHandle<TrackRecordCollection> trackRecordCollection{m_key, ctx};
      if (!trackRecordCollection.isValid()) {
          ATH_MSG_FATAL("Failed to retrieve track collection container " << m_key.fullKey());
          return false;
      }
      m_MuEntry_nParticles = trackRecordCollection->size();
      for(const auto& it : *trackRecordCollection ) {
        m_MuEntry_particlePdg_id.push_back(it.GetPDGCode());
        m_MuEntry_particleBarcode.push_back(it.barcode()); // FIXME barcode-based
        const Amg::Vector3D threeMom = Amg::Hep3VectorToEigen(it.GetMomentum());     
        m_MuEntry_mom.push_back(threeMom.perp(), threeMom.eta(), threeMom.phi(), it.GetEnergy());
        m_MuEntry_pos.push_back(Amg::Hep3VectorToEigen(it.GetPosition()));
    }
    return true;
  }
}

