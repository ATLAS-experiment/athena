/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_JJETINPUTALGTOOL_H
#define GLOBALSIM_JJETINPUTALGTOOL_H

/**
 * AlgTool to obtain a GlobalSim::jJetTOBArray from
 * a jFexSRJetRoIContainer
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/jJetTOBArray.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"


#include <string>

namespace GlobalSim {

  class jJetInputAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    jJetInputAlgTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);
    
    virtual ~jJetInputAlgTool() = default;

    StatusCode initialize() override;

    // adapted from L1TopoSimulation jFexInputProvider
    
    virtual StatusCode run(const EventContext& ctx) const override;

    virtual std::string toString() const override;
    
    
  private:
   
    SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_jJetRoIKey{this,
	"jFexSRJetRoIKey", "L1_jFexSRJetRoI", "jFEX Jet EDM"};

    SG::WriteHandleKey<GlobalSim::jJetTOBArray>
    m_jJetTOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			    "key to write out an jJetTOBArray"};


    static constexpr int s_Et_conversion{2};               // 200 MeV to 100 MeV
    static constexpr double s_EtDouble_conversion{0.1};    // 100 MeV to GeV
    static constexpr double s_phiDouble_conversion{0.05};  // 20 x phi to phi
    static constexpr double s_etaDouble_conversion{0.025}; // 40 x eta to eta
    
  };
}
#endif
