/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <map>
#include <utility>
#include <set>
#include <tuple>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <sstream>

#include "L1CaloCTPMonitorAlgorithm.h"

using TrigConf::L1DataDef;

L1CaloCTPMonitorAlgorithm::L1CaloCTPMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator),
    m_errorTool("LVL1::TrigT1CaloMonErrorTool/TrigT1CaloMonErrorTool")
{
}

StatusCode L1CaloCTPMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("L1CaloCTPMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);

  // we initialise all the containers that we need
  ATH_CHECK(m_ctpRdoKey.initialize());
  ATH_CHECK(m_cmxJetHitsLocation.initialize());
  ATH_CHECK(m_cmxEtSumsLocation.initialize());
  ATH_CHECK(m_cmxCpHitsLocation.initialize());

  ATH_CHECK( m_L1MenuKey.initialize() );
  renounce(m_L1MenuKey); // Detector Store data - hide this Data Dependency from the MT Scheduler


  ATH_CHECK(m_errorTool.retrieve());
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode L1CaloCTPMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("L1CaloCTPMonitorAlgorithm::fillHistograms");
  ATH_MSG_DEBUG("m_ctpRdoKey=" << m_ctpRdoKey);
  ATH_MSG_DEBUG("m_cmxJetHitsLocation=" << m_cmxJetHitsLocation);
  ATH_MSG_DEBUG("m_cmxEtSumsLocation=" << m_cmxEtSumsLocation);
  ATH_MSG_DEBUG("m_cmxCpHitsLocation=" << m_cmxCpHitsLocation);

  // monitored variables
  std::vector<int> errors;
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;

  // 1D
  auto run = Monitored::Scalar<int>("run",GetEventInfo(ctx)->runNumber());

  // read all objects needed
  ATH_MSG_DEBUG("Begin to fetch store gate objects ..");

  //--------------------------------------------------------------------------
  //---------------- get information sent from CP CMXs to CTP ----------------
  //--------------------------------------------------------------------------

  // Retrieve CMX CP hits from StoreGate
  SG::ReadHandle<xAOD::CMXCPHitsContainer> cmxCpHitsTES(m_cmxCpHitsLocation,ctx);
  ATH_CHECK(cmxCpHitsTES.isValid());

  uint32_t EMHits0  = 0;
  uint32_t EMHits1  = 0;
  uint32_t TauHits0 = 0;
  uint32_t TauHits1 = 0;  

  // CMX information for transmission check: System CMX -> CTP 
 if (cmxCpHitsTES.isValid()) { 
    xAOD::CMXCPHitsContainer::const_iterator cmxIterator    = cmxCpHitsTES->begin();
    xAOD::CMXCPHitsContainer::const_iterator cmxIteratorEnd = cmxCpHitsTES->end();   
 
    for (; cmxIterator != cmxIteratorEnd; ++cmxIterator) {
      
      const uint8_t source = (*cmxIterator)->sourceComponent();
      const uint8_t crate  = (*cmxIterator)->crate();
      const uint8_t cmx    = (*cmxIterator)->cmx();

      if (source == xAOD::CMXCPHits::TOTAL && crate == 3) {                       
        if (cmx == 1) {
          EMHits0 = (*cmxIterator)->hits0();
          EMHits1 = (*cmxIterator)->hits1();
        }
        else {
          TauHits0 = (*cmxIterator)->hits0();
          TauHits1 = (*cmxIterator)->hits1();
        }
      } 
    } 
  }   


  //--------------------------------------------------------------------------
  //----------------- get information sent from JEP CMXs to CTP --------------
  //--------------------------------------------------------------------------

  // Retrieve CMX Jet Hits from Storegate
  SG::ReadHandle<xAOD::CMXJetHitsContainer> cmxJetHitsTES(m_cmxJetHitsLocation,ctx);
  ATH_CHECK(cmxJetHitsTES.isValid());  
 
  if(!cmxJetHitsTES.isValid()){
  ATH_MSG_ERROR("No Jet Hits found in TES  "<< m_cmxJetHitsLocation); 
  return StatusCode::FAILURE;
  } 

  int Jet3BitHits = 0;  // 3-bit multiplicities, thresholds 0-9, cable JET1
  int Jet2BitHits = 0;  // 2-bit multiplicities, thresholds 10-24, cable JET2
  
  int Jet3BitHits0 = 0; // 3-bit multiplicities, thresholds 0-4, cable JET1
  int Jet3BitHits1 = 0; // 3-bit multiplicities, thresholds 5-9, cable JET1
  int Jet2BitHits0 = 0; // 2-bit multiplicities, thresholds 10-17, cable JET2
  int Jet2BitHits1 = 0; // 2-bit multiplicities, thresholds 18-24, cable JET2

  xAOD::CMXJetHitsContainer::const_iterator cmxJetIterator    = cmxJetHitsTES->begin(); 
  xAOD::CMXJetHitsContainer::const_iterator cmxJetIteratorEnd = cmxJetHitsTES->end(); 

  // Transmission check: system CMX -> CTP
  for (; cmxJetIterator != cmxJetIteratorEnd; ++cmxJetIterator) {
    const uint32_t source = (*cmxJetIterator)->sourceComponent();
    const uint32_t crate  = (*cmxJetIterator)->crate(); 
    
    if (source == xAOD::CMXJetHits::TOTAL_MAIN && crate == 1) {                                               
      Jet3BitHits0 = (*cmxJetIterator)->hits0();
      Jet3BitHits1 = (*cmxJetIterator)->hits1();
      Jet3BitHits  = Jet3BitHits0 + (Jet3BitHits1<<15);
    }
    if (source == xAOD::CMXJetHits::TOTAL_FORWARD && crate == 1) {
      Jet2BitHits0 = (*cmxJetIterator)->hits0();
      Jet2BitHits1 = (*cmxJetIterator)->hits1();
      Jet2BitHits  = Jet2BitHits0 + (Jet2BitHits1<<16);
    }
  }

  // Retrieve CMX Et sums from Storegate
  SG::ReadHandle<xAOD::CMXEtSumsContainer> cmxEtHitsTES(m_cmxEtSumsLocation,ctx);
  ATH_CHECK(cmxEtHitsTES.isValid());

  if(!cmxEtHitsTES.isValid()){
  ATH_MSG_ERROR("No Et Hits found in TES  "<< m_cmxEtSumsLocation);
  return StatusCode::FAILURE;
  }

  int TEHits     = 0; // Cable EN1 (full eta)
  int XEHits     = 0;
  int XSHits     = 0;
  int TERestHits = 0; // Cable EN2 (restricted eta)
  int XERestHits = 0;

  xAOD::CMXEtSumsContainer::const_iterator cmxEtSumsIterator    = cmxEtHitsTES->begin();
  xAOD::CMXEtSumsContainer::const_iterator cmxEtSumsIteratorEnd = cmxEtHitsTES->end(); 

  for (; cmxEtSumsIterator != cmxEtSumsIteratorEnd; ++ cmxEtSumsIterator) {
    const uint32_t source = (*cmxEtSumsIterator)->sourceComponent();
    const uint32_t crate  = (*cmxEtSumsIterator)->crate();

    // Sum Et hits 
    if (source == xAOD::CMXEtSums::SUM_ET_STANDARD && crate == 1) { // KW crate check might not be needed here...
      TEHits = (*cmxEtSumsIterator)->et();
    }

    // Missing Et hits 
    if (source == xAOD::CMXEtSums::MISSING_ET_STANDARD && crate == 1) {
      XEHits = (*cmxEtSumsIterator)->et();
    }

    // Missing Et significance hits 
    if (source == xAOD::CMXEtSums::MISSING_ET_SIG_STANDARD && crate == 1) {
      XSHits = (*cmxEtSumsIterator)->et();
    }

    // Sum Et hits (restricted eta)
    if (source == xAOD::CMXEtSums::SUM_ET_RESTRICTED && crate == 1) {  
      TERestHits = (*cmxEtSumsIterator)->et();
    }

    // Missing Et hits (restricted eta)
    if (source == xAOD::CMXEtSums::MISSING_ET_RESTRICTED && crate == 1) {
      XERestHits = (*cmxEtSumsIterator)->et();
    }
  }

//--------------------------------------------------------------------------
//----------------------- get the TIPs (input for CTP) ---------------------
//--------------------------------------------------------------------------

  SG::ReadHandle<CTP_RDO> const_ctpRdo(m_ctpRdoKey,ctx);
  ATH_CHECK(const_ctpRdo.isValid());

  if(!const_ctpRdo.isValid()){
  ATH_MSG_ERROR("No CTP_RDO found in TES  "<< m_ctpRdoKey);
  return StatusCode::FAILURE;
  }


  // Make a writable copy and Set CTP version number to 4 when reading persistified data
  std::vector<uint32_t> ctp_data=const_ctpRdo->getDataWords();
  CTP_RDO ctpRdo(4,std::move(ctp_data));
  ctpRdo.setL1AcceptBunchPosition(const_ctpRdo->getL1AcceptBunchPosition());
  ctpRdo.setTurnCounter(const_ctpRdo->getTurnCounter());

  if (ctpRdo.getCTPVersionNumber()==0) {
    ATH_MSG_DEBUG("CTP version number not set, skipping CTP test");
  }

  CTP_Decoder ctp;
  ctp.setRDO(&ctpRdo);

  const uint16_t l1aPos = ctpRdo.getL1AcceptBunchPosition();
  if (l1aPos >= ctp.getBunchCrossings().size()) {
    ATH_MSG_DEBUG("CTP_RDO gave Invalid l1aPos");;
    return StatusCode::SUCCESS;
  }
   
  ATH_MSG_DEBUG("CTP l1aPos, size : " << l1aPos << ", " << ctp.getBunchCrossings().size());
    const CTP_BC& bunch = ctp.getBunchCrossings().at(l1aPos);

//std::cout<<"CTP Confg = "<<getL1Menu(ctx)->thresholds().size()<<std::endl;

 //--------------------------------------------------------------------------
 //---------------------- compare L1Calo data with CTP ----------------------
 //--------------------------------------------------------------------------

  const int max_JET_2bit_Threshold_Number = 15;
  const int max_JET_3bit_Threshold_Number = 10;
  const int max_XE_Threshold_Number = 16;
  const int max_TE_Threshold_Number = 16;
  const int max_XS_Threshold_Number = 8;

  //------------------------ EM Hits (3 bits / thresh) -----------------------
  int offset = 0;
  int threshBits = 3;
  int totalBits = threshBits*L1DataDef::typeConfig(L1DataDef::EM).max/2;
  ATH_MSG_DEBUG("totalBits = " << totalBits);

  compare(bunch, EMHits0, totalBits, offset, EM1Type, ctx); // Cable EM1

  offset += totalBits;
  compare(bunch, EMHits1, totalBits, offset, EM2Type, ctx); // Cable EM2

  //----------------------- Tau Hits (3 bits / thresh) ---------------------
  offset += totalBits;
  compare(bunch, TauHits0, totalBits, offset, Tau1Type, ctx); // Cable TAU1

  offset += totalBits; 
  compare(bunch, TauHits1, totalBits, offset, Tau2Type, ctx); // Cable TAU2

  //------------------------ Jet Hits (3 bits / thresh) ---------------------
  offset += totalBits;
  totalBits = threshBits*max_JET_3bit_Threshold_Number;    
  compare(bunch, Jet3BitHits, totalBits, offset, Jet3BitType, ctx); // Cable JET1
  
  //----------------------- Jet Hits (2 bits / thresh) -------------------
  offset += totalBits;
  threshBits--;
  totalBits = threshBits*max_JET_2bit_Threshold_Number;
  compare(bunch, Jet2BitHits, totalBits, offset, Jet2BitType, ctx); // Cable JET2

  //---------------------- TE Hits (1 bit / thresh) ------------------
  offset += totalBits;
  threshBits--;
  totalBits = threshBits*max_TE_Threshold_Number/2;  // Cable EN1 
  compare(bunch, TEHits, totalBits, offset, TEFullEtaType, ctx);
  
  //------------------------ XE Hits (1 bit / thresh) --------------------
  offset += totalBits;
  totalBits = threshBits*max_XE_Threshold_Number/2;  
  compare(bunch, XEHits, totalBits, offset, XEFullEtaType, ctx);
  
  //---------------------- XS Hits (1 bit / thresh) ------------------
  offset += totalBits;
  totalBits = threshBits*max_XS_Threshold_Number;   
  compare(bunch, XSHits, totalBits, offset, XSType, ctx);
  
  //------------------------ Restricted Eta TE Hits (1 bit / thresh) --------------------
  offset += totalBits;
  totalBits = threshBits*max_TE_Threshold_Number/2;  // Cable EN2
  compare(bunch, TERestHits, totalBits, offset, TERestrictedEtaType, ctx);
  
  //---------------------- Restricted Eta XE Hits (1 bit / thresh) ------------------
  offset += totalBits;
  totalBits = threshBits*max_XE_Threshold_Number/2; 
  compare(bunch, XERestHits, totalBits, offset, XERestrictedEtaType, ctx);

  variables.push_back(run);
  fill(m_packageName,variables);
  variables.clear();

  return StatusCode::SUCCESS;
}

