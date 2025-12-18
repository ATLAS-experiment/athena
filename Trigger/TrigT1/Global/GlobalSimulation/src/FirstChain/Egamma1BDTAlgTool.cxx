/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Egamma1BDTAlgTool.h"
#include "../Utilities/dump.h"
#include "../Utilities/dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "./Egamma1BDT/parameters.h"

#include "../IO/eEmEg1BDTTOB.h"

namespace GlobalSim {

  using eEmEg1BDTTOB = IOBitwise::eEmEg1BDTTOB;

  Egamma1BDTAlgTool::Egamma1BDTAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode Egamma1BDTAlgTool::initialize() {
       
    CHECK(m_nbhdTOBContainerReadKey.initialize());
    CHECK(m_BDTResultKey.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  Egamma1BDTAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in LArStrip neighborhood TOBs from the event store
    auto in =
      SG::ReadHandle<IOBitwise::eEmNbhoodTOBContainer>(m_nbhdTOBContainerReadKey,
						       ctx);
    CHECK(in.isValid());

    ATH_MSG_DEBUG("read in " << (*in).size() << " neighborhoods");

    SG::WriteHandle<IOBitwise::eEmEg1BDTTOBContainer> h_BDTResult(m_BDTResultKey, ctx);
    CHECK(h_BDTResult.record(std::make_unique<IOBitwise::eEmEg1BDTTOBContainer>()));
    
    for (const auto nbhdTOB : *in) {
      auto c_phi = combine_phi(nbhdTOB);
      if (c_phi.empty()) {continue;}  // corner case: not all phi have len 17
      auto input = digitizer::digitize10(c_phi);

      assert(input.size() == n_features);
      ap_int<10>* c_input = &input[0];  // vector->array

      score_t scores[GlobalSim::BDT::fn_classes(n_classes)];

      // the bdt variable is already set up 
      bdt.decision_function(c_input, scores);
      if (msgLevel() <= MSG::DEBUG) {
	std::stringstream ss;
	ss << "BDT input: ";
	for (const auto& i : input) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      if (msgLevel() <= MSG::DEBUG) {
	std::stringstream ss;
	ss << "C BDT output: ";
	for (const auto& i : scores) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      //Extract the bits (one by one) from the ap_fixed<10,5> object -> Bitset<10>
      std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> result;
      for (int i=0;i<scores[0].length();i++){
	result[i] = scores[0][0];
      }

      h_BDTResult->push_back(std::make_unique<IOBitwise::eEmEg1BDTTOB>(*nbhdTOB, result));
      
    }
    return StatusCode::SUCCESS;
  }

  
  std::vector<double>
  Egamma1BDTAlgTool::combine_phi(const IOBitwise::eEmNbhoodTOB* nbhdTOB) const  {
    auto result = std::vector<double>();

    const auto& phi_low = nbhdTOB->Neighbourhood().phi_low();
    if (phi_low.size() != s_required_phi_len) {return result;}

    const auto& phi_center = nbhdTOB->Neighbourhood().phi_center();
    if (phi_center.size() != s_required_phi_len) {return result;}

    
    const auto& phi_high = nbhdTOB->Neighbourhood().phi_high();
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

  std::string Egamma1BDTAlgTool::toString() const {

    std::stringstream ss;
    ss << "Egamma1BDTAlgTool. name: " << name() << '\n'
       << m_nbhdTOBContainerReadKey << '\n'
       << '\n';
    return ss.str();
  }

}

