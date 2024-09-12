/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_DELTARSQRINCL2ALGTOOL_H
#define GLOBALSIM_DELTARSQRINCL2ALGTOOL_H

/**
 * AlgTool run the L1Topo DeltaRSqrIncl2 DECISION Algorithm
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/GenericTOBArray.h"
#include "../IO/Decision.h"
#include "../IO/GenericTOBArrayVector_clid.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
  class DeltaRSqrIncl2AlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    DeltaRSqrIncl2AlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~DeltaRSqrIncl2AlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:
    
    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};

    Gaudi::Property<unsigned int> m_MaxTOB1 {
      this,
      "MaxTOB1",
      0u,
      "maximum of the obs for argument 1 to consider"};

    Gaudi::Property<unsigned int> m_MaxTOB2 {
      this,
      "MaxTOB2",
      0u,
      "maximum of the obs for argument 2 to consider"};

        
    Gaudi::Property<std::vector<unsigned int>> m_MinET1 {
      this,
      "MinET1",
      {},
      "Min ET values for argument 1, one per output bit"
    };

            
    Gaudi::Property<std::vector<unsigned int>> m_MinET2 {
      this,
      "MinET2",
      {},
      "Min ET values for argument 2, one per output bit"
    };

    
    Gaudi::Property<std::vector<unsigned int>> m_DeltaRMin {
      this,
      "DeltaRMin",
      {},
      "Delta R min values, one per output bit"
    };

        
    Gaudi::Property<std::vector<unsigned int>> m_DeltaRMax {
      this,
      "DeltaRMax",
      {},
      "Delta R max values, one per output bit"
    };


    Gaudi::Property<unsigned int> m_NumResultBits {
      this,
      "NumResultBits",
      {0u},
      "number of bits to set in the Decision object"
    };

    Gaudi::Property<bool>
    m_doDump{this,
	     "do_dump",
	     {false},
	     "flag to enable dumps"};
    
    ToolHandle<GenericMonitoringTool>
    m_monTool{this, "monTool", {}, "MonitoringTool"};
    
    
    SG::ReadHandleKey<GlobalSim::GenericTOBArray>
    m_genericTOBArrayReadKey0 {this, "TOBArrayReadKey0", "",
			      "key to read in a genericTOBArray 1st arg"};

    
    SG::ReadHandleKey<GlobalSim::GenericTOBArray>
    m_genericTOBArrayReadKey1 {this, "TOBArrayReadKey1", "",
			      "key to read in a genericTOBArray 2nd arg"};


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
