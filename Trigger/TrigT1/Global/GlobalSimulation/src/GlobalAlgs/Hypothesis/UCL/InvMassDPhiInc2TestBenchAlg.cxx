/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "InvMassDPhiInc2TestBenchAlg.h"

#include <algorithm>

namespace GlobalSim {

    
  InvMassDPhiInc2TestBenchAlg::InvMassDPhiInc2TestBenchAlg(const std::string& name,
							   ISvcLocator *pSvcLocator):
    AthAlgorithm(name, pSvcLocator) {
  }


  

  StatusCode InvMassDPhiInc2TestBenchAlg::initialize () {
    ATH_MSG_DEBUG("initialising");
    CHECK(m_tobs1_WriteKey.initialize());
    CHECK(m_tobs2_WriteKey.initialize());
    CHECK( m_expectations_WriteKey.initialize());

    // initialisation is either from a file of test vectors
    // or by filling in values by hand in this Algorithm

    if (m_testsFileName1.empty()){
      ATH_MSG_INFO("Initialisation is manual");
      CHECK(init_manual());
    } else {
      ATH_MSG_ERROR("initialisation from file not yet implemented");
      return StatusCode::FAILURE;
    }
    
    
    if (m_testTobs1.empty()) {
      ATH_MSG_INFO("no events with testTobs1");
      return StatusCode::FAILURE;
    }
    
    
    if (m_testTobs1.size() !=  m_testTobs2.size()) {
      auto ss = std::stringstream();
      ss << "number events with testTobs2 ["
	 << m_testTobs1.size()
	 << "] != number events with testTobs2 ["
	 << m_testTobs2.size()
	 << ']';	
      ATH_MSG_INFO(ss.str());
      return StatusCode::FAILURE;
    }

    for (std::size_t i =0; i !=  m_testTobs2.size(); ++i) {
      if (m_testTobs1[i].size() != m_testTobs2[i].size()) {
	ATH_MSG_ERROR("Number of input test tobs differ for event " << i);
	return StatusCode::FAILURE;
      }
    }

    if (m_expectedResults.size() != m_testTobs1.size()) {
      ATH_MSG_ERROR("Number of expected results and number of sets of input tobs differ");
      return StatusCode::FAILURE;

    }

    ATH_MSG_INFO("Number of events stored " << m_expectedResults.size());
	
    return StatusCode::SUCCESS;
  }

  StatusCode InvMassDPhiInc2TestBenchAlg::init_from_file(){
    CHECK(init_tobs1_from_file());
    CHECK(init_tobs2_from_file());
    CHECK(init_expected_results_from_file());
    return StatusCode::SUCCESS;
  }

  StatusCode InvMassDPhiInc2TestBenchAlg::init_tobs1_from_file(){
    return StatusCode::SUCCESS;
  }

  
  StatusCode InvMassDPhiInc2TestBenchAlg::init_tobs2_from_file(){
    return StatusCode::SUCCESS;
  }

  StatusCode InvMassDPhiInc2TestBenchAlg::init_expected_results_from_file(){
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  InvMassDPhiInc2TestBenchAlg::init_manual() {


    // store an event of GenericTobs 1, GenericTobs 2 and expected values
    // initialised from python.  All inputs are binary strings.

    if (m_testRepeat < 1) {
      ATH_MSG_ERROR("Invalid repeat of input data requested: " << m_testRepeat);
      return StatusCode::FAILURE;
    }

    // apply the repeat value
    {
      auto container = GenericTobContainer();
      container.reserve(m_testRepeat*(m_testTobs1_in.size()));

      for (std::size_t i = 0; i != m_testRepeat; ++i) {
	std::transform(std::cbegin(m_testTobs1_in),
		       std::cend(m_testTobs1_in),
		       std::back_inserter(container),
		       [](const auto& bs){
			 return std::make_shared<GenericTob>(bs);});
	
      }
      m_testTobs1.push_back(container);
    }

    {
      auto container = GenericTobContainer();
      container.reserve(m_testRepeat*(m_testTobs2_in.size()));
      
      for (std::size_t i = 0; i != m_testRepeat; ++i) {
	std::transform(std::cbegin(m_testTobs2_in),
		       std::cend(m_testTobs2_in),
		       std::back_inserter(container),
		       [](const auto& bs){
			 return std::make_shared<GenericTob>(bs);});
	
      }
      m_testTobs2.push_back(container);
    }

    m_expectedResults.push_back(m_expResults_in);


    
    {
      auto ss = std::stringstream();
      
      ss << "bitstrings tobs1 [" << m_testTobs1_in.size()<< "]\n";
      for (const auto& bs : m_testTobs1_in) {
	ss << bs << '\n';
      }
      ATH_MSG_DEBUG(ss.str());
    }
    
    {
      auto ss = std::stringstream();
      std::size_t i_ev{0};
      
      ss << "nevents tobs1 " << m_testTobs1.size()<< '\n';
      for (const auto& ev : m_testTobs1) {
	ss << "ev " << i_ev++ << '\n';
	for (const auto& gt : ev) {
	  ss << *gt << '\n';
	}
	ATH_MSG_DEBUG(ss.str());
      }
    }

    {
      auto ss = std::stringstream();
      std::size_t i_ev{0};
      
      ss << "nevents tobs2 " << m_testTobs2.size() << '\n';
      for (const auto& ev : m_testTobs2) {
	ss << "ev " << i_ev++ << '\n';
	for (const auto& gt : ev) {
	  ss << *gt << '\n';
	}
	ATH_MSG_DEBUG(ss.str());
      }
    }
    
    return StatusCode::SUCCESS;
  }


  StatusCode
  InvMassDPhiInc2TestBenchAlg::execute() {
    ATH_MSG_DEBUG("executing");
    
    // Write out a 2 vectors of GenericTob bits streams per event.
    // over running the number of such vectors is an error

    if (m_dataIndex ==  m_testTobs1.size()) {
      ATH_MSG_ERROR("Attempting to read beyond stored data");
      return StatusCode::FAILURE;
    }

    {
      auto h_write =
	SG::WriteHandle<GenericTobContainer>(m_tobs1_WriteKey);
      auto tobs =
	std::make_unique<GenericTobContainer>(m_testTobs1[m_dataIndex]);
      CHECK(h_write.record(std::move(tobs)));
    }

    
    {
      auto h_write =
	SG::WriteHandle<GenericTobContainer>(m_tobs2_WriteKey);
    auto tobs =
	std::make_unique<GenericTobContainer>(m_testTobs2[m_dataIndex]);
    CHECK(h_write.record(std::move(tobs)));
    }


    {  
      auto h_write =
	SG::WriteHandle<Result>(m_expectations_WriteKey);

      auto expectations =
	std::make_unique<Result>(m_expectedResults[m_dataIndex]);
      
      
      CHECK(h_write.record(std::move(expectations)));
    }
    
    ++m_dataIndex;
    
    return StatusCode::SUCCESS;
  }


}
