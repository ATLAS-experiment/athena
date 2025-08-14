/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSignatureMoni.h"

#include "AthenaInterprocess/Incidents.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include <algorithm>
#include <regex>
#include <format>

/// Default bin numbers
enum BINS {
  INPUT = 1,
  AFTER_PS = 2,
  OUTPUT = 3,
  EXPRESS = 4,
  N_BINS = 4
};


StatusCode TrigSignatureMoni::initialize() {
  ATH_CHECK(m_l1DecisionsKey.initialize());
  ATH_CHECK(m_finalDecisionKey.initialize());
  ATH_CHECK(m_HLTMenuKey.initialize());
  ATH_CHECK(m_L1MenuKey.initialize());
  ATH_CHECK(m_decisionCollectorTools.retrieve());
  ATH_CHECK(m_featureCollectorTools.retrieve());
  ATH_CHECK(m_histSvc.retrieve());

  ATH_CHECK(m_incidentSvc.retrieve());
  m_incidentSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::start() {
  SG::ReadHandle<TrigConf::L1Menu> l1MenuHandle = SG::makeHandle(m_L1MenuKey);
  SG::ReadHandle<TrigConf::HLTMenu> hltMenuHandle = SG::makeHandle(m_HLTMenuKey);
  ATH_CHECK(hltMenuHandle.isValid());

  // Retrieve chain information from menus
  m_groupToChainMap.clear();
  m_streamToChainMap.clear();
  m_expressChainMap.clear();

  std::unordered_map<std::string,std::string> mapStrNameToTypeName; // e.g. {Main -> physics_Main}
  try {
    for (const TrigConf::DataStructure& stream : hltMenuHandle->streams()) {
      mapStrNameToTypeName.insert({stream.getAttribute("name"), stream.getAttribute("type")+"_"+stream.getAttribute("name")});
    }
  } catch (const std::exception& ex) {
    ATH_MSG_ERROR("Exception reading stream tag configuration from the HLT menu: " << ex.what());
    return StatusCode::FAILURE;
  }

  for (const TrigConf::Chain& chain : *hltMenuHandle) {
    for (const std::string& group : chain.groups()) {
      // Save chains per RATE group
      if (group.starts_with( "RATE")){
        m_groupToChainMap[group].insert(HLT::Identifier(chain.name()));
      }
    }

    // Save chain to stream map
    for (const std::string& streamName : chain.streams()){
      const auto it = mapStrNameToTypeName.find(streamName);
      if (it==mapStrNameToTypeName.cend()) {
        ATH_MSG_ERROR("Stream name " << streamName << " assigned to chain " << chain.name()
                      << " is missing from menu streams definition");
        return StatusCode::FAILURE;
      }
      if (it->second == "express_express") {
        m_expressChainMap[it->second].insert(HLT::Identifier(chain.name()));
      } else {
        m_streamToChainMap[it->second].insert(HLT::Identifier(chain.name()));
      }
    }
  }

  // Prepare the histograms

  // Initialize SignatureAcceptance and DecisionCount histograms that will monitor 
  //    chains, groups and sequences per each step
  const int x {nBinsX(hltMenuHandle)};
  const int y {nSteps()};
  ATH_MSG_DEBUG( "Histogram " << x << " x " << y << " bins");
  std::unique_ptr<TH2> hSA = std::make_unique<TH2I>("SignatureAcceptance", "Raw acceptance of signatures in;chain;step", x, 1, x + 1, y, 1, y + 1);  
  std::unique_ptr<TH2> hDC = std::make_unique<TH2I>("DecisionCount", "Positive decisions count per step;chain;step", x, 1, x + 1, y, 1, y + 1);

  ATH_CHECK(m_histSvc->regShared(m_bookingPath + "/" + name() + "/SignatureAcceptance", std::move(hSA), m_passHistogram));
  ATH_CHECK(m_histSvc->regShared(m_bookingPath + "/" + name() + "/DecisionCount", std::move(hDC), m_countHistogram));

  ATH_CHECK(initHist(m_passHistogram, hltMenuHandle));
  ATH_CHECK(initHist(m_countHistogram, hltMenuHandle));

  // Initialize Rate histogram to save the rates of positive decisions in given interval 
  //  per chain/group/sequence per in, after ps, out steps
  if ( x > 0 ){
    const std::string outputRateName = std::format("Rate{:d}s", m_duration.value());
    m_rateHistogram.init(outputRateName, "Rate of positive decisions;chain;step", x, N_BINS,
                         std::format("{}/{}/{}", m_bookingPath.value(), name(), outputRateName), m_histSvc).ignore();
    ATH_CHECK(initHist(m_rateHistogram.getHistogram(), hltMenuHandle, false));
    ATH_CHECK(initHist(m_rateHistogram.getBuffer(), hltMenuHandle, false));
  }

  // Initialize SequencesExecutionRate histogram to save the rates of sequences execution
  // Save sequences names to be monitored
  std::set<std::string> sequencesSet;
  for (auto& ctool: m_decisionCollectorTools){
    ctool->getSequencesNames(sequencesSet);
  }
  const int xc = sequencesSet.size();
  const int yc {1}; // Only rate, this histogram is really 1 D
  if (xc > 0){
    const std::string outputSequenceName = std::format("SequencesExecutionRate{:d}s", m_duration.value());
    m_sequenceHistogram.init(outputSequenceName, "Rate of sequences execution;sequence;rate", xc, yc,
                             std::format("{}/{}/{}", m_bookingPath.value(), name(), outputSequenceName), m_histSvc).ignore();

    ATH_CHECK(initSeqHist(m_sequenceHistogram.getHistogram(), sequencesSet));
    ATH_CHECK(initSeqHist(m_sequenceHistogram.getBuffer(), sequencesSet));
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::stop() {
  m_rateHistogram.stopTimer();
  m_sequenceHistogram.stopTimer();

  if (m_chainIDToBinMap.empty()) {
    ATH_MSG_INFO( "No chains configured, no counts to print" );
    return StatusCode::SUCCESS;
  }
  
  SG::ReadHandle<TrigConf::HLTMenu> hltMenuHandle = SG::makeHandle(m_HLTMenuKey);
  ATH_CHECK(hltMenuHandle.isValid());

  // Retrieve information whether chain was active in Step
  std::unordered_map<std::string, std::set<int>> chainToStepsId;
  for (const TrigConf::Chain& chain : *hltMenuHandle){
    int nstep {1}; // Start from step=1
    for (const std::string& seqName : chain.sequencers()){
      // Example sequencer name is "Step1_FastCalo_electron", we need only information about Step + number
      std::smatch stepNameMatch;
      std::regex_search(seqName.begin(), seqName.end(), stepNameMatch, std::regex("[Ss]tep[0-9]+"));

      std::string stepName = stepNameMatch[0];
      stepName[0] = std::toupper(stepName[0]); // Fix "stepX" -> "StepX"
      // Check that the step name is set with the same position in the execution (empty steps support)
      if (std::format("Step{:d}", nstep) == stepName) {
        chainToStepsId[chain.name()].insert(nstep);
      } else {
      	ATH_MSG_DEBUG("Missing counts for step" << nstep << " in chain " << chain.name());
      }
      nstep++;
    }
  }

  auto collToString = [&](int xbin, const LockedHandle<TH2>& hist, int startOfset=0, int endOffset=0){ 
    std::string v;
    const int stepsSize = hist->GetYaxis()->GetNbins() - N_BINS;
    for (int ybin = 1; ybin <= hist->GetYaxis()->GetNbins()-endOffset; ++ybin) {
      if (ybin > startOfset) {
        // Skip steps where chain wasn't active
        // ybins are for all axis labes, steps are in bins from 3 to stepsSize + 2
        const std::string chainName = m_passHistogram->GetXaxis()->GetBinLabel(xbin);

        if (ybin < 3 || ybin > stepsSize + 2 || chainToStepsId[chainName].contains(ybin - 2)) {
          v += std::format("{:<11d}", static_cast<int>(hist->GetBinContent(xbin, ybin)));
        } else {
          v += std::format("{:<11s}", "-");
        }
      } else {
        v += std::format("{:<11s}", " ");
      }
    }
    return v;
  };
  
  std::string v;
  v += std::format("{:<11s}", "L1");
  v += std::format("{:<11s}", "AfterPS");
  for (int bin = 1; bin <= m_passHistogram->GetYaxis()->GetNbins()-N_BINS; ++bin) {
    v += std::format("Step{:<7d}", bin);
  }
  v += std::format("{:<11s}", "Output");
  v += std::format("{:<11s}", "Express");
  
  ATH_MSG_INFO("Chains passing step (1st row events & 2nd row decision counts):");  
  ATH_MSG_INFO(std::format("{:<30s}", "ChainName") << v);

  /*
    comment for future dev:
    for combined chains we find x2 the number of decisions, because we count both the HypoAlg and the combo Alg decisions
  */
  
  for (int bin = 1; bin <= (*m_passHistogram)->GetXaxis()->GetNbins(); ++bin) {
    const std::string chainName = m_passHistogram->GetXaxis()->GetBinLabel(bin);
    const std::string chainID = std::to_string(HLT::Identifier(chainName));
    if (chainName.starts_with( "HLT")) { // print only for chains
      ATH_MSG_INFO( std::format("{:s} #{:s}", chainName, chainID) );
      ATH_MSG_INFO( std::format("{:<30s}", std::format("-- #{} Events", chainID)) << collToString( bin, m_passHistogram) );
      ATH_MSG_INFO( std::format("{:<30s}", std::format("-- #{} Features", chainID)) << collToString( bin, m_countHistogram , 2, 1 ) );
    }
    if (chainName.starts_with( "All")){
      ATH_MSG_INFO( std::format("{:<30s}", chainName) << collToString( bin, m_passHistogram) );
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::fillHistogram(const TrigCompositeUtils::DecisionIDContainer& dc, int row, LockedHandle<TH2>& histogram) const {
  for (TrigCompositeUtils::DecisionID id : dc)  {
    auto id2bin = m_chainIDToBinMap.find( id );
     if ( id2bin == m_chainIDToBinMap.end() && HLT::Identifier(id).name().find("leg") != 0 ) {
      ATH_MSG_WARNING( "HLT chain " << HLT::Identifier(id) << " not configured to be monitored" );
    } else {
      histogram->Fill( id2bin->second, double(row) );
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::fillRate(const TrigCompositeUtils::DecisionIDContainer& dc, int row) const {
  return fillHistogram(dc, row, m_rateHistogram.getBuffer());
}

StatusCode TrigSignatureMoni::fillPassEvents(const TrigCompositeUtils::DecisionIDContainer& dc, int row) const {
  return fillHistogram(dc, row, m_passHistogram);
}

StatusCode TrigSignatureMoni::fillDecisionCount(const std::vector<TrigCompositeUtils::DecisionID>& dc, int row) const {
  for (TrigCompositeUtils::DecisionID id : dc)  {
    TrigCompositeUtils::DecisionID chain = id;
    if (TrigCompositeUtils::isLegId(HLT::Identifier(id)) ) chain = TrigCompositeUtils::getIDFromLeg(id);
    auto id2bin = m_chainIDToBinMap.find( chain );
    if ( id2bin == m_chainIDToBinMap.end()) {    
      ATH_MSG_WARNING("HLT chain " << HLT::Identifier(chain) << " not configured to be monitored");
    } else {
      m_countHistogram->Fill(id2bin->second, static_cast<double>(row));
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::fillSequences(const std::set<std::string>& sequences) const {
  for (const std::string& seq : sequences) {
    m_sequenceHistogram.fill(m_sequenceToBinMap.at(seq), 1);
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::fillStreamsAndGroups(const std::map<std::string, TrigCompositeUtils::DecisionIDContainer>& nameToChainsMap, const TrigCompositeUtils::DecisionIDContainer& dc) const {
  const int countOutputRow {nSteps()-1};
  for (const auto& [name, decisions] : nameToChainsMap) {
    for (TrigCompositeUtils::DecisionID id : dc) {
      if (decisions.contains(id)) {
        const double bin = m_nameToBinMap.at(name);
        m_countHistogram->Fill(bin, countOutputRow);
        m_rateHistogram.fill(bin, OUTPUT);
        m_passHistogram->Fill(bin, countOutputRow);
        break;
      }
    }
  }
  return StatusCode::SUCCESS;
}

void TrigSignatureMoni::handle( const Incident& incident ) {
  // Create and start timer after fork
  if (incident.type() == AthenaInterprocess::UpdateAfterFork::type()) {
    if (m_rateHistogram.getTimer() || m_sequenceHistogram.getTimer()) {
      ATH_MSG_WARNING("Timer is already running. UpdateAfterFork incident called more than once?");
      return;
    }

    // Prevent from publishing empty histograms
    if (nBinsX() > 0) {
      m_rateHistogram.startTimer(m_duration, m_intervals);
    }
    
    if (!m_sequenceToBinMap.empty()) {
      m_sequenceHistogram.startTimer(m_duration, m_intervals);
    }    
    
    ATH_MSG_DEBUG("Started rate timer");
  }
}

StatusCode TrigSignatureMoni::execute( const EventContext& context ) const {

  SG::ReadHandle<TrigCompositeUtils::DecisionContainer> l1Decisions = SG::makeHandle(m_l1DecisionsKey, context);

  [[maybe_unused]] static const bool sanityCheckDone = [&] {
    if (l1Decisions->at(INPUT-1)->name() == "l1seeded" &&
        l1Decisions->at(AFTER_PS-1)->name() == "unprescaled") {
      return true;
    }
    throw GaudiException(m_l1DecisionsKey.key() + " does not contain the expected entries",
                         name(), StatusCode::FAILURE);
  }();

  auto fillL1 = [&](int index) -> StatusCode {    
    TrigCompositeUtils::DecisionIDContainer ids;    
    TrigCompositeUtils::decisionIDs(l1Decisions->at(index-1), ids);
    ATH_MSG_DEBUG( "L1 " << index-1 << " N positive decisions " << ids.size()  );
    ATH_CHECK(fillPassEvents(ids, index));
    ATH_CHECK(fillRate(ids, index));
    if (!ids.empty()){
      m_passHistogram->Fill(1, static_cast<double>(index));
      m_rateHistogram.fill(1, static_cast<double>(index));
    }
    return StatusCode::SUCCESS;
  };

  // Fill histograms with L1 decisions in and after prescale
  ATH_CHECK(fillL1(INPUT));
  ATH_CHECK(fillL1(AFTER_PS));

  // Fill HLT steps
  int step = 0;
  std::vector<TrigCompositeUtils::DecisionID> stepSum;
  std::set<std::string> stepSequences;
  for ( auto& ctool: m_decisionCollectorTools ) {
    ctool->getDecisions( stepSum, stepSequences, context );
    ATH_MSG_DEBUG( " Step " << step << " decisions (for decisions): " << stepSum.size() );
    TrigCompositeUtils::DecisionIDContainer stepUniqueSum( stepSum.begin(), stepSum.end() );
    ATH_CHECK( fillPassEvents( stepUniqueSum, 3+step ) );
    ATH_CHECK( fillSequences( stepSequences ) );
    ++step;
    stepSum.clear();
    stepSequences.clear();
  }

  step = 0;
  for ( auto& ctool: m_featureCollectorTools ) {
    stepSum.clear();
    ctool->getDecisions( stepSum, context );
    ATH_MSG_DEBUG( " Step " << step << " decisions (for features): " << stepSum.size() );
    ATH_CHECK( fillDecisionCount( stepSum, 3+step ) );
    ++step;
  }

  // Collect the final decisions
  SG::ReadHandle<TrigCompositeUtils::DecisionContainer> finalDecisionsHandle = SG::makeHandle( m_finalDecisionKey, context );
  ATH_CHECK( finalDecisionsHandle.isValid() );
  TrigCompositeUtils::DecisionIDContainer finalIDs;
  const TrigCompositeUtils::Decision* decisionObject = TrigCompositeUtils::getTerminusNode(*finalDecisionsHandle);
  if (!decisionObject) {
    ATH_MSG_WARNING("Unable to locate trigger navigation terminus node. Cannot tell which chains passed the event.");
  } else {
    TrigCompositeUtils::decisionIDs(decisionObject, finalIDs);
  }

  // Collect the express stream decisions
  TrigCompositeUtils::DecisionIDContainer expressFinalIDs;
  const TrigCompositeUtils::Decision* expressDecisionObject = TrigCompositeUtils::getExpressTerminusNode(*finalDecisionsHandle);
  if (!expressDecisionObject) {
    ATH_MSG_WARNING("Unable to locate trigger navigation express terminus node. Cannot tell which chains passed the express stream in this event.");
  } else {
    TrigCompositeUtils::decisionIDs(expressDecisionObject, expressFinalIDs);
  }

  // Fill the histograms with output counts/rate
  const int countOutputRow {nSteps()-1};
  ATH_CHECK( fillStreamsAndGroups(m_streamToChainMap, finalIDs));
  ATH_CHECK( fillStreamsAndGroups(m_groupToChainMap, finalIDs));
  ATH_CHECK( fillPassEvents(finalIDs, countOutputRow));
  ATH_CHECK( fillRate(finalIDs, OUTPUT));

  // Fill the histograms with express counts/rate
  const int countExpressRow {nSteps()};
  ATH_CHECK( fillStreamsAndGroups(m_expressChainMap, expressFinalIDs));
  ATH_CHECK( fillPassEvents(expressFinalIDs, countExpressRow));
  ATH_CHECK( fillRate(expressFinalIDs, EXPRESS));

  // Fill the "All" column in counts/rate histograms
  if (!finalIDs.empty()) {
    m_passHistogram->Fill(1, static_cast<double>(countOutputRow));
    m_rateHistogram.fill(1, static_cast<double>(OUTPUT));
  }
  if (!expressFinalIDs.empty()) {
    m_passHistogram->Fill(1, static_cast<double>(countExpressRow));
    m_rateHistogram.fill(1, static_cast<double>(EXPRESS));
  }

  return StatusCode::SUCCESS;
}

int TrigSignatureMoni::nBinsX(SG::ReadHandle<TrigConf::HLTMenu>& hltMenuHandle) const {
  return nChains(hltMenuHandle) + m_groupToChainMap.size() + m_streamToChainMap.size() + m_expressChainMap.size();
}

int TrigSignatureMoni::nBinsX() const {
  return m_chainIDToBinMap.size() + m_groupToChainMap.size() + m_streamToChainMap.size() + m_expressChainMap.size() + 1;
}

int TrigSignatureMoni::nChains(SG::ReadHandle<TrigConf::HLTMenu>& hltMenuHandle) const {
  return hltMenuHandle->size() + 1; // Chains + "All"
}

int TrigSignatureMoni::nSteps() const {
  return m_decisionCollectorTools.size() + N_BINS;
}

StatusCode TrigSignatureMoni::initHist(LockedHandle<TH2>& hist, SG::ReadHandle<TrigConf::HLTMenu>& hltMenuHandle, bool steps) {
  TAxis* x = hist->GetXaxis();
  x->SetBinLabel(1, "All");
  int bin = 2; // 1 is for total count, (remember bin numbering in ROOT starts from 1)

  std::set<std::string> sortedChainsList;
  for ( const TrigConf::Chain& chain: *hltMenuHandle ) {
    sortedChainsList.insert( chain.name() );
  }
  
  for ( const std::string& chainName: sortedChainsList ) {
    x->SetBinLabel( bin, chainName.c_str() );
    m_chainIDToBinMap[ HLT::Identifier( chainName ).numeric() ] = bin;
    bin++;
  }

 
  for ( const auto& stream : m_streamToChainMap){
    x->SetBinLabel( bin, ("str_"+stream.first).c_str());
    m_nameToBinMap[ stream.first ] = bin;
    bin++;
  }

  for ( const auto& stream : m_expressChainMap){
    x->SetBinLabel( bin, ("str_"+stream.first).c_str());
    m_nameToBinMap[ stream.first ] = bin;
    bin++;
  }

  for ( const auto& group : m_groupToChainMap){
    x->SetBinLabel( bin, ("grp_"+group.first.substr(group.first.find(':')+1)).c_str() );
    m_nameToBinMap[ group.first ] = bin;
    bin++;
  }


  TAxis* y = hist->GetYaxis();
  y->SetBinLabel(INPUT, steps ? "L1" : "Input");
  y->SetBinLabel(AFTER_PS, "AfterPS");
  for ( size_t i = 0; steps && i < m_decisionCollectorTools.size(); ++i) {
    y->SetBinLabel(3 + i, std::format("Step {:d}", i).c_str());
  }
  y->SetBinLabel(y->GetNbins()-1, "Output"); // Second to last bin
  y->SetBinLabel(y->GetNbins(), "Express"); // Last bin

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoni::initSeqHist(LockedHandle<TH2>& hist, std::set<std::string>& sequenceSet) {
  TAxis* x = hist->GetXaxis();
  int bin = 1;

  // Set bin labels
  for (const std::string& seqName : sequenceSet) {
    x->SetBinLabel(bin, seqName.c_str());
    m_sequenceToBinMap[seqName] = bin;
    ++bin;
  }

  TAxis* y = hist->GetYaxis();
  y->SetBinLabel(1, "Rate");

  return StatusCode::SUCCESS;
}


TrigSignatureMoni::RateHistogram::~RateHistogram(){
  delete m_bufferHistogram.get();
}

StatusCode TrigSignatureMoni::RateHistogram::init( const std::string& histoName, const std::string& histoTitle,
  const int x, const int y, const std::string& registerPath, const ServiceHandle<ITHistSvc>& histSvc ){
  std::unique_ptr<TH2> h = std::make_unique<TH2F>(histoName.c_str(), histoTitle.c_str(), x, 1, x + 1, y, 1, y + 1);
  ATH_CHECK( histSvc->regShared( registerPath.c_str(), std::move(h), m_histogram));
  
  TH2I * hB = new TH2I( (histoName + "Buffer").c_str(), histoTitle.c_str(), x, 1, x + 1, y, 1, y + 1);
  m_bufferHistogram.set(hB, &m_mutex);
  m_bufferHistogram->SetDirectory(0);

  return StatusCode::SUCCESS;
}

LockedHandle<TH2> & TrigSignatureMoni::RateHistogram::getHistogram ATLAS_NOT_CONST_THREAD_SAFE () const {
  return m_histogram;
}

LockedHandle<TH2> & TrigSignatureMoni::RateHistogram::getBuffer ATLAS_NOT_CONST_THREAD_SAFE () const {
  return m_bufferHistogram;
}

std::unique_ptr<Athena::AlgorithmTimer> & TrigSignatureMoni::RateHistogram::getTimer() {
  return m_timer;
}

void TrigSignatureMoni::RateHistogram::fill(const double x, const double y) const {
  m_bufferHistogram->Fill(x, y);
}

void TrigSignatureMoni::RateHistogram::startTimer(unsigned int duration, unsigned int intervals) {
  m_duration = duration;
  m_timeDivider = std::make_unique<TimeDivider>(intervals, duration, TimeDivider::seconds);
  m_timer = std::make_unique<Athena::AlgorithmTimer>(duration*50, std::bind(&RateHistogram::callback, this));
}

void TrigSignatureMoni::RateHistogram::stopTimer() {
  if (m_timer) {
    m_timer->stop();
    time_t t = time(0);
    unsigned int interval;
    unsigned int duration = m_timeDivider->forcePassed(t, interval);
    updatePublished(duration); // Divide by time that really passed not by interval duration
  }

}

void TrigSignatureMoni::RateHistogram::updatePublished(unsigned int duration) const {
  m_histogram->Reset("ICES");
  m_histogram->Add(m_bufferHistogram.get(), 1./duration);
  m_bufferHistogram->Reset("ICES");
}


void TrigSignatureMoni::RateHistogram::callback() {
  // Ask time divider if we need to switch to new interval
  time_t t = time(0);
  unsigned int newinterval;
  unsigned int oldinterval;

  if (m_timeDivider->isPassed(t, newinterval, oldinterval)) {
    updatePublished(m_duration);
  }

  // Schedule itself in another 1/20 of the integration period in milliseconds
  if (m_timer) m_timer->start(m_duration*50);
}
