/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_JJETSELECTALGTOOL_H
#define GLOBALSIM_JJETSELECTALGTOOL_H

/**
 * AlgTool run the L1Topo jJetSelect SORT Algorithm
 */

#include "../IL1TopoAlgTool.h"
#include "../IO/jJetTOBArray.h"
#include "../IO/GenericTOBArray.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

namespace GlobalSim {

  class jJetSelectAlgTool: public extends<AthAlgTool, IL1TopoAlgTool> {

  public:
    jJetSelectAlgTool(const std::string& type,
		      const std::string& name,
		      const IInterface* parent);
    
    virtual ~jJetSelectAlgTool() = default;

    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;

    virtual std::string toString() const override;
    
  private:
    
    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};
    
    // variable names taken from L1Menu
    Gaudi::Property<unsigned int> m_InputWidth{
      this,
      "InputWidth",
      {0},
      "Maximum number of TOBS to output"
    };
    
    Gaudi::Property<unsigned int> m_MinET{
      this,
      "MinET",
      {0},
      "jJetSelect construction parameter"
    };

    Gaudi::Property<unsigned int> m_MinEta{
      this,
      "MinEta",
      {0},
      "jJetSelect construction parameter"
    };

    
    Gaudi::Property<unsigned int> m_MaxEta{
      this,
      "MaxEta",
      {0},
      "jJetSelect construction parameter"
    };

    
    SG::ReadHandleKey<GlobalSim::jJetTOBArray>
    m_jJetTOBArrayReadKey {this, "TOBArrayReadKey", "",
			   "key to read in an jJetTOBArray"};

        
    SG::WriteHandleKey<GlobalSim::GenericTOBArray>
    m_TOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			"key to write out selected generic TOBs"};
    
    
  };
}
#endif
