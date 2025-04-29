/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoRatesCalculator.h"
#include <xAODEgamma/ElectronContainer.h>
#include "TrigConfData/L1Connector.h"
#include <tuple>
#include "TH1.h"
#include "GaudiKernel/ITHistSvc.h"
#include <nlohmann/json.hpp>

L1TopoRatesCalculator::L1TopoRatesCalculator( const std::string& name, ISvcLocator* pSvcLocator ) : RatesAnalysisAlg(name, pSvcLocator) {
}

StatusCode L1TopoRatesCalculator::initialize() {

  ATH_MSG_ALWAYS("Initializing L1TopoRatesCalculator");

  ATH_CHECK( RatesAnalysisAlg::initialize() ); // Initialise my parent class first

  ATH_CHECK( m_trigDecisionKey.initialize() ); // Then initialise my fields afterwards
  
  ATH_CHECK(m_l1topoKey.initialize()); //L1Topo

  //Accessing L1Menu----------------------------------------------
 
  const TrigConf::L1Menu * l1menu = nullptr;
  ATH_CHECK( detStore()->retrieve(l1menu)); //detStore() returns m_detStore. detStore() from AthCommonDataStore.h

  for (size_t i = 0; i < m_userDefinedNames.value().size(); ++i) {
	  m_userDefinedMap[m_userDefinedNames.value()[i]] = m_userDefinedDefinitions.value()[i];
  }
  for (const std::string& L1_item : m_L1_items_json) {
	bool found = false;
	for (const TrigConf::L1Item& L1item : *l1menu) {
		const std::string& L1name = L1item.name();
		if (L1name == L1_item){
			const std::string& L1definition = L1item.definition();
			m_L1_item_definitions.push_back(L1definition);
			m_L1_items.push_back(L1_item);
			found = true;
			break;
		}
	}
	if (!found) {
		ATH_MSG_DEBUG("Warning: L1 item '" << L1_item << "' not found in the L1 menu!");
	}	
  }

  for (const auto& pair : m_userDefinedMap) {
  	m_L1_items.push_back(pair.first);
	m_L1_item_definitions.push_back(pair.second);
  }
  // Filling trigger map --------------
  
  std::map<std::string, TriggerInfo> triggerMap;
  for (size_t i = 0; i < m_L1_items.size(); ++i){
    TriggerInfo info;

    // Regular expression to capture the name of the trigger and the number in square brackets
    std::regex re(R"(([\w\d_-]+)\[x(\d+)\])");
    std::sregex_iterator iter((m_L1_item_definitions[i]).begin(), (m_L1_item_definitions[i]).end(), re);
    std::sregex_iterator end;
    std::vector<size_t> triggerPositions;

    // Extract all triggers and their positions
    while (iter != end) {
        std::smatch match = *iter;
        std::string triggerName = match.str(1);
        std::string triggerCount = match.str(2);
        info.triggers.push_back(triggerCount + triggerName);

        // Store the position of the end of the trigger in the original chain to identify the operations between them
        triggerPositions.push_back(match.position(0) + match.length(0));
        ++iter;
    }
    // Capturing operations using the positions between triggers
    size_t prevPos = 0;
    for (size_t j = 0; j < triggerPositions.size(); ++j) {
        size_t pos = triggerPositions[j];
        std::string part = (m_L1_item_definitions[i]).substr(prevPos, pos - prevPos);

        // Eliminate unnecessary spaces 
        part.erase(std::remove_if(part.begin(), part.end(),
                                  [](unsigned char c) { return std::isspace(c); }),
                   part.end());

        // Si la parte contiene un operador &, |, guardamos esa operación
        if (part.find("&") != std::string::npos) {
            info.operations.push_back("&");
        } else if (part.find("|") != std::string::npos) {
            info.operations.push_back("|");
        }

        prevPos = pos;
    }
    // Handling of final operation if there is only one trigger
    std::string lastPart = (m_L1_item_definitions[i]).substr(prevPos);
    lastPart.erase(std::remove_if(lastPart.begin(), lastPart.end(),
                                  [](unsigned char c) { return std::isspace(c); }),
                   lastPart.end());	
    triggerMap[m_L1_items[i]] = info;
  }

  m_triggerMap = triggerMap;
  //--------------------------------
  for (const auto& pair : triggerMap) {
        ATH_MSG_DEBUG("Key: " << pair.first);
        for (size_t i = 0; i < pair.second.triggers.size(); ++i) {
            ATH_MSG_DEBUG("Trigger " << i + 1 << ": " << pair.second.triggers[i]);
            if (i < pair.second.operations.size()) {
                ATH_MSG_DEBUG("Operation: " << pair.second.operations[i]);
            }
        }
    }
 
  // Listing the triggers needed for the L1 items (w/o the multiplicities)-----------
  
  std::vector<std::string> beforeCTP_triggers;
  std::vector<std::string> beforeCTP_triggers_mult; 
 
  for (const auto& pair : triggerMap) {
        for (size_t i = 0; i < pair.second.triggers.size(); ++i) {
		beforeCTP_triggers_mult.push_back(pair.second.triggers[i]);
		size_t pos_number = 0;
		while (pos_number < (pair.second.triggers[i]).size() && std::isdigit((pair.second.triggers[i])[pos_number])) {
        		++pos_number;
		}
		beforeCTP_triggers.push_back((pair.second.triggers[i]).substr(pos_number));
	}
  }

  std::unordered_set<std::string> seen_mult;
  std::vector<std::string> beforeCTP_triggers_mult_unique;
  
  for (const auto& str_mult : beforeCTP_triggers_mult) {
    auto [it, inserted] = seen_mult.insert(str_mult);
    if (inserted) {
        beforeCTP_triggers_mult_unique.push_back(str_mult);
    }
  }

  std::sort(beforeCTP_triggers.begin(), beforeCTP_triggers.end());  
  auto beforeCTP_triggers_unique = std::unique(beforeCTP_triggers.begin(), beforeCTP_triggers.end());  
  beforeCTP_triggers.erase(beforeCTP_triggers_unique, beforeCTP_triggers.end());  

  m_beforeCTP_triggers = beforeCTP_triggers;
  m_beforeCTP_triggers_mult = beforeCTP_triggers_mult_unique;

  // --------------------------------
  
  for (size_t i = 0; i < beforeCTP_triggers.size(); ++i){	
      ATH_MSG_DEBUG("Filling L1menu parameters from item: " << beforeCTP_triggers[i]);
      ResultDefinition definition;
      definition.flatindex = 0;
      definition.clock = 0;
      definition.nBit = 0;
      definition.fromSim = true;
      definition.overflow = true;
      ATH_MSG_DEBUG("Item being analyzed: " << beforeCTP_triggers[i]);
      for( const auto & connName : l1menu->connectorNames() ) {
          auto & conn = l1menu->connector(connName);
          for( auto & tl : conn.triggerLines() ) {
              //For electrical connectors from the L1Topo boards a triggerline vector holds up to 16 signals
              if ((connName == "Topo2El") || (connName == "Topo3El")){
                  for (size_t fpga = 0; fpga < 2; ++fpga){ //run over fpgas
                      for (size_t clock = 0; clock < 2; ++clock){ //run over clocks
                          for (auto & tl : conn.triggerLines(fpga,clock)){
                              if (tl.name() == beforeCTP_triggers[i]){
                                  definition.flatindex = tl.flatindex();
                                  definition.nBit = tl.nbits();
                                  definition.clock = tl.clock();
                                  if (connName == "Topo2El") definition.conID = 2;
                                  if (connName == "Topo3El") definition.conID = 3;
                              }
                              else{
                                  continue;
                              }
                          }
                      }
                  }
              }else{
                  if (tl.name() == beforeCTP_triggers[i]){
                      definition.flatindex = tl.flatindex();
                      definition.nBit = tl.nbits();
                      definition.clock = 0;
                      if (connName == "Topo1Opt0") definition.conID = 4;
		      if (connName == "Topo1Opt1") definition.conID = 5;
                      if (connName == "Topo1Opt2") definition.conID = 6;
                      if (connName == "Topo1Opt3") definition.conID = 7;
                  }else{
                      continue;
                  }
              }
          }
      }
      m_definitions.push_back(definition);	
  }
   
  //----------------------------------------------------- 
  return StatusCode::SUCCESS;
}

