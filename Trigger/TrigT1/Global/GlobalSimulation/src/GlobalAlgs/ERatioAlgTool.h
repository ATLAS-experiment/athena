/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ERATIOALGTOOL_H
#define GLOBALSIM_ERATIOALGTOOL_H

/**
 * AlgTool to read in LArStripNeighborhoods, and run the ERatio Algorithm.
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/LArStripNeighborhoodContainer.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
  class ERatioAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    ERatioAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~ERatioAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

  private:
    
    // parameter used by ERatio
    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};

    Gaudi::Property<bool>
    m_enableDump{this,
	     "enableDump",
	     {false},
	     "flag to enable dumps"};

    // parameter used by ERatio
    Gaudi::Property<double>
    m_minDeltaE{this,
		"minDeltaE",
		{50.0},
		"min energy between strips considered significant"};

        
    // parameter used by ERatio
    Gaudi::Property<double>
    m_maxERCut{this,
		"maxERCut",
		{2.5},
		"max acceptable peak ratio for pi0 acceptance"};


    // input to the  ERatio Algorithm
    SG::ReadHandleKey<LArStripNeighborhoodContainer>
    m_nbhdContainerReadKey {
      this,
      "LArNeighborhoodContainerReadKey",
      "stripNeighborhoodContainer",
      "key to read inLArNeighborhoodReadKeys"};

  };
}
#endif
