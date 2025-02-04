/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./cTauInputAlgTool.h"
#include "L1TopoSimulationUtils/Conversions.h"


#include "L1TopoEvent/eTauTOB.h"
#include "L1TopoEvent/jTauTOB.h"
#include "../IO/cTauTOBArray.h"
#include <sstream>

namespace GlobalSim {
  
  cTauInputAlgTool::cTauInputAlgTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode cTauInputAlgTool::initialize() {
    CHECK(m_eTauRoIKey.initialize());
    CHECK(m_jTauRoIKey.initialize());
    CHECK(m_cTauTOBArrayWriteKey.initialize());

    return StatusCode::SUCCESS;
  }
  
  StatusCode cTauInputAlgTool::run(const EventContext& ctx) const {
    
    auto cTaus = std::make_unique<GlobalSim::cTauTOBArray>("InputcTaus", 120);
    
    CHECK(eFex_cTaus(*cTaus, ctx));
    CHECK(jFex_cTaus(*cTaus, ctx));
    
    SG::WriteHandle<GlobalSim::cTauTOBArray> h_write(m_cTauTOBArrayWriteKey,
						     ctx);
    CHECK(h_write.record(std::move(cTaus)));

    return StatusCode::SUCCESS;
  }

   StatusCode
  cTauInputAlgTool::eFex_cTaus(GlobalSim::cTauTOBArray& cTaus,
			       const EventContext& ctx) const {
    
    SG::ReadHandle<xAOD::eFexTauRoIContainer>
      eTauContainer(m_eTauRoIKey, ctx);
    
    CHECK(eTauContainer.isValid());

    for(const auto eFexTauRoI : *eTauContainer){
      
      
      /*
       * eFexNumber()      : 8 bit unsigned integer  eFEX number 
       * et()              : et value of the Tau cluster in MeV
       * etTOB()           : et value of the Tau cluster in units of 100 MeV
       * eta()             : floating point global eta
       * phi()             : floating point global phi
       * iEtaTopo()        :  40 x eta (custom function for L1Topo)
       * iPhiTopo()        : 20 x phi (custom function for L1Topo)
       * tauOneThresholds() : rCore or BDT working point
       *                         (eTau algo agnostic accessor)
       * tauTwoThresholds() : rHad
       */
    
      ATH_MSG_DEBUG( "EDM eFex Number: " 
		     << eFexTauRoI->eFexNumber()
		     << " et: " 
		     << eFexTauRoI->et()
		     << " etTOB: " 
		     << eFexTauRoI->etTOB()
		     << " eta: "
		     << eFexTauRoI->eta()
		     << " phi: "
		     << eFexTauRoI->phi()
		     << " iEtaTopo: "
		     << eFexTauRoI->iEtaTopo()
		     << " iPhiTopo: "
		     << eFexTauRoI->iPhiTopo()
		     << " rCore: "
		     << eFexTauRoI->tauOneThresholds()
		     << " rHad: "
		     << eFexTauRoI->tauTwoThresholds()
		     );

	
      unsigned int EtTopo = eFexTauRoI->etTOB();
      int etaTopo = eFexTauRoI->iEtaTopo();
      int phiTopo = eFexTauRoI->iPhiTopo();
      unsigned int rCore = eFexTauRoI->tauOneThresholds();
      unsigned int rHad = eFexTauRoI->tauTwoThresholds();

      //Tau TOB
      TCS::eTauTOB eTau(EtTopo,
			etaTopo,
			static_cast<unsigned int>(phiTopo),
			TCS::ETAU);
      
      eTau.setEtDouble(static_cast<double>(EtTopo * s_EtDouble_conversion));
      eTau.setEtaDouble(static_cast<double>(etaTopo * s_etaDouble_conversion));
      eTau.setPhiDouble(static_cast<double>(phiTopo * s_phiDouble_conversion));
      eTau.setRCore(rCore);
      eTau.setRHad(rHad);

      TCS::cTauTOB cTau(eTau.Et(), eTau.eta(), eTau.phi(), TCS::ETAU);
      cTau.setEtDouble( eTau.EtDouble() );
      cTau.setEtaDouble( eTau.etaDouble() );
      cTau.setPhiDouble( eTau.phiDouble() );
      cTau.setRCore( eTau.rCore() );
      cTau.setRHad( eTau.rHad() );
      
      cTaus.push_back(cTau);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode cTauInputAlgTool::jFex_cTaus(GlobalSim::cTauTOBArray& cTaus,
					  const EventContext& ctx) const {
       
    SG::ReadHandle<xAOD::jFexTauRoIContainer>
      jTauRoIContainer(m_jTauRoIKey, ctx);
    
    CHECK(jTauRoIContainer.isValid());
    

    for(const auto jFexTauRoI : *jTauRoIContainer){
      
      
      /*
       * jFexNumber()  8 bit unsigned integer  jFEX number 
       * et()        et value of the Tau cluster in MeV
       * tobEt()      et value of the Tau cluster in units of 200 MeV
       * globalEta()  simplified global eta in units of 0.1
       *               (fcal straightened out, not suitable for use
       *                in L1TopoSim)
       * globalPhi()  simplified global phi in units of 0.1
       *               (fcal straightened out,
       *                 not suitable for use in L1TopoSim)
       * isolation    isolation value in units of 200 MeV
       */
    
      ATH_MSG_DEBUG( "EDM jFex Number: " 
		     << static_cast<int>(jFexTauRoI->jFexNumber())
		     << " et: " 
		     << jFexTauRoI->et()
		     << " topEt: " 
		     << jFexTauRoI->tobEt()
		     << " globalEta: "
		     << jFexTauRoI->globalEta()
		     << " globalPhi: "
		     << jFexTauRoI->globalPhi()
		     << " isolation: "
		     << jFexTauRoI->tobIso()
		     );

	
      unsigned int EtTopo = jFexTauRoI->tobEt() * s_Et_conversion;
      unsigned int phiTopo = TSU::toTopoPhi(jFexTauRoI->phi());
      int etaTopo = TSU::toTopoEta(jFexTauRoI->eta());
      unsigned int isolation = jFexTauRoI->tobIso() * s_Et_conversion;

      if (EtTopo == 0) {continue;}

      TCS::jTauTOB jTau(EtTopo, etaTopo, phiTopo);
      jTau.setEtDouble(static_cast<double>(EtTopo * s_EtDouble_conversion));
      jTau.setEtaDouble(static_cast<double>(etaTopo * s_etaDouble_conversion));
      jTau.setPhiDouble(static_cast<double>(phiTopo * s_phiDouble_conversion));
      jTau.setEtIso(isolation);

      TCS::cTauTOB cTau(jTau.Et(), jTau.eta(), jTau.phi(), TCS::JTAU);
      cTau.setEtDouble( jTau.EtDouble() );
      cTau.setEtaDouble( jTau.etaDouble() );
      cTau.setPhiDouble( jTau.phiDouble() );
      cTau.setEtIso( jTau.EtIso() );
      cTaus.push_back(cTau);
    }
    return StatusCode::SUCCESS;

  }

  std::string cTauInputAlgTool::toString() const {
    std::stringstream ss;
    ss << "cTauInputAlgTool: '" << name() << '\n'
       << m_jTauRoIKey << '\n'
       << m_eTauRoIKey << '\n'
       << m_cTauTOBArrayWriteKey << '\n';
    return ss.str();
  }



}

