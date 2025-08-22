/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JRoIsUnpackingTool.h"
#include "TrigT1Result/RoIBResult.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "AthenaMonitoringKernel/Monitored.h"


JRoIsUnpackingTool::JRoIsUnpackingTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
  : RoIsUnpackingToolBase(type, name, parent) {}


StatusCode JRoIsUnpackingTool::initialize() {
  ATH_CHECK( RoIsUnpackingToolBase::initialize() );
  ATH_CHECK( m_recRoIsKey.initialize() );

  return StatusCode::SUCCESS;
}


StatusCode JRoIsUnpackingTool::start() {
  ATH_CHECK(decodeMapping([](const std::string& name){
    return name.find('J') == 0 or name.find("JF") == 0 or name.find("JB") == 0;
  }));
  return StatusCode::SUCCESS;
}


StatusCode JRoIsUnpackingTool::unpack(const EventContext& ctx,
                                      const ROIB::RoIBResult& roib,
                                      const HLT::IDSet& activeChains) const {
  using namespace TrigCompositeUtils;
  // create and record the collections needed
  SG::WriteHandle<TrigRoiDescriptorCollection> trigRoIs = createAndStoreNoAux(m_trigRoIsKey, ctx );
  SG::WriteHandle< DataVector<LVL1::RecJetRoI> > recRoIs = createAndStoreNoAux( m_recRoIsKey, ctx );
  SG::WriteHandle<DecisionContainer> decisionOutput = createAndStore(m_decisionsKey, ctx );

  // Retrieve the L1 menu configuration
  SG::ReadHandle<TrigConf::L1Menu> l1Menu = SG::makeHandle(m_l1MenuKey, ctx);
  ATH_CHECK(l1Menu.isValid());
  std::optional<ThrVecRef> jetThresholds;
  ATH_CHECK(getL1Thresholds(*l1Menu, "JET", jetThresholds));

  // Flag if there was an overflow in the TOB transmission to CMX (there were more TOBs than can be transferred)
  bool overflow{false};
  constexpr static unsigned int s_maxJetTOBs{4}; // Hardcoded in L1Calo firmware, see ATR-23697 and ATR-12285
  std::unordered_map<unsigned int, std::unordered_map<unsigned int, unsigned int>> tobCounts; // {crate, {module, count}}

  // RoIBResult contains vector of jet fragments
  for ( const auto & jetFragment : roib.jetEnergyResult() ) {
    for ( const auto & roi : jetFragment.roIVec() ) {
      uint32_t roIWord = roi.roIWord();
      if ( not ( LVL1::TrigT1CaloDefs::JetRoIWordType == roi.roIType() ) )  {
        ATH_MSG_DEBUG( "Skipping RoI as it is not JET threshold " << roIWord <<" Type "<< roi.roIType() );
        continue;
      }

      if (!overflow) {
        unsigned int crate = m_jepDecoder.crate(roIWord);
        unsigned int module = m_jepDecoder.module(roIWord);
        overflow = (++tobCounts[crate][module] > s_maxJetTOBs);
      }

      recRoIs->push_back( std::make_unique<LVL1::RecJetRoI>(roIWord, l1Menu.cptr()) );
      const LVL1::RecJetRoI* recRoI = recRoIs->back();

      trigRoIs->push_back( std::make_unique<TrigRoiDescriptor>(
        roIWord, 0u ,0u,
        recRoI->eta(), recRoI->eta()-m_roIWidth, recRoI->eta()+m_roIWidth,
        recRoI->phi(), recRoI->phi()-m_roIWidth, recRoI->phi()+m_roIWidth) );

      ATH_MSG_DEBUG( "RoI word: 0x" << MSG::hex << std::setw( 8 ) << roIWord << MSG::dec );

      // The hltSeedingNodeName denotes an initial node with no parents
      Decision* decision = TrigCompositeUtils::newDecisionIn( decisionOutput.ptr(), hltSeedingNodeName() );

      std::vector<unsigned> passedThresholdIDs;

      for (const auto& th : jetThresholds.value().get()) {
        ATH_MSG_VERBOSE( "Checking if the threshold " << th->name() << " passed" );
        if ( recRoI->passedThreshold(th->mapping()) ) {
          passedThresholdIDs.push_back( HLT::Identifier( th->name() ) );
          ATH_MSG_DEBUG( "Passed Threshold name " << th->name() );
          addChainsToDecision( HLT::Identifier( th->name() ), decision, activeChains );
          ATH_MSG_DEBUG( "Labeled object with chains: " << [&](){
            TrigCompositeUtils::DecisionIDContainer ids;
            TrigCompositeUtils::decisionIDs( decision, ids );
            return std::vector<TrigCompositeUtils::DecisionID>( ids.begin(), ids.end() ); }() );
        }
      }

      decision->setDetail( "thresholds", passedThresholdIDs );
      decision->setObjectLink( initialRoIString(),
                               ElementLink<TrigRoiDescriptorCollection>(m_trigRoIsKey.key(), trigRoIs->size()-1, ctx) );
      decision->setObjectLink( initialRecRoIString(),
                               ElementLink<DataVector<LVL1::RecJetRoI>>(m_recRoIsKey.key(), recRoIs->size()-1, ctx) );
    }
  }

  // Decorate the decisions with overflow information
  if (overflow) {
    ATH_MSG_WARNING("L1Calo overflow for JET TOBs to CMX detected");
  }
  for (Decision* decision : *decisionOutput) {
    decision->setDetail("overflow", static_cast<char>(overflow));
  }

  if ( msgLvl(MSG::DEBUG) ) {
    for ( auto roi: *trigRoIs ) {
      ATH_MSG_DEBUG( "RoI Eta: " << roi->eta() << " Phi: " << roi->phi() << " RoIWord: " << roi->roiWord() );
    }
  }

  // monitoring
  {
    auto RoIsCount = Monitored::Scalar( "count", trigRoIs->size() );
    auto RoIsPhi   = Monitored::Collection( "phi", *trigRoIs, &TrigRoiDescriptor::phi );
    auto RoIsEta   = Monitored::Collection( "eta", *trigRoIs, &TrigRoiDescriptor::eta );
    Monitored::Group( m_monTool,  RoIsCount, RoIsEta, RoIsPhi );
  }

  ATH_MSG_DEBUG( "Unpacked " << trigRoIs->size() << " RoIs" );

  return StatusCode::SUCCESS;
}
