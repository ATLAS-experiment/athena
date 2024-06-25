/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SimpleConeAlgTool.h"
#include "SimpleCone.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"


namespace GlobalSim {
  SimpleConeAlgTool::SimpleConeAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode SimpleConeAlgTool::initialize() {
    
    CHECK(m_genericTOBArrayReadKey.initialize());
    CHECK(m_genericTOBArrayVectorWriteKey.initialize());
    CHECK(m_decisionWriteKey.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode
  SimpleConeAlgTool::run(const EventContext& ctx) const {
    auto alg = SimpleCone(m_algInstanceName,
			  m_InputWidth,
			  m_MaxRSqr,
			  m_MinET,
			  m_MinSumET);

    auto tobArray_in =
      SG::ReadHandle<GlobalSim::GenericTOBArray>(m_genericTOBArrayReadKey,
						 ctx);
    CHECK(tobArray_in.isValid());

    auto tobArrayVector_out =
      std::make_unique<std::vector<GlobalSim::GenericTOBArray>>();

    auto decision = std::make_unique<GlobalSim::Decision>();
   
    
    CHECK(alg.run(*tobArray_in,
		  *tobArrayVector_out,
		  *decision));

    SG::WriteHandle<std::vector<GlobalSim::GenericTOBArray>>
      h_tav_write(m_genericTOBArrayVectorWriteKey, ctx);
    CHECK(h_tav_write.record(std::move(tobArrayVector_out)));


    SG::WriteHandle<GlobalSim::Decision>
      h_decision_write(m_decisionWriteKey, ctx);
    CHECK(h_decision_write.record(std::move(decision)));


    //monitoring
   
    auto nrb =  m_MinSumET.size();
    for (unsigned int i = 0; i <  nrb; ++i) {
      const auto&  passValues = alg.ETPassByBit(i);
      const auto&  failValues = alg.ETFailByBit(i);

      std::string label_pass = m_algInstanceName +
	"_pass_by_bit_" + std::to_string(i);

      std::string label_fail = m_algInstanceName +
	"_fail_by_bit_" + std::to_string(i);
      
      auto pass = Monitored::Collection(std::move(label_pass), passValues);
      auto fail = Monitored::Collection(std::move(label_fail), failValues);
      auto group = Monitored::Group(m_monTool, pass, fail);
    }

    
    return StatusCode::SUCCESS;
  }

  std::string SimpleConeAlgTool::toString() const {

    std::stringstream ss;
    ss << "SimpleConeAlgTool.  name: " << name() << '\n'
       << m_genericTOBArrayReadKey << '\n'
       << m_genericTOBArrayVectorWriteKey << '\n'
       << m_decisionWriteKey << "\nalg:\n"
       << SimpleCone(m_algInstanceName,
		     m_InputWidth,
		     m_MaxRSqr,
		     m_MinET,
		     m_MinSumET).toString();

    return ss.str();
  }
}