StatusCode L1TopoRatesCalculator::ratesInitialize() {
  ATH_MSG_DEBUG("In ratesInitialize()");
  
  if (m_doHistograms){
    ATH_MSG_DEBUG("################## Registering rates matrix:");
    m_ratesMatrixHist = new TH2D("rates_matrix","L1item Rates matrix",150,-3,3,150,-3,3);
    ATH_CHECK( histSvc()->regHist("/RATESTREAM/rates_matrix", m_ratesMatrixHist) );

  } 
  // Here we assume a full-ring, other functions are available to change this assumption.
  // @see setTargetLumiMu(const double lumi, const double mu);
  // @see setTargetLumiBunches(const double lumi, const int32_t bunches);
  // @see setTargetMuBunches(const double mu, const int32_t bunches);
  setTargetLumi( m_lumi );
 
 //-------------------
 
  // Define triggers to emulate
  // TDT can be used instead by ATH_CHECK(addAllExisting());

  // name, prescale, expressPrescale, seedName, seedPrescale, groups
  std::set<std::string> triggerGroup {"RATE_SingleElectron"};

  // Initialize rates matrix of zeros and right size ----------------------------------------
 
  std::vector<double> vector_zeros(m_L1_items.size(), 0);
  for (size_t i = 0; i < m_L1_items.size(); ++i){
          m_rates_matrix.push_back(vector_zeros);
          m_rates_matrix2.push_back(vector_zeros);

  }
  // Set labels of Rates matrix 

  for (size_t i = 1; i <= m_L1_items.size(); ++i){
        int j = i-1;
        m_ratesMatrixHist ->GetXaxis()->SetBinLabel(i, m_L1_items[j].c_str());
        m_ratesMatrixHist ->GetYaxis()->SetBinLabel(i, m_L1_items[j].c_str());
  }  
  //-----------------------------------------------------------------------------------------
  ATH_MSG_ALWAYS("Add Existing");

  for (const std::string& L1_item : m_L1_items_json) {
	ATH_CHECK( addExisting(L1_item));
  }

  return StatusCode::SUCCESS;
}

