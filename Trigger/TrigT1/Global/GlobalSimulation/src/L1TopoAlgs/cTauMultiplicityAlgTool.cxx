/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "cTauMultiplicityAlgTool.h"
#include "cTauMultiplicity.h"

#include "../dump.h"
#include "../dump.icc"

#include "TrigConfData/L1Menu.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>


namespace GlobalSim {
  cTauMultiplicityAlgTool::cTauMultiplicityAlgTool(const std::string& type,
						   const std::string& name,
						   const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode cTauMultiplicityAlgTool::initialize() {

    if (m_nbits == 0u) {
      ATH_MSG_ERROR("m_nbits == 0");
      return StatusCode::FAILURE;
    }

    const TrigConf::L1Menu* l1menu;
    ATH_CHECK(detStore()->retrieve(l1menu));
    ATH_MSG_INFO("L1Menu name " << l1menu->getAttribute("name"));
    m_l1MenuResources =
      std::make_unique<L1MenuResources>(*l1menu,
					m_algInstanceName,
					"MULTTOPO");

    CHECK(m_cTauTOBArrayReadKey.initialize());
    CHECK(m_CountWriteKey.initialize());

    
    return StatusCode::SUCCESS;
  }

  StatusCode
  cTauMultiplicityAlgTool::run(const EventContext& ctx) const {
 
    SG::ReadHandle<GlobalSim::cTauTOBArray> cTaus(m_cTauTOBArrayReadKey, ctx);
    CHECK(cTaus.isValid());

    auto count = std::make_unique<GlobalSim::Count>();

    const auto & threshold =
      dynamic_cast<const TrigConf::L1Threshold_cTAU&>(m_l1MenuResources->threshold());
    
    auto alg =  
      cTauMultiplicity(m_algInstanceName,
		       m_nbits,
		       threshold,
		       m_l1MenuResources->isolationFW_CTAU(),
		       m_l1MenuResources->isolationFW_CTAU_jTAUCoreScale());


    CHECK(alg.run(*cTaus, *count));
    SG::WriteHandle<GlobalSim::Count> h_write(m_CountWriteKey,
					      ctx);
     if (m_doDump) {
       dump(name() + '_' + std::to_string(ctx.evt()), *count);
    }

    CHECK(h_write.record(std::move(count)));
   
    CHECK(monitor(alg));

   
    return StatusCode::SUCCESS;
  }

  StatusCode
  cTauMultiplicityAlgTool::monitor(const cTauMultiplicity& alg) const {

    const auto & accept_eta_data = alg.accept_eta();
    
    auto accept_eta_moncol = Monitored::Collection("accept_eta",
						   accept_eta_data);
    const auto & accept_et_data = alg.accept_et();
    auto accept_et_moncol = Monitored::Collection("accept_et",
						  accept_et_data);
    
    const auto & counts_data = alg.counts();
    auto counts_moncol = Monitored::Collection("counts", counts_data);

    auto TOBet_data = alg.TOB_et();
    auto TOBet_moncol = Monitored::Collection("Et", TOBet_data);

       
    const auto & TOBeta_data = alg.TOB_eta();
    auto TOBeta_moncol = Monitored::Collection("eta", TOBeta_data);
 
    
    const auto & TOBphi_data = alg.TOB_phi();
    auto TOBphi_moncol = Monitored::Collection("phi", TOBphi_data);
       
    const auto & TOBisolation_partial_loose_data = alg.TOB_isolation_partial_loose();
    auto TOBisolation_partial_loose_moncol =
      Monitored::Collection("iso_loose", TOBisolation_partial_loose_data);
 
    const auto & TOBisolation_partial_medium_data = alg.TOB_isolation_partial_medium();
    auto TOBisolation_partial_medium_moncol =
      Monitored::Collection("iso_medium", TOBisolation_partial_medium_data);

    const auto & TOBisolation_partial_tight_data = alg.TOB_isolation_partial_tight();
    auto TOBisolation_partial_tight_moncol =
      Monitored::Collection("iso_tight", TOBisolation_partial_tight_data);


    auto group = Monitored::Group(m_monTool,
				  accept_eta_moncol,
				  accept_et_moncol,
				  TOBet_moncol,	
				  TOBphi_moncol,
				  TOBeta_moncol,
				  TOBisolation_partial_loose_moncol,
				  TOBisolation_partial_medium_moncol,
				  TOBisolation_partial_tight_moncol
				  );

    return StatusCode::SUCCESS;
  }

  std::string cTauMultiplicityAlgTool::toString() const {

    std::stringstream ss;
    const auto & threshold =
      dynamic_cast<const TrigConf::L1Threshold_cTAU&>(m_l1MenuResources->threshold());
    ss << "cTauMultiplicityAlgTool. name: "  << name() << '\n'
       << m_cTauTOBArrayReadKey << '\n'
       << m_CountWriteKey << '\n' 
       << cTauMultiplicity(m_algInstanceName,
			   m_nbits,
			   threshold,
			   m_l1MenuResources->isolationFW_CTAU(),
			   m_l1MenuResources->isolationFW_CTAU_jTAUCoreScale()).toString()
       << '\n';
    return ss.str();
  }

			    

}

