
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmSortSelectCountContainerComparator.h"

#include "../../../dump.h"
#include "../../../dump.icc"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {
  

  
  eEmSortSelectCountContainerComparator::eEmSortSelectCountContainerComparator(const std::string& type,
									 const std::string& name,
									 const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode eEmSortSelectCountContainerComparator::initialize() {
       
    CHECK(m_HypoFIFOReadKey.initialize());
    CHECK(m_portsOutReadKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  eEmSortSelectCountContainerComparator::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in input data (a FIFO) for GepAlgoHypothesis from the event store

    auto fifo =
      SG::ReadHandle<GlobalSim::GepAlgoHypothesisFIFO>(m_HypoFIFOReadKey,
						       ctx);
    CHECK(fifo.isValid());
     
    ATH_MSG_DEBUG("read in GepAlgoHypothesis fifo ");

    auto ports_out =
      SG::ReadHandle<GlobalSim::eEmSortSelectCountContainerPortsOut>(
								     m_portsOutReadKey,
								     ctx);
    CHECK(ports_out.isValid());

    {
      std::stringstream ss;
      ss << "eEmTobs from FIFO:\n";
      for (const auto& i : *fifo) {
	ss << i.m_I_eEmTobs << '\n';
      }

      ATH_MSG_DEBUG(ss.str());
    }

    {
      std::stringstream ss;
      ss << "eEmSortSelectCountContainerPortsOut tob bits:\n";
      for (const auto& tob : ports_out->m_O_eEmGenTob) {
	ss << tob->as_bits() << '\n';
      }
      ss << '\n';
      ATH_MSG_DEBUG(ss.str());
    }
    
    {
      std::stringstream ss;
      ss << "eEmSortSelectCountContainerPortsOut multiplicity bits:\n";
      ss << *(ports_out->m_O_Multiplicity) << '\n';
      ATH_MSG_DEBUG(ss.str());
    }
 
   
    return StatusCode::SUCCESS;
  }

  std::string
  eEmSortSelectCountContainerComparator::toString() const {
    
    std::stringstream ss;
    ss << "eEmSortSelectCountContainerComparator.name: " << name() << '\n'
       << m_HypoFIFOReadKey << '\n'
       << m_portsOutReadKey
       << '\n';
    return ss.str();
  }
}

