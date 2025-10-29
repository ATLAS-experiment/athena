/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ERatioAlgTool.h"
#include "ERatio.h"
#include "../Utilities/dump.h"
#include "../Utilities/dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"


namespace GlobalSim {
  ERatioAlgTool::ERatioAlgTool(const std::string& type,
					       const std::string& name,
					       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode ERatioAlgTool::initialize() {
       
    CHECK(m_nbhdContainerReadKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  ERatioAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

    // instantiate Algorithm object
    auto alg = ERatio(m_algInstanceName, m_minDeltaE, m_maxERCut);

    // read in LArStrip neighborhoods from the event store
    // there is one neighborhood per EFex RoI
    auto in =
      SG::ReadHandle<LArStripNeighborhoodContainer>(m_nbhdContainerReadKey,
						    ctx);
    CHECK(in.isValid());

    ATH_MSG_DEBUG("read in " << (*in).size() << " neighborhoods");


    if (m_enableDump) {
      dump(name() + "_"  + std::to_string(ctx.evt()), alg);
    }

    std::vector<bool> found(in->size(), false);

    // check for the presence of a pi0 candidate in each neighborhood.
    std::size_t inbhd{0};
    for (const auto nbhd : *in) {
      bool result{false};
      CHECK(alg.run(*nbhd, result));
      found.at(inbhd++) = result;
    }


    for (const auto res : found) {
      ATH_MSG_DEBUG("neighborhood result: " <<std::boolalpha << res);
    }

    
    return StatusCode::SUCCESS;
  }

  std::string ERatioAlgTool::toString() const {

    std::stringstream ss;
    ss << "ERatioAlgTool. name: " << name() << '\n'
       << m_nbhdContainerReadKey << '\n'
       << ERatio(m_algInstanceName).toString()
       << '\n';
    return ss.str();
  }
  
}

