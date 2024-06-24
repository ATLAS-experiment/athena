/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./jXEInputAlgTool.h"
#include "L1TopoEvent/jXETOB.h"

#include <sstream>


namespace GlobalSim {
  
  jXEInputAlgTool::jXEInputAlgTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode jXEInputAlgTool::initialize() {

    CHECK(m_jFexMETRoIKey.initialize());
    CHECK(m_jXETOBArrayWriteKey.initialize());

    CHECK(m_monTool.retrieve());
    
    return StatusCode::SUCCESS;
  }

  StatusCode jXEInputAlgTool::run(const EventContext& ctx) const {  

    SG::ReadHandle<xAOD::jFexMETRoIContainer>
      jXEContainer(m_jFexMETRoIKey, ctx);
    
    CHECK(jXEContainer.isValid());

    auto jXEs = std::make_unique<GlobalSim::jXETOBArray>("InputjJets", 1);

    
    int global_ExTopo = 0;
    int global_EyTopo = 0;

    for(const xAOD::jFexMETRoI* jFexRoI : *jXEContainer){

      // Get the XE components and convert to 100 MeV units
      int ExTopo = jFexRoI->tobEx() * s_Et_conversion;
      int EyTopo = jFexRoI->tobEy() * s_Et_conversion;
      int jFexNumber = jFexRoI->jFexNumber();
      int fpgaNumber = jFexRoI->fpgaNumber();  

      //Note: flipped to produce the right sign
      int hemisphere = fpgaNumber == 0 ? -1 : 1;

      ExTopo = hemisphere * ExTopo;
      EyTopo = hemisphere * EyTopo;

      global_ExTopo += ExTopo;
      global_EyTopo += EyTopo;

      ATH_MSG_DEBUG("EDM jFex XE Number: "
		     << jFexNumber
		     << " FPGA Number: "
		     << fpgaNumber
		     << " Ex: "
		     << ExTopo
		     << " Ey: "
		     << EyTopo
		    );
    }

    unsigned long long global_ExTopoLong =
      static_cast<unsigned long long>(global_ExTopo);
  
    unsigned long long global_EyTopoLong =
      static_cast<unsigned long long>(global_EyTopo);
  
    unsigned long long Et2Topo =
      global_ExTopoLong*global_ExTopoLong + global_EyTopoLong*global_EyTopoLong;

    unsigned int EtTopo =  std::sqrt(Et2Topo);
  
    TCS::jXETOB jxe(global_ExTopo, global_EyTopo, EtTopo, TCS::JXE);

    jxe.setExDouble(static_cast<double>(global_ExTopo * s_EtDouble_conversion));
    jxe.setEyDouble(static_cast<double>(global_EyTopo * s_EtDouble_conversion));
    jxe.setEtDouble(static_cast<double>(EtTopo * s_EtDouble_conversion));
    jxe.setEt2(Et2Topo);

    jXEs->push_back(jxe);
    
    SG::WriteHandle<GlobalSim::jXETOBArray> h_write(m_jXETOBArrayWriteKey,
						    ctx);
    CHECK(h_write.record(std::move(jXEs)));
    
    auto mon_h_jXE_Pt =
      Monitored::Scalar("jXETOBPt", jxe.EtDouble());
  
    auto mon_h_jXE_Phi =
      Monitored::Scalar("jXETOBPhi", atan2(jxe.Ey(),jxe.Ex()));
  
  
    Monitored::Group(m_monTool,
		     mon_h_jXE_Pt,
		     mon_h_jXE_Phi);

    return StatusCode::SUCCESS;
  }

  std::string jXEInputAlgTool::toString() const {
    std::stringstream ss;
    ss << "jXEInputAlgTool: " << name() << '\n'
       <<  m_jFexMETRoIKey << '\n'
       << m_jXETOBArrayWriteKey << '\n';
    return ss.str();
  }
}

