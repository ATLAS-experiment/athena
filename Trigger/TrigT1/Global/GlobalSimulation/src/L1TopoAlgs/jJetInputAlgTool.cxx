/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./jJetInputAlgTool.h"
#include "L1TopoSimulationUtils/Conversions.h"


#include <sstream>

namespace GlobalSim {

  
  jJetInputAlgTool::jJetInputAlgTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode jJetInputAlgTool::initialize() {
    CHECK(m_jJetRoIKey.initialize());
    CHECK(m_jJetTOBArrayWriteKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  jJetInputAlgTool::run(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> jJetRoIContainer(m_jJetRoIKey,
								 ctx);
    CHECK(jJetRoIContainer.isValid());

    auto jJets = std::make_unique<GlobalSim::jJetTOBArray>("InputjJets", 60);

    for(const xAOD::jFexSRJetRoI* jFexRoI : *jJetRoIContainer){

      /*
       * jFexNumber(): 8 bit unsigned int
       * et():         MeV
       * tobEt():      et in units of 200 MeV
       * globalEta():  simplified global eta in units of 0.1
       *                  (fcal straightened out,
       *                   not suitable for use in L1TopoSim)
       * globalPhi():   simplified global phi in units of 0.1
       *                  (fcal straightened out,
       *                   not suitable for use in L1TopoSim)
       */

      ATH_MSG_DEBUG( "EDM jFex Jet Number: " 
		     << +jFexRoI->jFexNumber()
		     << " et: " 
		     << jFexRoI->et()
		     << " tobEt: " 
		     << jFexRoI->tobEt()
		     << " globalEta: "
		     << jFexRoI->globalEta()
		     << " globalPhi: "
		     << jFexRoI->globalPhi()
		     );

      unsigned int EtTopo = jFexRoI->tobEt() * s_Et_conversion;
      unsigned int phiTopo = TSU::toTopoPhi(jFexRoI->phi());
      int etaTopo = TSU::toTopoEta(jFexRoI->eta());

      // Avoid the events with 0 Et (events below threshold)
      if (EtTopo==0) continue;

      TCS::jJetTOB jet( EtTopo, etaTopo, phiTopo );
      jet.setEtDouble(static_cast<double>(EtTopo * s_EtDouble_conversion));
      jet.setEtaDouble(static_cast<double>(etaTopo * s_etaDouble_conversion));
      jet.setPhiDouble(static_cast<double>(phiTopo * s_phiDouble_conversion));
 
      jJets-> push_back(jet);
    }

    SG::WriteHandle<GlobalSim::jJetTOBArray> h_write(m_jJetTOBArrayWriteKey,
						     ctx);
    CHECK(h_write.record(std::move(jJets)));


    return StatusCode::SUCCESS;
  }

  std::string jJetInputAlgTool::toString() const {
    std::stringstream ss;
    ss << "jJetInputAlgTool. " << name() <<'\n'
       << m_jJetRoIKey << '\n'
       <<  m_jJetTOBArrayWriteKey << '\n';
    
    return ss.str();
  }



}

