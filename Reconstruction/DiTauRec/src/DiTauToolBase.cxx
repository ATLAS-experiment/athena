/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/DiTauToolBase.h"
 
static const InterfaceID DiTauToolBaseID( "DiTauToolBase", 1, 0 );
 
const InterfaceID& DiTauToolBase::interfaceID() {
  return DiTauToolBaseID;
}
 
 
DiTauToolBase::DiTauToolBase( const std::string &type,
			      const std::string &name,
			      const IInterface *parent ):
  AthAlgTool(type, name, parent)
{
}
 

DiTauToolBase::~DiTauToolBase()
= default;
 

StatusCode DiTauToolBase::initialize()
{
  return StatusCode::SUCCESS;
}
 

StatusCode DiTauToolBase::eventInitialize( DiTauCandidateData *)
{
  return StatusCode::SUCCESS;
}
 

StatusCode DiTauToolBase::execute( DiTauCandidateData *,
				   const EventContext& /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

StatusCode DiTauToolBase::executeObj( xAOD::DiTauJet& ,
                                      const EventContext& /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

StatusCode DiTauToolBase::finalize()
{
  return StatusCode::SUCCESS;
}
 
 
// Helpers
 
template <class T>
bool DiTauToolBase::retrieveTool(T & tool) {
  if (tool.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool " << tool);
    return false;
  } else {
    ATH_MSG_VERBOSE("Retrieved tool " << tool);
  }
  return true;
}
