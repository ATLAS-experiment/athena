/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_DBREFLEX_H
#define STORAGESVC_DBREFLEX_H

// Framework include files
#include "PersistentDataModel/Guid.h"
#include "DataModelRoot/RootType.h"

// C++ include files
#include <string>
#include <vector>
#include <typeinfo>
#include <set>

/** @class DbReflex DbReflex.h StorageSvc/DbReflex.h
    *
    * Description:
    * Provides mapping between type IDs (GUIDs) and type description objects (RootType)
    * Uses ROOT automatic dictionaru loading to find types
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

#endif 
