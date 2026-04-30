/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./eFexRoIAlgTool.h"
#include "L1TopoEvent/eEmTOB.h"

#include <cmath> //for abs

namespace GlobalSim {
  
  eFexRoIAlgTool::eFexRoIAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent):
    AthAlgTool(type, name, parent){
  }

  StatusCode eFexRoIAlgTool::initialize() {
    CHECK(m_eEmRoIKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  eFexRoIAlgTool::RoIs(std::vector<const xAOD::eFexEMRoI*>& selectedRoIs,
		       const EventContext& ctx) const {
    SG::ReadHandle<xAOD::eFexEMRoIContainer>
      eFexEMRoIContainer(m_eEmRoIKey, ctx);
    
    CHECK(eFexEMRoIContainer.isValid());

      
      
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
    
 
    auto roiSelector = [&etMin=m_etMin,
			&etaMin=m_etaMin,
			&etaMax=m_etaMax](const auto& roi) {

      auto abs_eta = std::abs(roi->eta());
      return (etaMin <= abs_eta) and
	(abs_eta < etaMax) and
	(roi->et() > etMin);
    };

    std::copy_if((*eFexEMRoIContainer).begin(),
		 (*eFexEMRoIContainer).end(),
		 std::back_inserter(selectedRoIs),
		 std::move(roiSelector));
    
    return StatusCode::SUCCESS;
  }

  std::string eFexRoIAlgTool::toString() const {
    std::string s = "eFexRoIAlgTool: name" + name() + '\n'
       + m_eEmRoIKey.key() + '\n';
    return s;
  }
}