uint32_t L1TopoRatesCalculator::L1TopoSimResultsContainer_decoder(const L1TopoRatesCalculator::ResultDefinition& definition, SG::ReadHandle<xAOD::L1TopoSimResultsContainer>& cont) {
// Decoding L1TopoSimResults

  uint32_t resultValue = 999;

  if (definition.fromSim) {
        std::vector<uint32_t>result_vector(2,0);
	xAOD::L1TopoSimResultsContainer const* resultCont = cont.cptr();
	for (const xAOD::L1TopoSimResults* result : *resultCont) {
		long long topoWordPrint = result->topoWord64();
		std::bitset<64> wordPrint(topoWordPrint);
          	if (result->connectionId() != definition.conID) continue;

          	std::vector<uint32_t> connectorWords(4); //4 x 32 = 128 bits should be enough
		unsigned int bitWidth = result->bitWidth();
		long long topoWord64 = result->topoWord64();
		std::bitset<64> word(topoWord64);
		if (bitWidth==32) { 
			ATH_MSG_DEBUG("Electrical connector ");
			
 			long long topoWord = result->topoWord();
			std::bitset<32> word(topoWord);
                 	if (definition.overflow == 1){
                                long long topoWordOverflow = result->topoWordOverflow();

                                std::bitset<32> wordRes(topoWord);
                                std::bitset<32> wordOver(topoWordOverflow);
                                word = wordRes | wordOver;
                        }
			connectorWords[result->clock()] = static_cast<uint32_t>(word.to_ullong());
          	} else { 
                 	ATH_MSG_DEBUG("Optical connector ");
                 	
                 	if (definition.overflow == 1){
				long long topoWord64Overflow = result->topoWord64Overflow();

                	        std::bitset<64> wordRes(topoWord64);
        	                std::bitset<64> wordOver(topoWord64Overflow);
                        	word = wordRes | wordOver;
			}
        	        // May be used to have enough space for the 96 bits on optical connectors 
                	connectorWords[2*result->clock() + 0] = static_cast<uint32_t>(word.to_ullong()); //first 32 bits 
                 	connectorWords[2*result->clock() + 1] = static_cast<uint32_t>(word.to_ullong() >> 32); //second 32 bits
		}

          	//startOffset: for extraction of HW results to account for two fibers worth of data in one readout "TOB" block

          	unsigned int startOffset = 0;

          	if (result->connectionId() == 5 || result->connectionId() == 7){ // if Topo1Opt1 or Topo1Opt3
                	startOffset = 96;
          	}else{ // All other connectors
                	startOffset = 0;
          	}
	  	resultValue = extractResult(connectorWords, definition, startOffset);
		result_vector[result->clock()] = resultValue; //Saving the result from the loop over result->clock
  	}
  
  if ((result_vector[0]>1)||(result_vector[1]>1)){
  	if (result_vector[0]>1) resultValue = result_vector[0];
	if (result_vector[1]>1) resultValue = result_vector[1];
  }else{
  	resultValue = result_vector[0] || result_vector[1]; //OR between the results from the two result->clock iterations
  }

  return resultValue;

  }else{
        ATH_MSG_ERROR("definition.fromSim set to false");	
  	return 999;
  }
}


