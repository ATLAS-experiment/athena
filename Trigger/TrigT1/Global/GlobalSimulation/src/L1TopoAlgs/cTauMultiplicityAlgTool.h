/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_CTAUMULTIPLICITYALGTOOL_H
#define GLOBALSIM_CTAUMULTIPLICITYALGTOOL_H

/**
 * AlgTool run the L1Topo cTauMultiplicty COUNT Algorithm
 */

#include "../L1MenuResources.h"
#include "../IL1TopoAlgTool.h"
#include "../IO/cTauTOBArray.h"
#include "../IO/Count.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include <string>



namespace GlobalSim {

  class cTauMultiplicity;
  
  class cTauMultiplicityAlgTool: public extends<AthAlgTool, IL1TopoAlgTool> {
    
  public:
    cTauMultiplicityAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~cTauMultiplicityAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:
    
    Gaudi::Property<std::string> m_algInstanceName {
      this,
	"alg_instance_name",
	  {},
	"instance name of concrete L1Topo Algorithm"};
    
    Gaudi::Property<unsigned int>
    m_nbits{this,
	"nbits",
	  {0u},
	"number of output bits to set in TCS::Count"};

    Gaudi::Property<bool>
    m_doDump{this,
	     "do_dump",
	     {false},
	     "flag to enable dumps"};
    
 
    std::unique_ptr<L1MenuResources> m_l1MenuResources{nullptr};

    SG::ReadHandleKey<GlobalSim::cTauTOBArray>
    m_cTauTOBArrayReadKey {this, "TOBArrayReadKey", "",
			  "key to read in an cTauTOBArray"};

    SG::WriteHandleKey<GlobalSim::Count>
    m_CountWriteKey {this, "CountWriteKey", "",
		     "key to write out a GlobalSim::Count object"};
    
    
    ToolHandle<GenericMonitoringTool>
    m_monTool{this, "monTool", {}, "MonitoringTool"};

    StatusCode monitor(const cTauMultiplicity&) const; // pass alg to monitor

  };
}
#endif
