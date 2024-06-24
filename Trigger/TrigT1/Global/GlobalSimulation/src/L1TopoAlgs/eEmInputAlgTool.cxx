/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmInputAlgTool.h"
#include "L1TopoEvent/eEmTOB.h"

#include <sstream>

namespace GlobalSim {
  
  eEmInputAlgTool::eEmInputAlgTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode eEmInputAlgTool::initialize() {
    CHECK(m_eEmRoIKey.initialize());
    CHECK(m_eEmTOBArrayWriteKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode eEmInputAlgTool::run(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::eFexEMRoIContainer>
      eEmRoIontainer(m_eEmRoIKey, ctx);
    
    CHECK(eEmRoIontainer.isValid());

    auto eEms = std::make_unique<GlobalSim::eEmTOBArray>("InputeEms", 60);
    for(const auto eFexRoI : *eEmRoIontainer){
      
      
      /*
       * eFexNumber()      : 8 bit unsigned integer  eFEX number 
       * et()              : et value of the EM cluster in MeV
       * etTOB()           : et value of the EM cluster in units of 100 MeV
       * eta()             : floating point global eta
       * phi()             : floating point global phi
       * iEtaTopo()        :  40 x eta (custom function for L1Topo)
       * iPhiTopo()        : 20 x phi (custom function for L1Topo)
       * RetaThresholds()  : jet disc 1
       * RhadThresholds()  : jet disc 2
       * WstotThresholds() : jet disc 3
       */
    
      ATH_MSG_DEBUG( "EDM eFex Number: " 
		     << eFexRoI->eFexNumber()
		     << " et: " 
		     << eFexRoI->et()
		     << " etTOB: " 
		     << eFexRoI->etTOB()
		     << " eta: "
		     << eFexRoI->eta()
		     << " phi: "
		     << eFexRoI->phi()
		     << " iEtaTopo: "
		     << eFexRoI->iEtaTopo()
		     << " iPhiTopo: "
		     << eFexRoI->iPhiTopo()
		     << " reta: "
		     << eFexRoI->RetaThresholds()
		     << " rhad: "
		     << eFexRoI->RhadThresholds()
		     << " wstot: "
		     << eFexRoI->WstotThresholds()
		     );

	
      unsigned int EtTopo = eFexRoI->etTOB();
      int etaTopo = eFexRoI->iEtaTopo();
      int phiTopo = eFexRoI->iPhiTopo();
      unsigned int reta = eFexRoI->RetaThresholds();
      unsigned int rhad = eFexRoI->RhadThresholds();
      unsigned int wstot = eFexRoI->WstotThresholds();

      //Em TOB
      TCS::eEmTOB eem(EtTopo, etaTopo, static_cast<unsigned int>(phiTopo), TCS::EEM , static_cast<long int>(eFexRoI->word0()));
      eem.setEtDouble(static_cast<double>(EtTopo * s_EtDouble_conversion));
      eem.setEtaDouble(static_cast<double>(etaTopo * s_etaDouble_conversion));
      eem.setPhiDouble(static_cast<double>(phiTopo * s_phiDouble_conversion));
      eem.setReta(reta);
      eem.setRhad(rhad);
      eem.setWstot(wstot);
    
      eEms->push_back(eem);
    }

   SG::WriteHandle<GlobalSim::eEmTOBArray> h_write(m_eEmTOBArrayWriteKey,
						    ctx);
    CHECK(h_write.record(std::move(eEms)));

    return StatusCode::SUCCESS;
  }

  std::string eEmInputAlgTool::toString() const {
    std::stringstream ss;
    ss << "eEmInputAlgTool: name" << name() << '\n'
       << m_eEmRoIKey << '\n'
       << m_eEmTOBArrayWriteKey << '\n';
    return ss.str();
  }



}

