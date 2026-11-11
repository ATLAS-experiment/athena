/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TileSimUtils/TileCalibHitCntNtup.h"

// Atlas includes
#include "GaudiKernel/MsgStream.h"
#include "Gaudi/Property.h"
#include "AthenaKernel/errorcheck.h"

// ROOT includes
#include "TTree.h"

static const InterfaceID IID_ITileCalibHitCntNtup("TileCalibHitCntNtup", 1, 0);

const InterfaceID&
TileCalibHitCntNtup::interfaceID() {
  return IID_ITileCalibHitCntNtup;
}

/**
 * Constructor
 */
TileCalibHitCntNtup::TileCalibHitCntNtup(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent)
{
  declareInterface<TileCalibHitCntNtup>(this);
}

/**
 * Destructor
 */
TileCalibHitCntNtup::~TileCalibHitCntNtup() {
}

/**
 * Initializer
 */
StatusCode TileCalibHitCntNtup::initialize() {

  ATH_MSG_INFO("in initialize()");

  // retrieve ID helpers from det store
  CHECK(detStore()->retrieve(m_caloCell_ID));

  CHECK(detStore()->retrieve(m_caloDM_ID));

  CHECK(m_thistSvc.retrieve());

  m_ntuplePtr = new TTree(m_ntupleID.value().c_str(), "TileCalibHitCntNtuple");
  m_ntuplePtr->SetMaxTreeSize(m_treeSize);

  // The fields characterizing the Event
  m_ntuplePtr->Branch("Tile_Nhit_Active", &m_act_nchan, "Tile_Nhit_Active/I");
  m_ntuplePtr->Branch("Tile_Nhit_Inact", &m_inact_nchan, "Tile_Nhit_Inact/I");
  m_ntuplePtr->Branch("Tile_Nhit_DM", &m_dm_nchan, "Tile_Nhit_DM/I");

  m_ntuplePtr->Branch("Tile_Active_Total_Ene", &m_act_totE, "Tile_Active_Total_Ene/F");
  m_ntuplePtr->Branch("Tile_Active_Vis_Ene", &m_act_visE, "Tile_Active_Vis_Ene/F");
  m_ntuplePtr->Branch("Tile_Inact_Total_Ene", &m_inact_totE, "Tile_Inact_Total_Ene/F");
  m_ntuplePtr->Branch("Tile_Inact_Vis_Ene", &m_inact_visE, "Tile_Inact_Vis_Ene/F");
  m_ntuplePtr->Branch("Tile_DM_Total_Ene", &m_DM_totE, "Tile_DM_Total_Ene/F");
  m_ntuplePtr->Branch("Tile_DM_Vis_Ene", &m_DM_visE, "Tile_DM_Vis_Ene/F");

  m_ntuplePtr->Branch("Barr_Active_Total_Ene", &m_barr_totE, "Barr_Active_Total_Ene/F");
  m_ntuplePtr->Branch("Barr_Active_Em_Ene", &m_barr_Em, "Barr_Active_Em_Ene/F");
  m_ntuplePtr->Branch("Barr_Active_NonEm_Ene", &m_barr_NonEm, "Barr_Active_NonEm_Ene/F");
  m_ntuplePtr->Branch("Barr_Active_Invisible_Ene", &m_barr_Invisible, "Barr_Active_Invisible_Ene/F");
  m_ntuplePtr->Branch("Barr_Active_Escaped_Ene", &m_barr_Escaped, "Barr_Active_Escaped_Ene/F");

  m_ntuplePtr->Branch("Barr_Inact_Total_Ene", &m_inact_barr_totE, "Barr_Inact_Total_Ene/F");
  m_ntuplePtr->Branch("Barr_Inact_Em_Ene", &m_inact_barr_Em, "Barr_Inact_Em_Ene/F");
  m_ntuplePtr->Branch("Barr_Inact_NonEm_Ene", &m_inact_barr_NonEm, "Barr_Inact_NonEm_Ene/F");
  m_ntuplePtr->Branch("Barr_Inact_Invisible_Ene", &m_inact_barr_Invisible, "Barr_Inact_Invisible_Ene/F");
  m_ntuplePtr->Branch("Barr_Inact_Escaped_Ene", &m_inact_barr_Escaped, "Barr_Inact_Escaped_Ene/F");

  m_ntuplePtr->Branch("Ext_Active_Total_Ene", &m_ext_totE, "Ext_Active_Total_Ene/F");
  m_ntuplePtr->Branch("Ext_Active_Em_Ene", &m_ext_Em, "Ext_Active_Em_Ene/F");
  m_ntuplePtr->Branch("Ext_Active_NonEm_Ene", &m_ext_NonEm, "Ext_Active_NonEm_Ene/F");
  m_ntuplePtr->Branch("Ext_Active_Invisible_Ene", &m_ext_Invisible, "Ext_Active_Invisible_Ene/F");
  m_ntuplePtr->Branch("Ext_Active_Escaped_Ene", &m_ext_Escaped, "Ext_Active_Escaped_Ene/F");

  m_ntuplePtr->Branch("Ext_Inact_Total_Ene", &m_inact_ext_totE, "Ext_Inact_Total_Ene/F");
  m_ntuplePtr->Branch("Ext_Inact_Em_Ene", &m_inact_ext_Em, "Ext_Inact_Em_Ene/F");
  m_ntuplePtr->Branch("Ext_Inact_NonEm_Ene", &m_inact_ext_NonEm, "Ext_Inact_NonEm_Ene/F");
  m_ntuplePtr->Branch("Ext_Inact_Invisible_Ene", &m_inact_ext_Invisible, "Ext_Inact_Invisible_Ene/F");
  m_ntuplePtr->Branch("Ext_Inact_Escaped_Ene", &m_inact_ext_Escaped, "Ext_Inact_Escaped_Ene/F");

  m_ntuplePtr->Branch("ITC_Active_Total_Ene", &m_itc_totE, "ITC_Active_Total_Ene/F");
  m_ntuplePtr->Branch("ITC_Active_Em_Ene", &m_itc_Em, "ITC_Active_Em_Ene/F");
  m_ntuplePtr->Branch("ITC_Active_NonEm_Ene", &m_itc_NonEm, "ITC_Active_NonEm_Ene/F");
  m_ntuplePtr->Branch("ITC_Active_Invisible_Ene", &m_itc_Invisible, "ITC_Active_Invisible_Ene/F");
  m_ntuplePtr->Branch("ITC_Active_Escaped_Ene", &m_itc_Escaped, "ITC_Active_Escaped_Ene/F");

  m_ntuplePtr->Branch("ITC_Inact_Total_Ene", &m_inact_itc_totE, "ITC_Inact_Total_Ene/F");
  m_ntuplePtr->Branch("ITC_Inact_Em_Ene", &m_inact_itc_Em, "ITC_Inact_Em_Ene/F");
  m_ntuplePtr->Branch("ITC_Inact_NonEm_Ene", &m_inact_itc_NonEm, "ITC_Inact_NonEm_Ene/F");
  m_ntuplePtr->Branch("ITC_Inact_Invisible_Ene", &m_inact_itc_Invisible, "ITC_Inact_Invisible_Ene/F");
  m_ntuplePtr->Branch("ITC_Inact_Escaped_Ene", &m_inact_itc_Escaped, "ITC_Inact_Escaped_Ene/F");

  m_ntuplePtr->Branch("GapScin_Active_Total_Ene", &m_gscin_totE, "GapScin_Active_Total_Ene/F");
  m_ntuplePtr->Branch("GapScin_Active_Em_Ene", &m_gscin_Em, "GapScin_Active_Em_Ene/F");
  m_ntuplePtr->Branch("GapScin_Active_NonEm_Ene", &m_gscin_NonEm, "GapScin_Active_NonEm_Ene/F");
  m_ntuplePtr->Branch("GapScin_Active_Invisible_Ene", &m_gscin_Invisible, "GapScin_Active_Invisible_Ene/F");
  m_ntuplePtr->Branch("GapScin_Active_Escaped_Ene", &m_gscin_Escaped, "GapScin_Active_Escaped_Ene/F");

  m_ntuplePtr->Branch("GapScin_Inact_Total_Ene", &m_inact_gscin_totE, "GapScin_Inact_Total_Ene/F");
  m_ntuplePtr->Branch("GapScin_Inact_Em_Ene", &m_inact_gscin_Em, "GapScin_Inact_Em_Ene/F");
  m_ntuplePtr->Branch("GapScin_Inact_NonEm_Ene", &m_inact_gscin_NonEm, "GapScin_Inact_NonEm_Ene/F");
  m_ntuplePtr->Branch("GapScin_Inact_Invisible_Ene", &m_inact_gscin_Invisible, "GapScin_Inact_Invisible_Ene/F");
  m_ntuplePtr->Branch("GapScin_Inact_Escaped_Ene", &m_inact_gscin_Escaped, "GapScin_Inact_Escaped_Ene/F");

  // The fields characterizing the Hit
  m_ntuplePtr->Branch("Tile_Active_Total_Ene", &m_Total);
  m_ntuplePtr->Branch("Tile_Active_Em_Ene", &m_Em);
  m_ntuplePtr->Branch("Tile_Active_NonEm_Ene", &m_NonEm);
  m_ntuplePtr->Branch("Tile_Active_Invisible_Ene", &m_Invisible);
  m_ntuplePtr->Branch("Tile_Active_Escaped_Ene", &m_Escaped);
  m_ntuplePtr->Branch("Tile_Active_subCalo", &m_subCalo);
  m_ntuplePtr->Branch("Tile_Active_section", &m_section);
  m_ntuplePtr->Branch("Tile_Active_side", &m_side);
  m_ntuplePtr->Branch("Tile_Active_module", &m_module);
  m_ntuplePtr->Branch("Tile_Active_tower", &m_tower);
  m_ntuplePtr->Branch("Tile_Active_sample", &m_sample);

  m_ntuplePtr->Branch("Tile_Inact_Total_Ene", &m_inact_Total);
  m_ntuplePtr->Branch("Tile_Inact_Em_Ene", &m_inact_Em);
  m_ntuplePtr->Branch("Tile_Inact_NonEm_Ene", &m_inact_NonEm);
  m_ntuplePtr->Branch("Tile_Inact_Invisible_Ene", &m_inact_Invisible);
  m_ntuplePtr->Branch("Tile_Inact_Escaped_Ene", &m_inact_Escaped);
  m_ntuplePtr->Branch("Tile_Inact_subCalo", &m_inact_subCalo);
  m_ntuplePtr->Branch("Tile_Inact_section", &m_inact_section);
  m_ntuplePtr->Branch("Tile_Inact_side", &m_inact_side);
  m_ntuplePtr->Branch("Tile_Inact_module", &m_inact_module);
  m_ntuplePtr->Branch("Tile_Inact_tower", &m_inact_tower);
  m_ntuplePtr->Branch("Tile_Inact_sample", &m_inact_sample);

  m_ntuplePtr->Branch("Tile_DM_Ene", &m_dm_ene);
  m_ntuplePtr->Branch("Tile_DM_Vis", &m_dm_vis);
  m_ntuplePtr->Branch("Tile_DM_subDetector", &m_dm_subDet);
  m_ntuplePtr->Branch("Tile_DM_type", &m_dm_type);
  m_ntuplePtr->Branch("Tile_DM_sampling", &m_dm_sampling);
  m_ntuplePtr->Branch("Tile_DM_region", &m_dm_region);
  m_ntuplePtr->Branch("Tile_DM_eta", &m_dm_eta);
  m_ntuplePtr->Branch("Tile_DM_phi", &m_dm_phi);

  CHECK(m_thistSvc->regTree("/" + m_streamName.value() + "/" + m_ntupleID.value(), m_ntuplePtr));

  return StatusCode::SUCCESS;
}

