/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmSelectAlgTool.h"
#include "eEmSelect.h"

namespace GlobalSim {
  eEmSelectAlgTool::eEmSelectAlgTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode eEmSelectAlgTool::initialize() {

    CHECK(m_eEmTOBArrayReadKey.initialize());
    CHECK(m_TOBArrayWriteKey.initialize());
    
    return StatusCode::SUCCESS;
  }
   
  StatusCode
  eEmSelectAlgTool::run(const EventContext& ctx) const {

    SG::ReadHandle<GlobalSim::eEmTOBArray> eems(m_eEmTOBArrayReadKey, ctx);
    CHECK(eems.isValid());
   
    auto outTOBArray = std::make_unique<GenericTOBArray>("eEmSelectTOBArray");
    
    auto alg = eEmSelect(m_algInstanceName,
			 m_InputWidth,
			 m_MinET,
			 m_REtaMin,
			 m_RHadMin,
			 m_WsTotMin);

    CHECK(alg.run(*eems, *outTOBArray));

    
    SG::WriteHandle<GenericTOBArray> h_write(m_TOBArrayWriteKey,
					     ctx);
    CHECK(h_write.record(std::move(outTOBArray)));
    
    return StatusCode::SUCCESS;
  }

  std::string eEmSelectAlgTool::toString() const {

    std::stringstream ss;
    ss << "eEmSelectAlgTool. name: " << name() << '\n'
       <<  m_eEmTOBArrayReadKey <<  '\n'
       << m_TOBArrayWriteKey << "\nalg:\n"
       << eEmSelect(m_algInstanceName,
		    m_InputWidth,
		    m_MinET,
		    m_REtaMin,
		    m_RHadMin,
		    m_WsTotMin).toString();

    return ss.str();
  }
}

