/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**************************************************************************
 **
 **   File:         TrigDecisionMakerMT.h
 **
 **   Description:  - Re-entrant Algorithm-derived class to run after the MT Trigger to create the
 **                   xAOD::TrigDecision object. Based on TrigDecisionMaker.cxx
 **
 * @author Tim Martin      <Tim.Martin@cern.ch>      - University of Warwick
 **
 **   Created:      16 July 2018
 **
 **************************************************************************/

#include "TrigDecisionMakerMT.h"

#include "TrigSteeringEvent/OnlineErrorCode.h"
#include "TrigSteeringEvent/Lvl1Result.h"
#include "Lvl1ResultAccessTool.h"

#include "xAODTrigger/TrigDecisionAuxInfo.h"

#include <boost/dynamic_bitset.hpp>

TrigDec::TrigDecisionMakerMT::TrigDecisionMakerMT(const std::string &name, ISvcLocator *pSvcLocator)
    : ::AthReentrantAlgorithm(name, pSvcLocator)
{}

TrigDec::TrigDecisionMakerMT::~TrigDecisionMakerMT() {}

StatusCode
TrigDec::TrigDecisionMakerMT::initialize()
{

  bool resultObjectIsUsed = true;
  if (!m_bitsMakerTool.empty()) {
    ATH_MSG_INFO("TrigDecisionMakerMT is setting up to run after the Trigger in a job with POOL output. "
                 "The TriggerBitsMakerTool will be used directly to create the xAOD::TrigDecision.");
    ATH_CHECK( m_bitsMakerTool.retrieve() );
    resultObjectIsUsed = false;
  } else {
    ATH_MSG_INFO("TrigDecisionMakerMT is setting up to read the trigger bits from trigger bytestream. "
      "The HLTResultMT object will be used as the source of the trigger bits.");
  }
  ATH_CHECK( m_hltResultKeyIn.initialize(resultObjectIsUsed) ); // If false, this removes the ReadHandle

  ATH_CHECK( m_bgKey.initialize() );
  ATH_CHECK( m_HLTMenuKey.initialize() );

  ATH_CHECK( m_ROIBResultKeyIn.initialize() );
  ATH_CHECK( m_EventInfoKeyIn.initialize() );

  ATH_CHECK( m_trigDecisionKeyOut.initialize() );

  ATH_CHECK( m_lvl1Tool.retrieve() );

  ATH_CHECK( m_CTPResultKeyIn.initialize( m_useEDMxAOD ) );
  ATH_CHECK( m_l1ResultKeyOut.initialize( m_useEDMxAOD ) );

  return StatusCode::SUCCESS;
}

StatusCode
TrigDec::TrigDecisionMakerMT::finalize()
{
  // print out stats: used also to do regression tests
  ATH_MSG_DEBUG ("=============================================");
  ATH_MSG_DEBUG ("REGTEST Run summary:");
  ATH_MSG_DEBUG ("REGTEST  Events processed         : " << m_nEvents);
  ATH_MSG_DEBUG ("REGTEST  Level 1  : passed        = " << m_l1Passed);
  ATH_MSG_DEBUG ("REGTEST  HLT      : passed        = " << m_hltPassed);
  ATH_MSG_DEBUG ("=============================================");

  return StatusCode::SUCCESS;
}

