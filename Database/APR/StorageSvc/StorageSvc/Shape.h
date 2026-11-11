/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGE_SHAPE_H
#define STORAGE_SHAPE_H

// Framework include files
#include "PersistentDataModel/Guid.h"
#include <string>

/** @class Shape Shape.h StorageSvc/Shape.h
    *
    *  Base class for DbTypeInfo. 
    *
    * @author   M.Frank
    */
class Shape  {
  protected:
    /// Object global identifier
    Guid                  m_id;
  public:
    /// Standard Constructor
    Shape()                                   {                               }
    /// Standard Destructor
    virtual ~Shape()                          {                               }
    /// Access database identifier
    const Guid& shapeID() const               {  return m_id;                 }
          Guid& shapeID()                     {  return m_id;                 }
    /// Access database identifier
    void setShapeID(const Guid& id)           {  m_id=id;                     }
    Guid& setShapeID(const std::string &id)   {  m_id.fromString(id); return  shapeID(); }
};

#endif
