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
       
    CHECK(m_tobsInReadKey1.initialize());
    CHECK(m_tobsInReadKey2.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

    auto tobs1 =
      SG::ReadHandle<GenericTobContainer>(m_tobsInReadKey1,
					  ctx);

    auto tobs2 =
      SG::ReadHandle<GenericTobContainer>(m_tobsInReadKey2,
					  ctx);

    auto ss = std::stringstream();
    ss << "Tobs 1 in\n";
    for (const auto& tob: *tobs1) {
      ss << *tob << '\n';
    }

    ss << "Tobs 2 in\n";
    for (const auto& tob: *tobs1) {
      ss << *tob << '\n';
    }
    ATH_MSG_DEBUG(ss.str());

    return StatusCode::SUCCESS;
  }

  std::string
  InvariantMassDeltaPhiInclusive2AlgTool::toString() const {

    std::stringstream ss;
    ss << "name: " << name() << '\n'
       << " tobs in 1 read key " << m_tobsInReadKey1
       << " tobs in 2 read key " << m_tobsInReadKey2
       << '\n';
    
    return ss.str();
  }
}