StatusCode
TrigDec::TrigDecisionMakerMT::execute(const EventContext &context) const
{
  using namespace TrigCompositeUtils;

  // increment event counter
  m_nEvents++;

  std::unique_ptr<xAOD::TrigDecision>        trigDec    = std::make_unique<xAOD::TrigDecision>();
  std::unique_ptr<xAOD::TrigDecisionAuxInfo> trigDecAux = std::make_unique<xAOD::TrigDecisionAuxInfo>();
  trigDec->setStore(trigDecAux.get());

  SG::ReadHandle<TrigConf::HLTMenu> hltMenu(m_HLTMenuKey, context);
  ATH_CHECK(hltMenu.isValid());
  trigDec->setSMK(hltMenu->smk());

  if (m_doL1) {
    const LVL1CTP::Lvl1Result* l1Result = nullptr;
    ATH_CHECK(getL1Result(l1Result, context));

    trigDec->setTAV(l1Result->itemsAfterVeto());
    trigDec->setTAP(l1Result->itemsAfterPrescale());
    trigDec->setTBP(l1Result->itemsBeforePrescale());

    if (l1Result->isAccepted()) {
      ++m_l1Passed;
    }
  }

  if (m_doHLT) {

    boost::dynamic_bitset<uint32_t> passRawBitset, prescaledBitset;

    if (m_bitsMakerTool.isSet()) {

      ATH_MSG_DEBUG ("MC Mode: Creating bits with TriggerBitsMakerTool");
      ATH_CHECK(m_bitsMakerTool->getBits(passRawBitset, prescaledBitset, context));

    } else {

      ATH_MSG_DEBUG ("Data Mode: Reading bits from HLTResultMT");
      SG::ReadHandle<HLT::HLTResultMT> hltResult = SG::makeHandle<HLT::HLTResultMT>(m_hltResultKeyIn, context);
      ATH_CHECK(hltResult.isValid());

      passRawBitset = hltResult->getHltPassRawBits();
      prescaledBitset = hltResult->getHltPrescaledBits();

      // Inspect error codes from event header
      const std::vector<HLT::OnlineErrorCode> errorCodes = hltResult->getErrorCodes();
      bool truncated = false;
      uint32_t code = 0;
      for (size_t i = 0; i < errorCodes.size(); ++i) {
        truncated |= (errorCodes.at(i) == HLT::OnlineErrorCode::RESULT_TRUNCATION);
        if (i == 0) {
          code = static_cast<uint32_t>(errorCodes.at(0));
        }
      }

      // If the event header does not show any truncation check the individual HLT ROBs
      // to cover the case of allowed result truncation (see ATR-27986).
      //
      // We only support one truncation status in the TrigDecision. So the convention is:
      //  1) If the main HLT result is available, its status is reflected (e.g. Physics stream)
      //  2) If not, we take the combined status of all other ROBs, but in practice there
      //     should only ever be one (TLA) ROB in a given stream.
      if (!truncated) {
        std::set<uint16_t> allModuleIds;  // sorted(!) set of module IDs
        for (const auto& itr : hltResult->getSerialisedData()) allModuleIds.insert(itr.first);

        for (uint16_t moduleId : allModuleIds) {
          const std::vector<uint32_t>& status = hltResult->getRobStatus(moduleId);
          if (status.size() > 1) {  // status[0] is the generic eformat event status
            truncated |= (status[1] == static_cast<uint32_t>(HLT::OnlineErrorCode::RESULT_TRUNCATION));
            code |= status[1];
          }
          if (moduleId==0) break;
        }
      }

      trigDec->setEFErrorBits(code);
      trigDec->setEFTruncated(truncated);
    }

    ATH_MSG_DEBUG ("Number of HLT chains passed raw: " << passRawBitset.count());
    ATH_MSG_DEBUG ("Number of HLT chains prescaled out: " << prescaledBitset.count());

    if (passRawBitset.any()) {
      ++m_hltPassed;
    }

    std::vector<uint32_t> passRaw, prescaled;

    passRaw.resize(passRawBitset.num_blocks());
    prescaled.resize(prescaledBitset.num_blocks());

    boost::to_block_range(passRawBitset, passRaw.begin());
    boost::to_block_range(prescaledBitset, prescaled.begin());

    // OFFLINE FIX TO P1 OPERATIONAL ISSUE 28th-31st July 2022
    // [ATR-26036] These runs had an incomplete fix applied leading to incorrect prescale bits, prior runs in Run 3 had no presacled bits.
    // Incorrect prescale bits will cause incorrect results from the Trig Decision Tool. Hence these need to be zeroed out (no prescale bits is better than wrong prescale bits)
    static const std::set<unsigned> buggyPrescaleBitRuns = {429603, 429606, 429612, 429658, 429697, 429716};
    const bool isBuggyPrescaleBitsRun = (std::find(buggyPrescaleBitRuns.begin(), buggyPrescaleBitRuns.end(), context.eventID().run_number()) != buggyPrescaleBitRuns.end());
    if (isBuggyPrescaleBitsRun) {
      ATH_MSG_DEBUG("T0 Trigger fix for 429603-429716 is deleting incorrect isPrescaled trigger bits decoded from the bytestream.");
      for (size_t i = 0; i < prescaled.size(); ++i) {
        prescaled.at(i) = 0;
      }
    }

    if (passRaw.size() != prescaled.size()) {
      ATH_MSG_ERROR("Trigger bitsets are not all the same size! passRaw:" 
        << passRaw.size() << " prescaled:" << prescaled.size() );
      return StatusCode::FAILURE;
    } 

    // Run 3 note: with only a two-level system, with no passthrough bit, no resurrection bit
    // the raw-pass equates to the physics-pass.
    trigDec->setEFPassedRaw(passRaw);
    trigDec->setEFPassedPhysics(passRaw);
    trigDec->setEFPrescaled(prescaled);

  }

  // get the bunch crossing id
  const xAOD::EventInfo* eventInfo = SG::get(m_EventInfoKeyIn, context);
  SG::ReadCondHandle<TrigConf::L1BunchGroupSet> bgkey(m_bgKey, context);
  ATH_CHECK(bgkey.isValid());
  const TrigConf::L1BunchGroupSet* l1bgs = *bgkey;
  if (l1bgs) {
    // We currently only support 8 bits/bunchgroups (ATR-24030)
    trigDec->setBGCode( static_cast<char>(l1bgs->bgPattern(eventInfo->bcid())) );
  }
  else {
    ATH_MSG_WARNING("Could not read " << m_bgKey);
  }

  ATH_MSG_DEBUG ( "Run '" << eventInfo->runNumber()
                  << "'; Event '" << eventInfo->eventNumber()
                  << "'; BCID '" << eventInfo->bcid()
                  << "'; BG Code '" << trigDec->bgCode() << "'" ) ;

  ATH_MSG_DEBUG ("Decision object dump: " << *(trigDec.get()));
  auto trigDecWriteHandle = SG::makeHandle( m_trigDecisionKeyOut, context );
  ATH_CHECK( trigDecWriteHandle.record( std::move( trigDec ), std::move( trigDecAux ) ) );
  ATH_MSG_DEBUG ("Recorded xAOD::TrigDecision to StoreGate with key = " << m_trigDecisionKeyOut.key());

  return StatusCode::SUCCESS;
}

