/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ENERGYTHRESHOLDALGTOOL_JXE_H
#define GLOBALSIM_ENERGYTHRESHOLDALGTOOL_JXE_H

/**
 * AlgTool run the L1Topo EnergyThreshold COUNT Algorithm
 */

#include "../IL1TopoAlgTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include <string>

namespace GlobalSim {

  class jXETOBArray;
  class Count;

  class EnergyThresholdAlgTool_jXE: public extends<AthAlgTool, IL1TopoAlgTool> {
    
  public:
    EnergyThresholdAlgTool_jXE(const std::string& type,
				const std::string& name,
				const IInterface* parent);
    
    virtual ~EnergyThresholdAlgTool_jXE() = default;
    
    StatusCode initialize() override;

    virtual StatusCode
    run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:

    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};

    Gaudi::Property<unsigned int> m_nbits {
      this,
      "nbits",
      {0u},
      "number of  bits to be set in TCS::Count"};

    Gaudi::Property<unsigned int> m_threshold {
      this,
      "hundredMeVThreshold",
      {0u},
      "threshold from menu * 10"};

    SG::ReadHandleKey<GlobalSim::jXETOBArray>
    m_jXETOBArrayReadKey {this, "TOBArrayReadKey", "",
			  "key to read in an jXETOBArray"};

    SG::WriteHandleKey<GlobalSim::Count>
    m_CountWriteKey {this, "CountWriteKey", "",
		     "key to write out an L1Topo Count object"};
    

    ToolHandle<GenericMonitoringTool>
    m_monTool{this, "monTool", {}, "MonitoringTool"};
  };
}
#endif