// *********************************************************************
// Private Methods
// *********************************************************************

void L1CaloCTPMonitorAlgorithm::compare(const CTP_BC& bunch, int hits, int totalBits,
                           int offset, L1CaloCTPHitTypes type, const EventContext& ctx ) const
{


  const int max_JET_2bit_Threshold_Number = 15;
  const int max_JET_3bit_Threshold_Number = 10;
  const int max_TAU_3bit_Threshold_Number = 16;
  const int max_XE_Threshold_Number = 16;
  const int max_TE_Threshold_Number = 16;
  const int max_XS_Threshold_Number = 8;

       std::map<std::string, int> threshMap;

      const std::vector<std::shared_ptr<TrigConf::L1Threshold>>& thresholds = getL1Menu(ctx)->thresholds();
      ATH_MSG_DEBUG("Size of thresholds vector: " << thresholds.size());
      
      for (const auto& it : thresholds) {
        int offset = 0;
        int nbits = 3;
        const int threshNumber = it->mapping();
        int fixedThreshNumber  = threshNumber;

        while (true) {
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::EM) ) {
            if (threshNumber >= (int)L1DataDef::typeConfig(L1DataDef::EM).max/2) {  // Cable EM2; else cable EM1
              offset += nbits*L1DataDef::typeConfig(L1DataDef::EM).max/2;
              fixedThreshNumber -= L1DataDef::typeConfig(L1DataDef::EM).max/2;
            }
            break;
          }
          offset += nbits*L1DataDef::typeConfig(L1DataDef::EM).max;
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::TAU) ) {
            if (threshNumber >= (int)max_TAU_3bit_Threshold_Number/2) { // Cable TAU2; else cable TAU1
              offset += nbits*max_TAU_3bit_Threshold_Number/2;
              fixedThreshNumber -= max_TAU_3bit_Threshold_Number/2;
            }
            break;
          }
          offset += nbits*max_TAU_3bit_Threshold_Number; 
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::JET) ) {
            if (threshNumber >= (int)max_JET_3bit_Threshold_Number) {   // Cable JET2 (2-bit thresholds); else JET1 (3-bit)
              offset += 3*max_JET_3bit_Threshold_Number;
              fixedThreshNumber -= max_JET_3bit_Threshold_Number;
              nbits--;
            }
            break;
          }
          offset += 3*max_JET_3bit_Threshold_Number;
          nbits--;
          offset += 2*max_JET_2bit_Threshold_Number;
          nbits--;
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::TE) ) {
            if (threshNumber >= (int)max_TE_Threshold_Number/2) {  // Restricted eta TE threshold: jump to cable EN2
              offset += nbits*max_TE_Threshold_Number/2 + nbits*max_XE_Threshold_Number/2 + nbits*max_XS_Threshold_Number; // 8+8+8 bits on cable EN1
              fixedThreshNumber -= max_TE_Threshold_Number/2;
            }
            break;  // Full eta & restricted eta thresholds separated on two cables
         }
          offset += nbits*max_TE_Threshold_Number/2; 
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::XE) ) {
            if (threshNumber >= (int)max_XE_Threshold_Number/2) { // Restricted eta XE threshold: jump to cable EN2
              offset += nbits*max_TE_Threshold_Number/2 + nbits*max_XE_Threshold_Number/2 + nbits*max_XS_Threshold_Number;
              fixedThreshNumber -= max_XE_Threshold_Number/2;
            }
            break;
          }
          offset += nbits*max_XE_Threshold_Number/2;
          if ( it->type() == L1DataDef::typeAsString(L1DataDef::XS) ) break;
          offset += nbits*max_XS_Threshold_Number;
          nbits--;
          break;
        }
        if (nbits == 0) continue;
        if (threshNumber < 0) continue;
        threshMap.insert(std::make_pair(it->name(),
	                 offset + fixedThreshNumber*nbits));
        ATH_MSG_DEBUG("threshMap: name, offset, threshNumber, nbits  " << it->name() << " " << offset << " " << fixedThreshNumber << " " << nbits);
      } // End loop over thresholds vector

      ATH_MSG_DEBUG("Size of threshMap = " << threshMap.size());

    std::vector<std::pair<std::string, int>> newMap;
    newMap.clear();

    for (const auto& entry : threshMap) {
	
	//int numBits = getNumBits(entry.first, ctx);
        TrigConf::TriggerLine trigger_line = getL1Menu(ctx)->connector(getL1Menu(ctx)->connectorNameFromThreshold(entry.first)).triggerLine(entry.first);
	int numBits = trigger_line.nbits();

        for (int bit = 0; bit < numBits; ++bit) {
            int newValue = entry.second + bit;

            // Add the entry to the new map
            newMap.push_back(std::make_pair(entry.first, newValue));
	}
     }

    // Sort the newMap based on the second position values
    std::sort(newMap.begin(), newMap.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.second < rhs.second;
    });

    ATH_MSG_DEBUG("Size of new threshMap = " << newMap.size());

  // 1D
  auto ctp_1d_L1CaloNeCTPSummary = Monitored::Scalar<float>("ctp_1d_L1CaloNeCTPSummary",0); ///< Transmission Errors between L1Calo and CTP
  auto ctp_1d_L1CaloEqCTPSummary = Monitored::Scalar<float>("ctp_1d_L1CaloEqCTPSummary",0); ///< Transmission Matches between L1Calo and CTP
  auto ctp_1d_TIPMatches = Monitored::Scalar<float>("ctp_1d_TIPMatches",0); ///< CTP/L1Calo TIP Matches
  auto ctp_1d_HitNoTIPMismatch = Monitored::Scalar<float>("ctp_1d_HitNoTIPMismatch",0); ///< L1Calo Hit but no CTP TIP Mismatches
  auto ctp_1d_TIPNoHitMismatch = Monitored::Scalar<float>("ctp_1d_TIPNoHitMismatch",0); ///< CTP TIP but no L1Calo Hit Mismatches

  ATH_MSG_DEBUG("offset: " << offset << " totalBits: " << totalBits);
  //std::string name(ctp_1d_L1CaloNeCTPSummary->GetXaxis()->GetBinLabel(1+type));
  //std::string subdet((type == EM1Type  || type == EM2Type || type == Tau1Type || type == Tau2Type) ? " CP:  " : " JEP: ");

  int mask    = 0;
  int tipHits = 0;
 
  for (int bit = 0; bit < totalBits; ++bit) {
    const int TIPid = (newMap[offset + bit]).second;

  ATH_MSG_DEBUG("TIPid = " << TIPid);
    if (TIPid < 0){ //|| TIPid > 511) {
      continue;
    }
    const int HITbit= ((hits >> bit) & 0x1);
    const int TIPbit = bunch.getTIP().test( TIPid );

    ATH_MSG_DEBUG(TIPbit << " ");  
    tipHits |= (TIPbit << bit);
    mask    |= (1 << bit);
    if (HITbit && HITbit == TIPbit){
       ctp_1d_TIPMatches = TIPid;
       fill(m_packageName,ctp_1d_TIPMatches);
    }
    else if (HITbit != TIPbit) {
      if (HITbit){
	 ctp_1d_HitNoTIPMismatch = TIPid;
         fill(m_packageName,ctp_1d_HitNoTIPMismatch);
      }
      else{
        ctp_1d_TIPNoHitMismatch = TIPid;
        fill(m_packageName,ctp_1d_TIPNoHitMismatch);
      }
    }
  }

  
  if (tipHits != (hits&mask)) {
    ctp_1d_L1CaloNeCTPSummary = type;
    fill(m_packageName, ctp_1d_L1CaloNeCTPSummary);
   } 
  else if (tipHits){
    ctp_1d_L1CaloEqCTPSummary = type;
    fill(m_packageName,ctp_1d_L1CaloEqCTPSummary);
  } 
}