StatusCode
TrigDec::TrigDecisionMakerMT::getL1Result(const LVL1CTP::Lvl1Result *&result, const EventContext &context) const
{

  if (m_useEDMxAOD) {

    SG::ReadHandle<xAOD::CTPResult> ctpResult = SG::makeHandle<xAOD::CTPResult>(m_CTPResultKeyIn, context);
    ATH_CHECK(ctpResult.isValid());

    // Fill TBP, TAP, TAV when creating the Lvl1Result
    auto lvl1Result = std::make_unique<LVL1CTP::Lvl1Result>(true);

    // TBP words
    const std::vector<uint32_t> ctpTBPWords = ctpResult->getTBPWords();
    for (unsigned int iWord=0; iWord < ctpTBPWords.size(); ++iWord) {
        lvl1Result->itemsBeforePrescale().push_back(ctpTBPWords[iWord]);
        ATH_MSG_DEBUG( "TBP word #" << iWord << " is 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpTBPWords[iWord] << std::dec);
    }

    // TAP words
    const std::vector<uint32_t> ctpTAPWords = ctpResult->getTAPWords();
    for (unsigned int iWord=0; iWord < ctpTAPWords.size(); ++iWord) {
        lvl1Result->itemsAfterPrescale().push_back(ctpTAPWords[iWord]);
        ATH_MSG_DEBUG("TAP word #" << iWord << " is 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpTAPWords[iWord] << std::dec);
    }

    // TAV words
    const std::vector<uint32_t> ctpTAVWords = ctpResult->getTAVWords();
    for (unsigned int iWord = 0; iWord < ctpTAVWords.size(); ++iWord) {
        lvl1Result->itemsAfterVeto().push_back(ctpTAVWords[iWord]);
        ATH_MSG_DEBUG("TAV word #" << iWord << " is 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpTAVWords[iWord] << std::dec);
    }

    // make sure TBP, TAP, TAV all have 8 entries!
    if (lvl1Result->itemsBeforePrescale().size() < 8) lvl1Result->itemsBeforePrescale().resize(8, 0);
    if (lvl1Result->itemsAfterPrescale().size() < 8) lvl1Result->itemsAfterPrescale().resize(8, 0);
    if (lvl1Result->itemsAfterVeto().size() < 8) lvl1Result->itemsAfterVeto().resize(8, 0);

    result = static_cast<const LVL1CTP::Lvl1Result*>(lvl1Result.get());
    if (result == nullptr) {
      ATH_MSG_ERROR ( "Could not construct L1 result from xAOD::CTPResult");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(SG::makeHandle(m_l1ResultKeyOut, context).record( std::move(lvl1Result)), {});
    ATH_MSG_DEBUG ( "Built LVL1CTP::Lvl1Result from valid xAOD::CTPResult.");

  } else {

    SG::ReadHandle<ROIB::RoIBResult> roIBResult = SG::makeHandle<ROIB::RoIBResult>(m_ROIBResultKeyIn, context);
    ATH_CHECK(roIBResult.isValid());

    std::vector< std::unique_ptr<LVL1CTP::Lvl1Item> > itemConfig = m_lvl1Tool->makeLvl1ItemConfig(context);

    if (roIBResult->cTPResult().isComplete()) {
      m_lvl1Tool->createL1Items(itemConfig, *roIBResult, &result);
      ATH_MSG_DEBUG ( "Built LVL1CTP::Lvl1Result from valid ROIB::CTPResult.");
    }

    if (result == nullptr) {
      ATH_MSG_ERROR ( "Could not construct L1 result from roIBResult");
      return StatusCode::FAILURE;
    }

  }

  return StatusCode::SUCCESS;
}
