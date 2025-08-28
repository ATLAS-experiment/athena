/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
// $Id: DbSelect.h 726071 2016-02-25 09:23:05Z krasznaa $
//
//  Package    : pool/StorageSvc (The pool framework)
//
//  @author      M.Frank
//
//====================================================================
// $Header: /cvs/PF/pool/StorageSvc/StorageSvc/DbSelect.h,v 1.9 2007/11/20 16:43:06 frankb Exp $
#ifndef POOL_DBSELECT_H
#define POOL_DBSELECT_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbStatus.h"

// C++ include files
#include <string>

/*
 *  POOL namespace declaration
 */
namespace pool    {

  // Forward declaration
  class DbSelectContext;
  class DbDatabase;
  class DbContainer;

  /** @class DbSelect DbSelect.h StorageSvc/DbSelect.h
    *
    * Definition of the DbSelect class.
    *
    * Class encapsulating select statement.
    *
    * @author  Markus Frank                                               
    * @version 1                                                         
    */
  class DbSelect  {
  private:
    /// Pointer with database context
    DbSelectContext*  m_context;
    /// No copy constructor
    DbSelect(const DbSelect&) = delete;
    DbSelect& operator= (const DbSelect&) = delete;
  public:
    /// Constructor
    DbSelect();
    /// Standard destructor
    virtual ~DbSelect();
    /// Access to link handle
    Token::OID_t& link() const;
    /// Access referring database context
    DbDatabase& database() const;
    /// Set database context
    void setDatabase(const DbDatabase& dbcH);
    /// Access referring container context
    DbContainer& container() const;
    /// Set container context
    void setContainer(DbContainer& cntH);
    /// Access shape ID
    const Guid& shapeID() const;
    /// Set shape ID
    void setShapeID(const Guid& id);
    /// Start selective iterator over a container in a database
    DbStatus start(DbDatabase& dbH, const std::string& cntName);
    /// Access the next token of a sequence
    DbStatus next(Token*& refpToken);
  };
}      // End namespace pool
#endif // POOL_DBSELECT_H
