/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EnergyThresholdAlgTool_jXE.h"
#include "EnergyThreshold_jXE.h"
#include "../IO/jXETOBArray.h"
#include "../IO/Count.h"


#include <sstream>

namespace GlobalSim {

  EnergyThresholdAlgTool_jXE::EnergyThresholdAlgTool_jXE(const std::string& type,
							 const std::string& name,
							 const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode EnergyThresholdAlgTool_jXE::initialize() {

    if (m_nbits == 0u) {
      ATH_MSG_ERROR("m_nbits == 0");
      return StatusCode::FAILURE;
    }

    CHECK(m_jXETOBArrayReadKey.initialize());
    CHECK(m_CountWriteKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode
  EnergyThresholdAlgTool_jXE::run(const EventContext& ctx) const {
     
    
    SG::ReadHandle<GlobalSim::jXETOBArray> jXEs(m_jXETOBArrayReadKey, ctx);
    CHECK(jXEs.isValid());
    
    auto count = std::make_unique<GlobalSim::Count>();

    CHECK(EnergyThreshold_jXE(m_algInstanceName, m_nbits, m_threshold).run(*jXEs, *count));

    SG::WriteHandle<GlobalSim::Count> h_write(m_CountWriteKey,
					      ctx);
    CHECK(h_write.record(std::move(count)));
    
    return StatusCode::SUCCESS;
  }
  
  std::string EnergyThresholdAlgTool_jXE::toString() const {

    std::stringstream ss;
    ss << "EnergyThresholdAlgTool_jXE. name: " << name() << '\n'
       << m_jXETOBArrayReadKey << '\n'
       <<  m_CountWriteKey << '\n';

    ss <<EnergyThreshold_jXE(m_algInstanceName,
			     m_nbits,
			     m_threshold).toString();
    return ss.str();
  }
}