// Decoding L1Topo item (fromSim = true)
uint32_t L1TopoRatesCalculator::extractResult(const std::vector<uint32_t>& connectorContents, const L1TopoRatesCalculator::ResultDefinition& definition, unsigned int startOffset) {
   
   uint32_t result = 0; 
   unsigned int startindex = definition.flatindex + startOffset; //for optical clock is 0, for electrical account for structure of argument
   unsigned int endindex   = startindex + definition.nBit - 1;
   unsigned int firstWord  = startindex / 32; //integer division on purpose
   unsigned int lastWord   = endindex   / 32; //integer division on purpose
   unsigned int nBitAdded = 0;
   uint32_t word = 0; //buffer

   if ((firstWord>1) | (lastWord>1)){
	   firstWord = firstWord%3;
   	   lastWord = lastWord%3;
   }

   std::vector<uint32_t>result_vec(lastWord,0);
   uint32_t mask = 0;
   
   for (unsigned int wordIndex=firstWord; wordIndex <= lastWord; wordIndex++) {
      unsigned int startPosInWord = (wordIndex == firstWord) ? (startindex % 32) : 0 ;
      unsigned int endPosInWord   = (wordIndex == lastWord)  ? (endindex % 32)   : 31;
      // might be %3 (3words/fiber)
      mask =  ( ( 1u<< (endPosInWord+1) ) - 1 );
      word = connectorContents[wordIndex] & mask;
      word >>= startPosInWord;
      result |= word << nBitAdded; //account for bits already accumulated from previous word(s)
      nBitAdded += endPosInWord - startPosInWord + 1;
      result_vec.push_back(result);
   }
   if (result_vec.size()>1){
   	result = result_vec[0] || result_vec[1];
   }
   return result;

}

