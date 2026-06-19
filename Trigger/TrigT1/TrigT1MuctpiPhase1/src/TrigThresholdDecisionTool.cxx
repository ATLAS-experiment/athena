/*                                                                                                                      
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigThresholdDecisionTool.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"


namespace LVL1
{
  namespace MURoIThresholdsToolParams {
    const char ContainerName[] = "LVL1MuonRoIs";
    const char ThresholdType[] = "MU";
  }

  TrigThresholdDecisionTool::TrigThresholdDecisionTool(const std::string& type, 
						       const std::string& name, 
						       const IInterface* parent)
    : base_class(type, name, parent) {}
  
  StatusCode TrigThresholdDecisionTool::initialize()
  {
    ATH_MSG_DEBUG( "========================================" );
    ATH_MSG_DEBUG( "Initialize for TrigThresholdDecisionTool"  );
    ATH_MSG_DEBUG( "========================================" );

    ATH_CHECK(RoIThresholdsTool::initialize());
    ATH_CHECK( m_rpcTool.retrieve() );
    ATH_CHECK( m_tgcTool.retrieve() );

    if(m_MenuFromxAOD) {
        ATH_CHECK( m_configSvc.retrieve() );
    }
    return StatusCode::SUCCESS;
  }
  
  StatusCode TrigThresholdDecisionTool::start()
  {
    ATH_MSG_DEBUG( "==========================================" );
    ATH_MSG_DEBUG( "Start for Phase1 TrigThresholdDecisionTool"  );
    ATH_MSG_DEBUG( "==========================================" );

    // we configure the tool here only if we are not running from xAOD
    if (!m_MenuFromxAOD){
        
        SG::ReadHandle<TrigConf::L1Menu> l1Menu = SG::makeHandle(m_l1MenuKey);
        ATH_CHECK(l1Menu.isValid());
        ATH_CHECK(configureToolFromMenu(*l1Menu));
    }
    return StatusCode::SUCCESS;
  }
  
StatusCode TrigThresholdDecisionTool::configureToolFromMenu(const TrigConf::L1Menu& l1Menu) const {

    std::lock_guard guard{m_mutex};
    if (m_isInitialized) {
        return StatusCode::SUCCESS;
    }

    //buffered parsed TGC/RPC flags
    parsedFlagsMap parsed_flags;
    m_tgcFlag_decisions.clear();
    m_rpcFlag_decisions.clear();

    //front-load the TGC flag parsing and all possible 3-bit decisions for the menu
    std::optional<ThrVecRef> menuThresholds = getMenuThresholds(l1Menu);
    ATH_CHECK(menuThresholds.has_value());

    for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds.value().get()) {
        auto thr = static_cast<TrigConf::L1Threshold_MU*>(thrBase.get());

        //parse the tgc flags and buffer them
        std::string tgcFlags = getShapedFlags( thr->tgcFlags() );
        parseFlags(tgcFlags, parsed_flags);

        //loop over all 3-bit flag combinations
        for (unsigned flags=0;flags<8;flags++) {
            bool F=flags&0b100;
            bool C=flags&0b010;
            bool H=flags&0b001;
            makeTGCDecision(tgcFlags, F, C, H, parsed_flags);
        }

        //parse the rpc flags and buffer them
        std::string rpcFlags = getShapedFlags( thr->rpcFlags() );
        parseFlags(rpcFlags, parsed_flags);

        //loop over all 2-bit flag combinations
        for (unsigned flags=0;flags<2;flags++){
            bool M=flags&0b1;
            makeRPCDecision(rpcFlags, M, parsed_flags);
        }
    }
    m_isInitialized = true;
    return StatusCode::SUCCESS;
}

  uint64_t TrigThresholdDecisionTool::getPattern(const EventContext& /*ctx*/,
                                                 const xAOD::MuonRoI& roi,
                                                 const ThrVec& menuThresholds,
                                                 const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {
    return getPattern(roi.roiWord(), menuThresholds, menuExtraInfo);
  }

  uint64_t TrigThresholdDecisionTool::getPattern(uint32_t dataWord,
                                                 const ThrVec& menuThresholds,
                                                 const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {
    if (m_MenuFromxAOD and !m_isInitialized){
        const TrigConf::L1Menu& l1Menu = m_configSvc->l1Menu( Gaudi::Hive::currentContext());
        if (configureToolFromMenu(l1Menu) != StatusCode::SUCCESS){
            throw std::runtime_error("Error configuring the TrigThresholdDecisionTool from metadata!");
        }
    }

    uint64_t thresholdsPattern = 0;

    //first figure out if we need to use the RPC or TGC tool for decoding the ROI
    LVL1::ITrigT1MuonRecRoiTool::MuonTriggerSystem system = m_rpcTool->getSystem(dataWord);
    const LVL1::ITrigT1MuonRecRoiTool* roiTool;
    if (system == LVL1::ITrigT1MuonRecRoiTool::Barrel) roiTool = &(*m_rpcTool);
    else roiTool = &(*m_tgcTool);

    //buffer the some information
    unsigned isub = roiTool->getBitMaskValue(&dataWord, roiTool->SubSysIDMask());
    LVL1MUONIF::Lvl1MuCTPIInputPhase1::MuonSubSystem side = static_cast<LVL1MUONIF::Lvl1MuCTPIInputPhase1::MuonSubSystem>(isub);
    unsigned ptword = roiTool->getBitMaskValue(&dataWord, roiTool->ThresholdMask());
    unsigned roi, sectorID;
    if (system == LVL1::ITrigT1MuonRecRoiTool::Barrel) {
      roi      = roiTool->getBitMaskValue(&dataWord, roiTool->BarrelRoIMask());
      sectorID = roiTool->getBitMaskValue(&dataWord, roiTool->BarrelSectorIDMask());
    } else if (system == LVL1::ITrigT1MuonRecRoiTool::Endcap) {
      roi      = roiTool->getBitMaskValue(&dataWord, roiTool->EndcapRoIMask());
      sectorID = roiTool->getBitMaskValue(&dataWord, roiTool->EndcapSectorIDMask());
    } else { // Forward
      roi      = roiTool->getBitMaskValue(&dataWord, roiTool->ForwardRoIMask());
      sectorID = roiTool->getBitMaskValue(&dataWord, roiTool->ForwardSectorIDMask());
    }
    const TrigConf::L1ThrExtraInfo_MU& muThrExtraInfo = dynamic_cast<const TrigConf::L1ThrExtraInfo_MU&>(menuExtraInfo);

    //buffer (notional) TGC/RPC flags
    bool F=false, C=false, H=false, M=false;
    if (system == LVL1::ITrigT1MuonRecRoiTool::Barrel)
    {
      M  = dataWord & roiTool->OverflowPerRoIMask();
    }
    else
    {
      F = dataWord & roiTool->BW2Or3Mask();
      C = dataWord & roiTool->InnerCoinMask();
      H = dataWord & roiTool->GoodMFMask();
    }

    //loop over the thresholds
    for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
      auto thr = static_cast<TrigConf::L1Threshold_MU*>(thrBase.get());

      bool passed{false};
      if (system == LVL1::ITrigT1MuonRecRoiTool::Barrel) {
        //skip the threshold with regions not corresponding to ALL or barrel
        if (thr->region().find("ALL") == std::string::npos &&
            thr->region().find("BA") == std::string::npos) continue;

        //veto this candidate from this multiplicity if it's part of the excluded ROI list
        const bool isSideC = (side == LVL1MUONIF::Lvl1MuCTPIInputPhase1::idSideC());
        if (isExcludedRPCROI(muThrExtraInfo, thr->rpcExclROIList(), roi, sectorID, isSideC)) continue;

        if (ptword >= thr->idxBarrel()) {
          // mark this threshold as passed
          passed = true;
        }

        passed &= getRPCDecision(getShapedFlags(thr->rpcFlags()), M);
      }
      else { // Endcap or Forward
        if (system == LVL1MUONIF::Lvl1MuCTPIInputPhase1::idEndcapSystem()) { // Endcap
          //skip the threshold with regions not corresponding to ALL or endcap
          if (thr->region().find("ALL") == std::string::npos &&
              thr->region().find("EC") == std::string::npos) continue;

          if (ptword >= thr->idxEndcap()) {
            // mark this threshold as passed
            passed = true;
          }
        }
        else { // Forward
          //skip the threshold with regions not corresponding to ALL or forward
          if (thr->region().find("ALL") == std::string::npos &&
              thr->region().find("FW") == std::string::npos) continue;

          if (ptword >= thr->idxForward()) {
            // mark this threshold as passed
            passed = true;
          }
        }

        passed &= getTGCDecision(getShapedFlags(thr->tgcFlags()), F, C, H);
      } // end Endcap or Forward

      if (passed) {
        // set the corresponding bit in the pattern
        thresholdsPattern |= (1ull << thr->mapping());
      }

    } // loop over thresholds

    return thresholdsPattern;
  }

  std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >
  TrigThresholdDecisionTool::getThresholdDecisions(uint32_t dataWord,
                                                   const EventContext& eventContext) const {
    // Retrieve the L1 menu configuration
    const TrigConf::L1Menu* l1Menu;
    if (m_MenuFromxAOD){
        l1Menu = &m_configSvc->l1Menu( eventContext );
        if (!m_isInitialized){
            if (configureToolFromMenu(*l1Menu) != StatusCode::SUCCESS){
                throw std::runtime_error("Error configuring the TrigThresholdDecisionTool from metadata!");
            }
        } 
    }
    else{
        SG::ReadHandle<TrigConf::L1Menu> l1MenuHandle = SG::makeHandle(m_l1MenuKey, eventContext);
        l1Menu = l1MenuHandle.cptr();
    }

    std::optional<ThrVecRef> menuThresholds = getMenuThresholds(*l1Menu);
    std::optional<ExtraInfoRef> menuExtraInfo = getMenuThresholdExtraInfo(*l1Menu);
    // Call the other overload
    return getThresholdDecisions(dataWord, menuThresholds.value().get(), menuExtraInfo.value().get());
  }

  std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >
  TrigThresholdDecisionTool::getThresholdDecisions(uint32_t dataWord,
                                                   const ThrVec& menuThresholds,
                                                   const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const {
    if (m_MenuFromxAOD and !m_isInitialized){
        const TrigConf::L1Menu& l1Menu = m_configSvc->l1Menu( Gaudi::Hive::currentContext());
        if (configureToolFromMenu(l1Menu) != StatusCode::SUCCESS){
            throw std::runtime_error("Error configuring the TrigThresholdDecisionTool from metadata!");
        }
    }

    const uint64_t pattern = getPattern(dataWord, menuThresholds, menuExtraInfo);

    //the object that will be returned: pairs of thresholds and pass/fail decisions
    std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> > threshold_decisions;
    threshold_decisions.resize(menuThresholds.size());
    for (const std::shared_ptr<TrigConf::L1Threshold>& thr : menuThresholds) {
      const bool decision = pattern & (1 << thr->mapping());
      threshold_decisions[thr->mapping()] = std::make_pair(thr, decision);
    }
    return threshold_decisions;
  }

  std::pair<std::string, double> TrigThresholdDecisionTool::getMinThresholdNameAndValue(const std::vector<std::pair<std::shared_ptr<TrigConf::L1Threshold>, bool> >& decisions, const double& eta) const
  { 
    if (m_MenuFromxAOD and !m_isInitialized){
        const TrigConf::L1Menu& l1Menu = m_configSvc->l1Menu( Gaudi::Hive::currentContext());
        if (configureToolFromMenu(l1Menu) != StatusCode::SUCCESS){
            throw std::runtime_error("Error configuring the TrigThresholdDecisionTool from metadata!");
        }
    }

    //find the highest pt threshold passed - depite the name of this function
    std::string thrName="";
    double thrVal=0;
    double thrValTmp=0;
    for (unsigned idec=0;idec<decisions.size();++idec) {
      if (!decisions[idec].second) continue;
      const TrigConf::L1Threshold_MU* thr = static_cast<TrigConf::L1Threshold_MU*>(decisions[idec].first.get());
      if(std::abs(eta)<1.05){
	  thrValTmp = thr->ptBarrel();
      }
      else{
	 thrValTmp = thr->ptEndcap();
      }
      if (thrValTmp > thrVal)
	{
	  thrVal = thrValTmp;
	  thrName = thr->name();
	}
    }
    return std::make_pair(std::move(thrName), thrVal);
  }

  bool TrigThresholdDecisionTool::isExcludedRPCROI(const TrigConf::L1ThrExtraInfo_MU& menuExtraInfo,
                                                   const std::string& rpcExclROIList,
                                                   const unsigned roi,
                                                   const unsigned sectorID,
                                                   const bool isSideC) const {
    if (!rpcExclROIList.empty())
    {
      const std::map<std::string, std::vector<unsigned int> >& exclList = menuExtraInfo.exclusionList(rpcExclROIList);
      if (exclList.size() != 0)
      {
	//build the sector name of this ROI to compare against the exclusion list
	std::string sectorName("B");
	int sectorNumber=sectorID;
	if (isSideC) sectorNumber += 32;
	if (sectorNumber < 10) sectorName += '0';
	sectorName += std::to_string(sectorNumber);
	
	//do the comparison
	auto exclROIs = exclList.find(sectorName);
	if (exclROIs != exclList.end())
	{
	  for (auto roi_itr=exclROIs->second.begin();roi_itr!=exclROIs->second.end();roi_itr++)
	  {
	    if (*roi_itr == roi) return true;
	  }
	}
      }
    } // rpcExclList != ""

    return false;
  }

bool TrigThresholdDecisionTool::getTGCDecision(const std::string& tgcFlags, const bool F, const bool C, const bool H) const
{
  auto it = m_tgcFlag_decisions.find(tgcFlags);
  if (it == m_tgcFlag_decisions.end()) return false;

  return it->second.isPassed(F, C, H);
}

void TrigThresholdDecisionTool::makeTGCDecision(const std::string& tgcFlags, const bool F, const bool C, const bool H, const parsedFlagsMap& parsed_flags) const
{
  // If no flags are specified, it automatically passes quality checks
  if (tgcFlags.empty()) {
    m_tgcFlag_decisions[tgcFlags].setPassed(F, C, H);
    return;
  }

  // Check the quality based on the flags
  bool passedFlags = false;
  const auto* vec_flags = &parsed_flags.at(tgcFlags);
  for (auto or_itr = vec_flags->begin(); or_itr != vec_flags->end(); or_itr++)
  {
    bool passedAnd = true;
    for (auto and_itr = or_itr->begin(); and_itr != or_itr->end(); and_itr++)
    {
      if (*and_itr == "F") passedAnd = passedAnd && F;
      else if (*and_itr == "C") passedAnd = passedAnd && C;
      else if (*and_itr == "H") passedAnd = passedAnd && H;
    }
    passedFlags = passedFlags || passedAnd;
  }

  // Use the struct helper to register a passing combination
  if (passedFlags) {
    m_tgcFlag_decisions[tgcFlags].setPassed(F, C, H);
  }
}

bool TrigThresholdDecisionTool::getRPCDecision(const std::string& rpcFlags, const bool M) const
{
  auto it = m_rpcFlag_decisions.find(rpcFlags);
  if (it == m_rpcFlag_decisions.end()) return false;

  return it->second.isPassed(M);;
}

void TrigThresholdDecisionTool::makeRPCDecision(const std::string& rpcFlags, const bool M, const parsedFlagsMap& parsed_flags) const
{

  if (rpcFlags.empty()) {
    m_rpcFlag_decisions[rpcFlags].setPassed(M);
    return;
  }

  bool passedFlags = false;
  const auto* vec_flags = &parsed_flags.at(rpcFlags);
  for (auto or_itr = vec_flags->begin(); or_itr != vec_flags->end(); or_itr++)
  {
    bool passedAnd = true;
    for (auto and_itr = or_itr->begin(); and_itr != or_itr->end(); and_itr++)
    {
      if (*and_itr == "M") passedAnd = passedAnd && M;
    }
    passedFlags = passedFlags || passedAnd;
  }
  if (passedFlags) {
    m_rpcFlag_decisions[rpcFlags].setPassed(M);
  }
}

  void TrigThresholdDecisionTool::parseFlags(const std::string& flags, parsedFlagsMap& parsed_flags) const
  {
    //parse the logic of the quality flag into a 2D vector, where outer layer contains the logic |'s and inner layer contains the logical &'s.
    //save the 2D vector in a map so we don't have to parse it each time we want to check the flags.
    // 1. Single-lookup insertion check using try_emplace.
    // If the key exists, it returns immediately without doing any work.
    auto [it, inserted] = parsed_flags.try_emplace(flags);
    
    if (!inserted) {
        return; 
    }

    // Grab a stable string_view referencing the map's internal persistent key
    // This ensures the views remain valid as long as the key is in the map.
    std::string_view stable_key = it->first;
    std::vector<std::vector<std::string_view>> vec_flags;

    // 2. C++20 lazy splitting for OR tokens
    for (auto or_chunk : stable_key | std::views::split('|')) {
        std::vector<std::string_view> and_tokens;
        
        // 3. C++20 lazy splitting for AND tokens
        for (auto and_chunk : or_chunk | std::views::split('&')) {
            // Reconstruct a string_view from the subrange without copying the underlying characters
            and_tokens.emplace_back(and_chunk.begin(), and_chunk.end());
        }
        
        vec_flags.push_back(std::move(and_tokens));
    }

    // Assign the completely parsed zero-allocation views back to the value slot
    it->second = std::move(vec_flags);
  }

  
std::string TrigThresholdDecisionTool::getShapedFlags( std::string_view flags) {
    if (flags.empty()) return {};

    // Helper lambda to strip spaces from a string_view range
    auto drop_spaces = std::views::filter([](char c) { return !std::isspace(static_cast<unsigned char>(c)); });

    // 1. Split by OR ('|') using ranges
    auto or_splits = flags | std::views::split('|');
    std::vector<std::string> unique_ors;

    for (auto or_chunk : or_splits) {
        // 2. Split by AND ('&') 
        auto and_splits = or_chunk | std::views::split('&');
        std::vector<std::string_view> and_tokens;

        for (auto and_chunk : and_splits) {
            // Reconstruct a clean string_view without whitespace bounds
            auto clean_view = and_chunk | drop_spaces;
            
            // Converting a filtered range back to a contiguous string_view if needed.
            // If sub-strings might have internal spaces, we need to instantiate a small string, 
            // but if it's just stripping padding, we can grab the underlying bounds.
            std::string token;
            std::ranges::copy(clean_view, std::back_inserter(token));
            if (!token.empty()) {
                and_tokens.push_back(std::move(token)); 
            }
        }

        if (and_tokens.empty()) continue;

        // Deduplicate and sort AND tokens
        std::ranges::sort(and_tokens);
        auto [last, end] = std::ranges::unique(and_tokens);
        and_tokens.erase(last, end);

        // Reconstruct the normalized AND group
        std::string stabilized_and;
        for (size_t i = 0; i < and_tokens.size(); ++i) {
            stabilized_and += and_tokens[i];
            if (i < and_tokens.size() - 1) stabilized_and += '&';
        }
        unique_ors.push_back(std::move(stabilized_and));
    }

    // Deduplicate and sort OR groups
    std::ranges::sort(unique_ors);
    auto [or_last, or_end] = std::ranges::unique(unique_ors);
    unique_ors.erase(or_last, or_end);

    // 3. Final Assembly
    std::string result;
    for (size_t i = 0; i < unique_ors.size(); ++i) {
        result += unique_ors[i];
        if (i < unique_ors.size() - 1) result += '|';
    }

    return result;
}

}