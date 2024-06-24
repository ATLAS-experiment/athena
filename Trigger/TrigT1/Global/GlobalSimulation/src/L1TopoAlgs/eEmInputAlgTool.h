/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMINPUTALGTOOL_H
#define GLOBALSIM_EEMINPUTALGTOOL_H

/**
 * AlgTool to obtain a GlobalSim::eEmTOBArray
 */

#include "../IL1TopoAlgTool.h"
#include "../IO/eEmTOBArray.h"


#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"


#include <string>

namespace GlobalSim {

  class eEmInputAlgTool: public extends<AthAlgTool, IL1TopoAlgTool> {
    
  public:
    eEmInputAlgTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);
    
    virtual ~eEmInputAlgTool() = default;

    StatusCode initialize() override;

    // adapted from L1TopoSimulation eFexInputProvider
 
    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
    
  private:
   
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer>
    m_eEmRoIKey {this, "eFexEMRoIKey", "L1_eEMRoI", "eFEXEM EDM"};

    SG::WriteHandleKey<GlobalSim::eEmTOBArray>
    m_eEmTOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			   "key to write out an eEmTOBArray"};

    static constexpr double s_EtDouble_conversion{0.1};     // 100 MeV to GeV
    static constexpr double s_phiDouble_conversion{0.05};   // 20 x phi to phi
    static constexpr double s_etaDouble_conversion{0.025};  // 40 x eta to eta
    
  };
}
#endif
