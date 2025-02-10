//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "HypoTestBenchAlg.h"
#include "AlgoConstants.h"

namespace GlobalSim {

  HypoTestBenchAlg::HypoTestBenchAlg(const std::string& name,
			 ISvcLocator *pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator) {
  }

  StatusCode HypoTestBenchAlg::initialize () {
    ATH_MSG_DEBUG("initialising");
    CHECK(m_hypothesisFIFO_WriteKey.initialize());
    return StatusCode::SUCCESS;
  }

  
  StatusCode HypoTestBenchAlg::execute (const EventContext& ctx) const {
    ATH_MSG_DEBUG("executing");


    auto fifo = std::make_unique<GepAlgoHypothesisFIFO>();
    for(int i = 0; i < 5; ++i) {
      fifo->push_back(GepAlgoHypothesisPortsIn());
      ATH_MSG_INFO("m_I_eEmTobs 0 " << *(fifo->back().m_I_eEmTobs));

      // set bottom bits of I_eEmTob - this is Et
      *(fifo->back().m_I_eEmTobs) = i;
      ATH_MSG_INFO("m_I_eEmTobs 1 " << *(fifo->back().m_I_eEmTobs));

      // set some eta values. The range in eEmTob os [-100, 100]
      // corresponding to am eta range of [-2.5, 2.5]               
      std::bitset<AlgoConstants::eFexEtaBitWidth> bit_eta = 80+10*i;
      for (std::size_t i = 0; i != AlgoConstants::eFexEtaBitWidth; ++i){
	if (bit_eta.test(i)) {
	  (fifo->back().m_I_eEmTobs)->set(32+i);
	}
      }
      ATH_MSG_INFO("m_I_eEmTobs 2 " << *(fifo->back().m_I_eEmTobs));
	  
	
    }
    auto h_write =
      SG::WriteHandle<GepAlgoHypothesisFIFO>(m_hypothesisFIFO_WriteKey,
					     ctx);
    
    CHECK(h_write.record(std::move(fifo)));
    return StatusCode::SUCCESS;
  }
}
