/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "jJetSelectAlgTool.h"
#include "jJetSelect.h"

namespace GlobalSim {
  jJetSelectAlgTool::jJetSelectAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }

   
  StatusCode jJetSelectAlgTool::initialize() {

    CHECK(m_jJetTOBArrayReadKey.initialize());
    CHECK(m_TOBArrayWriteKey.initialize());

    return StatusCode::SUCCESS;
  }
  
  StatusCode
  jJetSelectAlgTool::run(const EventContext& ctx) const {

    SG::ReadHandle<GlobalSim::jJetTOBArray> jets(m_jJetTOBArrayReadKey,
						 ctx);
    CHECK(jets.isValid());
    
    auto outTOBArray = std::make_unique<GenericTOBArray>("jJetSelectTOBArray");

    auto alg = jJetSelect(m_algInstanceName,
			  m_InputWidth,
			  m_MinET,
			  m_MinEta,
			  m_MaxEta);

    CHECK(alg.run(*jets, *outTOBArray));

       
    SG::WriteHandle<GenericTOBArray> h_write(m_TOBArrayWriteKey,
					     ctx);
    CHECK(h_write.record(std::move(outTOBArray)));
    
  
    
    return StatusCode::SUCCESS;
  }

  std::string jJetSelectAlgTool::toString() const {

    std::stringstream ss;
    ss << "jJetSelectAlgTool.  name: " << name() << '\n'
       << m_jJetTOBArrayReadKey << '\n'
       << m_TOBArrayWriteKey << "\nalg:\n"
       << jJetSelect(m_algInstanceName,
		     m_InputWidth,
		     m_MinET,
		     m_MinEta,
		     m_MaxEta).toString();

    return ss.str();
  }
}

