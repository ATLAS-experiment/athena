//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "eEmMultTestBench.h"
#include "../IO/IeEmTOB.h"
#include "../IO/eEmTOB.h"
#include "../IO/CommonTOB.h"
#include "../IO/TipWord_clid.h"
#include "../Utilities/trim.h"

#include "PathResolver/PathResolver.h"
#include <bitset>
#include <sstream>

namespace GlobalSim {

  using namespace GlobalSim::IOBitwise;
  
  eEmMultTestBench::eEmMultTestBench(const std::string& name,
				     ISvcLocator *pSvcLocator):
    AthAlgorithm(name, pSvcLocator) {
  }

  StatusCode eEmMultTestBench::initialize () {
    ATH_MSG_DEBUG("initialising");
    CHECK(m_eEmTOBContainer_WriteKey.initialize());
    CHECK( m_TIPword_WriteKey.initialize());

    // initialisation is from a file of test vectors

    std::string fn = PathResolver::find_file(m_tobs_fileName,
					     "DATAPATH");
    if (fn.empty()) {
      ATH_MSG_FATAL("Failure to find tob file " << m_tobs_fileName
		    << " on " << "$DATAPATH");
      return StatusCode::FAILURE;
    }
    
    m_tob_stream = std::ifstream (fn);

    if (!m_tob_stream) {
      ATH_MSG_FATAL("Failure to open tob stream " << fn);
      return StatusCode::FAILURE;
    }
      
    if(!m_tob_stream) {
      ATH_MSG_FATAL("Failure to initialise TOB stream " << fn);
      return StatusCode::FAILURE;
    }

    fn = PathResolver::find_file(m_TIPword_fileName,
				 "DATAPATH");
    if (fn.empty()) {
      ATH_MSG_FATAL("Failure to find TIP file " << m_TIPword_fileName
		    << " on " << "$DATAPATH");
      return StatusCode::FAILURE;
    }

    m_TIPword_stream = std::make_unique<std::ifstream> (fn);
    if(!(*m_TIPword_stream)) {
      std::stringstream ss;
      ATH_MSG_FATAL("Failure to initialise TIP word stream " << fn );
      return StatusCode::FAILURE;
    }


    return StatusCode::SUCCESS;
  }

  
  StatusCode eEmMultTestBench::execute() {
    ATH_MSG_DEBUG("executing");

        
    auto padded_line = std::string();
    auto tobs = std::make_unique<GlobalSim::IOBitwise::IeEmTOBContainer>();

    while (true) {
      std::getline(m_tob_stream, padded_line);
      auto line = trim(padded_line);

      if(!m_tob_stream) {
	ATH_MSG_ERROR("input stream bad state " << m_tobs_fileName);
	return StatusCode::FAILURE;
      }
           
      if(line == "EOE") {break;}
      
      if (line.starts_with("//")){continue;}
      tobs->push_back(make_tob(line));

    }

    using WH_TOB = SG::WriteHandle<GlobalSim::IOBitwise::IeEmTOBContainer>;
    auto h_write_tobs = WH_TOB(m_eEmTOBContainer_WriteKey);    
    CHECK(h_write_tobs.record(std::move(tobs)));

    auto h_write_TIPword = SG::WriteHandle<TIPword>(m_TIPword_WriteKey);
    auto twp = TIPword_from_file();
    CHECK(h_write_TIPword.record(std::move(twp)));
    
    return StatusCode::SUCCESS;
  }
  
  std::unique_ptr<TIPword> eEmMultTestBench::TIPword_from_file() const{

    auto line = std::string();
    using TIP = std::bitset<ITIPwriterAlgTool::s_nbits_TIP>;
    std::getline(*m_TIPword_stream, line);
    auto twp =  std::make_unique<TIP>(std::stoul(trim(std::move(line))));
    return twp;
  }



  IeEmTOB*
  eEmMultTestBench::make_tob(const std::string& trimmed_line) const {
    ATH_MSG_INFO("in make_tob> line: " <<trimmed_line);

    std::stringstream ss(trimmed_line);
    std::string et, eta, phi, RHad, WsTot, REta, seed, UpNotDown, SeedIsMax;
    ss >> et;

    const auto& s_et_width = ICommonTOB::s_et_width;
    const auto& s_eta_width = ICommonTOB::s_eta_width;
    const auto& s_phi_width = ICommonTOB::s_phi_width;

    auto common = CommonTOB(std::bitset<s_et_width>(et),
			    std::bitset<s_eta_width>(eta),
			    std::bitset<s_phi_width>(phi));


    const auto&  s_RHad_width = IeEmTOB::s_RHad_width;
    const auto&  s_REta_width = IeEmTOB::s_REta_width;
    const auto&  s_WsTot_width = IeEmTOB::s_WsTot_width;
    const auto&  s_Seed_width = IeEmTOB::s_Seed_width;
    const auto&  s_UpNotDown_width = IeEmTOB::s_UpNotDown_width;
    const auto&  s_SeedIsMax_width = IeEmTOB::s_SeedIsMax_width;


    return 
      new eEmTOB(common,
		 std::bitset(std::bitset<s_RHad_width>(RHad)),
		 std::bitset(std::bitset<s_REta_width>(REta)),
		 std::bitset(std::bitset<s_WsTot_width>(WsTot)),
		 std::bitset(std::bitset<s_Seed_width>(seed)),
		 std::bitset(std::bitset<s_UpNotDown_width>(UpNotDown)),
		 std::bitset(std::bitset<s_SeedIsMax_width>(SeedIsMax)));
    
  }

}
