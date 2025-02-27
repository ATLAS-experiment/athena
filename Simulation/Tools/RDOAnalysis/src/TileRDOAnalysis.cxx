/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TileRDOAnalysis.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/HWIdentifier.h"
#include "StoreGate/ReadHandle.h"

#include "TileEvent/TileDigits.h"
#include "TileEvent/TileMuonReceiverObj.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelCollection.h"
#include "TileEvent/TileDigitsCollection.h"

#include "TTree.h"
#include "TString.h"
#include "TileEvent/TileTTL1.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>

TileRDOAnalysis::TileRDOAnalysis(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
  , m_adcID(0)
  , m_pmtID(0)
  , m_cellID(0)
  , m_ttID(0)
  , m_mtID(0)
  , m_fragID(0)
  , m_rawAmp(0)
  , m_rawTime(0)
  , m_rawQual(0)
  , m_rawPed(0)
  , m_adcHWID_mu(0)
  , m_fragID_mu(0)
  , m_rawAmp_mu(0)
  , m_rawTime_mu(0)
  , m_rawQual_mu(0)
  , m_rawPed_mu(0)
  , m_muRcvID(0)
  , m_muRcv_dec(0)
  , m_muRcv_thresh(0)
  , m_muRcv_energy(0)
  , m_muRcv_time(0)
  , m_ttl1MBTS_ID(0)
  , m_ttl1MBTS_digits(0)
  , m_ttl1_ID(0)
  , m_ttl1_digits(0)
  , m_L2ID(0)
  , m_L2val(0)
  , m_L2eta(0)
  , m_L2phi(0)
  , m_L2energyA(0)
  , m_L2energyBC(0)
  , m_L2energyD(0)
  , m_L2qual(0)
  , m_L2sumE(0)
  , m_fragSize(0)
  , m_fragBCID(0)
  , m_digits(0)
  , m_muFragSize(0)
  , m_muFragBCID(0)
  , m_muDigits(0)

  , m_h_adcID(0)
  , m_h_rawAmp(0)
  , m_h_rawTime(0)
  , m_h_rawQual(0)
  , m_h_rawPed(0)
  , m_h_adcHWID_mu(0)
  , m_h_rawAmp_mu(0)
  , m_h_rawTime_mu(0)
  , m_h_rawQual_mu(0)
  , m_h_rawPed_mu(0)
  , m_h_muRcvID(0)
  , m_h_muRcv_dec(0)
  , m_h_muRcv_thresh(0)
  , m_h_muRcv_energy(0)
  , m_h_muRcv_time(0)
  , m_h_ttl1MBTS_ID(0)
  , m_h_ttl1MBTS_digits(0)
  , m_h_ttl1_ID(0)
  , m_h_ttl1_digits(0)
  , m_h_L2ID(0)
  , m_h_L2val(0)
  , m_h_L2eta(0)
  , m_h_L2phi(0)
  , m_h_L2energyA(0)
  , m_h_L2energyBC(0)
  , m_h_L2energyD(0)
  , m_h_L2qual(0)
  , m_h_L2sumE(0)
  , m_h_digits(0)
  , m_h_muDigits(0)

  , m_tree(0)
  , m_ntupleFileName("/ntuples/file1")
  , m_ntupleDirName("/TileRDOAnalysis/")
  , m_ntupleTreeName("TileRDOAna")
  , m_path("/TileRDOAnalysis/")
  , m_thistSvc("THistSvc", name)
{
  declareProperty("NtupleFileName", m_ntupleFileName);
  declareProperty("NtupleDirectoryName", m_ntupleDirName);
  declareProperty("NtupleTreeName", m_ntupleTreeName);
  declareProperty("HistPath", m_path);
}

StatusCode TileRDOAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing TileRDOAnalysis" );


  // This will check that the properties were initialized
  // properly by job configuration.
  ATH_CHECK( m_inputRawChKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputMuRcvRawChKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputMuRcvKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputMBTS_TTL1Key.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputTileTTL1Key.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputL2Key.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_inputDigitsFltKey.initialize() );
  ATH_CHECK( m_inputDigitsMuRcvKey.initialize() );

  ATH_CHECK(m_cablingSvc.retrieve());

  // Grab Ntuple and histogramming service for tree
  ATH_CHECK(m_thistSvc.retrieve());

  m_tree = new TTree(TString(m_ntupleTreeName), "TileRDOAna");
  std::string fullNtupleName = "/" + m_ntupleFileName + "/" + m_ntupleDirName + "/" + m_ntupleTreeName;
  ATH_CHECK(m_thistSvc->regTree(fullNtupleName, m_tree));
  if (m_tree) {
    m_tree->Branch("adcID", &m_adcID);
    m_tree->Branch("pmtID", &m_pmtID);
    m_tree->Branch("cellID", &m_cellID);
    m_tree->Branch("ttID", &m_ttID);
    m_tree->Branch("mtID", &m_mtID);
    m_tree->Branch("fragID", &m_fragID);
    m_tree->Branch("rawAmp", &m_rawAmp);
    m_tree->Branch("rawTime", &m_rawTime);
    m_tree->Branch("rawQual", &m_rawQual);
    m_tree->Branch("rawPed", &m_rawPed);
    m_tree->Branch("adcHWID_mu", &m_adcHWID_mu);
    m_tree->Branch("fragID_mu", &m_fragID_mu);
    m_tree->Branch("rawAmp_mu", &m_rawAmp_mu);
    m_tree->Branch("rawTime_mu", &m_rawTime_mu);
    m_tree->Branch("rawQual_mu", &m_rawQual_mu);
    m_tree->Branch("rawPed_mu", &m_rawPed_mu);
    m_tree->Branch("muRcvID", &m_muRcvID);
    m_tree->Branch("muRcv_dec", &m_muRcv_dec);
    m_tree->Branch("muRcv_thresh", &m_muRcv_thresh);
    m_tree->Branch("muRcv_energy", &m_muRcv_energy);
    m_tree->Branch("muRcv_time", &m_muRcv_time);
    m_tree->Branch("ttl1MBTS_ID", &m_ttl1MBTS_ID);
    m_tree->Branch("ttl1MBTS_digits", &m_ttl1MBTS_digits);
    m_tree->Branch("ttl1_ID", &m_ttl1_ID);
    m_tree->Branch("ttl1_digits", &m_ttl1_digits);
    m_tree->Branch("L2ID", &m_L2ID);
    m_tree->Branch("L2val", &m_L2val);
    m_tree->Branch("L2eta", &m_L2eta);
    m_tree->Branch("L2phi", &m_L2phi);
    m_tree->Branch("L2energyA", &m_L2energyA);
    m_tree->Branch("L2energyBC", &m_L2energyBC);
    m_tree->Branch("L2energyD", &m_L2energyD);
    m_tree->Branch("L2qual", &m_L2qual);
    m_tree->Branch("L2sumE", &m_L2sumE);
    m_tree->Branch("fragSize", &m_fragSize);
    m_tree->Branch("fragBCID", &m_fragBCID);
    m_tree->Branch("digits", &m_digits);
    m_tree->Branch("muFragSize", &m_muFragSize);
    m_tree->Branch("muFragBCID", &m_muFragBCID);
    m_tree->Branch("muDigits", &m_muDigits);
  }
  else {
    ATH_MSG_ERROR("No tree found!");
  }

  // HISTOGRAMS
  m_h_adcID = new TH1F("h_adcID", "adc ID", 100, 0, 9.25e18);
  m_h_adcID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_adcID->GetName(), m_h_adcID));

  m_h_rawAmp = new TH1F("h_rawAmp", "Raw amplitude", 100, -1200, 1200);
  m_h_rawAmp->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawAmp->GetName(), m_h_rawAmp));

  m_h_rawTime = new TH1F("h_rawTime", "Raw time", 100, -90, 90);
  m_h_rawTime->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawTime->GetName(), m_h_rawTime));

  m_h_rawQual = new TH1F("h_rawQual", "Raw quality", 100, 0, 1100);
  m_h_rawQual->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawQual->GetName(), m_h_rawQual));

  m_h_rawPed = new TH1F("h_rawPed", "Raw pedestal", 100, 0, 2e5);
  m_h_rawPed->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawPed->GetName(), m_h_rawPed));

  m_h_adcHWID_mu = new TH1F("h_adcHWID_mu", "MuRcv adc HW ID", 100, 0, 9.25e18);
  m_h_adcHWID_mu->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_adcHWID_mu->GetName(), m_h_adcHWID_mu));

  m_h_rawAmp_mu = new TH1F("h_rawAmp_mu", "MuRcv raw amplitude", 100, -1000, 11000);
  m_h_rawAmp_mu->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawAmp_mu->GetName(), m_h_rawAmp_mu));

  m_h_rawTime_mu = new TH1F("h_rawTime_mu", "MuRcv raw time", 100, -90, 90);
  m_h_rawTime_mu->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawTime_mu->GetName(), m_h_rawTime_mu));

  m_h_rawQual_mu = new TH1F("h_rawQual_mu", "MuRcv raw quality", 100, 0, 8e34);
  m_h_rawQual_mu->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawQual_mu->GetName(), m_h_rawQual_mu));

  m_h_rawPed_mu = new TH1F("h_rawPed_mu", "MuRcv raw pedestal", 100, 0, 13);
  m_h_rawPed_mu->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_rawPed_mu->GetName(), m_h_rawPed_mu));

  m_h_muRcvID = new TH1F("h_muRcvID", "Muon receiver object ID", 100, 0, 500);
  m_h_muRcvID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muRcvID->GetName(), m_h_muRcvID));

  m_h_muRcv_dec = new TH1F("h_muRcv_dec", "Muon receiver object decision", 100, 0, 2);
  m_h_muRcv_dec->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muRcv_dec->GetName(), m_h_muRcv_dec));

  m_h_muRcv_thresh = new TH1F("h_muRcv_thresh", "Muon receiver object threshold", 100, 0, 650);
  m_h_muRcv_thresh->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muRcv_thresh->GetName(), m_h_muRcv_thresh));

  m_h_muRcv_energy = new TH1F("h_muRcv_energy", "Muon receiver object energy", 100, 0, 20000);
  m_h_muRcv_energy->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muRcv_energy->GetName(), m_h_muRcv_energy));

  m_h_muRcv_time = new TH1F("h_muRcv_time", "Muon receiver object time", 100, -90, 90);
  m_h_muRcv_time->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muRcv_time->GetName(), m_h_muRcv_time));

  m_h_ttl1MBTS_ID = new TH1F("h_ttl1MBTS_ID", "TTL1 MBTS ID", 100, 0, 9.25e18);
  m_h_ttl1MBTS_ID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_ttl1MBTS_ID->GetName(), m_h_ttl1MBTS_ID));

  m_h_ttl1MBTS_digits = new TH1F("h_ttl1MBTS_digits", "TTL1 MBTS digits", 100, 0, 2000);
  m_h_ttl1MBTS_digits->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_ttl1MBTS_digits->GetName(), m_h_ttl1MBTS_digits));

  m_h_ttl1_ID = new TH1F("h_ttl1_ID", "TTL1 ID", 100, 0, 2e19);
  m_h_ttl1_ID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_ttl1_ID->GetName(), m_h_ttl1_ID));

  m_h_ttl1_digits = new TH1F("h_ttl1_digits", "TTL1 digits", 100, 0, 2000);
  m_h_ttl1_digits->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_ttl1_digits->GetName(), m_h_ttl1_digits));

  m_h_L2ID = new TH1F("h_L2ID", "L2 ID", 100, 0, 2e19);
  m_h_L2ID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2ID->GetName(), m_h_L2ID));

  m_h_L2val = new TH1F("h_L2val", "L2 data values", 100, 0, 100);
  m_h_L2val->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2val->GetName(), m_h_L2val));

  m_h_L2eta = new TH1F("h_L2eta", "L2 eta", 100, -1.5, 1.5);
  m_h_L2eta->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2eta->GetName(), m_h_L2eta));

  m_h_L2phi = new TH1F("h_L2phi", "L2 phi", 100, -3.5, 3.5);
  m_h_L2phi->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2phi->GetName(), m_h_L2phi));

  m_h_L2energyA = new TH1F("h_L2energyA", "L2 energy in A cells", 100, 0, 12500);
  m_h_L2energyA->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2energyA->GetName(), m_h_L2energyA));

  m_h_L2energyBC = new TH1F("h_L2energyBC", "L2 energy in BC cells", 100, 0, 12500);
  m_h_L2energyBC->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2energyBC->GetName(), m_h_L2energyBC));

  m_h_L2energyD = new TH1F("h_L2energyD", "L2 energy in D cells", 100, 0, 12500);
  m_h_L2energyD->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2energyD->GetName(), m_h_L2energyD));

  m_h_L2qual = new TH1F("h_L2qual", "L2 quality", 100, 0, 2);
  m_h_L2qual->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2qual->GetName(), m_h_L2qual));

  m_h_L2sumE = new TH1F("h_L2sumE", "L2 energy sum", 100, 0, 2.25e5);
  m_h_L2sumE->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_L2sumE->GetName(), m_h_L2sumE));

  m_h_digits = new TH1F("h_digits", "Tile digits", 100, 0, 1100);
  m_h_digits->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_digits->GetName(), m_h_digits));

  m_h_muDigits = new TH1F("h_muDigits", "Tile muon receiver object digits", 100, 0, 150);
  m_h_muDigits->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + m_h_muDigits->GetName(), m_h_muDigits));



  return StatusCode::SUCCESS;
}

