/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthAlgorithm.h"

#include "CxxUtils/checker_macros.h"


AthAlgorithm::AthAlgorithm( const std::string& name,
                            ISvcLocator* pSvcLocator ) :
  AthCommonAlgorithm<Gaudi::Algorithm>( name, pSvcLocator )
{
  // default cardinality for non-reentrant algorithms
  setProperty( "Cardinality", 1 ).orThrow("Unable to set property 'Cardinality'", name);
}


StatusCode AthAlgorithm::execute ( const EventContext& ctx ) const
{
  // "Thread-safe" because scheduler ensures algorithm never gets called concurrently.
  auto nc_this ATLAS_THREAD_SAFE = const_cast<AthAlgorithm*>( this );
  return nc_this->execute( ctx );
}
