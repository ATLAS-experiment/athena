/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AssociationUtils/FEAssociationAlg.h"

namespace ORUtils
{

#ifndef XAOD_STANDALONE
FEAssociationAlg::FEAssociationAlg(const std::string& name, ISvcLocator* svcLoc)
  : AthReentrantAlgorithm(name, svcLoc)
{}
#else
FEAssociationAlg::FEAssociationAlg(const std::string& name, ISvcLocator* svcLoc)
  : EL::AnaAlgorithm(name, svcLoc)
{}
#endif

StatusCode FEAssociationAlg::initialize()
{
  ATH_CHECK(m_tool.retrieve());
  return StatusCode::SUCCESS;
}

#ifndef XAOD_STANDALONE
StatusCode FEAssociationAlg::execute(const EventContext& ctx) const
{
  return m_tool->buildAssociations(ctx);
}
#else
StatusCode FEAssociationAlg::execute()
{
  return m_tool->buildAssociations();
}
#endif

} // namespace ORUtils