StatusCode L1TopoRatesCalculator::ratesExecute() { //EXECUTE
  
  ATH_MSG_DEBUG("In ratesExecute");
  
  SG::ReadHandle<xAOD::TrigDecision> trigDecisionHandle(m_trigDecisionKey); 
  
  const xAOD::TrigDecision* trigDecision = trigDecisionHandle.get();
  const std::vector<uint32_t> l1Triggers = trigDecision->tbp();
  SG::ReadHandle<xAOD::L1TopoSimResultsContainer> cont(m_l1topoKey);

  if(!cont.isValid()){
    ATH_MSG_FATAL("Could not retrieve L1Topo EDM Container from the Simulation.");
    return StatusCode::FAILURE;
  }

  std::vector<std::string> l1Items_vector;
  l1Items_vector = m_beforeCTP_triggers;
  std::vector<uint32_t> resultValue(l1Items_vector.size(), 0);
  //---------------------------------------Mult Items
  
  // Main loop to fill matrix and trigger rates
  // It loops over m_l1items_mult (mult) and l1Items_vector (no mult) to fill the correct decision in the matrix and in the trigger rate
  std::map<std::string, bool> beforeCTP_result_Map;
  for (size_t j = 0; j < m_beforeCTP_triggers_mult.size(); ++j){
  	beforeCTP_result_Map[m_beforeCTP_triggers_mult[j]] = false;
  }
  for (size_t i = 0; i < m_beforeCTP_triggers.size(); ++i) {
	  
	  //Decision of the trigger item
          resultValue[i] = L1TopoSimResultsContainer_decoder(m_definitions[i],cont);
          
	  ATH_MSG_DEBUG("Trigger item: " << m_beforeCTP_triggers[i]); 
	  ATH_MSG_DEBUG("Decision from the decoder (L1TopoResultsContainer): " << resultValue[i]);
 
	  //m_l1items_mult includes the 4 multiplicity items
          for (size_t j = 0; j < m_beforeCTP_triggers_mult.size(); ++j){
		ATH_MSG_DEBUG("Loop over multiplicity array, analysing: " << m_beforeCTP_triggers_mult[j]);
		const auto& mult_item = m_beforeCTP_triggers_mult[j];
		size_t pos = 0;
		while (pos < mult_item.size() && std::isdigit(mult_item[pos])) {
                	++pos;
            	}
		std::string leading_number = mult_item.substr(0, pos);
            	std::string item_name = mult_item.substr(pos);
 				
		//Compares the name of the trigger item of the l1Items_vector with the multiplicities vector
		if (item_name == l1Items_vector[i]) {
				if (leading_number <= std::to_string(resultValue[i])) {
					if (resultValue[i] >= 1){
						beforeCTP_result_Map[mult_item] = true;
						break;
					}
		
				}
		}
		
	  }
  }


  //-------------------------
  // Applying L1items operations-------

  std::map<std::string, bool> L1_result_Map;
  std::vector<bool> isPassed_L1item;

  for (const auto& pair : m_triggerMap) {

   	const std::string& key = pair.first;
        const TriggerInfo& info = pair.second;
	if (info.triggers.empty()) continue;

	bool result = beforeCTP_result_Map[info.triggers[0]];

	for (size_t i = 1; i < info.triggers.size(); ++i) {
		const std::string& currentTrigger = info.triggers[i];
                const std::string& currentOp = info.operations[i - 1];
		if (currentOp == "&") {
                	result &= beforeCTP_result_Map[currentTrigger];
            	} else if (currentOp == "|") {
                	result |= beforeCTP_result_Map[currentTrigger];
            	}
	}
	L1_result_Map[key] = result;
	isPassed_L1item.push_back(result);
  }

  // ----------------------
  //Rates Matrix-------------------------------------------
 
  int bin = 1;
  for (const auto& pair : L1_result_Map) {
        const std::string& label = pair.first;
        m_ratesMatrixHist->GetXaxis()->SetBinLabel(bin, label.c_str());
        m_ratesMatrixHist->GetYaxis()->SetBinLabel(bin, label.c_str());
        ++bin;
  }

  m_denominator.push_back(m_ratesDenominator);
  double weight=0;
  m_weighted_sum += m_weightingValues.m_enhancedBiasWeight;
  for (size_t i = 0; i < m_rates_matrix.size(); ++i){
	  for (size_t j = 0; j < m_rates_matrix.size(); ++j){
		weight=static_cast<double>((isPassed_L1item[i] && isPassed_L1item[j])*(m_weightingValues.m_enhancedBiasWeight)*(m_weightingValues.m_linearLumiFactor));
		(m_rates_matrix[i])[j] += weight;
		(m_rates_matrix2[i])[j] += weight*weight;
	  }
  }

  //-------------------------------------------------------  
  
  return StatusCode::SUCCESS;
}

StatusCode L1TopoRatesCalculator::ratesFinalize() {
  ATH_MSG_DEBUG("In ratesFinalize()");
  //Fill rates matrix----------
  
  for (size_t i = 0; i < m_rates_matrix.size(); ++i) {
        for (size_t j = 0; j < m_rates_matrix.size(); ++j) {
            m_ratesMatrixHist->SetBinContent(j+1, i+1, ((m_rates_matrix[i])[j])/(m_ratesDenominator));
            m_ratesMatrixHist->SetBinError(j+1, i+1,std::sqrt(((m_rates_matrix2[i])[j]))/(m_ratesDenominator));
	}
  }
  //--------------------------
  return StatusCode::SUCCESS;
}
