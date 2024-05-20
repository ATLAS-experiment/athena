/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Athena/Gaudi includes
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"


// local includes
#include "TrigT1NSWSimTools/MMTriggerTool.h"

// local includes
#include "TTree.h"

namespace NSWL1 {

    void MMTriggerTool::handle(const Incident& inc) {
      if( inc.type()==IncidentType::BeginEvent ) {
        ATH_MSG_DEBUG( "Handling..." );
        this->clear_ntuple_variables();
      }
    }

    StatusCode MMTriggerTool::book_branches() {
      m_trigger_diamond_ntrig       = new std::vector<unsigned int>();
      m_trigger_diamond_bc          = new std::vector<int>();
      m_trigger_diamond_sector      = new std::vector<char>();
      m_trigger_diamond_stationPhi  = new std::vector<int>();
      m_trigger_diamond_totalCount  = new std::vector<unsigned int>();
      m_trigger_diamond_realCount   = new std::vector<unsigned int>();
      m_trigger_diamond_XbkgCount   = new std::vector<unsigned int>();
      m_trigger_diamond_UVbkgCount  = new std::vector<unsigned int>();
      m_trigger_diamond_XmuonCount  = new std::vector<unsigned int>();
      m_trigger_diamond_UVmuonCount = new std::vector<unsigned int>();
      m_trigger_diamond_iX          = new std::vector<int>();
      m_trigger_diamond_iU          = new std::vector<int>();
      m_trigger_diamond_iV          = new std::vector<int>();
      m_trigger_diamond_age         = new std::vector<int>();
      m_trigger_diamond_mx          = new std::vector<double>();
      m_trigger_diamond_my          = new std::vector<double>();
      m_trigger_diamond_Uavg        = new std::vector<double>();
      m_trigger_diamond_Vavg        = new std::vector<double>();
      m_trigger_diamond_mxl         = new std::vector<double>();
      m_trigger_diamond_theta       = new std::vector<double>();
      m_trigger_diamond_eta         = new std::vector<double>();
      m_trigger_diamond_dtheta      = new std::vector<double>();
      m_trigger_diamond_phi         = new std::vector<double>();
      m_trigger_diamond_phiShf      = new std::vector<double>();
      m_trigger_diamond_TP_phi_id   = new std::vector<uint8_t>();
      m_trigger_diamond_TP_R_id     = new std::vector<uint8_t>();
      m_trigger_diamond_TP_dTheta_id = new std::vector<uint8_t>();

      m_trigger_RZslopes     = new std::vector<double>();
      m_trigger_trueEtaRange = new std::vector<double>();
      m_trigger_truePtRange  = new std::vector<double>();

      m_trigger_VMM          = new std::vector<int>();
      m_trigger_plane        = new std::vector<int>();
      m_trigger_station      = new std::vector<int>();
      m_trigger_strip        = new std::vector<int>();
      m_trigger_slope        = new std::vector<double>();
      m_trigger_trueThe      = new std::vector<double>();
      m_trigger_truePhi      = new std::vector<double>();
      m_trigger_trueDth      = new std::vector<double>();
      m_trigger_trueEtaEnt   = new std::vector<double>();
      m_trigger_trueTheEnt   = new std::vector<double>();
      m_trigger_truePhiEnt   = new std::vector<double>();
      m_trigger_trueEtaPos   = new std::vector<double>();
      m_trigger_trueThePos   = new std::vector<double>();
      m_trigger_truePhiPos   = new std::vector<double>();

      m_NSWMM_dig_stationName = new std::vector<std::string>();
      m_NSWMM_dig_stationEta  = new std::vector<int>();
      m_NSWMM_dig_stationPhi  = new std::vector<int>();
      m_NSWMM_dig_multiplet   = new std::vector<int>();
      m_NSWMM_dig_gas_gap     = new std::vector<int>();
      m_NSWMM_dig_channel     = new std::vector<int>();

      m_NSWMM_dig_time          = new std::vector< std::vector<float> >;
      m_NSWMM_dig_charge        = new std::vector< std::vector<float> >;
      m_NSWMM_dig_stripPosition = new std::vector< std::vector<int> >;
      m_NSWMM_dig_stripLposX    = new std::vector< std::vector<double> >;
      m_NSWMM_dig_stripLposY    = new std::vector< std::vector<double> >;
      m_NSWMM_dig_stripGposX    = new std::vector< std::vector<double> >;
      m_NSWMM_dig_stripGposY    = new std::vector< std::vector<double> >;
      m_NSWMM_dig_stripGposZ    = new std::vector< std::vector<double> >;

      if (m_tree) {
        m_tree->Branch("MM_diamond_bc",           &m_trigger_diamond_bc);
        m_tree->Branch("MM_diamond_ntrig",        &m_trigger_diamond_ntrig);
        m_tree->Branch("MM_diamond_sector",       &m_trigger_diamond_sector);
        m_tree->Branch("MM_diamond_stationPhi",   &m_trigger_diamond_stationPhi);
        m_tree->Branch("MM_diamond_totalCount",   &m_trigger_diamond_totalCount);
        m_tree->Branch("MM_diamond_realCount",    &m_trigger_diamond_realCount);
        m_tree->Branch("MM_diamond_XbkgCount",    &m_trigger_diamond_XbkgCount);
        m_tree->Branch("MM_diamond_UVbkgCount",   &m_trigger_diamond_UVbkgCount);
        m_tree->Branch("MM_diamond_XmuonCount",   &m_trigger_diamond_XmuonCount);
        m_tree->Branch("MM_diamond_UVmuonCount",  &m_trigger_diamond_UVmuonCount);
        m_tree->Branch("MM_diamond_iX",           &m_trigger_diamond_iX);
        m_tree->Branch("MM_diamond_iU",           &m_trigger_diamond_iU);
        m_tree->Branch("MM_diamond_iV",           &m_trigger_diamond_iV);
        m_tree->Branch("MM_diamond_age",          &m_trigger_diamond_age);
        m_tree->Branch("MM_diamond_mx",           &m_trigger_diamond_mx);
        m_tree->Branch("MM_diamond_my",           &m_trigger_diamond_my);
        m_tree->Branch("MM_diamond_Uavg",         &m_trigger_diamond_Uavg);
        m_tree->Branch("MM_diamond_Vavg",         &m_trigger_diamond_Vavg);
        m_tree->Branch("MM_diamond_mxl",          &m_trigger_diamond_mxl);
        m_tree->Branch("MM_diamond_theta",        &m_trigger_diamond_theta);
        m_tree->Branch("MM_diamond_eta",          &m_trigger_diamond_eta);
        m_tree->Branch("MM_diamond_dtheta",       &m_trigger_diamond_dtheta);
        m_tree->Branch("MM_diamond_phi",          &m_trigger_diamond_phi);
        m_tree->Branch("MM_diamond_phiShf",       &m_trigger_diamond_phiShf);
        m_tree->Branch("MM_diamond_TP_phi_id",    &m_trigger_diamond_TP_phi_id);
        m_tree->Branch("MM_diamond_TP_R_id",      &m_trigger_diamond_TP_R_id);
        m_tree->Branch("MM_diamond_TP_dTheta_id", &m_trigger_diamond_TP_dTheta_id);

        m_tree->Branch("MM_RZslopes",             &m_trigger_RZslopes);
        m_tree->Branch("MM_trueEtaRange",         &m_trigger_trueEtaRange);
        m_tree->Branch("MM_truePtRange",          &m_trigger_truePtRange);

        m_tree->Branch("MM_VMM",                  &m_trigger_VMM);
        m_tree->Branch("MM_plane",                &m_trigger_plane);
        m_tree->Branch("MM_station",              &m_trigger_station);
        m_tree->Branch("MM_strip",                &m_trigger_strip);
        m_tree->Branch("MM_slope",                &m_trigger_slope);
        m_tree->Branch("MM_trueThe",              &m_trigger_trueThe);
        m_tree->Branch("MM_truePhi",              &m_trigger_truePhi);
        m_tree->Branch("MM_trueDth",              &m_trigger_trueDth);
        m_tree->Branch("MM_trueEtaEnt",           &m_trigger_trueEtaEnt);
        m_tree->Branch("MM_trueTheEnt",           &m_trigger_trueTheEnt);
        m_tree->Branch("MM_truePhiEnt",           &m_trigger_truePhiEnt);
        m_tree->Branch("MM_trueEtaPos",           &m_trigger_trueEtaPos);
        m_tree->Branch("MM_trueThePos",           &m_trigger_trueThePos);
        m_tree->Branch("MM_truePhiPos",           &m_trigger_truePhiPos);

        m_tree->Branch("Digits_MM_stationName", &m_NSWMM_dig_stationName);
        m_tree->Branch("Digits_MM_stationEta",  &m_NSWMM_dig_stationEta);
        m_tree->Branch("Digits_MM_stationPhi",  &m_NSWMM_dig_stationPhi);
        m_tree->Branch("Digits_MM_multiplet",   &m_NSWMM_dig_multiplet);
        m_tree->Branch("Digits_MM_gas_gap",     &m_NSWMM_dig_gas_gap);
        m_tree->Branch("Digits_MM_channel",     &m_NSWMM_dig_channel);

        m_tree->Branch("Digits_MM_time",          &m_NSWMM_dig_time);
        m_tree->Branch("Digits_MM_charge",        &m_NSWMM_dig_charge);
        m_tree->Branch("Digits_MM_stripPosition", &m_NSWMM_dig_stripPosition);
        m_tree->Branch("Digits_MM_stripLposX",    &m_NSWMM_dig_stripLposX);
        m_tree->Branch("Digits_MM_stripLposY",    &m_NSWMM_dig_stripLposY);
        m_tree->Branch("Digits_MM_stripGposX",    &m_NSWMM_dig_stripGposX);
        m_tree->Branch("Digits_MM_stripGposY",    &m_NSWMM_dig_stripGposY);
        m_tree->Branch("Digits_MM_stripGposZ",    &m_NSWMM_dig_stripGposZ);
      } else {
        return StatusCode::FAILURE;
      }

      return StatusCode::SUCCESS;
    }