StatusCode TileRDOAnalysis::execute() {
  ATH_MSG_DEBUG( "In TileRDOAnalysis::execute()" );

  m_adcID->clear();
  m_pmtID->clear();
  m_cellID->clear();
  m_ttID->clear();
  m_mtID->clear();
  m_fragID->clear();
  m_rawAmp->clear();
  m_rawTime->clear();
  m_rawQual->clear();
  m_rawPed->clear();
  m_adcHWID_mu->clear();
  m_fragID_mu->clear();
  m_rawAmp_mu->clear();
  m_rawTime_mu->clear();
  m_rawQual_mu->clear();
  m_rawPed_mu->clear();
  m_muRcvID->clear();
  m_muRcv_dec->clear();
  m_muRcv_thresh->clear();
  m_muRcv_energy->clear();
  m_muRcv_time->clear();
  m_ttl1MBTS_ID->clear();
  m_ttl1MBTS_digits->clear();
  m_ttl1_ID->clear();
  m_ttl1_digits->clear();
  m_L2ID->clear();
  m_L2val->clear();
  m_L2eta->clear();
  m_L2phi->clear();
  m_L2energyA->clear();
  m_L2energyBC->clear();
  m_L2energyD->clear();
  m_L2qual->clear();
  m_L2sumE->clear();
  m_fragSize->clear();
  m_fragBCID->clear();
  m_digits->clear();
  m_muFragSize->clear();
  m_muFragBCID->clear();
  m_muDigits->clear();

  // Tile Raw Channels
  // Raw info (pulse height, time, quality) for in-time beam crossing in Tile
  if (!m_presampling) {

      if (!m_inputRawChKey.empty()) {
        SG::ReadHandle<TileRawChannelContainer> rawChannelContainer(m_inputRawChKey);
        ATH_CHECK(rawChannelContainer.isValid());
        // loop over tile raw channels container

        for (const TileRawChannelCollection* rawChannelCollection : *rawChannelContainer) {
          for (const TileRawChannel* rawChannel : *rawChannelCollection) {

            const Identifier adcID(rawChannel->adc_ID());
            if (!adcID.is_valid()) continue;

            const Identifier pmtID(rawChannel->pmt_ID());
            const Identifier cellID(rawChannel->cell_ID());
            const Identifier ttID(rawChannel->tt_ID());
            const Identifier mtID(rawChannel->mt_ID());
            const int fragID(rawChannel->frag_ID());

            const unsigned long long adcID_int = adcID.get_compact();
            const unsigned long long pmtID_int = pmtID.get_compact();
            const unsigned long long cellID_int = cellID.get_compact();
            const unsigned long long ttID_int = ttID.get_compact();
            const unsigned long long mtID_int = mtID.get_compact();

            m_adcID->push_back(adcID_int);
            m_pmtID->push_back(pmtID_int);
            m_cellID->push_back(cellID_int);
            m_ttID->push_back(ttID_int);
            m_mtID->push_back(mtID_int);
            m_fragID->push_back(fragID);

            m_h_adcID->Fill(adcID_int);

            for (int ix = 0; ix != rawChannel->size(); ++ix) {
              m_rawAmp->push_back(rawChannel->amplitude(ix)); // [ADC counts]
              m_h_rawAmp->Fill(rawChannel->amplitude(ix));
            }
            for (int jx = 0; jx != rawChannel->sizeTime(); ++jx) {
              m_rawTime->push_back(rawChannel->time(jx)); // rel to triggering bunch
              m_h_rawTime->Fill(rawChannel->time(jx));
            }
            for (int kx = 0; kx != rawChannel->sizeQuality(); ++kx) {
              m_rawQual->push_back(rawChannel->quality(kx)); // sampling distr.
              m_h_rawQual->Fill(rawChannel->quality(kx));
            }
            m_rawPed->push_back(rawChannel->pedestal()); // reconstructed
            m_h_rawPed->Fill(rawChannel->pedestal());
          }
        }
      }

      // Muon Receiver Raw Channels

      if (!m_inputMuRcvRawChKey.empty()) {
        SG::ReadHandle<TileRawChannelContainer> muRawChannelContainer(m_inputMuRcvRawChKey);
        ATH_CHECK(muRawChannelContainer.isValid());
        // loop over muon receiver raw channels container
        for (const TileRawChannelCollection* muRawChannelCollection : *muRawChannelContainer) {
          for (const TileRawChannel* muRawChannel : *muRawChannelCollection) {

            const HWIdentifier adcHWID_mu(muRawChannel->adc_HWID());

            const unsigned long long adcHWID_mu_int = adcHWID_mu.get_compact();

            m_h_adcHWID_mu->Fill(adcHWID_mu_int);
            m_adcHWID_mu->push_back(adcHWID_mu_int);

            const int fragID_mu(muRawChannel->frag_ID());
            m_fragID_mu->push_back(fragID_mu);

            for (int lx = 0; lx != muRawChannel->size(); ++lx){
              m_rawAmp_mu->push_back(muRawChannel->amplitude(lx));
              m_h_rawAmp_mu->Fill(muRawChannel->amplitude(lx));
            }
            for (int mx = 0; mx != muRawChannel->sizeTime(); ++mx) {
              m_rawTime_mu->push_back(muRawChannel->time(mx));
              m_h_rawTime_mu->Fill(muRawChannel->time(mx));
            }
            for (int nx = 0; nx != muRawChannel->sizeQuality(); ++nx) {
              m_rawQual_mu->push_back(muRawChannel->quality(nx));
              m_h_rawQual_mu->Fill(muRawChannel->quality(nx));
            }
            m_rawPed_mu->push_back(muRawChannel->pedestal());
            m_h_rawPed_mu->Fill(muRawChannel->pedestal());
          }
        }
      }


      // Tile Container - TileMuonReceiverContainer

      if (!m_inputMuRcvKey.empty()) {
        SG::ReadHandle<TileMuonReceiverContainer> muRcvContainer(m_inputMuRcvKey);
        ATH_CHECK(muRcvContainer.isValid());
        // loop over muon receiver container

        for (const TileMuonReceiverObj* muRcv : *muRcvContainer) {
          const int muRcvID(muRcv->GetID());
          const std::vector<bool>& dec_vec = muRcv->GetDecision();
          const std::vector<float>& thresh_vec = muRcv->GetThresholds();
          const std::vector<float>& ene_vec = muRcv->GetEne();
          const std::vector<float>& time_vec = muRcv->GetTime();

          m_muRcvID->push_back(muRcvID);

          for (bool dec : dec_vec) {
            m_muRcv_dec->push_back(dec);
            m_h_muRcv_dec->Fill(dec);
          }
          for (float thresh : thresh_vec) {
            m_muRcv_thresh->push_back(thresh);
            m_h_muRcv_thresh->Fill(thresh);
          }
          for (float ene : ene_vec) {
            m_muRcv_energy->push_back(ene);
            m_h_muRcv_energy->Fill(ene);
          }
          for (float time : time_vec) {
            m_muRcv_time->push_back(time);
            m_h_muRcv_time->Fill(time);
          }

          m_h_muRcvID->Fill(muRcvID);
        }
      }


      // Tile Container - TileTTL1Container
      // Raw Tile L1 Trigger Towers

      if (!m_inputMBTS_TTL1Key.empty()) {
        SG::ReadHandle<TileTTL1Container> ttl1MBTSContainer(m_inputMBTS_TTL1Key);
        ATH_CHECK(ttl1MBTSContainer.isValid());
        // loop over TTL1 MBTS container

        for (const TileTTL1* ttl1MBTS : *ttl1MBTSContainer) {
          const Identifier ttl1MBTS_ID(ttl1MBTS->identify());
          const std::vector<double> ttl1MBTS_digits(ttl1MBTS->samples());

          const unsigned long long ttl1MBTS_ID_int = ttl1MBTS_ID.get_compact();
          m_ttl1MBTS_ID->push_back(ttl1MBTS_ID_int); // identifier
          m_ttl1MBTS_digits->push_back(ttl1MBTS_digits); // hardware sum of Tile channels; read out in N time slices

          for (double sample : ttl1MBTS_digits) {
            m_h_ttl1MBTS_digits->Fill(sample);
          }

          m_h_ttl1MBTS_ID->Fill(ttl1MBTS_ID_int);
        }
      }

      if (!m_inputTileTTL1Key.empty()) {
        SG::ReadHandle<TileTTL1Container> ttl1Container(m_inputTileTTL1Key);
        ATH_CHECK(ttl1Container.isValid());
        // loop over TTL1 container
        for (const TileTTL1* tile_TTL1 : *ttl1Container) {

          const Identifier ttl1ID(tile_TTL1->identify());
          const std::vector<double> ttl1_digits(tile_TTL1->samples());

          const unsigned long long ttl1ID_int = ttl1ID.get_compact();
          m_ttl1_ID->push_back(ttl1ID_int);
          m_ttl1_digits->push_back(ttl1_digits);

          for (double sample : ttl1_digits) {
            m_h_ttl1_digits->Fill(sample);
          }

          m_h_ttl1_ID->Fill(ttl1ID_int);
        }
      }


      // Tile Container - TileL2
      // TileMuId and Et computed at TileCal ROD DSPs (use for L2 trigger)
      std::vector<unsigned int> val_vec;
      std::vector<float> eta_vec;
      std::vector<float> enemu0_vec;
      std::vector<float> enemu1_vec;
      std::vector<float> enemu2_vec;
      std::vector<unsigned int> qual_vec;
      std::vector<float> sumE_vec;


      if (!m_inputL2Key.empty()) {
        SG::ReadHandle<TileL2Container> l2Container(m_inputL2Key);
        ATH_CHECK(l2Container.isValid());
        // loop over L2 container
        for (const TileL2* tile_L2 : *l2Container) {
          // drawer ID
          const int L2ID(tile_L2->identify());
          // packed muon info (32-bit words)
          for (unsigned int ii = 0; ii != tile_L2->Ndata(); ii++) {
            val_vec.push_back(tile_L2->val(ii));
            m_h_L2val->Fill(val_vec.at(ii));
          }
          // muon info - energy deposited in TileCal layers, eta, quality flag
          for (unsigned int jj = 0; jj != tile_L2->NMuons(); jj++) {
            eta_vec.push_back(tile_L2->eta(jj));
            enemu0_vec.push_back(tile_L2->enemu0(jj));
            enemu1_vec.push_back(tile_L2->enemu1(jj));
            enemu2_vec.push_back(tile_L2->enemu2(jj));
            qual_vec.push_back(tile_L2->qual(jj));

            m_h_L2eta->Fill(eta_vec.at(jj));
            m_h_L2energyA->Fill(enemu0_vec.at(jj));
            m_h_L2energyBC->Fill(enemu1_vec.at(jj));
            m_h_L2energyD->Fill(enemu2_vec.at(jj));
            m_h_L2qual->Fill(qual_vec.at(jj));
          }
          // drawer phi
          const float l2phi(tile_L2->phi(0));
          // vector sumE = [sumEt, sumEz, sumE] per TileCal superdrawer
          for (unsigned int kk = 0; kk != tile_L2->NsumE(); kk++) {
            sumE_vec.push_back(tile_L2->sumE(kk));
            m_h_L2sumE->Fill(sumE_vec.at(kk));
          }

          m_L2ID->push_back(L2ID);
          m_L2val->push_back(val_vec);
          m_L2eta->push_back(eta_vec);
          m_L2energyA->push_back(enemu0_vec);
          m_L2energyBC->push_back(enemu1_vec);
          m_L2energyD->push_back(enemu2_vec);
          m_L2qual->push_back(qual_vec);
          m_L2phi->push_back(l2phi);
          m_L2sumE->push_back(sumE_vec);

          m_h_L2ID->Fill(L2ID);
          m_h_L2phi->Fill(l2phi);

          val_vec.clear();
          eta_vec.clear();
          enemu0_vec.clear();
          enemu1_vec.clear();
          enemu2_vec.clear();
          qual_vec.clear();
          sumE_vec.clear();
        }
      }
    }

  // TileDigitsContainer - TileDigitsFlt

  if (!m_inputDigitsFltKey.empty()) {
    SG::ReadHandle<TileDigitsContainer> digitsContainer(m_inputDigitsFltKey);
    ATH_CHECK(digitsContainer.isValid());
    // loop over tile digits container
    for (const TileDigitsCollection* digitsCollection : *digitsContainer) {

      uint32_t fragSize(digitsCollection->getFragSize());
      uint32_t fragBCID(digitsCollection->getFragBCID());

      m_fragSize->push_back(fragSize);
      m_fragBCID->push_back(fragBCID);

      for (const TileDigits* tileDigits : *digitsCollection) {
        const std::vector<double> digits(tileDigits->get_digits());
        m_digits->push_back(digits);

        for (const double sample : digits) {
          m_h_digits->Fill(sample);
        }
      }
    }
  }

  // TileDigitsContainer - MuRcvDigitsCnt

  if (!m_inputDigitsMuRcvKey.empty()) {
    SG::ReadHandle<TileDigitsContainer> muRcvDigitsContainer(m_inputDigitsMuRcvKey);
    ATH_CHECK(muRcvDigitsContainer.isValid());
    // loop over tile digits container
    for (const TileDigitsCollection* muRcvDigitsCollection : *muRcvDigitsContainer) {
      const uint32_t muFragSize(muRcvDigitsCollection->getFragSize());
      const uint32_t muFragBCID(muRcvDigitsCollection->getFragBCID());

      m_muFragSize->push_back(muFragSize);
      m_muFragBCID->push_back(muFragBCID);

      for (const TileDigits* muRcvDigits : *muRcvDigitsCollection) {
        const std::vector<double> muDigits(muRcvDigits->get_digits());
        m_muDigits->push_back(muDigits);
        for (const double sample : muDigits) {
          m_h_muDigits->Fill(sample);
        }
      }
    }
  }

  if (m_tree) {
    m_tree->Fill();
  }

  return StatusCode::SUCCESS;
}

StatusCode TileRDOAnalysis::finalize() {
  return StatusCode::SUCCESS;
}
