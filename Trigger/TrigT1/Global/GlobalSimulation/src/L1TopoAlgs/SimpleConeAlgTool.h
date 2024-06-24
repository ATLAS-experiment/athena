/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_SIMPLECONEALGTOOL_H
#define GLOBALSIM_SIMPLECONEALGTOOL_H

/**
 * AlgTool run the L1Topo SimpleCone DECISIOM Algorithm
 */

#include "../IL1TopoAlgTool.h"
#include "../IO/GenericTOBArray.h"
#include "../IO/Decision.h"
#include "../IO/GenericTOBArrayVector_clid.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include <ostream>
#include <string>
#include <vector>

namespace GlobalSim {
  
  class SimpleConeAlgTool: public extends<AthAlgTool, IL1TopoAlgTool> {

    
  public:
    SimpleConeAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~SimpleConeAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    
    virtual std::string toString() const override;
    
  private:
    
    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};
    
    Gaudi::Property<unsigned int> m_InputWidth {
      this,
      "InputWidth",
      {0u},
      "max number of input objects to consider"};
       
    Gaudi::Property<unsigned int> m_MinET {
      this,
      "MinET",
      {0u},
      "min RT for contributing TOB"};
    
     
    Gaudi::Property<std::vector<unsigned int>> m_MinSumET {
      this,
      "MinSumET",
      {},
      "minum sumET for cone jet. one value per oputput bit"};
    
    Gaudi::Property<unsigned int> m_MaxRSqr{
      this,
      "MaxRSqr",
      {0u},
      "square radius of cone"};

    
    ToolHandle<GenericMonitoringTool>
    m_monTool{this, "monTool", {}, "MonitoringTool"};

    SG::ReadHandleKey<GlobalSim::GenericTOBArray>
    m_genericTOBArrayReadKey {this, "TOBArrayReadKey", "",
			      "key to read in a genericTauTOBArray"};

    SG::WriteHandleKey<std::vector<GlobalSim::GenericTOBArray>>
    m_genericTOBArrayVectorWriteKey{this,
				    "TOBArrayVectorWriteKey",
				    "",
				    "key to write out a vector<GlobalSim::GenericTOBArray>"};
    
    SG::WriteHandleKey<GlobalSim::Decision>
    m_decisionWriteKey {this, "DecisionWriteKey", "",
			"key to write out an GlobalSim::Decision object"};
    
  };
}
#endif
