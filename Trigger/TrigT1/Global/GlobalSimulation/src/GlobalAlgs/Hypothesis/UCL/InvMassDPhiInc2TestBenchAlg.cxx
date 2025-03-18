/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "InvMassDPhiInc2TestBenchAlg.h"

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
    
    
    if (m_testTobs1.size() ==  m_testTobs2.size()) {
      ATH_MSG_INFO("no events with testTobs2 != no of events with testTobs2");
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
    for(int i = 0; i != m_testRepeat; ++i) {
      m_testTobs1.push_back(m_testTobs1_in);
      m_testTobs2.push_back(m_testTobs2_in);
    }

    m_expectedResults.push_back(m_expResults_in);

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
	SG::WriteHandle<TobContainer>(m_tobs1_WriteKey);
      auto tobs =
	std::make_unique<TobContainer>(m_testTobs1[m_dataIndex]);
      CHECK(h_write.record(std::move(tobs)));
    }

    
    {
      auto h_write =
	SG::WriteHandle<TobContainer>(m_tobs2_WriteKey);
    auto tobs =
	std::make_unique<TobContainer>(m_testTobs2[m_dataIndex]);
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