    void MMTriggerTool::clear_ntuple_variables() {
      //clear the ntuple variables
      if(m_tree==0) return;

      m_trigger_diamond_bc->clear();
      m_trigger_diamond_ntrig->clear();
      m_trigger_diamond_sector->clear();
      m_trigger_diamond_stationPhi->clear();
      m_trigger_diamond_totalCount->clear();
      m_trigger_diamond_realCount->clear();
      m_trigger_diamond_XbkgCount->clear();
      m_trigger_diamond_UVbkgCount->clear();
      m_trigger_diamond_XmuonCount->clear();
      m_trigger_diamond_UVmuonCount->clear();
      m_trigger_diamond_iX->clear();
      m_trigger_diamond_iU->clear();
      m_trigger_diamond_iV->clear();
      m_trigger_diamond_age->clear();
      m_trigger_diamond_mx->clear();
      m_trigger_diamond_my->clear();
      m_trigger_diamond_Uavg->clear();
      m_trigger_diamond_Vavg->clear();
      m_trigger_diamond_mxl->clear();
      m_trigger_diamond_theta->clear();
      m_trigger_diamond_eta->clear();
      m_trigger_diamond_dtheta->clear();
      m_trigger_diamond_phi->clear();
      m_trigger_diamond_phiShf->clear();
      m_trigger_diamond_TP_phi_id->clear();
      m_trigger_diamond_TP_R_id->clear();
      m_trigger_diamond_TP_dTheta_id->clear();

      m_trigger_RZslopes->clear();
      m_trigger_trueEtaRange->clear();
      m_trigger_truePtRange->clear();

      m_trigger_VMM->clear();
      m_trigger_plane->clear();
      m_trigger_station->clear();
      m_trigger_strip->clear();
      m_trigger_slope->clear();
      m_trigger_trueThe->clear();
      m_trigger_truePhi->clear();
      m_trigger_trueDth->clear();
      m_trigger_trueEtaEnt->clear();
      m_trigger_trueTheEnt->clear();
      m_trigger_truePhiEnt->clear();
      m_trigger_trueEtaPos->clear();
      m_trigger_trueThePos->clear();
      m_trigger_truePhiPos->clear();

      // information of the module down to the channel closest to the initial G4 hit
      // size of vector is m_NSWMM_nDigits
      m_NSWMM_dig_stationName->clear();
      m_NSWMM_dig_stationEta->clear();
      m_NSWMM_dig_stationPhi->clear();
      m_NSWMM_dig_multiplet->clear();
      m_NSWMM_dig_gas_gap->clear();
      m_NSWMM_dig_channel->clear();

      // vectors of size m_NSWMM_nDigits that hold vectors in which an entry
      // corresponds to a strip that was decided to be fired by the digit
      // (information from VMM chip response emulation)
      m_NSWMM_dig_time->clear();
      m_NSWMM_dig_charge->clear();
      m_NSWMM_dig_stripPosition->clear();
      m_NSWMM_dig_stripLposX->clear();
      m_NSWMM_dig_stripLposY->clear();
      m_NSWMM_dig_stripGposX->clear();
      m_NSWMM_dig_stripGposY->clear();
      m_NSWMM_dig_stripGposZ->clear();
    }
    void MMTriggerTool::fillNtuple(const histogramDigitVariables& histDigVars) const {

      *m_NSWMM_dig_stationName = histDigVars.NSWMM_dig_stationName;
      *m_NSWMM_dig_stationEta  = histDigVars.NSWMM_dig_stationEta;
      *m_NSWMM_dig_stationPhi  = histDigVars.NSWMM_dig_stationPhi;
      *m_NSWMM_dig_multiplet   = histDigVars.NSWMM_dig_multiplet;
      *m_NSWMM_dig_gas_gap     = histDigVars.NSWMM_dig_gas_gap;
      *m_NSWMM_dig_channel     = histDigVars.NSWMM_dig_channel;
      *m_NSWMM_dig_time          = histDigVars.NSWMM_dig_time;
      *m_NSWMM_dig_charge        = histDigVars.NSWMM_dig_charge;
      *m_NSWMM_dig_stripPosition = histDigVars.NSWMM_dig_stripPosition;
      *m_NSWMM_dig_stripLposX    = histDigVars.NSWMM_dig_stripLposX;
      *m_NSWMM_dig_stripLposY    = histDigVars.NSWMM_dig_stripLposY;
      *m_NSWMM_dig_stripGposX    = histDigVars.NSWMM_dig_stripGposX;
      *m_NSWMM_dig_stripGposY    = histDigVars.NSWMM_dig_stripGposY;
      *m_NSWMM_dig_stripGposZ    = histDigVars.NSWMM_dig_stripGposZ;
    }
}//end namespace
