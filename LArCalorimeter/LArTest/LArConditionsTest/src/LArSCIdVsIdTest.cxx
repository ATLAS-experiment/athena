#include "LArSCIdVsIdTest.h"

#include "CaloDetDescr/CaloDetDescrElement.h"

bool inRange(const double* boundaries, const double value, const double tolerance = 0.02) {
  return value >= boundaries[0] - tolerance && value <= boundaries[1] + tolerance;
}

StatusCode LArSCIdvsIdTest::initialize() {

  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_scCablingKey.initialize());
  ATH_CHECK(m_caloSuperCellMgrKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_onlineId, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_caloCellId, "CaloCell_ID"));
  ATH_CHECK(detStore()->retrieve(m_scOnlineId, "LArOnline_SuperCellID"));
  ATH_CHECK(detStore()->retrieve(m_scCaloCellId, "CaloCell_SuperCell_ID"));
  ATH_CHECK(m_scidtool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode LArSCIdvsIdTest::execute(const EventContext& ctx) const {

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
  if (!cablingHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> scCablingHdl(m_scCablingKey, ctx);
  if (!scCablingHdl.isValid()) {
    ATH_MSG_ERROR("Do not have Onl-Ofl cabling map for SuperCells !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<CaloSuperCellDetDescrManager> scDetMgr(m_caloSuperCellMgrKey, ctx);
  if (!scDetMgr.isValid()) {
    ATH_MSG_ERROR("Do not have CaloSuperCellDetDescrManager !!!!");
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<CaloDetDescrManager> detMgr(m_caloMgrKey, ctx);
  if (!detMgr.isValid()) {
    ATH_MSG_ERROR("Do not have CaloSuperCellDetDescrManager !!!!");
    return StatusCode::FAILURE;
  }

  std::bitset<190000> usedIds;
  for (const HWIdentifier scHWID : m_scOnlineId->channel_range()) {
    const Identifier scId = scCablingHdl->cnvToIdentifier(scHWID);

    const std::array<int, 3> scReg{m_scCaloCellId->sub_calo(scId), m_scCaloCellId->pos_neg(scId), m_scCaloCellId->calo_sample(scId)};
    const std::array<int, 3> scFTid{m_scOnlineId->barrel_ec(scHWID), m_scOnlineId->pos_neg(scHWID), m_scOnlineId->feedthrough(scHWID)};

    const CaloDetDescrElement* scDDE = scDetMgr->get_element(scId);
    const double scEtaRange[2] = {scDDE->eta_raw() - scDDE->deta() / 2.0, scDDE->eta_raw() + scDDE->deta() / 2.0};
    const double scPhiRange[2] = {scDDE->phi_raw() - scDDE->dphi() / 2.0, scDDE->phi_raw() + scDDE->dphi() / 2.0};
    const std::vector<Identifier>& regularIDs = m_scidtool->superCellToOfflineID(scId);
    std::cout << "SuperCell 0x" << std::hex << scId.get_identifier32().get_compact() << " (0x" << scHWID.get_identifier32().get_compact() << std::dec
              << "), eta=" << scDDE->eta_raw() << ", phi=" << scDDE->phi_raw() << ", sampling=" << m_scCaloCellId->calo_sample(scId) << std::endl;
    std::cout << "Online " << m_scOnlineId->channel_name(scHWID) << std::endl;
    for (const Identifier id : regularIDs) {
      const IdentifierHash hash = m_caloCellId->calo_cell_hash(id);
      if (usedIds.test(hash)) {
        ATH_MSG_ERROR("Identifier " << id.get_identifier32().get_compact() << "used in multiple SCs");
      }
      usedIds.set(hash);
      const HWIdentifier hwid = cablingHdl->createSignalChannelIDFromHash(hash);
      const CaloDetDescrElement* dde = detMgr->get_element(id);
      std::cout << "\tOnline=" << m_onlineId->channel_name(hwid) << std::endl;

      const int barrel_ec = m_onlineId->barrel_ec(hwid);
      const int pos_neg = m_onlineId->pos_neg(hwid);
      int feedthough = m_onlineId->feedthrough(hwid);

      // EMEC IW special-case cabling:
      if (m_scOnlineId->isEMECIW(scHWID)) {
        feedthough -= 1;
      }

      const std::array<int, 3> FTid{barrel_ec, pos_neg, feedthough};
      if (scFTid != FTid) {
        ATH_MSG_ERROR("Online FTID mismatch: SC-hwid=" << scFTid << ", hwid=" << FTid);
      }
      // int layer=m_layerMap[m_caloCellId->calo_sample(id)];
      int layer = m_caloCellId->calo_sample(id);

      // Special case: No HEC layers in SC everything labelled HEC0
      const std::set<CaloSampling::CaloSample> hecSamplings{CaloSampling::HEC1, CaloSampling::HEC2, CaloSampling::HEC3};
      if (hecSamplings.contains((CaloSampling::CaloSample)layer))
        layer = CaloSampling::HEC0;

      const std::array<int, 3> reg{m_caloCellId->sub_calo(id), m_caloCellId->pos_neg(id), layer};
      if (reg != scReg) {
        ATH_MSG_ERROR("Region id mismatch: SC reg " << scReg << ", reg=" << reg);
      }
      std::cout << "\tEta=" << dde->eta_raw() << " Phi=" << dde->phi_raw() << ", sampling=" << layer << std::endl;
      if (!inRange(scEtaRange, dde->eta_raw(), 0.1)) {
        ATH_MSG_ERROR("Cell eta=" << dde->eta_raw() << " outside of supercell eta boundaries [" << scEtaRange[0] << "," << scEtaRange[1] << "]");
      }
      if (!inRange(scPhiRange, dde->phi_raw())) {
        ATH_MSG_ERROR("Cell phi=" << dde->phi_raw() << " outside of supercell phi boundaries [" << scPhiRange[0] << "," << scPhiRange[1] << "]");
      }
    }  // end loop over regular cells connected to this SC
  }  // end loop over supercells

  unsigned nUnused = 0;
  for (unsigned i = 0; i < m_caloCellId->calo_cell_hash_max(); ++i) {
    if (!usedIds.test(i)) {
      const HWIdentifier hwid = cablingHdl->createSignalChannelIDFromHash(i);
      if (hwid.is_valid()) {
        ATH_MSG_DEBUG("Regular channel " << m_onlineId->channel_name(hwid) << " is not connected to any SuperCell.");
        nUnused++;
      }
    }
  }
  if (nUnused != 128) {
    ATH_MSG_ERROR("Expected 128 regular cells not connected to any SC, got " << nUnused);
  }

  return StatusCode::SUCCESS;
}