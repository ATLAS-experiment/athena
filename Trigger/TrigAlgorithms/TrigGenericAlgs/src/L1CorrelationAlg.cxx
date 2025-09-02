/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "L1CorrelationAlg.h"
#include "CTPfragment/CTPfragment.h"
#include "CTPfragment/CTPdataformat.h"

#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/WriteDecorHandle.h"

L1CorrelationAlg::L1CorrelationAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode L1CorrelationAlg::initialize() {

  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_CHECK(m_trigCompositeKey.initialize());
  ATH_CHECK(m_l1MenuKey.initialize());
  ATH_CHECK(m_passKey.initialize());
  ATH_CHECK(m_l1AKey.initialize());
  ATH_CHECK(m_otherTypeKey.initialize());
  ATH_CHECK(m_beforeAfterKey.initialize());
  ATH_CHECK(m_otherTypeBeforeKey.initialize());
  ATH_CHECK(m_otherTypeAfterKey.initialize());
  ATH_CHECK(m_beforeOffsetKey.initialize());
  ATH_CHECK(m_afterOffsetKey.initialize());
  ATH_CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode L1CorrelationAlg::start(){

  //get L1 menu and initialise bitMasks based on l1 em/j/mu triggers
  SG::ReadHandle<TrigConf::L1Menu> rh_l1Menu = SG::makeHandle(m_l1MenuKey);
  ATH_CHECK(rh_l1Menu.isValid());

  std::vector<int> ctpids_ele;
  std::vector<int> ctpids_mu;
  std::vector<int> ctpids_jets;
  std::vector<int> ctpids;

  for(const TrigConf::L1Item& item : *rh_l1Menu){
    for(unsigned int nl1=0; nl1<m_l1itemlist.size(); nl1++){
      if(m_l1itemlist[nl1].compare(item.name()) == 0 ){
        ATH_MSG_DEBUG("L1CorrAlgInit: Configured to use item:" <<item.name().c_str()<< " CTPID:"<<item.ctpId());
        ctpids.push_back( item.ctpId() );
        if( m_l1itemlist[nl1].find( "L1_EM" )  != std::string::npos ){
          ctpids_ele.push_back( item.ctpId() );
        }
        if( m_l1itemlist[nl1].find( "L1_MU" )  != std::string::npos ){
          ctpids_mu.push_back( item.ctpId() );
        }
        if((  m_l1itemlist[nl1].find( "L1_J")  != std::string::npos ) || ( m_l1itemlist[nl1].find( "L1_jJ" )  != std::string::npos)){
          ctpids_jets.push_back( item.ctpId() );
        }
      }
    }
  }
    
  // init with 0
  m_bitmasks.clear();
  m_bitmasks_ele.clear();
  m_bitmasks_mu.clear();
  m_bitmasks_jets.clear();
  for(int n=0; n<16 ; n++){
    uint32_t tmpword=0;
    m_bitmasks.push_back(tmpword);
    m_bitmasks_ele.push_back(tmpword);
    m_bitmasks_mu.push_back(tmpword);
    m_bitmasks_jets.push_back(tmpword);
  }

  //all ctpids
  for(unsigned int n=0; n<ctpids.size()  ; n++){
    // check in which word it belongs
    int cycle =  ctpids[n] / 32;
    int pos = ctpids[n] % 32;
     
    uint32_t currentmask = m_bitmasks[cycle];
    uint32_t tmpmask = 1;
    // shift this pos positions to left
    tmpmask = tmpmask << (pos);
      
    // OR this with the existing mask
    m_bitmasks[cycle] = tmpmask | currentmask;
  }

  //electrons
  for(unsigned int n=0; n<ctpids_ele.size()  ; n++){
    // check in which word it belongs
    int cycle =  ctpids_ele[n] / 32;
    int pos = ctpids_ele[n] % 32;
     
    uint32_t currentmask = m_bitmasks_ele[cycle];
    uint32_t tmpmask = 1;
    // shift this pos positions to left
    tmpmask = tmpmask << (pos);
     
    // OR this with the existing mask
    m_bitmasks_ele[cycle] = tmpmask | currentmask;
  }

  //muons
  for(unsigned int n=0; n<ctpids_mu.size()  ; n++){
    // check in which word it belongs
    int cycle =  ctpids_mu[n] / 32;
    int pos = ctpids_mu[n] % 32;
    
    uint32_t currentmask = m_bitmasks_mu[cycle];
    uint32_t tmpmask = 1;
    // shift this pos positions to left
    tmpmask = tmpmask << (pos);
    
    // OR this with the existing mask
    m_bitmasks_mu[cycle] = tmpmask | currentmask;
  }  

  //jets
  for(unsigned int n=0; n<ctpids_jets.size()  ; n++){
    // check in which word it belongs
    int cycle =  ctpids_jets[n] / 32;
    int pos = ctpids_jets[n] % 32;
    
    uint32_t currentmask = m_bitmasks_jets[cycle];
    uint32_t tmpmask = 1;
    // shift this pos positions to left
    tmpmask = tmpmask << (pos);
     
    // OR this with the existing mask
    m_bitmasks_jets[cycle] = tmpmask | currentmask;
  }  

  return StatusCode::SUCCESS;
}


StatusCode L1CorrelationAlg::execute(const EventContext& ctx) const {

  //TrigComposite Container to record
  SG::WriteHandle<xAOD::TrigCompositeContainer> wh_trigComposite(m_trigCompositeKey, ctx);
  ATH_CHECK(wh_trigComposite.record(std::make_unique<xAOD::TrigCompositeContainer>(), std::make_unique<xAOD::TrigCompositeAuxContainer>()));
  auto trigCompCont = wh_trigComposite.ptr();

  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompL1A(m_l1AKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompOther(m_otherTypeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompBeforeAfter(m_beforeAfterKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompPass(m_passKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompOtherBefore(m_otherTypeBeforeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompOtherAfter(m_otherTypeAfterKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompBeforeOffset(m_beforeOffsetKey, ctx);
  SG::WriteDecorHandle<xAOD::TrigCompositeContainer, int> trigCompAfterOffset(m_afterOffsetKey, ctx);

  //CTP ROB
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> robFragments;
  std::vector<uint32_t> roblist;
  // magic number!
  roblist.push_back(0x770000); 
  m_robDataProviderSvc->addROBData(ctx, roblist);
  m_robDataProviderSvc->getROBData(ctx, roblist, robFragments);
    if (msgLvl(MSG::DEBUG)) {
    std::ostringstream os;
    for(auto rob : roblist){
      os << std::hex<<rob;
    }
    ATH_MSG_DEBUG(roblist.size() << "/" << robFragments.size()
		 << " ROBs requested/retrieved:" << os.str());
  }
  if (robFragments.size()<1){
    ATH_MSG_DEBUG("Could not retrieve ROB!");
    return StatusCode::SUCCESS;
  }
  const eformat::ROBFragment<const uint32_t*>* rbf = robFragments[0];

  // L1A index within the CTP readout window (not a BCID).
  // For a symmetric window nBC = 2*k + 1, L1A sits at index k:
  // e.g. ±1 → nBC=3 → l1a_idx=1; ±2 → nBC=5 → l1a_idx=2.
  const int l1a_idx = static_cast<int>(CTPfragment::lvl1AcceptBunch(rbf)); // index in readout window

  const int nBC = 2 * l1a_idx + 1; // symmetric by requirement
  const int mid = l1a_idx;         // central element

  // Cache TBP/TAP once per BC
  std::vector<std::vector<uint32_t>> tbpWords(nBC), tapWords(nBC);
  for (int i = 0; i < nBC; ++i) {
    const unsigned k = static_cast<unsigned>(i);
    tbpWords[i] = CTPfragment::triggerDecisionBeforePrescales(rbf, k);
    tapWords[i] = CTPfragment::triggerDecisionAfterPrescales (rbf, k);
  }

  // Helper: any-bit-on under mask
  auto firedFromWords = [&](const std::vector<uint32_t>& w,
                            const std::vector<uint32_t>& mask) -> uint8_t {
    const std::size_t nw = std::min(w.size(), mask.size()); // (16 by design)
    uint8_t on = 0;
    for (std::size_t iw = 0; iw < nw; ++iw) on |= static_cast<uint8_t>((w[iw] & mask[iw]) != 0u);
    return on;
  };

  // Per-BC fired flags (TBP)
  std::vector<uint8_t> firedbc(nBC,0), firedbc_ele(nBC,0), firedbc_mu(nBC,0), firedbc_jet(nBC,0);
  for (int i = 0; i < nBC; ++i) {
    firedbc    [i] = firedFromWords(tbpWords[i], m_bitmasks);
    firedbc_ele[i] = firedFromWords(tbpWords[i], m_bitmasks_ele);
    firedbc_mu [i] = firedFromWords(tbpWords[i], m_bitmasks_mu );
    firedbc_jet[i] = firedFromWords(tbpWords[i], m_bitmasks_jets);
  }

  // Central-BC inclusion flag
  if (m_currentBCincl) {
    firedbc[mid] = firedbc_ele[mid] = firedbc_mu[mid] = firedbc_jet[mid] = 1;
  }

  // Occupancy maps (delta BC, CTPID) for TBP/TAP
  auto collect_ids_from_words = [this](const std::vector<uint32_t>& words) {
    std::vector<int> ids;
    const std::size_t nw = std::min(words.size(), this->m_bitmasks.size()); // typically 16
    ids.reserve(32u * nw);
    for (std::size_t iw = 0; iw < nw; ++iw) {
      uint32_t w = words[iw];
      if (!w) continue;
      const unsigned base = static_cast<unsigned>(iw * 32u);
      for (unsigned b = 0; b < 32; ++b) if (w & (1u << b)) ids.emplace_back(static_cast<int>(base + b));
    }
    return ids;
  };

  std::vector<int> vDeltaBC_all_TBP, vCtpId_all_TBP, vDeltaBC_all_TAP, vCtpId_all_TAP;
  vDeltaBC_all_TBP.reserve(64); vCtpId_all_TBP.reserve(64);
  vDeltaBC_all_TAP.reserve(64); vCtpId_all_TAP.reserve(64);

  for (int i = 0; i < nBC; ++i) {
    const int delta = i - mid;
    {
      const auto ids = collect_ids_from_words(tbpWords[i]);
      vDeltaBC_all_TBP.insert(vDeltaBC_all_TBP.end(), ids.size(), delta);
      vCtpId_all_TBP.insert (vCtpId_all_TBP.end(),  ids.begin(), ids.end());
    }
    {
      const auto ids = collect_ids_from_words(tapWords[i]);
      vDeltaBC_all_TAP.insert(vDeltaBC_all_TAP.end(), ids.size(), delta);
      vCtpId_all_TAP.insert (vCtpId_all_TAP.end(),  ids.begin(), ids.end());
    }
  }

  // Pass–fail decision: nearest neighbour on each side
  int isPassed = 0;
  int beforeafterflag = 0; // sign: side; abs: offset
  int offset_before = 0, offset_after = 0;
  for (int d = 1; d <= l1a_idx; ++d) {
    if (firedbc[mid] && firedbc[mid - d] && offset_before == 0) { offset_before = d; isPassed = 1; }
    if (firedbc[mid] && firedbc[mid + d] && offset_after  == 0) { offset_after  = d; isPassed = 1; }
    if (offset_before && offset_after) break;
  }

  // Classification (legacy 1..7)
  auto classify = [&](int idx)->int {
    if      (firedbc_ele[idx] && !firedbc_mu[idx] && !firedbc_jet[idx]) return 1;
    else if (firedbc_mu [idx] && !firedbc_ele[idx] && !firedbc_jet[idx]) return 2;
    else if (firedbc_jet[idx] && !firedbc_ele[idx] && !firedbc_mu[idx])  return 3;
    else if (firedbc_ele[idx] &&  firedbc_mu[idx] && !firedbc_jet[idx])  return 4;
    else if (firedbc_ele[idx] &&  firedbc_jet[idx] && !firedbc_mu[idx])  return 5;
    else if (firedbc_mu [idx] &&  firedbc_jet[idx] && !firedbc_ele[idx]) return 6;
    else if (firedbc_ele[idx] &&  firedbc_mu[idx] &&  firedbc_jet[idx])  return 7;
    return 0;
  };

  int l1a_type = classify(mid);
  int other_type_before = (offset_before > 0) ? classify(mid - offset_before) : 0;
  int other_type_after  = (offset_after  > 0) ? classify(mid + offset_after ) : 0;

  // Choose operative side
  if (offset_before > 0 && offset_after > 0) {
    if (offset_before < offset_after) beforeafterflag = -offset_before;
    else if (offset_after < offset_before) beforeafterflag = +offset_after;
    else beforeafterflag = +offset_after; // tie → positive side
  } else if (offset_before > 0) {
    beforeafterflag = -offset_before;
  } else if (offset_after > 0) {
    beforeafterflag = +offset_after;
  }

  int other_type = 0;
  if      (beforeafterflag < 0) other_type = other_type_before;
  else if (beforeafterflag > 0) other_type = other_type_after;

  // Create TrigComposite only if passed
  if (isPassed) {
    auto trigComp = new xAOD::TrigComposite();
    trigCompCont->push_back(trigComp);
    trigComp->setName("mistimemon_L1Dec");

    trigCompL1A         (*trigComp) = l1a_type;
    trigCompOther       (*trigComp) = other_type;
    trigCompBeforeAfter (*trigComp) = beforeafterflag;
    trigCompPass        (*trigComp) = isPassed;
    trigCompOtherBefore (*trigComp) = other_type_before;
    trigCompOtherAfter  (*trigComp) = other_type_after;
    trigCompBeforeOffset(*trigComp) = offset_before;
    trigCompAfterOffset (*trigComp) = offset_after;
  }

  // Monitoring: global summary + side-specific
  auto mon_l1a         = Monitored::Scalar<int>("l1Accept", l1a_type);
  auto mon_otherType   = Monitored::Scalar<int>("otherType", other_type);
  auto mon_beforeAfter = Monitored::Scalar<int>("BeforeAfterFlag", beforeafterflag);
  auto monitorIt = Monitored::Group(m_monTool, mon_l1a, mon_otherType, mon_beforeAfter);

  if (offset_before > 0 && offset_after > 0) {
    auto mon_offsetBefore = Monitored::Scalar<int>("BeforeOffset", offset_before);
    auto mon_offsetAfter  = Monitored::Scalar<int>("AfterOffset",  offset_after);
    auto mon_otherBefore  = Monitored::Scalar<int>("OtherTypeBefore", other_type_before);
    auto mon_otherAfter   = Monitored::Scalar<int>("OtherTypeAfter",  other_type_after);
    auto gBoth = Monitored::Group(m_monTool, mon_offsetBefore, mon_offsetAfter, mon_otherBefore, mon_otherAfter);
    (void)gBoth;
  } else if (offset_before > 0) {
    auto mon_offsetBefore = Monitored::Scalar<int>("BeforeOffset", offset_before);
    auto mon_otherBefore  = Monitored::Scalar<int>("OtherTypeBefore", other_type_before);
    auto gB = Monitored::Group(m_monTool, mon_offsetBefore, mon_otherBefore);
    (void)gB;
  } else if (offset_after > 0) {
    auto mon_offsetAfter = Monitored::Scalar<int>("AfterOffset",  offset_after);
    auto mon_otherAfter  = Monitored::Scalar<int>("OtherTypeAfter",  other_type_after);
    auto gA = Monitored::Group(m_monTool, mon_offsetAfter, mon_otherAfter);
    (void)gA;
  }

  // Full occupancy maps (TBP/TAP)
  auto mon_dbc_all_tbp = Monitored::Collection("DeltaBCAll",      vDeltaBC_all_TBP);
  auto mon_id_all_tbp  = Monitored::Collection("CTPIDAll",        vCtpId_all_TBP);
  auto mon_dbc_all_tap = Monitored::Collection("DeltaBCAll_TAP",  vDeltaBC_all_TAP);
  auto mon_id_all_tap  = Monitored::Collection("CTPIDAll_TAP",    vCtpId_all_TAP);
  auto gMaps = Monitored::Group(m_monTool, mon_dbc_all_tbp, mon_id_all_tbp,
                                            mon_dbc_all_tap, mon_id_all_tap);
  (void)gMaps;

  // Pair maps at abs(delta) = 1,2,3... (using cached words)
  const auto ids0_tbp = collect_ids_from_words(tbpWords[mid]);
  const auto ids0_tap = collect_ids_from_words(tapWords[mid]);

  auto fill_pair_map = [&](int deltaSel, bool useTAP,
                           const char* varX, const char* varY) {
    const int kN = mid + deltaSel;
    if (kN < 0 || kN >= nBC) return;

    const auto idsN = useTAP ? collect_ids_from_words(tapWords[kN])
                             : collect_ids_from_words(tbpWords[kN]);
    const auto& ids0 = useTAP ? ids0_tap : ids0_tbp;

    if (ids0.empty() || idsN.empty()) return;

    std::vector<int> vId0; vId0.reserve(ids0.size() * idsN.size());
    std::vector<int> vIdN; vIdN.reserve(vId0.capacity());
    for (int id0 : ids0) for (int idN : idsN) { vId0.emplace_back(id0); vIdN.emplace_back(idN); }

    auto mon_x = Monitored::Collection(varX, vId0);
    auto mon_y = Monitored::Collection(varY, vIdN);
    auto g     = Monitored::Group(m_monTool, mon_x, mon_y);
    (void)g;
  };

  // TBP: delta = +2, −2, +1, −1
  fill_pair_map(+2, false, "CTPID0tbp_p2", "CTPIDtbp_p2");
  fill_pair_map(-2, false, "CTPID0tbp_m2", "CTPIDtbp_m2");
  fill_pair_map(+1, false, "CTPID0tbp_p1", "CTPIDtbp_p1");
  fill_pair_map(-1, false, "CTPID0tbp_m1", "CTPIDtbp_m1");

  // TAP: delta = +2, −2, +1, −1
  fill_pair_map(+2, true , "CTPID0tap_p2", "CTPIDtap_p2");
  fill_pair_map(-2, true , "CTPID0tap_m2", "CTPIDtap_m2");
  fill_pair_map(+1, true , "CTPID0tap_p1", "CTPIDtap_p1");
  fill_pair_map(-1, true , "CTPID0tap_m1", "CTPIDtap_m1");

  return StatusCode::SUCCESS;
}