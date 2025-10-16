/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTop/TopHeavyFlavorFilterAugmentation.h"
#include "StoreGate/WriteDecorHandle.h"


namespace DerivationFramework {


TopHeavyFlavorFilterAugmentation::TopHeavyFlavorFilterAugmentation(const std::string& t, const std::string& n, const IInterface* p):
  base_class(t,n,p)
{
}



TopHeavyFlavorFilterAugmentation::~TopHeavyFlavorFilterAugmentation() = default;



StatusCode TopHeavyFlavorFilterAugmentation::initialize(){
  ATH_MSG_DEBUG("Initialize " );
  ATH_CHECK(m_eventInfoName.initialize());
  ATH_CHECK(m_filterTool.retrieve());

  return StatusCode::SUCCESS;

}



StatusCode TopHeavyFlavorFilterAugmentation::addBranches(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoName, ctx};
  if (!eventInfo.isValid()) {
    ATH_MSG_ERROR("could not retrieve event info " <<m_eventInfoName);
    return StatusCode::FAILURE;
  }

  const int flavortype=m_filterTool->filterFlag();
  SG::WriteDecorHandle<xAOD::EventInfo, int> decoration{m_filterFlagKey, ctx};
  decoration(*eventInfo) = flavortype;

 return StatusCode::SUCCESS;

}



} /// namespace
