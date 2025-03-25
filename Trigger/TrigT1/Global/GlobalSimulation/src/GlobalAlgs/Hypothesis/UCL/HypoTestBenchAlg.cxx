//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "HypoTestBenchAlg.h"
#include "AlgoConstants.h"

#include <fstream>

namespace GlobalSim {

  StatusCode
  HypoTestBenchAlg::hexTOB2bitsetTOB(std::string s,
				     std::bitset<72>& bit_tob) const{

    auto bs = std::bitset<4>();

    std::size_t ind{s.size()*4};

    // ensure s is lower case
    std::transform(s.begin(), s.end(), s.begin(),
		   [](unsigned char c){return std::tolower(c);});

    for(const char& c : s) {
      bs = (c >= 'a') ? (c - 'a' + 10) : (c-'0');
      for (int j = 3; j != -1; --j) {
	bit_tob[--ind] = bs[j];
      }
      if (ind == 0) {break;}
    }

    return StatusCode::SUCCESS;
  }
  
  HypoTestBenchAlg::HypoTestBenchAlg(const std::string& name,
				     ISvcLocator *pSvcLocator):
    AthAlgorithm(name, pSvcLocator) {
  }

  StatusCode HypoTestBenchAlg::initialize () {
    ATH_MSG_DEBUG("initialising");
    CHECK(m_hypothesisFIFO_WriteKey.initialize());
    CHECK( m_eEmSortSelectCountExpectations_WriteKey.initialize());

    // initialisation is either from a file of test vectors
    // or by filling in values by hand in this Algorithm

    if (m_testsFileName.empty()){
      ATH_MSG_INFO("Initialisation is manual");
      CHECK(init_manual());
    } else {
      ATH_MSG_INFO("Initialisation is from file " <<  m_testsFileName);
      CHECK(init_from_file());
    }

    ATH_MSG_INFO("Number of fifos " << m_fifos.size());
    for(const auto& fifo : m_fifos) {
          ATH_MSG_INFO("Fifo size " <<
		       fifo->size());
    }
	
    return StatusCode::SUCCESS;
  }

  
  StatusCode HypoTestBenchAlg::execute () {
    ATH_MSG_DEBUG("executing");

    // Write out a FIFO, one per event. Running more events
    // than FIFOs is an error.

    if (m_fifo_ptr == m_fifos.size()) {
      ATH_MSG_ERROR("Attempting to read from an exhausted FIFO vector");
      return StatusCode::FAILURE;
    }
    
    auto h_write =
      SG::WriteHandle<GepAlgoHypothesisFIFO>(m_hypothesisFIFO_WriteKey);
    
    CHECK(h_write.record(std::move(m_fifos[m_fifo_ptr])));

    auto expectations = std::make_unique<eEmSortSelectCountExpectations>(
	m_expected_tobs[m_fifo_ptr],
	m_expected_mults[m_fifo_ptr]);
    
    auto h_write_exp =
      SG::WriteHandle<eEmSortSelectCountExpectations>(m_eEmSortSelectCountExpectations_WriteKey);
    
    CHECK(h_write_exp.record(std::move(expectations)));

    ++m_fifo_ptr;
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  HypoTestBenchAlg::init_manual() {


    // build a single  GepAlgoHypothesisFIFO  that contains
    // GepAlgoHypothesisPortsIn objects. The PortsIn objects contain
    // data only for eEmTobs.

    if (m_testRepeat < 1) {
      ATH_MSG_ERROR("Invalid repeat of input data requested: " << m_testRepeat);
      return StatusCode::FAILURE;
    }

    for(int i = 0; i != m_testRepeat; ++i) {
      m_testVecs.insert(std::begin(m_testVecs),
			std::cbegin(m_testVecs_in),
			std::cend(m_testVecs_in));
    }

    auto fifo = std::make_unique<GepAlgoHypothesisFIFO>();
    for (const auto& tv : m_testVecs) {
      auto ports_in = GepAlgoHypothesisPortsIn();
      CHECK(hexTOB2bitsetTOB(tv, *(ports_in.m_I_eEmTobs)));
      fifo->push_back(ports_in);
    }
    m_fifos.push_back(std::move(fifo));

    m_expected_mults.push_back(m_expMults_in);
    m_expected_tobs.push_back(m_expTobs_in);

    return StatusCode::SUCCESS;
  }

  
      
  std::string trim(std::string s){
    const char* t = " \t\n\r\f\v";
    
    // trim from right
    auto l_rtrim =  [&t](std::string& s){
      s.erase(s.find_last_not_of(t) + 1);
      return s;
    };
   
    // trim from left
    auto l_ltrim = [&t] (std::string& s){
      s.erase(0, s.find_first_not_of(t));
      return s;
    };
    
    auto rs = l_rtrim(s);
    return l_ltrim(rs);
  }

  StatusCode
  HypoTestBenchAlg::init_from_file() {
    CHECK(init_tests_from_file());
    CHECK(init_expected_mults_from_file());
    CHECK(init_expected_tobs_from_file());

    if (m_fifos.empty()) {
      ATH_MSG_ERROR("no fifo data read in");
      return StatusCode::FAILURE;
    }

    
    if (m_fifos.size() != m_expected_mults.size()) {
      ATH_MSG_ERROR("no fifo data objs read in "
		    << m_fifos.size()
		    << " !=  no of expected mults "
		    << m_expected_mults.size());
      return StatusCode::FAILURE;
    }

    
    if (m_fifos.size() != m_expected_mults.size()) {
      ATH_MSG_ERROR("no fifo data objs read in "
		    << m_fifos.size()
		    << " !=  no of expected tobs "
		    << m_expected_tobs.size());
      
      return StatusCode::FAILURE;
    }


    return StatusCode::SUCCESS;
  }
  
  StatusCode
  HypoTestBenchAlg::init_tests_from_file() {


    std::ifstream tob_stream(m_testsFileName);
    if(!tob_stream) {
      std::stringstream ss;
      ATH_MSG_FATAL("Failure to open tob file " << m_testsFileName);
      return StatusCode::FAILURE;
    }

    auto padded_line = std::string();
    auto fifo = std::make_unique<GepAlgoHypothesisFIFO>();
    
    while (std::getline(tob_stream, padded_line)) {
      auto line = trim(padded_line);
      // filter out entries with only '0' entries.nThis are used
      // in VHDL to handle special conditions
      if (std::all_of(std::cbegin(line),
		       std::cend(line),
		      [](const auto& c) {return c == '0';})){
	continue;
      }

      auto ports_in = GepAlgoHypothesisPortsIn();
      CHECK(hexTOB2bitsetTOB(line, *(ports_in.m_I_eEmTobs)));

 		       
      fifo->push_back(ports_in);
      
      // the end of the fifo data is signaled by having the top bit
      // set on an input tob

      auto top_ind = (ports_in.m_I_eEmTobs)->size()-1;
      if ((ports_in.m_I_eEmTobs)->test(top_ind)) {
	m_fifos.push_back(std::move(fifo));
	fifo = std::make_unique<GepAlgoHypothesisFIFO>();
      }

    }
    
    return StatusCode::SUCCESS;
  }

   
  StatusCode
  HypoTestBenchAlg::init_expected_mults_from_file() {
    
    CHECK(init_expected_from_file(m_expected_mults,
				  m_expectedMults_FileName));    
    return StatusCode::SUCCESS;
  }
    
  StatusCode
  HypoTestBenchAlg::init_expected_tobs_from_file() {
    
    CHECK(init_expected_from_file(m_expected_tobs,
				  m_expectedTobs_FileName));    
    return StatusCode::SUCCESS;
  }

     
  StatusCode
  HypoTestBenchAlg::init_expected_from_file(std::vector<std::string>& dest,
					    const std::string& fn) {
    

    std::ifstream in_stream(fn);
    if(!in_stream) {
      std::stringstream ss;
      ATH_MSG_FATAL("Failure to open expected  file " << fn);
      return StatusCode::FAILURE;
    }

    auto padded_line = std::string();
    
    while (std::getline(in_stream, padded_line)) {
      auto line = trim(padded_line);
      dest.push_back(line);
    }
    
    return StatusCode::SUCCESS;
  }
  
  
}
