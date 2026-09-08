/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: DbReflex.h 601961 2014-06-16 14:49:09Z mnowak $
//====================================================================
//  DbDatabase and related class definitions
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBREFLEX_H
#define POOL_DBREFLEX_H

// Framework include files
#include "PersistentDataModel/Guid.h"
#include "DataModelRoot/RootType.h"

// C++ include files
#include <string>
#include <vector>
#include <typeinfo>
#include <set>

/*
 *  POOL namespace declaration
 */
namespace pool  {
  /** @class DbReflex DbReflex.h StorageSvc/DbReflex.h
    *
    * Description:
    * Handle transparent loading of reflection classes.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class DbReflex  {
  private:
    /// No public construction
    DbReflex() {}
    /// No public destruction
    ~DbReflex() {}
  public:
    /// Access classes by Guid
    /** @param info     [IN]    Reference to Guid
      *
      * @return  Pointer to reflection class if present.
      */
    static const RootType forGuid(const Guid& info);

    /// Determine Guid (normalized string form) from reflection type.
    /** @param type     [IN]    Reflection type 
      *
      * @return  String containing full scoped name
      */
    static Guid guid(const RootType& id);
  };
}       // End namespace pool
#endif  // POOL_DBREFLEX_H
