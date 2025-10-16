/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTop/BoostedHadTopAndTopPairFilterAugmentation.h"
#include "StoreGate/WriteDecorHandle.h"


namespace DerivationFramework {


  BoostedHadTopAndTopPairFilterAugmentation::BoostedHadTopAndTopPairFilterAugmentation(const std::string& t, const std::string& n, const IInterface* p):
    base_class(t,n,p)
  {
  }



  BoostedHadTopAndTopPairFilterAugmentation::~BoostedHadTopAndTopPairFilterAugmentation(){}



  StatusCode BoostedHadTopAndTopPairFilterAugmentation::initialize(){
    ATH_MSG_DEBUG( "Initialize" );
    ATH_CHECK(m_eventInfoName.initialize());
    ATH_CHECK(m_ttbarSysPt_HighKey.initialize());
    ATH_CHECK(m_HadTopPt_HighKey.initialize());
    ATH_CHECK(m_ttbarSysPt_LowKey.initialize());
    ATH_CHECK(m_HadTopPt_LowKey.initialize());
    ATH_CHECK(m_filterTool_High.retrieve());
    ATH_CHECK(m_filterTool_Low.retrieve());
    return StatusCode::SUCCESS;
  }



  StatusCode BoostedHadTopAndTopPairFilterAugmentation::addBranches(const EventContext& ctx) const{
    SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoName, ctx};
    if (!eventInfo.isValid()) {
      ATH_MSG_ERROR("could not retrieve event info " <<m_eventInfoName);
      return StatusCode::FAILURE;
    }

    // call first the tool for high pT values
    SG::WriteDecorHandle<xAOD::EventInfo, int> decorationttbarSysPt_High(m_ttbarSysPt_HighKey, ctx);
    SG::WriteDecorHandle<xAOD::EventInfo, int> decorationHadTopPt_High(m_HadTopPt_HighKey, ctx);

    int filterCode_High = m_filterTool_High->filterFlag(500000.0, 350000.0);

    if (filterCode_High == 0 ){
      decorationHadTopPt_High(*eventInfo) = 0;
      decorationttbarSysPt_High(*eventInfo) = 0;
    }
    else if ( filterCode_High == 1){
      decorationttbarSysPt_High(*eventInfo) = 1;
      decorationHadTopPt_High(*eventInfo) = 0;
    }
    else if ( filterCode_High == 2){
      decorationttbarSysPt_High(*eventInfo) = 0;
      decorationHadTopPt_High(*eventInfo) = 1;

    }
    else if ( filterCode_High == 3){
      decorationttbarSysPt_High(*eventInfo) = 1;
      decorationHadTopPt_High(*eventInfo) = 1;
    }


    // now call tool for low pT values
    SG::WriteDecorHandle<xAOD::EventInfo, int> decorationttbarSysPt_Low(m_ttbarSysPt_LowKey, ctx);
    SG::WriteDecorHandle<xAOD::EventInfo, int> decorationHadTopPt_Low(m_HadTopPt_LowKey, ctx);

    int filterCode_Low = m_filterTool_Low->filterFlag(200000.0, 150000.0);

    if (filterCode_Low == 0 ){
      decorationHadTopPt_Low(*eventInfo) = 0;
      decorationttbarSysPt_Low(*eventInfo) = 0;
    }
    else if ( filterCode_Low == 1){
      decorationttbarSysPt_Low(*eventInfo) = 1;
      decorationHadTopPt_Low(*eventInfo) = 0;
    }
    else if ( filterCode_Low == 2){
      decorationttbarSysPt_Low(*eventInfo) = 0;
      decorationHadTopPt_Low(*eventInfo) = 1;

    }
    else if ( filterCode_Low == 3){
      decorationttbarSysPt_Low(*eventInfo) = 1;
      decorationHadTopPt_Low(*eventInfo) = 1;
    }



    //ATH_MSG_INFO("filterCode "<<filterCode );


    return StatusCode::SUCCESS;

  }



} /// namespace
