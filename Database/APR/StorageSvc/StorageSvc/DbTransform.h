/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_DBTRANSFORM_H
#define STORAGESVC_DBTRANSFORM_H

#include <string>

// Framework includes
#include "GaudiKernel/StatusCode.h"

class Guid;

class DbTypeInfo;

  /** @class DbTransform DbTransform.h StorageSvc/DbTransform.h
    *
    * @author   M.Frank
    */
class DbTransform    {
  private:
    /// This class does not want to be instantiated....
    DbTransform()   {}
  public:
    /// Get the main shape matching the class GUID
    static StatusCode getShape(const Guid&        shape_Guid,
                               const DbTypeInfo*& shape);
    /// Get the shape matching the class GUID and the exact type shape given as string rep
    static const DbTypeInfo* getShape(const Guid& shape_Guid, const std::string& rep);
    /// Access entry in shape registry
    static StatusCode regShape    (const DbTypeInfo*  shape);

   /// keep shape until the end
    static void ownShape(const DbTypeInfo*  shape);
    
   /// Access entry in shape registry
    static StatusCode removeShape (const DbTypeInfo*  shape);
};

#endif
