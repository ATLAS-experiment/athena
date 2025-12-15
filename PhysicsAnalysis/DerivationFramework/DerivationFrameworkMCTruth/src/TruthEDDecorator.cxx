/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Class header file
#include "TruthEDDecorator.h"

namespace DerivationFramework {

  StatusCode TruthEDDecorator::initialize(){

    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_eventShapeKeys.initialize());
    ATH_CHECK(m_eventDensityDecorKeys.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode TruthEDDecorator::addBranches(const EventContext& ctx) const{
    ATH_MSG_VERBOSE("addBranches()");


    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    if (!eventInfo.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve " << m_eventInfoKey);
      return StatusCode::FAILURE;
    }

    for (size_t i=0;i<m_eventShapeKeys.size();++i){
      // Get the event shapes from which we'll get the densities
      SG::ReadHandle<xAOD::EventShape> eventShape(m_eventShapeKeys[i], ctx);
      if (!eventShape.isValid()) {
        ATH_MSG_ERROR ("Could not retrieve " << m_eventShapeKeys[i]);
        return StatusCode::FAILURE;
      }

      // Decorate the densities onto the event info
      SG::WriteDecorHandle<xAOD::EventInfo, double> dec_eventDensity(m_eventDensityDecorKeys[i], ctx);
      dec_eventDensity(*eventInfo) = eventShape->getDensity(xAOD::EventShape::Density);
    }

    return StatusCode::SUCCESS;
  }

} /// namespace
