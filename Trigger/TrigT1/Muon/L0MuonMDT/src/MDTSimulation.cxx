
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSimulation.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"

namespace L0Muon {


StatusCode MDTSimulation::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_keyMdtRdo.initialize());
  ATH_CHECK(m_cablingKey.initialize());

  /// container of output RoIs
  ATH_CHECK(m_outputMuonRoIKey.initialize());

  /// retrieve the monitoring tool
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode MDTSimulation::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  SG::ReadHandle inputRDO(m_keyMdtRdo,ctx);
  ATH_CHECK(inputRDO.isPresent());
  ATH_MSG_DEBUG("Number of MDT RDO: " <<  inputRDO->size());

  SG::ReadCondHandle cablingMap{m_cablingKey, ctx};
  ATH_CHECK(cablingMap.isValid());

  /// monitor RDO quantities
  if ( !m_monTool.empty() ) {
    auto n_of_RDO = Monitored::Scalar<unsigned int>("n_of_RDO",inputRDO->size());
  }

  return StatusCode::SUCCESS;
}


}   // end of namespace