/**
 * Executer
 */
StatusCode TileCalibHitCntNtup::StoreCNT(const CaloCalibrationHitContainer* ActiveHitCnt,
                                         const CaloCalibrationHitContainer* InactiveHitCnt,
                                         const CaloCalibrationHitContainer* dmHitCnt) {

  m_act_nchan = m_inact_nchan = m_dm_nchan = 0;

  m_act_totE = m_act_visE = m_inact_totE = m_inact_visE = m_DM_totE = m_DM_visE = 0.;

  m_barr_totE = 0.;
  m_ext_totE = 0.;
  m_itc_totE = 0.;
  m_gscin_totE = 0.;
  m_barr_Em = 0.;
  m_ext_Em = 0.;
  m_itc_Em = 0.;
  m_gscin_Em = 0.;
  m_barr_NonEm = 0.;
  m_ext_NonEm = 0.;
  m_itc_NonEm = 0.;
  m_gscin_NonEm = 0.;
  m_barr_Invisible = 0.;
  m_ext_Invisible = 0.;
  m_itc_Invisible = 0.;
  m_gscin_Invisible = 0.;
  m_barr_Escaped = 0.;
  m_ext_Escaped = 0.;
  m_itc_Escaped = 0.;
  m_gscin_Escaped = 0.;

  m_inact_barr_totE = 0.;
  m_inact_ext_totE = 0.;
  m_inact_itc_totE = 0.;
  m_inact_gscin_totE = 0.;
  m_inact_barr_Em = 0.;
  m_inact_ext_Em = 0.;
  m_inact_itc_Em = 0.;
  m_inact_gscin_Em = 0.;
  m_inact_barr_NonEm = 0.;
  m_inact_ext_NonEm = 0.;
  m_inact_itc_NonEm = 0.;
  m_inact_gscin_NonEm = 0.;
  m_inact_barr_Invisible = 0.;
  m_inact_ext_Invisible = 0.;
  m_inact_itc_Invisible = 0.;
  m_inact_gscin_Invisible = 0.;
  m_inact_barr_Escaped = 0.;
  m_inact_ext_Escaped = 0.;
  m_inact_itc_Escaped = 0.;
  m_inact_gscin_Escaped = 0.;

  m_Total->clear();
  m_Em->clear();
  m_NonEm->clear();
  m_Invisible->clear();
  m_Escaped->clear();

  m_subCalo->clear();
  m_section->clear();
  m_side->clear();
  m_module->clear();
  m_tower->clear();
  m_sample->clear();

  m_inact_Total->clear();
  m_inact_Em->clear();
  m_inact_NonEm->clear();
  m_inact_Invisible->clear();
  m_inact_Escaped->clear();

  m_inact_subCalo->clear();
  m_inact_section->clear();
  m_inact_side->clear();
  m_inact_module->clear();
  m_inact_tower->clear();
  m_inact_sample->clear();

  m_dm_ene->clear();
  m_dm_vis->clear();
  m_dm_subDet->clear();
  m_dm_type->clear();
  m_dm_sampling->clear();
  m_dm_region->clear();
  m_dm_eta->clear();
  m_dm_phi->clear();

  //------------ Go inside the CalibHit containers ------------
  int nhit;
  char text_info[100];
  if (ActiveHitCnt) {
    nhit = ActiveHitCnt->Size();
    sprintf(text_info, "Number of CalibHits in Tile ActiveCell container   = %d", nhit);
    ATH_MSG_INFO(text_info);

    if (nhit != 0) {
      for (const CaloCalibrationHit* hit : *ActiveHitCnt) {
        m_act_totE += hit->energyTotal();
        m_act_visE += hit->energyEM() + hit->energyNonEM();

        m_Total->push_back( hit->energyTotal());
        m_Em->push_back( hit->energyEM());
        m_NonEm->push_back( hit->energyNonEM());
        m_Invisible->push_back( hit->energyInvisible());
        m_Escaped->push_back( hit->energyEscaped());

        Identifier id = hit->cellID();

        m_subCalo->push_back(m_caloCell_ID->sub_calo(id));
        m_section->push_back(m_caloCell_ID->section(id));
        m_side->push_back(m_caloCell_ID->side(id));
        m_module->push_back(m_caloCell_ID->module(id));
        m_tower->push_back(m_caloCell_ID->tower(id));
        m_sample->push_back(m_caloCell_ID->sample(id));

        if (m_caloCell_ID->is_tile_barrel(id)) {
          m_barr_totE += hit->energyTotal();
          m_barr_Em += hit->energyEM();
          m_barr_NonEm += hit->energyNonEM();
          m_barr_Invisible += hit->energyInvisible();
          m_barr_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_extbarrel(id)) {
          m_ext_totE += hit->energyTotal();
          m_ext_Em += hit->energyEM();
          m_ext_NonEm += hit->energyNonEM();
          m_ext_Invisible += hit->energyInvisible();
          m_ext_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_gapscin(id)) {
          m_gscin_totE += hit->energyTotal();
          m_gscin_Em += hit->energyEM();
          m_gscin_NonEm += hit->energyNonEM();
          m_gscin_Invisible += hit->energyInvisible();
          m_gscin_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_gap(id)) {
          m_itc_totE += hit->energyTotal();
          m_itc_Em += hit->energyEM();
          m_itc_NonEm += hit->energyNonEM();
          m_itc_Invisible += hit->energyInvisible();
          m_itc_Escaped += hit->energyEscaped();
        } else {
          ATH_MSG_WARNING("CalibHit in TileCalibHitActiveCell container hasn't Tile Identifier");
        }
        ++m_act_nchan;
        if (m_act_nchan >= m_max_chan) break;
      } // loop over container's hit
    }
  } else {
    ATH_MSG_WARNING("TileCalibHitActiveCell not exists for this event");
  }

  if (InactiveHitCnt) {
    nhit = InactiveHitCnt->Size();
    sprintf(text_info, "Number of CalibHits in Tile InactiveCell container = %d", nhit);
    ATH_MSG_INFO(text_info);

    if (nhit != 0) {
      for (const CaloCalibrationHit* hit : *InactiveHitCnt) {
        m_inact_totE += hit->energyTotal();
        m_inact_visE += hit->energyEM() + hit->energyNonEM();

        m_inact_Total->push_back( hit->energyTotal());
        m_inact_Em->push_back( hit->energyEM());
        m_inact_NonEm->push_back( hit->energyNonEM());
        m_inact_Invisible->push_back( hit->energyInvisible());
        m_inact_Escaped->push_back( hit->energyEscaped());

        Identifier id = hit->cellID();

        m_inact_subCalo->push_back(m_caloCell_ID->sub_calo(id));
        m_inact_section->push_back(m_caloCell_ID->section(id));
        m_inact_side->push_back(m_caloCell_ID->side(id));
        m_inact_module->push_back(m_caloCell_ID->module(id));
        m_inact_tower->push_back(m_caloCell_ID->tower(id));
        m_inact_sample->push_back(m_caloCell_ID->sample(id));

        if (m_caloCell_ID->is_tile_barrel(id)) {
          m_inact_barr_totE += hit->energyTotal();
          m_inact_barr_Em += hit->energyEM();
          m_inact_barr_NonEm += hit->energyNonEM();
          m_inact_barr_Invisible += hit->energyInvisible();
          m_inact_barr_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_extbarrel(id)) {
          m_inact_ext_totE += hit->energyTotal();
          m_inact_ext_Em += hit->energyEM();
          m_inact_ext_NonEm += hit->energyNonEM();
          m_inact_ext_Invisible += hit->energyInvisible();
          m_inact_ext_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_gapscin(id)) {
          m_inact_gscin_totE += hit->energyTotal();
          m_inact_gscin_Em += hit->energyEM();
          m_inact_gscin_NonEm += hit->energyNonEM();
          m_inact_gscin_Invisible += hit->energyInvisible();
          m_inact_gscin_Escaped += hit->energyEscaped();
        } else if (m_caloCell_ID->is_tile_gap(id)) {
          m_inact_itc_totE += hit->energyTotal();
          m_inact_itc_Em += hit->energyEM();
          m_inact_itc_NonEm += hit->energyNonEM();
          m_inact_itc_Invisible += hit->energyInvisible();
          m_inact_itc_Escaped += hit->energyEscaped();
        } else {
          ATH_MSG_WARNING("CalibHit in TileCalibHitInactiveCell container hasn't Tile Identifier");
        }
        ++m_inact_nchan;
        if (m_inact_nchan >= m_max_chan) break;
      } // loop over container's hit
    }
  } else {
    ATH_MSG_WARNING("TileCalibHitInactiveCell not exists for this event");
  }

  if (dmHitCnt) {
    nhit = dmHitCnt->Size();
    sprintf(text_info, "Number of CalibHits in Tile DeadMaterial container = %d", nhit);
    ATH_MSG_INFO(text_info);

    if (nhit != 0) {
      for (const CaloCalibrationHit* hit : *dmHitCnt) {
        m_DM_totE += hit->energyTotal();
        m_DM_visE += hit->energyEM() + hit->energyNonEM();

        Identifier id = hit->cellID();

        m_dm_ene->push_back( hit->energyTotal());
        m_dm_vis->push_back( hit->energyEM() + hit->energyNonEM());
        m_dm_subDet->push_back(m_caloDM_ID->pos_neg_z(id));
        m_dm_type->push_back(m_caloDM_ID->dmat(id));
        m_dm_sampling->push_back(m_caloDM_ID->sampling(id));
        m_dm_region->push_back(m_caloDM_ID->region(id));
        m_dm_eta->push_back(m_caloDM_ID->eta(id));
        m_dm_phi->push_back(m_caloDM_ID->phi(id));

        ++m_dm_nchan;
        if (m_dm_nchan >= m_max_chan) break;
      }
    }
  } else {
    ATH_MSG_WARNING("TileCalibHitDeadMaterial not exists for this event");
  }

  if (m_ntuplePtr->Fill()==0) {
    ATH_MSG_ERROR("StoreCNT(): Unable to Fill Cnt Ntuple");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("StoreCNT() completed successfully");
    return StatusCode::SUCCESS;
  }
}

/**
 * Finalizer
 */
StatusCode TileCalibHitCntNtup::finalize() {
  ATH_MSG_INFO("Finalizing");
  return StatusCode::SUCCESS;
}

