/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "CTPUnpackingTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigT1Result/CTPResult.h" // TODO: Deprecate in favour of using new xAOD::CTPResult class
#include "TrigT1Result/RoIBResult.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <boost/algorithm/string.hpp>

using namespace HLT;


CTPUnpackingTool::CTPUnpackingTool( const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent )
  : CTPUnpackingToolBase(type, name, parent) {}


StatusCode CTPUnpackingTool::initialize() {
  ATH_CHECK( m_L1MenuKey.initialize() );
  ATH_CHECK( m_HLTMenuKey.initialize() );
  ATH_CHECK( m_ctpResultKey.initialize( m_useEDMxAOD ) );


  ATH_CHECK( CTPUnpackingToolBase::initialize() );

  return StatusCode::SUCCESS;
}


StatusCode CTPUnpackingTool::start() {
  ATH_MSG_INFO( "Updating CTP bits decoding configuration");

  // iterate over all items and obtain the CPT ID for each item. Then, package that in the map: name -> CTP ID
  ATH_MSG_INFO( "start(): use new L1 trigger menu" );  
  // Cleanup in case there was a stop/start transition
  m_itemNametoCTPIDMap.clear();
  m_ctpToChain.clear();

  auto l1menu = SG::makeHandle( m_L1MenuKey );
  if( l1menu.isValid() ) {
    for ( const TrigConf::L1Item & item:   *l1menu ) {
      m_itemNametoCTPIDMap[item.name()] = item.ctpId();
    }
  } else {
    ATH_MSG_ERROR( "TrigConf::L1Menu does not exist" );
  }
  
  auto addIfItemExists = [&]( const std::string& itemName, HLT::Identifier id, bool warningOnly = false ) -> StatusCode {
    if ( m_itemNametoCTPIDMap.find( itemName ) != m_itemNametoCTPIDMap.end() ) {
      m_ctpToChain[ m_itemNametoCTPIDMap[itemName] ].push_back( id );
      return StatusCode::SUCCESS;
    }
    if( warningOnly ) {
       // this code should be removed after the L1 menu is migrated to the new json version
       ATH_MSG_WARNING(itemName << " used to seed the chain " << id <<" not in the configuration ");
       return StatusCode::SUCCESS;
    }
    ATH_MSG_ERROR(itemName << " used to seed the chain " << id <<" not in the configuration ");
    return StatusCode::FAILURE;
  };

  SG::ReadHandle<TrigConf::HLTMenu>  hltMenuHandle = SG::makeHandle( m_HLTMenuKey );
  ATH_CHECK( hltMenuHandle.isValid() );
  for ( const TrigConf::Chain& chain: *hltMenuHandle ) {
    HLT::Identifier chainID{ chain.name() };
    if ( chain.l1item().empty() ) { // unseeded chain
      m_ctpToChain[ s_CTPIDForUnseededChains ].push_back( chainID );
    } else if ( chain.l1item().find(',') != std::string::npos ) { // OR seeds

      std::vector<std::string> items;
      boost::split(items, chain.l1item(), [](char c){return c == ',';});
      for ( const std::string& i: items ) {
         ATH_CHECK( addIfItemExists( i, chainID, true ) );
      }
    } else { // regular chain
      ATH_CHECK( addIfItemExists( chain.l1item(), chainID ) );
    }
  }

  for ( const auto& ctpIDtoChain: m_ctpToChain ) {
    for ( auto chain: ctpIDtoChain.second ) {
      ATH_MSG_DEBUG( "CTP seed of " << ctpIDtoChain.first << " enables chains " << chain );
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode CTPUnpackingTool::decode( const EventContext& ctx, const ROIB::RoIBResult& roib,  HLT::IDVec& enabledChains ) const {
  auto nItems = Monitored::Scalar( "Items", 0 );
  auto nChains = Monitored::Scalar( "Chains", 0 );

  std::vector<uint32_t> ctpBits;
  if (m_useEDMxAOD) { // Need to make sure you can pick up the correct CTPResult in StoreGate

    // Retrieve xAOD::CTPResult object
    SG::ReadHandle<xAOD::CTPResult> ctpRes{ m_ctpResultKey, ctx };
    if (!ctpRes.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve CTPResult with key " << m_ctpResultKey.key());
      return StatusCode::FAILURE;
    };

    // Get the appropriate trigger bits for L1A bunch
    ctpBits = m_useTBPBit ? ctpRes->getTBPWords() : ctpRes->getTAVWords();

  }
  else {

    // Get appropriate trigger bits from ROIB::CTPResult object in RoIBResult object
    auto rois = m_useTBPBit ? roib.cTPResult().TBP() : roib.cTPResult().TAV();
    ctpBits.reserve(rois.size());

    // Trigger bits stored in vector of ROIB::CTPRoI objects (which are just uint32_t), need to convert to a vector of type uint32_t
    std::transform(rois.begin(), rois.end(), std::back_inserter(ctpBits), [](const ROIB::CTPRoI& roi){ return roi.roIWord(); });

  }
  const size_t bitsSize = ctpBits.size();
  constexpr static size_t wordSize{32};

  for ( size_t wordCounter = 0; wordCounter < bitsSize; ++wordCounter ) {
    for ( size_t bitCounter = 0;  bitCounter < wordSize; ++bitCounter ) {
      const size_t ctpIndex = wordSize*wordCounter + bitCounter;
      const bool decision = ( ctpBits[wordCounter] & ((uint32_t)1 << bitCounter) ) > 0;

      if ( decision or m_forceEnable ) {
        if ( decision ) {
          nItems = nItems + 1;
          ATH_MSG_DEBUG("L1 item " << ctpIndex << " active, enabling chains "
                        << (m_forceEnable ? " due to the forceEnable flag" : " due to the seed"));
        }

        auto itr = m_ctpToChain.find( ctpIndex );
        if ( itr != m_ctpToChain.end() ) {
          enabledChains.insert( enabledChains.end(), itr->second.begin(), itr->second.end() );
        }
      }
    }
  }
  // the unseeded chains are always enabled at this stage
  const auto itr = m_ctpToChain.find( s_CTPIDForUnseededChains );
  if ( itr != m_ctpToChain.cend() ) {
    enabledChains.insert( enabledChains.end(), itr->second.begin(), itr->second.end());
  }

  nChains = enabledChains.size();
  for ( auto chain: enabledChains ) {
    ATH_MSG_DEBUG( "Enabling chain: " << chain );
  }
  if ( nItems == 0 ) {
    ATH_MSG_ERROR( "All CTP bits were disabled, this event should not have shown here" );
    return StatusCode::FAILURE;
  }
  Monitored::Group( m_monTool, nItems, nChains );
  return StatusCode::SUCCESS;
}


StatusCode CTPUnpackingTool::passBeforePrescaleSelection(const ROIB::RoIBResult* roib,
                                                         const std::vector<std::string>& l1ItemNames,
                                                         bool& pass) const {

  pass = false;

  const auto ctpBits = roib->cTPResult().TBP();

  for (const std::string& l1name : l1ItemNames) {
    try {
      // Retrieve before prescale decision
      const size_t ctpId = m_itemNametoCTPIDMap.at(l1name);
      const size_t bitCounter = ctpId % 32;
      const size_t wordCounter = ctpId / 32;

      const bool decision = (ctpBits[wordCounter].roIWord() & ((uint32_t)1 << bitCounter)) > 0;

      pass = (pass || decision);
    }
    catch (const std::exception& e) {
      ATH_MSG_ERROR ( l1name << " is not part of L1Menu!" );
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}
