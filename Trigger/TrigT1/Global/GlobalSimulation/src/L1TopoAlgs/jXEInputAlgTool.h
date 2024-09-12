/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_JXEINPUTALGTOOL_H
#define GLOBALSIM_JXEINPUTALGTOOL_H

/**
 * AlgTool to obtain a GlobalSim::jXETOBArray from a jFexMETRoIContainer
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/jXETOBArray.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include <string>

namespace GlobalSim {
 
  class jXEInputAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    jXEInputAlgTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);
    
    virtual ~jXEInputAlgTool() = default;

    StatusCode initialize() override;

    // adapted from L1TopoSimulation jFexInputProvider

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
    
  private:
   
    SG::ReadHandleKey<xAOD::jFexMETRoIContainer> m_jFexMETRoIKey{this,
	"jFexMETRoIKey", "L1_jFexMETRoI", "jFEX TE EDM"};

    SG::WriteHandleKey<GlobalSim::jXETOBArray>
    m_jXETOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			    "key to write out an jXETOBArray"};
    
    static constexpr int s_Et_conversion{2};  // 200 MeV to 100 MeV
    static constexpr double s_EtDouble_conversion{0.1}; // 100 MeV to GeV;

    
    ToolHandle<GenericMonitoringTool> m_monTool {
      this, "monTool", "", "Monitoring tool to create histograms"};
    
  };
}
#endif
