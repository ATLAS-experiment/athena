/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CORACOOL_CORACOOLDATABASESVCFACTORY_H
#define CORACOOL_CORACOOLDATABASESVCFACTORY_H
// CoraCoolDatabaseSvcFactory.h 
// Singleton factory class for CoraCoolDatabaseSvc instances
// Uses underlying COOL utilities to load CORAL libraries and instantiate
// both COOL and CoraCool for use in standalone applications.
//
// Richard Hawkings, started 27/10/06

#include "CoraCool/CoraCoolDatabaseSvc.h"
#include "CxxUtils/checker_macros.h"

class ATLAS_NOT_THREAD_SAFE CoraCoolDatabaseSvcFactory {
 public:
  static CoraCoolDatabaseSvc& databaseService ATLAS_NOT_THREAD_SAFE ();
};
#endif // CORACOOL_CORACOOLDATABASESVCFACTORY_H
