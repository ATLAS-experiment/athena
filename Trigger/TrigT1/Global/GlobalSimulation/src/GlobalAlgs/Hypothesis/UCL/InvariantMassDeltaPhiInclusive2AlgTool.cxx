
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InvariantMassDeltaPhiInclusive2AlgTool.h"
#include "AlgoDataTypes.h"  // bitSetToInt()
#include "DataCollector.h"

#include "../../../dump.h"
#include "../../../dump.icc"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {
  
  InvariantMassDeltaPhiInclusive2AlgTool::InvariantMassDeltaPhiInclusive2AlgTool(const std::string& type,
									 const std::string& name,
									 const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode InvariantMassDeltaPhiInclusive2AlgTool::initialize() {
       
    CHECK(m_portsInReadKey.initialize());
    CHECK(m_portsOutWriteKey.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::run(const EventContext&) const {
    ATH_MSG_DEBUG("run()");

     return StatusCode::SUCCESS;
  }

  std::string
  InvariantMassDeltaPhiInclusive2AlgTool::toString() const {

    std::stringstream ss;
    ss << "name: " << name() << '\n'
       << " read key " <<m_portsInReadKey
       << " write key " << m_portsOutWriteKey
       << '\n';
    
    return ss.str();
  }
}

