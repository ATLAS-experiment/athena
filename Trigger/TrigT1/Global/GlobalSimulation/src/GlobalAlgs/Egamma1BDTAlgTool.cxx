/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Egamma1BDTAlgTool.h"
#include "../dump.h"
#include "../dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "./Egamma1BDT/parameters.h"

namespace GlobalSim {

  Egamma1BDTAlgTool::Egamma1BDTAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode Egamma1BDTAlgTool::initialize() {
       
    CHECK(m_nbhdContainerReadKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  Egamma1BDTAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in LArStrip neighborhoods from the event store
    // there is one neighborhood per EFex RoI
    auto in =
      SG::ReadHandle<LArStripNeighborhoodContainer>(m_nbhdContainerReadKey,
						    ctx);
    CHECK(in.isValid());

    ATH_MSG_DEBUG("read in " << (*in).size() << " neighborhoods");

    
    for (const auto nbhd : *in) {
      auto c_phi = combine_phi(nbhd);
      if (c_phi.empty()) {continue;}  // corner case: not all phi have len 17
      auto input = digitize(c_phi);

      assert(input.size() == n_features);
      ap_int<10>* c_input = &input[0];  // vector->array

      score_t scores[GlobalSim::BDT::fn_classes(n_classes)];

      // the bdt variable is already set up 
      bdt.decision_function(c_input, scores);
      {
	std::stringstream ss;
	ss << "BDT input: ";
	for (const auto& i : input) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      {
	std::stringstream ss;
	ss << "C BDT output: ";
	for (const auto& i : scores) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

    }

   
    return StatusCode::SUCCESS;
  }

  
  std::vector<double>
  Egamma1BDTAlgTool::combine_phi(const LArStripNeighborhood* nbhd) const  {
    auto result = std::vector<double>();

    const auto& phi_low = nbhd->phi_low();
    if (phi_low.size() != s_required_phi_len) {return result;}

    const auto& phi_center = nbhd->phi_center();
    if (phi_center.size() != s_required_phi_len) {return result;}

    
    const auto& phi_high = nbhd->phi_high();
    if (phi_high.size() != s_required_phi_len) {return result;}

    result.resize(s_combination_len);

    constexpr int c{8};

    result.at(0) = phi_center.at(c).m_e;

    result.at(1) = std::max(phi_low.at(c).m_e, phi_high.at(c).m_e);

    int ri{2};
    for (int diff = 1; diff != 9; ++diff) { 
      result.at(ri) =
	std::max({phi_center.at(c-diff).m_e,
	    phi_center.at(c+diff).m_e});

      result.at(ri+1) =
	std::max({phi_low.at(c-diff).m_e,
	  phi_low.at(c+diff).m_e,
	  phi_high.at(c-diff).m_e,
	  phi_high.at(c+diff).m_e});
      
      ri += 2;
    }
    
    return result;
  }

  
  std::vector<ap_int<10>>
  Egamma1BDTAlgTool::digitize(const std::vector<double>& v) const {
    auto sf = [](double v) {
      if (v < 0) {return 0.;}
      if (v < 8000) {return v / 31.25;}
      if (v < 40000) {return 192. + v / 125;}
      if (v < 168000) {return 432. + v / 500;}
      if (v < 678000) {return 686. + v / 2000;}
      return 1023.;
    };

    auto result = std::vector<ap_int<10>>();
    result.reserve(s_combination_len);
      
    std::transform(std::cbegin(v),
		   std::cend(v),
		   std::back_inserter(result),
		   sf);
		   
    return result;
  }


  std::string Egamma1BDTAlgTool::toString() const {

    std::stringstream ss;
    ss << "Egamma1BDTAlgTool. name: " << name() << '\n'
       << m_nbhdContainerReadKey << '\n'
       << '\n';
    return ss.str();
  }
}

