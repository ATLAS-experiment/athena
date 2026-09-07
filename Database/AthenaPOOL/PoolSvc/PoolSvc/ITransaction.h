/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ITRANSACTION_H
#define POOLSVC_ITRANSACTION_H

#include "StorageSvc/pool.h"

/*
 *   POOL namespace declaration
 */
namespace pool {

  /** @class ITransaction ITransaction.h PoolSvc/ITransaction.h
   *
   *  ITransaction is the interface class for user (macroscopic transactions)
   *  Every operation with the pool storage system should be performed within a transaction.
   */

  class ITransaction {
  public:
    /// Starts a new transaction. Returns the success of the operation
    virtual bool start( Io::IoFlag type = Io::READ ) = 0;

    /// Commits the transaction.
    virtual bool commit() = 0;

    /// Commits the holds transaction.
    virtual bool commitAndHold() = 0;

    /// Checks if the transaction is active
    virtual bool isActive() const = 0;

    /// Returns the transaction type
    virtual Io::IoFlag type() const = 0;

  protected:
    /// Default destructor
    virtual ~ITransaction() = default;  
  };

}

#endif

