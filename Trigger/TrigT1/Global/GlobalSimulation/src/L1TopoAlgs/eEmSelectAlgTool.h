/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSELECTALGTOOL_H
#define GLOBALSIM_EEMSELECTALGTOOL_H

/**
 * AlgTool run the L1Topo eEmSelect SORT Algorithm
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/eEmTOBArray.h"
#include "../IO/GenericTOBArray.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

namespace GlobalSim {
  
  class eEmSelectAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    eEmSelectAlgTool(const std::string& type,
		     const std::string& name,
		     const IInterface* parent);
    
    virtual ~eEmSelectAlgTool() = default;
    
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
      "eEmSelect construction parameter"
    };

    Gaudi::Property<unsigned int> m_REtaMin{
      this,
      "REtaMin",
      {0},
      "eEmSelect construction parameter"
    };

    Gaudi::Property<unsigned int> m_RHadMin{
      this,
      "RHadMin",
      {0},
      "eEmSelect construction parameter"
    };

    Gaudi::Property<unsigned int> m_WsTotMin{
      this,
      "WsTotMin",
      {0},
      "eEmSelect construction parameter"
    };


    SG::ReadHandleKey<GlobalSim::eEmTOBArray>
    m_eEmTOBArrayReadKey {this, "TOBArrayReadKey", "",
			  "key to read in an eEmTOBArray"};
    
    SG::WriteHandleKey<GlobalSim::GenericTOBArray>
    m_TOBArrayWriteKey {this, "TOBArrayWriteKey", "",
			"key to write out selected generic TOBs"};
    
  };
}
#endif
