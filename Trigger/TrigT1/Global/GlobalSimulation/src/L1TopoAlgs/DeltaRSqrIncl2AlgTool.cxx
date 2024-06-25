/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DeltaRSqrIncl2AlgTool.h"
#include "DeltaRSqrIncl2.h"
#include "../dump.h"
#include "../dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"


namespace GlobalSim {
  DeltaRSqrIncl2AlgTool::DeltaRSqrIncl2AlgTool(const std::string& type,
					       const std::string& name,
					       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode DeltaRSqrIncl2AlgTool::initialize() {
       
    CHECK(m_genericTOBArrayReadKey0.initialize());
    CHECK(m_genericTOBArrayReadKey1.initialize());
    CHECK(m_genericTOBArrayVectorWriteKey.initialize());
    CHECK(m_decisionWriteKey.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode
  DeltaRSqrIncl2AlgTool::run(const EventContext& ctx) const {
    
    auto alg = DeltaRSqrIncl2(m_algInstanceName,
			      m_MaxTOB1,
			      m_MaxTOB2,
			      m_MinET1,
			      m_MinET2,
			      m_DeltaRMin,
			      m_DeltaRMax,
			      m_NumResultBits);

    auto in0 =
      SG::ReadHandle<GlobalSim::GenericTOBArray>(m_genericTOBArrayReadKey0,
						 ctx);
    CHECK(in0.isValid());


    auto in1 =
      SG::ReadHandle<GlobalSim::GenericTOBArray>(m_genericTOBArrayReadKey1,
						 ctx);
    CHECK(in1.isValid());

    auto tobArrayVector_out =
      std::make_unique<std::vector<GlobalSim::GenericTOBArray>>();

    auto decision = std::make_unique<GlobalSim::Decision>();

    CHECK(alg.run(*in0,
		  *in1,
		  *tobArrayVector_out,
		  *decision));

    if (m_doDump) {
      dump(name() + "_"  + std::to_string(ctx.evt()), *decision);
    }
    
    SG::WriteHandle<std::vector<GlobalSim::GenericTOBArray>>
      h_tav_write(m_genericTOBArrayVectorWriteKey, ctx);
    CHECK(h_tav_write.record(std::move(tobArrayVector_out)));
    
    
    SG::WriteHandle<GlobalSim::Decision>
      h_decision_write(m_decisionWriteKey, ctx);
    CHECK(h_decision_write.record(std::move(decision)));
    
    
    // monitor
    
    auto nrb = alg.numResultBits();
    
    for (unsigned int i = 0; i <  nrb; ++i) {
      const auto&  passValues = alg.deltaRSqPassByBit(i);
      const auto&  failValues = alg.deltaRSqFailByBit(i);
      std::string label_pass =
	m_algInstanceName + "_pass_by_bit_" + std::to_string(i);
	
      std::string label_fail = m_algInstanceName +
	"_fail_by_bit_" + std::to_string(i);
      auto pass = Monitored::Collection(std::move(label_pass), passValues);
      auto fail = Monitored::Collection(std::move(label_fail), failValues);
      auto group = Monitored::Group(m_monTool, pass, fail);
    }
    
    return StatusCode::SUCCESS;
  }

  std::string DeltaRSqrIncl2AlgTool::toString() const {

    std::stringstream ss;
    ss << "DeltaRSqrIncl2AlgTool. name: " << name() << '\n'
       << m_genericTOBArrayReadKey0 << '\n'
       << m_genericTOBArrayReadKey1 << '\n'
       << m_genericTOBArrayVectorWriteKey << '\n'
       << m_decisionWriteKey << "\n alg:\n"
       << DeltaRSqrIncl2(m_algInstanceName,
			 m_MaxTOB1,
			 m_MaxTOB2,
			 m_MinET1,
			 m_MinET2,
			 m_DeltaRMin,
			 m_DeltaRMax,
			 m_NumResultBits).toString()
       << '\n';
    return ss.str();
  }
}