const TrigConf::L1Menu* L1CaloCTPMonitorAlgorithm::getL1Menu(const EventContext& ctx) const {
  const TrigConf::L1Menu* menu = nullptr;
  if (detStore()->contains<TrigConf::L1Menu>(m_L1MenuKey.key())) {
    SG::ReadHandle<TrigConf::L1Menu>  l1MenuHandle = SG::makeHandle( m_L1MenuKey, ctx );
    if( l1MenuHandle.isValid() ){
      menu=l1MenuHandle.cptr();
    }
  } else {
    menu = &(m_configSvc->l1Menu(ctx)); 
  }

  return menu;
}

/*void L1CaloCTPMon::setLabels(LWHist* hist, bool xAxis)
{
  LWHist::LWHistAxis* axis = (xAxis) ? hist->GetXaxis() : hist->GetYaxis();
  axis->SetBinLabel(1+EM1Type,             "EM1");
  axis->SetBinLabel(1+EM2Type,             "EM2");
  axis->SetBinLabel(1+Tau1Type,            "Tau1");
  axis->SetBinLabel(1+Tau2Type,            "Tau2");
  axis->SetBinLabel(1+Jet3BitType,         "Jet1 (3-bit)");
  axis->SetBinLabel(1+Jet2BitType,         "Jet2 (2-bit)"); 
  axis->SetBinLabel(1+TEFullEtaType,       "TE (full eta)");  
  axis->SetBinLabel(1+XEFullEtaType,       "XE (full eta)");
  axis->SetBinLabel(1+XSType,              "XS");
  axis->SetBinLabel(1+TERestrictedEtaType, "TE (restr. eta)");
  axis->SetBinLabel(1+XERestrictedEtaType, "XE (restr. eta)");
}*/




