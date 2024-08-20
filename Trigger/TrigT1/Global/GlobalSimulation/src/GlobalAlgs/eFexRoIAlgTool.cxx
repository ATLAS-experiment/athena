/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./eFexRoIAlgTool.h"
#include "L1TopoEvent/eEmTOB.h"

#include <numbers>

#include <sstream>

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
    
 
    auto roiSelector = [etMin=m_etMin](const auto& roi) {

      // fiducial cut values
      constexpr double eta_out_fid{1.4};
      constexpr double eta_in_fid{0.2};
      
      auto abs_eta = abs(roi->eta());
      return (eta_in_fid <= abs_eta) and
	(abs_eta < eta_out_fid) and
	(roi->et() > etMin);
    };

    std::copy_if((*eFexEMRoIContainer).begin(),
		 (*eFexEMRoIContainer).end(),
		 std::back_inserter(selectedRoIs),
		 roiSelector);
    
    return StatusCode::SUCCESS;
  }

  std::string eFexRoIAlgTool::toString() const {
    std::stringstream ss;
    ss << "eFexRoIAlgTool: name" << name() << '\n'
       << m_eEmRoIKey << '\n';
    return ss.str();
  }
}

